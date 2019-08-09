/**************\
              * 
 Program      * Robot
              * Atul Butte
              *
              * This program animates a little robot on the screen.
              *
             /*******************************************************************/

/**************\
              * 
 Includes     *
              *
             /*******************************************************************/

#include "sphigslocal.h"

/* robot.h   Atul Butte cs123044 */

#define MOVE_STEPS		4

#define NULL_OBJECT		 0
#define STANDARD_BOX		 1
#define STANDARD_CUBE		 2
#define OUR_ARM			 3
#define OUR_ARM_DRAW_ROD	 4
#define OUR_ARM_ROTATE_L_THUMB	 5
#define OUR_ARM_ROTATE_R_THUMB	 6
#define OUR_BODY		 7
#define OUR_BODY_ROTATE_ARM	 8
#define OUR_ROD			 9
#define OUR_ROOM		10
#define OUR_ROOM_ROTATE_ROBOT	11
#define OUR_ROOM_MOVE_ROBOT	12
#define OUR_ROOM_DRAW_ROD	13

enum direction { NORTH, SOUTH, EAST, WEST };



#include <stdio.h>
#include <assert.h>
#include <math.h>

/**************\
              * 
 Defines      *
              *
             /*******************************************************************/

typedef enum { false, true } Boolean;
#define WaitForKeystroke        SRGP_waitEvent(-1)


/**************\
              * 
 Data         *
              *
             /*******************************************************************/

/* verticies for room */

static point room_vertices[] = {
    { -50., -50., -50. },
    { 50., -50., -50. },
    { -50., 50., -50. },
    { -50., -50., 50. },
    { 50., 50., -50. },
    { 50., -50., 50. },
    { -50., 50., 50. },
    { 50., 50., 50. }
};

/* facets for room */

static vertex_index room_facets[] = {
     0,1,4,2, -1,
     2,4,7,6,-1,
     6,7,5,3,-1,
     3,5,1,0, -1,
     3,0,2,6,-1,
    1,5,7,4, -1
};

/* verticies for arm */

static point arm_vertices[] = {
    { -2.5, 0., -2.5 },
    { 2.5, 0., -2.5 },
    { -2.5, 20., -2.5 },
    { -2.5, 0, 2.5 },
    { 2.5, 20., -2.5 },
    { 2.5, 0., 2.5 },
    { -2.5, 20., 2.5 },
    { 2.5, 20., 2.5 }
};

/* facets for arm */

static vertex_index arm_facets[] = {
  0,1,4,2, -1,
  2,4,7,6, -1,
  6,7,5,3, -1,
  3,5,1,0, -1,
  3,0,2,6, -1,
  1,5,7,4, -1
};



/* READS three doubles from stdin */
ReadVector (vec)
double *vec;
{
   scanf("%lf %lf %lf\n", &vec[0], &vec[1], &vec[2]);
}

CreateNormalizedRoomStructure()
{
   int i;

   SPH_openStructure (STANDARD_BOX);
   SPH_polyhedron (8,6, room_vertices, room_facets);
   SPH_closeStructure();
}

CreateNormalizedArmStructure()
{
   int i;

   SPH_openStructure (STANDARD_CUBE);
   SPH_polyhedron (8,6, arm_vertices, arm_facets);
   SPH_closeStructure();
}

#define WaitForKeystroke	SRGP_waitEvent(-1)
#define WriteMessage(s)  	SRGP_text (SRGP_defPoint(15,4), s)
#define WriteUpperMessage(s)  	SRGP_text (SRGP_defPoint(15,20), s)

/**************\
              * 
 Procedure    * main( )
              *
              * Mainline. Initializes SPHIGS, sets up the camera angle,
              * defines the animated objects, and animates them.
              *
             /*******************************************************************/

