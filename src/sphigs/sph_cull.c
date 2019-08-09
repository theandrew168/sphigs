#ifndef lint
static char Version[]=
   "$Id: sph_cull.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_cull.c
 *
 *	Backface culling unit, changes p_normals in objects.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include <stdio.h>
#include <assert.h>
#include "sphigslocal.h"

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Calculates normals for each facet, and optionally reduces the
 * 		number of nodes in the objects list through backface culling
 * 		with the UVN normals.  
 * ------------------------------------------------------------------------- */
void
OBJECT__cull (
   view_spec    *vs,
   register obj *current,	
   boolean       enable_cull)
{
   register obj        *prev;		/* keep the previous for node removal */
   register MAT3hvec   *verts;		/* vertex list */
   MAT3hvec	 	vec1;		/* subtracted vector 1 */
   MAT3hvec	 	vec2;		/* subtracted vector 2 */
   MAT3hvec	 	canon_normal;	/* canonized normal to the face */
   double	 	length;		/* used in normalize */
   
   verts = vs->npcVertices;

   switch (current->type) {

    case OBJ_LINE:
    case OBJ_POINT:
    case OBJ_TEXT:
      /* normals calculated elsewhere */
      break;

    case OBJ_FACE:	       

      /* Calculate Normal */
      MAT3_SUB_VEC (vec1,
     	       verts [current->data.face.points[1]],
     	       verts [current->data.face.points[0]]);
      MAT3_SUB_VEC (vec2,
     	       verts [current->data.face.points[2]],
     	       verts [current->data.face.points[1]]);
      vec1[3] = 1;
      vec2[3] = 1;
      MAT3cross_product ( current->p_normal, vec1, vec2);
      MAT3_NORMALIZE_VEC( current->p_normal, length );
      current->p_normal[3] = 1;


      /* Backface Culling for Painter's Algorithm */
      if( enable_cull &&
          ! current->data.face.doubleSided &&
          current->p_normal[Z] < -MAT3_EPSILON ) {
         current->visible = FALSE;
      }
      break;
   }	
}
