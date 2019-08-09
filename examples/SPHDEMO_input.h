
/* -------------------------------------------------------------------------
 * SPHDEMO_input.h
 *
 * 	Definitions for the input module.
 * ------------------------------------------------------------------------- */


/* --------------------------------- Types --------------------------------- */

typedef enum { BTN_DOWN, BTN_MOTION, BTN_UP } buttonEventType;
typedef struct {
   inputDevice 			type;

   char 			keypress;	/* keyboard only */

   point 			position;	/* locator only */
   short			button;
   buttonEventType		status;
   int				viewID;
} eventStruct;

/* ------------------------------ Prototypes ------------------------------- */

extern int  SampleEvent(eventStruct *event);
extern int  SampleMouse(eventStruct *event);
extern int  SampleKeyboard(eventStruct *event);
extern void InitInput(void);
