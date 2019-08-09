#ifndef lint
static char Version[]=
   "$Id: sph_elemdebug.c,v 1.1 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_elemdebug.c
 *
 *	Diagnostic output for structure newtorks
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "sphigslocal.h"


/* ------------------------------- Typedefs -------------------------------- */

typedef struct {
  char *line1, *line2;
} elem_descr;

#define NULL_DESCR	{ NULL, NULL }

typedef enum {
   XFORM_TRANS = 0x01,
   XFORM_ROT   = 0x02,
   XFORM_SCALE = 0x04,

   NUM_XFORM_TYPES = 3
} xform_type;


/* ----------------------------- String Tables ----------------------------- */

static 
elem_descr ELEMENT_TYPE_STR[] = {
                                    NULL_DESCR,

/* ADD_TO_NAME_SET      1 */        { "NAME SET", "ADD" },
/* REMOVE_FROM_NAME_SET 2 */        { "NAME SET", "REMOVE" },
/*  */          
/* POLYHEDRON           3 */        { "POLYHEDRON", NULL, },
/* POLYLINE             4 */        { "POLYLINE", NULL },
/* POLYMARKER           5 */        { "POLYMARKER", NULL },
/* FILL_AREA            6 */        { "FILL AREA", NULL },
/* TEXT                 7 */        { "TEXT", NULL },

                                    NULL_DESCR,
                                    NULL_DESCR,

/* SET_INTERIOR_COLOR  10 */        { "SET COLOR", "INTERIOR" },
/* SET_TEXT_FONT       11 */        { "SET FONT", "TEXT" },
/* SET_LINE_COLOR      12 */        { "SET COLOR", "LINE" },
/* SET_LINE_WIDTH      13 */        { "SET WIDTH", "LINE" },
/* SET_LINE_STYLE      14 */        { "SET STYLE", "LINE" },
/* SET_MARKER_COLOR    15 */        { "SET COLOR", "MARKER" },
/* SET_MARKER_SIZE     16 */        { "SET SIZE", "MARKER" },
/* SET_MARKER_STYLE    17 */        { "SET STYLE", "MARKER" },
/* SET_EDGE_COLOR      18 */        { "SET COLOR", "EDGE" },
/* SET_EDGE_WIDTH      19 */        { "SET WIDTH", "EDGE" },
/* SET_EDGE_STYLE      20 */        { "SET STYLE", "EDGE" },
/* SET_EDGE_FLAG       21 */        { "SET FLAG", "EDGE" },
/* SET_TEXT_COLOR      22 */        { "SET COLOR", "TEXT" },

/* SET_MODXFORM        23 */        { "XFORM", NULL },
/* EXECUTE_STRUCTURE   24 */        { "EXECUTE", NULL },
/* LABEL               25 */        { "LABEL", NULL },
/* PICK_ID             26 */        { "PICK ID", NULL },
/* CLEAR_MODXFORM      27 */        { "XFORM", "CLEAR" },

				    NULL_DESCR
};


static char *STYLE_STR[] = {
   "continuous", "dashed", "dotted", "dot-dashed"
};

static char *MARKER_STR[] =  {
   "circle", "square", "cross"
};

static char *XFORM_MOD_STR[] = {
   NULL, "assign", "pre", "post"
};

static char *XFORM_TYPE_STR[] = {
   /* 000 */ "(???)",
   /* 001 */ "TRANS",
   /* 010 */ "ROT",
   /* 011 */ "T-R",
   /* 100 */ "SCALE",
   /* 101 */ "T-S",
   /* 110 */ "R-S",
   /* 111 */ "T-S-R",
	     NULL
};

/* ----------------------------- Static Globals ---------------------------- */

void SPH_displayStructureNetwork (int viewIndex,
				  char *struct_names[], char *label_names[],
				  char *pickid_names[], char *nameset_names[]);

extern structure *OPENSTRUCT;
extern int        element_ptr_index;
extern element   *element_ptr;   

static rectangle elemRect;	/* box for an element */
static int             textSpacing;	/* offset tween lines of text */
static srgp__point     elemStart;	/* start position for elements */
static srgp__point     elemSpacing;	/* x/y offsets tween element boxes */
static srgp__point     elemBorder;	/* border around all element boxes */
static srgp__point     elemGrid;	/* number of x/y positions */

#define ELEM_POSITION(IDX)						\
   SRGP_defPoint (							\
      ((IDX) / elemGrid.x) % 2 ?					\
      elemStart.x + elemSpacing.x * (elemGrid.x - ((IDX) % elemGrid.x) - 1) :\
      elemStart.x + elemSpacing.x * ((IDX) % elemGrid.x),		\
      elemStart.y - elemSpacing.y * ((IDX) / elemGrid.x))

/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Attempts to identify a matrix's function by examining
 * 		row/column values.
 * ------------------------------------------------------------------------- */
