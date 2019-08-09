#include "HEADERS.h"
#include "SPHDEMO.h"
#include "SPHDEMO_struct.h"
#include "SPHDEMO_view.h"
#include "SPHDEMO_input.h"

/* -------------------------------------------------------------------------
 * SPHDEMO_main.c
 *
 *	Main module for SPHIGS test program, an interactive camera viewer.
 * ------------------------------------------------------------------------- */

typedef enum {
   CAM_VRP_UVN = 1,
   CAM_VRP_MC  = 2,
   CAM_VPN     = 3,
   CAM_PRP     = 4,
   CAM_FRONT   = 5,
   CAM_BACK    = 6
} camControl;

static int 	  motion_granularity = 30;
static camControl curCameraControl;


/*----------------------------------------------------------------------*/
/* Help Messages */

static void
SetCameraControl (int whichitem)
{
   curCameraControl = whichitem;

   switch (whichitem)
   {
    case CAM_VRP_UVN:
      fprintf (stderr, "You are now controlling the VRP along the UVN axes\n");
      break;
    case CAM_VRP_MC:
      fprintf (stderr, "You are now controlling the VRP along the XYZ axes\n");
      break;
    case CAM_VPN:
      fprintf (stderr, "You are now controlling the VPN\n\
      You can 'yaw' (rotate about the V axis) by using the S and D keys\n\
      You can 'pitch' (rotate about the U axis) by using the E and X keys\n");
      break;
    case CAM_PRP:
      fprintf (stderr, "You are now controlling the PRP\n\
      Move it towards the viewplane (reduce the focal length) using - key\n\
      Move it away from the viewplane (increase focal length) using = key\n");
      break;
    case CAM_FRONT:
      fprintf (stderr, "You are now controlling the Front clipping plane\n\
      Use = to bring it towards eye, - to push away from eye\n");
      break;
    case CAM_BACK:
      fprintf (stderr, "You are now controlling the Back clipping plane\n\
      Use = to bring it towards eye, - to push away from eye\n");
      break;
   }      
}
         
static void
DisplayMotionGranularity (void)
{
   fprintf (stderr,  "Current motion granularity is %d\n", motion_granularity);
}

static void
PrintCorrelationInfo (pickInformation *pinfo)
{
   int i;

   fprintf (stderr, "You picked ");

   fprintf (stderr, "%s", 		
	            (pinfo->pickLevel == 4) ? "the chimney on " : "");

   fprintf (stderr, "house #%d ", 
	            1 + (int)((pinfo->path[1].elementIndex - 1) / 4));

   fprintf (stderr, "on street #%d.\n",
	            1 + (int)(pinfo->path[0].elementIndex / 2));
}

/*----------------------------------------------------------------------*/
/* Action Routines */

static void
AddGridInFrontOfRoot (int structID, int viewID)
{
   SPH_unpostRoot (structID, viewID);
   SPH_postRoot (GRID_STRUCT, viewID);
   SPH_postRoot (structID, viewID);
}

static void
ChangeCameraOnAxis (int axis, double bias)
{
   double pitch = 0.0;
   double yaw = 0.0;

   switch (curCameraControl)
   {
    case CAM_VRP_UVN:
      ChangeVRP_relativeUVN (bias*motion_granularity, axis); 
      break;

    case CAM_VRP_MC:
      ChangeVRP_relativeWC (bias*motion_granularity, axis); 
      break;

    case CAM_VPN:
      switch (axis) {
       case X_AXIS: 
	 yaw = bias * motion_granularity; break;
       case Y_AXIS: 
	 pitch = bias * motion_granularity; break;
       default:
	 return;
      }
      ChangeVPN (pitch, yaw); 
      break;

    case CAM_PRP:
      if (axis != Z_AXIS) 
	 return;
      ChangePRP (bias*motion_granularity);
      break;

    case CAM_FRONT:
      if (axis != Z_AXIS) 
	 return;
      ChangeFrontClipPlane (bias*motion_granularity);
      break;

    case CAM_BACK:
      if (axis != Z_AXIS) 
	 return;
      ChangeBackClipPlane (bias*motion_granularity);
      break;
   }

   ShowUsingNewView1 ();
}

