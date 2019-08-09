/* Copyright 1991, Brown Computer Graphics Group.  All Rights Reserved. */

/* -------------------------------------------------------------------------
		       Public MAT3 include file
   ------------------------------------------------------------------------- */

/* $Id: mat3.h,v 1.4 1993/03/09 02:00:54 crb Exp $ */

#ifndef MAT3_HAS_BEEN_INCLUDED
#define MAT3_HAS_BEEN_INCLUDED

/* -----------------------------  Constants  ------------------------------ */

/*
 * Make sure the math library .h file is included, in case it wasn't.
 */

#ifndef HUGE
#include <math.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef lint
#define MAT3_FIX  , errno = 1
#else
#define MAT3_FIX
#endif

#define MAT3_EPSILON	1e-12			/* Close enough to zero   */
#define MAT3_PI 	M_PI			/* Pi			  */
#define MAT3_NULL_MAT	MAT3_null_mat

/*
 * DESCR :	Determines how many values we store in our precomputed
 *       	sin and cos tables
 */
#define MAT3_SIN_RES    360

#define MAT3__MAT_REF(MMAT)  ((MMAT)->ref)
#define MAT3__VEC_REF(MVEC)  ((MVEC)->ref)
#define MAT3__MAT_MAT(MMAT)  ((MMAT)->mat)
#define MAT3__VEC_VEC(MVEC)  ((MVEC)->vec)

/* ------------------------------  Types  --------------------------------- */

#define BOOL unsigned char

typedef double MAT3mat[4][4];		/* 4x4 matrix			 */
typedef double MAT3vec[3];		/* Vector			 */
typedef double MAT3hvec[4];             /* Vector with homogeneous coord */

/* ------------------------------  Macros  -------------------------------- */

/*
 * TITLE   :	Comparison macros
 * RETURNS :	int
 */

/* 
 * DESCR   :	Tests if a number is within EPSILON of zero
 */
#define MAT3_IS_ZERO(N) 	((N) < MAT3_EPSILON && (N) > -MAT3_EPSILON)

/* 
 * DESCR   :	Tests a vector @V@ for all components being close to zero
 */
#define MAT3_IS_ZERO_VEC(V)	(MAT3_IS_ZERO((V)[0]) && \
				 MAT3_IS_ZERO((V)[1]) && \
				 MAT3_IS_ZERO((V)[2]))

/* 
 * DESCR   :	Tests if two numbers @A@ and @B@ are within EPSILON of each 
 *         	other.
 */
#define MAT3__EQ(A,B)					\
	((A) < (B) ? (((B) - (A)) < MAT3_EPSILON) : 	\
		     (((A) - (B)) < MAT3_EPSILON))

/*
 * DESCR   :	Tests if vectors @V1@ and @V2@ are within EPSILON of each other.
 */
#define MAT3_EQUAL_VECS(V1, V2)						\
	(MAT3__EQ((V1)[0], (V2)[0]) && MAT3__EQ((V1)[1], (V2)[1]) &&	\
	 MAT3__EQ((V1)[2], (V2)[2]))

/*
 * TITLE   :	Vector operations
 * RETURNS :	void
 */

#define MAT3_SET_VEC(RESULT_V,X,Y,Z)	\
        ((RESULT_V)[0]=(X), (RESULT_V)[1]=(Y), (RESULT_V)[2]=(Z) MAT3_FIX)
#define MAT3_COPY_VEC(TO,FROM)	((TO)[0] = (FROM)[0], \
				 (TO)[1] = (FROM)[1], \
				 (TO)[2] = (FROM)[2] MAT3_FIX)

/*
 * TITLE   :
 * DESCR   :	Basic operations on vector @V@, that modify vector
 *		@RESULT_V@.
 *
 */

#define MAT3_NEGATE_VEC(RESULT_V,V) \
        MAT3_SET_VEC(RESULT_V, -(V)[0], -(V)[1], -(V)[2])

