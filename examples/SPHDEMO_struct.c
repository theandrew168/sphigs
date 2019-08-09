#include <stdio.h>
#include "SPHDEMO.h"
#include "SPHDEMO_struct.h"
#include "SPHDEMO_view.h"


/* ------------------------------ Declarations ----------------------------- */

#ifndef COPY_BYTE
#ifdef THINK_C
#define COPY_BYTE(TO, FROM, TYPE, NUM)	memcpy(TO, FROM, (NUM) * sizeof(TYPE))
#else
#define COPY_BYTE(TO, FROM, TYPE, NUM)	bcopy(FROM, TO, (NUM) * sizeof(TYPE))
#endif   
#endif

extern void
SPH_displayStructureNetwork (	/* in sph_elemdebug.c */
   int   viewIndex,
   char *struct_names[],
   char *label_names[],
   char *pickid_names[],
   char *nameset_names[]);

#define DISPLAY_STRUCT							\
SPH_displayStructureNetwork (PERSPECTIVE_VIEW,				\
			     struct_names, NULL, pickid_names, NULL);	\
getchar();

#undef  DISPLAY_STRUCT
#define DISPLAY_STRUCT

/* ----------------------------- String Tables ----------------------------- */

static char *struct_names[] = {
   "CAMERA",
   "HOUSE",
   "STREET", 
   "NEIGHBORHOOD",
   "FAKE_HOUSE",
   "FAKE_NEIGHBORHOOD_TOP",  
   "FAKE_NEIGHBORHOOD_SIDE",
   "GRID",
   "TEXT",
   "FAKE_HOUSE_TOP",
   "FAKE_HOUSE_SIDE",
   "AXES",
   "CHIMNEY",
   "SPHERE",

   NULL
};

static char *pickid_names[] = {
   NULL,
   "pickHouse",
};

/************************************************************************/
/* Structure Editing Routines						*/
/************************************************************************/

/*----------------------------------------------------------------------*/
void AddChimneyToHouse (int housestruct)
{
   SPH_openStructure (housestruct);
   SPH_setPickIdentifier (1);
   SPH_executeStructure (CHIMNEY_STRUCT);
   SPH_closeStructure ();
}
/*----------------------------------------------------------------------*/
void TakeChimneyFromHouse (int housestruct)
{
   SPH_openStructure (housestruct);
   SPH_deleteElement ();
   SPH_deleteElement ();
   SPH_closeStructure ();
}

/************************************************************************/
/* Structure Creation Routines						*/
/************************************************************************/

