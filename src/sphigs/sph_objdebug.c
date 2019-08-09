#ifndef lint
static char Version[]=
   "$Id: sph_objdebug.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_objdebug.c
 *
 *	Diagnostic output for object lists
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "sphigslocal.h"


void SPH__print_objects_in_view( );

/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Counts # objects in the list
 * ------------------------------------------------------------------------- */
static int
CountObjects(
   obj *current)
{
   register i = 0;
     
   while( current != NULL ) {
      i++;
      current = current->next;
   }
     
   return( i );
}

/* -------------------------------------------------------------------------
 * DESCR   :	Diagnostic output for a single object
 * ------------------------------------------------------------------------- */
static void
PrintObject(
   view_spec *vs,
   obj 	     *current,
   int 	      where)
{
   register i;
   FILE * stream;
     
   assert( vs != NULL );
   assert( current != NULL );
   if( where == 1 )
      stream = stderr;
   else
      stream = stdout;
     
   fprintf( stream, "[ ]\tobject address = 0x%x\n",
	   (long) current );

   fprintf( stream, "\tobject type = " );
   switch( current->type ) {
    case OBJ_FACE:
      printf( "face\n" );
      break;
    case OBJ_LINE:
      printf( "line\n" );
      break;
    case OBJ_TEXT:
      printf( "text\n" );
      break;
    case OBJ_POINT:
      printf( "point\n" );
      break;
   }	
   
   fprintf( stream, "\tnormal = [ %lf %lf %lf %lf ]\n", current->normal[0],
	   current->normal[1], current->normal[2], current->normal[3] );

   fprintf( stream, "\tp normal = [ %lf %lf %lf %lf ]\n", current->p_normal[0],
	   current->p_normal[1], current->p_normal[2], current->p_normal[3] );

   fprintf( stream, "\tmin = [ %lf %lf %lf %lf ]\n", current->min[0],
	   current->min[1], current->min[2], current->min[3] );

   fprintf( stream, "\tmax = [ %lf %lf %lf %lf ]\n", current->max[0],
	   current->max[1], current->max[2], current->max[3] );

   fprintf( stream, "\tintensity = %lf\n", 
	   current->intensity );

   fprintf( stream, "\tdirected distance = %lf\n",
	   current->directed_distance );
     


   switch( current->type ) {
    case OBJ_FACE:
      fprintf( stream, "\tinterior color = %d\n",
	      current->attributes.interior_color );
      fprintf( stream, "\tnumPoints = %d\n",
	      current->data.face.numPoints );
      fprintf( stream, "\tpoints = 0x%x\n",
	      (long) current->data.face.points );

      assert( current->data.face.points != NULL );
      fprintf( stream, "\tarray = " );
      for( i = 0; i < current->data.face.numPoints; i++ )
	 fprintf( stream, "%d ", (int) current->data.face.points[ i ] );
      fprintf( stream, "\n" );

      for( i = 0; i < current->data.face.numPoints; i++ ) {
	 if( vs->npcVertices != NULL ) {
	    fprintf( stream, "\t\t%d (%lf %lf %lf) (%lf %lf %lf)\n",
		    current->data.face.points[ i ],
		    vs->uvnVertices[current->data.face.points[i] ][0],
		    vs->uvnVertices[current->data.face.points[i] ][1],
		    vs->uvnVertices[current->data.face.points[i] ][2],
		    vs->npcVertices[current->data.face.points[i] ][0],
		    vs->npcVertices[current->data.face.points[i] ][1],
		    vs->npcVertices[current->data.face.points[i] ][2] );
	 } else {
	    fprintf( stream, "\t\t%d (%lf %lf %lf)\n",
		    current->data.face.points[ i ],
		    vs->uvnVertices[current->data.face.points[i] ][0],
		    vs->uvnVertices[current->data.face.points[i] ][1],
		    vs->uvnVertices[current->data.face.points[i] ][2] );
	 }	
      }
	  
     
      fprintf( stream, "\tp_min = [ %lf %lf %lf %lf ]\n", current->p_min[0],
	      current->p_min[1], current->p_min[2], current->p_min[3] );
     
      fprintf( stream, "\tp_max = [ %lf %lf %lf %lf ]\n", current->p_max[0],
	      current->p_max[1], current->p_max[2], current->p_max[3] );
	  
      break;
	  

    case OBJ_LINE:
      fprintf( stream, "\tend1 = (%lf %lf %lf) (%lf %lf %lf) \n",
	      vs->uvnVertices [current->data.line.pt[0] ][0],
	      vs->uvnVertices [current->data.line.pt[0] ][1],
	      vs->uvnVertices [current->data.line.pt[0] ][2],
	      vs->npcVertices [current->data.line.pt[0] ][0],
	      vs->npcVertices [current->data.line.pt[0] ][1],
	      vs->npcVertices [current->data.line.pt[0] ][2]);

      fprintf( stream, "\tend2 = (%lf %lf %lf) (%lf %lf %lf) \n",
	      vs->uvnVertices [current->data.line.pt[1] ][0],
	      vs->uvnVertices [current->data.line.pt[1] ][1],
	      vs->uvnVertices [current->data.line.pt[1] ][2],
	      vs->npcVertices [current->data.line.pt[1] ][0],
	      vs->npcVertices [current->data.line.pt[1] ][1],
	      vs->npcVertices [current->data.line.pt[1] ][2]);
      break;
	  

    case OBJ_TEXT:
      fprintf( stream, "\ttext = %s\n", current->data.text.text );
      break;
	  

    case OBJ_POINT:
      break;
   }  
}

/* -------------------------------------------------------------------------
 * DESCR   :	Prints diagnostics for all objects in a list.
 * ------------------------------------------------------------------------- */
static void
PrintObjects (
   view_spec *vs,
   obj *current)
{
   assert( vs != NULL );
   fprintf( stdout, "START OF LIST\n" );
   while (current) {
      PrintObject( vs, current, 0 );
      current = current -> next;
   }
   fprintf( stdout, "END OF LIST\n" );
}

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Prints all objects shown in the given viewport
 * ------------------------------------------------------------------------- */
void
SPH__print_objects_in_view (
   view_spec *vs)
{
   assert( vs != NULL );
   PrintObjects( vs, vs->objects );
}

