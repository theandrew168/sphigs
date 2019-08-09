
/* -------------------------------------------------------------------------
 * SPHDEMO_view.h
 *
 *	View definitions for the SPHIGS demo.
 * ------------------------------------------------------------------------- */

typedef enum {
   SIDE_ORTHO_VIEW  = 1,
   TOP_ORTHO_VIEW,
   PERSPECTIVE_VIEW,
   TEXT_VIEW,

   NUMBER_OF_VIEWS
} viewID;

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

extern point   vrp1, prp1;
extern vector  vpn1, vupv1;
extern double  umin1, umax1, vmin1, vmax1;
extern double  fplane1, bplane1;
extern double  viewportxmin, viewportxmax, viewportymin, viewportymax;
extern short   persptype1;
