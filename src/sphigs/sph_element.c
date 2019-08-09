#ifndef lint
static char Version[]=
   "$Id: sph_element.c,v 1.5 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_element.c
 *
 *	Routines for creating structure elements
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"
#include <string.h>

#ifdef THINK_C
#include <stdlib.h>
#endif


static element *baby;       /* temporary used to create new elements */

#define MAKE_BABY_ELEMENT   ALLOC_RECORDS (baby, element, 1)
#define INSERT_BABY         SPH__insertElement (baby)



/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Copies a string
 * ------------------------------------------------------------------------- */
static char *
MAKE_COPY_OF_STRING(
   char *str)
{
   char *copy;

   ALLOC (copy, char, strlen(str)+1);
   strcpy (copy, str);
   return copy;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Converts point list for polyLines
 * ------------------------------------------------------------------------- */
static void
StorePointList (
   element *baby,
   int      vCount, 
   point   *verts)
{
   register i;

   baby->info.count = vCount;
   ALLOC (baby->data.points, MAT3hvec, vCount);

   /* COPY VERTICES, TRANSFORMING TO HVERTS. */
   for (i=0; i<vCount; i++)
      MAT3_SET_HVEC (baby->data.points[i] ,
                     verts[i][0], verts[i][1], verts[i][2], 1.0);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Copies point list for poly-vertex objects
 * ------------------------------------------------------------------------- */
static void
CopyPointList (
   element  *baby,
   int       vCount, 
   MAT3hvec *verts)
{
   baby->info.count = vCount;

   ALLOC (baby->data.points, MAT3hvec, vCount);
   COPY_BYTE (baby->data.points, verts, MAT3hvec, vCount);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Tests a polygon for convexity / planarity, also calculates
 *		its normal.
 * ------------------------------------------------------------------------- */
static int
FacetCheck(
   register facet    *facet,
   register MAT3hvec *verts )
{
   register int	          i1, i2, i3;
   register vertex_index *indices = facet->vertex_indices;
   MAT3hvec	          test, temp1, temp2;
   double	          dummy;
   int		          count	=   facet->vertex_count;

   i1 = count - 2;
   i2 = count - 1;
   i3 = 0;

   MAT3_SUB_VEC (temp1, verts [indices [i2]], verts [indices [i1]]);
   MAT3_SUB_VEC (temp2, verts [indices [i3]], verts [indices [i2]]);
   MAT3cross_product (facet->normal, temp1, temp2);
   MAT3_NORMALIZE_VEC (facet->normal, dummy);
   facet->normal [3] = 1.0;

   for(;  i3 < count;  i1 = i2, i2 = i3, i3++) {
      MAT3_SUB_VEC (temp1, verts [indices [i2]], verts [indices [i1]]);
      MAT3_SUB_VEC (temp2, verts [indices [i3]], verts [indices [i2]]);
      MAT3cross_product (test, temp1, temp2);
      MAT3_NORMALIZE_VEC (test, dummy);
      
      if (!MAT3_EQUAL_VECS (test, facet->normal))
	 return FALSE;
   }

   return TRUE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Abbreviated element creation
 * ------------------------------------------------------------------------- */
static void 
SimpleElement (
   int type,
   int value)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = type;
   baby->data.value = value;
   INSERT_BABY;
}

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Polyhedron
 * ------------------------------------------------------------------------- */
polyhedron *
SPH__newPolyhedron (
   int   	 numverts, 
   int   	 numfacets,
   MAT3hvec 	*verts,
   vertex_index *facets)
{
   register int 	  i;
   register vertex_index *viptr;
   vertex_index 	 *start_viptr;
   facet 		 *fptr;
   polyhedron 		 *newpoly;

   ALLOC_RECORDS (newpoly, polyhedron, 1);
   ALLOC_RECORDS (newpoly->facet_list, facet, numfacets);

   /* assumedly this has already been done by StorePointList() */
/*    ALLOC_RECORDS (newpoly->vertex_list, MAT3hvec, numverts); */
   
   newpoly->vertex_count = numverts;
   newpoly->facet_count = numfacets;
   newpoly->vertex_list = verts;
   
   /* SEPARATE LONG LIST OF VERTEX INDICES INTO INDIVIDUAL FACET DEF'S */
   fptr = newpoly->facet_list;
   viptr = facets;
   do {

      /* Calculate number of vertex indices in the next list */
      if(viptr) {
	 start_viptr = viptr;
	 while (*(++viptr) != (-1));   /* scan forward to find next sentinel */
	 fptr->vertex_count = viptr - start_viptr;
      } 
      else
	 fptr->vertex_count = numverts;


      /* Allocate space for them */
      ALLOC_RECORDS (fptr->vertex_indices, vertex_index, fptr->vertex_count);

      
      if (viptr)
	 /* Copy into the new memory */
	 COPY_BYTE (fptr->vertex_indices, start_viptr, 
		    vertex_index, fptr->vertex_count);
      else
	 /* For fill areas, generate dummy vertex index list */
	 for (i = 0; i < numverts; i++)
	    fptr->vertex_indices[i] = i;
	 

      /* Calculate normal */
      if (!FacetCheck (fptr, newpoly->vertex_list))
	 SPH__error (ERR_NONPLANAR_FACET);

      viptr++;   /* to move past sentinel to start of next list */
      fptr++;    /* to move to next facet struct in the array of facets */

   } while (--numfacets > 0);
   
   return (newpoly);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Copies a polyhedron structure
 * ------------------------------------------------------------------------- */
polyhedron *
SPH__copyPolyhedron (
   polyhedron *orig)
{
   register int         i;
   register polyhedron *clone;
   facet               *fclone, *forig;

   ALLOC_RECORDS (clone, polyhedron, 1);
   COPY_STRUCT (*clone, *orig);
   
   ALLOC_RECORDS (clone->vertex_list, MAT3hvec, orig->vertex_count);
   COPY_BYTE (clone->vertex_list, orig->vertex_list, 
	      MAT3hvec, orig->vertex_count);

   ALLOC_RECORDS (clone->facet_list, facet, orig->facet_count);
   COPY_BYTE (clone->facet_list, orig->facet_list, 
	      facet, orig->facet_count);

   fclone = clone->facet_list;
   forig =  orig->facet_list;
   for (i = 0;  i < clone->facet_count;  i++, fclone++, forig++) {
      ALLOC_RECORDS (fclone->vertex_indices,
		     vertex_index, 
		     forig->vertex_count);
      COPY_BYTE (fclone->vertex_indices, forig->vertex_indices,
		 vertex_index, 
		 forig->vertex_count);
   }
   
   return (clone);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Deallocates the associated vertex list.
 * ------------------------------------------------------------------------- */
void
SPH__freePolyhedron (
   polyhedron *poly)
{
   facet *fptr = poly->facet_list;
   int fcount = poly->facet_count;

   free (poly->vertex_list);

   while (fcount--) {
      free (fptr->vertex_indices);
      fptr = fptr++;
   }

   free (poly->facet_list);
   free (poly);
}


/* -------------------------------------------------------------------------
 * DESCR   :	Copies a single element and all of its substructures.
 * ------------------------------------------------------------------------- */
element *
SPH__copyElement(
   element *orig )
{
   extern structure *OPENSTRUCT; 	/* from sph_element.c */
   extern int ID_of_open_struct;

   if (!orig)
      return NULL;

   MAKE_BABY_ELEMENT;
   COPY_STRUCT (*baby, *orig);

   switch (baby->type) {

    case ELTYP__TEXT:
      baby->info.textstring = MAKE_COPY_OF_STRING (baby->info.textstring);
      break;
      
    case ELTYP__POLYHEDRON:
      baby->data.poly = SPH__copyPolyhedron(baby->data.poly);
      break;

    case ELTYP__POLYMARKER:
    case ELTYP__POLYLINE:
    case ELTYP__FILL_AREA:
      CopyPointList (baby, baby->info.count, baby->data.points);
      break;

    case ELTYP__EXECUTE_STRUCTURE:
      ++SPH__structureTable[baby->data.value].refcount;
      SPH__setBit (OPENSTRUCT->child_list, baby->data.value);
      VIEWOPT__newExecuteStructure (ID_of_open_struct, baby->data.value);
      break;

   }

   return baby;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Tests for descendent relationships.  Used to circumvent
 * 		recursive networks.
 * ------------------------------------------------------------------------- */
static boolean   
SPH__isDescendent (
   int childID,
   int parentID)
{
   structure *child  = &SPH__structureTable [childID];
   structure *parent = &SPH__structureTable [parentID];
   register int i;

   for (i=0; i < MAX_STRUCTURE_ID; i++)
      if (SPH__testBit (parent->child_list, i) &&
	  (childID == i || SPH__isDescendent (childID, i)))
	 return TRUE;

   return FALSE;
}

/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - name set Inclusion
 * ------------------------------------------------------------------------- */
void
SPH_addToNameSet (
   name name)
{
   SPH_check_name;

   SimpleElement (ELTYP__ADD_TO_NAME_SET, name-1);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - name set Inclusion
 * ------------------------------------------------------------------------- */
void
SPH_removeFromNameSet (
   name name)
{
   SPH_check_name;

   SimpleElement (ELTYP__REMOVE_FROM_NAME_SET, name-1);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Set Xform
 * ------------------------------------------------------------------------- */
void
SPH_setLocalTransformation (
   matrix mat,
   mode method)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_modxform_method;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__SET_MODXFORM;
   baby->info.update_type = method;
   MAT3copy (baby->data.matrix, mat);
   INSERT_BABY;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Set Xform
 * ------------------------------------------------------------------------- */
void
SPH_setModelingTransformation (
   matrix mat,
   int method)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_modxform_method;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__SET_MODXFORM;
   baby->info.update_type = method;
   MAT3copy (baby->data.matrix, mat);
   INSERT_BABY;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Clear Xform
 * ------------------------------------------------------------------------- */
/*!*/
void
SPH_clearModelingTransformation (void)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__CLEAR_MODXFORM;
   INSERT_BABY;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Text
 * ------------------------------------------------------------------------- */
void
SPH_text (
   point origin,
   char *string)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__TEXT;
   baby->info.textstring = MAKE_COPY_OF_STRING (string);
   MAT3_COPY_VEC (baby->data.point, origin);
   baby->data.point[3] = 1.0;   /* h-coordinate */
   INSERT_BABY;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Polyhedron
 * ------------------------------------------------------------------------- */
void
SPH_polyhedron (
   int   	 numverts, 
   int   	 numfacets,
   point 	*verts,
   vertex_index *facets)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_vertex_count(numverts, 3);
   SPH_check_facet_count(numfacets);
   SPH_check_vert_facet_list(verts);
   SPH_check_vert_facet_list(facets);

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__POLYHEDRON;
   StorePointList (baby, numverts, verts);
   baby->data.poly = SPH__newPolyhedron (numverts, numfacets, 
					 baby->data.points, facets);
   INSERT_BABY;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Polyline
 * ------------------------------------------------------------------------- */
void
SPH_polyLine (
   int vCount,
   point *verts)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_vertex_count(vCount, 2);
   SPH_check_vert_facet_list(verts);

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__POLYLINE;
   StorePointList (baby, vCount, verts);
   INSERT_BABY;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Polymarker
 * ------------------------------------------------------------------------- */
void
SPH_polyMarker (
   int vCount, 
   point *verts)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_vertex_count(vCount, 1);
   SPH_check_vert_facet_list(verts);

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__POLYMARKER;
   StorePointList (baby, vCount, verts);
   INSERT_BABY;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Fill Area
 * ------------------------------------------------------------------------- */
void
SPH_fillArea (
   int vCount,
   point *verts)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_vertex_count(vCount, 3);
   SPH_check_vert_facet_list(verts);

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__FILL_AREA;
   StorePointList (baby, vCount, verts);
   baby->data.poly = SPH__newPolyhedron (vCount, 1, baby->data.points, NULL);
   INSERT_BABY;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Label
 * ------------------------------------------------------------------------- */
void
SPH_label (
   int label)
{
   SimpleElement (ELTYP__LABEL, label);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Pick ID
 * ------------------------------------------------------------------------- */
void
SPH_setPickIdentifier (
   int label)
{
   SimpleElement (ELTYP__PICK_ID, label);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Facet Color Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setInteriorColor (
   int value)
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 0;  /* white is default color index for interiors */
   SimpleElement (ELTYP__SET_INTERIOR_COLOR,value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Line Color Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setLineColor (
   int value)
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 1;  /* black is default color index for framed objects */
   SimpleElement (ELTYP__SET_LINE_COLOR,value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Line Width Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setLineWidthScaleFactor (
   double value)
{
   register i;
   
   SimpleElement 
      (ELTYP__SET_LINE_WIDTH,
       ((i=LINE_WIDTH_UNIT_IN_PIXELS*value) < 1) ? 1 : i);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Line Style Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setLineStyle (
   lineStyle value)
{
   SimpleElement (ELTYP__SET_LINE_STYLE,value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Marker Color Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setMarkerColor (
   int value)
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 1;  /* black is default color index for framed objects */
   SimpleElement (ELTYP__SET_MARKER_COLOR,value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Marker Size Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setMarkerSizeScaleFactor (
   double value)
{
   register i;
   
   SimpleElement 
      (ELTYP__SET_MARKER_SIZE,
       ((i=MARKER_SIZE_UNIT_IN_PIXELS*value) < 1) ? 1 : i);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Marker Style Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setMarkerStyle (
   markerStyle value)
{
   SimpleElement (ELTYP__SET_MARKER_STYLE,value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Edge Color Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setEdgeColor (
   int value)
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 1;  /* black is default color index for framed objects */
   SimpleElement (ELTYP__SET_EDGE_COLOR,value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Edge Style Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setEdgeStyle (
   lineStyle value)
{
   SimpleElement (ELTYP__SET_EDGE_STYLE, value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Edge Flag Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setEdgeFlag (
   flag value)
{
   SimpleElement (ELTYP__SET_EDGE_FLAG, value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Edge Width Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setEdgeWidthScaleFactor (
   double value)
{
   register i;
   
   SimpleElement 
      (ELTYP__SET_EDGE_WIDTH,
       ((i=LINE_WIDTH_UNIT_IN_PIXELS*value) < 1) ? 1 : i);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Text Font Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setTextFont (
   int value)
{
   SimpleElement (ELTYP__SET_TEXT_FONT, value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Text Color Attribute
 * ------------------------------------------------------------------------- */
void
SPH_setTextColor (
   int value)
{
   if ( ! IS_LEGAL_COLOR_INDEX(value))
      value = 1;  /* black is default color index for framed objects */
   SimpleElement (ELTYP__SET_TEXT_COLOR, value);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Creates a new element - Nested Structure
 * ------------------------------------------------------------------------- */
void
SPH_executeStructure (
   int structID)
{
   register int sid;

   extern structure *OPENSTRUCT;        /** Borrows data from edit.c **/
   extern int ID_of_open_struct;

   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_structure_id;

   if (structID == ID_of_open_struct ||
       SPH__isDescendent (ID_of_open_struct, structID))
      SPH__error(ERR_RECURSIVE_NETWORK);

   MAKE_BABY_ELEMENT;
   baby->type = ELTYP__EXECUTE_STRUCTURE;
   baby->data.value = structID;
   INSERT_BABY;

   SPH__structureTable[structID].refcount++;

   /* UPDATE THE OPEN STRUCTURE'S CHILD_LIST. */
   SPH__setBit (OPENSTRUCT->child_list, structID);

   /* UPDATE VIEWS' DESCENDENT LISTS! */
   VIEWOPT__newExecuteStructure (ID_of_open_struct, structID);
}
