/* Copyright 1991, Brown Computer Graphics Group.  All Rights Reserved. */

#ifndef lint
static char Version[] = 
   "$Id: MAT3vec.c,v 1.2 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 *
 *  This file contains routines that operate on matrices and vectors, or
 *  vectors and vectors.
 *  
 * ------------------------------------------------------------------------- */

#include "mat3defs.h"
#include <assert.h>

/* ------------------------ Static Routines  ------------------------------- */


/* ------------------------ Private Routines ------------------------------- */


/* ------------------------ Public Routines  ------------------------------- */
/*LINTLIBRARY*/

/* -------------------------------------------------------------------------
 * DESCR :	Multiplies the vector @vec@ by the matrix @mat@, and places
 * 		the result in @result_vec@. The routine assumes all
 * 		homogeneous coordinates are 1. The two vectors involved may
 * 		be the same.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3mult_vec(
   register MAT3vec result_vec,	/* The product of vec and mat (output) */
   register MAT3vec vec,       	/* The vector to be multiplied (input) */
   register MAT3mat mat 	/* The matrix it's multiplied by (input)*/
   )
{
   MAT3vec		tempvec;
   register double	*temp = tempvec;

   temp[0] =	vec[0] * mat[0][0] + vec[1] * mat[0][1] +
		vec[2] * mat[0][2] +	      mat[0][3];
   temp[1] =	vec[0] * mat[1][0] + vec[1] * mat[1][1] +
		vec[2] * mat[1][2] +	      mat[1][3];
   temp[2] =	vec[0] * mat[2][0] + vec[1] * mat[2][1] +
		vec[2] * mat[2][2] +	      mat[2][3];

   MAT3_COPY_VEC(result_vec, temp);
}

/* -------------------------------------------------------------------------
 * DESCR :	Multiplies the vector of size 4 (@vec@) by the matrix @mat@,
 * 		placing the result in @result_vec@. (The multiplication is of
 * 		the form matrix times vector, and the vector should be
 * 		considered to be a column vector). The fourth element of the
 * 		vector is the homogeneous coordinate, which may or may not be
 * 		1.  If the @homogenize@ parameter is TRUE, then the result
 * 		vector will be normalized so that the homogeneous coordinate
 * 		is 1. The two vectors involved may be the same. This returns
 * 		zero if the vector was to be homogenized, but couldn't be.
 *
 *
 * RETURNS :	FALSE if the result vector was to be homogenized, but had
 * 		homogeneous coordinate zero. In this case, the results should
 * 		be regarded as garbage. Otherwise the routine returns TRUE.
 * ------------------------------------------------------------------------- */
