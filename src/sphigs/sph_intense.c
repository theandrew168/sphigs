#ifndef lint
static char Version[]=
   "$Id: sph_intense.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_intense.c
 *
 * 	Light intensity calculations for rendering display objects.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include <stdio.h>
#include "sphigslocal.h"
#include <assert.h>

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Takes the list of objects and calculates Gouraud intensities
 * 		(flat shading) for each object.  This changes each object's
 * 		intensity field, and should be called before any polygons
 * 		have been split and while 
 * ------------------------------------------------------------------------- */
void
OBJECT__intensity (
   view_spec    *vs,
   register obj *scanObj)
{
   MAT3vec distanceVec;
   MAT3vec centerFace;
   MAT3vec oddNormal;
   MAT3vec pointLight;
   register int  i;
   register light_source *light;
   double dist;
   double amount;

   if (scanObj->type == OBJ_FACE) {

      /* ray to center of face */
      MAT3mult_vec (centerFace, scanObj->center, vs->vo_matrix);

      MAT3_COPY_VEC (oddNormal, scanObj->normal);
      MAT3_NORMALIZE_VEC (oddNormal, dist);
      if (vs->algorithm == RENDER_BSP)
	 MAT3mult_vec (oddNormal, oddNormal, currentNormalMCtoUVNxform);

      scanObj->intensity = 0.0;

      for (i = 0, light = vs->lights;
           i <= MAX_LIGHT_INDEX;
           i++, light++) {

         if (!light->active)
            continue;

         MAT3_SUB_VEC (distanceVec, light->uvnPosition, centerFace);
         MAT3_NORMALIZE_VEC (distanceVec, dist);
         amount = ABS (MAT3_DOT_PRODUCT (distanceVec, oddNormal)) *
                  light->intensity;
         if (light->attenuate)
            amount *= light->attenFactor * dist;

         scanObj->intensity += amount;
      }
   }
}
