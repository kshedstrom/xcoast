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

#include "fig.h"
#include "figx.h"
#include "resources.h"
#include "mode.h"
#include "paintop.h"
#include "object.h"
#include "u_coords.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"

#define PARALLEL		0
#define MERIDIAN		1

#define RCT_LAT_DEG		10
#define RCT_LNG_DEG		10
#define RCT_HALFDIAG_DEG	7.1
#define RCT_LAT_RAD		(DEG2RAD * RCT_LAT_DEG)
#define RCT_LNG_RAD		(DEG2RAD * RCT_LNG_DEG)

extern void	redisplay_canvas(void);
extern void	init_grid_drawing(double x, double y);
extern void	get_grid_point(double x, double y);
extern void	create_gridobject(void);
extern void     map_translate(double ilat, double ilon, double *u, double *v);
extern void     free_grid(F_grid **list);

void drawGridSegment(const struct ltln *llVxStart,
		  double step, double maxDist, int m);

void init_grid(void)
{
}

/* grid in xcoast is simply another type of object */

void setup_grid()
{
    struct ltln     llVx;
    double          sLat, sLng, dist;
    int		    i, j, k, maxlat;
    static int	    prev_grid = -1;
    double          u1, u2, u3, u4, v1, v2, v3, v4;
    double          box_u_max, box_v_max, box_u_min, box_v_min;
    extern double   u_min, u_max, v_min, v_max;

    if (gridmode == GRID_0) {
	free_grid(&objects.grids);
    } else {
	if (iprj == F_PROJ_CONIC)
	    maxlat = 90;
	else
	    maxlat = 80;
	for (i = -maxlat; i < maxlat; i += RCT_LAT_DEG) {
	    for (j = -180; j < 180; j += RCT_LNG_DEG) {
                /* grid rectangle center */
		llVx.lat = i + .5 * RCT_LAT_DEG;
		llVx.lng = j + .5 * RCT_LNG_DEG;

		/* two ways to check if inside window */
		dist = ArcDist(&display_center, &llVx);
		if (dist > display_radius + DEG2RAD * RCT_HALFDIAG_DEG)
		    continue;
		map_translate((double)i, (double)j, &u1, &v1);
		map_translate((double)(i + RCT_LAT_DEG),
		     (double)j, &u2, &v2);
		map_translate((double)i,
		     (double)(j + RCT_LNG_DEG), &u3, &v3);
		map_translate((double)(i + RCT_LAT_DEG),
		     (double)(j + RCT_LNG_DEG), &u4, &v4);
		box_v_max = max2(max2(v1,v2),max2(v3,v4));
		box_u_max = max2(max2(u1,u2),max2(u3,u4));
		box_v_min = min2(min2(v1,v2),min2(v3,v4));
		box_u_min = min2(min2(u1,u2),min2(u3,u4));
		if (box_u_max < u_min || box_u_min > u_max)
		    continue;
		if (box_v_max < v_min || box_v_min > v_max)
		    continue;

		/* still there? draw it */
		sLat = (double)RCT_LAT_DEG / gridsize;
		sLng = (double)RCT_LNG_DEG / gridsize;
		for (k = 0; k < gridsize; k++) {
		    llVx.lat = i + k * sLat;
		    llVx.lng = j;
		    drawGridSegment(&llVx, sLng / 4, RCT_LNG_DEG, PARALLEL);
		}
		if (i == 80 && iprj == (F_PROJ_MERC ||
		     iprj == F_PROJ_FAST_MERC) ) continue;
		for (k = 0; k < gridsize; k++) {
		    llVx.lat = i;
		    llVx.lng = j + k * sLng;
		    drawGridSegment(&llVx, sLat / 4, RCT_LAT_DEG, MERIDIAN);
		}
	    }
	}
    }
    if (prev_grid == GRID_0 && gridmode == GRID_0)
	redisplay_canvas();
    prev_grid = gridmode;
}

void
drawGridSegment(const struct ltln *llVxStart,
		  double step, double maxDist, int m)
{
    double		lat, lng, u1, v1, x1, y1;
    double		d = 0.0;
    extern double	u_min, v_min;
    extern double	screen_max_y;
    extern double	pix_per_unit;

    lat = llVxStart->lat;
    lng = llVxStart->lng;
    lng = (lng < plon-180) ? lng+360 : lng;
    lng = (lng > plon+180) ? lng-360 : lng;
    map_translate(lat, lng, &u1, &v1);
    x1 = (u1 - u_min)*pix_per_unit;
    y1 = screen_max_y - (v1 - v_min)*pix_per_unit;
    init_grid_drawing(x1, y1);
    do {
	d += step;
	if (d + 1.0e-10 > maxDist) d = maxDist;
	lat = llVxStart->lat + (m * d);     /* m is 0 or 1 */
	lng = llVxStart->lng + ((1 - m) * d);
	map_translate(lat, lng, &u1, &v1);
	x1 = (u1 - u_min)*pix_per_unit;
	y1 = screen_max_y - (v1 - v_min)*pix_per_unit;
	get_grid_point(x1, y1);
    } while (d < maxDist);
    create_gridobject();
}

void redisplay_grid(void)
{
}