#define MAT3_SCALE_VEC(RESULT_V,V,SCALE) \
	MAT3_SET_VEC(RESULT_V, (V)[0]*(SCALE), (V)[1]*(SCALE), (V)[2]*(SCALE))


#define MAT3_ADD_VEC(RESULT_V,V1,V2) \
	MAT3_SET_VEC(RESULT_V, (V1)[0]+(V2)[0], (V1)[1]+(V2)[1], \
			       (V1)[2]+(V2)[2])

#define MAT3_SUB_VEC(RESULT_V,V1,V2) \
	MAT3_SET_VEC(RESULT_V, (V1)[0]-(V2)[0], (V1)[1]-(V2)[1], \
			       (V1)[2]-(V2)[2])

#define MAT3_MULT_VEC(RESULT_V,V1,V2) \
	MAT3_SET_VEC(RESULT_V, (V1)[0]*(V2)[0], (V1)[1]*(V2)[1], \
			       (V1)[2]*(V2)[2])

/*
 * TITLE   :
 * DESCR   :	More  complex operations on multiple vectors
 *
 */

/*
 * DESCR   :	Compute dot product of 3-vectors @V1@ and @V2@
 * RETURNS :	double
 */
#define MAT3_DOT_PRODUCT(V1,V2) \
			((V1)[0]*(V2)[0] + (V1)[1]*(V2)[1] + (V1)[2]*(V2)[2])

/*
 * DESCR   :	Compute length of 3-vector @V@.
 * RETURNS :	double
 */
#define MAT3_LENGTH_VEC(V)	(sqrt(MAT3_DOT_PRODUCT(V, V)))

/*
 * DESCR :	Place the cross product of @VEC1@ and @VEC2@ in @RESULT_VEC@.
 * 		@TEMP@ is used to hold an intermediate result.
 * DETAILS :	The factors and the product vectors may safely be the same.
 */
#define MAT3_CROSS_PRODUCT(RESULT_VEC, VEC1, VEC2, TEMP)             \
       ((TEMP)[0] = (VEC1)[1] * (VEC2)[2] - (VEC1)[2] * (VEC2)[1],   \
        (TEMP)[1] = (VEC1)[2] * (VEC2)[0] - (VEC1)[0] * (VEC2)[2],   \
        (TEMP)[2] = (VEC1)[0] * (VEC2)[1] - (VEC1)[1] * (VEC2)[0],   \
        MAT3_COPY_VEC(RESULT_VEC, TEMP))

/*
 * DESCR :	Place the cross product of @VEC1@ and @VEC2@ in @RVEC@.
 * 		The arguments must not be the same as the result vector.
 * DETAILS :	The result cannot be the same as the factor vectors.
 */
#define MAT3_CROSS_PRODUCT_QUICK(RVEC, VEC1, VEC2)                  \
       ((RVEC)[0] = (VEC1)[1] * (VEC2)[2] - (VEC1)[2] * (VEC2)[1],  \
        (RVEC)[1] = (VEC1)[2] * (VEC2)[0] - (VEC1)[0] * (VEC2)[2],  \
        (RVEC)[2] = (VEC1)[0] * (VEC2)[1] - (VEC1)[1] * (VEC2)[0]  MAT3_FIX)


/*
 * DESCR   :	Normalize the 3-vector @V@, using @TEMP@ as temporary variable.
 * DETAILS :	If the norm of @V@ is too close to zero, no normalization
 * 		takes place, and @TEMP@ is set to zero.
 */
#define MAT3_NORMALIZE_VEC(V,TEMP) \
	if ((TEMP = sqrt(MAT3_DOT_PRODUCT(V,V))) > MAT3_EPSILON) { \
	   TEMP = 1.0 / TEMP; \
	   MAT3_SCALE_VEC(V,V,TEMP); \
	} else TEMP = 0.0

