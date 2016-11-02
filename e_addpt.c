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
#include "u_create.h"
#include "u_elastic.h"
#include "u_search.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_mousefun.h"

static void	init_point_adding(char *p, int type, int x, int y, int px, int py);
static void	fix_linepoint_adding(int x, int y);
static void	fix_splinepoint_adding(int x, int y);
static void	init_splinepointadding(int px, int py);
static void	init_linepointadding(int px, int py);
static void	find_endpoints(F_point *p, double x, double y, F_point **fp, F_point **sp);


extern int new_objmask;
extern void win_setmouseposition(Window w, int x, int y);
extern void pw_vector(Window w, double x1, double y1, double x2, double y2,
   int op, int line_width, int line_style, float style_val, int color);
void splinepoint_adding(F_spline *spline, F_point *left_point, F_point *added_point, F_point *right_point);
extern void draw_line(F_line *line, int op);
extern void draw_spline(F_spline *spline, int op);
extern void remake_control_points(F_spline *s);
extern void clean_up(void);
extern void set_last_prevpoint(F_point *prev_point);
extern void set_last_selectedpoint(F_point *selected_point);
extern void set_action_object(int action, int object);
extern void set_latestspline(F_spline *spline);
void linepoint_adding(F_line *line, F_point *left_point, F_point *added_point);
extern void set_latestline(F_line *line);
extern int close_to_vector(double x1, double y1, double x2, double y2, double xp, double yp, int d, float dd, int *px, int *py);
extern void update_markers(int mask);

