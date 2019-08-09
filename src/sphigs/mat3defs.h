/* Copyright 1991, Brown Computer Graphics Group.  All Rights Reserved. */

/* -------------------------------------------------------------------------
		       Private include file
   ------------------------------------------------------------------------- */

/* $Id: mat3defs.h,v 1.4 1993/03/09 02:00:54 crb Exp crb $ */

#include "mat3.h"
#include "macros.h"
#include "sph_errtypes.h"

/* --------------------------    Constants     ----------------------------- */

#define MAT3_EPS  1e-7

/* --------------------------      Types       ----------------------------- */
 

/* --------------------------      Macros      ----------------------------- */


#ifdef THINK_C
/* We hide this from gnu's compiler, which doesn't understand it. */
void SPH__error (...);
#endif
 
#define SEVERES(M)   						\
   if (1) { fprintf(stderr, M); } else
#define FATALS(M)   						\
   if (1) { SPH__error(ERR_MAT3_PACKAGE, M); } else

#define ALLOCN(P,T,N,M) 					\
   if (ALLOC (P,T,N) == NULL)  					\
      SEVERES(M); 						\
   else

#ifndef CNULL
#define CNULL NULL
#endif
#ifndef TRUE
#define TRUE  (0==0)
#endif
#ifndef FALSE
#define FALSE (0==1)
#endif

#define ABS(A)		((A) > 0   ? (A) : -(A))
#define MIN(A,B)	((A) < (B) ? (A) :  (B))
#define MAX(A,B)	((A) > (B) ? (A) :  (B))

#define SWAP(A,B,TEMP)    (TEMP=(A), (A)=(B), (B)=(TEMP))
#define IS_ZERO(N,EPS)	  ((N) < EPS && (N) > -EPS)

/* -------------------------- Private Routines ----------------------------- */

