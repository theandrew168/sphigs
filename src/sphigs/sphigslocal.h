/* $Id: sphigslocal.h,v 1.5 1993/03/09 02:00:54 crb Exp crb $ */

#ifndef SPHIGSLOCAL_H_INCLUDED_ALREADY
#define SPHIGSLOCAL_H_INCLUDED_ALREADY

/* -------------------------------------------------------------------------
		        Internal SPHIGS include file
   ------------------------------------------------------------------------- */

#define REPORT_ERROR  SPH__error

#include "mat3defs.h"
#include "macros.h"
#include "sphigs.h"
#include "geom.h"
#include "sph_errtypes.h"
#include "falloc.h"
#include "assert.h"

#ifdef SPHIGS_BOSS
#define DECLARE
#else
#define DECLARE extern
#endif


/* -----------------------------  Constants  ------------------------------ */

#define X 	0			/* for indexing */
#define Y 	1
#define Z 	2

#define MAX_VERTS_PER_OBJECT	 200   	/* chunk size for vertex lists */
#define MAX_EDGES_PER_OBJECT	 200   	/* chunk size for edge bitstrings */


/* Device-Dependent Measurements
 * Add one for your own screen if not already listed here.
 */
#ifdef sparc
#define MARKER_SIZE_UNIT_IN_PIXELS  10  /* unit size geared towards Sparcstn */
#define LINE_WIDTH_UNIT_IN_PIXELS    2
#endif
#ifdef THINK_C
#define MARKER_SIZE_UNIT_IN_PIXELS   6  /* unit size geared towards MacII */
#define LINE_WIDTH_UNIT_IN_PIXELS    1
#endif

/* a default in case you don't provide on for your machine */
#ifndef MARKER_SIZE_UNIT_IN_PIXELS
#define MARKER_SIZE_UNIT_IN_PIXELS   6  /* I assume big pixels */
#define LINE_WIDTH_UNIT_IN_PIXELS    1
#endif

/* -------------------------------  Types  -------------------------------- */

/* MISCELLANEOUS */

#define matrix matrix_4x4
#define vector vector_3D
typedef int            pdc_point[3];
typedef unsigned char  Bbyte;  /* to avoid collision with THINK C's "Byte" */

/* ------------------------------------ *
 * SUBSTRUCT_BITSTRING
 *
 * Each structure uses one of these to keep track of which structures
 * are subordinates of it.
 *
 * It allows a string of N bits to be cleared, set, and tested.
 *   BYTE [0] stores bits 0 through 7
 *   BYTE [1] stores bits 8 through 15
 *
 * Each byte stores its 8 bits in right-to-left order, so the bit with
 * the lowest number is stored in the one's place, the bit with the
 * highest number is stored in the 2^7 place. 
 *
 * Below is a picture of the first byte of a bitstring:
 *
 *		7   6   5   4   3   2   1   0
 *		-----------------------------
 *
 * The bitstring (shown in hex)   03 82 00 00 00 00
 *
 * thus says that bits 0, 1, 9, and 15 are ON.
 */
typedef unsigned char 
   *substruct_bitstring,  /* semi-variable length - for structure hierarchy */
   *edge_bitstring,	  /* variable length  - for polygon edge rendering */
   *nameset;	  	  /* variable length  - for name sets and filters */

extern void    SPH__clearBitstring (substruct_bitstring*);
extern boolean SPH__bitstringIsClear (substruct_bitstring);

#define SPH__allocNBitstring(B,N)      ALLOC_RECORDS  ((B), unsigned char, ((N)+7) >> 3)
#define SPH__fallocNBitstring(B,N)     FALLOC_RECORDS ((B), unsigned char, ((N)+7) >> 3)
#define SPH__freeBitstring(B)          FREE(B)
#define SPH__clearNBitstring(B,N)      (bzero(B, ((N)+7) >> 3))
#define SPH__copyNBitstring(Bd,Bs,N)   (bcopy(Bs,Bd,((N)+7) >> 3))

#define SPH__copyBitstring(Bd,Bs)      (bcopy(Bs,Bd,BYTES_PER_BITSTRING))
#define SPH__setBit(B,N)	       (B[N>>3] |= ((char)1)<<(N&7))
#define SPH__clearBit(B,N)	       (B[N>>3] &= ( ! ( ((char)1)<<(N&7))))
#define SPH__testBit(B,N)	       (B[N>>3] &  ((char)1)<<(N&7))
#define SPH__bitstringsAreEqual(B1,B2) (!(bcmp(B1,B2,BYTES_PER_BITSTRING)))

