#ifndef lint
static char Version[]=
   "$Id: sph_bitstr.c,v 1.2 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_bitstr.c
 *
 *	More bit string manipulations
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"
#include <string.h>

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Sets all bits in the bitstring to zero.
 * ------------------------------------------------------------------------- */
void
SPH__clearBitstring (
   substruct_bitstring *B)
{
   if (*B == NULL) 
      ALLOC_RECORDS (*B, unsigned char, BYTES_PER_BITSTRING);
   bzero (*B,BYTES_PER_BITSTRING);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Checks each byte in a bitstring to verify all bits are off. 
 *
 * DETAILS :	I wish I could have made this a macro, but it wasn't
 * 		possible.   
 * ------------------------------------------------------------------------- */
boolean
SPH__bitstringIsClear (
   substruct_bitstring B)
{
   register int i; 

   for (i=0; i<BYTES_PER_BITSTRING; i++)
      if (B[i])
	 return FALSE;
   return TRUE;
}
