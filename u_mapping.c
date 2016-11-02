/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985 by Supoj Sutanthavibul
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both the copyright
 * notice and this permission notice appear in supporting documentation. 
 * No representations are made about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or implied warranty."
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "fig.h"
#include "figx.h"
#include "mode.h"
#include "resources.h"
#include "u_coords.h"
#include "w_setup.h"

#define SIGN(a1,a2)	(a2 >= 0. ? fabs(a1) : -fabs(a1))


/* set up initialization parameters for map_translate */
static double		sino, coso, cosr, sinr;
extern double		screen_max_x, screen_max_y;
extern double		u_min, v_min;
extern double		u_max, v_max;
extern double		pix_per_unit, meters_per_unit;
extern void             map_translate(double ilat, double ilon, double *u, double *v);
double udeg;

/********VARIABLES**********************************************

  iprj === type of projection

  plat, plon are the projection latitude and longitude.

  rota === rotation factor (input)

*****************************************************************/


extern void perror (const char *);

void
map_init(void)
{
    struct ltln		llLowerLeft, llUpperRight;
    double		rad1, rad2;
    double		del_u, del_v, diff_uv;
    double		u1, u2, v1, v2;

    /* local initialization */
    double chi1, chi2, avg;
    double tmp1, tmp2;
    double sint, cost;

    plon_save = plon;
    switch (iprj) {
    case F_PROJ_MERC:
	if (fabs(rota) <= 0.0001 && fabs(plat) <= 0.0001) {
	    sino = 1.0;
	    coso = 0.0;
	    sinr = 0.0;
	    cosr = 1.0;
	    iprj = F_PROJ_FAST_MERC;
	} else if (fabs(rota) >= 179.9999 && fabs(plat) <= 0.0001) {
	    sino = -1.0;
	    plon = plon + 180.0;
	    coso = 0.0;
	    sinr = 0.0;
	    cosr = 1.0;
	    iprj = F_PROJ_FAST_MERC;
	} else {
            tmp1 = rota*DEG2RAD;
            tmp2 = plat*DEG2RAD;
            sinr = sin(tmp1);
            cosr = cos(tmp1);
            sino = sin(tmp2);
            coso = cos(tmp2);
            sint = coso*cosr;
            cost = sqrt(1. - sint*sint);
            tmp1 = sinr / cost;
            tmp2 = sino / cost;
            plon = plon - atan2(tmp1, -cosr*tmp2) * RAD2DEG;
            sinr = tmp1 * coso;
            cosr = - tmp2;
            sino = sint;
            coso = cost;
        }
	break;
    case F_PROJ_CONIC:
	avg = 0.5*(plat+rota);
	sino = SIGN(1.0, avg);
	chi1 = (90.0 - sino*plat)*DEG2RAD;
	if (plat==rota)
	    coso = cos(chi1);
	else{
	    double aa, bb, cc, dd;
	    chi2 = (90.0 - sino*rota)*DEG2RAD;
	    aa = sin(chi1);
	    bb = sin(chi2);
	    cc = tan(0.5*chi1);
	    dd = tan(0.5*chi2);
	    coso = log(aa/bb)/log(cc/dd);
	}
	break;
    case F_PROJ_STEREO:
	tmp1 = rota*DEG2RAD;
	tmp2 = plat*DEG2RAD;
	sinr = sin(tmp1);
	cosr = cos(tmp1);
	sino = sin(tmp2);
	coso = cos(tmp2);
	break;
    case F_PROJ_FAST_MERC:
	if (fabs(rota) <= 0.0001) {
	    sino = 1.0;
	    coso = 0.0;
	    sinr = 0.0;
	    cosr = 1.0;
	} else if (fabs(rota) >= 179.9999) {
	    sino = -1.0;
	    coso = 0.0;
	    sinr = 0.0;
	    cosr = 1.0;
	}
	break;
    default:
    /* error, you didn't specify a projection */
	perror("ERROR in map_init -- no projection specified.");
	exit(1);
    }
    display_center.lat = .5 * (rlat1 + rlat2);
    display_center.lng = .5 * (rlon1 + rlon2);
    llLowerLeft.lat = rlat1;
    llLowerLeft.lng = rlon1;
    llUpperRight.lat = rlat2;
    llUpperRight.lng = rlon2;
    rad1 = ArcDist(&display_center, &llLowerLeft);
    rad2 = ArcDist(&display_center, &llUpperRight);
    display_radius = max2(rad1, rad2);

    screen_max_x = ZOOM_FACTOR * (double)CANVAS_WD;
    screen_max_y = ZOOM_FACTOR * (double)CANVAS_HT;

    map_translate(rlat1, rlon1, &u_min, &v_min);
    map_translate(rlat2, rlon2, &u_max, &v_max);

    /*
    printf("(u_max, v_max) ===> (%e, %e)\n",u_max, v_max);
    printf("(u_min, v_min) ===> (%e, %e)\n",u_min, v_min);
    */

    /**************************************************************** 
	  Make sure the (u,v) domain for the transformed (lat,lon)
	  coordinates is a square.
     ****************************************************************/

    del_u = (u_max - u_min)/screen_max_x;
    del_v = (v_max - v_min)/screen_max_y;
    if (del_u > del_v) {
	diff_uv = del_u - del_v;
	v_max = v_max + 0.5*diff_uv*screen_max_y;
	v_min = v_min - 0.5*diff_uv*screen_max_y;
    } else {
	diff_uv = del_v - del_u;
	u_max = u_max + 0.5*diff_uv*screen_max_x;
	u_min = u_min - 0.5*diff_uv*screen_max_x;
    }

    /* Find scaling factors */
    map_translate(plat+.5, plon_save, &u1, &v1);
    map_translate(plat-.5, plon_save, &u2, &v2);
    udeg = sqrt((u2-u1)*(u2-u1) + (v2-v1)*(v2-v1));
    meters_per_unit = DEG2RAD*REarth/udeg;
    pix_per_unit = screen_max_y/(v_max - v_min);
} /* end function map_init */

