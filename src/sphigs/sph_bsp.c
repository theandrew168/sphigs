#ifndef lint
static char Version[]=
   "$Id: sph_bsp.c,v 1.1 1993/03/09 02:00:54 crb Exp crb $";
#endif

/* -------------------------------------------------------------------------
 * sph_bsp.c
 *
 * 	Binary Space Partition Trees.  Used to order polygons along the
 * z-axis.  For now, we recompute the tree everytime any structure geometry
 * changes.
 * ------------------------------------------------------------------------- */

#define NDEBUG		/* lift assertions */
#include "HEADERS.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sphigslocal.h"

/* ---------------------------- Static Routines ---------------------------- */

/* The following code is based loosely on rendering code from a software package
 * named "fnord" that performs multi-dimensional formula visualization,
 * courtesy of Matthew D. Stone.
 */

/*
 * Copyright 1990, 1991, 1992, Brown University, Providence, RI
 *
 * Permission to use and modify this software and its documentation for
 * any purpose other than its incorporation into a commercial product is
 * hereby granted without fee.  Permission to copy and distribute this
 * software and its documentation only for non-commercial use is also
 * granted without fee, provided, however, that the above copyright notice
 * appear in all copies, that both that copyright notice and this permission
 * notice appear in supporting documentation, that the name of Brown
 * University not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission,
 * and that the person doing the distribution notify Brown University of
 * such distributions outside of his or her organization. Brown University
 * makes no representations about the suitability of this software for
 * any purpose.  It is provided "as is" without express or implied warranty.
 * Brown University requests notification of any modifications to this
 * software or its documentation.
 *
 * Send the following redistribution information:
 *
 *	Name:
 *	Organization:
 *	Address (postal and/or electronic):
 *
 * To:
 *	Software Librarian
 *	Computer Science Department, Box 1910
 *	Brown University
 *	Providence, RI 02912
 *
 *		or
 *
 *	brusd@cs.brown.edu
 *
 * We will acknowledge all electronic notifications.
 *
 */


/* ------------------------------- Constants ------------------------------- */

#ifdef THINK_C
#define random Random
#else
/* extern long random(); */
#endif

#define SQRT sqrt
#define EXP  exp
#define LOG  log
#define RAND rand

#define BSP_CARELESSNESS_FACTOR 10000.0
#define BSP_SWITCH_PT 50

/* --------------------------------- Types --------------------------------- */

typedef enum {
   FIRST_MODE = 0,
   RANDOM_MODE,
   PLANE_MODE
} BSP_treeMode;

typedef enum {
   MUST_SPLIT = 0x8,
   THIS_SIDE  = 0x4,
   THAT_SIDE  = 0x2,
   DONT_CARE  = 0x1
} BSP_compareCode;

/* -------------------------------- Macros --------------------------------- */

extern double BSP__epsilon;		/* varying epsilon factor */

#define BSP_IS_ZERO(N) 		((N) < BSP__epsilon && (N) > -BSP__epsilon)
#define BSP_IS_POS(N)		((N) > BSP__epsilon)
#define BSP_IS_NEG(N)		((N) < -BSP__epsilon)
#define BSP_EQUAL(A,B)		BSP_IS_ZERO((A) - (B))

#define BSP_IS_ZERO_VEC(V) 	(BSP_IS_ZERO((V)[X]) && 	\
				 BSP_IS_ZERO((V)[Y]) && 	\
				 BSP_IS_ZERO((V)[Z]))
#define BSP_EQUAL_VEC(A, B) 	(BSP_EQUAL((A)[X], (B)[X]) &&	\
				 BSP_EQUAL((A)[Y], (B)[Y]) &&	\
				 BSP_EQUAL((A)[Z], (B)[Z]))


#define BSP_LIST_INSERT(list, item)  	((item)->front=(list), (list)=(item))
#define BSP_PLANE_DIST(norm, dist, pt)  (MAT3_DOT_PRODUCT(norm, pt) - (dist))
#define BSP_PLANE_SIDE(dist)		(BSP_IS_POS (dist) ? THIS_SIDE : \
					 BSP_IS_NEG (dist) ? THAT_SIDE : \
					                     DONT_CARE )