/*
 * DESCR   :	Sets @RESULT_V@ to the linear combination of @V1@ and @V2@,
 * 		scaled by @SCALE1@ and @SCALE2@, respectively 
 */
#define MAT3_LINEAR_COMB(RESULT_V,SCALE1,V1,SCALE2,V2) \
	MAT3_SET_VEC(RESULT_V,	(SCALE1)*(V1)[0] + (SCALE2)*(V2)[0], \
				(SCALE1)*(V1)[1] + (SCALE2)*(V2)[1], \
				(SCALE1)*(V1)[2] + (SCALE2)*(V2)[2])

/*
 * DESCR   :	Set @RESULT_V@ to be @V1@ + @SCALE2@ * @V2@.
 */
#define MAT3_RAY_POINT(RESULT_V,V1,SCALE2,V2) \
	MAT3_SET_VEC(RESULT_V,	(V1)[0] + (SCALE2)*(V2)[0], \
				(V1)[1] + (SCALE2)*(V2)[1], \
				(V1)[2] + (SCALE2)*(V2)[2])

/*
 * DESCR   :	The point @POINT@ and the plane-through-the-origin given by
 * 		the equation @NORMAL@ dot (X, Y, Z) = 0 together can be used
 * 		to determine a basis for 3-space. The second vector in the
 * 		basis is the plane normal @NORMAL@. The first is some vector
 * 		perpendicular to this, i.e., some vector in the plane. The
 * 		third vector in the basis is the cross product of these two.
 * 		These vectors are placed in the first three rows of the 4
 * 		x 4 matrix @BASIS@, and the last row gets the given point
 * 		@POINT@ put in it.
 */
#define MAT3_BASIS_FROM_PLANE(BASIS, POINT, NORMAL)                    \
        ((BASIS)[0][3]=(BASIS)[1][3]=(BASIS)[2][3]=0, (BASIS)[3][3]=1, \
         MAT3_COPY_VEC((BASIS)[3], POINT),                             \
         MAT3_COPY_VEC((BASIS)[1], NORMAL),                            \
         MAT3perp_vec ((BASIS)[0], NORMAL, TRUE),                      \
         MAT3_CROSS_PRODUCT_QUICK((BASIS)[2], NORMAL, (BASIS)[0]))

/*
 * TITLE   :	Homogeneous vector operations
 */

#define MAT3_SET_HVEC(V,X,Y,Z,W) ((V)[0]=(X), (V)[1]=(Y), \
				  (V)[2]=(Z), (V)[3]=(W) MAT3_FIX)

#define MAT3_COPY_HVEC(TO,FROM) ((TO)[0] = (FROM)[0], \
				 (TO)[1] = (FROM)[1], \
				 (TO)[2] = (FROM)[2], \
				 (TO)[3] = (FROM)[3] MAT3_FIX)

/*
 * TITLE   :
 * DESCR   :	Basic operations on homogeneous vectors
 *
 */
#define MAT3_ADD_HVEC(RESULT_V,V1,V2) 					\
	MAT3_SET_HVEC(RESULT_V, (V1)[0]+(V2)[0], (V1)[1]+(V2)[1], 	\
			       (V1)[2]+(V2)[2], (V1)[3]+(V2)[3] )

#define MAT3_SCALE_HVEC(RESULT_V,V,SCALE)				\
	MAT3_SET_HVEC(RESULT_V, (V)[0]*(SCALE), (V)[1]*(SCALE), 	\
		     (V)[2]*(SCALE), (V)[3]*(SCALE))

/*
 * TITLE   :	Fast math
 * DESCR   :	Compute sin or cos of @X@ (an angle in radians) by table lookup
 * 		Returns the approximate value of the trig function. @X@ is in
 *		degrees in the second two macros.
 *
 * RETURNS :	double
 */
