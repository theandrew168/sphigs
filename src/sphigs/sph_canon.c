#ifndef lint
static char Version[]=
   "$Id: sph_canon.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_canon.c
 *
 *	Coordinate conversions between NPC and UVN
 * ------------------------------------------------------------------------- */
   
#include "HEADERS.h"
#include <stdio.h>
#include "sphigslocal.h"
#include <assert.h>
   
/* -------------------------------------------------------------------------
 * DESCR   :	Calculates the uvn vertices from the modeling coordinates.
 * ------------------------------------------------------------------------- */
void
SPH__map_to_camera (
   view_spec *vs)
{
   register int		count;
   register MAT3hvec   *mcVertices;
   register MAT3hvec   *uvnVertices;
   
   /* Allocate space for the npc coords to be generated in */
   if (vs->uvnVertices)
      free (vs->uvnVertices);
   ALLOC_RECORDS (vs->uvnVertices, MAT3hvec, vs->vertexArraySize);
   
   /* Map using the view-mapping matrix to get NDC coords */
   mcVertices = vs->mcVertices;
   uvnVertices = vs->uvnVertices;
   for (count = 0; count < vs->vertexCount; count++) {
      MAT3mult_hvec
	 (*uvnVertices, *mcVertices, currentMCtoUVNxform, 1);
      ++mcVertices;
      ++uvnVertices;
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Calculates the npc vertices from the uvn, negates the z
 * 		coordinate to keep the same handedness as the uvn space 
 * ------------------------------------------------------------------------- */
void
SPH__map_to_canon (
   view_spec *vs)
{
   register int		count;
   register MAT3hvec   *mcVertices;
   register MAT3hvec   *uvnVertices;
   register MAT3hvec   *npcVertices;
   
   /* Allocate space for the npc coords to be generated in */
   if (vs->npcVertices)
      free (vs->npcVertices);
   ALLOC_RECORDS (vs->npcVertices, MAT3hvec, vs->vertexArraySize);
   
   /* Map using the view-mapping matrix to get NDC coords */
   uvnVertices = vs->uvnVertices;
   npcVertices = vs->npcVertices;
   for (count = 0; count < vs->vertexCount; count++) {
      MAT3mult_hvec
	 (*npcVertices, *uvnVertices, vs->vm_matrix, 1);
      (*npcVertices)[Z] *= -1;
      ++npcVertices;
      ++uvnVertices;
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Calculates the pdc points from the npc vertices.  Changes pdc
 * 		points in object struct, and assumes map_to_canon was called
 * 		prior.  
 * ------------------------------------------------------------------------- */
void  
SPH__map_to_pdc (
   view_spec *vs)
{
   MAT3hvec		   result_vec;
   register MAT3hvec      *scanNPCvertex;
   register srgp__point   *scanPDCvertex;
   MAT3vec		   tempVertex;
   register int	           count;

   scanNPCvertex = vs->npcVertices;

   if (vs->pdcVertices)
        free (vs->pdcVertices);

   ALLOC_RECORDS (vs->pdcVertices, srgp__point, vs->vertexCount);
   assert( vs->pdcVertices != NULL );
   
   scanPDCvertex = vs->pdcVertices;
   
   for (count = 0; count < vs->vertexCount; count++) {
      tempVertex[0] = (* scanNPCvertex)[0] * SPH__ndcSpaceSizeInPixels;
      tempVertex[1] = (* scanNPCvertex)[1] * SPH__ndcSpaceSizeInPixels;
      scanPDCvertex->x = irint( tempVertex[0] );
      scanPDCvertex->y = irint( tempVertex[1] );
      scanPDCvertex++;
      scanNPCvertex++;
   }
}
