#ifndef lint
static char Version[]=
   "$Id: sph_draw.c,v 1.4 1993/03/09 02:00:54 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 * sph_draw.c
 *
 * 	3-D graphics primitives (mostly a wrapper around SRGP).
 *
 * DETAILS:	These draw routines assume that the display-traverser has set
 * 		the two variables "currentViewIndex" and
 * 		"currentCompositeModxform".  
 * ------------------------------------------------------------------------- */

#include "HEADERS.h"
#include "sphigslocal.h"

/* this oughta be enough */
srgp__point srgp_pdc_points[100], srgp_polygon_points[100];


/* ---------------------------- Static Routines ---------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Coordinate conversions for vertex lists.
 * ------------------------------------------------------------------------- */
static void 
ChangeMCpointsToPDC3d_points (
   MAT3hvec  *MCvlist, 
   pdc_point *PDCvlist, 
   int 	      vertex_count)
{
   register i;
   MAT3hvec temp_outpt;

   for (i=(vertex_count-1); i>=0; i--) {
      MAT3mult_hvec (temp_outpt, MCvlist[i], currentTOTALxform, 1);
      PDCvlist[i][0] = (int)temp_outpt[0];
      PDCvlist[i][1] = (int)temp_outpt[1];
      PDCvlist[i][2] = (int)temp_outpt[2];
   }
}

static void 
ChangeMCpointsToSRGP_points (
   MAT3hvec    *MCvlist, 
   srgp__point *SRGP_vlist, 
   int          vertex_count)
{
   register i;
   MAT3hvec temp_outpt;

   for (i=(vertex_count-1); i>=0; i--) {
      MAT3mult_hvec (temp_outpt, MCvlist[i], currentTOTALxform, 1);
      SRGP_vlist[i].x = (int)temp_outpt[0];
      SRGP_vlist[i].y = (int)temp_outpt[1];
   }
}

/* --------------------------- Internal Routines --------------------------- */

/* -------------------------------------------------------------------------
 * DESCR   :	Draws a Polyhedron element
 * ------------------------------------------------------------------------- */
void
SPH__draw_polyhedron (
   polyhedron 	   *poly,
   attribute_group *attrs)
{
   register int v, f, viindex;
   pdc_point *vertptr;
   vertex_index *viptr;
   facet *facetptr;
   obj *newobj;

   switch (currentRendermode) {

    case WIREFRAME_RAW:

      ChangeMCpointsToSRGP_points (poly->vertex_list, srgp_pdc_points,
				   poly->vertex_count);

      SPH__set_attributes (attrs, ATTRIB_EDGE);

      for (f = 0, facetptr = poly->facet_list;
	   f < poly->facet_count;      
	   f++, facetptr++) {
	 viindex = facetptr->vertex_count;
	 viptr = facetptr->vertex_indices + viindex;
	 while (viindex--)
	    srgp_polygon_points[viindex] = srgp_pdc_points[*(--viptr)];
	 SRGP_polygon (facetptr->vertex_count, srgp_polygon_points);
      }
      break;

    default:
      OBJECT__addPoly (currentViewSpec, poly, FALSE, 
		       currentCompositeModxform, attrs);
      break;
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Draws a Polyhedron element
 * ------------------------------------------------------------------------- */
void
SPH__draw_fill_area (
   polyhedron 	   *poly,
   attribute_group *attrs)
{
   register int v, f, viindex;
   pdc_point *vertptr;
   vertex_index *viptr;
   facet *facetptr;
   obj *newobj;

   switch (currentRendermode) {

    case WIREFRAME_RAW:

      ChangeMCpointsToSRGP_points (poly->vertex_list, srgp_pdc_points,
				   poly->vertex_count);

      SPH__set_attributes (attrs, ATTRIB_EDGE);

      facetptr = poly->facet_list;
      viindex = facetptr->vertex_count;
      viptr = facetptr->vertex_indices + viindex;
      while (viindex--)
	 srgp_polygon_points[viindex] = srgp_pdc_points[*(--viptr)];
      SRGP_polygon (facetptr->vertex_count, srgp_polygon_points);
      break;

    default:
      OBJECT__addPoly (currentViewSpec, poly, TRUE,
		       currentCompositeModxform, attrs);
      break;
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Draws a Polyline element
 * ------------------------------------------------------------------------- */
void
SPH__draw_lines (
   element         *elptr, 
   attribute_group *attrs)
{
   switch (currentRendermode) {
    case WIREFRAME_RAW:
      
      ChangeMCpointsToSRGP_points 
	 (elptr->data.points, srgp_pdc_points, elptr->info.count);

      SPH__set_attributes (attrs, ATTRIB_LINE);
      SRGP_polyLine (elptr->info.count, srgp_pdc_points);
      break;

    default:
      OBJECT__addLines (currentViewSpec, 
			elptr->info.count, elptr->data.points,
			currentCompositeModxform, attrs);
      break;
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Draws a Polymarker element
 * ------------------------------------------------------------------------- */
void
SPH__draw_markers (
   element	   *elptr, 
   attribute_group *attrs)
{
   switch (currentRendermode) {
    case WIREFRAME_RAW:
      
      ChangeMCpointsToSRGP_points 
	 (elptr->data.points, srgp_pdc_points, elptr->info.count);

      SPH__set_attributes (attrs, ATTRIB_MARKER);
      SRGP_polyMarker (elptr->info.count, srgp_pdc_points);
      break;

    default:
      OBJECT__addPoints (currentViewSpec, 
			elptr->info.count, elptr->data.points,
			currentCompositeModxform, attrs);
      break;
   }
}
		 
/* -------------------------------------------------------------------------
 * DESCR   :	Draws a Text element
 * ------------------------------------------------------------------------- */
void
SPH__draw_text (
   MAT3hvec 	    mc_origin,
   char            *text,
   attribute_group *attrs)
{
   switch (currentRendermode) {
    case WIREFRAME_RAW:
      
      ChangeMCpointsToSRGP_points ((MAT3hvec*)mc_origin, srgp_pdc_points, 1);
      SPH__set_attributes (attrs, ATTRIB_TEXT);
      SRGP_text (srgp_pdc_points[0], text);
      break;

    default:
      OBJECT__addText (currentViewSpec, 
		       mc_origin, text,
		       currentCompositeModxform, attrs);
      break;
   }
}


/* -------------------------------------------------------------------------
 * DESCR   :	Sets SRGP attributes from an attribute set.
 * ------------------------------------------------------------------------- */
void
SPH__set_attributes (
   attribute_group *attrs,
   int 		    flags )
{
   if (flags & ATTRIB_LINE) {
      SRGP_setColor 	(attrs->line_color);
      SRGP_setLineStyle (attrs->line_style);
      SRGP_setLineWidth (attrs->line_width);      
   }
   if (flags & ATTRIB_MARKER) {
      SRGP_setColor 	  (attrs->marker_color);
      SRGP_setMarkerStyle (attrs->marker_style);
      SRGP_setMarkerSize  (attrs->marker_size);
   }
   if (flags & ATTRIB_EDGE) {
      SRGP_setColor	(attrs->edge_color);
      SRGP_setLineWidth (attrs->edge_width);
      SRGP_setLineStyle (attrs->edge_style);
   }
   if (flags & ATTRIB_FACE) {
      /* see also ChooseColor() in sph_object.c */
      SRGP_setFillStyle (SOLID);
      SRGP_setColor	(attrs->interior_color);
   }
   if (flags & ATTRIB_TEXT) {
      SRGP_setColor (attrs->text_color);
      SRGP_setFont  (attrs->font);
   }
}
