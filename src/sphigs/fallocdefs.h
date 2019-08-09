/* $Id: fallocdefs.h,v 1.4 1993/03/09 02:00:54 crb Exp $ */

/* -------------------------------------------------------------------------
		        Internal FALLOC include file
   ------------------------------------------------------------------------- */

#ifndef FALLOCDEFS_H_ALREADY_INCLUDED
#define FALLOCDEFS_H_ALREADY_INCLUDED

/* -----------------------------  Constants  ------------------------------ */

/* Define an alignment based on a type or an absolute size for any new machine
** or it will default to sizeof(double), at some small cost in space.
*/

#ifdef vax
#define ALIGN_WORST 2
#endif

#ifdef apollo
#define ALIGN_WORST 2
#endif

#ifndef ALIGN_WORST
#define ALIGN_WORST (sizeof(double))
#endif

#define ALIGN_SIZE (ALIGN_WORST - 1)
#define ALIGN_MASK (~ALIGN_SIZE)

/* ------------------------------  Macros  -------------------------------- */

#define MALLOC_FATAL(P,T,N,M)      ALLOC_RECORDS (P,T,N)
#define MALLOC_RET(P,T,N,R)        { if ((ALLOC(P,T,N)) == NULL) return R; }
#define REALLOC_RET(P,T,N,R)       { if ((REALLOC(P,P,T,N)) == NULL) return R; }

#define SWAP(A, B, T) ((T) = (A), (A) = (B), (B) = (T));
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define MAGIC ((int) 0xbad1)

#endif /* FALLOCDEFS_H_ALREADY_INCLUDED */
