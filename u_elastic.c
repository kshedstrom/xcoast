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
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "u_elastic.h"
#include "w_canvas.h"
#include "w_setup.h"
#include "w_zoom.h"


/********************** EXPORTS **************/

int		constrained;
double		fix_x, fix_y;
double		x1off, x2off, y1off, y2off;
double		from_x, from_y;
double		cosa, sina;
F_point	       *left_point, *right_point;

/*************************** BOXES *************************/


extern int pw_vector (Window w, double x1, double y1, double x2, double y2,
  int op, int line_width, int line_style, float style_val, int color);

void elastic_box(int x1, int y1, int x2, int y2)
{
    /* line_style = RUBBER_LINE so that we don't scale it */
    pw_vector(canvas_win, (double)x1, (double)y1, (double)x1, (double)y2, INV_PAINT, 1, RUBBER_LINE, 0.0,
	      DEFAULT_COLOR);
    pw_vector(canvas_win, (double)x1, (double)y2, (double)x2, (double)y2, INV_PAINT, 1, RUBBER_LINE, 0.0,
	      DEFAULT_COLOR);
    pw_vector(canvas_win, (double)x2, (double)y2, (double)x2, (double)y1, INV_PAINT, 1, RUBBER_LINE, 0.0,
	      DEFAULT_COLOR);
    pw_vector(canvas_win, (double)x2, (double)y1, (double)x1, (double)y1, INV_PAINT, 1, RUBBER_LINE, 0.0,
	      DEFAULT_COLOR);
}

void elastic_movebox(void)
{
    register double    x1, y1, x2, y2;

    x1 = cur_x + x1off;
    x2 = cur_x + x2off;
    y1 = cur_y + y1off;
    y2 = cur_y + y2off;
    elastic_box(x1, y1, x2, y2);
}

void moving_box(double x, double y)
{
    elastic_movebox();
    adjust_pos(x, y, fix_x, fix_y, &cur_x, &cur_y);
    elastic_movebox();
}

/*************************** LINES *************************/

void elastic_line(void)
{
    pw_vector(canvas_win, fix_x, fix_y, cur_x, cur_y,
	      INV_PAINT, 1, RUBBER_LINE, 0.0, DEFAULT_COLOR);
}

void freehand_line(int x, int y)
{
    elastic_line();
    cur_x = (double)x;
    cur_y = (double)y;
    elastic_line();
}

void reshaping_line(int x, int y)
{
    elastic_linelink();
    adjust_pos((double)x, (double)y, from_x, from_y, &cur_x, &cur_y);
    elastic_linelink();
}

void elastic_linelink(void)
{
    if (left_point != NULL) {
	pw_vector(canvas_win, left_point->x, left_point->y,
	       cur_x, cur_y, INV_PAINT, 1, RUBBER_LINE, 0.0, DEFAULT_COLOR);
    }
    if (right_point != NULL) {
	pw_vector(canvas_win, right_point->x, right_point->y,
	       cur_x, cur_y, INV_PAINT, 1, RUBBER_LINE, 0.0, DEFAULT_COLOR);
    }
}

void elastic_moveline(F_point *pts)
{
    F_point	   *p;
    double	    dx, dy, x, y, xx, yy;

    p = pts;
    if (p->next == NULL) {	/* dot */
	pw_vector(canvas_win, cur_x, cur_y, cur_x, cur_y,
		  INV_PAINT, 1, RUBBER_LINE, 0.0, DEFAULT_COLOR);
    } else {
	dx = cur_x - fix_x;
	dy = cur_y - fix_y;
	x = p->x + dx;
	y = p->y + dy;
	for (p = p->next; p != NULL; x = xx, y = yy, p = p->next) {
	    xx = p->x + dx;
	    yy = p->y + dy;
	    pw_vector(canvas_win, x, y, xx, yy, INV_PAINT, 1,
		      RUBBER_LINE, 0.0, DEFAULT_COLOR);
	}
    }
}

/*********** AUXILIARY FUNCTIONS FOR CONSTRAINED MOVES ******************/

void adjust_pos(double curs_x, double curs_y, double orig_x, double orig_y, double *ret_x, double *ret_y)
{
    if (constrained) {
	if (abs(orig_x - curs_x) > abs(orig_y - curs_y)) {
	    *ret_x = curs_x;
	    *ret_y = orig_y;
	} else {
	    *ret_x = orig_x;
	    *ret_y = curs_y;
	}
    } else {
	*ret_x = curs_x;
	*ret_y = curs_y;
    }
}