int
MAT3mult_hvec(
   MAT3hvec          result_vec, /* The product of vec with mat (output) */
   register MAT3hvec vec,	 /* The vector to multiply (input) */
   register MAT3mat  mat,	 /* The matrix to multiply (input) */
   int               homogenize	 /* Should we homogenize the result?  */
   )
{
   MAT3hvec             tempvec;
   double		norm_fac;
   register double	*temp = tempvec;
   register int 	ret = TRUE;

   temp[0] =	vec[0] * mat[0][0] + vec[1] * mat[0][1] +
		vec[2] * mat[0][2] + vec[3] * mat[0][3];
   temp[1] =	vec[0] * mat[1][0] + vec[1] * mat[1][1] +
		vec[2] * mat[1][2] + vec[3] * mat[1][3];
   temp[2] =	vec[0] * mat[2][0] + vec[1] * mat[2][1] +
		vec[2] * mat[2][2] + vec[3] * mat[2][3];
   temp[3] =	vec[0] * mat[3][0] + vec[1] * mat[3][1] +
		vec[2] * mat[3][2] + vec[3] * mat[3][3];

   /* Homogenize if asked for, possible, and necessary */
   if (homogenize) {
      if (MAT3_IS_ZERO(temp[3])) {
	 SEVERES("Can't homogenize vec. in MAT3mult_hvec: last coord is 0");
	 ret = FALSE;
      }
      else {
	 norm_fac = 1.0 / temp[3];
	 MAT3_SCALE_VEC(result_vec, temp, norm_fac);
	 result_vec[3] = 1.0;
      }
   }
   else MAT3_COPY_HVEC(result_vec, temp);

   return(ret);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Multiply the matrix @mat@ by the vector @vec@ and normalize
 * 		the result. The vector @vec@ has no homogeneous component,
 * 		and thus the homogeneous component is assumed to be 1. The
 * 		resulting vector @result_vec@ is of the same form.
 *
 * RETURNS :	TRUE unless the resulting vector would have had a homogeneous
 * 		coordinate of zero, in which case FALSE is returned and the
 * 		values in @result_vec@ are meaningless.
 * ------------------------------------------------------------------------- */
int
MAT3mult_vec_affine(
   MAT3vec           result_vec,	/* The computed product (output) */
   register MAT3vec  vec,		/* The vector factor (input) */
   register MAT3mat  mat 		/* The matrix factor (input) */
   )
{
   MAT3hvec             tempvec;
   double		norm_fac;
   register double	*temp = tempvec;
   register int 	ret = TRUE;

   temp[0] =	vec[0] * mat[0][0] + vec[1] * mat[0][1] +
		vec[2] * mat[0][2] +          mat[0][3];
   temp[1] =	vec[0] * mat[1][0] + vec[1] * mat[1][1] +
		vec[2] * mat[1][2] +          mat[1][3];
   temp[2] =	vec[0] * mat[2][0] + vec[1] * mat[2][1] +
		vec[2] * mat[2][2] +          mat[2][3];
   temp[3] =	vec[0] * mat[3][0] + vec[1] * mat[3][1] +
		vec[2] * mat[3][2] +          mat[3][3];

   /* Homogenize if possible */
   if (MAT3_IS_ZERO(temp[3])) {
	 SEVERES("Can't homogenize vec in MAT3mult_vec_affine: last coord is 0");
	 ret = FALSE;
   }
   else {
      norm_fac = 1.0 / temp[3];
      MAT3_SCALE_VEC(result_vec, temp, norm_fac);
   }

   return(ret);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Set @result_vec@ to be the cross product of @vec1@ and
 * 		@vec2@, in that order.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3cross_product(
   MAT3vec 	 result_vec,	/* The computed cross-product (output) */
   register MAT3vec vec1,	/* The first factor (input) */
   register MAT3vec vec2 	/* The second factor (input) */
   )
{
   MAT3vec		tempvec;
   register double	*temp = tempvec;

   temp[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
   temp[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
   temp[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];

   MAT3_COPY_VEC(result_vec, temp);
}

/* -------------------------------------------------------------------------
 * DESCR :	Computes a unit vector perpendicular to @vec@, and stores it
 * 		in @result_vec@. If @is_unit@ is TRUE, @vec@ is assumed to be
 * 		of unit length (possibly allowing a time savings by avoiding
 * 		normalization of @result_vec@).
 *
 * RETURNS :	void
 *
 * DETAILS :	The method used takes advantage of this technique: As it is
 * 		easier to compute a perpendicular vector in R2 than R3, we
 * 		essentially do that.  We first determine the component of
 * 		@vec@ with the mimimum absolute value, setting corresponding
 * 		component of @result_vec@ to 0. We next set the other two
 * 		components of @result_vec@ by swapping the remaining
 * 		(non-mimimum) components of @vec@ and negating one of these.
 * 		Then we normalize as necessary.
 *
 * ------------------------------------------------------------------------- */
#define MAT3ABS(V) ((V) >= 0 ? (V) : (V) * -1)
void
MAT3perp_vec(
   MAT3vec result_vec,		/* The vecotr perp to vec (output) */
   MAT3vec vec,			/* A vector to which a perp is needed (input) */
   int is_unit 			/* TRUE is input vector is a unit vector */
   )
{
   double abs_vec[3];
   int min;
   int other1;
   int other2;
   double dot;

   /* Compute the absolute value of the components of the input vector. */

   abs_vec[0] = MAT3ABS(vec[0]);
   abs_vec[1] = MAT3ABS(vec[1]);
   abs_vec[2] = MAT3ABS(vec[2]);

   /* Determine the component with the minimum absolute value, saving its
      index in "min".  Save the indexes of the other two components in
      "other1" and "other2". */

   if (abs_vec[0] <= abs_vec[1])
      if (abs_vec[0] <= abs_vec[2]) {
	 min = 0; other1 = 1; other2 = 2;
      }
      else {
	 min = 2; other1 = 0; other2 = 1;
      }
   else
      if (abs_vec[1] <= abs_vec[2]) {
	 min = 1; other1 = 0; other2 = 2;
      }
      else {
	 min = 2; other1 = 0; other2 = 1;
      }

   /* Generate the perpendicular vector by setting the "min" compenent to
      0, and swapping the "other1" and "other2' components from "vec",
      negating one of these. */

   result_vec[min] = 0.0;
   result_vec[other1] = vec[other2];
   result_vec[other2] = -vec[other1];

   /* If the input vector has unit length and the component with the mimimum
      absolute value is 0, the computed perpendicular vector already has
      unit length.  Otherwise, we must normalize it. */

   if (!is_unit ||
       vec[min] != 0.0)
      MAT3_NORMALIZE_VEC(result_vec, dot);
}



