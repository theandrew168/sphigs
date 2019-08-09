#ifndef lint
static char Version[]=
   "$Id: sph_edit.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_edit.c
 *
 *	Convenience methods for editing a structure's elements.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"
#include <assert.h> 
#include <string.h> 

/* Globals - also used in sph_elemdebug.c */
element 	 	 *element_ptr;			   /* current element */
int     	          element_ptr_index;		   /* its index */
static substruct_bitstring possible_no_longer_child = NULL;

/* Globals - used by sph_element.c */
structure *OPENSTRUCT;		       /* structure currently open (one only) */
int        ID_of_open_struct;	       /* its ID # */

#define MARK_BSP_REFRESH      					\
   if (OPENSTRUCT->last_valid_index < element_ptr_index)	\
      (OPENSTRUCT->last_valid_index = element_ptr_index)


/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Frees up any malloc'd areas associated with an element.
 * ------------------------------------------------------------------------- */
static void
SPH_cleanup_and_kill_element (
   element *elptr)
{
   switch (elptr->type) {
      
    case ELTYP__TEXT:
      free (elptr->info.textstring);
      break;

    case ELTYP__POLYHEDRON:
      SPH__freePolyhedron (elptr->data.poly);
      break;

    case ELTYP__POLYMARKER:
    case ELTYP__POLYLINE:
    case ELTYP__FILL_AREA:
      free (elptr->data.points);
      break;

    case ELTYP__EXECUTE_STRUCTURE:
      SPH__structureTable[elptr->data.value].refcount--;
      SPH__setBit (possible_no_longer_child, elptr->data.value);
      break;
      
   }

   free (elptr);
}

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Initializes the global list of structures.
 *
 * DETAILS :	All structures exist initially; they're simply empty.
 * ------------------------------------------------------------------------- */
