#ifndef lint
static char Version[]=
   "$Id: sph_input.c,v 1.6 1993/06/24 05:41:52 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_input.c
 *
 *	Pick correlation routines.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include  "sphigslocal.h"

#define GROW_RECTANGLE(R, PIX) {			\
   (R).bottom_left.x-=(PIX); (R).top_right.x+=(PIX);	\
   (R).bottom_left.y-=(PIX); (R).top_right.y+=(PIX);  }

#define POINT_IN_RECTANGLE(P, R) 				\
   ((P).x >= (R).bottom_left.x && (P).x <= (R).top_right.x &&	\
    (P).y >= (R).bottom_left.y && (P).y <= (R).top_right.y)

/* for constructing pick path */
static unsigned short   cur_traversal_index, hit_traversal_index;
static pickInformation *INFO;


/* for clipping algorithm pick correlation */
static rectangle epsilonSquare;

static boolean IsPowerOfTwo[] = {
   FALSE,		/*  0 */
   TRUE,		/*  1 */
   TRUE,		/*  2 */
   FALSE,		/*  3 */
   TRUE,		/*  4 */
   FALSE,		/*  5 */
   FALSE,		/*  6 */
   FALSE,		/*  7 */
   TRUE,		/*  8 */
   FALSE,		/*  9 */
   FALSE,		/* 10 */
   FALSE,		/* 11 */
   FALSE,		/* 12 */
   FALSE,		/* 13 */
   FALSE,		/* 15 */
   TRUE			/* 16 */
};

/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Converts SRGP locator struct (mouse click location struct)
 * 		from device coordinates to world coordinates.
 * ------------------------------------------------------------------------- */