static int
MatrixType (
   matrix mat)
{
   register int i, j;
   int type = 0;

   /* entries in last column indicate translation */
   if (!MAT3_IS_ZERO (mat[0][3]) ||
       !MAT3_IS_ZERO (mat[1][3]) ||
       !MAT3_IS_ZERO (mat[2][3]))

      type |= XFORM_TRANS;


   /* entries down diagonal indicate scale */
   if (!MAT3__EQ (mat[0][0], 1.0) ||
       !MAT3__EQ (mat[1][1], 1.0) ||
       !MAT3__EQ (mat[1][1], 1.0))

      type |= XFORM_SCALE;

   
   /* entries in top-left 3x3 portion of matrix, not along diagonal, indicate
      rotation */
   for (i=0; i<2; i++)
      for (j=0; j<2; j++)
	 if (i!=j && !MAT3_IS_ZERO (mat[i][j]))
	 {
	    type |= XFORM_ROT;
	    break;
	 }

   return type;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Outputs text centered at the given coordinate
 * ------------------------------------------------------------------------- */
static void
CenterText (
   srgp__point pt,
   char       *text)
{
   int width, height, dummy;

   if (!text)
      return;

   SRGP_inquireTextExtent (text, &width, &height, &dummy);

   pt.x -= width / 2;
   pt.y -= height / 2;
   SRGP_text (pt, text);
}

/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	These routines offset geometric struxtures by given amounts
 * 		in the x and y directions.
 * ------------------------------------------------------------------------- */
static srgp__point
SPH__offsetPoint (
   srgp__point target,
   srgp__point offset)
{
   target.x += offset.x;
   target.y += offset.y;

   return target;
}

static rectangle
SPH__offsetRect (
   rectangle target,
   srgp__point 	   offset)
{
   target.bottom_left.x += offset.x;
   target.bottom_left.y += offset.y;
   target.top_right.x += offset.x;
   target.top_right.y += offset.y;

   return target;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Outputs a graphical description of a single structure
 * 		element. 
 * ------------------------------------------------------------------------- */
static void
DisplayElement (
   element     *elem,
   srgp__point  center,
   char        *struct_names[],
   char        *label_names[],
   char        *pickid_names[],
   char        *nameset_names[])
{
   srgp__point text_pt = center;
   char        buf[256];

   SRGP_setColor (SRGP_WHITE);
   SRGP_fillRectangle (SPH__offsetRect (elemRect, center));
   SRGP_setColor (SRGP_BLACK);
   SRGP_rectangle (SPH__offsetRect (elemRect, center));

#define PRINT_ID_STR(VAL, TABLE)     					\
      if (TABLE)	 sprintf (buf, "(%.10s)", (TABLE)[VAL]);	\
      else		 sprintf (buf, "(%d)", (VAL));	 


   /* LINE 1 */
   text_pt.y += textSpacing;
   CenterText (text_pt, ELEMENT_TYPE_STR [elem->type].line1);


   /* LINE 2 */
   text_pt.y -= textSpacing;
   switch (elem->type) {
    case ELTYP__POLYHEDRON:
      sprintf (buf, "(%d facets,", elem->data.poly->facet_count);
      CenterText (text_pt, buf);
      break;

    case ELTYP__SET_MODXFORM:
      sprintf (buf, "%s", XFORM_TYPE_STR [MatrixType (elem->data.matrix)]);
      CenterText (text_pt, buf);
      break;

    default:
      CenterText (text_pt, ELEMENT_TYPE_STR [elem->type].line2);
      break;
   }


   /* LINE 3 */
   text_pt.y -= textSpacing;
   buf[0] = (char) NULL;

   switch (elem->type) {
    case ELTYP__ADD_TO_NAME_SET:
    case ELTYP__REMOVE_FROM_NAME_SET:
      PRINT_ID_STR (elem->data.value, nameset_names);
      break;

    case ELTYP__TEXT:
      if (strlen (elem->info.textstring) <= 9)
	 sprintf (buf, "\"%s\"", elem->info.textstring);
      else
	 sprintf (buf, "\"%.7s..\"", elem->info.textstring);
      break;

    case ELTYP__POLYHEDRON:
      sprintf (buf, "%d verts)", elem->data.poly->vertex_count);
      break;

    case ELTYP__POLYLINE:
    case ELTYP__POLYMARKER:
    case ELTYP__FILL_AREA:
      sprintf (buf, "(%d verts)", elem->info.count);
      break;

    case ELTYP__SET_INTERIOR_COLOR:
    case ELTYP__SET_TEXT_COLOR:
    case ELTYP__SET_EDGE_COLOR:
    case ELTYP__SET_MARKER_COLOR:
    case ELTYP__SET_LINE_COLOR:
      sprintf (buf, "(%d)", elem->data.value);
      break;

    case ELTYP__SET_TEXT_FONT:
      sprintf (buf, "(%d)", elem->data.value);
      break;

    case ELTYP__SET_LINE_WIDTH:
    case ELTYP__SET_MARKER_SIZE:
    case ELTYP__SET_EDGE_WIDTH:
      sprintf (buf, "(%d)", elem->data.value);
      break;

    case ELTYP__SET_EDGE_STYLE:
    case ELTYP__SET_LINE_STYLE:
      sprintf (buf, "(%s)", STYLE_STR [elem->data.value]);
      break;

    case ELTYP__SET_MARKER_STYLE:
      sprintf (buf, "(%s)", MARKER_STR [elem->data.value]);
      break;

    case ELTYP__SET_EDGE_FLAG:
      sprintf (buf, "(%s)", elem->data.value ? "on" : "off");
      break;

    case ELTYP__SET_MODXFORM:
      sprintf (buf, "(%s)", XFORM_MOD_STR [elem->info.update_type]);
      break;

    case ELTYP__EXECUTE_STRUCTURE:
      PRINT_ID_STR (elem->data.value, struct_names);
      break;

    case ELTYP__LABEL:
      PRINT_ID_STR (elem->data.value, label_names);
      break;

    case ELTYP__PICK_ID:
      PRINT_ID_STR (elem->data.value, pickid_names);
      break;

    case ELTYP__CLEAR_MODXFORM:
      break;
   }
   if (*buf)
      CenterText (text_pt, buf);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Called prior to graphic output, this routine calculates some
 * 		spacing constants.
 * ------------------------------------------------------------------------- */
static void
InitSpacing (
   int viewId)
{
   int width, height, dummy;
   view_spec *vs = &SPH__viewTable[viewId];

   SRGP_inquireTextExtent ("WWWWWWWWWWW", &width, &height, &dummy);
   elemRect = SRGP_defRectangle (-(width / 2), - (height*5 / 2),
				   width / 2,     height*5 / 2);
   textSpacing = height * 3 / 2;

   elemBorder.x = elemRect.top_right.x - elemRect.bottom_left.x;
   elemBorder.y = elemRect.top_right.y - elemRect.bottom_left.y;

   elemSpacing = elemBorder;
   elemSpacing.x *= 3;   elemSpacing.y *= 3;
   elemSpacing.x /= 2;   elemSpacing.y /= 2;

   elemStart.y = vs->pdc_viewport.top_right.y - elemBorder.y;
   elemStart.x = vs->pdc_viewport.bottom_left.x + elemBorder.x;

   elemGrid.y = vs->pdc_viewport.top_right.y - vs->pdc_viewport.bottom_left.y;
   elemGrid.x = vs->pdc_viewport.top_right.x - vs->pdc_viewport.bottom_left.x;
   elemGrid.y = (elemGrid.y - 2 * elemBorder.y) / elemSpacing.y + 1;
   elemGrid.x = (elemGrid.x - 2 * elemBorder.x) / elemSpacing.x + 1;

   SRGP_setClipRectangle (vs->pdc_viewport);
   SRGP_setColor (vs->background_color);
   SRGP_fillRectangle (vs->pdc_viewport);
}

/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Uses the entire viewport to display the opened structure 
 * 	        network.  The current element pointer is positioned at the
 * 		center of the viewport. 
 * ------------------------------------------------------------------------- */
void
SPH_displayStructureNetwork (
   int   viewIndex,
   char *struct_names[],
   char *label_names[],
   char *pickid_names[],
   char *nameset_names[])
{
   attributeGroup attr;
   register int      i;
   register element *cur_elem;
   srgp__point p1, p2;

   SPH_check_open_structure;
   SPH_check_view_index;

   SRGP_inquireAttributes (&attr);

   InitSpacing (viewIndex);


#  define DISPLAY_CURRENT_ELEMENT_PTR(INDEX, MARKER_TYPE)		\
   {  p1 = ELEM_POSITION ((INDEX));					\
      p2 = ELEM_POSITION ((INDEX) - 1);					\
      SRGP_setColor (SRGP_BLACK);					\
      SRGP_setMarkerStyle (MARKER_TYPE);				\
      SRGP_marker (SRGP_defPoint ((p1.x+p2.x) / 2, (p1.y+p2.y) / 2)); }


   for (cur_elem = OPENSTRUCT->first_element, i = 0;
	cur_elem;
	cur_elem = cur_elem->next,	      i++)
   {
      if (element_ptr == cur_elem)
	 DISPLAY_CURRENT_ELEMENT_PTR(i+1, MARKER_X);

      if (cur_elem->next) {
	 SRGP_setColor (SRGP_BLACK);
	 SRGP_line (ELEM_POSITION (i), ELEM_POSITION (i+1));
      }

      DisplayElement (cur_elem, ELEM_POSITION (i),
		      struct_names, label_names, pickid_names, nameset_names);
   }
   if (!element_ptr)
      DISPLAY_CURRENT_ELEMENT_PTR(0, MARKER_X);

   /* display current element pointer */
   DISPLAY_CURRENT_ELEMENT_PTR(element_ptr_index, MARKER_CIRCLE);
   SRGP_refresh();

   SRGP_setAttributes (&attr);
}

