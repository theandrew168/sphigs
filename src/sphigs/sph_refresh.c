#ifndef lint
static char Version[]=
   "$Id: sph_refresh.c,v 1.5 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_refresh.c
 *
 *	Code for refreshing the viewport displays.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"

static boolean double_buffer_flag = FALSE;
static int     cur_screen_width, cur_screen_height;
static int     buffer_canvas;				/* SRGP canvas id */

/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Macros over SRGP for drawing
 * ------------------------------------------------------------------------- */
static void
PREPARE_FOR_DRAWING (void)
{
   SRGP_useCanvas (double_buffer_flag ? buffer_canvas : 0);
}

#define BUFFER_TO_SCREEN   TERMINATE_DRAWING

static void
TERMINATE_DRAWING (void)
{
   if (double_buffer_flag) {
      SRGP_useCanvas (0);
      SRGP_copyPixel (buffer_canvas, 
		      SRGP_inquireCanvasExtent(0), SRGP_defPoint(0,0));
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Should be called only if a redraw (at least) is needed!
 *
 * 		IF wireframe rendering (raw or otherwise):
 * 			full retraversal is performed
 * 		ELSE IF view has obsolete object list:
 * 			full retraversal
 * 		ELSE IF view has obsolete PDC vertex list:
 * 			PDC vertices are recalculated from the UVN list
 * 		ELSE 
 * 			Redrawing *only* is performed
 *
 * DETAILS :	This function resets the obsolescence flags in the view
 * 		table!
 * ------------------------------------------------------------------------- */
static void
RefreshOneView (
   int view)
{
   register root_header *root;
   register view_spec *vs;

   currentViewIndex  = view;
   currentViewSpec   = vs = &(SPH__viewTable[view]);
   currentRendermode = vs->rendermode;

   /* Ignore this view if no roots are posted to it, or it's disabled */
   if (vs->lowestOverlapNetwork == NULL || vs->currently_disabled)
      return;
   
   /* Clear the viewport area */
   SRGP_setClipRectangle (vs->pdc_viewport);
   SRGP_setFillStyle (SOLID);
   SRGP_setColor (vs->background_color);
   SRGP_fillRectangle (vs->pdc_viewport);
   
   /* always regenerate objects for Painter's Algorithm */
   if (vs->obsolete_camera && vs->algorithm == RENDER_PAINTERS)
      vs->obsolete_objects = TRUE;

   if (vs->obsolete_objects || vs->rendermode == WIREFRAME_RAW)
   {
      if (vs->rendermode > WIREFRAME_RAW)   
         OBJECT__init (vs);

      for (root = vs->lowestOverlapNetwork;
	   root != NULL;
	   root = root->nextHigherOverlapRoot)
	 SPH__traverse_network_for_display (vs, root);
   }

   /* IF rendermode IS NOT RAW, WE NEED TO PROCESS THE OBJECTS. */
   if (currentRendermode > WIREFRAME_RAW) {
      OBJECT__process (vs);
      OBJECT__drawAll (vs);
   }

   /* TELL SRGP TO REFRESH SCREEN  (no-op unless X11) */
   SRGP_refresh();
   
   /* RESET THE FLAGS */
   vs->obsolete_render = 
   vs->obsolete_camera = 
   vs->obsolete_objects = FALSE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Whites out the canvas
 * ------------------------------------------------------------------------- */
static void
WHITEWASH_RECT (
   rectangle r)
{
   SRGP_setClipRectangle (r);
   SRGP_setFillStyle (SOLID);
   SRGP_setColor (SRGP_WHITE);
   SRGP_fillRectangle (r);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Only refreshes views that actually need to be redrawn or
 * 		retraversed due to changes in objects or in the view
 * 		specification.
 * ------------------------------------------------------------------------- */
static void
PERFORM_REFRESH (void)
{
   register vi;
   register view_spec *v;
   boolean  refresh_occurred = FALSE;

   for (vi=0; vi <= MAX_VIEW_INDEX; vi++) {
      v = &(SPH__viewTable[vi]);

      if ((v->lowestOverlapNetwork != NULL) && 
	  (!v->currently_disabled) &&
	  (v->obsolete_objects || v->obsolete_camera || v->obsolete_render))
      {
	 if (!refresh_occurred)
	    PREPARE_FOR_DRAWING();
	 refresh_occurred = TRUE;

	 RefreshOneView (vi);
      }
   }

   if (refresh_occurred)
      TERMINATE_DRAWING();
}

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Called  when we are told by the window manager that an update
 * 		should occur to handle damage.  ASSUMES no data change has
 * 		taken place since screen last painted. 
 *
 * DETAILS :	Very efficient if double buffering happens to be on.
 * 		CURRENTLY NOT INSTALLED.  Moreover, it's not needed when
 * 		backing store is on.  
 * ------------------------------------------------------------------------- */
void
SPH__repaintScreen (void)
{
   if (double_buffer_flag)
      BUFFER_TO_SCREEN();
   else
      SPH_regenerateScreen();
}

/* -------------------------------------------------------------------------
 * DESCR   :	For refreshing after a structure is closed.
 * ------------------------------------------------------------------------- */
void
SPH__refresh_structure_close (
   int structID)
{
   register int v;
   boolean flag = FALSE;

   for (v=0; v<=MAX_VIEW_INDEX; v++)
     if (SPH__testBit(SPH__viewTable[v].descendent_list, structID)) {
        SPH__viewTable[v].obsolete_objects = TRUE;
        if (SPH__implicitRegenerationMode == ALLOWED) {
	   if (!flag)
	      PREPARE_FOR_DRAWING();
	   flag = TRUE;
           RefreshOneView (v);
	}
     }
   if (flag)
      TERMINATE_DRAWING();
}

/* -------------------------------------------------------------------------
 * DESCR   :	For refreshing after a filter has been added or removed.
 * ------------------------------------------------------------------------- */
void
SPH__refresh_filter (
   int view)
{
   SPH__viewTable[view].obsolete_camera = TRUE;
   if (SPH__implicitRegenerationMode == ALLOWED) {
      PREPARE_FOR_DRAWING();
      RefreshOneView (view);
      TERMINATE_DRAWING();
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	For refreshing after the list of light source has been
 * 		altered. 
 * ------------------------------------------------------------------------- */
void
SPH__refresh_lights (
   int view)
{
   SPH__viewTable[view].obsolete_render = TRUE;
   if (SPH__implicitRegenerationMode == ALLOWED) {
      PREPARE_FOR_DRAWING();
      RefreshOneView (view);
      TERMINATE_DRAWING();
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	For refreshing after a structure is posted.
 * ------------------------------------------------------------------------- */
void
SPH__refresh_post (
   int view)
{
   SPH__viewTable[view].obsolete_objects = TRUE;
   if (SPH__implicitRegenerationMode == ALLOWED) {
      PREPARE_FOR_DRAWING();
      RefreshOneView (view);
      TERMINATE_DRAWING();
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Refreshing after an unpost.  If the view no longer has any
 * 		posted members, do a whitewash! 
 * ------------------------------------------------------------------------- */
void 
SPH__refresh_unpost (
   int view)
{
   if (SPH__viewTable[view].highestOverlapNetwork == NULL) {
      PREPARE_FOR_DRAWING();
      WHITEWASH_RECT (SPH__viewTable[view].pdc_viewport);
      TERMINATE_DRAWING();
   }
   else {
      SPH__viewTable[view].obsolete_objects = TRUE;
      if (SPH__implicitRegenerationMode == ALLOWED) {
	 PREPARE_FOR_DRAWING();
         RefreshOneView (view);
	 TERMINATE_DRAWING();
      }
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Refreshing after a viewport change.  Should we whitewash the
 * 		area that is occupied by the current viewport for this view?
 * 		Only if:
 * 		      	1) There is at least one structure posted to the view.
 * 		   AND  2) The intersection of the current vp and the new vp
 * 				is NOT equal to the current vp.
 *            
 * DETAILS :	Later we will have to repair damage done to overlapping
 * 		innocent-bystander views.  
 * ------------------------------------------------------------------------- */
void 
SPH__refresh_viewport_change (
   int 		   viewIndex, 
   rectangle       old_viewport_rect)
{
   rectangle intersection;
   boolean flag = FALSE;


        /* GEOM SAYS THEY DO INTERSECT */
   if (!GEOM_computeRectIntersection(old_viewport_rect, 
				     SPH__viewTable[viewIndex].pdc_viewport,
				     &intersection) ||

       /* AH!  The intersection is not equal to the original viewport. */
       memcmp(&intersection,&old_viewport_rect, sizeof(rectangle)))
   {
      PREPARE_FOR_DRAWING();
      flag = TRUE;
      WHITEWASH_RECT (old_viewport_rect);
   }

   SPH__viewTable[viewIndex].obsolete_camera = TRUE;


   if (SPH__implicitRegenerationMode == ALLOWED) {
      if (!flag)
         PREPARE_FOR_DRAWING();
      flag = TRUE;
      RefreshOneView (viewIndex);
   }
      
   if (flag)
      TERMINATE_DRAWING();
}


/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Allows (or disllows) SPHIGS to refresh itself.
 * ------------------------------------------------------------------------- */
void
SPH_setImplicitRegenerationMode (
   int mode)
{
   SPH_check_system_state;

   if (mode == SPH__implicitRegenerationMode)
      return;
      
   if (mode==ALLOWED)
	 PERFORM_REFRESH();
	 
   SPH__implicitRegenerationMode = mode;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Sets the rendering method.
 * ------------------------------------------------------------------------- */
void
SPH_setRenderingMode (
   int viewIndex, 
   int value)
{
   SPH_check_system_state;
   SPH_check_view_index;
   SPH_check_rendering_mode (value);

   if (SPH__viewTable[viewIndex].rendermode != value) {
      SPH__viewTable[viewIndex].rendermode = value;
      SPH__viewTable[viewIndex].obsolete_render = TRUE;
      if (SPH__implicitRegenerationMode == ALLOWED) {
         PREPARE_FOR_DRAWING();
         RefreshOneView (viewIndex);
         TERMINATE_DRAWING();
      }
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Allows (or disllows) SPHIGS to do double-buffering (a bit
 * 		slower with than without).
 * ------------------------------------------------------------------------- */
void
SPH_setDoubleBufferingFlag (
   boolean flag)
{
   SPH_check_system_state;

   if (flag == double_buffer_flag)
      return;
      
   if (flag) {
      /* USER IS TURNING ON DOUBLE BUFFERING */
      /* Allocate an SRGP canvas */
      SRGP_inquireCanvasSize (0, &cur_screen_width, &cur_screen_height);
      buffer_canvas = SRGP_createCanvas (cur_screen_width, cur_screen_height);
      /* Initialize to copy of current screen */
      SRGP_useCanvas (buffer_canvas);
      SRGP_copyPixel (0, SRGP_inquireCanvasExtent(0), SRGP_defPoint(0,0));
   }
   else {
      /* USER IS DISABLING DOUBLE BUFFERING */
      /* Deallocate the canvas */
      SRGP_deleteCanvas (buffer_canvas);
   }
   
   double_buffer_flag = flag;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Shuts off and erases the specified view.
 * ------------------------------------------------------------------------- */
void
SPH_disableView (
   int viewIndex)
{
   SPH_check_system_state;
   SPH_check_view_index;

  if (SPH__viewTable[viewIndex].currently_disabled)
     return;
     
  WHITEWASH_RECT (SPH__viewTable[viewIndex].pdc_viewport);
  SPH__viewTable[viewIndex].currently_disabled = TRUE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Activates the specified view.
 * ------------------------------------------------------------------------- */
void
SPH_enableView (
   int viewIndex)
{
   SPH_check_system_state;
   SPH_check_view_index;

  if ( ! SPH__viewTable[viewIndex].currently_disabled)
     return;
     
  SPH__viewTable[viewIndex].currently_disabled = FALSE;
  SPH__refresh_viewport_change (viewIndex,
				SPH__viewTable[viewIndex].pdc_viewport);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Called by the application, typically when it has turned off
 * 		implicit regeneration. 
 *
 * DETAILS :	Thus, we can't assume we're just repairing screen damage!
 * 		But it does not retraverse or recompute PDCs unless a true
 * 		data change occurred, so it is efficient.  
 * ------------------------------------------------------------------------- */
void
SPH_regenerateScreen (void)
{
   register vi;

   SPH_check_system_state;

   PREPARE_FOR_DRAWING();
   for (vi=0; vi <= MAX_VIEW_INDEX; vi++)
       RefreshOneView (vi);
   TERMINATE_DRAWING();
}
