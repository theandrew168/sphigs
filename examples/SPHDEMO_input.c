#include "HEADERS.h"
#include "SPHDEMO.h"
#include "SPHDEMO_struct.h"
#include "SPHDEMO_view.h"
#include "SPHDEMO_input.h"

/* -------------------------------------------------------------------------
 * SPHDEMO_input.c
 *
 *	Wrapper around SPHIGS event handler that spits out events like Motif
 * does.  This makes mouse mapping easier.
 * ------------------------------------------------------------------------- */


/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Polls the mouse for any new events.  This is slightly
 * 		different from the way SPHIGS normally samples the mouse,
 * 		since motion events only matter while the mouse button is
 * 		down. 
 * ------------------------------------------------------------------------- */
int				/* Returns: TRUE if an event occurred */
SampleMouse(eventStruct *event)
{
   static short    btnDown = -1;
   static point    lastPt;
   locator_measure locMeas;
   int 		   i;

   SPH_sampleLocator (&locMeas);

   /* determine relevant button */
   if (btnDown == -1) {
      for(i = 0; i < 3; i++) 
	 if (locMeas.button_chord[i] == DOWN) 
	    break;
      if (i == 3)
	 return FALSE;

      btnDown = i;
      event->status = BTN_DOWN;
   }
   else {
      if (locMeas.button_chord[btnDown] == DOWN) {
	 if (MAT3_EQUAL_VECS (lastPt, locMeas.position))
	    return FALSE;
	 event->status = BTN_MOTION;
      } 
      else
	 event->status = BTN_UP;
   }

   event->button = btnDown;
   event->type = LOCATOR;
   event->viewID = locMeas.view_index;
   MAT3_COPY_VEC(lastPt, locMeas.position);
   MAT3_COPY_VEC(event->position, locMeas.position);
   
   if (event->status == BTN_UP)
      btnDown = -1;

   return TRUE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Samples the keyboard for any NEW keypresses.
 * ------------------------------------------------------------------------- */
int				/* Returns: TRUE if an event occurred */
SampleKeyboard(eventStruct *event)
{
   static int		 	  init = FALSE;
   static srgp_timestamp 	  timestamp;
   char 			  cbuf[2];
   srgp__deluxe_keyboard_measure  keyMeas;
   
   keyMeas.buffer 	 = cbuf;
   keyMeas.buffer_length = 2;
   SRGP_sampleDeluxeKeyboard (&keyMeas);

   if (init && timestamp.seconds == keyMeas.timestamp.seconds &&
               timestamp.ticks == keyMeas.timestamp.ticks)
      return FALSE;

   init = TRUE;
   timestamp.seconds = keyMeas.timestamp.seconds;
   timestamp.ticks = keyMeas.timestamp.ticks;

   if (!isprint(cbuf[0]))
      return FALSE;

   event->type = KEYBOARD;
   event->keypress = cbuf[0];

   /* hack! clears key input buffer */
   SPH_setKeyboardProcessingMode (EDIT);
   SPH_setKeyboardProcessingMode (RAW);

   return TRUE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Initializes the input module.
 * ------------------------------------------------------------------------- */
void
InitInput ()
{
   point       keyLoc;

   SPH_setKeyboardProcessingMode (RAW);
   SPH_setLocatorButtonMask (LEFT_BUTTON_MASK | 
			     MIDDLE_BUTTON_MASK | 
			     RIGHT_BUTTON_MASK);

   /* prevents nasty blips everytime we type a key */
   MAT3_SET_VEC (keyLoc, 0.0, 0.0, 0.0);
   SPH_setKeyboardEchoOrigin (keyLoc);

   SPH_setInputMode (KEYBOARD, SAMPLE);
   SPH_setInputMode (LOCATOR, SAMPLE);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Polls all devices for pending events.
 * ------------------------------------------------------------------------- */
int				/* Returns: TRUE if an event occurred */
SampleEvent(eventStruct *event)
{
   return (SampleMouse (event) || SampleKeyboard (event));
}