main(argc, argv )
int argc;
char **argv;
{
   matrix vo_matrix, vm_matrix;
   matrix temp_matrix;

   /* VIEW 0 INITIALLY: */
   point vrp, prp;
   double umin, umax, vmin, vmax;
   double fplane, bplane;
   vector vpn, vupv;

   int i;

   fprintf (stderr, "I hope you redirected stdin...\n");

   ReadVector (vrp);
   ReadVector (vpn);
   ReadVector (vupv);
   ReadVector (prp);
   scanf ("%lf %lf\n", &fplane, &bplane);
   scanf ("%lf %lf   %lf %lf\n", &umin, &umax, &vmin, &vmax);

   SPH_begin (500, 500, 1, 0);

   SPH_setDoubleBufferingFlag (TRUE);


   SPH_evaluateViewOrientationMatrix 
      (vrp,
       vpn,
       vupv,
       vo_matrix);

   SPH_evaluateViewMappingMatrix
      (umin, umax,   vmin, vmax, PERSPECTIVE,
       prp,
       fplane, bplane,
       0.0,1.0, 0.0,1.0, 0.0,1.0,
       vm_matrix);

   SPH_setViewRepresentation
      (0,
       vo_matrix, vm_matrix,
       0.0, 1.0,  0.0,1.0,  0.0,1.0);


   SPH_setViewPointLightSource (0,   0.0, 100.0, 150.0);

   SPH_loadCommonColor (2, "yellow");
   SPH_loadCommonColor (3, "green");
   SPH_loadCommonColor (4, "turquoise");
   SPH_loadCommonColor (5, "pink");
   SPH_loadCommonColor (6, "goldenrod");
   SPH_loadCommonColor (7, "beige");

   SPH_setRenderingMode (0, WIREFRAME_RAW); 

/*    set_up_camera( ); */
    define_structures( );
    animate( );
    SPH_end( );
}

/**************\
              * 
 Procedure    * set_up_camera( )
              *
              * Initializes the SPHIGS camera angle and position.
              *
             /*******************************************************************/

set_up_camera( )
{
    point	view_ref_point;				/* view reference point */
    vector	view_plane_normal;			/* view plane normal */
    vector	view_up_vector;				/* view up vector */
    matrix	vo_matrix;				/* view orientation matrix */
    
    double	umin, umax, vmin, vmax;			/* max/min VRC size */
    point	proj_ref_point;				/* projection reference point */
    double	proj_viewport_minx, proj_viewport_maxx;	/* max/min viewport size */
    double	proj_viewport_miny, proj_viewport_maxy;	/* max/min viewport size */
    matrix	vm_matrix;				/* view mapping matrix */
    
    view_ref_point[0] = 0.0;
    view_ref_point[1] = 0.0;
    view_ref_point[2] = 0.0;
    
    view_plane_normal[0] = 0.0;
    view_plane_normal[1] = 0.0;
    view_plane_normal[2] = 1.0;
    
    view_up_vector[0] = 0.0;
    view_up_vector[1] = 1.0;
    view_up_vector[2] = 0.0;
    
    SPH_evaluateViewOrientationMatrix( view_ref_point, view_plane_normal, view_up_vector, vo_matrix );


    proj_ref_point[0] = 20.0;
    proj_ref_point[1] = 30.0;
    proj_ref_point[2] = -120.0;
    
    umin = -120.0;
    umax = 120.0;
    vmin = -120.0;
    vmax = 120.0;
    
    proj_viewport_minx = 0.0;
    proj_viewport_maxx = 1.0;
    proj_viewport_miny = 0.0;
    proj_viewport_maxy = 1.0;
    
    SPH_evaluateViewMappingMatrix 
       (umin, umax, vmin, vmax, PERSPECTIVE, 
	proj_ref_point, 
	-0.001, -99999999.0,
	proj_viewport_minx, proj_viewport_maxx, 
	proj_viewport_miny, proj_viewport_maxy, 
	0.0, 1.0,
	vm_matrix);
    
    SPH_setViewRepresentation
       (0, vo_matrix, vm_matrix, 0.0,1.0, 0.0,1.0, 0.0,1.0);
}

/**************\
              * 
 Procedure    * define_structures( )
              *
              * This routine constructs all the objects. These objects include
              * the standard box (center in middle), the standard cube (center
              * at middle of bottom facet), the rod, the robot arm, the robot
              * body, and the entire room.
              * 
              * These objects are obvious constructed with knowledge on how
              * they are going to be manipulated in the animation (e.g. space
              * is made for the rod in the hand, etc).
              *
             /*******************************************************************/