static void
MouseMap (eventStruct *event)
{
   static point anchor;
   char   cbuf;
   vector delta;
   int    i, map;

   if (event->status != BTN_DOWN &&
       event->viewID == PERSPECTIVE_VIEW) {

      MAT3_SUB_VEC(delta, event->position, anchor);

      if (! MAT3_IS_ZERO (delta[X]) ||
	  ! MAT3_IS_ZERO (delta[Y])) {

	 switch(event->button) {      
	  case 0: 		/* left button */
	    ChangeVRP_relativeUVN (-delta[X]*8*motion_granularity, X_AXIS); 
	    ChangeVRP_relativeUVN (-delta[Y]*8*motion_granularity, Y_AXIS); 
	    break;

          case 1: 		/* middle button */
            ChangeVRP_relativeUVN (delta[X]*8*motion_granularity, Z_AXIS); 
            break;
      
          case 2: 		/* right button */
            ChangeVPN (delta[Y]*8*motion_granularity,
           	    -delta[X]*8*motion_granularity);
            break;
         }
	 ShowUsingNewView1 ();
      }
   }
   MAT3_COPY_VEC(anchor, event->position);
}

/*----------------------------------------------------------------------*/
/* Initialization */

static void
InitSPHIGS (int argc, char **argv)
{
   boolean READ_FROM_STDIN = FALSE;
   int 	   numplanes, numshades;

   if (argc > 1)
      if ( ! strcmp(argv[1],"-stdin"))
	 READ_FROM_STDIN = TRUE;
   
   /* STARTUP SPHIGS */
   numplanes = 0; numshades = 10;
   if (READ_FROM_STDIN)
   {
      printf ("\nPlease enter number of framebuffer planes to allocate:\n");
      scanf  ("%d", &numplanes);

      printf ("\nPlease enter number of shades per flexicolor:\n");
      printf ("   (Note: this affects how many different flexicolors will ");
      printf (						"be available.)\n");
      scanf  ("%d", &numshades);
   }

#ifdef THINK_C
   SPH_begin (500,400, numplanes, numshades);
#else
   SPH_begin (850,680, numplanes, numshades);
#endif
   SPH_setDoubleBufferingFlag (FALSE);   
   SPH_setImplicitRegenerationMode (SUPPRESSED);

   /* We hope there are enough flexicolors for these. */

   SPH_loadCommonColor (2, "yellow");
   SPH_loadCommonColor (3, "green");
   SPH_loadCommonColor (4, "turquoise");
   SPH_loadCommonColor (5, "pink");
   SPH_loadCommonColor (6, "goldenrod");
   SPH_loadCommonColor (7, "firebrick");

   /* But these need not be flexicolors. */

   SPH_loadCommonColor (red, "red");
   SPH_loadCommonColor (grey, "lightgrey");
   SPH_loadCommonColor (orange, "orange");
   SPH_loadCommonColor (yellow, "yellow");
   SPH_loadCommonColor (limegreen, "yellowgreen");
   SPH_loadCommonColor (forestgreen, "forestgreen");
   SPH_loadCommonColor (blue, "blue");

   printf ("\nThis is what I was able to provide re: colors...\n");
   printf ("     Number of custom, flexible colors: %d\n", NUM_OF_FLEXICOLORS);
   printf ("     Number of custom, non-flexible colors: %d\n\n", 
	   (NUM_OF_APPL_SETTABLE_COLORS-NUM_OF_FLEXICOLORS));
}

/*----------------------------------------------------------------------*/
/* Main Routine */

