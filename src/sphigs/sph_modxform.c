#ifndef lint
static char Version[]=
   "$Id: sph_modxform.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_modxform.c
 *
 *      Transformation matrices.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"
      
static double degtoradfactor = 0.01745329252;


/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Constructs a scale matrix.
 * ------------------------------------------------------------------------- */
void
SPH_scale(
   double scaleX,
   double scaleY,
   double scaleZ,
   matrix result)
{
   MAT3hvec scalevec;
   
   scalevec[0] = scaleX;
   scalevec[1] = scaleY;
   scalevec[2] = scaleZ;
   MAT3scale (result, scalevec);
}


/* -------------------------------------------------------------------------
 * DESCR   :	Constructs a translate matrix.
 * ------------------------------------------------------------------------- */
void
SPH_translate(
   double translateX,
   double translateY,
   double translateZ,
   matrix result )
{
   MAT3hvec translatevec;
   
   translatevec[0] = translateX;
   translatevec[1] = translateY;
   translatevec[2] = translateZ;
   MAT3translate (result, translatevec);
}

   
/* -------------------------------------------------------------------------
 * DESCR   :	Constructs a matrix for rotation around the X axis.
 * ------------------------------------------------------------------------- */
void
SPH_rotateX(
   double angle,
   matrix result) 
{
   static MAT3hvec rotaxisvec = {0.0, 0.0, 0.0};

   rotaxisvec[0] = 1;
   MAT3rotate (result, rotaxisvec, angle*degtoradfactor);
   rotaxisvec[0] = 0;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Constructs a matrix for rotation around the Y axis.
 * ------------------------------------------------------------------------- */
void
SPH_rotateY(
   double angle,
   matrix result) 
{
   static MAT3hvec rotaxisvec = {0.0, 0.0, 0.0};
   
   rotaxisvec[1] = 1;
   MAT3rotate (result, rotaxisvec, angle*degtoradfactor);
   rotaxisvec[1] = 0;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Constructs a matrix for rotation around the Z axis.
 * ------------------------------------------------------------------------- */
void
SPH_rotateZ(
   double angle,
   matrix result) 
{
   static MAT3hvec rotaxisvec = {0.0, 0.0, 0.0};
   
   rotaxisvec[2] = 1;
   MAT3rotate (result, rotaxisvec, angle*degtoradfactor);
   rotaxisvec[2] = 0;
}


/* -------------------------------------------------------------------------
 * DESCR   :	Provided for backward compatibility, since the vector package
 * 		once used by SPHIGS did matrix mults and transforms via row
 * 		vectors.  This function emulates that behavior.
 * ------------------------------------------------------------------------- */
void
SPH_oldComposeMatrix(
   matrix mat1,
   matrix mat2,
   matrix result) 
{
   MAT3mult (result, mat2, mat1);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Provided for linkage compatibility.  There should be no
 *		prototype for this function in sphigs.h.
 * ------------------------------------------------------------------------- */
#undef SPH_compose
void SPH_compose(matrix, matrix, matrix);	/* for THINK_C proto-checking */
void
SPH_compose(matrix p1, matrix p2, matrix p3) 
{
   SPH_oldComposeMatrix (p1, p2, p3);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Simple matrix multiplication.
 * ------------------------------------------------------------------------- */
void
SPH_composeMatrix(
   matrix mat1,
   matrix mat2,
   matrix result) 
{
   MAT3mult (result, mat1, mat2);
}
