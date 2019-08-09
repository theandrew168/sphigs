#ifndef lint
static char Version[]=
   "$Id: sph_falloc.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_falloc.c
 *
 *      Optimized dynamic memory blocks, ideal for large data chunks that 
 * grow rapidly and in successive steps.
 * ------------------------------------------------------------------------- */

#define REPORT_ERROR  SPH__error
#include "macros.h"

#include "HEADERS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sph_errtypes.h"
#include "falloc.h"
#include "fallocdefs.h"

#ifdef THINK_C
void SPH__error (...);
#endif

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Allocates a new chunk.
 * ------------------------------------------------------------------------- */
FALLOCchunk *
FALLOCnew_chunk(void)
{
  register FALLOCchunk *chunk;

  /* These are tiny, so we can just fatal error if they fail */

  MALLOC_FATAL(chunk,              FALLOCchunk, 1, "new chunk");
  MALLOC_FATAL(chunk->blocks,      char *, 1, "first blockptr in new chunk");
  MALLOC_FATAL(chunk->over_blocks, char *, 1, "first blockptr in new chunk");

  chunk->magic           =  MAGIC;
  chunk->free_bytes      =  0;
  chunk->cur_block       = -1;
  chunk->num_blocks      =  0;
  chunk->num_over_blocks =  0;

  return chunk;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Allocs from the given chunk.
 * ------------------------------------------------------------------------- */
char *
FALLOCalloc(
   register FALLOCchunk *chunk,
   register int 	 nbytes,	
   int 			 zero)
{
  register char *ret;
  register int   cb = chunk->cur_block;
  register int   nb = chunk->num_blocks;
  register int   ob = chunk->num_over_blocks;

  assert (chunk->magic == MAGIC);
  
  nbytes = (nbytes + ALIGN_SIZE) & ALIGN_MASK;

  if (nbytes <= chunk->free_bytes) {
    ret                = chunk->cur_ptr;
    chunk->cur_ptr    += nbytes;
    chunk->free_bytes -= nbytes;
  }

  else if (nbytes > FALLOC_BLOCK_SIZE) {
    REALLOC_RET(chunk->over_blocks, char *, ob + 1, (char *) NULL);
    MALLOC_RET(chunk->over_blocks[ob], char, nbytes, (char *) NULL);
    ret = chunk->over_blocks[ob];
    chunk->num_over_blocks++;
  }

  else {
    if (++chunk->cur_block >= chunk->num_blocks) {
      REALLOC_RET(chunk->blocks, char *, nb + 1, (char *) NULL);
      MALLOC_RET(chunk->blocks[nb], char, FALLOC_BLOCK_SIZE, (char *) NULL);
      chunk->num_blocks++;
    }

    chunk->free_bytes = FALLOC_BLOCK_SIZE - nbytes;
    ret               = chunk->blocks[cb + 1];
    chunk->cur_ptr    = ret + nbytes;
  }

#ifdef THINK_C
  if (zero) memset(ret, 0, nbytes);
#else
  if (zero) bzero(ret, nbytes);
#endif
  return ret;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Frees a falloc chunk.
 * ------------------------------------------------------------------------- */
void
FALLOCfree(
   register FALLOCchunk *chunk)
{
  register int i;

  assert (chunk->magic == MAGIC);

  chunk->magic = ~MAGIC;

  for (i = 0; i < chunk->num_blocks; i++) FREE(chunk->blocks[i]);
  for (i = 0; i < chunk->num_over_blocks; i++) FREE(chunk->over_blocks[i]);

  FREE(chunk->blocks);
  FREE(chunk->over_blocks);
  FREE(chunk);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Clears but does not free a chunk.  It becomes reusable.	
 * ------------------------------------------------------------------------- */
void 
FALLOCclear_chunk(
   register FALLOCchunk *chunk)
{
  register int i;

  assert (chunk->magic == MAGIC);

  chunk->cur_block  = -1;
  chunk->free_bytes = 0;

  for (i = 0; i < chunk->num_over_blocks; i++) FREE(chunk->over_blocks[i]);
  chunk->num_over_blocks = 0;

  /* No need to free chunk->over_blocks, next realloc will take care of it */
}