#define SPH__mergeBitstring(Bdest,Bsrc) { 	\
   register int i;				\
   for (i=0; i<BYTES_PER_BITSTRING; i++)	\
      Bdest[i] |= Bsrc[i];			\
}
#define SPH__andBitstrings(Bdest,B1,B2) {	\
   register int i;				\
   for (i=0; i<BYTES_PER_BITSTRING; i++)	\
      Bdest[i] = B1[i] & B2[i];			\
}

#define SPH__overlapNBitstrings(result,B1,B2,N) {	\
   register int i;					\
   result = 0;						\
   for (i=0; i < (((N)+7) >> 3); i++)			\
      result |= B1[i] & B2[i];				\
}


/* ------------------------------------ *
 * INTELLIGENT RECTANGLE - unused!
 *
 * A rectangle which may be either empty (meaning non-existent) or nonempty.
 * An empty rectangle is used to, for example, initialize a data item like
 * the extent of a group of prims: before any primitives are drawn, the
 * extent is of course "non-existent".
 */
typedef struct {
   boolean nonempty;
   rectangle rect;
} intelligent_rectangle;       


/* ------------------------------------ *
 * ATTRIBUTE GROUP
 *
 * Only exists during traversal.
 * Each structure inherits its parent's attribute group as a value-parameter.
 * Some of the attributes stored here are already transformed into SRGP
 *   "pixel-units" versions (like linewidth, markersize).
 */

typedef enum {
   ATTRIB_LINE  	= (1 << 0),
   ATTRIB_MARKER	= (1 << 1),
   ATTRIB_EDGE		= (1 << 2),
   ATTRIB_FACE		= (1 << 3),
   ATTRIB_TEXT		= (1 << 4)
} attributeFlags;	/* used by SPH__set_attributes */

typedef struct {

   /*.... line */
   Bbyte line_color;
   Bbyte line_width;
   Bbyte line_style;

   /*.... marker */
   Bbyte marker_color;
   Bbyte marker_size;
   Bbyte marker_style;

   /*.... polyhedra and fill area */
   Bbyte interior_color;
   Bbyte edge_width;
   Bbyte edge_style;
   Bbyte edge_flag;
   Bbyte edge_color;

   /*.... text */
   Bbyte text_color;
   Bbyte font;

} attribute_group;

#include "sph_face.h"


/* Each face-set given to the painter-alg must carry with it its attributes */
typedef struct {
   attribute_group attrs;
} sphigs_info_struct;


/* ------------------------------------ *
 * FACET
 */
typedef struct {
   vertex_index  *vertex_indices;   	/* vertex_index defined in sphigs.h */
   Bbyte      	  vertex_count;
   MAT3hvec 	  normal;
   boolean 	  do_draw;
} facet;


/* ------------------------------------ *
 * POLYHEDRON
 */
typedef struct {
   int       vertex_count;
   MAT3hvec *vertex_list;  /* POINTS TO ARRAY OF 4-element vectors */
   int       facet_count;
   facet    *facet_list;   /* POINTS TO ARRAY OF FACET STRUCTURES */
} polyhedron;


/* ------------------------------------ *
 * ELEMENT
 *
 * An element's type field determines which subfields of its two union fields 
 * are active.
 */
typedef struct element {
   short type;               /* enumerated values begin with ELTYP__ */	
   union {
      int   count;	     /* used by poly- elements, fill area */
      char *textstring;      /* used only by text element */
      int   update_type;     /* used only by setModXform element */
   } info;
   union {
      int         value;
      MAT3hvec    point;
      MAT3hvec   *points;    /* ptr to dynamically allocated space */
      polyhedron *poly;      /* ptr to dynamically allocated space */
      matrix_4x4 matrix;
   } data;
   struct element *next, *previous;
} element;


/* ------------------------------------ *
 * STRUCTURE
 *
 * A child of structure S is a structure invoked DIRECTLY by structure S.
 * A descendent is a structure invoked indirectly by structure S.
 * The refcount tells us how many times the structure is DIRECTLY INVOKED
 *    anywhere in the Universe.  Each posting of structure S counts for
 *    one "ref"; and each execute-structure-S counts for one "ref".
 */
typedef struct {
   short               refcount;
   short               element_count;
   short               last_valid_index;	/* for BSP regeneration */
   element            *first_element;
   element            *last_element;
   substruct_bitstring child_list;
} structure;


