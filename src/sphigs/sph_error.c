#ifndef lint
static char Version[]=
   "$Id: sph_error.c,v 1.5 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_error.c
 *
 *	Error handler for run time errors.  Most of these amount to illegal
 * parameter cases, or improper calling sequences.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"
#include <stdio.h>

/*
 * Error message strings - see "sph_err_types.h" for error code IDs
 */ 

#ifndef THINK_C
static char *sphigs_errmsgs[] =
  {
   "UNUSED",
   "SPHIGS is not enabled yet!  Have you heard about SPH_begin()?\n",
   "You sent a bad rectangle (either your l>r, or your b>t).\n",
   "A structure is already open.  You attempted to open another one.\n",
   "No structure is currently open.  You can't do editing operations.\n",
   "You sent a bad structure-id.\n",
   "You sent a bad name.\n", 
   "You sent a bad view index.\n",
   "You attempted to change the window or viewport for view #0.\n",
   "You tried to move the element ptr outside the bounds of the structure.\n",
   "You referred to a label not found during a forward scan.\n",
   "You tried to delete an element where none exists.\n",
   "You tried to unpost a structure that is not known to be posted there.\n",
   "SPHIGS already enabled!  You enabled it twice or called\n functions out of order.\n",
   "You sent a bad 'update-method' (second parameter).\n",
   "You sent a bad vertex count (%d).\n",
   "You sent a bad facet count (%d).\n",
   "You referred to a device other than KEYBOARD.\n",
   "Pick corr. is illegal when the display is out-of-synch with the CSS!\n",
   "Pick correlation is illegal for a viewport using wireframe-raw mode.\n", 
   "You defined a polyhedron face or fill area that is not planar.\n", 
   "You sent a bad light source index.\n",
   "You sent a bad value (%d) when setting a maximum index value.\n",
   "You created a recursive network in a call to SPH__executeStructure.",
   "You sent an empty facet or vertex list when creating a new primitive\n",
   "Viewing parameter error: %s\n",
   "You sent an element-index-range where the first index\n was not <= the second one!\n",
   "The MAT3 matrix package encountered an error.  See SRGPlogfile.\n",
   "You tried to set a rendering mode that is either illegal or unsupported.\n",
   "Sorry!  I ran out of dynamic memory!\n",
   "You sent a color index that is beyond the available range.\n",
   "You can't set entries 0, 1, or any out-of-range entry of the color table.\n",
   
   "UNUSED"
};
#endif


extern char *srgp__identifierOfPackage;

#ifdef THINK_C
extern int   srgp__identifierOfMessages;
#else
extern char **srgp__identifierOfMessages;
#endif

#ifdef THINK_C
/* We hide this from gnu's compiler, which doesn't understand it. */
void SRGP__error (int errtype, ...);
#endif

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Report a user error and crash.
 * ------------------------------------------------------------------------- */
void
SPH__error (
   int errtype, 
   int arg1, int arg2, int arg3, int arg4, int arg5)
{
   srgp__identifierOfPackage = "SPHIGS";
#ifdef THINK_C
   srgp__identifierOfMessages = 129;
#else
   srgp__identifierOfMessages = sphigs_errmsgs;
#endif
   SRGP__error (errtype, arg1, arg2, arg3, arg4, arg5);
}
