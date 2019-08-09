/* Copyright 1991, Brown Computer Graphics Group.  All Rights Reserved. */

#ifndef lint
static char Version[] = 
   "$Id: MAT3factor.c,v 1.2 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * Routines to do the MAT3factor operation, which
 * factors a matrix m:
 *     m = r s r^ u t, where r^ means transpose of r, and r and u are
 *  rotations, s is a scale, and t is a translation.
 * 
 *  It is based on the Jacobi method for diagonalizing real symmetric
 *  matrices, taken from Linear Algebra, Wilkenson and Reinsch, Springer-Verlag
 *  math series, Volume II, 1971, page 204.  Call number QA251W623.
 *  In ALGOL!
 * ------------------------------------------------------------------------- */

#include "mat3defs.h"

/* ------------------------ Static Routines  ------------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Perform the jacobi factorization on an n x n symmetric matrix A
 * producing eigenvectors of the matrix as well, if so requested by
 * eivec flag. The eigenvalues are returned as the entries of the array d,
 * and the eigenvectors are returned as the columns of the matrix v.
 * The value rot indicates the number of jacobi rotations used in doing
 * the computation. The returned factorization is passed through the 
 * superdiagonal entries of the matrix A.
 * 
 * RETURNS :	void
 *
 * DETAILS :	
 * ------------------------------------------------------------------------- */

/*
 * Variable declarations from the original source:
 *
 * n	: order of matrix A
 * eivec: true if eigenvectors are desired, false otherwise.
 * a	: Array [1:n, 1:n] of numbers, assumed symmetric!
 *
 * a	: Superdiagonal elements of the original array a are destroyed.
 *	  Diagonal and subdiagonal elements are untouched.
 * d	: Array [1:n] of eigenvalues of a.
 * v	: Array [1:n, 1:n] containing (if eivec = TRUE), the eigenvectors of
 *	  a, with the kth column being the normalized eigenvector with
 *	  eigenvalue d[k].
 * rot	: The number of jacobi rotations required to perform the operation.
 */

static void
MAT3_jacobi(
   MAT3mat a,			/* Symmetric array */		
   int	   n,			/* Number of rows of a */	
   int	   eivec_flag,		/* Whether to compute eigenvectors */	
   MAT3mat d,			/* Array of eigenvalues of a */
   MAT3mat v,			/* Array of eigenvectors of a */		
   int	  *rot 			/* Number of jacobi rotations used */	
   )
{
   double sm, c, s,		/* Smallest entry, cosine, sine of theta */
	  t,  h, g,		/* tan of theta, two scrap values	 */
	  tau, theta, thresh,	/* sine/(1+cos), angle for jacobi rotn,
				 * threshold for doing rotation at all	 */
	  *b, *z;		/* two arrays of n doubles		 */
   int     p, q, i, j;

   ALLOCN(b, double, n, "Array 'b' in MAT3_jacobi");
   ALLOCN(z, double, n, "Array 'z' in MAT3_jacobi");

   MAT3identity(d);
   if (eivec_flag == TRUE) MAT3identity(v);

   for (p = 0; p < n; p++) {
      b[p] = d[p][p] = a[p][p];
      z[p] = 0.0;
   }
   *rot = 0;

   for (i = 0; i < 50; i++) { /* why 50? I don't know--it's the way the
				 * folks who wrote the algorithm did it */
      sm = 0.0;
      for (p = 0; p < n-1; p++) for (q = p+1; q < n; q++) sm += ABS(a[p][q]);

      if (sm == 0.0) goto out;

      thresh = (i < 3 ? (.2 * sm / (n * n)) : 0.0);

      for (p = 0; p < n-1; p++) for (q = p+1; q < n; q++) {

	 g = 100.0 * ABS(a[p][q]);

	 if (i > 3 && (ABS(d[p][p]) + g == ABS(d[p][p])) &&
		      (ABS(d[q][q]) + g == ABS(d[q][q])))
	    a[p][q] = 0.0;

	 else if (ABS(a[p][q]) > thresh) {
	    h = d[q][q] - d[p][p];

	    if (ABS(h) + g == ABS(h)) t = a[p][q] / h;
	    else {
	       theta = .5 * h / a[p][q];
	       t = 1.0 / (ABS(theta) + sqrt(1 + theta * theta));
	       if (theta < 0.0)  t = -t;
	    } /* end of computing tangent of rotation angle */

	    c = 1.0 / sqrt(1.0 + t*t);
	    s = t * c;
	    tau = s / (1.0 + c);
	    h = t * a[p][q];
	    z[p]    -= h;
	    z[q]    += h;
	    d[p][p] -= h;
	    d[q][q] += h;
	    a[p][q] = 0.0;

	    for (j = 0; j < p; j++) {
	       g = a[j][p];
	       h = a[j][q];
	       a[j][p] = g - s * (h + g * tau);
	       a[j][q] = h + s * (g - h * tau);
	    }

	    for (j = p+1; j < q; j++) {
	       g = a[p][j];
	       h = a[j][q];
	       a[p][j] = g - s * (h + g * tau);
	       a[j][q] = h + s * (g - h * tau);
	    }

	    for (j = q+1; j < n; j++) {
	       g = a[p][j];
	       h = a[q][j];
	       a[p][j] = g - s * (h + g * tau);
	       a[q][j] = h + s * (g - h * tau);
	    }

	    if (eivec_flag) for (j = 0; j < n; j++) {
	       g = v[j][p];
	       h = v[j][q];
	       v[j][p] = g - s * (h + g * tau);
	       v[j][q] = h + s * (g - h * tau);
	    }
	 }
	 (*rot)++;
      }
      for (p = 0; p < n; p++){
	 d[p][p] = b[p] += z[p];
	 z[p] = 0;
      }
   }
out:
   
   FREE(b);
   FREE(z);
}

