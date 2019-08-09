#ifndef lint
static char Version[]=
   "$Id: sph_post.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_post.c
 *
 *	Display posting for structures
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"


#define PROPER_VIEW  SPH__viewTable[viewIndex]


/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Posts a structure for display.
 *
 * DETAILS :	I make no attempt to verify that this very structure hasn't
 * 		already been posted to this very view.
 *
 * 		Strange thing: let's say that I post while regeneration is
 * 		suppressed.  And immediately obtain a pointer measure.  It
 * 		would be possible for an object in the newly posted network
 * 		to be "picked" via the correlation even though it has never
 * 		appeared on the screen! 
 * ------------------------------------------------------------------------- */
void
SPH_postRoot (
   int structID, 
   int viewIndex)
{
   root_header *baby_network;

   SPH_check_system_state;
   SPH_check_no_open_structure;
   SPH_check_structure_id;
   SPH_check_view_index;

   ALLOC_RECORDS (baby_network, root_header, 1);
   baby_network->root_structID = structID;

   SPH__structureTable[structID].refcount++;

   /* ADD TO VIEW DATABASE: to have highest overlap in its own view's list */
   baby_network->nextHigherOverlapRoot = NULL;
   baby_network->nextLowerOverlapRoot = PROPER_VIEW.highestOverlapNetwork;
   if (PROPER_VIEW.lowestOverlapNetwork == NULL)
      PROPER_VIEW.lowestOverlapNetwork = baby_network;
   else
      PROPER_VIEW.highestOverlapNetwork->nextHigherOverlapRoot = baby_network;
   PROPER_VIEW.highestOverlapNetwork = baby_network;

   VIEWOPT__afterNewPosting (&PROPER_VIEW, structID);
   SPH__refresh_post (viewIndex);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Removes a structure from a display list.
 * ------------------------------------------------------------------------- */
void
SPH_unpostRoot (
   int structID, 
   int viewIndex)
{
   root_header *network_to_die;


   SPH_check_system_state;
   SPH_check_no_open_structure;
   SPH_check_structure_id;
   SPH_check_view_index;

   /* LOOK FOR THE ROOT HEADER BY SCANNING VIEW'S NETWORK LIST */
   network_to_die = PROPER_VIEW.highestOverlapNetwork;
   while (network_to_die->root_structID != structID) {
      network_to_die = network_to_die->nextLowerOverlapRoot;
      if (network_to_die == NULL)
	 SPH__error (ERR_UNPOST_NONEXTANT_ROOT);
   }

   /* DELETE W/ HIGHER OVERLAPPERS */
   if (network_to_die->nextHigherOverlapRoot == NULL)
      PROPER_VIEW.highestOverlapNetwork = network_to_die->nextLowerOverlapRoot;
   else
      network_to_die->nextHigherOverlapRoot->nextLowerOverlapRoot =
	 network_to_die->nextLowerOverlapRoot;
   /* DELETE W/ LOWER OVERLAPPERS */
   if (network_to_die->nextLowerOverlapRoot == NULL)
      PROPER_VIEW.lowestOverlapNetwork = network_to_die->nextHigherOverlapRoot;
   else
      network_to_die->nextLowerOverlapRoot->nextHigherOverlapRoot =
	 network_to_die->nextHigherOverlapRoot;

   SPH__structureTable[structID].refcount--;

   VIEWOPT__afterUnposting (&PROPER_VIEW, structID);
   SPH__refresh_unpost (viewIndex);
   
   free (network_to_die);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Removes all structures from a viewport's display list.
 * ------------------------------------------------------------------------- */
void
SPH_unpostAllRoots (
   int viewIndex)
{
   root_header *network_to_die, *next_network_to_die;

   SPH_check_system_state;
   SPH_check_no_open_structure;
   SPH_check_view_index;

   network_to_die = PROPER_VIEW.highestOverlapNetwork;
   while (network_to_die != NULL) {
      next_network_to_die = network_to_die->nextLowerOverlapRoot;
      SPH__structureTable[network_to_die->root_structID].refcount--;
      free (network_to_die);
      network_to_die = next_network_to_die;
   }
   
   PROPER_VIEW.highestOverlapNetwork = PROPER_VIEW.lowestOverlapNetwork = NULL;
   
   SPH__clearBitstring (&(PROPER_VIEW.descendent_list));
   SPH__refresh_unpost (viewIndex);
}