/* ------------------------------------ *
 * ROOT HEADER
 *
 * Representation of a network; the structure ID of the root of this
 * network is stored.
 */
typedef struct root_header {
   int                 root_structID;
   struct root_header *nextHigherOverlapRoot;
   struct root_header *nextLowerOverlapRoot;
} root_header;


/* ------------------------------------ *
 * LIGHT SOURCE 
 */

typedef struct light_source {
   unsigned active 	   : 1;		/* - is light active? */
   unsigned cameraRelative : 1;		/* - is position relative to camera? */
   unsigned attenuate      : 1;		/* - use attenuation? */
   unsigned 		   : 13;
   MAT3hvec position;			/* - position (in modeling coords or */
					/* 	relative to the camera) */
   MAT3hvec uvnPosition;		/* - position used in intensity */
					/* 	calculations */
   double   intensity;			/* - light intensity */
   double   attenFactor;		/* - attenuation factor */
} light_source;

/* ------------------------------------ *
 * VIEW TRANSFORMATION TABLE
 *
 * The list of objects stored in the view table is updated each time the
 * view's roots are traversed for display.  So it does not necessarily
 * represent the current state of the networks posted to the view,
 * especially if implicit regeneration has been disabled.
 *
 * The stored list of objects is used for pick correlation, and for
 * optimized screen regeneration.
 */

typedef struct view_spec {
                      				/* View Representation: */
   NDC_rectangle       viewport;		/* - viewport rect */
   rectangle           pdc_viewport;   		/* - in pixel coords */
   MAT3hvec	       prp;			/* - viewpoint for BSP */
   matrix              vo_matrix;		/* - view orientation matx */
   matrix	       vm_matrix;		/* - view mapping matx */
   matrix	       cammat;			/* - composite mtx to pixels */
   double 	       frontPlaneDistance;	/* - clip distances: front */
   double	       backPlaneDistance;	/*		     back */


   /*      Vertices for rendered objects are all stored in a list global
    * to each viewport to facilitate coordinate conversions.  For BSP
    * rendering, we would like a tree which is view independent.  For the
    * clipping step that follows, we need to add new vertices for
    * polygons and lines that intersect the clipping planes.  The
    * 'vertexCutoff' field below contains the highest index of the BSP
    * vertices, before clipping begins.  All vertices below this index
    * can be reused for other view vantages.
    */
						/* Object Vertices: */
   int		       vertexArraySize;		/* - alloc'ed # of vertices */
   int 		       vertexCount;		/* - real # of vertices */
   int 		       vertexCutoff;		/* - above this #, all verts 
						     are temporary (BSP) */
   MAT3hvec 	      *mcVertices;		/* - in modeling coords */
   MAT3hvec 	      *uvnVertices;		/* - after view orient */
   MAT3hvec	      *npcVertices;		/* - after view mapping */
   srgp__point 	      *pdcVertices;		/* - after device mapping */

						/* Render Objects: */
   obj		      *objects;			/* - head of list */
   obj		      *objectTail;		/* - tail of list */
   FALLOCchunk 	      *objectChunk;		/* - allocation object */
   obj 		      *objectTree;		/* - root of BSP tree */
   obj 		      *textObjects;		/* - separate list (BSP) */

						/* Structure Networks: */
   root_header        *highestOverlapNetwork;	/* - head of list */
   root_header	      *lowestOverlapNetwork;	/* - tail of list */

						/* Misc: */
   char                rendermode;
   int		       algorithm;
   light_source	      *lights;
   short               background_color;  	/* - default is WHITE */

   nameset	       invisFilter;
   nameset	       hiliteFilter;

   unsigned short      curTraversalIndex;
   substruct_bitstring descendent_list;

   boolean             obsolete_objects;	/* chanegd object list */
   boolean             obsolete_camera;		/* changed view repres. */
   boolean             obsolete_render;		/* changed lighting/rendering */
   boolean             currently_disabled;


} view_spec;


/* -------------------------------  Globals  ------------------------------ */


DECLARE int NUM_OF_FLEXICOLORS;
DECLARE int NUM_OF_SHADES_PER_FLEXICOLOR;
DECLARE int BASE_OF_SHADE_LUT_ENTRIES;
DECLARE int BYTES_PER_BITSTRING;


