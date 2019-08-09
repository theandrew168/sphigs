#ifndef lint
static char Version[]=
   "$Id: sph_zsort.c,v 1.5 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_zsort.c
 *
 * 	Painter's algorithm; takes the list of objects, zsorts them and
 * corrects any ambiguities.  Assumes normalized object p_normals; computes
 * plane equations.     
 *
 * CURRENTLY: splitting of polygons is commented out!  See "HORRORS". 
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include <stdio.h>
#include <assert.h>
#include "sphigslocal.h"


/* ------------------------------------------------------------------------- 
 * Support Routines (all static):
 *
 * 	int     CCW()
 * 	boolean GetIndexLine()
 * 	int     TestRelation()
 * 	int     TestPoint()
 * 	boolean TestLines()
 * 	boolean TestProjIntersect()
 * 	
 * 	boolean XYExtentIntersect()
 * 	void    PerspectBound()
 * 	void    Interpolate3DPoint()
 * 	void    ComputePlaneEquation()
 *
 * 	void    Split()
 * 	void    Sort()
 * 	void    Correct()
 * ------------------------------------------------------------------------- */


/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Counterclockwise test
 * ------------------------------------------------------------------------- */
static int
CCW(
   int        end0,
   int        end1,
   int        end2,
   view_spec *vs)
{
   register MAT3hvec *  npcVertices;
   double               dx1, dx2, dy1, dy2;
   
   assert(vs != NULL);
   assert(vs->npcVertices != NULL);

   npcVertices = vs->npcVertices;
   
   dx1 = (npcVertices [end1]) [X] - (npcVertices [end0]) [X];
   dy1 = (npcVertices [end1]) [Y] - (npcVertices [end0]) [Y];
   dx2 = (npcVertices [end2]) [X] - (npcVertices [end0]) [X];
   dy2 = (npcVertices [end2]) [Y] - (npcVertices [end0]) [Y];
   
   return (dx1 * dy2 > dy1 * dx2) ?	1 :	/* counterclockwise */
          (dx1 * dy2 < dy1 * dx2) ?    -1 :	/* clockwise */
                                        0 ;	/* colinear */
}

/* -------------------------------------------------------------------------
 * DESCR   :	Returns FALSE if the index is out of bounds
 * ------------------------------------------------------------------------- */
static boolean
GetIndexLine(
   obj *obj1,
   int  index,
   int *end1,
   int *end2)
{
   vertex_index *       points;
   
   assert(end1 != NULL);
   assert(end2 != NULL);
   assert(obj1 != NULL);
   
   switch(obj1->type) {
    case OBJ_FACE:
      points = obj1->data.face.points;
      if(index >= obj1->data.face.numPoints)
         return(FALSE);
      *end1 = points[ index ];
      if(index == obj1->data.face.numPoints - 1)
         *end2 = points[ 0 ];
      else
         *end2 = points[ index + 1 ];
      break;
    case OBJ_TEXT:
    case OBJ_LINE:
    case OBJ_POINT:
      return FALSE;
   }
   return(TRUE);
}

/* -------------------------------------------------------------------------
 * DESCR   :	returns -1 if p is behind plane with normal
 * 		returns  0 if p is on plane with normal
 * 		returns  1 if p is in front of plane with normal
 * ------------------------------------------------------------------------- */
static int
TestPoint(
   MAT3hvec p,	
   MAT3hvec normal,
   double distance)
{
   double plane_point;
   double eps = 1e-4;
   register int    result;

   plane_point = MAT3_DOT_PRODUCT (normal, p) + distance;
   
   result = (plane_point < -eps) ? -1 : 
            (plane_point > eps) ?   1 :
	                            0 ;
   if(normal[Z] < 0)
      result = -result;

   return  result;
}


/* -------------------------------------------------------------------------
 * DESCR   :	Returns TRUE if the lines intersect
 * ------------------------------------------------------------------------- */
