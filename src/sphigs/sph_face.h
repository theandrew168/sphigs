/* $Id: sph_face.h,v 1.5 1993/03/09 02:00:54 crb Exp $ */

/* -------------------------------------------------------------------------
 * 			Display objects for SPHIGS
 *
 * All rendered structure elements get boiled down to these display primitives.
 * Used by SPH__calc_intensity(), SPH__clip(), SPH__cull(), SPH__map_to_canon(),
 * SPH__zsort(), SPH__generate_pdc_vertices().
 * ------------------------------------------------------------------------- */

/* -------------------------------  Types  -------------------------------- */

typedef enum {				/* Display Types: */
   OBJ_NULL =0, 			/* - dummy plane (not rendered) */
   OBJ_FACE,    			/* - face */
   OBJ_LINE, 				/* - line */
   OBJ_TEXT,				/* - text */
   OBJ_POINT				/* - point */
} objType;


typedef struct _typeFace {		/* Face Specific Fields: */
   int	          numPoints; 	  	/* - need minimum 3 points */
   vertex_index  *points;		/* - indices to view vertices */
   edge_bitstring edgeFlags;		/* - edge visibility flags */
   unsigned	  doubleSided : 1;	/* - true for fillAreas */
   unsigned	              : 15;
} typeFace;

typedef struct _typeLine {		/* Line Specific Fields: */
   vertex_index pt[2];			/* - endpoints */
} typeLine;

typedef struct _typeText {		/* Text Specific Fields: */
   vertex_index start;			/* - start location */
   char        *text;			/* - text string */
} typeText;

typedef struct _typePoint {		/* Point Specific Fields: */
   vertex_index pt;			/* - point location */
} typePoint;


typedef struct obj obj;
struct obj {	    	 		/* Display Object: */
   objType		type;		/* - object type */
   MAT3hvec 		normal;		/* - passed for face */
   double	        directed_distance; 
   MAT3hvec		min;     	/* - bounding cube: min */
   MAT3hvec		max;		/* 		    max */
   MAT3hvec		center;		/* - for intensity (in MC) */
   double		intensity;  	/* - color intensity */
   attribute_group 	attributes;	/* - attribute set */

   union {			
      typeFace	face;
      typeLine	line;
      typeText	text;
      typePoint	point;
   } data;				/* - object-specific data */

   MAT3hvec	     p_min;		/* - for pick correlation */
   MAT3hvec	     p_max;
   MAT3hvec	     p_normal;
   unsigned short    traversal_index; 	/* - used for pick correlation */

   nameset	     names;

   unsigned	     visible        : 1;
   unsigned	     hilite         : 1;
   unsigned	     already_moved  : 1;
   unsigned		            : 13;

   /* The object to display; if unclipped, this is the object itself; if the
      object is completely clipped, this equals NULL; otherwise, this
      points to a clipped copy of the object. */      
   obj *display;

   /* Next node in list of objects in viewport (valid for BSP and painter's
      algorithm  */
   obj *next;   			

   /* BSP tree children (unused by painter's algorithm) */
   obj *front;
   obj *back;   			
};