#define SPH_viewTable SPH__viewTable
DECLARE view_spec *SPH__viewTable;
DECLARE structure *SPH__structureTable;
DECLARE int        SPH__implicitRegenerationMode;
DECLARE int        SPH__ndcSpaceSizeInPixels;
DECLARE boolean    SPH__enabled
#ifdef SPHIGS_BOSS
   = FALSE
#endif
;
DECLARE boolean	   SPH__structureCurrentlyOpen
#ifdef SPHIGS_BOSS
   = FALSE
#endif
;

/* used during display traversal */
DECLARE nameset    currentNameset;
DECLARE int        currentRendermode;
DECLARE int        currentViewIndex;
DECLARE view_spec *currentViewSpec;
DECLARE matrix     currentCompositeModxform; 
DECLARE matrix     currentMCtoUVNxform;
DECLARE matrix     currentNormalMCtoUVNxform;
DECLARE matrix     currentTOTALxform;


/* -------------------------------  Macros  ------------------------------- */


/*
 * Color Table Macros 
 */

#define NUM_OF_APPL_SETTABLE_COLORS  (BASE_OF_SHADE_LUT_ENTRIES-2)

#define IS_A_FLEXICOLORINDEX(i)    \
   ( (i > 1) && (i < (2 + NUM_OF_FLEXICOLORS)) )

#define IS_LEGAL_COLOR_INDEX(C)         \
   ( (C>=0) && (C<BASE_OF_SHADE_LUT_ENTRIES) )

#define IS_CHANGEABLE_COLOR_INDEX(C)         \
   ( (C>=2) && (C<BASE_OF_SHADE_LUT_ENTRIES) )


/*
 * Input Checks
 */

#define SPH_check_system_state	\
   if (!SPH__enabled) SPH__error(ERR_NOT_ENABLED)

#define SPH_check_max_id		\
   if (id < 1) SPH__error(ERR_INVALID_MAX, id)

#define SPH_check_no_system_state	\
   if (SPH__enabled) SPH__error(ERR_ALREADY_ENABLED)

#define SPH_check_rectangle(LX,BY,RX,TY)		\
   if(!((LX<=RX)&&(BY<=TY)))  SPH__error(ERR_BAD_RECT)

#define SPH_check_no_open_structure	\
   if (SPH__structureCurrentlyOpen) SPH__error(ERR_STRUCTURE_IS_OPEN)

#define SPH_check_open_structure	\
   if ( ! SPH__structureCurrentlyOpen) SPH__error(ERR_NO_STRUCTURE_IS_OPEN)

#define SPH_check_structure_id	\
   if ((structID<0) || (structID>MAX_STRUCTURE_ID)) SPH__error(ERR_STRUCTURE_ID)

#define SPH_check_name	\
   if ((name<1) || (name>MAX_NAME)) SPH__error(ERR_NAME)

#define SPH_check_view_index	\
   if ((viewIndex<0) || (viewIndex>MAX_VIEW_INDEX)) SPH__error(ERR_VIEW_INDEX)

#define SPH_check_light_index			       \
   if ((lightIndex<0) || (lightIndex>MAX_LIGHT_INDEX)) \
      SPH__error(ERR_LIGHT_INDEX)

#define SPH_check_vertex_count(C, MIN)	\
   if (C<MIN) SPH__error(ERR_VERTEX_COUNT, C)

#define SPH_check_rendering_mode(M)	\
   if ((M) < WIREFRAME_RAW || (M) > LIT_FLAT) SPH__error(ERR_RENDER_MODE)

#define SPH_check_facet_count(C)	\
   if (C<1) SPH__error(ERR_FACET_COUNT, C)

#define SPH_check_vert_facet_list(L)	\
   if (!L) SPH__error(ERR_NULL_LIST)

#define SPH_check_modxform_method	\
   if ((method<REPLACE) || (method>POSTCONCATENATE)) SPH__error(ERR_METHOD)

#define SPH_check_elindex_range         \
   if ( ! (first_index<=second_index)) SPH__error(ERR_BAD_ELEMENT_INDEX_RANGE)


/*
 * Allocation Macro Used By Matrix Package
 */

#define COPY_BYTE(TO, FROM, TYPE, NUM)	bcopy(FROM, TO, (NUM) * sizeof(TYPE))
#define COPY_STRUCT(TO, FROM)		bcopy(&(FROM), &(TO), sizeof (TO))

/* -----------------------------  Prototypes  ----------------------------- */

/* sph_attrib.c */
void         SPH__initColorTable (int num_of_planes);

