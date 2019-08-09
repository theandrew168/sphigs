/* Copyright 1991, Brown Computer Graphics Group.  All Rights Reserved. */

#ifndef lint
static char Version[] = 
   "$Id: MAT3geom.c,v 1.2 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 *
 * This file contains routines that perform geometry-related operations
 * on matrices.
 *
 * ------------------------------------------------------------------------- */

#include "mat3defs.h"

/* ------------------------ Static Routines  ------------------------------- */

/* ------------------------ Private Routines ------------------------------- */

/* ------------------------ Public Routines  ------------------------------- */
/*LINTLIBRARY*/

/* -------------------------------------------------------------------------
 * DESCR   :	Take a matrix that transforms points and return a matrix
 * 		appropriate for transforming directions. This is just
 * 		the upper 3 x 3 block of the source matrix.
 * ------------------------------------------------------------------------- */
void
MAT3direction_matrix(
   register MAT3mat result_mat,	/* Matrix for transforming directions */
   register MAT3mat mat 		/* Source matrix */
   )
{
   MAT3copy(result_mat, mat);

   result_mat[0][3] = result_mat[1][3] = result_mat[2][3] = 0.0;
   result_mat[3][3] = 1.0;
}


/* -------------------------------------------------------------------------
 * DESCR :	This takes a matrix used to transform points, and returns a
 * 		corresponding matrix that can be used to transform vectors
 * 		that must remain perpendicular to planes defined by the
 * 		points.  It is useful when you are transforming some object
 * 		that has both points and normals in its definition, and you
 * 		only have the transformation matrix for the points. This
 * 		routine returns FALSE if the normal matrix is uncomputable.
 * 		Otherwise, it returns TRUE. The computed matrix is the
 * 		adjoint for the non-homogeneous part of the transformation.
 *
 * RETURNS : 	FALSE if the normal matrix is uncomputable, true otherwise.
 *
 * DETAILS :	The normal matrix is uncomputable if the determinant of the
 * 		source matrix is too small in absolute value (i.e.,
 * 		approximately zero). 
 * ------------------------------------------------------------------------- */
int
MAT3normal_matrix(
   register MAT3mat result_mat,	/* The compute normal matrix */
   register MAT3mat mat 	/* The matrix from which it is computed */
   )
{
   register int ret;
   MAT3mat	tmp_mat;

   MAT3direction_matrix(tmp_mat, mat);

   if (ret = MAT3invert(tmp_mat, tmp_mat)) MAT3transpose(result_mat, tmp_mat);

   return(ret);
}


/* -------------------------------------------------------------------------
 * DESCR   :	Takes a 3-vector of scale values, and creates a matrix with
 * 		these as its diagonal entries and zeroes off the diagonal,
 * 		i.e., a scale matrix with scale factors given by the entries
 * 		of the given vector.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3scale(
   MAT3mat result_mat,		/* A diagonal matrix (a scaling matrix) */
   MAT3vec scale 		/* The entries for the diagonal */
   )
{
   MAT3identity(result_mat);

   result_mat[0][0] = scale[0];
   result_mat[1][1] = scale[1];
   result_mat[2][2] = scale[2];
}