/*----------------------------------------------------------------------*/
void CreateTextStructure (void)
{
   point textorigin = {0.1, 0.95, 0.5};

#  define CarriageReturn   textorigin[Y]-=0.05

   /* SRGP_loadFont (1, "Times.14"); */
   SPH_openStructure (TEXT_STRUCT);
      /* SPH_setTextFont (1); */

      SPH_setTextColor (red);
      SPH_text (textorigin, "Red = View Ref. Point");
      CarriageReturn;

      SPH_setTextColor (orange);
      SPH_text (textorigin, "Orange = Projection Ref. Point");
      CarriageReturn;

      SPH_setTextColor (limegreen);
      SPH_text (textorigin, "Lime = Front Clipping Plane");
      CarriageReturn;
      
      SPH_setTextColor (forestgreen);
      SPH_text (textorigin, "Forest = Rear Clipping Plane");
      CarriageReturn;
   SPH_closeStructure ();

   SPH_postRoot (TEXT_STRUCT, TEXT_VIEW);
}
/*----------------------------------------------------------------------*/
void CreateNormalizedHouseStructure (void)
{
   int i;
   point tempv[8];
   
   int faketoparray[] =  {1,2,7,6,5,0,1,6};
   int fakesidearray[] = {2,3,8,7,6,1,2,7};
	
   point house_poly_vertices[] = {
      {  0.,  0., -40. },
      {  0., 10., -40. },
      {  8., 16., -40. },
      { 16., 10., -40. },
      { 16.,  0., -40. },
      {  0.,  0., -40. },
      {  0.,  0.,   0. },
      {  0., 10.,   0. },
      {  8., 16.,   0. },
      { 16., 10.,   0. },
      { 16.,  0.,   0. },
      {  0.,  0.,   0. },
   };

   point house_vertices[] = {
      {  0., 10.,   0. },
      {  8., 16.,   0. },
      { 16., 10.,   0. },
      { 16.,  0.,   0. },
      {  0.,  0.,   0. },
      {  0., 10., -40. },
      {  8., 16., -40. },
      { 16., 10., -40. },
      { 16.,  0., -40. },
      {  0.,  0., -40. }
   };
	
   /* FACETS ARE VERTEX LISTS counter-clockwise from point of view of
      someone OUTSIDE the house. */
   vertex_index house_facets[] = {
      4,3,2,1,0, -1,
      5,6,7,8,9, -1,
      2,7,6,1,   -1,
      2,3,8,7,   -1,
      3,4,9,8,   -1,
      0,5,9,4,   -1,
      0,1,6,5,   -1
   };


   SPH_openStructure (HOUSE_STRUCT);
      SPH_setEdgeStyle (DASHED);
      SPH_setEdgeColor (3);
      SPH_setEdgeWidthScaleFactor (1.7);
      SPH_polyhedron (10, 7, house_vertices, house_facets);
   SPH_closeStructure();
   

   /* Make fake houses (incomplete) for use in wire orthographic depictions */
   for (i=0; i<8; i++)
      COPY_BYTE (tempv[i], house_vertices[faketoparray[i]], point, 1);

   SPH_openStructure (FAKE_HOUSE_TOP_STRUCT);
     SPH_polyLine (8, tempv);
   SPH_closeStructure();
   

   for (i=0; i<8; i++)
      COPY_BYTE (tempv[i], house_vertices[fakesidearray[i]], point, 1);

   SPH_openStructure (FAKE_HOUSE_SIDE_STRUCT);
     SPH_polyLine (8, tempv);
   SPH_closeStructure();
}
/*----------------------------------------------------------------------*/
void CreateSphereStructure () 
{
   matrix mat;

   point sphere_verts[] = {
      {  0,      0,      1     },     
      { -0.894,  0,      0.447 }, 
      { -0.276,  0.851,  0.447 }, 
      {  0.724,  0.526,  0.447 }, 
      {  0.724, -0.526,  0.447 }, 
      { -0.276, -0.851,  0.447 }, 
      { -0.724,  0.526, -0.447 }, 
      {  0.276,  0.851, -0.447 }, 
      {  0.894,  0,     -0.447 }, 
      {  0.276, -0.851, -0.447 }, 
      { -0.724, -0.526, -0.447 }, 
      {  0,      0,     -1     }
   };
   vertex_index sphere_facets[] = {
      0,  2,  1,   -1,
      0,  3,  2,   -1, 
      0,  4,  3,   -1,
      0,  5,  4,   -1, 
      0,  1,  5,   -1,
      1,  2,  6,   -1, 
      2,  3,  7,   -1,
      3,  4,  8,   -1, 
      4,  5,  9,   -1, 
      5,  1, 10,   -1,
      1,  6, 10,   -1, 
      2,  7,  6,   -1, 
      3,  8,  7,   -1, 
      4,  9,  8,   -1, 
      5, 10,  9,   -1,
      6,  7, 11,   -1, 
      7,  8, 11,   -1,
      8,  9, 11,   -1,
      9, 10, 11,   -1,
      10, 6, 11,   -1
   };

   SPH_openStructure (SPHERE_STRUCT);
      SPH_scale (10.0, 10.0, 10.0, mat); 
      SPH_setModelingTransformation (mat, ASSIGN);
      SPH_polyhedron (12, 20, sphere_verts, sphere_facets);
   SPH_closeStructure();
}
/*----------------------------------------------------------------------*/
void CreateChimneyStructure ()
{
   matrix mat;
   
   point chim_verts[] = {
      {0.0, 0.0, 0.0},
      {4, 3, 0.0},
      {4, 7, 0.0},
      {0.0, 7, 0.0},
      {0.0, 0.0, -10},
      {4, 3, -10},
      {4, 7, -10},
      {0, 7, -10}
   };
   
   vertex_index chim_facets[] = 
      {0,1,2,3,  -1,
       4,7,6,5,  -1,
       7,4,0,3,  -1,
       1,5,6,2,  -1,
       7,3,2,6,  -1};
       
   SPH_openStructure (CHIMNEY_STRUCT);
      SPH_setInteriorColor (firebrick);
      SPH_translate (0.0,10.1, -30.0, mat); 
      SPH_setModelingTransformation (mat, ASSIGN);
      SPH_polyhedron (8, 5, chim_verts, chim_facets);
   SPH_closeStructure();
}
/*----------------------------------------------------------------------*/
void CreateAxesStructure ()
{
   point verts[2] = { {0.0,0.0,0.0}, {0.0,0.0,0.0} };
   
   SPH_openStructure (AXES_STRUCT);
      SPH_setLineColor (blue);
      SPH_setLineWidthScaleFactor (2.0);
      SPH_setTextColor (blue);
      
      verts[1][X] = 80.0; SPH_polyLine (2, verts); 
      SPH_text (verts[1], "X"); verts[1][X]=0.0;
      verts[1][Y] = 80.0; SPH_polyLine (2, verts); 
      SPH_text (verts[1], "Y"); verts[1][Y]=0.0;
      verts[1][Z] = 80.0; SPH_polyLine (2, verts); 
      SPH_text (verts[1], "Z"); verts[1][Z]=0.0;
   SPH_closeStructure ();
}
/*----------------------------------------------------------------------*/
void CreateCameraStructure ()
{
   vector zerovec={0.0,0.0,0.0};
   double dy, dx, edgex, edgey;
   point p, frontplane[5], backplane[5], edges[2];
   int i;
   extern matrix inverseVO;
   
   SPH_openStructure (CAMERA_STRUCT);
      if ((i=SPH_inquireElementPointer()) > 0)
         SPH_deleteElementsInRange (1, i); /*delete all extant*/

      /* PLACE AN IMAGE OF THE VRP LOCATION */
      SPH_setMarkerColor (red);
      MAT3mult_vec (p, zerovec, inverseVO);
      SPH_polyMarker (1, &p);
      
      /* PLACE AN IMAGE OF THE PRP LOCATION */
      SPH_setMarkerColor (orange);
      MAT3mult_vec (p, prp1, inverseVO);
      SPH_polyMarker (1, &p);
      
      /* CALCULATE SLOPES OF THE VIEW VOLUME EDGES */
      /* Here, we assume PRP is always (0,0,somethingPositive) */
      /* And, we assume vmin=vmax and umin=umax. */
      if (persptype1 == PERSPECTIVE) {
         dy = vmax1/prp1[Z];
         dx = umax1/prp1[Z];
      }
      else
         dy = dx = 0.0;
      
      /* PLACE LINES FOR THE FRONT AND BACK CLIP PLANES */
      edgex = umax1 + dx*(-fplane1); edgey = vmax1 + dy*(-fplane1);
      MAT3_SET_VEC (frontplane[0], -edgex, +edgey, fplane1);
      MAT3_SET_VEC (frontplane[1], +edgex, +edgey, fplane1);
      MAT3_SET_VEC (frontplane[2], +edgex, -edgey, fplane1);
      MAT3_SET_VEC (frontplane[3], -edgex, -edgey, fplane1);
      edgex = umax1 + dx*(-bplane1); edgey = vmax1 + dy*(-bplane1);
      MAT3_SET_VEC  (backplane[0], -edgex, +edgey, bplane1);
      MAT3_SET_VEC  (backplane[1], +edgex, +edgey, bplane1);
      MAT3_SET_VEC  (backplane[2], +edgex, -edgey, bplane1);
      MAT3_SET_VEC  (backplane[3], -edgex, -edgey, bplane1);
      for (i=0; i<4; i++) {
         MAT3mult_vec (frontplane[i],  frontplane[i], inverseVO );
         MAT3mult_vec (backplane[i],  backplane[i], inverseVO );
      }
      MAT3_COPY_VEC (frontplane[4], frontplane[0]);
      MAT3_COPY_VEC (backplane[4], backplane[0]);
      SPH_setLineColor (limegreen);
      SPH_setLineWidthScaleFactor (2.0);
      SPH_polyLine (5, frontplane);
      SPH_setLineColor (forestgreen);
      SPH_polyLine (5, backplane);
      for (i=0; i<4; i++) {
         MAT3_COPY_VEC (edges[0], frontplane[i]);
         MAT3_COPY_VEC (edges[1], backplane[i]);
         SPH_polyLine (2, edges);
      }
   SPH_closeStructure ();
}
/*----------------------------------------------------------------------*/
void BuildEverything() 
{
   register i;
   matrix temp_matrix;
   vector zerovec={0.0,0.0,0.0};
   point fillareaPts[] = { { 50.0,   0.0,   0.0 },
			   { 50.0, 100.0,   0.0 },
			   { 50.0, 100.0, 100.0 },
			   { 50.0,   0.0, 100.0 } };
   point friendshipStarPts[] = { { -10.0, 5.0,  -50.0  },
				 {  30.0, 5.0,  -10.0  },
				 {  70.0, 5.0, -130.0  },
				 { 110.0, 5.0,  -90.0  },
				 { -10.0, 5.0,  -50.0  },
				 { -10.0, 5.0,  -90.0  },
				 {  30.0, 5.0, -130.0  },
				 {  70.0, 5.0,  -10.0  },
				 { 110.0, 5.0,  -50.0  },
				 { -10.0, 5.0,  -90.0  } };
   
   CreateNormalizedHouseStructure();
   CreateAxesStructure();
   CreateTextStructure();
   CreateSphereStructure();
   CreateChimneyStructure();
   
   SPH_openStructure (STREET_STRUCT);
   {
      SPH_translate (0.0, 0.0, 0.0, temp_matrix);
      SPH_setModelingTransformation (temp_matrix, PRECONCATENATE);
      for (i=0; i<6; i++) {
         SPH_setInteriorColor (2+i);
	 SPH_setEdgeFlag (EDGE_INVISIBLE);
	 SPH_executeStructure (HOUSE_STRUCT);
         SPH_translate (20.0,0.0,0.0, temp_matrix);
         SPH_setModelingTransformation (temp_matrix, PRECONCATENATE);
      }
   }
   SPH_closeStructure();


   SPH_openStructure (NEIGHBORHOOD_STRUCT);
   { 
      SPH_executeStructure (STREET_STRUCT);
      for (i=0; i<2; i++) {
         SPH_translate (0.0, 0.0,-70.0, temp_matrix);
         SPH_setModelingTransformation (temp_matrix, PRECONCATENATE);
         SPH_executeStructure (STREET_STRUCT);
      }
   }
   SPH_closeStructure();


   SPH_openStructure (FAKE_NEIGHBORHOOD_SIDE_STRUCT);
   {
      SPH_setLineColor (7);
      SPH_executeStructure (FAKE_HOUSE_SIDE_STRUCT);
      for (i=0; i<2; i++) {
         SPH_translate (0.0, 0.0, -70.0, temp_matrix);
         SPH_setModelingTransformation (temp_matrix, PRECONCATENATE);
         SPH_executeStructure (FAKE_HOUSE_SIDE_STRUCT);
      }
   }
   SPH_closeStructure();
   SPH_postRoot (FAKE_NEIGHBORHOOD_SIDE_STRUCT, SIDE_ORTHO_VIEW);
   
   
   SPH_openStructure (FAKE_NEIGHBORHOOD_TOP_STRUCT);
   {
      for (i=0; i<6; i++) {
         int j;
         SPH_setLineColor (i+2);
         for (j=0; j<3; j++) {
            SPH_translate (i*20.0, 0.0,-70.0*j, temp_matrix);
            SPH_setModelingTransformation (temp_matrix, ASSIGN);
            SPH_executeStructure (FAKE_HOUSE_TOP_STRUCT);
         }
      }
   }
   SPH_closeStructure();
   SPH_postRoot (FAKE_NEIGHBORHOOD_TOP_STRUCT, TOP_ORTHO_VIEW);


   SPH_openStructure (GRID_STRUCT);
   {
      register i;
      point p[2];
      
      SPH_setModXform (SPH_translate (5, 5, 5, temp_matrix), ASSIGN);

      SPH_setLineColor (yellow);
      
      /* DRAW THE LINES GOING FROM z=10 TO z=(-infinity) */
      MAT3_COPY_VEC (p[0], zerovec);
      MAT3_COPY_VEC (p[1], zerovec);
      p[0][Z] = (double)10;
      p[1][Z] = (double)-200;
      for (i=0; i<=200; i+=20) {
         p[0][X] = p[1][X] = (double)i;
         SPH_polyLine (2, p);
      }
      
      /* DRAW THE LINES GOING FROM x=-10 TO x=infinity */
      MAT3_COPY_VEC (p[0], zerovec);
      MAT3_COPY_VEC (p[1], zerovec);
      p[0][X] = (double)-10;
      p[1][X] = (double)200;
      for (i=0; i<=200; i+=20) {
         p[0][Z] = p[1][Z] = (double)(-i);
         SPH_polyLine (2, p);
      }
      SPH_executeStructure (AXES_STRUCT);
   }
   SPH_closeStructure ();

   SPH_postRoot (CAMERA_STRUCT, TOP_ORTHO_VIEW);
   SPH_postRoot (CAMERA_STRUCT, SIDE_ORTHO_VIEW);
}
