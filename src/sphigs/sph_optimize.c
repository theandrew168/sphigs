#ifndef lint
static char Version[]=
   "$Id: sph_optimize.c,v 1.3 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_optimize.c
 *
 *  	Optimizations for redisplay traversal.
 *
 *
 * DETAILS :	One simple form of optimization is to record, for each view
 * 		V, a list of the structures that are subordinates of view V.
 * 		This data must be updated in the following ways:     
 *
 *
 * 		WHEN A STRUCTURE S IS POSTED TO VIEW V: 
 * 			Add S to the subord-list for view V.
 * 			Traverse the network rooted at S:
 * 			Add each encountered structure to the subord-list for
 * 				view V. 
 * 		
 * 		WHEN A STRUCTURE S IS UNPOSTED FROM VIEW V:
 * 			Completely recalculate view V's subord-list.
 * 		
 * 		WHEN A STRUCTURE P BECOMES A NEW CHILD OF STRUCTURE S:
 * 			Create a temporary subord-list, initially clear.
 * 			Add P to the temp list.
 * 			Traverse the network rooted at P:
 * 				Add each encountered structure to the temp
 * 						subord-list. 	
 * 			For each view V:
 * 				If structure S is a subord of view V:
 * 				   "OR" the temp subord-list with view V's
 * 						current subord-list. 
 * 			
 * 		WHEN A STRUCTURE S IS EDITED IN SUCH A WAY THAT IT POSSIBLY
 * 				LOST A CHILD: 
 * 			For each view V:
 * 				If structure S is posted to view V:
 * 				   Completely recalculate view V's subord-list.
 * 			
 *
 * 		Here's the algorithm for completely recalculating view V's
 * 				subord list:	 
 * 			Clear the subord-list for view V.
 * 			For each structure P still posted to view V:
 * 			    Traverse the network rooted at P:
 * 			        Add each encountered structure to the
 * 						subord-list for view V. 
 *
 * ------------------------------------------------------------------------- */
			
#include "HEADERS.h"
#include "sphigslocal.h"


static substruct_bitstring temp_descendent_list = NULL;


/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Uses an optimization algorithm to avoid unnecessary
 * 		re-traversal.  Warning: assumes bit #sid already set!
 * ------------------------------------------------------------------------- */
static void
MergeDescendentList (
   int sid)
{
   register int s;
   
   for (s=0; s<=MAX_STRUCTURE_ID; s++)
      /* IF STRUCTURE s IS AN IMMEDIATE CHILD OF sid THEN... */
      if (SPH__testBit(SPH__structureTable[sid].child_list, s))
         /* RECURSE ONLY IF STRUCTURE s IS NOT YET RECORDED */
         if ( ! SPH__testBit(temp_descendent_list, s)) {
	    SPH__setBit (temp_descendent_list, s);
            MergeDescendentList (s);
         }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Find "descendents" of a structure hierarchy
 * ------------------------------------------------------------------------- */
static void
CalculateDescendentListForOneStructure (
   int struct_ID)
{
   
   SPH__clearBitstring (&temp_descendent_list);
   SPH__setBit (temp_descendent_list, struct_ID);
   MergeDescendentList (struct_ID);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Find "descendents" of a viewport's root structure 
 * ------------------------------------------------------------------------- */
static void 
CalculateDescendentListForView (
   view_spec *vs)
{
   root_header *rptr;
   
   SPH__clearBitstring (&temp_descendent_list);
   
   for (rptr=vs->highestOverlapNetwork; rptr; 
	rptr=rptr->nextLowerOverlapRoot) {	
      SPH__setBit (temp_descendent_list, rptr->root_structID);
      MergeDescendentList (rptr->root_structID);
   }
}

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Optimize after a structure gets posted.
 * ------------------------------------------------------------------------- */
void 
VIEWOPT__afterNewPosting (
   view_spec *v,
   int struct_ID)
{
   CalculateDescendentListForOneStructure (struct_ID);
   SPH__mergeBitstring (v->descendent_list, temp_descendent_list);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Optimize after a structure gets unposted.
 * ------------------------------------------------------------------------- */
void 
VIEWOPT__afterUnposting (
   view_spec *v, 
   int struct_ID)
{
   CalculateDescendentListForOneStructure (struct_ID);
   SPH__mergeBitstring (v->descendent_list, temp_descendent_list);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Optimize after a structure gains a child.
 * ------------------------------------------------------------------------- */
void
VIEWOPT__newExecuteStructure (
   int ID_of_new_parent, 
   int ID_of_new_child)
{
   register int v;
   
   CalculateDescendentListForOneStructure (ID_of_new_child);
   for (v=0; v<=MAX_VIEW_INDEX; v++)
      if (SPH__testBit(SPH__viewTable[v].descendent_list, ID_of_new_parent))
	 SPH__mergeBitstring 
	    (SPH__viewTable[v].descendent_list, temp_descendent_list);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Optimize after a structure loses a child.
 * ------------------------------------------------------------------------- */
void 
VIEWOPT__afterChildLoss (
   int ID_of_changed_parent)
{
   register int v;
   
   for (v=0; v<=MAX_VIEW_INDEX; v++)
      if (SPH__testBit(SPH__viewTable[v].descendent_list, ID_of_changed_parent))
	 CalculateDescendentListForView (&(SPH__viewTable[v]));
}