/* -------------------------------------------------------------------------
 * DESCR :	Sets up a matrix for a rotation about an axis given by the
 * 		line from (0,0,0) to @axis@, through an angle given by
 * 		@angle_in_radians@.  Looking along the axis toward the
 * 		origin, the rotation is counter-clockwise. The matrix is
 * 		returned in @result_mat@.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3rotate(
   MAT3mat result_mat,		/* Rotation matrix produced by the routine */
   MAT3vec axis,		/* The axis of the rotation matrix */
   double	angle_in_radians/* The angle of rotation */
   )
{
   MAT3vec	base1,  /* 1st unit basis vec, normalized axis          */
                base2,	/* 2nd unit basis vec, perp to axis		*/
	        base3;	/* 3rd unit basis vec, perp to axis & base2	*/
   MAT3mat	base_mat,	/* Change-of-basis matrix		*/
		base_mat_trans; /* Inverse of c-o-b matrix		*/
   double       dot;
   register int i;

   /* Step 1: extend { axis } to a basis for 3-space: { axis, base2, base3 }
    * which is orthonormal (all three have unit length, and all three are
    * mutually orthogonal). Also should be oriented, i.e. axis cross base2 =
    * base3, rather than -base3.
    *
    * Method: Find a vector linearly independent from axis. For this we
    * either use the y-axis, or, if that is too close to axis, the
    * z-axis. 'Too close' means that the dot product is too near to 1.
    */

   MAT3_COPY_VEC(base1, axis);
   MAT3_NORMALIZE_VEC(base1, dot);
   /* If the axis was too short to normalize (i.e., it is almost
    * a zero length vector), MAT3_NORMALIZE_VEC will set dot to 0.
    * In this case, just return the identity matrix (no rotation).
    */
   if (dot == 0.0) {
      MAT3identity(result_mat);
      return;
   }
   MAT3perp_vec     (base2, base1, TRUE);
   MAT3cross_product(base3, base1, base2);

   /* Set up the change-of-basis matrix, and its inverse */
   MAT3identity(base_mat);
   MAT3identity(base_mat_trans);
   MAT3identity(result_mat);

   for (i = 0; i < 3; i++){
      base_mat[i][0] = base_mat_trans[0][i] = base1[i];
      base_mat[i][1] = base_mat_trans[1][i] = base2[i];
      base_mat[i][2] = base_mat_trans[2][i] = base3[i];
   }

   /* If T(u) = uR, where R is base_mat, then T(x-axis) = axis,
    * T(y-axis) = base2, and T(z-axis) = base3. The inverse of base_mat is
    * its transpose.  OK?
    */

   result_mat[1][1] =	result_mat[2][2] = cos(angle_in_radians);
   result_mat[1][2] = -(result_mat[2][1] = sin(angle_in_radians));

   MAT3mult(result_mat, result_mat, base_mat_trans);
   MAT3mult(result_mat, base_mat, result_mat);
}


/* -------------------------------------------------------------------------
 * DESCR :	Sets the matrix @result_mat@ to be a translation matrix that
 * 		will translate by the vector @trans@.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3translate(
   MAT3mat result_mat,	/* Translation matrix [output] */
   MAT3vec trans	/* Translation vector [input]  */
   )
{
   MAT3identity(result_mat);

   result_mat[0][3] = trans[0];
   result_mat[1][3] = trans[1];
   result_mat[2][3] = trans[2];
}

/* -------------------------------------------------------------------------
 * DESCR :	Given the unit-length normal to a plane (@normal@) through
 * 		the origin, this sets @result_mat@ to one that reflects
 * 		points through that plane.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3mirror(
   MAT3mat result_mat,		/* A matrix that reflects through given plane */
   MAT3vec normal 		/* Plane normal to plane through origin  */
   )
{
   MAT3vec	basis2, basis3;
   MAT3mat	ab, c;
   register int i;

   /* Find an orthonormal basis with the normal as one vector */
   MAT3perp_vec(basis2, normal, TRUE);
   MAT3cross_product(basis3, normal, basis2);

   /* Mirroring is A (change of basis) x B (mirror) x C (change basis back).
    * The A matrix uses the basis vectors as rows, and C uses them as columns.
    * B is the identity matrix with the first entry a -1 instead of a 1.
    * A and B are implicitly multiplied here by negating the first row of A.
    */
   MAT3identity(ab);
   for (i = 0; i < 3; i++) {
      ab[0][i] = -normal[i];	/* Negate first row */
      ab[1][i] =  basis2[i];
      ab[2][i] =  basis3[i];
   }

   /* Create the C matrix with the basis as column vectors */
   MAT3identity(c);
   for (i = 0; i < 3; i++) {
      c[i][0] = normal[i];
      c[i][1] = basis2[i];
      c[i][2] = basis3[i];
   }

   /* Multiply them */
   MAT3mult(result_mat, c, ab);
}