#define MAT3_FAST_SIN(X)   (MAT3_sin[(int)(180.0*(X)/M_PI) % 360]) /* X is in */
#define MAT3_FAST_COS(X)   (MAT3_cos[(int)(180.0*(X)/M_PI) % 360]) /* radians */
#define MAT3_FAST_SIN_DEG(X) (MAT3_sin[((int) (X)) % 360]) /* X is in  */
#define MAT3_FAST_COS_DEG(X) (MAT3_cos[((int) (X)) % 360]) /* degrees  */

/* ------------------------------  Entries  ------------------------------- */

/* In MAT3factor.c */

BOOL MAT3factor(
         MAT3mat m,			/* Source matrix to be factored  */
         MAT3mat r,			/* Rotation for conjugation  */
         MAT3mat s,			/* Scale in rotated coord system  */ 
         MAT3mat u,			/* Rotation of conjugated matrix  */
         MAT3mat t			/* Translation component  */
);
BOOL MAT3eigen_values(
         MAT3vec values, 		/* Eigenvalues of the second argument */
         MAT3mat mat	 		/* Matrix of interest */
);


/* In MAT3geom.c */

void MAT3direction_matrix(
         register MAT3mat result_mat,	/* Matrix for transforming directions */
         register MAT3mat mat 		/* Source matrix */
);
int MAT3normal_matrix(
         register MAT3mat result_mat,	/* The compute normal matrix */
         register MAT3mat mat 		/* Matrix from which it is computed */
);
void MAT3scale(
         MAT3mat result_mat,		/* Scale matrix [output] */
         MAT3vec scale 			/* The entries for the diagonal */
);
void MAT3rotate(
         MAT3mat result_mat,		/* Rotation matrix [output] */
         MAT3vec axis,			/* The axis of the rotation matrix */
         double	angle_in_radians	/* The angle of rotation */
);
void MAT3translate(
         MAT3mat result_mat,		/* Translation matrix [output] */
         MAT3vec trans			/* Translation vector [input]  */
);
void MAT3mirror(
         MAT3mat result_mat,		/* Matrix reflected through plane */
         MAT3vec normal 		/* Plane normal to plane thru origin */
);
void MAT3shear(
         register MAT3mat result_mat,	/* Shear matrix [output] */
         register MAT3vec normal,	/* Normal to the shear plane */
         register MAT3vec shear 	/* Shearing vector [perp to normal] */
);
int MAT3orient(
         MAT3mat result_mat,		/* Orientation matrix [output] */
         MAT3vec from,			/* Pt to which origin is translated */
         MAT3vec at,			/* Z-axis is transformed to from-at */
         MAT3vec up 			/* Y-axis is transformed to from-up */
);
int MAT3span(
         MAT3mat   result_mat,		/* Span xform matrix [output] */
         MAT3vec   from,		/* Where the -1 point on Z-axis goes */
         MAT3vec   at,			/* Where the +1 point on Z-axis goes */
         MAT3vec   up,			/* Where the Y-axis goes */
         int       vol_preserve 	/* Should we preserve volume?  */
);
int MAT3axis_and_angle(
         register MAT3mat mat,		/* A rotation matrix to be studied */
         MAT3vec 	  axis,		/* The axis of rotation */
         double	 	 *angle 	/* The amount of the rotation */
);
void MAT3align(
         MAT3mat result_mat,	/* Computed matrix aligning vec1 with vec2 */
         MAT3vec vec1,	/* Vector to be transformed */
         MAT3vec vec2 	/* We want to move vec1 so that it points same */
         /* dir'n as vec2 */);


/* In MAT3inv.c */

int MAT3invert(
         MAT3mat result_mat,		/* The computed inverse */
         MAT3mat mat 			/* The matrix to be inverted */);


/* In MAT3mat.c */