/* ------------------------ Private Routines ------------------------------- */


/* ------------------------ Public Routines  ------------------------------- */
/*LINTLIBRARY*/
/* Copyright 1988, Brown Computer Graphics Group.  All Rights Reserved. */
/* -------------------------------------------------------------------------
 * DESCR   :	Factors a matrix @m@ into a product  @m@m = @r@ @s@
 * 		@r@^@u@@t@, where @r@^ means transpose of @r@, and @r@ and
 * 		@u@ are rotations, @s@ is a scale, and @t@ is a translation.
 *
 * RETURNS :	FALSE if matrix @m@ does not have fourth column = (0,0,0,1) or
 * 		if matrix @m@ has determinant zero, TRUE otherwise. If return
 * 		value is FALSE, values of @r@, @s@, @t@, and @u@ are
 * 		meaningless. 
 *
 * DETAILS :	The factorization requires that m[0][3] = m[1][3] = m[2][3] = 0
 * 		and m[3][3] = 1.
 * ------------------------------------------------------------------------- */
BOOL
MAT3factor(
   MAT3mat m,			/* Source matrix to be factored  */
   MAT3mat r,			/* Rotation for conjugation  */
   MAT3mat s,			/* Scale in rotated coord system  */ 
   MAT3mat u,			/* Rotation of conjugated matrix  */
   MAT3mat t			/* Translation component  */
   )
{
   double	det,		/* Determinant of matrix A	*/
		det_sign;	/* -1 if det < 0, 1 if det > 0	*/
   register int i;
   int		junk;
   MAT3mat	a, b, d, at, rt;

   /* (0) Make sure last row of m is as required. */
   det = 1.0 - m[3][3];
   if (! (MAT3_IS_ZERO_VEC(m[3]) && MAT3_IS_ZERO(det)))	return(FALSE);

   MAT3identity(r);	/* Adjust the values to something reasonable */
   MAT3identity(s);
   MAT3identity(u);
   MAT3identity(t);

   /* (1) T gets last column of M. */
   for (i = 0; i < 3; i++) t[i][3] = m[i][3];

   /* (2) A = M - T. */
   MAT3copy(a, m);
   for (i = 0; i < 3; i++) a[i][3] -= t[i][3];

   /* (3) Compute det A. If negative, set sign = -1, else sign = 1 */
   det = MAT3determinant(a);
   if (MAT3_IS_ZERO(det)) return(FALSE);
   det_sign = (det < 0.0 ? -1.0 : 1.0);

   /* (4) B = A * A^  (here A^ means A transpose) */
   MAT3transpose(at, a);
   MAT3mult(b, a, at);

   MAT3_jacobi(b, 3, TRUE, d, r, &junk);

   MAT3transpose(rt, r);

   /* Compute s = sqrt(d), with sign. Set d = s-inverse */
   MAT3copy(s, d);
   for (i = 0; i < 3; i++)
      d[i][i] = 1.0 / (s[i][i] = det_sign * sqrt(s[i][i]));

   /* (5) Compute U = R^ S! R A. */
   MAT3mult(u, d, r);
   MAT3mult(u, rt, u);
   MAT3mult(u, a, u);

   return(TRUE);
}


/* -------------------------------------------------------------------------
 * DESCR   :	Find the eigenvalues of the 3 x 3 part of a MAT3mat whose
 * 		determinant is nonzero and which is diagonalizable by the
 * 		Jacobi method.
 *
 * RETURNS :	TRUE if eigenvalues could be computed, FALSE otherwise.
 *
 * DETAILS :	Eigenvalues are returned in the array @values@. The sort is
 * 		numerical, from most negative to most positive.
 * ------------------------------------------------------------------------- */
BOOL
MAT3eigen_values(
   MAT3vec values, /* The eigenvalues of the second argument	*/
   MAT3mat mat	   /* The matrix whose eigenvalues are computed */
   )
{
   MAT3mat diag;   /* diagonalized matrix			*/
   MAT3mat e_vecs; /* the eigen vectors - not computed		*/
   int     rots;   /* number of rots needed to diagonalize	*/
   double  tmp;    /* temp swap var  && determinate		*/

   /*
    * check for imposible conditions
    */
   tmp = MAT3determinant(mat);
   if( MAT3_IS_ZERO(tmp) ) 
      return( FALSE );

   /*
    * Use the Jacobi algorithm (from above) to diagonalize the 
    * matrix. The eigen values are then the diagonal
    */
   MAT3_jacobi( mat, 3, FALSE, diag, e_vecs, &rots );

   /*
    * copy the diag elements into the result vector
    */
   MAT3_SET_VEC( values, diag[0][0], diag[1][1], diag[2][2] );
   
   /*
    * sort the values
    */
   if( values[0] > values[1] ) {
      tmp = values[0];
      values[0] = values[1];
      values[1] = tmp;
   }

   if( values[0] > values[2] ) {
      /* rotate entries */
      tmp = values[2];
      values[2] = values[1];
      values[1] = values[0];
      values[0] = tmp;
   } else if ( values[1] > values[2] ) {
      /* swap last two */
      tmp = values[1];
      values[1] = values[2];
      values[2] = tmp;      
   }
   return( TRUE );
}