/* -------------------------------------------------------------------------
 * DESCR :		Sets @result_mat@ to be a shear matrix that does the
 * 		following:Given a plane normal (@normal@) and a shear vector
 * 		(@shear@), the transformation leaves points in the plane
 * 		(through the origin, perp to the normal) fixed, while moving
 * 		points at a distance d from the plane by d times the shear
 * 		vector.  (The shear vector is first made to be perpendicular
 * 		to the normal.) The normal vector is assumed to be unit
 * 		length, and results are incorrect if it is not.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3shear(
   register MAT3mat result_mat,	/* A shear matrix generated by this routine  */
   register MAT3vec normal,	/* The normal to the shear plane */
   register MAT3vec shear 	/* The shearing vector [perp to normal] */
   )
{
   register int 	j;
   register double	*shear_used = shear;
   double		dot;
   MAT3vec		shear2;

   MAT3identity(result_mat);

   /* Make sure shear vector is perpendicular to the normal */
   dot = MAT3_DOT_PRODUCT(normal, shear);

   if (! MAT3_IS_ZERO(dot)) {
      shear_used = shear2;
      MAT3_SCALE_VEC(shear_used, normal, dot);
      MAT3_SUB_VEC(shear_used, shear, shear_used);
   }

   /* Set columns to coordinate axes transformed by shear */
   for (j = 0; j < 3; j++) {
      result_mat[j][0] += normal[0] * shear_used[j];
      result_mat[j][1] += normal[1] * shear_used[j];
      result_mat[j][2] += normal[2] * shear_used[j];
   }
}

/* -------------------------------------------------------------------------
 * DESCR :	Given @from@, @at@, and @up@ points, this routine computes a
 * 		matrix (@result_mat@) that implements an orientation
 * 		transformation.  The transformation brings the coordinate
 * 		axes to a new position. The origin is moved to @from@, the
 * 		negative Z-axis is transformed so that it points in the
 * 		direction from @from@ to @at@, and the positive Y-axis is
 * 		transformed so that it points in the direction P(@up-from@),
 * 		where P is a function that takes a vector and makles it
 * 		perpendicular to the @from-at@ vector.
 *
 * RETURNS :	TRUE if the three given points are non-colinear, FALSE
 * 		otherwise, in which case the returned matrix contains garbage.
 *
 * DETAILS :	To be more precise, one could say that the translation
 * 		component of the returned matrix is just translation from the
 * 		origin to the @from@ point, and that the rest of the matrix
 * 		is computed by considering the vectors @at@ - @from@ and @up@
 * 		- @from@. These two are run through the gram-schmidt process,
 * 		rendering them orthonormal, and then extended to an oriented
 * 		basis of 3-space. The standard basis is transformed to this
 * 		basis b1, b2, b3, by sending the negative Z axis to b1, the
 * 		positive Y axis to b2, and the positive X axis to b3.
 * ------------------------------------------------------------------------- */
int
MAT3orient(
   MAT3mat result_mat,	/* Orientation matrix produced by routine */
   MAT3vec from,	/* Point to which origin will be translated */
   MAT3vec at,		/* Z-axis is transformed to from-at */
   MAT3vec up 		/* Y-axis is transformed to from-up */
   )
{
   MAT3vec	basis1, basis2, basis3;
   double	t;
   int		i;

   /* Find a unit vector between 'from' and 'at' */
   MAT3_SUB_VEC(basis3, from, at);
   MAT3_NORMALIZE_VEC(basis3, t);

   /* Extend to an orthonormal basis of 3-space */
   MAT3_SUB_VEC(basis2, up, from);

   t = MAT3_DOT_PRODUCT(basis3, basis2);
   MAT3_LINEAR_COMB(basis2, 1.0, basis2, -t, basis3);
   MAT3_NORMALIZE_VEC(basis2, t);

   if (t == 0.0) {
      SEVERES("Dependent from, at, and up points in MAT3orient");
      return(FALSE);
   }

   MAT3cross_product(basis1, basis2, basis3);

   /* Create the matrix with the basis as row vectors, and "from" as
    * translation */
   MAT3identity(result_mat);
   for(i = 0; i < 4; ++i) {
      result_mat[i][0] = basis1[i];
      result_mat[i][1] = basis2[i];
      result_mat[i][2] = basis3[i];
      result_mat[i][3] = from[i];
   }
   return(TRUE);
}