define_structures( )
{
    int			i;			/* index through facets */
    matrix		temp_matrix;			/* transformation matrix */
    
/* Create NULL_OBJECT */
    
    SPH_openStructure( NULL_OBJECT );
    SPH_closeStructure( );
    
/* Create STANDARD_BOX */
    
    CreateNormalizedRoomStructure();

/* Create STANDARD_CUBE */

    CreateNormalizedArmStructure();
   
/* Create OUR_ROD */
    
    SPH_openStructure( OUR_ROD );
    SPH_setModXform (SPH_scale(1./50., 4./10., 1./50., temp_matrix), ASSIGN);
    SPH_executeStructure( STANDARD_BOX );
    SPH_closeStructure( );
    
/* Create OUR_ARM */
    
    SPH_openStructure( OUR_ARM );
    SPH_executeStructure( STANDARD_CUBE );
    
    /* left thumb */
    SPH_label( OUR_ARM_ROTATE_L_THUMB );
    SPH_setModXform( SPH_rotateZ( 90.0, temp_matrix ), ASSIGN );
    SPH_setModXform( SPH_scale( 1./5., 1./4., 1./5., temp_matrix), PRECONCATENATE );
    SPH_setModXform( SPH_translate( -1., 20., 0., temp_matrix), PRECONCATENATE );
    SPH_executeStructure( STANDARD_CUBE );
    
    /* right thumb */
    SPH_label( OUR_ARM_ROTATE_R_THUMB );
    SPH_setModXform( SPH_rotateZ( -90.0, temp_matrix ), ASSIGN );
    SPH_setModXform( SPH_scale( 1./5., 1./4., 1./5., temp_matrix ), PRECONCATENATE );
    SPH_setModXform( SPH_translate( 1., 20., 0., temp_matrix ), PRECONCATENATE );
    SPH_executeStructure( STANDARD_CUBE );
    
    /* space for rod */
    SPH_setModXform( SPH_rotateX( -90., temp_matrix ), ASSIGN );
    SPH_setModXform( SPH_translate( 0., 23., 0., temp_matrix ), PRECONCATENATE );
    SPH_setModXform( SPH_translate( 0., 0., -10., temp_matrix ), PRECONCATENATE );
    SPH_label( OUR_ARM_DRAW_ROD );
    SPH_executeStructure( NULL_OBJECT );

    SPH_closeStructure( );
    
/* Create OUR_BODY */
    
    SPH_openStructure( OUR_BODY );
    
    /* arm */
    SPH_label( OUR_BODY_ROTATE_ARM );
    SPH_setModXform( SPH_rotateX( 0.0, temp_matrix ), ASSIGN );
    SPH_setModXform( SPH_translate( 0., -30., 0., temp_matrix ), PRECONCATENATE );
    SPH_executeStructure( OUR_ARM );
    
    /* torso */
    SPH_setModXform( SPH_scale( 1.0/10.0, 1.0/5.0, 1.0/10.0, temp_matrix ), ASSIGN );
    SPH_setModXform( SPH_translate( 0., -40., 0., temp_matrix ), PRECONCATENATE );
    SPH_executeStructure( STANDARD_BOX );
    
    SPH_closeStructure( );

/* Create OUR_ROOM */
    
    SPH_openStructure( OUR_ROOM );
    
    /* draw boundaries of room */
    SPH_executeStructure( STANDARD_BOX );
    
    /* draw body */
    SPH_label( OUR_ROOM_ROTATE_ROBOT );
    SPH_setModXform( SPH_rotateY( 0.0, temp_matrix ), ASSIGN );
    SPH_label( OUR_ROOM_MOVE_ROBOT );
    SPH_setModXform( SPH_translate( 0., 0., 0., temp_matrix ), PRECONCATENATE );
    SPH_executeStructure( OUR_BODY );
    
    /* draw rod */
    SPH_setModXform( SPH_rotateY( 0.0, temp_matrix ), ASSIGN );
    SPH_setModXform( SPH_translate( 0., -20., -40., temp_matrix ), PRECONCATENATE );
    SPH_label( OUR_ROOM_DRAW_ROD );
    SPH_executeStructure( OUR_ROD );
    
    SPH_closeStructure( );
}