static void
ConvertLocatorMeasure (
   locator_measure       *lm,
   srgp__locator_measure *slm)
{
   register i;

   for(i=0;i<3;i++) lm->button_chord[i] = slm->button_chord[i];
   slm->button_of_last_transition = lm->button_of_last_transition;

   /* TRANSLATE srgp/pdc TO npc */
   lm->position[X] = (double)(slm->position.x) / SPH__ndcSpaceSizeInPixels;
   lm->position[Y] = (double)(slm->position.y) / SPH__ndcSpaceSizeInPixels;
   lm->position[Z] = 0.0;

   /* FIND THE RELEVANT VIEW INDEX:
      I will give higher-indexed views first priority, but ignore any
      views that have nothing posted to them.
      But, 0 will be returned if nothing else fits the bill. */
   for (i=MAX_VIEW_INDEX; i>=0; i--)
      if (GEOM_ptInRect(slm->position, SPH__viewTable[i].pdc_viewport))
	 if (SPH__viewTable[i].highestOverlapNetwork)
	    break;
   lm->view_index = i;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Quadrant-classification code generator for Cohen-Sutherland
 * 		line clipping algorithm.
 * ------------------------------------------------------------------------- */
static int
Outcode (
   srgp__point pt)
{
   int outcode;

   if (pt.x > epsilonSquare.top_right.x)
      outcode = 2;
   else if (pt.x < epsilonSquare.bottom_left.x)
      outcode = 1;
   else
      outcode = 0;

   if (pt.y > epsilonSquare.top_right.y)
      outcode += 8;
   else if (pt.y < epsilonSquare.bottom_left.y)
      outcode += 4;

   return outcode;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Cohen-Sutherland line-clipping, which can be used for pick
 * 		correlation by constructing a tolerance rectangle around the
 * 		click point and determining whether any lines cross it.
 * ------------------------------------------------------------------------- */
static boolean
LineCrossesEpsilonSquare (
   srgp__point pt1, 
   srgp__point pt2)
{
   enum {UNKNOWN, YES, NO} doesCross = UNKNOWN;
   int outcode1, outcode2;


   do {
      outcode1 = Outcode (pt1);
      outcode2 = Outcode (pt2);

      if ((outcode1 & outcode2) > 0) 
	 doesCross = NO;
      else if ((outcode1==0) || (outcode2==0)) 
	 doesCross = YES;
      else if ((IsPowerOfTwo[outcode1]) && (IsPowerOfTwo[outcode2])) {
	 if ((outcode1 ^ outcode2) == 3)
	    doesCross = YES;
	 else if ((outcode1 ^ outcode2) == 12)
	    doesCross = YES;
      }

      if (doesCross == UNKNOWN) {
	 if ((outcode1 & 8) != 0) {
	    pt1.x += ((pt2.x-pt1.x)*(epsilonSquare.top_right.y-pt1.y))/
	       (pt2.y-pt1.y);
	    pt1.y = epsilonSquare.top_right.y;
	 }
	 else if ((outcode1 & 4) != 0) {
	    pt1.x += ((pt2.x-pt1.x)*(epsilonSquare.bottom_left.y-pt1.y))/
	       (pt2.y-pt1.y);
	    pt1.y = epsilonSquare.bottom_left.y;
	 }
	 else if ((outcode1 & 2) != 0) {
	    pt1.y += ((pt2.y-pt1.y)*(epsilonSquare.top_right.x-pt1.x))/
	       (pt2.x-pt1.x);
	    pt1.x = epsilonSquare.top_right.x;
	 }
	 else if ((outcode1 & 1) != 0) {
	    pt1.y += ((pt2.y-pt1.y)*(epsilonSquare.bottom_left.x-pt1.x))/
	       (pt2.x-pt1.x);
	    pt1.x = epsilonSquare.bottom_left.x;
	 }
      }
   }
   while (doesCross == UNKNOWN);

   return (doesCross == YES);
}
      
/* -------------------------------------------------------------------------
 * DESCR   :	Tests for point inside polygon.
 * ------------------------------------------------------------------------- */
static boolean 
PtInPolygon (
   int           vertcount, 
   vertex_index *vertindexlist, 
   srgp__point  *verts)
{
   int hitcount = 0;
   int codecur, codeprev, codeLOOK;
   int jcur, jprev;
   double m, b;
   srgp__point ptcur, ptprev;
   register i;

   jcur = 0;
   ptcur = verts[vertindexlist[jcur]];
   codecur = Outcode (ptcur);

   for (i=1; i<=vertcount; i++) {
      jprev = jcur;
      codeprev = codecur;
      ptprev = ptcur;
      
      if (i==vertcount)
	 jcur = 0;
      else
	 jcur = i;
      
      ptcur = verts[vertindexlist[jcur]];
      codecur = Outcode (ptcur);

      /* FIRST, CHECK FOR SPECIAL CASE OF A VERTEX ON THE RAY ITSELF. */
      if ((codecur == 2) || (codeprev == 2)) {     /* if  *on* the ray! */
	 if (ptcur.y < ptprev.y)
	    codeLOOK = codecur;
	 else
	    codeLOOK = codeprev;
	 if ((codeLOOK == 2) && (codecur != codeprev))
	    hitcount++;

	 continue;   /* don't go into the switch statement */
      }
      
      switch (codecur & codeprev) {
       case 1:
       case 4:
       case 5:
       case 6:
       case 8:
       case 9:
       case 10:
	 /* TRIVIAL REJECT */
	 break;
       case 2:
	 /* TRIVIAL ACCEPT */
	 hitcount++;
	 break;
       default:
	 if (((codecur | codeprev) & 3) == 0) {
	    /* THE prev->cur LINE ACTUALLY PASSES THROUGH THE pdcpt !!! */
	    /* IGNORE IT!!! */
	 }
	 else {
#           define pdcpt   (epsilonSquare.bottom_left)
	    m = ((double)ptprev.y-ptcur.y) / ((double)ptprev.x-ptcur.x);
	    b = (double)ptprev.y - (m*ptprev.x);
	    if ( (((double)pdcpt.y - b) / m ) >= (double)pdcpt.x)
	       hitcount++;
#           undef pdcpt
	 }
      }
   }
   
   return ((hitcount % 2) == 1);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Pick correlation for polylines (and facet outlines in
 * 		WIREFRAME mode.
 * ------------------------------------------------------------------------- */
static boolean 
PtOnPolygon (
   int           vertcount, 
   vertex_index *vertindexlist, 
   srgp__point  *verts)
{
   int jcur, jprev;
   srgp__point ptcur, ptprev;
   register i;


   jcur = 0;
   ptcur = verts[vertindexlist[jcur]];

   for (i=1; i<=vertcount; i++) {
      jprev = jcur;
      ptprev = ptcur;
      
      if (i==vertcount)
	 jcur = 0;
      else
	 jcur = i;
      
      ptcur = verts[vertindexlist[jcur]];

      if (LineCrossesEpsilonSquare (ptcur, ptprev))
	 return TRUE; 
  }

   return FALSE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Pick correlation for polymarkers.
 * ------------------------------------------------------------------------- */
static boolean 
PtOnPoint (
   int           vertcount,	/* ignored */
   vertex_index *vertindexlist, 
   srgp__point  *verts)
{
   return POINT_IN_RECTANGLE (verts[*vertindexlist], epsilonSquare);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Builds pick path as a structure is traversed.
 * ------------------------------------------------------------------------- */
static boolean
TraverseStruct (
   int structID)
{
   register element *curel;

   pickPathItem *curpitem = &(INFO->path[INFO->pickLevel++]);

   curpitem->structureID = structID;
   curpitem->elementIndex = 0;
   curpitem->pickID = 0;

   for /* FOR EACH ELEMENT IN THE STRUCTURE, IN ORDER: */
      (curel = SPH__structureTable[structID].first_element;
       curel;
       curel = curel->next) {
	 curpitem->elementIndex++;
	 switch (curel->type) {
	  case ELTYP__PICK_ID:
	    curpitem->pickID = curel->data.value;
	    break;
	  case ELTYP__POLYMARKER:
	  case ELTYP__POLYLINE:
	  case ELTYP__POLYHEDRON:
	  case ELTYP__FILL_AREA:
	    if (cur_traversal_index++ == hit_traversal_index) {
	       /******* !!!!!!!!!!!! C O R R E L A T I O N !!!!!!!!!!!! ******/
	       /* SRGP_beep(); */
	       curpitem->elementType = curel->type;
	       return TRUE;
	    }
	    break;
	  case ELTYP__EXECUTE_STRUCTURE:
	    if (TraverseStruct (curel->data.value)) {
	       curpitem->elementType = curel->type;
	       return TRUE;
	    }
	 }
      }

   /* Well, we didn't make a correlate happen in this structure! */
   INFO->pickLevel--;
   return FALSE;
}

/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	SRGP wrapper
 * ------------------------------------------------------------------------- */
void
SPH_getLocator (
   locator_measure* lm)
{
   srgp__locator_measure slm;

   SPH_check_system_state;

   SRGP_getLocator (&slm);
   ConvertLocatorMeasure (lm, &slm);
}

/* -------------------------------------------------------------------------
 * DESCR   :	SRGP wrapper
 * ------------------------------------------------------------------------- */
void 
SPH_sampleLocator (
   locator_measure* lm)
{
   srgp__locator_measure slm;

   SPH_check_system_state;

   SRGP_sampleLocator (&slm);
   ConvertLocatorMeasure (lm, &slm);
}

/* -------------------------------------------------------------------------
 * DESCR   :	SRGP wrapper
 * ------------------------------------------------------------------------- */
void 
SPH_setLocatorMeasure (
   point position)
{
   srgp__point spt;
   register i;

   SPH_check_system_state;

   /* TRANSLATE npc TO srgp/pdc */
   spt.x = position[X] * SPH__ndcSpaceSizeInPixels + 0.5;
   spt.y = position[Y] * SPH__ndcSpaceSizeInPixels + 0.5;

   SRGP_setLocatorMeasure (spt);
}

/* -------------------------------------------------------------------------
 * DESCR   :	SRGP wrapper
 * ------------------------------------------------------------------------- */
void 
SPH_setKeyboardEchoOrigin (
   point position)
{
   srgp__point spt;
   register i;

   SPH_check_system_state;

   /* TRANSLATE npc TO srgp/pdc */
   spt.x = position[X] * SPH__ndcSpaceSizeInPixels + 0.5;
   spt.y = position[Y] * SPH__ndcSpaceSizeInPixels + 0.5;

   SRGP_setKeyboardEchoOrigin (spt);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Performs pick correlation on geometric elements of visible
 * 		structures, accumulates a top-down path through the
 * 		structure hierarchy as it progresses.
 * ------------------------------------------------------------------------- */
void
SPH_pickCorrelate (
   point 	    npc_position,
   int 		    viewIndex, 
   pickInformation *pickinfo)
{
   srgp__point 	 pdcpoint;
   register obj *curobj;
   root_header 	*root;
   view_spec 	*vs;
   vertex_index *indices;
   int		 count;
   int		 width;
   boolean 	 hit_was_made;
#ifndef THINK_C
   boolean     (*correlator)();
#else
   boolean     (*correlator)(...);
#endif

   SPH_check_system_state;

   /* Event struct might contain a -1 view index (NO VIEWPORT), so don't die */
   if (viewIndex < 0) {
      pickinfo->pickLevel = 0;
      return;
   }

   vs = &(SPH__viewTable[viewIndex]);

   if (vs->obsolete_objects || vs->obsolete_camera)
      return; /* SPH__error (ERR_PICK_CORR_WITH_UNSYNCH_DISPLAY); */

   if (vs->rendermode == WIREFRAME_RAW)
      return; /* SPH__error (ERR_PICK_CORR_WITH_WIRE_RAW); */


   /********* CONVERT NPC PICK POINT TO srgp COORDS. */
   pdcpoint.x = npc_position[X] * SPH__ndcSpaceSizeInPixels + 0.5;
   pdcpoint.y = npc_position[Y] * SPH__ndcSpaceSizeInPixels + 0.5;


   /********** FIRST, SCAN BACKWARDS THROUGH THE OBJECT LIST. 
     Optimization bug!  We can't scan backwards, because the object
     list currently is singly linked.  All I can do is scan forwards
     and I have to examine EVERYTHING in the list unconditionally!
     */
   hit_was_made = FALSE;   /* ...until proven guilty */

   for (curobj = vs->objects;
	curobj != NULL;
	curobj = curobj->next)

      if (curobj->display) 
      {      
	 switch (curobj->type) 
	 {
	  default:
	    continue;

	  case OBJ_FACE:
	    correlator = PtInPolygon;	   
	    width = curobj->attributes.edge_flag ? 
	                            curobj->attributes.edge_width >> 1 : 0;
	    break;
	  case OBJ_LINE:
	    correlator = PtOnPolygon;
	    width = curobj->attributes.line_width >> 1;
	    break;
	  case OBJ_POINT:
	    correlator = PtOnPoint;
	    width = curobj->attributes.marker_size >> 1;
	    break;
	 }

	 epsilonSquare.bottom_left = epsilonSquare.top_right = pdcpoint;
	 GROW_RECTANGLE (epsilonSquare, width + 1);
	 OBJECT__get_vertex_indices (curobj->display, &indices, &count);

	 if (correlator (count, indices, vs->pdcVertices))
	 {
	    /********** !!!!!!!!!!!  H I T !!!!!!!!!!!!! ************/
	    hit_was_made = TRUE;
	    hit_traversal_index = curobj->traversal_index;
	 }
      }

   if (FALSE == hit_was_made) {
      pickinfo->pickLevel = 0;
      return;
   }


   /***** NOW, WE KNOW THE UNIQUE TRAVERSAL INDEX OF THE PRIMITIVE SELECTED.
     We must traverse the network speedily (honoring only the
     execution elements) looking for the primitive having that index, keeping
     a stack with the pick information in it.
     */

   cur_traversal_index = 0;
   INFO = pickinfo;

   for (root = vs->lowestOverlapNetwork;
	root != NULL;
	root = root->nextHigherOverlapRoot) 
   {
      INFO->pickLevel  = 0;
      if (TraverseStruct (root->root_structID))
	 break;
   }
}