#define BSP_DIFFERENT_SIDES(S1, S2) 		  \
   ((((S1) & THIS_SIDE) && ((S2) & THAT_SIDE)) || \
    (((S1) & THAT_SIDE) && ((S2) & THIS_SIDE)))


#define MAKE_NEW_OBJECT(babyobj)					\
   (babyobj) = (obj*)							\
      FALLOCalloc (vs->objectChunk, sizeof(obj), FALLOC__DONT_ZERO);	
#define FALLOC_RECORDS(VAR, TYPE, NUM)					\
     (VAR = (TYPE *) FALLOCalloc (vs->objectChunk,		\
				  sizeof(TYPE) * (NUM), 	\
				  FALLOC__DONT_ZERO))

/* ------------------------------ Prototypes ------------------------------- */

static int 		BSP__computeNormal();
static void 		BSP__bounds();
static obj * 		BSP__choose();
static BSP_compareCode	BSP__compare();
static void 		BSP__flatten();
static void 		BSP__splitLine();
static void 		BSP__splitPolygon();
static obj *	 	BSP__linesPtsTree();
static obj *	 	BSP__newPlane();
static void	 	BSP__sort();
static obj * 		BSP__make_tree();

static double BSP__epsilon;		/* varying epsilon factor */

static obj   *zorder_head;			/* object list in z-order */
static obj   *zorder_tail;

#define ZLIST_INITIALIZE	(zorder_head = zorder_tail = NULL)
#define ZLIST_INSERT(item)  	{					\
				   if (zorder_tail)			\
  				      zorder_tail->next = (item);	\
                                   else					\
				      zorder_head = (item);		\
				   zorder_tail = (item); 		\
 			 	}
#define ZLIST_COMPLETE		(zorder_tail->next = NULL, zorder_head)

/* -------------------------- Diagnostic Routines -------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Measures the depth of a linear tree.
 * ------------------------------------------------------------------------- */