void
SPH__init_structure_table (void)
{
   register i;

   bzero (SPH__structureTable, sizeof(structure)*(MAX_STRUCTURE_ID+1));
   for (i=0; i<=MAX_STRUCTURE_ID; i++) {
      SPH__clearBitstring (&(SPH__structureTable[i].child_list));
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Inserts given element in the currently open structure so that
 * 		it follows the currently active element.
 *
 * DETAILS :	Because internal-use only, ASSUMES THERE IS AN OPEN
 * 		STRUCTURE. 
 * ------------------------------------------------------------------------- */
void
SPH__insertElement (
   element *baby)
{
   if (element_ptr == NULL) {
      /* BEING INSERTED AT VERY BEGINNING OF STRUCTURE */
      baby->next = OPENSTRUCT->first_element;
      OPENSTRUCT->first_element = baby;
      baby->previous = NULL;
   }
   else {
      /* BEING INSERTED AFTER AT LEAST ONE EXTANT ELEMENT */
      baby->next = element_ptr->next;
      element_ptr->next = baby;
      baby->previous = element_ptr;
   }

   OPENSTRUCT->element_count++;
   if (baby->next == NULL)
      OPENSTRUCT->last_element = baby;
   else
      baby->next->previous = baby;
   element_ptr = baby;

   /* this could be smarter, since inserting new drawing primitives won't
      alter the remainder of the element list. */
   MARK_BSP_REFRESH;

   element_ptr_index++;
}

/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Opens a structure for appending or editing.
 * ------------------------------------------------------------------------- */
void
SPH_openStructure (
   int structID)
{
   SPH_check_system_state;
   SPH_check_no_open_structure;
   SPH_check_structure_id;

   ID_of_open_struct = structID;
   OPENSTRUCT = &(SPH__structureTable[structID]);
   element_ptr_index = OPENSTRUCT->element_count;
   element_ptr = OPENSTRUCT->last_element;
   SPH__structureCurrentlyOpen = TRUE;

   SPH__clearBitstring (&possible_no_longer_child);
}
   
/* -------------------------------------------------------------------------
 * DESCR   :	Closes a previously opened structure.
 * ------------------------------------------------------------------------- */
void
SPH_closeStructure (void)
{
   static substruct_bitstring BB = NULL;

   SPH_check_system_state;
   SPH_check_open_structure;

   /* IF AN INVOCATION WAS DESTROYED BY THE EDITING.... */
   if ( ! SPH__bitstringIsClear(possible_no_longer_child)) {
      register element *elptr;


      /* RECALC CHILD_LIST */
      SPH__clearBitstring (&(OPENSTRUCT->child_list));
      elptr = OPENSTRUCT->first_element;
      while (elptr) {
	      if (elptr->type == ELTYP__EXECUTE_STRUCTURE)
	         SPH__setBit (OPENSTRUCT->child_list, elptr->data.value);
         elptr = elptr->next;
      }
      
      /* TEST FOR OBSOLESCENCE OF DEPENDENCIES. */
      SPH__clearBitstring (&BB);  /* forces a malloc the 1st time around */
      SPH__andBitstrings (BB, possible_no_longer_child, OPENSTRUCT->child_list);
      /* NOW: BB is a list of all structures that are definitely
       *   children AND that were suspected of having been lost by
       *   a deletion of an invocation.
       * IF BB is equal to the list of all structures suspected of
       *   having been lost, then it must be true that NO
       *   children were lost due to editing in this session.
       * IF BB is NOT equal to...,  then it must be true that
       *   at least one child was lost, and some views' descendent-lists
       *   may now be obsolete!
       */
      if ( ! SPH__bitstringsAreEqual (BB, possible_no_longer_child)) 
         /* All views that own S now have obsolete descendent lists! */
         VIEWOPT__afterChildLoss (ID_of_open_struct);
   }

   SPH__structureCurrentlyOpen = FALSE;
   SPH__refresh_structure_close (ID_of_open_struct);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Make the requested index the "currently active element".
 * 		NOTE: 0 means set element to even before the very first
 * 		element; 1 means set elptr to very first element.
 * ------------------------------------------------------------------------- */
void
SPH_setElementPointer (
   int index)
{
   register i;

   SPH_check_system_state;
   SPH_check_open_structure;

   if (index == 0)
      element_ptr = NULL;
   else
      if ((index > OPENSTRUCT->element_count) || (index < 0))
	 SPH__error (ERR_BAD_SET_ELPTR);
      else {
	 element_ptr = OPENSTRUCT->first_element;
	 for (i=1; i<index; i++)
	    element_ptr = element_ptr->next;
      }

   element_ptr_index = index;
}
      
/* -------------------------------------------------------------------------
 * DESCR   :	Offsets the element pointer forward by the given number of
 * 		elements.
 *
 * DETAILS :	Internal call to SPH__setElptr does the verification work
 * 		on the index.
 * ------------------------------------------------------------------------- */
void
SPH_offsetElementPointer (
   int delta)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   element_ptr_index += delta;
   SPH_setElementPointer (element_ptr_index);
}


/* -------------------------------------------------------------------------
 * DESCR   :	Searches forward in the structure list for the specified
 * 		label, and makes that the current element.  Search begins
 * 		with the element after (to the right) of the current one.
 *
 * DETAILS :	Question: should I include the current element in the search?
 * 		Or does the search begin with the first element after the
 * 		current one? 
 * 		This version gives fatal error if label not found.
 * ------------------------------------------------------------------------- */
void
SPH_moveElementPointerToLabel (
   int lab)
{
   SPH_check_system_state;
   SPH_check_open_structure;

   do {
      /* ADVANCE ELEMENT POINTER */
      element_ptr_index++;
      if (element_ptr_index == 1)
	 element_ptr = OPENSTRUCT->first_element;
      else
	 element_ptr = element_ptr->next;

      if ( ! element_ptr)
	 /* REACHED END: NOT FOUND: FATAL ERROR */
	 SPH__error (ERR_LABEL_NOT_FOUND);
      
   } while (  (element_ptr->type != ELTYP__LABEL)
	   || (element_ptr->data.value != lab) );
	    /* FOUND */
}

/* -------------------------------------------------------------------------
 * DESCR   :	Deletes the current element.  The element pointer is left
 * 		pointing to the element just before (to the left) the one to
 * 		be killed, if any.
 * ------------------------------------------------------------------------- */
void
SPH_deleteElement (void)
{
   element *el_to_die;

   SPH_check_system_state;
   SPH_check_open_structure;
   if (element_ptr == NULL)
      SPH__error (ERR_NO_ELEMENT_TO_DELETE);

   /* FIRST, HANDLE ITS RELATIONSHIP TO ITS PREVIOUS ELEMENTS */
   if (element_ptr->previous == NULL)
      OPENSTRUCT->first_element = element_ptr->next;
   else
      element_ptr->previous->next = element_ptr->next;
   /* THEN, HANDLE ITS RELATIONSHIP TO ITS NEXT ELEMENTS */
   if (element_ptr->next == NULL)
      OPENSTRUCT->last_element = element_ptr->previous;
   else
      element_ptr->next->previous = element_ptr->previous;

   /* UPDATE ELEMENT-COUNT AND ELEMENT-POINTER */
   OPENSTRUCT->element_count--;
   el_to_die = element_ptr;
   element_ptr = el_to_die->previous;

   element_ptr_index--;
   MARK_BSP_REFRESH;


   SPH_cleanup_and_kill_element (el_to_die);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Deletes all elements within and on the bounds of the given
 * 		range.  The element pointer is left pointing to the element
 *		just prior to the first element deleted.
 * ------------------------------------------------------------------------- */
void
SPH_deleteElementsInRange (
   int first_index, 
   int second_index)
{
   element *first_el_to_die, *last_el_to_die;
#  define number_to_be_killed    (second_index - first_index + 1)

   SPH_check_system_state;
   SPH_check_open_structure;
   
   SPH_setElementPointer (first_index);
   first_el_to_die = element_ptr;
   SPH_setElementPointer (second_index);
   last_el_to_die = element_ptr;

   SPH_check_elindex_range;
   if (first_el_to_die == NULL)
      SPH__error (ERR_NO_ELEMENT_TO_DELETE);

   /* FIRST, HANDLE ITS RELATIONSHIP TO ITS PREVIOUS ELEMENTS */
   if (first_el_to_die->previous == NULL)
      OPENSTRUCT->first_element = last_el_to_die->next;
   else
      first_el_to_die->previous->next = last_el_to_die->next;

   /* THEN, HANDLE ITS RELATIONSHIP TO ITS NEXT ELEMENTS */
   if (last_el_to_die->next == NULL)
      OPENSTRUCT->last_element = first_el_to_die->previous;
   else
      last_el_to_die->next->previous = first_el_to_die->previous;

   /* UPDATE ELEMENT-COUNT AND ELEMENT-POINTER */
   OPENSTRUCT->element_count -= number_to_be_killed;
   element_ptr = first_el_to_die->previous;

   element_ptr_index = first_index - 1;
   MARK_BSP_REFRESH;

   /* KILL AND CLEANUP THE ELEMENTS */
   do {
      SPH_cleanup_and_kill_element (first_el_to_die);
      if (first_el_to_die == last_el_to_die)
	 break;
      else
	 first_el_to_die = first_el_to_die->next;
   } while (1);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Deletes all elements in the structure that lie between the
 * 		given labels, but not the labels themselves.  The element
 * 		pointer is left pointing to the first label.
 * ------------------------------------------------------------------------- */
void
SPH_deleteElementsBetweenLabels (
   int lab1,
   int lab2)
{
   int first_index, second_index;

   SPH_check_system_state;
   SPH_check_open_structure;

   SPH_moveElementPointerToLabel (lab1);
   first_index = element_ptr_index;
   SPH_moveElementPointerToLabel (lab2);
   second_index = element_ptr_index;
   if (first_index == (second_index-1))
      /* do nothing: nothing lies between the label elements */;
   else
      SPH_deleteElementsInRange (first_index+1, second_index-1);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Returns the index of the current element.
 * ------------------------------------------------------------------------- */
int
SPH_inquireElementPointer (void)
{
   SPH_check_system_state;
   SPH_check_open_structure;
   return element_ptr_index;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Copies the elements of another structure into the open
 * 		structure after the current element, which is updated to
 * 		point to the last element inserted.  A structure can be
 * 		copied into itself.
 * ------------------------------------------------------------------------- */
void
SPH_copyStructure (
   int structID )
{
   structure *template;
   element *first_new, *last_new, *elptr, *clone;
   int count = 0;

   SPH_check_system_state;
   SPH_check_open_structure;
   SPH_check_structure_id;

   template = &(SPH__structureTable[structID]);
   if (template->element_count == 0)
      return;
   
   /* generate list of cloned elements */
   first_new = NULL;
   for (elptr = template->first_element;  elptr;  elptr = elptr->next) {
      clone = SPH__copyElement (elptr);

      clone->previous = clone->next = NULL;
      if (!first_new)
	 first_new = clone;
      else {
	 clone->previous = last_new;
	 last_new->next = clone;
      }
      last_new = clone;
      ++count;
   }
   assert (first_new);
   assert (count == template->element_count);


   /* add it to the structure */
   if (element_ptr == NULL) {
      last_new->next = OPENSTRUCT->first_element;
      OPENSTRUCT->first_element = first_new;
   } else {
      last_new->next = element_ptr->next;
      element_ptr->next = first_new;
   }
   first_new->previous = element_ptr;
   if (last_new->next == NULL)
      OPENSTRUCT->last_element = last_new;
   else
      last_new->next->previous = last_new;

   /* order important here!  in case struct is copied to itself */
   element_ptr_index += template->element_count;
   element_ptr = last_new;
   OPENSTRUCT->element_count += template->element_count;

   MARK_BSP_REFRESH;
}
