#ifndef lint
static char Version[]=
   "$Id: sph_utility.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_utility.c
 *
 *      Functions creating geometric data 
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"

/* --------------------------- Internal Routines --------------------------- */

/** UNUSED!
The below functions perform operations on "intelligent rectangles":
   SRGP rectangles that "know" whether they are empty.
**/

#ifdef COMMENT_OUT_THAT_FUNKY_THING
void
SPH__clip_intelligent_rectangle(
   intelligent_rectangle *ir,
   srgp__rectangle 	  sr
   )
{
   if (ir->nonempty) {
      if (GEOM_computeRectIntersection (ir->rect, sr, &(ir->rect)))
         ir->nonempty = TRUE;
      else
         ir->nonempty = FALSE;
   }
}

void
SPH_expand_intelligent_rectangle(
   intelligent_rectangle *ir,
   srgp__rectangle        sr
   )
{
   if (ir->nonempty)
      GEOM_computeRectUnion (ir->rect, sr, &(ir->rect));
   else
      ir->rect = sr;

   ir->nonempty = TRUE;
}
#endif


/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Sets a point in 3-D coordinate space
 * ------------------------------------------------------------------------- */
double *
SPH_defPoint(
   double *pt,  /* should already point to an array of 3 doubles */
   double x,
   double y,
   double z
   )
{
   pt[0] = x;
   pt[1] = y;
   pt[2] = z;
   return pt;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Sets a rectangle in normalized device coordinates
 * ------------------------------------------------------------------------- */
NDC_rectangle 
SPH_defNDCrectangle(
   double left_x,
   double bottom_y,
   double right_x,
   double top_y
   )
{
   NDC_rectangle rect;

   rect.bottom_left.x = left_x;
   rect.bottom_left.y = bottom_y;
   rect.top_right.x = right_x;
   rect.top_right.y = top_y;
   return rect;
}
