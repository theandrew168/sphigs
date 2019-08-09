#ifndef lint
static char Version[]=
   "$Id: sph_clip.c,v 1.5 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_clip.c
 *
 *	Clips display objects against front/back clipping planes.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "sphigslocal.h"


#define FRONT 1
#define BACK  2

#define FALLOC_RECORDS(VAR, TYPE, NUM)				\
     (VAR = (TYPE *) FALLOCalloc (vs->objectChunk,		\
				  sizeof(TYPE) * (NUM), 	\
				  FALLOC__DONT_ZERO))


/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Given two endpoints, returns the point at z coordinate
 * ------------------------------------------------------------------------- */
static int
InterpolatePoint(
   MAT3hvec p1,
   MAT3hvec p2, 
   double   z,
   MAT3hvec p3)
{
   double             yz_slope;
   double             xz_slope;
   double	      zdiff; 
   
   zdiff =  (p2[Z] - p1[Z]);
   if (MAT3_IS_ZERO (zdiff))
      return FALSE;

   yz_slope =  (p2[Y] - p1[Y]) / zdiff;
   xz_slope =  (p2[X] - p1[X]) / zdiff;
   
   p3[X] = p1[X] + xz_slope *  (z - p1[Z]);
   p3[Y] = p1[Y] + yz_slope *  (z - p1[Z]);
   p3[Z] = z;
   
   return TRUE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Clips a polygon against either front or back clippng planes.
 * ------------------------------------------------------------------------- */
static void 
ClipPoly (
   view_spec *vs,
   obj       *current,
   double     wall,
   int        WHICH)
{
   register int       i;
   register int	      index1, last_index1, index2;
   register int       last_zone, current_zone;
   MAT3hvec           new_point, last_point, current_point;

   int                new_global_index;
   edge_bitstring     new_edge_flags;
   vertex_index      *new_points;

   edge_bitstring     edge_flags;
   vertex_index      *points;

   typeFace	     *face;
   register boolean   flag;
   
   
   face = &current->data.face;
   index2 = index1 = 0;	   		/* go through all the points */
   
   new_points = (vertex_index *) 
      FALLOCalloc(vs->objectChunk, 
		  sizeof(vertex_index) * 
		  (face->numPoints + 2), FALLOC__DONT_ZERO);
   assert (new_points != NULL);

   SPH__fallocNBitstring (new_edge_flags, (face->numPoints + 2));
   SPH__clearNBitstring  (new_edge_flags, (face->numPoints + 2));

   edge_flags = face->edgeFlags;
   points = 	face->points;
   assert (points != NULL);


#  define CLIP_ZONE(PT, CLIP_Z)					\
            ((WHICH == BACK  &&  (PT)[Z] < (CLIP_Z)) ? -1 :	\
             (WHICH == FRONT &&  (PT)[Z] > (CLIP_Z)) ?  1 :	\
                                                        0)

   MAT3_COPY_VEC (last_point, vs->uvnVertices [points [face->numPoints - 1]]);
   last_zone = CLIP_ZONE (last_point, wall);
   flag = (last_zone != 0);

   for(index1 = 0;
       index1 < face->numPoints;
       index1++) {

      MAT3_COPY_VEC (current_point, vs->uvnVertices [points [index1]]);
      current_zone = CLIP_ZONE (current_point, wall);
      
      if (current_zone != last_zone) {
	 InterpolatePoint (last_point, current_point, wall, new_point);
	 new_global_index = SPH__add_global_point (vs, new_point, TRUE);
	 new_points [index2] = new_global_index;
	 if (flag || SPH__testBit (edge_flags, index1))
	    SPH__setBit (new_edge_flags, index2);
	 flag = TRUE;
	 index2 ++;
      }
      
      if (current_zone == 0) {
	 new_points [index2] = points [index1];
	 if (SPH__testBit (edge_flags, index1))
	    SPH__setBit (new_edge_flags, index2);
	 flag = FALSE;
	 index2 ++;
      }
      
      MAT3_COPY_VEC (last_point, current_point);
      last_zone = current_zone;
   }

   
   face->points    = new_points;
   face->edgeFlags = new_edge_flags;
   face->numPoints = index2;   
}

/* -------------------------------------------------------------------------
 * DESCR   :	Clips a line against the two clipping planes.
 * ------------------------------------------------------------------------- */
static void
ClipLine(
   view_spec *vs,
   obj       *current,
   double     front, 
   double     back)
{
   typeLine    *line;
   MAT3hvec     p1, p2, pNew;
   vertex_index temp;

   assert (current->type == OBJ_LINE);

   line = & current->data.line;	    
   
   if (vs->uvnVertices [line->pt[0]] [Z] > 
       vs->uvnVertices [line->pt[1]] [Z])
      SWAP (line->pt[1], line->pt[0], temp);

   MAT3_COPY_VEC (p1, vs->uvnVertices [line->pt[0]]);
   MAT3_COPY_VEC (p2, vs->uvnVertices [line->pt[1]]);

   if (p2[Z] > back  &&  back > p1[Z]) {
      InterpolatePoint (p2, p1, back, pNew); 
      line->pt[0] = SPH__add_global_point (vs, pNew, TRUE);
   }
   if (p2[Z] > front  &&  front > p1[Z]) {
      InterpolatePoint (p2, p1, front, pNew);
      line->pt[1] = SPH__add_global_point (vs, pNew, TRUE);
   }
} 

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Filters the list of objects by clipping the objects against
 * 		the front and back planes. 
 * ------------------------------------------------------------------------- */
void
OBJECT__clip (
   view_spec    *vs,
   register obj *current)
{
   register obj *     newObj;
   MAT3hvec           min;
   MAT3hvec           max;
   register double    front;
   register double    back;

   front = vs->frontPlaneDistance;
   back = vs->backPlaneDistance;
   assert (back < front);
   
   MAT3_COPY_VEC (min, current->min);
   MAT3_COPY_VEC (max, current->max);         

   assert (current->next != current);
   assert (max[X] - min[X] > -MAT3_EPSILON);
   assert (max[Y] - min[Y] > -MAT3_EPSILON);
   assert (max[Z] - min[Z] > -MAT3_EPSILON);


   /* CASE 1 - Object does not lie between front or back plane */
   /* 	(this will take care of POINTs and TEXT) */
   if (max[Z] < back || min[Z] > front)
      current->display = NULL;
      

   /* CASE 2 - front or back plane intersects object, so we edit it */
   else if (max[Z] > front || min[Z] < back) {
      
      newObj = OBJECT__copy (vs, current);
	 
      /* handle line intersection with front and back planes */
      if (newObj->type == OBJ_LINE)
	 ClipLine (vs, newObj, front, back);

      else if (newObj->type == OBJ_FACE) {
         assert (newObj->data.face.numPoints >= 3);
         
         /* handle plane intersection with front and back planes */
         if (max[Z] > front)
            ClipPoly  (vs, newObj, front, FRONT);
         if (min[Z] < back)
            ClipPoly  (vs, newObj, back, BACK);	    
      }

      current->display = newObj;
   }


   /* CASE 3 - not clipped */
   else
      current->display = current;
}