/**************\
              * 
 Procedure    * spinrobot( direction dir, Boolean ccw )
              *
              * Rotates the robot to the given direction with the given path
              * (i.e. clockwise or counterclockwise).
              *
             /*******************************************************************/

spinrobot( dir, ccw )
enum direction			dir;			/* final direction */
Boolean				ccw;			/* path */
{
    matrix			temp_matrix;		/* transformation matrix */
    static double		prev_dir = 0.0;		/* previous direction */
    double			rot_angle;		/* intermediate rotation angle */
    double			end_angle;		/* final angle */
    
    switch( dir ) {
      case NORTH:
	if( ccw )
	    end_angle = 0.0;
	else
	    end_angle = 360.0;
	break;
      case WEST:
	end_angle = 270.0;
	break;
      case SOUTH:
	end_angle = 180.0;
	break;
      case EAST:
	end_angle = 90.0;
	break;
    }
    
    if( prev_dir == 0.0 || prev_dir == 360.0 )
	prev_dir = ( ccw ) ? 360.0 : 0.0;
    
    rot_angle = ( end_angle - prev_dir ) / 2.0  + prev_dir;
    
    /* Draw intermediate angle */
    
    SPH_openStructure( OUR_ROOM );
    SPH_setElementPointer( 0 );
    SPH_moveElementPointerToLabel( OUR_ROOM_ROTATE_ROBOT );
    SPH_offsetElementPointer( 1 );
    SPH_deleteElement( );
    SPH_setModXform( SPH_rotateY( rot_angle, temp_matrix ), ASSIGN );
    SPH_closeStructure( );
    
    /* Draw final angle */
    
    SPH_openStructure( OUR_ROOM );
    SPH_setElementPointer( 0 );
    SPH_moveElementPointerToLabel( OUR_ROOM_ROTATE_ROBOT );
    SPH_offsetElementPointer( 1 );
    SPH_deleteElement( );
    SPH_setModXform( SPH_rotateY( end_angle, temp_matrix ), ASSIGN );
    SPH_closeStructure( );
    
    prev_dir = end_angle;
}

/**************\
              * 
 Procedure    * lowerarm( )
              *
              * Gradually lowers the arm of the robot.
              *
             /*******************************************************************/

lowerarm( )
{
    matrix	temp_matrix;		/* transformation matrix */
    double	i;			/* index through angles */
    
    for( i = 45.0; i <= 90.0; i += 45.0 ) {
	SPH_openStructure( OUR_BODY );
	SPH_setElementPointer( 0 );
	SPH_moveElementPointerToLabel( OUR_BODY_ROTATE_ARM );
	SPH_offsetElementPointer( 1 );
	SPH_deleteElement( );
	SPH_setModXform( SPH_rotateX( i, temp_matrix ), ASSIGN );
	SPH_closeStructure( );
    }
}

/**************\
              * 
 Procedure    * raisearm( )
              *
              * Gradually raises the arm of the robot.
              *
             /*******************************************************************/

raisearm( )
{
    matrix	temp_matrix;		/* transformation matrix */
    double	i;			/* index through angles */
    
    for( i = 45.0; i >= 0.0; i -= 45.0 ) {
	SPH_openStructure( OUR_BODY );
	SPH_setElementPointer( 0 );
	SPH_moveElementPointerToLabel( OUR_BODY_ROTATE_ARM );
	SPH_offsetElementPointer( 1 );
	SPH_deleteElement( );
	SPH_setModXform( SPH_rotateX( i, temp_matrix ), ASSIGN );
	SPH_closeStructure( );
    }
}

/**************\
              * 
 Procedure    * grab( )
              *
              * Grips the fingers together.
              *
             /*******************************************************************/

grab( )
{
    matrix	temp_matrix;		/* transformation matrix */
    double	i, j;			/* index through angles */

    for( i = 45.0, j = -45.0; i >= 0.0; i -= 45.0, j += 45.0 ) {
	SPH_openStructure( OUR_ARM );
	SPH_setElementPointer( 0 );
	SPH_moveElementPointerToLabel( OUR_ARM_ROTATE_L_THUMB );
	SPH_offsetElementPointer( 1 );
	SPH_deleteElement( );
	SPH_setModXform( SPH_rotateZ( i, temp_matrix ), ASSIGN );
	
	SPH_moveElementPointerToLabel( OUR_ARM_ROTATE_R_THUMB );
	SPH_offsetElementPointer( 1 );
	SPH_deleteElement( );
	SPH_setModXform( SPH_rotateZ( j, temp_matrix ), ASSIGN );
	SPH_closeStructure( );
    }
}