void MAT3identity(
         MAT3mat m 			/* Matrix to be set to identity */
);
void MAT3zero(
         MAT3mat m 			/* Matrix to be set to zero */
);
double MAT3determinant(
         MAT3mat mat 			/* Matrix of interest */
);
void MAT3copy(
         MAT3mat t,			/* Target matrix [output] */
         MAT3mat f			/* Source matrix [input] */
);
void MAT3mult(
         MAT3mat result_mat,		/* Computed product [output] */
         MAT3mat mat1,			/* The first factor of the product */
         MAT3mat mat2 			/* The second factor of the product */
);
void MAT3transpose(
         MAT3mat result_mat,		/* The computed transpose */
         MAT3mat mat 			/* Matrix whose transpose we compute */
);
void MAT3print(
         MAT3mat mat,			/* The matrix to be printed */
         FILE	*fp 			/* The stream on which to print it */
);
void MAT3print_formatted(
         MAT3mat mat,			/* Matrix to be printed out */
         FILE	*fp,			/* File stream to which it is printed */
         char	*title,			/* Title of printout */
         char	*head,			/* Printed before each row of matrix */
         char	*format,		/* Format for entries of matrix */
         char	*tail 			/* Printed after each row of matrix */
);
BOOL MAT3equal(
         MAT3mat m1,			/* First matrix to be compared */
         MAT3mat m2,			/* Second matrix to be compared */
         double	epsilon 		/* Range for a match */
);
double MAT3trace(
         MAT3mat mat			/* Matrix to compute trace for */
);
int MAT3power(
         MAT3mat result_mat,		/* Source matrix raised to the power */
         MAT3mat mat,			/* Source matrix */
         int	power 			/* Power to raise by */
);
int MAT3column_reduce(
         MAT3mat result_mat,		/* Column-reduced matrix [output] */
         MAT3mat mat,			/* Original matrix [input] */
         double	 epsilon 		/* Tolerance for testing against 0 */
);
int MAT3kernel_basis(
         MAT3mat result_mat,		/* Basis of kernel of matx [output] */
         MAT3mat 	 mat,		/* Matrix whose kernel we compute */
         double	 epsilon 		/* Tolerance for comparison with zero */
);
void MAT3scalar_mult(
         MAT3mat result_mat,		/* Result of scale [output] */
         MAT3mat m,			/* Matrix to be scaled [input] */
         double  scalar 		/* Scaling factor [input] */
);
void MAT3mat_add(
         MAT3mat result_mat,		/* The sum of the matrices [output] */
         MAT3mat A,			/* The first summand [input] */
         MAT3mat B 			/* The second summand [input] */
);


/* In MAT3vec.c */

/* The ordering of "vec" and "mat" parameters in following three functions is
 * probably counterintuitive but is retained for backward compatibility.
 */
void MAT3mult_vec(
         register MAT3vec result_vec,	/* Product of vec and mat (output) */
         register MAT3vec vec,		/* Vec to be multiplied (input) */
         register MAT3mat mat 		/* Matrix it's multiplied by (input)*/
);
int MAT3mult_hvec(
         MAT3hvec          result_vec,	/* Product of vec with mat (output) */
         register MAT3hvec vec,		/* Vector to multiply (input) */
         register MAT3mat  mat,		/* Matrix to multiply (input) */
         int               homogenize	/* Should we homogenize the result? */
);
int MAT3mult_vec_affine(
         MAT3vec           result_vec,	/* The computed product (output) */
         register MAT3vec  vec,		/* The vector factor (input) */
         register MAT3mat  mat		/* The matrix factor (input) */
);
void MAT3cross_product(
         MAT3vec 	 result_vec,	/* Computed cross-product (output) */
         register MAT3vec vec1,		/* The first factor (input) */
         register MAT3vec vec2 		/* The second factor (input) */
);
void MAT3perp_vec(
         MAT3vec result_vec,		/* The vector perp to vec (output) */
         MAT3vec vec,			/* Vector to which a perp is needed */
         int is_unit 			/* Is input vector is a unit vector? */
);

#endif /* MAT3_HAS_BEEN_INCLUDED */
