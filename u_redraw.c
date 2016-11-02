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
#include "resources.h"
#include "object.h"
#include "paintop.h"
#include "w_cursor.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"


void redisplay_lineobject(F_line *lines);
void redisplay_splineobject(F_spline *splines);
void redisplay_coastobject(F_coast *coasts);
void redisplay_gridobject(F_grid *grids);
extern int toggle_markers_in_compound(F_compound *cmpnd);
extern void draw_line(F_line *line, int op);
extern void draw_coastline(F_coast *coast, int op);
extern void draw_grids(F_grid *grid, int op);
extern void draw_spline(F_spline *spline, int op);
void redisplay_region(double xmin, double ymin, double xmax, double ymax);
extern int set_clip_window(int xmin, int ymin, int xmax, int ymax);
extern void clear_canvas(void);
extern void redisplay_grid(void);
extern int reset_clip_window(void);
extern int spline_bound(F_spline *s, double *xmin, double *ymin, double *xmax, double *ymax);
void redisplay_regions(double xmin1, double ymin1, double xmax1, double ymax1, double xmin2, double ymin2, double xmax2, double ymax2);
extern int line_bound(F_line *l, double *xmin, double *ymin, double *xmax, double *ymax);
extern int overlapping(double xmin1, double ymin1, double xmax1, double ymax1, double xmin2, double ymin2, double xmax2, double ymax2);

void redisplay_objects(F_compound *objects)
{

    if (objects == NULL)
	return;

    redisplay_lineobject(objects->lines);
    redisplay_splineobject(objects->splines);
    redisplay_coastobject(objects->coasts);
    redisplay_gridobject(objects->grids);
    toggle_markers_in_compound(objects);
}

/*
 * Redisplay a list of lines.
 */

void redisplay_lineobject(F_line *lines)
{
    F_line	   *lp;

    lp = lines;
    while (lp != NULL) {
	draw_line(lp, PAINT);
	lp = lp->next;
    }
}

/*
 * Redisplay a list of coastlines.
 */

void redisplay_coastobject(F_coast *coasts)
{
    F_coast *coast;

    coast = coasts;
    while (coast != NULL) {
        draw_coastline(coast, PAINT);
	coast = coast->next;
    }
}

void redisplay_gridobject(F_grid *grids)
{
    F_grid *grid;

    grid = grids;
    while (grid != NULL) {
        draw_grids(grid, PAINT);
	grid = grid->next;
    }
}

/*
 * Redisplay a list of splines.
 */

void redisplay_splineobject(F_spline *splines)
{
    F_spline	   *spline;

    spline = splines;
    while (spline != NULL) {
	draw_spline(spline, PAINT);
	spline = spline->next;
    }
}

/*
 * Redisplay the entire drawing.
 */
void
redisplay_canvas(void)
{
    redisplay_region(0, 0, CANVAS_WD, CANVAS_HT);
}

void redisplay_region(double xmin, double ymin, double xmax, double ymax)
{
    set_temp_cursor(wait_cursor);
    /* kludge so that markers are redrawn */
    xmin -= 8;
    ymin -= 8;
    xmax += 8;
    ymax += 8;
    set_clip_window((int)xmin, (int)ymin, (int)xmax, (int)ymax);
    clear_canvas();
    redisplay_grid();
    redisplay_objects(&objects);
    reset_clip_window();
    reset_cursor();
}

void redisplay_zoomed_region(double xmin, double ymin, double xmax, double ymax)
{
    redisplay_region(ZOOMX(xmin), ZOOMY(ymin), ZOOMX(xmax), ZOOMY(ymax));
}

void redisplay_spline(F_spline *s)
{
    double	    xmin, ymin, xmax, ymax;

    spline_bound(s, &xmin, &ymin, &xmax, &ymax);
    redisplay_zoomed_region(xmin, ymin, xmax, ymax);
}

void redisplay_splines(F_spline *s1, F_spline *s2)
{
    double	    xmin1, ymin1, xmax1, ymax1;
    double	    xmin2, ymin2, xmax2, ymax2;

    spline_bound(s1, &xmin1, &ymin1, &xmax1, &ymax1);
    spline_bound(s2, &xmin2, &ymin2, &xmax2, &ymax2);
    redisplay_regions(xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2);
}

void redisplay_line(F_line *l)
{
    double	    xmin, ymin, xmax, ymax;

    line_bound(l, &xmin, &ymin, &xmax, &ymax);
    redisplay_zoomed_region(xmin, ymin, xmax, ymax);
}

void redisplay_lines(F_line *l1, F_line *l2)
{
    double	    xmin1, ymin1, xmax1, ymax1;
    double	    xmin2, ymin2, xmax2, ymax2;

    line_bound(l1, &xmin1, &ymin1, &xmax1, &ymax1);
    line_bound(l2, &xmin2, &ymin2, &xmax2, &ymax2);
    redisplay_regions(xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2);
}

void redisplay_compound(F_compound *c)
{
    redisplay_zoomed_region(c->nwcorner.x, c->nwcorner.y,
			    c->secorner.x, c->secorner.y);
}

void redisplay_regions(double xmin1, double ymin1, double xmax1, double ymax1, double xmin2, double ymin2, double xmax2, double ymax2)
{
    if (xmin1 == xmin2 && ymin1 == ymin2 && xmax1 == xmax2 && ymax1 == ymax2) {
	redisplay_zoomed_region(xmin1, ymin1, xmax1, ymax1);
	return;
    }
    /* below is easier than sending clip rectangle array to X */
    if (overlapping(xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2)) {
	redisplay_zoomed_region(min2(xmin1, xmin2), min2(ymin1, ymin2),
				max2(xmax1, xmax2), max2(ymax1, ymax2));
    } else {
	redisplay_zoomed_region(xmin1, ymin1, xmax1, ymax1);
	redisplay_zoomed_region(xmin2, ymin2, xmax2, ymax2);
    }
}
