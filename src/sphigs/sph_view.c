#ifndef lint
static char version[]=
   "$Id: sph_view.c,v 1.5 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_view.c
 *
 *	Mathematics for view specification.
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"
#include <stdio.h>

/*
 * Math macros
 */

#define PI         	3.1415926535
#define FEPS         	(1e-12)
#define FZERO(N)      	((N) < FEPS && (N) > -FEPS)
#define FEQUAL(A,B)   	(((A) - (B)) < FEPS && ((A) - (B)) > -FEPS)

#define THE_VIEW    	SPH__viewTable[viewIndex]
/*#if defined(THINK_C) || defined(__MSDOS__)*/
#undef  HUGE
#define HUGE 		((double)9999999.0)
/*#endif*/

/* Have to save these between calls to viewMapping() and setViewRep()
 *   'cause in ORTHO mode we can't backcalc the front and back clipping planes
 *    from the vm matrix!
 */
static double most_recent_F, most_recent_B;

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Upon entry, the vo_matrix, vm_matrix, and viewport for the
 * 		given view can be assumed correct, but the pdc-viewport and
 * 		the final viewing matrix are obsolete and must be recomputed. 
 * ------------------------------------------------------------------------- */
void
SPH__updateViewInfo (
   int viewIndex)
{
   vector_3D scale_vec;
   matrix scale_mat;


   THE_VIEW.pdc_viewport.bottom_left.x =
       THE_VIEW.viewport.bottom_left.x * SPH__ndcSpaceSizeInPixels;
   THE_VIEW.pdc_viewport.bottom_left.y =
       THE_VIEW.viewport.bottom_left.y * SPH__ndcSpaceSizeInPixels;
   THE_VIEW.pdc_viewport.top_right.x =
      (THE_VIEW.viewport.top_right.x * SPH__ndcSpaceSizeInPixels) - 1;
   THE_VIEW.pdc_viewport.top_right.y =
      (THE_VIEW.viewport.top_right.y * SPH__ndcSpaceSizeInPixels) - 1;

   /* ONE FINAL STEP.  The transformation after the view-orientation
      and the view-mapping produces NDC coords (NPC, to PHIGS-gurus).
      We need to scale that by the pixel-size of our NDC space to
      get PDCs.
      The Z-coord of the PDC coord is being scaled quite substantially
         so when we turn it into an integer via truncation it doesn't
	 lose its meaning.  It will be used for HLHSR.
    */
   MAT3mult (THE_VIEW.cammat, THE_VIEW.vm_matrix, THE_VIEW.vo_matrix);
   MAT3_SET_VEC(scale_vec,  
		SPH__ndcSpaceSizeInPixels*1.0,
		SPH__ndcSpaceSizeInPixels*1.0,
		1000.0);
   MAT3scale (scale_mat, scale_vec);
   MAT3mult (THE_VIEW.cammat, scale_mat, THE_VIEW.cammat);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Initializes the package-wide global viewport table.
 * ------------------------------------------------------------------------- */
void
SPH__init_view_table (void)
{
   register viewIndex=0;
   register view_spec *v;
   point vrp, prp;
   vector_3D vpn, vupv;
   matrix vo_matrix, vm_matrix;

   SPH_defPoint(vrp,  0.0,0.0,0.0);
   SPH_defPoint(vpn,  0.0,0.0,1.0);
   SPH_defPoint(vupv, 0.0,1.0,0.0);
   SPH_evaluateViewOrientationMatrix(vrp, vpn, vupv, vo_matrix);

   SPH_defPoint(prp,  0.5,0.5,1.0);
   SPH_evaluateViewMappingMatrix
      (0.0,1.0,  0.0,1.0,
       PARALLEL,
       prp,
       0.99, -50000.0,
       0.0,1.0, 0.0,1.0, 0.0, 1.0,
       vm_matrix);


   bzero (SPH__viewTable, sizeof(view_spec)*(MAX_VIEW_INDEX+1));

   v = SPH__viewTable;
   for (viewIndex=0; viewIndex<=MAX_VIEW_INDEX; viewIndex++) 
   {
      SPH_setViewRepresentation
	 (viewIndex,
	  vo_matrix, vm_matrix,
	  0.0,1.0, 0.0,1.0, 0.0,1.0);

      /* name sets */
      SPH__allocNBitstring (v->invisFilter, MAX_NAME);
      SPH__allocNBitstring (v->hiliteFilter, MAX_NAME);
      SPH__clearNBitstring (v->invisFilter, MAX_NAME);
      SPH__clearNBitstring (v->hiliteFilter, MAX_NAME);

      /* light sources */
      ALLOC_RECORDS (v->lights, light_source, MAX_LIGHT_INDEX+1);
      bzero(v->lights, sizeof (light_source) * (MAX_LIGHT_INDEX+1));
      v->lights[0].active = TRUE;
      v->lights[0].cameraRelative = TRUE;
      v->lights[0].intensity = 1.0;
      MAT3_SET_HVEC (v->lights[0].position, 0.0, HUGE, HUGE, 1.0);

      /* misc */
      SPH__clearBitstring (&v->descendent_list);
      v->background_color = SRGP_WHITE;
      v->rendermode = WIREFRAME;
      v->algorithm = RENDER_BSP;

      v++;
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Converts all point light positions into the current NPC
 * 		coordinate system.
 * ------------------------------------------------------------------------- */
void
SPH__viewPrepareLights (
   view_spec *vs)
{
   register int 	  i;
   register light_source *light;

   for (i = 0, light = vs->lights;
	i <= MAX_LIGHT_INDEX;
	i++, light++)
   {
      if (!light->active)
	 continue;

      light->position [3] = 1.0;
      if (light->cameraRelative)
	 MAT3_COPY_HVEC (light->uvnPosition, light->position);
      else
	 MAT3mult_hvec (light->uvnPosition,
			light->position, vs->vo_matrix, TRUE);
   }  
}

/* ---------------------------- Public Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	First half of the view representation: a matrix representing
 * 		the camera's orientation in 3-D world coordinates.  This
 * 		defines the axes of VRC.
 * ------------------------------------------------------------------------- */
void 
SPH_evaluateViewOrientationMatrix (
   point  view_ref_point,
   vector_3D view_plane_normal,
   vector_3D view_up_vector,
   matrix vo_matrix /* RETURN */ )
{
   vector_3D tempvec, up_v, normal_v, proj_up_v, right_v;
   matrix trans, rot;
   double dot, tmp1;


   /* FIND THE UNIT VERSION OF THE up VECTOR */
   MAT3_COPY_VEC(up_v, view_up_vector);
   MAT3_NORMALIZE_VEC(up_v, tmp1);
   if (tmp1 == 0.0)
      SPH__error (ERR_MAT3_PACKAGE, "up vector has zero length");

   /* FIND THE UNIT VERSION OF THE view plane normal VECTOR */
   MAT3_COPY_VEC(normal_v, view_plane_normal);
   MAT3_NORMALIZE_VEC(normal_v, tmp1);
   if (tmp1 == 0.0)
      SPH__error (ERR_MAT3_PACKAGE, "View plane normal has zero length");

   /* Find the projection of the up vector on the film plane */
   dot = MAT3_DOT_PRODUCT(up_v, normal_v);
   if (dot == 1.0 || dot == -1.0)
      SPH__error (ERR_MAT3_PACKAGE, 
		          "Up vector and view plane normal are the same");

   MAT3_LINEAR_COMB(proj_up_v, 1.0, up_v, -dot, normal_v);
   MAT3_NORMALIZE_VEC(proj_up_v, tmp1);

   /* Find the vector that goes to the right on the film plane */
   MAT3cross_product(right_v,    normal_v, proj_up_v);


   /********* STEP 1 OF FOLEY CHAPTER VIEW.5.2 */
   /* Translate view reference point to origin */
   MAT3_SCALE_VEC(tempvec, view_ref_point, -1);
   MAT3translate(trans, tempvec);


   /********* STEP 2 OF FOLEY CHAPTER VIEW.5.2 */
   /* Rotate so normal lies on z-axis and proj_up is on y-axis */
   /* Remember: right-hand rule is in effect. */
   MAT3identity(rot);
   rot[2][0] = normal_v[X];
   rot[2][1] = normal_v[Y];
   rot[2][2] = normal_v[Z];
         
   rot[0][0] = -right_v[X];
   rot[0][1] = -right_v[Y];
   rot[0][2] = -right_v[Z];
         
   rot[1][0] = proj_up_v[X];
   rot[1][1] = proj_up_v[Y];
   rot[1][2] = proj_up_v[Z];


   /*************** PRODUCE THE FINAL PRODUCT: TRANS x ROT */
   MAT3mult(vo_matrix,  rot, trans);
}


/* -------------------------------------------------------------------------
 * DESCR   :	The second half of the view specification, in which the
 * 		canonical view volume is defined (NPC).
 * ------------------------------------------------------------------------- */
void
SPH_evaluateViewMappingMatrix (
   double umin,
   double umax,
   double vmin,
   double vmax,
   projectionType  proj_type, 	/* ORTHOGRAPHIC or PERSPECTIVE */
   point  proj_ref_point,
   double front_plane_dist,
   double back_plane_dist,
   double viewport_minx,
   double viewport_maxx, 
   double viewport_miny,
   double viewport_maxy,
   double viewport_minz,
   double viewport_maxz,
   matrix vm_matrix  	/* RETURN */ )
{
   matrix step3_matrix, step4_matrix, step3and4_matrix, step5_matrix,
          step6_matrix, sklar_matrix;
   MAT3hvec vrp;
   MAT3vec z_axis, shear;

   MAT3_SET_HVEC (vrp,    0.0, 0.0, 0.0, 1.0);


   /**** First, we test for "garbage in" ****/
   if (proj_ref_point[Z] <= 0.0)
      SPH__error (ERR_BAD_VIEWING_PARAMETER, 
		 "Z-coordinate of PRP is zero or negative\n");
   if (front_plane_dist >= proj_ref_point[Z])
      SPH__error (ERR_BAD_VIEWING_PARAMETER, 
		 "front clip plane lies behind or on the PRP\n");
   if (back_plane_dist >= front_plane_dist)
      SPH__error (ERR_BAD_VIEWING_PARAMETER, 
		 "back clip plane is closer to PRP than the front clip plane");


   /**** Save the F and B because currently there's a bug in ORTHO mode;
         we can't backcalc the F and B from the vm matrix in ORTH mode! ***/
   if (proj_type == PARALLEL) {
      most_recent_F = front_plane_dist;
      most_recent_B = back_plane_dist;
   }


   /********* STEP 3 OF FOLEY CHAPTER VIEW.5.2 */
   /* Only to be done for PERSPECTIVE. */
   /* Translate so the PRP (center of projection) is at the origin. */
   if (proj_type == PERSPECTIVE) {
      vector_3D tempvec;

      MAT3_SCALE_VEC(tempvec, proj_ref_point, -1);
      MAT3translate(step3_matrix, tempvec);
#ifdef DEBUG
      MAT3print_formatted
	 (step3_matrix, stdout, 0, "Foley step 3  ", 0, 0); 
#endif
      
      /* update vrp */
      MAT3_ADD_VEC(vrp,  vrp, tempvec);
   }
   else
      MAT3identity (step3_matrix);


   /********* STEP 4 OF FOLEY CHAPTER VIEW.5.2 */
   /* Shear so that the center line of the view volume lies on z-axis */
   {
      vector_3D dop_v;   /* direction of projection */

      if (proj_type == PERSPECTIVE) {
	 dop_v[0] = vrp[0] + (umax+umin)/2;
	 dop_v[1] = vrp[1] + (vmax+vmin)/2;
	 dop_v[2] = vrp[2];
      }
      else {
	 dop_v[0] = (umax+umin)/2 - proj_ref_point[0];
	 dop_v[1] = (vmax+vmin)/2 - proj_ref_point[1];
	 dop_v[2] = 0.0 - proj_ref_point[2];
      }

      MAT3_SET_VEC (z_axis, 0.0, 0.0, 1.0);
      MAT3_SET_VEC(shear, -dop_v[X] / dop_v[Z], -dop_v[Y] / dop_v[Z], 0.0);
      MAT3shear(step4_matrix, z_axis, shear);

#ifdef DEBUG
      MAT3print_formatted
	    (step4_matrix, stdout, 0, "Shear  ", 0, 0); 
#endif
   }


   /**** LET'S COMPUTE THE INTERMEDIATE RESULT: step3 times step4 */
   MAT3mult (step3and4_matrix, step4_matrix, step3_matrix);
#ifdef DEBUG
   MAT3print_formatted
      (step3and4_matrix, stdout, 0, "Foley step 3 x step 4  ", 0, 0); 
#endif


   /**** STEPS 5 and 6 DIFFER BETWEEN THE TWO TYPES OF PROJECTIONS. */

   if (proj_type == PARALLEL) {
      vector_3D tempvec;
      MAT3_SET_VEC (tempvec,  
		    (umax+umin)/(-2.0),
		    (vmax+vmin)/(-2.0),
		    0.0);			/******** F=0.0 */
      MAT3translate(step5_matrix, tempvec);
#ifdef DEBUG
      MAT3print_formatted
	    (step5_matrix, stdout, 0, "ORTHO step Tpar w/ F=0  ", 0, 0); 
#endif

      MAT3_SET_VEC (tempvec,  
		    2.0/(umax-umin),
		    2.0/(vmax-vmin),
		    1.0/2.0);			/******** B=2.0 */
      MAT3scale(step6_matrix, tempvec);
#ifdef DEBUG
      MAT3print_formatted
	    (step6_matrix, stdout, 0, "ORTHO step Spar w/ B=2  ", 0, 0); 
#endif
   }
   else {

      /********* PERSPECTIVE: STEP 5 OF FOLEY CHAPTER VIEW.5.2 */
      /* Get it to canonical perspective volume. */
      MAT3hvec vrp_prime;
      vector_3D scale_vec;
      double zmin;

      /* FIRST, find VRP' (VRP after steps 3 and 4) */
      MAT3mult_hvec (vrp_prime, vrp, step4_matrix, FALSE);
#ifdef DEBUG
      printf ("vrp_prime is found to be: (%f %f %f %f)\n",
	      vrp_prime[0], vrp_prime[1], vrp_prime[2], vrp_prime[3]);
#endif

      /* Compute a scale matrix ala Foley */
#define F front_plane_dist
#define B back_plane_dist
      MAT3_SET_VEC(scale_vec,
		   (2.0)*vrp_prime[Z]/((umax-umin)*(vrp_prime[Z]+B)),
		   (2.0)*vrp_prime[Z]/((vmax-vmin)*(vrp_prime[Z]+B)),
		   1.0/(vrp_prime[Z]+B));

      MAT3scale(step5_matrix, scale_vec);
#ifdef DEBUG
      MAT3print_formatted
	    (step5_matrix, stdout, 0, "Foley step 5 w/ B=2  ", 0, 0); 
#endif


      /************ PERSPECTIVE: STEP 6 OF FOLEY */
      /* Get it to canonical parallel volume. */
   
   
      zmin = (vrp_prime[Z]+F)/(vrp_prime[Z]+B);
#ifdef DEBUG
      fprintf (stderr, "VIEW MAPPING MATRIX CALC'D FOR    F=%f  B=%f\n", F,B);
      fprintf (stderr, "zmin is calculated as %f\n", zmin);
#endif
      MAT3zero (step6_matrix);
      step6_matrix[0][0] = 1.0;
      step6_matrix[1][1] = 1.0;
      step6_matrix[2][2] = 1.0 / (1.0 - zmin);
      step6_matrix[2][3] = (0.0 - zmin) / (1.0 - zmin);
      step6_matrix[3][2] = 1.0;
#ifdef DEBUG
      MAT3print_formatted
	    (step6_matrix, stdout, 0, "Foley step 6 w/ F=0  ", 0, 0); 
#endif
   }

  
   /******** DO SOME MERGING */
   MAT3mult(vm_matrix,   step5_matrix, step3and4_matrix);
   MAT3mult(vm_matrix,   step6_matrix, vm_matrix);
#ifdef DEBUG
   MAT3print_formatted
         (vm_matrix, stdout, 0, "All of Foley's steps  ", 0, 0); 
#endif


/*!*/
   /********* SKLAR'S STEP */
   /* To bring the canonical (-1 to +1) to the NPC viewport.
    * This transformation will produce NDC coordinates (if you ignore z).
    */
   {
      matrix scalemat, transmat;
      vector_3D scale_vec, trans_vec;

      MAT3_SET_VEC(scale_vec,
		   (viewport_maxx - viewport_minx)/2,
		   (viewport_maxy - viewport_miny)/2,
		   1.0);
      MAT3scale(scalemat,   scale_vec);
      MAT3_SET_VEC(trans_vec,
		   (viewport_minx+viewport_maxx)/2,
		   (viewport_miny+viewport_maxy)/2,
		   0);
      MAT3translate(transmat,   trans_vec);

      MAT3mult(sklar_matrix,   transmat, scalemat);
   }

   MAT3mult(vm_matrix,   sklar_matrix, vm_matrix);
   
#ifdef DEBUG
   MAT3print_formatted
         (vm_matrix, stdout, 0, "Final matrix  ", 0, 0); 
#endif
}


/* -------------------------------------------------------------------------
 * DESCR   :	Sets final view representation
 * ------------------------------------------------------------------------- */
void 
SPH_setViewRepresentation (
   int viewIndex,
   matrix vo_matrix, 
   matrix vm_matrix,
   double vp_minx, double vp_maxx, 
   double vp_miny, double vp_maxy,
   double vp_minz, double vp_maxz)
{
   rectangle 	oldpdcviewport;
   MAT3hvec 		tempvec, resultvec;
   matrix 		inverse_vm_matrix;
   matrix 		inverse_vo_matrix;


   SPH_check_system_state;
   SPH_check_no_open_structure;
   SPH_check_view_index;
   SPH_check_rectangle (vp_minx,vp_miny,vp_maxx,vp_maxy);


   oldpdcviewport = THE_VIEW.pdc_viewport;
   THE_VIEW.viewport = 
      SPH_defNDCrectangle (vp_minx, vp_miny, vp_maxx, vp_maxy);
       	   
   MAT3copy (THE_VIEW.vo_matrix,   vo_matrix);
   MAT3copy (THE_VIEW.vm_matrix,   vm_matrix);
   

   MAT3invert (inverse_vo_matrix,  vo_matrix);
   MAT3invert (inverse_vm_matrix,  vm_matrix);
   
   MAT3_SET_HVEC (tempvec, 0.0, 0.0, 0.0, 1.0);
   MAT3mult_hvec (resultvec, tempvec, inverse_vm_matrix, TRUE);
   THE_VIEW.frontPlaneDistance = resultvec[Z];
#ifdef DEBUG
   fprintf (stderr, "Backcalc the F from z=0 results in %f\n", resultvec[Z]);
#endif

   MAT3_SET_HVEC (tempvec, 0.0, 0.0, 1.0, 1.0);
   MAT3mult_hvec (resultvec, tempvec, inverse_vm_matrix, TRUE);
   THE_VIEW.backPlaneDistance = resultvec[Z];
#ifdef DEBUG
   fprintf (stderr, "Backcalc the B from z=1 results in %f\n", resultvec[Z]);
#endif

   if (THE_VIEW.frontPlaneDistance <= THE_VIEW.backPlaneDistance) {
      /**** WHOA!  Bad news!  Can't backcalc (happens in ORTHO matrices) ***/
      /* We'll have to take a not-so-wild guess. */
      THE_VIEW.frontPlaneDistance = most_recent_F;
      THE_VIEW.backPlaneDistance = most_recent_B;
   }

   MAT3_SET_HVEC (tempvec, 0.0, 0.0, -HUGE, 1.0);
   MAT3mult_hvec (tempvec, tempvec, inverse_vm_matrix, TRUE);
   MAT3mult_hvec (THE_VIEW.prp, tempvec, inverse_vo_matrix, TRUE);

   SPH__updateViewInfo (viewIndex);
   SPH__refresh_viewport_change (viewIndex, oldpdcviewport);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Sets the position of the viewport's default point light source
 * 		(has no effect in WIREFRAME modes)
 * ------------------------------------------------------------------------- */
void
SPH_setViewPointLightSource (
   int    viewIndex,
   double u,
   double v,
   double n)
{
   register light_source *light;

   SPH_check_system_state;
   SPH_check_view_index;

   light = & THE_VIEW.lights [0];

   light->active 	 = TRUE;
   light->cameraRelative = TRUE;
   light->attenuate	 = FALSE;
   light->intensity	 = 1.0;
   MAT3_SET_HVEC (light->position, u, v, n, 1.0);

   THE_VIEW.obsolete_render = TRUE;
   SPH__refresh_lights (viewIndex);
}


/* -------------------------------------------------------------------------
 * DESCR   :	Defines and activates a new point light source in the
 * 		specified view.  Note that any point light source previously
 * 		defined under the same index will be deleted.
 * ------------------------------------------------------------------------- */
void
SPH_addPointLightSource (
   int     viewIndex,
   int     lightIndex,
   point   position, 
   boolean cameraRelative,
   double  intensity,
   double  attenuation)
{
   register light_source *light;

   SPH_check_system_state;
   SPH_check_view_index;
   SPH_check_light_index;

   light = &(THE_VIEW.lights [lightIndex]);

   light->active 	 = TRUE;
   light->cameraRelative = cameraRelative;
   light->attenuate	 = attenuation > MAT3_EPSILON && attenuation < HUGE;

   light->attenFactor	 = attenuation;
   light->intensity	 = intensity;
   MAT3_COPY_VEC (light->position, position);
   light->position [3] = 1.0;

   THE_VIEW.obsolete_render = TRUE;
   SPH__refresh_lights (viewIndex);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Disables a view's point light source (has no effect in
 * 		WIREFRAME modes).
 * ------------------------------------------------------------------------- */
void
SPH_removePointLightSource (
   int     viewIndex,
   int     lightIndex)
{
   SPH_check_system_state;
   SPH_check_view_index;
   SPH_check_light_index;

   THE_VIEW.lights[lightIndex].active = FALSE;
   THE_VIEW.obsolete_render = TRUE;
   SPH__refresh_lights (viewIndex);
}

/* -------------------------------------------------------------------------
 * DESCR   :	On a per-view basis, the background color can be set.
 * 		Warning: this does NOT cause a refresh.  The new color will
 * 		only be seen upon the next operation causing a refresh.
 * ------------------------------------------------------------------------- */
void
SPH_setViewBackgroundColor (
   int viewIndex, 
   int color)
{
   SPH_check_system_state;
   SPH_check_view_index;

   if ( ! IS_LEGAL_COLOR_INDEX(color))
      color = 0;
   THE_VIEW.background_color = color;
}

/* -------------------------------------------------------------------------
 * DESCR   :	A (not really public) function to set the rendering algorithm
 * ------------------------------------------------------------------------- */
void
SPH_setViewRenderAlgorithm (
   int       viewIndex, 
   int	     algorithm)
{
   SPH_check_system_state;
   SPH_check_view_index;

   if (THE_VIEW.algorithm != algorithm) {
      THE_VIEW.algorithm = algorithm; 
      SPH__refresh_post (viewIndex);
   }
}