static int
BSP__treeListLength(register obj *x)
{
   register int count = 0;
   while (x) {
      count++;
      x = x->front;
   }
   return count;
}
static int
BSP__objListLength(register obj *x)
{
   register int count = 0;
   while (x) {
      count++;
      x = x->front;
   }
   return count;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Tests a polygon for convexity / planarity.
 * ------------------------------------------------------------------------- */
static int
BSP__checkConvex(
   view_spec    *vs,
   register obj *x)
{
   register int	      i1, i2, i3;
   MAT3hvec	      normal, test, temp1, temp2;
   double	      dummy;
   MAT3hvec *	      verts 	= vs->mcVertices;	
   vertex_index	     *indices 	= x->data.face.points;
   int		      count 	= x->data.face.numPoints;

   i1 = count - 2;
   i2 = count - 1;
   i3 = 0;

   MAT3_SUB_VEC (temp1, verts [indices [i2]], verts [indices [i1]]);
   MAT3_SUB_VEC (temp2, verts [indices [i3]], verts [indices [i2]]);
   MAT3_CROSS_PRODUCT_QUICK (normal, temp1, temp2);
   MAT3_NORMALIZE_VEC (normal, dummy);

   for(;  i3 < count;  i1 = i2, i2 = i3, i3++) {
      MAT3_SUB_VEC (temp1, verts [indices [i2]], verts [indices [i1]]);
      MAT3_SUB_VEC (temp2, verts [indices [i3]], verts [indices [i2]]);
      MAT3_CROSS_PRODUCT_QUICK (test, temp1, temp2);
      MAT3_NORMALIZE_VEC (test, dummy);
      
      if (!BSP_EQUAL_VEC (test, normal))
	 return FALSE;
   }

   return TRUE;
}

/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Computes the normal vector to a polygon to be ordered.  If
 * 		the first cross product that we calculate for the polygon has
 * 		problems (ie. a zero-length normal, or close enough), we take
 * 		the next pair of adjacent edges.  Generally this isn't a
 * 		problem, since non-convex polygons are disallowed, and
 * 		slivers are rare.
 * ------------------------------------------------------------------------- */
static int
BSP__computeNormal(
   MAT3hvec  *verts,
   obj       *poly,
   double    *area )
{
   register int       i1, i2, i3;
   register typeFace *face = &poly->data.face;
   vertex_index      *indices = face->points;
   vector_3D   	      first, second;

   i1 = face->numPoints-2;
   i2 = face->numPoints-1;
   i3 = 0;

   do {
      MAT3_SUB_VEC (first,  verts [indices [i2]], verts [indices [i1]]);
      MAT3_SUB_VEC (second, verts [indices [i3]], verts [indices [i2]]);
      MAT3_CROSS_PRODUCT_QUICK (poly->normal, first, second);

      poly->directed_distance = BSP_PLANE_DIST(poly->normal, 0.0,
					       verts [indices [i2]]); 

      if (area)
	 *area = MAT3_DOT_PRODUCT (poly->normal, poly->normal);

      i1=i2; i2=i3; i3++;

      /* try to get a valid normal */
      if (!BSP_IS_ZERO_VEC (poly->normal))
	 return TRUE;
   }
   while (BSP_IS_ZERO_VEC(poly->normal) && i3 < face->numPoints);


   /* Any polygons that reach this point are slivers and should be thrown out,
    * since their vertices are (roughly) colinear. */

   return (FALSE);
}

/* -------------------------------------------------------------------------
 * DESCR   :	 Picks a triangle at random from the list passed and removes
 * 		 it from the list, returning the list.  
 * ------------------------------------------------------------------------- */
static obj *
BSP__choose(
   obj *polygons,
   int 	polyCount )
{
   register int i;
   register obj *prev;
   register obj *curr;
   int limit;

   limit = (RAND() % polyCount) - 1;

   /* find the nth polygon */
   for(prev = polygons, curr = polygons->front, i = 0; 
       i < limit; 
       prev = curr, curr = curr->front, i++)
      ;

   prev->front = curr->front;
   curr->front = NULL;

   return curr;
}


/* -------------------------------------------------------------------------
 * DESCR   :	Takes an element to go into the bsp tree and computes the
 * 		distance between each of its vertices and the plane defined
 * 		by the passed plane. It returns a code indicatinq whether all
 * 		the vertices of the polygon are above the plane (or close
 * 		enough), all the vertices of the polygon are below the plane
 * 		(or close enough), or the polygon will have to be split. 
 * ------------------------------------------------------------------------- */
static BSP_compareCode
BSP__compare(
   MAT3hvec *verts,
   obj      *plane,
   obj      *curObj,
   double   *dists )
{
   double d;
   register int i;
   register int above, below;
   vertex_index  buf[2];
   vertex_index *indices;
   int           vCount;

   OBJECT__get_vertex_indices (curObj, &indices, &vCount);

   above = below = 0;

   for(i = 0;  i < vCount;  i++)
   {
      d = dists[i] = BSP_PLANE_DIST(plane->normal,
				    plane->directed_distance, 
				    verts [indices [i]]);
      if (BSP_IS_POS(d))
	 (above)++;
      else if (BSP_IS_NEG(d))
	 (below)++;
   }
   
   if (!(below) && !(above))
      return DONT_CARE;
   if (!(below))
      return THIS_SIDE;
   if (!(above))
      return THAT_SIDE;
   return MUST_SPLIT;
}

/* -------------------------------------------------------------------------
 * DESCR   :	This function is used to create balanced trees out of the
 * 		linear list of lines and points left around when the polygons
 * 		have all been taken care of in BSP__make_tree.
 * ------------------------------------------------------------------------- */
static void 
BSP__flatten(
   obj *tree,
   obj *list )
{
   register obj *treeCurr, *treeNext;
   register obj *objCurr, *objPrev;

   objCurr = list;
   for(treeCurr = tree;
       treeCurr != NULL;
       treeCurr = treeNext)
   {
      treeNext = treeCurr->front;

      treeCurr->front = objCurr;
      if (objCurr) {
	 objPrev = objCurr;
	 objCurr = objCurr->front;
      }

      treeCurr->back = objCurr;
      if (objCurr) {
	 objPrev = objCurr;
	 objCurr = objCurr->front;
      }
   }
   
   if (objCurr) {
      objPrev->front = NULL;
      BSP__flatten(list, objCurr);
   }
   else {
      objCurr = list;
      while (objCurr) {
	 objPrev = objCurr;
	 objCurr = objCurr->front;
	 objPrev->front = NULL;
      }
   }
}


/* -------------------------------------------------------------------------
 * DESCR   :	Splits a line segment known to be cut by a plane into
 * 		two new segments using the previously calculated plane
 * 		distances for each point.		
 * ------------------------------------------------------------------------- */
static void 
BSP__interpolate(
   MAT3hvec	      result,
   MAT3hvec 	     *verts,
   vertex_index	      index1,
   vertex_index	      index2,
   double	      dist1, 
   double	      dist2 )
{
   register 	      i;
   double	      ratio;

   /* we already calculated these in BSP__compare */
   ratio = dist1 / (dist1 - dist2);


   /* Compute the intersection point and adjust endpoints */
   MAT3_SUB_VEC   (result, verts [index2], verts [index1]);
   MAT3_SCALE_VEC (result, result, ratio);
   MAT3_ADD_VEC   (result, result, verts [index1]);
   
   for (i = X; i <= Z; i++)
      assert ((!BSP_IS_POS (verts[index2][i]-result[i]) &&
	       !BSP_IS_POS (result[i]-verts[index1][i])) ||
	      (!BSP_IS_NEG (verts[index2][i]-result[i]) &&
	       !BSP_IS_NEG (result[i]-verts[index1][i])));
}


/* -------------------------------------------------------------------------
 * DESCR   :	Takes a segment known to be split by a plane and that plane,
 * 		along with two lists to add the new segments to, and the
 * 		distance information about the endpoints of the segment
 * 		computed in the BSP__compare function, and does what you'd
 * 		think.  It creates two subsegments of the passed segment--
 * 		the parts above and below the plane.  The part above the
 * 		plane is added to the list pointed to by front; the part
 * 		below the plane is added to the part of the list pointed to
 * 		by back.  
 * ------------------------------------------------------------------------- */
static void 
BSP__splitLine(
   view_spec    *vs,
   register obj	*oldObj,
   obj 	       **front,
   obj 	       **back,
   double       *dists)
{
   register obj      *newObj;
   register typeLine *newLine, *oldLine;
   MAT3hvec	      newVert;
   vertex_index	      newIndex;

   /* Get the memory we need. */
   MAKE_NEW_OBJECT (newObj);
   COPY_STRUCT (*newObj, *oldObj);
   OBJECT__addToViewSpec (vs, newObj);

   oldLine = &oldObj->data.line;
   newLine = &newObj->data.line;

   /* Compute the intersection point and adjust endpoints */
   BSP__interpolate (newVert, vs->mcVertices,
		     oldLine->pt[0], oldLine->pt[1], dists[0], dists[1]);

   BSP_LIST_INSERT (*front, newObj);
   BSP_LIST_INSERT (*back,  oldObj);

   newIndex = SPH__add_global_point (vs, newVert, FALSE);
   if (BSP_IS_POS (dists[0]))
      newLine->pt[1] = oldLine->pt[0] = newIndex;
   else
      newLine->pt[0] = oldLine->pt[1] = newIndex;
}


/* -------------------------------------------------------------------------
 * DESCR   :	This is another function that does basically nothing
 * 		surprising.  It takes the plane passed and splits the polygon
 * 		into new polygons that are not intersected by the plane.
 * 		Those polygons that are above the plane are added onto the
 * 		list specified by front; those below it are added to the back
 * 		list.  Dists is the vertex location information calculated in
 * 		the comparison function.
 * ------------------------------------------------------------------------- */
static void
BSP__splitPolygon(
   view_spec    *vs,
   register obj *oldObj,
   obj	       **front,		/* OUTPUT */
   obj	       **back,	        /* OUTPUT */
   double       *dists )
{
   register int	      i, lasti, count;
   register int       side, newSide;
   register obj      *newObj = NULL;

   obj		      objBuffer;
   MAT3hvec	      newVert;
   vertex_index	      newIndex, savIndex = -1;
   int                polycount = 0;
   typeFace	     *newFace, *oldFace;
   boolean	      edgeFlag;


#  define INSERT_POINT(face, index, draw_edge)  			    \
     ((face->points [face->numPoints] = index),				    \
      (draw_edge) ? SPH__setBit (face->edgeFlags, face->numPoints) : FALSE, \
      face->numPoints++)

   assert (BSP__checkConvex(vs, oldObj));
   COPY_STRUCT (objBuffer, *oldObj);
   oldFace = &objBuffer.data.face;


   /* let's start with a boundary vertex (one that begins a new poly) */
   side = BSP_PLANE_SIDE (dists [lasti = oldFace->numPoints - 1]);
   for(i = 0;  i < oldFace->numPoints;  lasti = i, i++)
   {
      newSide = BSP_PLANE_SIDE (dists [i]);
      if (BSP_DIFFERENT_SIDES (side, newSide))
	 break;
      side |= newSide;
   }


   for(count = 0;
       count < oldFace->numPoints;
       count++,  lasti = i,  i = (i+1) % oldFace->numPoints) {

      newSide = BSP_PLANE_SIDE (dists [i]);
      if (BSP_DIFFERENT_SIDES (side, newSide)) 
      {
	 /* interpolate new vertex if necessary */
	 if (BSP_PLANE_SIDE (dists[lasti]) == DONT_CARE)
	    newIndex = oldFace->points [lasti];
	 else {
	    BSP__interpolate (newVert, vs->mcVertices,
			      oldFace->points[lasti], oldFace->points[i],
			      dists[lasti], dists[i]);
	    newIndex = SPH__add_global_point (vs, newVert, FALSE);
	    
	    /* complete the most recent new poly */
	    if (newObj == NULL)
	       savIndex = newIndex;
	    else			
	       INSERT_POINT (newFace, newIndex,
			     SPH__testBit(oldFace->edgeFlags, i));
	 }
	
	 /* make new object (reuse the original object's memory first) */
	 if (newObj == NULL)
	    newObj = oldObj;

	 else {
	    /* adjust min/max to reflect new vertices */	    
	    BSP__computeNormal (vs->mcVertices, newObj, NULL);
	    assert (BSP__checkConvex(vs, newObj));
	    assert (newFace->numPoints > 2);

	    MAKE_NEW_OBJECT (newObj);
	    COPY_STRUCT (*newObj, objBuffer);
	    OBJECT__addToViewSpec (vs, newObj);
	 }

	 polycount++;
	 newFace = &newObj->data.face;
	 FALLOC_RECORDS (newFace->points, vertex_index, oldFace->numPoints+1);
	 SPH__fallocNBitstring (newFace->edgeFlags, oldFace->numPoints+1);
	 SPH__clearNBitstring (newFace->edgeFlags, oldFace->numPoints+1);
	 newFace->numPoints = 0;
	 INSERT_POINT (newFace, newIndex, FALSE);


	 /* insert it in the proper list */
	 if (newSide & THIS_SIDE)
	    BSP_LIST_INSERT (*front, newObj);
         else
	    BSP_LIST_INSERT (*back, newObj);
	 side = 0;
      }

      /* we should have a new polygon on the first iteration */
      assert (newObj);

      INSERT_POINT (newFace, oldFace->points[i], 
		    SPH__testBit(oldFace->edgeFlags, i));
      
      side |= newSide;
   }
      
   /* Complete the last polygon */
   if (savIndex != -1)
      INSERT_POINT (newFace, savIndex, SPH__testBit(oldFace->edgeFlags, i));
   BSP__computeNormal (vs->mcVertices, newObj, NULL);
   assert (BSP__checkConvex(vs, newObj));
   assert (newFace->numPoints > 2);


   assert (polycount > 1);

   /* this function should work for any polygon, but we ought to only
      have convex ones for now */
   assert (polycount == 2);
}


/* -------------------------------------------------------------------------
 * DESCR   :	This function makes a BSP tree full of items that are not
 * 		order important even in hidden surface removal: lines and
 * 		points. 
 * ------------------------------------------------------------------------- */
static obj *
BSP__linesPtsTree(
   obj *objects )
{
   obj *ret;

   if (NULL == objects || NULL == objects->front)
      return objects;

   ret = objects;
   objects = objects->front;
   ret->front = NULL;

   BSP__flatten(ret, objects);

   return ret;
}


/* -------------------------------------------------------------------------
 * DESCR   :	This function generates a new plane whose normal is along the
 * 		axis of greatest extent and whose distance divides that
 * 		extent in half. 
 * ------------------------------------------------------------------------- */
static obj *
BSP__newPlane(
   view_spec *vs,
   MAT3hvec   min,
   MAT3hvec   max,
   MAT3hvec   fmin,
   MAT3hvec   fmax,
   MAT3hvec   bmin,
   MAT3hvec   bmax )
{
   double dist;
   register int    i;
   int    axis;
   obj   *newObj;

   MAKE_NEW_OBJECT (newObj);
   newObj->type = OBJ_NULL;

   dist = 0.0;
   axis = X;
   for(i = X; i <= Z; i++)
      if ((max[i] - min[i]) > dist) {
	 dist = max[i] - min[i];
	 axis = i;	
      }

   MAT3_SET_HVEC(newObj->normal, 0.0, 0.0, 0.0, 1.0);
   newObj->normal[axis] = -1.0;
   newObj->directed_distance = dist =  dist/2 + min[axis];

   MAT3_COPY_HVEC (fmin, min);
   MAT3_COPY_HVEC (bmin, min);
   MAT3_COPY_HVEC (fmax, max);
   MAT3_COPY_HVEC (bmax, max);
   fmin[axis] = bmax[axis] = dist;

   return newObj;
}
   

/* -------------------------------------------------------------------------
 * DESCR   :	Performs the task of splitting objects into two groups, each
 * 		of which lies on a different side of the splitting plane.
 * ------------------------------------------------------------------------- */
static void
BSP__sort(
   view_spec *vs,
   obj       *root,
   obj       *objList,
#ifdef THINK_C
   void       SPLIT(...),
#else
   void       SPLIT(),
#endif

   obj **front,		/* OUTPUTS: - objects in front */
   int  *numFront,	/*	    - their count */
   obj **back,		/* 	    - objects in back */
   int  *numBack)	/*	    - their count */
   
{
   register obj *curr, *next;
   MAT3hvec *verts = vs->mcVertices;

   double dists[MAX_VERTS_PER_OBJECT];
   int    decision;

   *front = *back = NULL;
   *numFront = *numBack = 0;

   for (curr = objList; curr != NULL; curr = next) {
      next = curr->front;

      assert (BSP__treeListLength (*front) >= *numFront);
      assert (BSP__treeListLength (*back) >= *numBack);

      decision = BSP__compare(verts, root, curr, dists);
      switch(decision) {

       case THIS_SIDE:
	 BSP_LIST_INSERT (*front, curr);
	 if (curr->type > OBJ_NULL)
	    (*numFront)++;
	 break;
	 
       case DONT_CARE:
	 if (*numFront < *numBack)
	 {
  	    BSP_LIST_INSERT (*front, curr);
	    if (curr->type > OBJ_NULL)
	       (*numFront)++;
	    break;
	 }
	 /* fall through */
	 
       case THAT_SIDE:
	 BSP_LIST_INSERT (*back, curr);
	 if (curr->type > OBJ_NULL)
	    (*numBack)++;
	 break;

       case MUST_SPLIT:
	 SPLIT (vs, curr, front, back, dists);
	 if (curr->type > OBJ_NULL) {
	    (*numFront)++;
	    (*numBack)++;
	 }
	 break;

       default:
	 assert (FALSE);
      }
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	This is the recursive function that actually does the bsp
 * 		sort/divide thing.  First it checks to make sure there are
 * 		triangles to split on.  If there aren't, then it happily
 * 		chucks the "stuff" that remains into a balanced tree, using
 * 		BSP__flatten, and returns that.  Otherwise it finds a good
 * 		polygon to split on (the last one if this one might be, or
 * 		else one chosen at random) and splits the two lists on it.
 * 		This involves going through each item in the list and putting
 * 		it (or its parts) on one or the other side of the random
 * 		polygon.  The number of polygons left is approximated (not
 * 		calculated), but that's OK since all we do with it is choose
 * 		which polygon to split on next.
 * ------------------------------------------------------------------------- */
static obj *
BSP__makeTree(
   view_spec   *vs,
   obj 	       *polygons,
   obj 	       *linesPts,
   int	        polyCount,
   MAT3hvec     min,
   MAT3hvec     max,
   BSP_treeMode mode )
{
   register obj *curr, *next;
   register obj *root;
   obj 		*front, *back;

   obj      *front_stuff, *back_stuff;
   MAT3hvec  dists, fmin, fmax, bmin, bmax;
   int       num_front, num_back;
   int       decision;

   assert (BSP__treeListLength (polygons) == polyCount);
   if (!polygons)
      return linesPts;

   /* Mode gets set at the end */
   switch(mode)
   {
    case FIRST_MODE:
      root = polygons;
      polygons = polygons->front;
      break;
      
    case PLANE_MODE:
    case RANDOM_MODE:
      root = BSP__choose(polygons, polyCount);
      break;

/*     case PLANE_MODE: */

   /* Unused.  The idea was to create new planes to partition the scene space
    * in half,  and thus guarantee tree balance.  In reality this is a lot
    * slower. */

      root = BSP__newPlane (vs, min, max, fmin, fmax, bmin, bmax);
      break;
   }


   /* Split all the "stuff"--the things that are not triangles */
   BSP__sort (vs, root, linesPts, BSP__splitLine,
	      &front_stuff, &num_front, &back_stuff, &num_back);


   /* Split all the polygons. */
   BSP__sort (vs, root, polygons, BSP__splitPolygon,
	      &front, &num_front, &back, &num_back);
   assert (BSP__treeListLength (front) >= num_front);
   assert (BSP__treeListLength (back) >= num_back);

#  define NEW_MODE(MODE, POLY_COUNT)				\
      ((2 > (POLY_COUNT)) ?	 	 FIRST_MODE : 		\
       (BSP_SWITCH_PT > (POLY_COUNT)) ?  RANDOM_MODE :		\
       ((MODE) == PLANE_MODE) ? 	 PLANE_MODE : 		\
                                         RANDOM_MODE )		\

						  
   root->front = (front == NULL) ? 
      BSP__linesPtsTree (front_stuff) :
      BSP__makeTree (vs, front, front_stuff, num_front, fmin, fmax,
		     NEW_MODE (mode, num_front));

   root->back = (back == NULL) ? 
      BSP__linesPtsTree (back_stuff) :
      BSP__makeTree (vs, back, back_stuff, num_back, bmin, bmax,
		     NEW_MODE (mode, num_back));

   return root;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Finds the bounding cube for a scene, minus all TEXT objects
 * ------------------------------------------------------------------------- */
static void
BSP__bounds (
   view_spec *vs,
   MAT3hvec   min,	/* OUTPUT */
   MAT3hvec   max)	/* OUTPUT */
{
   register obj *curr = vs->objects;
   register int  i;

   /* find first non-text object */
   for (curr = vs->objects;
	curr && curr->type == OBJ_TEXT;
	curr = curr->next)
      ;
   if (!curr)
      return;


   MAT3_COPY_HVEC (min, curr->min);
   MAT3_COPY_HVEC (max, curr->max);

   /* find smallest inclusive bounding cube */
   for (curr = curr->next;
	curr;
	curr = curr->next) {

      if (curr->type == OBJ_TEXT)
	 continue;

      for (i = X; i <= Z; i++) {
	 if (curr->min[i] < min[i])
	    min[i] = curr->min[i];
	 if (curr->max[i] > max[i])
	    max[i] = curr->max[i];
      }
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	In-order display traversal for BSP tree.
 * ------------------------------------------------------------------------- */
static void
BSP__display (
   register view_spec *vs,
   register MAT3hvec   viewPt,
   register obj       *tree)
{
   register BSP_compareCode side;

   assert (tree);
   ZLIST_INSERT (tree);

   side = (tree->type != OBJ_FACE) ?
      DONT_CARE :
      BSP_PLANE_SIDE (BSP_PLANE_DIST(tree->normal,
				     tree->directed_distance,
				     viewPt));

   switch (side) {
    case THIS_SIDE:
      if (tree->back)	    BSP__display (vs, viewPt, tree->back);
                            OBJECT__draw (vs, tree); 
      if (tree->front)	    BSP__display (vs, viewPt, tree->front);
      break;		    
			    
    case DONT_CARE:	    
      /* fall through */    
			    
    case THAT_SIDE:	    
      if (tree->front)	    BSP__display (vs, viewPt, tree->front);

      /* backface culling for facets (but not for fill Areas) */
      if (tree->type != OBJ_FACE || tree->data.face.doubleSided)
	 OBJECT__draw (vs, tree);  

      if (tree->back)  	    BSP__display (vs, viewPt, tree->back);
      break;

    default:
      assert (FALSE);
   }  
}

/* --------------------------- Internal Routines --------------------------- */ 

/* -------------------------------------------------------------------------
 * DESCR   :	This is the general function by which a pict can bsp-order
 * 		itself.  It first goes through all of the pict commands
 * 		associated with the pict and separates them into those that
 * 		can be split on and those that can't.  Those that can be
 * 		split on have a little processing done on them: they are
 * 		counted, and their plane equation (normals, and distance) are
 * 		calculated.  Triangles whose plane equations cannot be
 * 		calculated are "lined"--their offending coordinate is
 * 		returned by BSP__computeNorm, and removed, making the
 * 		ex-degenerate triangle the line it really is. The two lists
 * 		thus created are passed off to bsp_make_tree, and the
 * 		resultant tree is hooked back in as pict->commands.  Then the
 * 		pict is marked as ordered.
 * ------------------------------------------------------------------------- */
void
SPH__bsp_sort (
   view_spec *vs)
{
   register obj *curr, *next;
   register obj *polygons  = NULL;
   register obj *lines_pts = NULL;
   register obj *texts     = NULL;


   int      polyCount = 0;
   double   bigdiff = 0.0;
   double   area, sum;
   MAT3hvec min, max;

   curr = vs->objects;

   /* bounding cube for scene */
   BSP__bounds (vs, min, max);

   sum = 0.0;
   while (curr != NULL)
   {
      curr->back = NULL;
      next = curr->next;

      switch (curr->type) {

       case OBJ_FACE:
	 assert (curr->data.face.numPoints > 2);
	 if (BSP__computeNormal (vs->mcVertices, curr, &area)) {
	    BSP_LIST_INSERT (polygons, curr);
	    polyCount++;
	    sum += LOG(area);
	 } else {
	    printf("WARNING: Colinear polygon.");
	 }
	 break;

       case OBJ_TEXT:
	 BSP_LIST_INSERT (texts, curr);
	 break;

       default:
	 BSP_LIST_INSERT (lines_pts, curr);
	 break;
      }
      curr = next;
   }

   /* We're doing computer graphics here, not rocket science.  The
    * efficiency of the algorithm can be greatly improved by not worrying
    * about details (small things).  Heuristic is based on getting a fixed
    * precision on the average length of a polygon edge.
    */

   if (polyCount > 0)
      BSP__epsilon = SQRT (EXP (sum / polyCount)) / BSP_CARELESSNESS_FACTOR;
   else
      BSP__epsilon = 1.0;

   vs->objectTree = BSP__makeTree(vs, polygons, lines_pts, polyCount,
				  min, max, PLANE_MODE);
   vs->textObjects = texts;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Display traversal for BSP tree.
 * ------------------------------------------------------------------------- */
void
SPH__bsp_traverse (
   view_spec *vs)
{
   register obj *curObj;

   /* We need to get a proper z-order for pick-correlation... */
   ZLIST_INITIALIZE;

   if (vs->objectTree)
      BSP__display (vs, vs->prp, vs->objectTree);

   for (curObj = vs->textObjects;  curObj;  curObj = curObj->front) {
      ZLIST_INSERT (curObj);
      OBJECT__draw (vs, curObj);
   }

   assert (BSP__objListLength (ZLIST_COMPLETE) ==
	   BSP__objListLength (vs->objects));

   vs->objects = ZLIST_COMPLETE;
}