void point_adding_selected(void)
{
    set_mousefun("break/add here", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    init_searchproc_left(init_point_adding);
    canvas_leftbut_proc = object_search_left;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(pick9_cursor);
    constrained = MOVE_ARB;
}

/* ARGSUSED */
static void
init_point_adding(char *p, int type, int x, int y, int px, int py)
{
    set_action_on();
    set_mousefun("place new point", "", "cancel");
    draw_mousefun_canvas();
    set_temp_cursor(null_cursor);
    win_setmouseposition(canvas_win, px, py);
    switch (type) {
    case O_POLYLINE:
	cur_l = (F_line *) p;
	init_linepointadding(px, py);
	break;
    case O_SPLINE:
	cur_s = (F_spline *) p;
	init_splinepointadding(px, py);
	break;
    default:
	return;
    }
    /* draw in rubber-band line */
    elastic_linelink();
    canvas_locmove_proc = reshaping_line;
}

static
void wrapup_pointadding(void)
{
    reset_action_on();
    point_adding_selected();
    draw_mousefun_canvas();
}

static
void cancel_pointadding(void)
{
    elastic_linelink();
    /* turn back on all relevant markers */
    update_markers(new_objmask);
    wrapup_pointadding();
}

static
void cancel_line_pointadding(void)
{
    if (left_point != NULL && right_point != NULL)
	pw_vector(canvas_win, left_point->x, left_point->y,
		  right_point->x, right_point->y, INV_PAINT,
		  cur_l->thickness, cur_l->style, cur_l->style_val,
		  cur_l->color);
    cancel_pointadding();
}

/**************************  spline  *******************************/

static void
init_splinepointadding(int px, int py)
{
    find_endpoints(cur_s->points, (double)px, (double)py,
			&left_point, &right_point);

    cur_x = fix_x = (double)px;
    cur_y = fix_y = (double)py;
    canvas_leftbut_proc = fix_splinepoint_adding;
    canvas_rightbut_proc = cancel_pointadding;
    /* turn off all markers */
    update_markers(0);
}

static
void fix_splinepoint_adding(int x, int y)
{
    F_point	   *p;

    (*canvas_locmove_proc) (x, y);
    if ((p = create_point()) == NULL) {
	wrapup_pointadding();
	return;
    }
    p->x = cur_x;
    p->y = cur_y;
    elastic_linelink();
    splinepoint_adding(cur_s, left_point, p, right_point);
    /* turn back on all relevant markers */
    update_markers(new_objmask);
    wrapup_pointadding();
}

/*
 * Added_point is always inserted between left_point and
 * right_point, except in two cases. (1) left_point is NULL, the added_point
 * will be prepended to the list of points. This case will never occur if the
 * spline is closed (periodic). (2) right_point is NULL, the added_point will
 * be appended to the end of the list.
 */

void splinepoint_adding(F_spline *spline, F_point *left_point, F_point *added_point, F_point *right_point)
{
    F_control	   *c;

    if (int_spline(spline)) {	/* Interpolated spline */
	if ((c = create_cpoint()) == NULL)
	    return;
    }
    set_temp_cursor(wait_cursor);
    draw_spline(spline, ERASE); /* erase old spline */
    if (left_point == NULL) {
	added_point->next = spline->points;
	spline->points = added_point;
    } else {
	added_point->next = right_point;
	left_point->next = added_point;
    }

    if (int_spline(spline)) {	/* Interpolated spline */
	c->next = spline->controls;
	spline->controls = c;
	remake_control_points(spline);
    }
    draw_spline(spline, PAINT); /* draw the modified spline */
    clean_up();
    set_modifiedflag();
    set_last_prevpoint(left_point);
    set_last_selectedpoint(added_point);
    set_action_object(F_ADD_POINT, O_SPLINE);
    set_latestspline(spline);
    reset_cursor();
}

/***************************  line  ********************************/

static void
init_linepointadding(int px, int py)
{
    find_endpoints(cur_l->points, (double)px, (double)py,
			&left_point, &right_point);

    /* set cur_x etc at new point coords */
    cur_x = fix_x = (double)px;
    cur_y = fix_y = (double)py;
    /* erase line segment where new point is */
    if (left_point != NULL && right_point != NULL)
	pw_vector(canvas_win, left_point->x, left_point->y,
		  right_point->x, right_point->y, ERASE,
		  cur_l->thickness, cur_l->style, cur_l->style_val,
		  cur_l->color);

    /* draw in rubber-band line */
    canvas_leftbut_proc = fix_linepoint_adding;
    canvas_rightbut_proc = cancel_line_pointadding;;
    /* turn off all markers */
    update_markers(0);
}

static
void fix_linepoint_adding(int x, int y)
{
    F_point	   *p;

    (*canvas_locmove_proc) (x, y);
    if ((p = create_point()) == NULL) {
	wrapup_pointadding();
	return;
    }
    p->x = x;
    p->y = y;
    elastic_linelink();
    linepoint_adding(cur_l, left_point, p);
    /* turn back on all relevant markers */
    update_markers(new_objmask);
    wrapup_pointadding();
}

void linepoint_adding(F_line *line, F_point *left_point, F_point *added_point)
{
    /* turn off all markers */
    update_markers(0);
    draw_line(line, ERASE);
    if (left_point == NULL) {
	added_point->next = line->points;
	line->points = added_point;
    } else {
	added_point->next = left_point->next;
	left_point->next = added_point;
    }
    draw_line(line, PAINT);
    clean_up();
    set_action_object(F_ADD_POINT, O_POLYLINE);
    set_latestline(line);
    set_last_prevpoint(left_point);
    set_last_selectedpoint(added_point);
    set_modifiedflag();
}

/*******************************************************************/

/*
 * If (x,y) is close to a point, q, fp points to q and sp points to q->next
 * (right).  However if q is the first point, fp contains NULL and sp points
 * to q.
 */

static void
find_endpoints(F_point *p, double x, double y, F_point **fp, F_point **sp)
{
    int		    d;
    F_point	   *a = NULL, *b = p;

    if (x == b->x && y == b->y) {
	*fp = a;
	*sp = b;
	return;
    }
    for (a = p, b = p->next; b != NULL; a = b, b = b->next) {
	if (x == b->x && y == b->y) {
	    *fp = b;
	    *sp = b->next;
	    return;
	}
	if (close_to_vector(a->x, a->y, b->x, b->y, x, y, 1, 1.0, &d, &d)) {
	    *fp = a;
	    *sp = b;
	    return;
	}
    }
    *fp = a;
    *sp = b;
}