void
main (int argc, char **argv)
{
   double 		bias;
   pickInformation 	pickinfo;
   eventStruct 		event;
   char 		buf[100];
   int 			chimneyFlag = 'c';
   int			viewFlag    = 'A';
   point		p;
   
#ifdef THINK_C
#include <console.h>
  console_options.nrows = 8;
#endif

   SPH_setMaxLightSourceIndex (3);
   InitSPHIGS (argc, argv);
   InitAllViews ();
   BuildEverything ();
   DisplayAllViews ();
   ShowUsingNewView1 ();
   SPH_postRoot (NEIGHBORHOOD_STRUCT, PERSPECTIVE_VIEW);
   DisplayMotionGranularity ();   
   SetCameraControl (CAM_VRP_MC);
   InitInput ();
   
   while(TRUE)
   {
      SPH_setImplicitRegenerationMode (ALLOWED);
      SPH_setImplicitRegenerationMode (SUPPRESSED);
      bias = 1.0;
   
      if (SampleMouse (&event)) {	  
	 MouseMap(&event);
	 if (event.button == 0 && 
	     event.status == BTN_DOWN &&
	     event.viewID == PERSPECTIVE_VIEW) {
	    SPH_pickCorrelate (event.position, event.viewID, &pickinfo);
	    if (pickinfo.pickLevel > 0)
	       PrintCorrelationInfo (&pickinfo);
	 }
      }
      
      if (SampleKeyboard (&event)) {
	 switch (event.keypress) {
   
          case '<':
            if ((motion_granularity -= 5) < 1)  motion_granularity =  1;
            DisplayMotionGranularity(); 
	    break;
   
          case '>':
	    if ((motion_granularity += 5) > 99) motion_granularity = 99;
            DisplayMotionGranularity(); 
            break;
     
          case 'g':
            sscanf (buf, "g%d", &motion_granularity);
            DisplayMotionGranularity();
            break;
      
          case '0':
            RestoreCameraToDefault ();
            ShowUsingNewView1 ();
            break;
      
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
            SetCameraControl (event.keypress - '0');	
            break;
      
          case 's':
          case 'S':
            bias = -1.0; /* nobreak*/
          case 'd':
          case 'D':
            ChangeCameraOnAxis (X_AXIS, bias);
            break;
      
          case 'x':
          case 'X':
            bias = -1.0; /*nobreak*/
          case 'e':
          case 'E':
            ChangeCameraOnAxis (Y_AXIS, bias);
            break;
      
          case '-':
            bias = -1.0; /*nobreak*/
          case '=':
            ChangeCameraOnAxis (Z_AXIS, bias);
            break;
      
          case 'C':
          case 'c':
            if (event.keypress != chimneyFlag)
	    {
	       if (event.keypress == 'C')
	       {
		  AddChimneyToHouse (HOUSE_STRUCT);
		  AddChimneyToHouse (FAKE_HOUSE_TOP_STRUCT);
		  AddChimneyToHouse (FAKE_HOUSE_SIDE_STRUCT);
	       }
	       else 
	       {
		  TakeChimneyFromHouse (HOUSE_STRUCT);
		  TakeChimneyFromHouse (FAKE_HOUSE_TOP_STRUCT);
		  TakeChimneyFromHouse (FAKE_HOUSE_SIDE_STRUCT);
	       }
	       chimneyFlag = event.keypress;
	    }
            break;
      
          case 'a':
          case 'A':
            if (event.keypress != viewFlag) 
	    {
	       SPH_setImplicitRegenerationMode (SUPPRESSED);
	       if (event.keypress == 'a')
		  DisplayOnlyFullRenderView();
	       else
		  DisplayAllViews();
	       ShowUsingNewView1();
	       SPH_setImplicitRegenerationMode (ALLOWED);
	       viewFlag = event.keypress;
	    }
            break;
      
	  case 'G':
	    AddGridInFrontOfRoot (NEIGHBORHOOD_STRUCT, PERSPECTIVE_VIEW);
	    break;

	    /* Undocumented */
	  case '}':
	    ChangePRPforOrtho();
	    ShowUsingNewView1 ();
	    break;
	  case '{':
	    ChangePRPforClosestPerspective();
	    ShowUsingNewView1 ();
	    break;

	  case 'B':
	    SPH_setDoubleBufferingFlag (FALSE);   break;
	  case 'b':
	    SPH_setDoubleBufferingFlag (TRUE);    break;

          case 'W':
            SPH_setRenderingMode (PERSPECTIVE_VIEW, WIREFRAME_RAW); break;
          case 'w':
            SPH_setRenderingMode (PERSPECTIVE_VIEW, WIREFRAME); break;
          case 'F':
            SPH_setRenderingMode (PERSPECTIVE_VIEW, FLAT); break;
          case 'f':
            SPH_setRenderingMode (PERSPECTIVE_VIEW, LIT_FLAT); break;      

	  case 'P':
            SPH_setViewRenderAlgorithm (PERSPECTIVE_VIEW, RENDER_BSP); 
	    break;      
          case 'p':
            SPH_setViewRenderAlgorithm (PERSPECTIVE_VIEW, RENDER_PAINTERS);
	    break;      
   

          case 'q':
          case 'Q':
            SPH_end ();
	    fprintf (stderr, "Bye!\n");
	    exit (0);
	 }
      }
   }
}