/**************\
              * 
 Procedure    * pickup( )
              *
              * Erases the rod from the room and adds it to the hand (arm) of
              * the robot.
              *
             /*******************************************************************/

pickup( )
{
    matrix	temp_matrix;			/* transformation matrix */

    SPH_openStructure( OUR_ROOM );
    SPH_setElementPointer( 0 );
    SPH_moveElementPointerToLabel( OUR_ROOM_DRAW_ROD );
    SPH_offsetElementPointer( 1 );
    SPH_deleteElement( );
    SPH_executeStructure( NULL_OBJECT );
    SPH_closeStructure( );

    SPH_openStructure( OUR_ARM );
    SPH_setElementPointer( 0 );
    SPH_moveElementPointerToLabel( OUR_ARM_DRAW_ROD );
    SPH_offsetElementPointer( 1 );
    SPH_deleteElement( );
    SPH_executeStructure( OUR_ROD );
    SPH_closeStructure( );
}

/**************\
              * 
 Procedure    * moveto( double x, double z )
              *
              * Gradually moves the robot to the (x,z) coordinate in the room.
              *
             /*******************************************************************/

moveto( x, z )
double		x;			/* final x coordinate */
double		z;			/* final z coordinate */
{
    static double	prev_x = 0.0;				/* previous x coordinate */
    static double	prev_z = 0.0;				/* previous z coordinate */
    double		x_step = (x - prev_x) / MOVE_STEPS;	/* x step value */
    double		z_step = (z - prev_z) / MOVE_STEPS;	/* z step value */
    double		curr_x = prev_x + x_step;		/* current x coordinate in loop */
    double		curr_z = prev_z + z_step;		/* current z coordinate in loop */
    matrix		temp_matrix;				/* transformation matrix */
    int			i;					/* index through angles */
    
    for( i = 0; i < MOVE_STEPS; i++ ) {
	SPH_openStructure( OUR_ROOM );
	SPH_setElementPointer( 0 );
	SPH_moveElementPointerToLabel( OUR_ROOM_MOVE_ROBOT );
	SPH_offsetElementPointer( 1 );
	SPH_deleteElement( );
	SPH_setModXform( SPH_translate( curr_x, 0., curr_z, temp_matrix ), PRECONCATENATE );
	curr_x += x_step;
	curr_z += z_step;
	SPH_closeStructure( );
    }
    prev_x = x;
    prev_z = z;
}

/**************\
              * 
 Procedure    * animate( )
              *
              * Animates the robot with a script.
              *
             /*******************************************************************/

animate( )
{
    SPH_postRoot( OUR_ROOM, 0 );
    moveto( 0.0, 30.0 );
    spinrobot( EAST, false );
    moveto( 30.0, 30.0 );
    spinrobot( SOUTH, false );
    moveto( 30.0, -40.0 );
    spinrobot( WEST, false );
    moveto( -30.0, -40.0 );
    spinrobot( NORTH, false );
    moveto( -30.0, 30.0 );
    spinrobot( EAST, false );
    moveto( 0.0, 30.0 );
    spinrobot( SOUTH, false );
    moveto( 0.0, -13.0 );
    lowerarm( );
    grab( );
    pickup( );
    raisearm( );
    spinrobot( EAST, true );
    spinrobot( NORTH, true );
    moveto( 0.0, 30.0 );
    spinrobot( WEST, true );
    moveto( -30.0, 30.0 );
    spinrobot( SOUTH, true );
    moveto( -30.0, -30.0 );
    spinrobot( EAST, true );
    moveto( 30.0, -30.0 );
    spinrobot( NORTH, true );
    moveto( 30.0, 30.0 );
    spinrobot( WEST, true );
    moveto( 0.0, 30.0 );
    spinrobot( SOUTH, true );
    lowerarm( );
    raisearm( );
    WaitForKeystroke;
}