/* sph_bsp.c */
void         SPH__bsp_sort (view_spec *vs);
void         SPH__bsp_traverse (view_spec *vs);

/* sph_canon.c */
void         SPH__map_to_camera (view_spec *vs);
void         SPH__map_to_canon (view_spec *vs);
void         SPH__map_to_pdc (view_spec *vs);

/* sph_clip.c */
void	     OBJECT__clip (view_spec *vs, register obj *current);

/* sph_cull.c */
void	     OBJECT__cull (view_spec *vs, register obj *current, boolean);


/* sph_draw.c */
void         SPH__set_attributes (attribute_group *, int flags);
void         SPH__draw_polyhedron (polyhedron*, attribute_group*);
void         SPH__draw_fill_area (polyhedron*, attribute_group*);
void         SPH__draw_lines (element*, attribute_group*);
void         SPH__draw_markers (element*, attribute_group*);
void         SPH__draw_text (point, char*, attribute_group*);

/* sph_edit.c */
void         SPH__init_structure_table (void);
void         SPH__insertElement (element *baby);

/* sph_element.c */
element *    SPH__copyElement (element *original);
polyhedron * SPH__newPolyhedron (int, int, MAT3hvec *, vertex_index *);
polyhedron * SPH__copyPolyhedron (polyhedron *poly);
void 	     SPH__freePolyhedron (polyhedron *poly);

/* sph_intense.c */
void	     OBJECT__intensity (view_spec *vs, register obj *current);

/* sph_objdebug.c */
void         SPH__print_objects_in_view (view_spec *vs);


/* sph_object.c */
void         OBJECT__init (view_spec *vs);
void         OBJECT__addToViewSpec (view_spec *, obj *);
void         OBJECT__addPoly (view_spec *, polyhedron *, 
			      boolean doubleSided, matrix xform,
                	      attribute_group *attrs);
void         OBJECT__addLines (view_spec *, int vCount, MAT3hvec *verts,
			       matrix xform, attribute_group *attrs);
void         OBJECT__addPoints (view_spec *, int vCount, MAT3hvec *verts,
			       matrix xform, attribute_group *attrs);
void         OBJECT__addText (view_spec *, MAT3hvec origin, char *text,
			       matrix xform, attribute_group *attrs);
obj *        OBJECT__copy (view_spec *, obj *);
void         OBJECT__draw (view_spec *vs, obj *);
void         OBJECT__drawAll (view_spec *vs);
void 	     OBJECT__get_vertex_indices (obj *curObj,
					 vertex_index **indices, int *count);
void         OBJECT__filter (view_spec *vs, register obj *);
void         OBJECT__bound (register MAT3hvec*, register obj *);
void         OBJECT__process (view_spec *vs);
vertex_index SPH__add_global_point(view_spec *vs, MAT3hvec p, int);

/* sph_optimize.c */
void         VIEWOPT__newExecuteStructure (int ID_of_openStruct, int structID);
void         VIEWOPT__afterChildLoss (int ID_of_open_struct);
void         VIEWOPT__afterNewPosting (view_spec *v, int struct_ID);
void         VIEWOPT__afterUnposting (view_spec *v, int struct_ID);

/* sph_refresh.c */
void         SPH__repaintScreen (void);
void         SPH__refresh_post (int viewIndex);
void         SPH__refresh_filter (int viewIndex);
void         SPH__refresh_lights (int viewIndex);
void         SPH__refresh_unpost (int viewIndex);
void         SPH__refresh_structure_close (int structID);
void         SPH__refresh_viewport_change (int viewIndex,
                			   rectangle old_viewport_rect);

/* sph_traverse.c */
void	     SPH__computeNormalTransformer (void);
void         SPH__initDefaultAttributeGroup (void);
void         SPH__traverse_network_for_display (view_spec *, root_header *);
int          SPH__traverse_struct_for_bsp_preprocess (int structID);
void         SPH__traverse_struct_for_display (int structID, attribute_group*);

/* sph_view.c */             
void         SPH__init_view_table (void);
void         SPH__updateViewInfo (int viewIndex);
void         SPH__viewPrepareLights (view_spec *);

/* sph_zsort.c */
void         SPH__zsort (view_spec *vs);
   
#ifdef THINK_C
/* We hide this from gnu's compiler, which doesn't understand it. */
void SPH__error (...);
#endif

#endif /* SPHIGSLOCAL_H_INCLUDED_ALREADY */