/* -------------------------------------------------------------------------
 * DESCR :	Given @from@, @to@, and @up@ points, this computes a matrix
 * 		implementing a span transformation; a span transformation
 * 		takes a bi-unit object (e.g., the cube extending from -1 to 1
 * 		in each axis) and stretches, rotates, and translates it so
 * 		that what was the Z-axis of the objects extends from @from@
 * 		to @to@, and the former Y-axis of the object is aligned with
 * 		the vector @up@ (which is made perpendicular to @from@ - @to@).
 * 	                If the flag @vol_preserve@ is TRUE, the
 * 		transformation also scales in the (original) Y- and X-
 * 		directions to keep the determinant of the upper 3x3 part of
 * 		the transformation being 1.  
 *
 * RETURNS :	TRUE if computation succeeded, FALSE if @from@, @to@, and
 * 		@up@ are colinear, in which case the computed matrix contains
 * 		garbage. 
 * ------------------------------------------------------------------------- */
int
MAT3span(
   MAT3mat   result_mat,	/* Computed span transformation matrix */
   MAT3vec   from,		/* Where the -1 point on Z-axis goes   */
   MAT3vec   at,		/* Where the +1 point in the Z-axis goes */
   MAT3vec   up,		/* Where the Y-axis goes */
   int       vol_preserve 	/* Should we preserve volume?  */
   )
{
   MAT3mat   scale_mat;
   MAT3vec   middle, scale_vec;
   double    length, x_scale;
 
   MAT3_SUB_VEC(scale_vec, at, from); 
   length = MAT3_LENGTH_VEC(scale_vec)/2.0;   /* am assuming object of        */
   if (vol_preserve) {                        /* bi-unit length to be spanned */
      x_scale = 1.0/sqrt(length);
      MAT3_SET_VEC(scale_vec, x_scale, x_scale, length);
   } else     MAT3_SET_VEC(scale_vec, 1.0, 1.0, length);
   MAT3scale(scale_mat, scale_vec);

   MAT3_ADD_VEC(middle, at, from);
   MAT3_SCALE_VEC(middle, middle, 0.5);
   if (!MAT3orient(result_mat, middle, at, up)) {
      SEVERES("MAT3orient failed");
      return FALSE;
   }
   MAT3mult(result_mat, result_mat, scale_mat);
   return(TRUE);
}

/* -------------------------------------------------------------------------
 * DESCR :	Given a rotation matrix @mat@, find the axis about which it
 * 		rotates and how many radians it rotates about that axis.  It
 * 		returns the axis vector in @axis@ and the angle in @angle@.
 *
 * RETURNS :	TRUE.
 *
 * DETAILS :	Method:	the axis of rotation (v) is an eigenvector of the
 * 		rotation matrix (M).  Its associated eigenvalue is 1. Mv =
 * 		(lambda)v ==> Mv = v ==> Mv = Iv ==> (M - I)v = 0.  So v is
 * 		in the kernel of M - I. Then grab a vector perpendicular to v
 * 		(arm), extend it to a basis (arm2). Apply M to arm and
 * 		project that onto arm and arm2.  These are the sin and cos of
 * 		the angle, so use atan2 to find the angle. This returns TRUE
 * 		if all goes well, FALSE otherwise. Note: Rodriguez's formula
 * 		could be used instead, and would be far faster.
 * ------------------------------------------------------------------------- */
int
MAT3axis_and_angle(
register MAT3mat mat,		/* A rotation matrix to be studied */
   MAT3vec 	 axis,		/* The axis of rotation */
   double	 *angle 	/* The amount of the rotation */
   )
{
   MAT3mat	minus_ident,		/* M - I		*/
		basis;			/* basis for kernel	*/
   MAT3vec	arm, arm2, rotated_arm, temp_vec;
   double	temp, epsilon;
   register int i;

   /* Subtract identity from rotation matrix (upper 3x3) */
   MAT3copy(minus_ident, mat);
   for (i = 0; i < 3; i++) {
      minus_ident[i][i] -= 1.0;
      minus_ident[3][i] = minus_ident[i][3] = 0.0;
   }
   minus_ident[3][3] = 1.0;

   /*
    * A very-near rotation matrix might be rejected due to epsilon error.
    * This occurs in the MAT3kernel_basis.
    * So, if finding the kernel fails, try a larger epsilon.
    * larger epsilon = old epsilon times EPSILON_JUMP
    */
#define EPSILON_JUMP	(1e3)

   for (epsilon = MAT3_EPSILON;
	! MAT3kernel_basis(basis, minus_ident, epsilon);
	epsilon *= EPSILON_JUMP) ;

   MAT3_COPY_VEC(axis, basis[0]);
   MAT3_NORMALIZE_VEC(axis, temp);
   MAT3perp_vec(arm, axis, TRUE);
   MAT3_CROSS_PRODUCT(arm2, axis, arm, temp_vec);
   MAT3mult_vec(rotated_arm, arm, mat);

   *angle = atan2(MAT3_DOT_PRODUCT(arm2, rotated_arm),
		  MAT3_DOT_PRODUCT(arm , rotated_arm));

   return(TRUE);
}

