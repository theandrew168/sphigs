#ifndef lint
static char Version[]=
   "$Id: sph_object.c,v 1.5 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_object.c
 *
 *	Generation of display objects
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"
#include <stdlib.h>

/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Copies a vertex list into the viewport's common vertex list.
 * ------------------------------------------------------------------------- */
static void
AddVertices (
   view_spec *vs, 
   matrix     xform, 
   MAT3hvec  *points, 
   int 	      count)
{
   register i;

   /* Check to see if enough vertices remain in global array */
   if (vs->vertexCount + count > vs->vertexArraySize) 
   {
      vs->vertexArraySize += MAX_VERTS_PER_OBJECT;
      vs->mcVertices =  (MAT3hvec*) realloc (vs->mcVertices,
		(MALLOCARGTYPE)(vs->vertexArraySize * sizeof(MAT3hvec)));
      REALLOC (vs->mcVertices, vs->mcVertices, MAT3hvec, vs->vertexArraySize);

      if (vs->mcVertices == NULL)
	 SPH__error (ERR_MALLOC);
   }

   for (i = 0; i < count; i++)
      MAT3mult_hvec (vs->mcVertices[vs->vertexCount++], points[i], xform, 1);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Chooses a flat shading color for facet objects.
 * ------------------------------------------------------------------------- */
static void
ChooseColor (
   obj *curobj)
{
   int face_color, shade_number;

   face_color = curobj->attributes.interior_color;
   if IS_A_FLEXICOLORINDEX(face_color) {
      shade_number = 
	 fabs(curobj->intensity)*NUM_OF_SHADES_PER_FLEXICOLOR;
      if (shade_number >= NUM_OF_SHADES_PER_FLEXICOLOR)
	 shade_number = NUM_OF_SHADES_PER_FLEXICOLOR - 1;
      SRGP_setColor (BASE_OF_SHADE_LUT_ENTRIES +
		     ((face_color-2)*NUM_OF_SHADES_PER_FLEXICOLOR) +
		     shade_number);
      SRGP_setFillStyle (SOLID);
   }
   else {
      SRGP_setColor (face_color);
      SRGP_setBackgroundColor (1);
      shade_number = fabs(curobj->intensity)*64;
      if (shade_number >= 64)
	 shade_number = 63;
      SRGP_setFillStyle (BITMAP_PATTERN_OPAQUE);
      SRGP_setFillBitmapPattern (40 + shade_number);
   }
}

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Adds a display object to the view spec's display list.
 * ------------------------------------------------------------------------- */
void
OBJECT__addToViewSpec (
   view_spec *vs,
   obj	     *babyobj)
{
   if (vs->objectTail) {
      vs->objectTail->next = babyobj;
      vs->objectTail = babyobj;
   }
   else 
      vs->objects = vs->objectTail = babyobj;
   vs->objectTail->next = NULL;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Initializes list of objects to NULL.  Initializes the chunk
 * 		that will be used to falloc objects.
 *
 * DETAILS :	Makes sure there is space allocated for "fltptVertices"; if
 * 		it needs to allocate some, sets	"vertexArraySize".  Sets
 * 		"vertexCount" to 0.  
 *
 * 		TO BE CALLED just before starting to traverse the network
 * 		posted to a single view. 
 * ------------------------------------------------------------------------- */
void
OBJECT__init (
   view_spec *vs)
{
   if (vs->mcVertices == NULL) {
      ALLOC (vs->mcVertices, MAT3hvec, MAX_VERTS_PER_OBJECT);
      vs->vertexArraySize = MAX_VERTS_PER_OBJECT;
   }
   vs->vertexCount = 0;

   vs->objects     = NULL;
   vs->objectTail  = NULL;
   vs->textObjects = NULL;
   vs->objectTree  = NULL;

   vs->curTraversalIndex = 0;
   
   if (vs->objectChunk == NULL)
      vs->objectChunk = FALLOCnew_chunk();
   else
      FALLOCclear_chunk (vs->objectChunk);

}

#define FALLOC_OBJECT(dest)						\
   (dest) = (obj*)							\
      FALLOCalloc (vs->objectChunk, sizeof(obj), FALLOC__DONT_ZERO);	\

#define FALLOC_RECORDS(VAR, TYPE, NUM)					\
   (VAR) = (TYPE *) FALLOCalloc (vs->objectChunk, sizeof(TYPE) * (NUM), \
				 FALLOC__DONT_ZERO);			\

#define MAKE_BABY_OBJECT(TYPE)						\
   FALLOC_OBJECT (babyobj);						\
   babyobj->type = (TYPE);						\
   babyobj->attributes = *attrs;					\
   babyobj->traversal_index = vs->curTraversalIndex;			\
   SPH__fallocNBitstring (babyobj->names, MAX_NAME);			\
   SPH__copyNBitstring (babyobj->names, currentNameset, MAX_NAME);

/* -------------------------------------------------------------------------
 * DESCR   :	Divides a polyhedron into its component faces. 
 * ------------------------------------------------------------------------- */
void
OBJECT__addPoly(
   view_spec       *vs, 
   polyhedron      *poly, 
   boolean	    doublesided,
   matrix           xform, 	/* composite: ctm & vom */
   attribute_group *attrs)
{
   obj 		       *babyobj;
   register int 	i;
   register int 	j, lastj;
   int 			start_index;
   int 			vCount, eCount, eIndex;
   edge_bitstring	edges_added;
   facet	       *facet;
   double 		scale;

   start_index = vs->vertexCount;
   AddVertices (vs, xform, poly->vertex_list, poly->vertex_count);

   /* For each face, create a new object. */
   for (i=0; i<poly->facet_count; i++) {
      
      facet = &poly->facet_list[i];
      vCount = facet->vertex_count;

      /* Make new object */
      MAKE_BABY_OBJECT (OBJ_FACE);
      babyobj->data.face.doubleSided = doublesided;

      /* Collect Vertices */
      babyobj->data.face.numPoints = vCount;	
      FALLOC_RECORDS (babyobj->data.face.points, vertex_index, vCount);
      SPH__fallocNBitstring (babyobj->data.face.edgeFlags, vCount);
      SPH__clearNBitstring (babyobj->data.face.edgeFlags, vCount);
      MAT3_SET_VEC (babyobj->center, 0.0, 0.0, 0.0);
      
      for (lastj = vCount - 1, j = 0;  j < vCount;  lastj = j, j++)
      {
	 MAT3_ADD_VEC (babyobj->center, babyobj->center, 
		       poly->vertex_list [facet->vertex_indices[j]]);

	 babyobj->data.face.points [j] =
	    facet->vertex_indices[j] + start_index;

	 eIndex = facet->vertex_indices[lastj] * poly->vertex_count +
	          facet->vertex_indices[j];
	 SPH__setBit (babyobj->data.face.edgeFlags, j);
      }

      scale = 1.0 / vCount;
      MAT3_SCALE_VEC (babyobj->center, babyobj->center, scale);
      MAT3mult_vec (babyobj->normal,
		    poly->facet_list[i].normal, currentNormalMCtoUVNxform);
		    
      OBJECT__addToViewSpec (vs, babyobj);
   }

   vs->curTraversalIndex++;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Divides a polyline into its component segments. 
 * ------------------------------------------------------------------------- */
void
OBJECT__addLines(
   view_spec       *vs, 
   int		    vCount,
   MAT3hvec	   *verts,
   matrix           xform, 	/* composite: ctm & vom */
   attribute_group *attrs)
{
   obj *babyobj;
   register int       i;
   register MAT3hvec *pt1, *pt2;
   int start_index;

   start_index = vs->vertexCount;
   AddVertices (vs, xform, verts, vCount);

   /* For each segment, create a new object. */   
   for (i = 1, pt1 = verts, pt2 = verts + 1;
	i < vCount;
	i++, pt1++, pt2++) {

      if (MAT3_EQUAL_VECS (pt1, pt2))
	 continue;

      /* Make new object */
      MAKE_BABY_OBJECT (OBJ_LINE);
      babyobj->data.line.pt[0] = start_index + i - 1;	
      babyobj->data.line.pt[1] = start_index + i;	

      /* Compute normal */
      /* see ComputePlaneEquation(), sph_zsort.c */
		    
      OBJECT__addToViewSpec (vs, babyobj);
   }

   vs->curTraversalIndex++;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Divides a polymarker into its component points. 
 * ------------------------------------------------------------------------- */
void
OBJECT__addPoints(
   view_spec       *vs, 
   int		    vCount,
   MAT3hvec	   *verts,
   matrix           xform, 	/* composite: ctm & vom */
   attribute_group *attrs)
{
   obj *babyobj;
   register int       i;
   register MAT3hvec *p;
   int start_index;

   start_index = vs->vertexCount;
   AddVertices (vs, xform, verts, vCount);

   /* For each segment, create a new object. */   
   for (i = 0, p = verts;
	i < vCount;
	i++, p++) {

      /* Make new object */
      MAKE_BABY_OBJECT (OBJ_POINT);
      babyobj->data.point.pt = start_index + i;	

      /* Compute normal */
      /* see ComputePlaneEquation(), sph_zsort.c */
		    
      OBJECT__addToViewSpec (vs, babyobj);
   }

   vs->curTraversalIndex++;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Adds an item of annotation text.
 * ------------------------------------------------------------------------- */
void
OBJECT__addText(
   view_spec       *vs, 
   MAT3hvec	    mc_origin,
   char	           *text,
   matrix           xform, 	/* composite: ctm & vom */
   attribute_group *attrs)
{
   obj 	    *babyobj;
   MAT3hvec  plist[1];
   int start_index;

   start_index = vs->vertexCount;
   MAT3_COPY_HVEC (plist[0], mc_origin);
   AddVertices (vs, xform, plist, 1);

   /* Make new object */
   MAKE_BABY_OBJECT (OBJ_TEXT);
   babyobj->data.text.start = start_index;
   babyobj->data.text.text = text;	

   /* Compute normal */
   /* see ComputePlaneEquation(), sph_zsort.c */
		    
   OBJECT__addToViewSpec (vs, babyobj);

   /* annotation text is never pick-correlated! */
}

/* -------------------------------------------------------------------------
 * DESCR   :	Copies an object and duplicates its vertex list.  This is
 * 		primarily used to make clip objects out of polygons.
 * ------------------------------------------------------------------------- */
obj *
OBJECT__copy (
   view_spec    *vs,
   register obj *oldObj)
{
   register obj *newObj;
   register int  i;
   int           vCount;
   vertex_index *oldIndices, *newIndices;

   FALLOC_OBJECT (newObj);
   COPY_STRUCT (*newObj, *oldObj);
   OBJECT__get_vertex_indices (newObj, &oldIndices, &vCount);
   
   if (newObj->type == OBJ_FACE) {
      FALLOC_RECORDS (newObj->data.face.points, vertex_index, vCount);
      newIndices = newObj->data.face.points; 
      SPH__fallocNBitstring (newObj->data.face.edgeFlags, vCount);
      SPH__copyNBitstring (newObj->data.face.edgeFlags,
			   oldObj->data.face.edgeFlags, vCount);
   } 
   else
      newIndices = oldIndices;

   /* copy data.text.text? */

   for (i = 0; i < vCount; i++)
      newIndices [i] = SPH__add_global_point (vs, 
					      vs->uvnVertices [oldIndices [i]],
					      TRUE);
   return newObj;
}


/* ------------------------ The Rendering Pipeline ------------------------- */

#define FOREACH_OBJECT(V, OBJ)		\
   for ((OBJ) = (V)->objects;  (OBJ);  (OBJ) = (OBJ)->next)

#define FOREACH_VISIBLE_OBJECT(V, OBJ)	\
   FOREACH_OBJECT (V, OBJ)  if ((OBJ)->visible)


/* -------------------------------------------------------------------------
 * DESCR   :	This function launches the processes of culling, clipping,
 * 		etc.  Called after the objects have been collected.
 *
 * DETAILS :	The coordinates are still in mc.  Which processes are
 * 		actually done depends upon the current rendering mode.  This
 * 		will NEVER be called when rendermode is WIREFRAME_RAW
 * 		(fastest).      
 * ------------------------------------------------------------------------- */
void
OBJECT__process (
   view_spec *vs)
{
   register obj *curObj;

   if (currentRendermode == WIREFRAME_RAW)
      return;
   MAT3copy (currentMCtoUVNxform, vs->vo_matrix);


   /* BSP - sorting happens first */
   if (currentRendermode >= FLAT && vs->algorithm == RENDER_BSP &&
       vs->obsolete_objects)
   {
      FOREACH_OBJECT (vs, curObj)
	 OBJECT__bound (vs->mcVertices, curObj);
      SPH__bsp_sort (vs);
   }

   if (vs->obsolete_objects)
      vs->vertexCutoff = vs->vertexCount;
   else if (vs->obsolete_camera)
      vs->vertexCount = vs->vertexCutoff;


   /* invisibility/hilite filters */
   FOREACH_OBJECT (vs, curObj)
      OBJECT__filter (vs, curObj);


   /* camera mapping */
   if (vs->obsolete_camera || vs->obsolete_objects) {
      SPH__map_to_camera (vs);
      SPH__map_to_canon (vs);
   }

   if (currentRendermode >= LIT_FLAT) {
      SPH__computeNormalTransformer();
      SPH__viewPrepareLights (vs);     
   }
   
   FOREACH_VISIBLE_OBJECT (vs, curObj) {
      if (vs->obsolete_camera || vs->obsolete_objects) {
	 OBJECT__bound (vs->uvnVertices, curObj);
	 OBJECT__cull (vs, curObj, TRUE);         
	 OBJECT__clip (vs, curObj);
      }
      if (currentRendermode >= LIT_FLAT)
	 OBJECT__intensity (vs, curObj);
   }

   /* Painter's Algorithm - sort happens LAST */
   if (currentRendermode >= FLAT && vs->algorithm == RENDER_PAINTERS &&
       (vs->obsolete_camera || vs->obsolete_objects))
      SPH__zsort (vs);


   /* map to viewport */
   if (vs->obsolete_camera || vs->obsolete_objects)
      SPH__map_to_pdc (vs);
}


/* -------------------------------------------------------------------------
 * DESCR   :	Draws a single render object.
 * ------------------------------------------------------------------------- */
void
OBJECT__draw (
   view_spec *vs,
   obj       *curObj)
{
   register obj	     *dispObj;
   register int       v, lastv;
   typeLine	     *line;
   typeFace	     *face;
   typePoint	     *point;
   typeText	     *text;
   extern srgp__point srgp_pdc_points[];

   if (!curObj || !curObj->visible || !curObj->display)
      return;

   dispObj = curObj->display;

   switch (dispObj->type) {

    case OBJ_FACE:
      face = &dispObj->data.face;
      
      /* Place vertices in contiguous buffer */
      for	 (v = 0; v < face->numPoints; v++)
         srgp_pdc_points[v] = vs->pdcVertices[face->points[v]];

      switch (currentRendermode) {

       case FLAT:       /* Draw Face */
       case LIT_FLAT:
         if (currentRendermode == LIT_FLAT)
            ChooseColor (curObj);
         else
            SPH__set_attributes ( &dispObj->attributes, ATTRIB_FACE );
         SRGP_fillPolygon (face->numPoints, srgp_pdc_points);

	 /* fill in gaps left between split polygons */
	 SRGP_setLineWidth (1);
	 SRGP_setLineStyle (SOLID);
	 for (v = 0,  lastv = face->numPoints - 1;
	      v < face->numPoints;
	      lastv = v,  v++) 
	    if (!SPH__testBit (face->edgeFlags, v))		  
	       SRGP_line (srgp_pdc_points[lastv], srgp_pdc_points[v]);

         if (dispObj->attributes.edge_flag != EDGE_VISIBLE)
	    break;
         /* fall through */
         
       case WIREFRAME: /* Draw Edges */
	 SPH__set_attributes ( &dispObj->attributes, ATTRIB_EDGE );
	 
	 for (v = 0,  lastv = face->numPoints - 1;
	      v < face->numPoints;
	      lastv = v,  v++)
	    if (SPH__testBit (face->edgeFlags, v))		  
	       SRGP_line (srgp_pdc_points[lastv], srgp_pdc_points[v]);
      }
      break;


    case OBJ_LINE:
      line = &dispObj->data.line;

      SPH__set_attributes ( &dispObj->attributes, ATTRIB_LINE );
      SRGP_line (vs->pdcVertices[line->pt[0]], vs->pdcVertices[line->pt[1]]);
      break;

    case OBJ_POINT:
      point = &dispObj->data.point;

      SPH__set_attributes ( &dispObj->attributes, ATTRIB_MARKER );
      SRGP_marker (vs->pdcVertices[point->pt]);
      break;

    case OBJ_TEXT:
      text = &dispObj->data.text;

      SPH__set_attributes ( &dispObj->attributes, ATTRIB_TEXT );
      SRGP_text (vs->pdcVertices[text->start], text->text);
      break;
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Called after clipping, culling, sorting etc has all been done
 * 		on an entire set of objects posted to a single view. 
 *
 * DETAILS :	The objects by now are all in integer pdc coords!
 * ------------------------------------------------------------------------- */
void
OBJECT__drawAll (
   view_spec *vs)
{
   register obj *curObj;

   if (currentRendermode > WIREFRAME && vs->algorithm == RENDER_BSP)
   {
      SPH__bsp_traverse (vs);
      return;
   }

   FOREACH_VISIBLE_OBJECT (vs, curObj)
      OBJECT__draw (vs, curObj);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Returns a list of vertex indices for the relevant object.
 * ------------------------------------------------------------------------- */
void
OBJECT__get_vertex_indices (
   obj 		 *curObj,
   vertex_index **indices,	/* OUTPUT */
   int		 *vCount )	/* OUTPUT */
{
   switch (curObj->type)
   {
    case OBJ_FACE: 		*indices = curObj->data.face.points;	
                                *vCount = curObj->data.face.numPoints;
                                break;

    case OBJ_LINE: 		*indices = curObj->data.line.pt; 
                                *vCount = 2;
                                break;

    case OBJ_POINT: 		*indices = &curObj->data.point.pt;
                                *vCount = 1;                                
                                break;

    case OBJ_TEXT: 		*indices = &curObj->data.text.start;
                                *vCount = 1;                                
                                break;
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Processes object name set to set visibility/hilite flags. 
 * ------------------------------------------------------------------------- */
void
OBJECT__filter (
   view_spec    *vs,
   register obj *curObj)
{
   int filter;

   SPH__overlapNBitstrings (filter, 
			    vs->invisFilter, curObj->names, MAX_NAME);
   curObj->visible = !filter;
   SPH__overlapNBitstrings (curObj->hilite, 
			    vs->hiliteFilter, curObj->names, MAX_NAME);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Calculates the uvn bounding cube for the object
 * ------------------------------------------------------------------------- */
void
OBJECT__bound (
   register MAT3hvec *verts, 
   register obj	     *current)
{
   register int       i, j;
   MAT3hvec           p;
   MAT3hvec           max;
   MAT3hvec           min;
   typeLine	     *line;
   typeFace	     *face;
   typePoint	     *point;

   switch (current->type) {

    case OBJ_FACE:

      face = & (current->data.face);
      MAT3_COPY_VEC (max, verts [face->points[0]]);
      MAT3_COPY_VEC (min, verts [face->points[0]]);
      
      for (i = 1; i < face->numPoints; i++) {
	 MAT3_COPY_VEC (p, verts [face->points[i]]);

	 for (j = X; j <= Z; j++) {
	    if (p[j] > max[j])     max[j] = p[j];
	    if (p[j] < min[j])     min[j] = p[j];
	 }
      }

      MAT3_COPY_VEC (current->min, min);
      MAT3_COPY_VEC (current->max, max);
      break;

    case OBJ_LINE:

      line = & (current->data.line);
      for (i = X; i <= Z; i++) {
         min[i] = MIN (verts [line->pt[0]] [i], 
		       verts [line->pt[1]] [i]);
         max[i] = MAX (verts [line->pt[0]] [i], 
		       verts [line->pt[1]] [i]);
      }
      MAT3_COPY_VEC (current->min, min);
      MAT3_COPY_VEC (current->max, max);
      break;
      
    case OBJ_TEXT:
    case OBJ_POINT:

      /* these overlap anyway, but let's do good engineering */
      i = current->type == OBJ_POINT ? current->data.point.pt :
	                               current->data.text.start;
      MAT3_COPY_VEC (current->min, verts [i]);
      MAT3_COPY_VEC (current->max, verts [i]);
      break;

   }
}


/* -------------------------------------------------------------------------
 * DESCR   :	Adds a point to the global list of points, returns the index
 * 		of the point in the array; also adds the perspectivized point
 * 		into its array.
 * ------------------------------------------------------------------------- */
vertex_index
SPH__add_global_point(
   view_spec *vs,
   MAT3hvec   p,
   int	      canonize)
{
   register int count;

   assert (vs != NULL);
   assert (!canonize || vs->uvnVertices != NULL);
   assert (!canonize || vs->npcVertices != NULL);
   assert (vs->vertexArraySize != 0);
   assert (vs->vertexCount <= vs->vertexArraySize);
   
   count = vs->vertexCount;


#  define VERT_REALLOC(VLIST, SIZE) 					     \
   {									     \
      REALLOC(VLIST, VLIST, MAT3hvec, SIZE);				     \
      if ((VLIST) == NULL) 						     \
         SPH__error (ERR_MALLOC);					     \
   }


   if (vs->vertexCount == vs->vertexArraySize) 
   {
      vs->vertexArraySize /= 2;
      vs->vertexArraySize *= 3;

      VERT_REALLOC (vs->mcVertices, vs->vertexArraySize);
      if (canonize) {
	 VERT_REALLOC (vs->uvnVertices, vs->vertexArraySize);
	 VERT_REALLOC (vs->npcVertices, vs->vertexArraySize);
      }
   }
   
   MAT3_COPY_VEC ((vs->mcVertices) [count], p);
   ((vs->mcVertices) [count]) [3] = 1;

   if (canonize)
   {
      /* then we were really passed in a uvn Vertex */
      MAT3_COPY_VEC ((vs->uvnVertices) [count], p);
      ((vs->uvnVertices) [count]) [3] = 1;

      MAT3mult_hvec
	 (vs->npcVertices [count], vs->uvnVertices [count], vs->vm_matrix, 1);
      (vs->npcVertices [count])[Z] *= -1;
   }
   
   return vs->vertexCount++;
}
