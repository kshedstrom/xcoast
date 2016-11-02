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
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "resources.h"
#include "u_coords.h"
#include "u_elastic.h"
#include "w_canvas.h"
#include "w_setup.h"


/* global variables */
double          pix_per_unit, meters_per_unit;

extern void             map_translate(double ilat, double ilon, double *u, double *v);

extern int		num_point;
extern F_point		*first_point, *cur_point;
extern F_compound	objects;
extern char		data_file[200];

double			u_min, v_min; 
double			u_max, v_max;
double			screen_max_x, screen_max_y;

static int		i_min, i_max, j_min, j_max;

extern void		create_coastobject(void);
extern void		init_coast_drawing(double x, double y);
extern void		get_coast_point(double x, double y);


extern int printf (const char *, ...);
extern int put_msg (const char*, ...);
extern int fscanf (FILE *, const char *, ...);
extern int fclose (FILE *);

void
draw_coast(void)
{
    double	 u1, u2, v1, v2;

    FILE        *database;
    float        lat, lon;

    double       x1, y1, x2, y2;
    int          inwind1, inwind2;

    if ((database = fopen(data_file, "r")) == NULL)
	return;
    else {

	printf("Inside draw_coast function...\n");
	put_msg("PLEASE WAIT; Coastline Drawing in Progress...");

	i_min = 0;
	i_max = screen_max_x;
	j_min = 0;
	j_max = screen_max_y;

	while (fscanf(database,"%f %f",&lat, &lon) != EOF) {
	    if (lat > 900.) {
	        fscanf(database,"%f %f",&lat, &lon);
	    }
		lon = (lon < plon-180) ? lon+360 : lon;
		lon = (lon > plon+180) ? lon-360 : lon;
		map_translate(lat, lon, &u1, &v1);
		x1 = (u1 - u_min)*pix_per_unit;
		y1 = screen_max_y - (v1 - v_min)*pix_per_unit;
		if (x1 > i_max || x1 < i_min ||
			y1 > j_max || y1 < j_min ) {
            	    inwind1 = FALSE;
	    	} else {
		    inwind1 = TRUE;
		    init_coast_drawing(x1, y1);
		}

		while(fscanf(database,"%f %f",&lat, &lon), lat < 91) {
		    lon = (lon < plon-180) ? lon+360 : lon;
        	    lon = (lon > plon+180) ? lon-360 : lon;
        	    if ((lat < 99.0))
			inwind2 = TRUE;
        	    else
			inwind2 = FALSE;
        	    if (inwind2) {
			map_translate(lat, lon, &u2, &v2);
			x2 = (u2 - u_min)*pix_per_unit;
			y2 = screen_max_y - (v2 - v_min)*pix_per_unit;
			if (x2 == x1 && y2 == y1)
			    continue;
			if (x2 > i_max || x2 < i_min || y2 > j_max || y2 < j_min )
			    inwind2 = FALSE;
			else
			    inwind2 = TRUE;
		    }
          
		    if (inwind1 && inwind2) {
			get_coast_point(x2, y2);
		    }
		    if (inwind1 && !inwind2) {
			create_coastobject();
		    }
		    if (!inwind1 && inwind2) {
			init_coast_drawing(x2,y2);
		    }
		    x1 = x2;
		    y1 = y2;
		    inwind1 = inwind2;
  		}
		if (inwind1) create_coastobject();
	} /* end !EOF */
        fclose(database);
        printf("Done with coast drawing function...\n");
        put_msg("Done with coastal drawing utility.");
        return;
    }
}
