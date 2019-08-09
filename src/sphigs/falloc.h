/* $Id: falloc.h,v 1.4 1993/03/09 02:00:54 crb Exp $ */

/* -------------------------------------------------------------------------
		        Public FALLOC include file
   ------------------------------------------------------------------------- */

#ifndef FALLOC_H_ALREADY_INCLUDED
#define FALLOC_H_ALREADY_INCLUDED

/* -----------------------------  Constants  ------------------------------ */

#define FALLOC__ZERO      1
#define FALLOC__DONT_ZERO 0
#define FALLOC_BLOCK_SIZE (4096-12)

/* -------------------------------  Types  -------------------------------- */

typedef struct FALLOCchunk FALLOCchunk;
struct FALLOCchunk {		/* Controlled Allocation Object: */
   int    free_bytes;		/* - bytes avail in current block */
   char  *cur_ptr;		/* - free addr within current block */
   int    cur_block;		/* - current block index */
   int    num_blocks;		/* - number of extant blocks */
   char **blocks;		/* - list of extant blocks */
   int    num_over_blocks;	/* - number of oversize blocks */
   char **over_blocks;          /* - list of oversize blocks */
   int    magic;		/* - magic number signature */
};

/* -----------------------------  Prototypes  ----------------------------- */

FALLOCchunk     *FALLOCnew_chunk (void);
char            *FALLOCalloc(FALLOCchunk*, int nbytes, int zero);
void             FALLOCfree (FALLOCchunk*);
void             FALLOCclear_chunk (FALLOCchunk*);

#endif /* FALLOC_H_ALREADY_INCLUDED */
