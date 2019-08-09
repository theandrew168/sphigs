#ifndef lint
static char Version[]=
   "$Id: sph_filter.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_filter.c
 *
 * 	Filter features for highlighting and invisibility on namesets.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"

#  define ADD_FILTER(FILTER, NAME)			\
   if (!SPH__testBit (FILTER, (NAME) - 1)) {		\
      SPH__setBit (FILTER, (NAME) - 1);			\
      SPH__refresh_filter (viewIndex);			\
   }

#  define REMOVE_FILTER(FILTER, NAME)			\
   if (SPH__testBit (FILTER, (NAME) - 1)) {		\
      SPH__clearBit (FILTER, (NAME) - 1);		\
      SPH__refresh_filter (viewIndex);			\
   }
   
   
/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Adds a name to the viewport's invisibility filter.
 * ------------------------------------------------------------------------- */
void
SPH_addToInvisFilter (
   int  viewIndex, 
   name name)
{
   SPH_check_system_state;
   SPH_check_view_index;
   SPH_check_name;
   
   ADD_FILTER (SPH__viewTable [viewIndex].invisFilter, name);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Removes a name from the viewport's invisibility filter.
 * ------------------------------------------------------------------------- */
void
SPH_removeFromInvisFilter (
   int  viewIndex, 
   name name)
{
   SPH_check_system_state;
   SPH_check_view_index;
   SPH_check_name;

   REMOVE_FILTER (SPH__viewTable [viewIndex].invisFilter, name);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Adds a name to the viewport's highlighting filter.
 * ------------------------------------------------------------------------- */
void
SPH_addToHiliteFilter (
   int  viewIndex, 
   name name)
{
   SPH_check_system_state;
   SPH_check_view_index;
   SPH_check_name;
   
   ADD_FILTER (SPH__viewTable [viewIndex].hiliteFilter, name);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Removes a name from the viewport's highlighting filter.
 * ------------------------------------------------------------------------- */
void
SPH_removeFromHiliteFilter (
   int  viewIndex, 
   name name)
{
   SPH_check_system_state;
   SPH_check_view_index;
   SPH_check_name;

   REMOVE_FILTER (SPH__viewTable [viewIndex].hiliteFilter, name);
}