static boolean
TestLines(
   int          end1,
   int          end2,
   int          endA,
   int          endB,
   view_spec   *vs)
{
   assert(vs != NULL);
   
   if((end1 == endA && end2 == endB) || (end1 == endB && end2 == endA))
      return FALSE;
   
   return(CCW(end1, end2, endA, vs) * CCW(end1, end2, endB, vs) < 0  &&
          CCW(endA, endB, end1, vs) * CCW(endA, endB, end2, vs) < 0);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Tests each point in obj1 against the plane equation of obj2
 * 			returns -1 if obj1 is outside obj2
 * 			returns  0 if obj1 is intersecting obj2
 * 			returns  1 if obj1 is inside obj2
 * ------------------------------------------------------------------------- */
static int
TestRelation(
   obj       *obj1,	
   obj       *obj2,	
   view_spec *vs)
{
   MAT3hvec obj1_point;
   MAT3hvec obj2_normal;
   int      index;
   int      relation;           /* relation of first point of obj1 to obj2; */
                                /*      the relation to match */
   int      new_relation;       /* the relation of each other point of obj1 */
   double   plane_point;        /* each point of the plane obj1 */
   
   assert(vs != NULL);
   assert(vs->npcVertices != NULL);
   assert(obj1 != NULL);
   assert(obj2 != NULL);
   
   switch(obj1->type) {

        case OBJ_FACE:     /******* face *******/
          index = 0;      
          MAT3_COPY_VEC(obj2_normal, obj2->p_normal);     
          MAT3_COPY_VEC(obj1_point, 
                        (vs->npcVertices)[ obj1->data.face.points[index]]);
          relation = TestPoint(obj1_point, obj2_normal,
			       obj2->directed_distance);
          
          for(index = 1; index < obj1->data.face.numPoints; index++) {       
             MAT3_COPY_VEC(obj1_point, 
                           (vs->npcVertices)[ obj1->data.face.points[index]]);
             new_relation = TestPoint(obj1_point,
                                       obj2_normal, obj2->directed_distance);
             
             /* check for intersecting between faces */      
             if(relation == 0 && new_relation != 0)
                    relation = new_relation;
             
             if(new_relation != relation && new_relation + relation == 0)
                    return(0);
          }
          return(relation);
          break;

        case OBJ_LINE:     /******* line *******/
          MAT3_COPY_VEC(obj2_normal, obj2->normal);
          
          MAT3_COPY_VEC(obj1_point, vs->npcVertices [obj1->data.line.pt[0]]);
          relation = TestPoint(obj1_point, obj2->normal, 
                                obj2->directed_distance);
          
          MAT3_COPY_VEC(obj1_point, vs->npcVertices [obj1->data.line.pt[1]]);
          new_relation = TestPoint(obj1_point, obj2->normal,
                                    obj2->directed_distance);
          if(relation != new_relation)
             return(0);
          
          return(relation);
          break;
          
        case OBJ_POINT:    /******* point *******/
assert(FALSE);	 
          MAT3_COPY_VEC(obj2_normal, obj2->normal);
          MAT3_COPY_VEC(obj1_point, vs->npcVertices [obj1->data.point.pt]);
          relation = TestPoint(obj1_point, obj2->normal,
                                obj2->directed_distance);
          return(relation);
   }
}


/* -------------------------------------------------------------------------
 * DESCR   :	Returns TRUE if the projections intersect
 *
 * DETAILS :	A brutal n^2 approach
 * ------------------------------------------------------------------------- */
static boolean
TestProjIntersect(
   obj       *obj1,	
   obj       *obj2,	
   view_spec *vs)
{
   int  end1;
   int  end2;
   int  endA;
   int  endB;
   int  i, j;
   int  old_cw;
   int  last;
   int  cw;
   boolean same;
   
   assert(vs != NULL);
   assert(obj1 != NULL);
   assert(obj2 != NULL);

/* 
   last = -1;
   same = TRUE;
   for( i = 0; i < obj1->data.face.numPoints; i++) {
      GetIndexLine(obj2, i, &end1, &end2);
      cw = CCW(obj1->data.face.points[ i ], end1, end2, vs);
      if(last == -1) {
         old_cw = cw;
         last = 0;
      } else {
         same = same & (cw == old_cw);
      }
   }
   if(same)
      return(TRUE);
   
   last = -1;
   same = TRUE;
   for(i = 0; i < obj2->data.face.numPoints; i++) {
      GetIndexLine(obj1, i, &end1, &end2);
      cw = CCW(obj2->data.face.points[ i ], end1, end2, vs);
      if(last == -1) {
         old_cw = cw;
         last = 0;
      } else {
         same = same & (cw == old_cw);
      }
   }
   if(same)
      return(TRUE);
*/
   
   assert(vs != NULL);
   
   i = 0;
   while(GetIndexLine(obj1, i, &end1, &end2)) {
      j = 0;
      while(GetIndexLine(obj2, j, &endA, &endB)) {
         if(TestLines(end1, end2, endA, endB, vs))
            return TRUE;
         j ++;
      }
      i ++;
   }
   return FALSE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	unused!
 * ------------------------------------------------------------------------- */
#ifdef DONT_COMPILE_THIS_CHUNK_OF_JUNK
static void
Interpolate3DPoint(
   obj         *obj1,
   MAT3hvec     start,
   MAT3hvec 	end,	
   MAT3hvec 	new)
{
   double               p;
   MAT3hvec             slope;
   
   MAT3_SUB_VEC (slope, end, start);
   p = - (MAT3_DOT_PRODUCT (obj1->normal, start) + obj1->directed_distance) /
	 MAT3_DOT_PRODUCT (obj1->normal, slope);

   MAT3_SCALE_VEC (new, slope, p);
   MAT3_ADD_VEC (new, new, start);
}
#endif

/* -------------------------------------------------------------------------
 * DESCR   :	unused!
 *
 * RETURNS :	unused!
 * ------------------------------------------------------------------------- */
#ifdef NEVER_EVER_EVER_WILL_BE_DEFINED_DARNIT
static int
MakeSplitPointList(
   int           antizone,	
   obj          *obj1,
   obj          *obj2,
   vertex_index *new_points,
   view_spec    *vs 
   )
{
   int           last_zone;
   int           current_zone;
   int           index1, index2;
   MAT3hvec      new_point;
   MAT3hvec      last_point;
   MAT3hvec      current_point;
   int           new_global_index;
   vertex_index *points;
   MAT3hvec     *uvnVertices;
   boolean       final_point;
   
   assert(obj1 != NULL);
   assert(obj2 != NULL);
   assert(new_points != NULL);
   assert(vs != NULL);
   assert(vs->uvnVertices != NULL);
   
   uvnVertices = vs->uvnVertices;
   
   /* go through all the points */
   index2 = 0;
   index1 = 0;
   
   points = & obj2->data.face.points[0];
   assert(points != NULL);
   
   MAT3_COPY_VEC(last_point, uvnVertices[ points[ index1 ] ]);
   last_zone = TestPoint(last_point, obj1->normal, obj1->directed_distance);
   if(last_zone != antizone) {                  /* getting front face first */
      new_points[ index2 ] = points[ index1 ];
      index2 ++;
   }
   
   final_point = FALSE;
   for(index1 = 1;
       index1 <= obj2->data.face.numPoints && ! final_point;
       index1 ++) {
          
      if(index1 == obj2->data.face.numPoints) {
         final_point = TRUE;
         MAT3_COPY_VEC(current_point, uvnVertices[ points[ 0 ] ]);
      } else
         MAT3_COPY_VEC(current_point, uvnVertices[ points[ index1 ] ]);
          
      current_zone = TestPoint(current_point,
                                obj1->normal, obj1->directed_distance);
          
      /* interpolate only when going from zone 1 -> -1 or 1 -> 1 */
      if(current_zone != last_zone && current_zone + last_zone == 0) {   
         Interpolate3DPoint(last_point, current_point, obj1, new_point);
         new_global_index = SPH__add_global_point(vs, new_point);
         new_points[ index2 ] = new_global_index;
         index2 ++;     
      }
          
      if(current_zone != antizone && ! final_point) {
         new_points[ index2 ] = points[ index1 ];
         index2 ++;
      }
          
      MAT3_COPY_VEC(last_point, current_point);
      last_zone = current_zone;
   }
   
   return index2;
}
#endif

/* -------------------------------------------------------------------------
 * DESCR   :	Splits obj2 along its intersection with obj1
 * ------------------------------------------------------------------------- */
#ifdef GET_RID_OF_THAT_NASTY_CODE
static void
Split(
   obj       *obj1,
   obj       *obj2,
   obj       *obj2prev,
   view_spec *vs)
{
   vertex_index *       new_list1;
   vertex_index *       new_list2;
   int          size_list1;
   int          size_list2;
   obj *                new_object;
   
   assert(vs != NULL);
   assert(vs->uvnVertices != NULL);
   assert(obj1 != NULL);
   assert(obj2 != NULL);
   assert(obj2prev != NULL);
   
   new_list1 = (vertex_index *) 
      FALLOCalloc(vs->objectChunk,
                  sizeof(vertex_index) * (obj2->data.face.numPoints + 2),
                  FALLOC__DONT_ZERO);
   assert(new_list1 != NULL);

   new_list2 = (vertex_index *) 
      FALLOCalloc(vs->objectChunk,
                  sizeof(vertex_index) * (obj2->data.face.numPoints + 2), 
                  FALLOC__DONT_ZERO);
   assert(new_list2 != NULL);
   
   size_list1 = MakeSplitPointList(-1, obj1, obj2, new_list1, vs);
   size_list2 = MakeSplitPointList(1, obj1, obj2, new_list2, vs);
   
   new_object = (obj *)
          FALLOCalloc(vs->objectChunk,  sizeof(obj), FALLOC__DONT_ZERO);
   assert(new_object != NULL);
   *new_object = *obj2;
   
   obj2->data.face.points = new_list1;
   new_object->data.face.points = new_list2;
   obj2->data.face.numPoints = size_list1;
   new_object->data.face.numPoints = size_list2;
   
   SPH__bound(vs, obj2);
   SPH__bound(vs, new_object);
   
   if(obj2->min[Z] < new_object->min[Z])
          obj2->next = new_object;
   else {
          obj2prev->next = new_object;
          new_object->next = obj2;
   }
}
#endif


/* -------------------------------------------------------------------------
 * DESCR   :	Calculates the bounding cube for the object
 * ------------------------------------------------------------------------- */
static void
PerspectBound(
   view_spec *vs,
   obj       *current)
{
   register int i;
   MAT3hvec     p, p1, p2;
   MAT3hvec     max;
   MAT3hvec     min;
   
   assert(vs != NULL);
   assert(vs->npcVertices != NULL);
   assert(current != NULL);
   
   switch(current->type) {

    case OBJ_FACE:         

      max[X] = max[Y] = -HUGE_VAL;
      min[X] = min[Y] = HUGE_VAL;
      for(i = 0; i < current->data.face.numPoints; i++) {
         MAT3_COPY_VEC(p, (vs->npcVertices)[current->data.face.points[i]]);
         if(p[X] > max[X])
            max[X] = p[X];
         if(p[Y] > max[Y])
            max[Y] = p[Y];
         
         if(p[X] < min[X])
            min[X] = p[X];
         if(p[Y] < min[Y])
            min[Y] = p[Y];
      }           
      MAT3_COPY_VEC(current->p_min, min);
      MAT3_COPY_VEC(current->p_max, max);
      break;

    case OBJ_LINE:         

      MAT3_COPY_VEC(p1, (vs->npcVertices)[current->data.line.pt[0]]);
      MAT3_COPY_VEC(p2, (vs->npcVertices)[current->data.line.pt[1]]);
      for(i = X; i <= Y; i++) {
	 max[i] = MAX (p1[i], p2[i]);
	 min[i] = MIN (p1[i], p2[i]);
      }
      MAT3_COPY_VEC(current->p_min, min);
      MAT3_COPY_VEC(current->p_max, max);
      break;

    case OBJ_TEXT:         
    case OBJ_POINT:         
      
      i = current->type==OBJ_POINT ? current->data.point.pt :
	                             current->data.text.start ;
      MAT3_COPY_VEC(p, (vs->npcVertices)[i]);
      MAT3_COPY_VEC(current->p_min, p);
      MAT3_COPY_VEC(current->p_max, p);
      break;
   }
}


/* -------------------------------------------------------------------------
 * DESCR   :	Computes the plane equation for the object, and calculates
 * 		the normalized "normal" for lines 
 * ------------------------------------------------------------------------- */
static void
ComputePlaneEquation(
   view_spec *vs,
   obj       *current)
{
   MAT3hvec             p, p1, p2;
   MAT3hvec             normal;
   double               length;
   
   assert(current != NULL);
   assert(vs != NULL);
   assert(vs->npcVertices != NULL);
   
   PerspectBound(vs, current);
   
   switch(current->type) {
    case OBJ_FACE:
      MAT3_COPY_VEC(p, vs->npcVertices[ current->data.face.points[0] ]);

      MAT3_COPY_VEC(normal, current->p_normal);
      current->directed_distance = - MAT3_DOT_PRODUCT (normal, p);
      break;
          
    case OBJ_LINE:
      MAT3_COPY_VEC (p1, vs->npcVertices [current->data.line.pt[0]]);
      MAT3_COPY_VEC (p2, vs->npcVertices [current->data.line.pt[1]]);

      MAT3_SET_HVEC (normal,  p2[Z]-p1[Z], 0, p2[X]-p1[X],  1);
      MAT3_NORMALIZE_VEC(normal, length);
      MAT3_COPY_VEC(current->normal, normal);
      current->directed_distance =  - MAT3_DOT_PRODUCT(normal, p1);
      break;
          
    case OBJ_TEXT:
    case OBJ_POINT:
      break;
   }
}


/* -------------------------------------------------------------------------
 * DESCR   :	Returns TRUE if the x-y bounds intersect
 * ------------------------------------------------------------------------- */
static boolean
XYExtentIntersect(
   obj *obj1,	
   obj *obj2)
{
   MAT3hvec             obj2_p_min, obj2_p_max;
   boolean              x_intersect;
   boolean              y_intersect;
   
   MAT3_COPY_VEC((double *) obj2_p_min, (double *) obj2->p_min);
   MAT3_COPY_VEC((double *) obj2_p_max, (double *) obj2->p_max);
   
   x_intersect = (obj1->p_min[X] >= obj2_p_min[X] &&
                  obj1->p_min[X] < obj2_p_max[X]) |
                 (obj1->p_max[X] > obj2_p_min[X] &&
                  obj1->p_max[X] <= obj2_p_max[X]);
   y_intersect = (obj1->p_min[Y] >= obj2_p_min[Y] && 
                  obj1->p_min[Y] < obj2_p_max[Y]) |
                 (obj1->p_max[Y] > obj2_p_min[Y] && 
                  obj1->p_max[Y] <= obj2_p_max[Y]);
   return  x_intersect & y_intersect;
}


/* -------------------------------------------------------------------------
 * DESCR   :	Uses a simple insertion sort to sort the bounding cubes of
 * 		the objects.
 * ------------------------------------------------------------------------- */
static void
Sort(
   view_spec *vs)
{
   obj *list;
   obj *sorted_list;            /* list sorted from back to front */
   obj *invisible_list;
   obj *current;
   obj *prev;
   obj *next_object;
   
   assert(vs != NULL);
   list = vs->objects;
   sorted_list = NULL;
   invisible_list = NULL;

   while(list != NULL) {        

      if (!list->visible) {
	 next_object = list->next;
	 list->next = invisible_list;
	 invisible_list = list;
	 /* for now we just throw these away */
      } 

      else {
         ComputePlaneEquation(vs, list);
         list->already_moved = FALSE;
         
         prev = NULL;
         current = sorted_list;
         
         /* search in the new list for the place to put it */
         while(current != NULL && 
               (list->type == OBJ_TEXT || list->min[Z] > current->min[Z])) {
            prev = current;
            assert(current != current->next);
            current = current->next;
         }
             
         if(prev == NULL)
            sorted_list = list;
         else
            prev->next = list;
             
         next_object = list->next;
         list->next = current;
      }

      list = next_object;         
   }
   
   vs->objects = sorted_list;
}

/* -------------------------------------------------------------------------
 * DESCR   :	The heart of the algorithm; checks objects in intersecting z
 * 		bounds against each other with four tests 
 * ------------------------------------------------------------------------- */
static void
Correct(
   view_spec *vs)
{
   obj *list;
   obj *current_prev;
   obj *current;
   obj *current_next;
   obj *scan_prev;
   obj *scan;
   obj *scan_next;
   int  relation1, relation2;
   obj  swap;
   int  advance;

   assert(vs != NULL);
   
   list = vs->objects;
   
   for (current = list, current_prev = NULL;
	current != NULL;
	(advance) ? (current_prev = current, current = current->next) : NULL) {


      advance = TRUE;
      if (current->type == OBJ_TEXT || !current->visible)
	 continue;


      for (scan = current->next, scan_prev = current;
	   scan != NULL && scan->min[Z] <= current->max[Z];
	   scan_prev = scan, scan = scan->next) {	 
	 
	 /* text should be at end of list */
	 if (scan->type == OBJ_TEXT)
	    continue;


         if(! XYExtentIntersect(current, scan))		/* test 1 */
            continue;

         relation1 = TestRelation(current, scan, vs);	/* test 2 */
         if(relation1 == -1)
            continue;

         relation2 = TestRelation(scan, current, vs);	/* test 3 */
         if(relation2 == 1)
            continue;
             
         if(relation1 == relation2 == 0) {
            /* HORRORS! a pair of intersecting polygons! */
            /* Split(current, scan, scan_prev, vs); */
            /*********scan_prev = scan;         SKLAR added as a guess*/
            /*********scan = scan->next;        SKLAR added as a guess*/
            /* continue; */  /*SKLAR commented out as a guess*/
         }
             
         if(!TestProjIntersect(scan, current, vs))	/* test 4 */
            continue;
             
         if(current->already_moved)
            continue;
             

	 /* Optimization bug - should just do pointer manipulations
	    but the following is easier to code */

         current_next = current->next;
         scan_next = scan->next;
             
         swap = *scan, *scan = *current, *current = swap;
         
         current->next = current_next;
         scan->next = scan_next;
         scan->already_moved = TRUE;
             

         /* restart this level of scanning */        
	 advance = FALSE;
         break;
      }          
   }    
   
   vs->objects = list;
}


/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Sorts and corrects the list of objects
 * ------------------------------------------------------------------------- */
void
SPH__zsort(
   view_spec *vs)
{
   assert(vs != NULL);
   
   if(vs->objects != NULL)
      Sort(vs);
   
   if(vs->objects != NULL)
      Correct(vs);
}
