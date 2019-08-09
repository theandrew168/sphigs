#ifndef lint
static char Version[]=
   "$Id: sph_state.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/*---------------------------------------------------------------------------
 * sph_state.c
 *
 *	Basic adminstrative routines
 *---------------------------------------------------------------------------*/

#include "HEADERS.h"
#define SPHIGS_BOSS
#include "sphigslocal.h"
#include <math.h>

/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Called by SRGP when window gets resized
 * ------------------------------------------------------------------------- */
static int
ResizeCallback (
   int width,
   int height
   )
{
   register vi;

   SPH__ndcSpaceSizeInPixels = (width>height) ? height : width;
   for (vi=0; vi<=MAX_VIEW_INDEX; vi++) {
      SPH__updateViewInfo (vi);
      SPH__refresh_viewport_change (vi, SPH__viewTable[vi].pdc_viewport);
   }
}   

/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Initializes SPHIGS' global state and enables the package
 * ------------------------------------------------------------------------- */
/*!*/
void
SPH_begin (
   int width,	
   int height,
   int color_planes_desired, 
   int shades_per_flexicolor
   )
{
   SPH_check_no_system_state;

   SPH__enabled = TRUE;

   /* What should be the size in pixels of 1 NPC unit? */
   SPH__ndcSpaceSizeInPixels = (width>height) ? height : width;
 
   SRGP_begin ("SPHIGS", width, height, color_planes_desired, FALSE);
   SRGP_tracing(TRUE);
   ALLOC_RECORDS (SPH__viewTable, view_spec, MAX_VIEW_INDEX+1);
   ALLOC_RECORDS (SPH__structureTable, structure, MAX_STRUCTURE_ID+1);
   
   BYTES_PER_BITSTRING = ceil((MAX_STRUCTURE_ID+1.0)/8.0);

   SPH__allocNBitstring (currentNameset, MAX_NAME);
   SRGP_allowResize (TRUE);
   SRGP_registerResizeCallback (ResizeCallback);

   SPH__init_structure_table();
   SPH__initDefaultAttributeGroup();
   SPH__init_view_table();
  
   SPH_setImplicitRegenerationMode (ALLOWED);

   SPH__initColorTable (shades_per_flexicolor);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Shuts down the package
 * ------------------------------------------------------------------------- */
/*!*/
void
SPH_end (void)
{
   register int i;

   SPH_check_system_state;

   for (i=0; i<=MAX_STRUCTURE_ID; i++)
      SPH__freeBitstring (SPH__structureTable[i].child_list);
   FREE (SPH__structureTable);

   for (i=0; i<=MAX_VIEW_INDEX; i++) {
      SPH__freeBitstring (SPH__viewTable[i].invisFilter);
      SPH__freeBitstring (SPH__viewTable[i].hiliteFilter);
      FREE (SPH__viewTable[i].lights);
   }
   FREE (SPH__viewTable);

   SRGP_end();
   SPH__enabled = FALSE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Allows application to set size of structure table.  Must be
 * 	        called *before* SPHIGS is enabled.
 * ------------------------------------------------------------------------- */
void
SPH_setMaxStructureID (
   int id
   )
{
   SPH_check_no_system_state;
   SPH_check_max_id;
   MAX_STRUCTURE_ID = id;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Allows application to set size of view table.  Must be 
 *		called *before* SPHIGS is enabled.
 * ------------------------------------------------------------------------- */
void
SPH_setMaxViewIndex (
   int id
   )
{
   SPH_check_no_system_state;
   SPH_check_max_id;
   MAX_VIEW_INDEX = id;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Allows application to set number of point light sources per
 * 		view.  Must be called *before* SPHIGS is enabled.
 * ------------------------------------------------------------------------- */
void
SPH_setMaxLightSourceIndex (
   int id
   )
{
   SPH_check_no_system_state;
   SPH_check_max_id;
   MAX_LIGHT_INDEX = id;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Allows application to set maximum number of names.  Must be
 * 		called *before* SPHIGS is enabled. 
 * ------------------------------------------------------------------------- */
void
SPH_setMaxNameID (
   int id)
{
   SPH_check_no_system_state;
   SPH_check_max_id;
   MAX_NAME = id;
}