/* -------------------------------------------------------------------------
 * DESCR :	Compute the transformation matrix needed to align @vec1@ with
 * 		@vec2@, storing the result in @result_mat@.
 *
 * RETURNS :	void
 *
 * DETAILS :	The final tranformation matrix is computed as the product of
 * 		two intermediate transformation matrices.  The first of these
 * 		aligns @vec1@ with the X-axis.  To compute this matrix we
 * 		simply take the inverse of the matrix to align the X-axis
 * 		with @vec1@ (easily computed by extending @vec1@ to an
 * 		orthonormal basis in 3-space).  The second intermediate
 * 		matrix aligns the X-axis with @vec2@.  This is generated by
 * 		extending @vec2@ to an orthonormal basis in 3-space.
 * ------------------------------------------------------------------------- */
#define UNIT 1
void
MAT3align(
   MAT3mat result_mat,	/* Computed matrix aligning vec1 with vec2 */
   MAT3vec vec1,	/* Vector to be transformed */
   MAT3vec vec2 	/* We want to move vec1 so that it points same */
			/* dir'n as vec2 */
   )
{
   MAT3vec v, vp, vpp;
   MAT3vec w, wp, wpp;
   MAT3mat a_t, b;
   MAT3vec temp;
   double dot;

   /* Compute the three basis vectors for the transformation matrix to align
      the X-axis with "vec1".  Basis 1 ("v") is the normalized "vec1".  Basis
      2 ("vp") is a vector perpendicular to "v".  Basis 3 ("vpp") is the cross
      product of "v" and "vp". */

   MAT3_COPY_VEC(v, vec1);
   MAT3_NORMALIZE_VEC(v, dot);

   MAT3perp_vec(vp, v, UNIT);

   MAT3_CROSS_PRODUCT(vpp, v, vp, temp);

   /* Set up the transformation matrix to align "vec1" with the X-axis.  This
      is the inverse of the matrix that would be formed from the basis vectors
      just computed.  Since the basis vectors are orthonormal, the inverse is
      simply the transpose. */

   a_t[0][0] = v[0];  a_t[0][1] = vp[0];  a_t[0][2] = vpp[0];  a_t[0][3] = 0.0;
   a_t[1][0] = v[1];  a_t[1][1] = vp[1];  a_t[1][2] = vpp[1];  a_t[1][3] = 0.0;
   a_t[2][0] = v[2];  a_t[2][1] = vp[2];  a_t[2][2] = vpp[2];  a_t[2][3] = 0.0;
   a_t[3][0] = 0.0;   a_t[3][1] = 0.0;    a_t[3][2] = 0.0;     a_t[3][3] = 1.0;

   /* Compute the three basis vectors for the transformation matrix to align
      the X-axis with "vec2".  The process is the same as that used for the
      previous set of basis vectors. */

   MAT3_COPY_VEC(w, vec2);
   MAT3_NORMALIZE_VEC(w, dot);

   MAT3perp_vec(wp, w, UNIT);

   MAT3_CROSS_PRODUCT(wpp, w, wp, temp);

   /* Set up the transformation matrix to align the X-axis with "vec2".  This
      is done directly using the basis vectors just computed. */

   b[0][0] = w[0];   b[0][1] = w[1];   b[0][2] = w[2];   b[0][3] = 0.0;
   b[1][0] = wp[0];  b[1][1] = wp[1];  b[1][2] = wp[2];  b[1][3] = 0.0;
   b[2][0] = wpp[0]; b[2][1] = wpp[1]; b[2][2] = wpp[2]; b[2][3] = 0.0;
   b[3][0] = 0.0;    b[3][1] = 0.0;    b[3][2] = 0.0;    b[3][3] = 1.0;

   /* Compute the final transformation matrix (to align "vec1" with "vec2")
      as the product of the two transformation matrices just computed. */

   MAT3mult(result_mat, a_t, b);
}