void map_translate(double ilat, double ilon, double *u, double *v)

/********VARIABLES**********************************************
 *
 *  ilat, ilon are the input latitude and longitude.
 *
 *  rota === rotation factor (input)
 *
 *  u, v === projections of ilat and ilon on the transformed plane (u,v
 *	   plane) in the specified projection
 *
 *****************************************************************/

{
    /* global map initialization paramaters from mapinit.c */
    extern double rota, sino, coso, cosr, sinr;

    /* local variables */
    double tmp1, tmp2, cosa, sina, cosb, sinb;
    double chi, r, sinph, sinla, cosph, cosla, amin1;
    double tcos;

    /* diagnostics variables ********/
    double sign_arg1, sign_arg2;

  /*
   * set up u and v for the for the fast paths.
   *  u ==> longitude in degrees. -180.0 <= u <= 180.0
   *  v ==> latitude in degrees.
   */

    tmp1 = ilon - plon;
    sign_arg1 = SIGN(180.0, tmp1+180.0);
    sign_arg2 = SIGN(180.0, 180.0-tmp1); 
    *u = tmp1 - sign_arg1 + sign_arg2;
    *v = ilat;
  
  /* fast path cylindrical projections require that plat==rota==0.0 */
    switch (iprj) {
    case F_PROJ_FAST_MERC:
	if (fabs(ilat) > 89.9999){ 
	    /* invisible or undefined projection */
	    perror("Undefined projection in map_translate; unable to do fast-path Mercator");
	    exit(1);
	}
	*u = (*u)*DEG2RAD;
	*v = log(tan((ilat+90.0) * DEG2RAD * .5));
	break;
    case F_PROJ_CONIC:
	chi = 90.0 - sino*ilat;
	if (chi >= 179.9999){
	    perror("Undefined projection in map_translate; unable to do conformal conic");
	    exit(1);
	}
	r = pow(tan(DEG2RAD * .5 * chi),coso);
	*u = (*u)*coso*DEG2RAD;
	*v = -r*sino*cos(*u);
	*u = r*sin(*u);
	break;
    case F_PROJ_STEREO:
    case F_PROJ_MERC:
	tmp1 = (*u)*DEG2RAD;
	tmp2 = (*v)*DEG2RAD;
	sinph = sin(tmp1);
	sinla = sin(tmp2);
	cosph = cos(tmp1);
	cosla = cos(tmp2);
	tcos = cosla*cosph;

	amin1 = min2(1.0,  sinla*sino + tcos*coso);
	cosa  = max2(-1.0, amin1); 
	sina = sqrt(1.0 - cosa*cosa);

	if (sina < 0.0001) {   /* error trap */
	    sina = 0.0;
	    if (cosa < 0.0){
		/*invisible or undefined projection */
		perror("Error in map_translate; cannot initialize stereographic projection");
		exit(1);
	    }
	    *u = 0.0;
	    *v = 0.0;
	    /* if this point is reached, we have bad initialization */
	    /* parameters bomb out and indicate this */
	    perror("Bad initial parameters in map_translate, no projection can be done");
	    exit(2);
	} /* end error trap */ 

	sinb = cosla*sinph/sina;
	cosb = (sinla*coso - tcos*sino)/sina;

	if (iprj == F_PROJ_STEREO) {
	    if (fabs(sina) < 0.0001)
		r = sina/2.0;
	    else
	        r = (1.0 - cosa)/sina;
	    *u = r*(sinb*cosr + cosb*sinr);
	    *v = r*(cosb*cosr - sinb*sinr);
	} else {
            *u = atan2(sinb*cosr + cosb*sinr, sinb*sinr - cosb*cosr);
            *v = log((1. + cosa)/sina);
	}
    } /* end switch */
} /* end function map_translate */
