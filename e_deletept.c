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
#include "u_search.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_mousefun.h"

static void	init_delete_point(char *obj, int type, int x, int y, F_point *p, F_point *q);


extern int num_points (F_point *points);
extern void put_msg (const char*, ...);
void linepoint_deleting (F_line *line, F_point *prev_point, F_point *selected_point);
void splinepoint_deleting (F_spline *spline, F_point *prev_point, F_point *selected_point);
extern void mask_toggle_splinemarker (F_spline *s);
extern void draw_line (F_line *line, int op);
extern void draw_spline (F_spline *spline, int op);
extern void remake_control_points (F_spline *s);
extern void clean_up (void);
extern void set_action_object (int action, int object);
extern void set_latestspline (F_spline *spline);
extern void set_last_prevpoint (F_point *prev_point);
extern void set_last_selectedpoint (F_point *selected_point);
extern void set_last_nextpoint (F_point *next_point);
extern void mask_toggle_linemarker (F_line *l);
extern void set_latestline (F_line *line);

void delete_point_selected(void)
{
    set_mousefun("delete point", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    init_searchproc_left(init_delete_point);
    canvas_leftbut_proc = point_search_left;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(pick9_cursor);
}

/* ARGSUSED */
static
void init_delete_point(char *obj, int type, int x, int y, F_point *p, F_point *q)
{
    int		    n;

    switch (type) {
    case O_POLYLINE:
	cur_l = (F_line *) obj;
	n = num_points(cur_l->points);
	if (n <= 1) {
	    /* alternative would be to remove the dot altogether */
	    put_msg("A dot must have at least 1 point");
	    return;
	}
	linepoint_deleting(cur_l, p, q);
	break;
    case O_SPLINE:
	cur_s = (F_spline *) obj;
	n = num_points(cur_s->points);
	if (n <= 3) {	/* it must be an open interpolated spline */
	    put_msg("An interpolated spline cannot have less than 3 points");
	    return;
	}
	splinepoint_deleting(cur_s, p, q);
	break;
    default:
	return;
    }
}

/**************************  spline  *******************************/

void splinepoint_deleting(F_spline *spline, F_point *prev_point, F_point *selected_point)
{
    F_point	   *next_point;

    next_point = selected_point->next;
    set_temp_cursor(wait_cursor);
    			/* open spline */
    mask_toggle_splinemarker(spline);
    draw_spline(spline, ERASE);	/* erase the spline */
    if (prev_point == NULL)
	spline->points = next_point;
    else
	prev_point->next = next_point;
    if (int_spline(spline)) {
	F_control      *c;

	c = spline->controls;
	spline->controls = c->next;
	c->next = NULL;
	free((char *) c);
	remake_control_points(spline);
    }
    draw_spline(spline, PAINT);
    mask_toggle_splinemarker(spline);
    clean_up();
    set_action_object(F_DELETE_POINT, O_SPLINE);
    set_latestspline(spline);
    set_last_prevpoint(prev_point);
    set_last_selectedpoint(selected_point);
    set_last_nextpoint(next_point);
    set_modifiedflag();
    reset_cursor();
}

/***************************  line  ********************************/

/*
 * In deleting a point selected_point, linepoint_deleting uses prev_point and
 * next_point of the point. The relationship between the three points is:
 * prev_point->selected_point->next_point except when selected_point is the
 * first point in the list, in which case prev_point will be NULL.
 */
void linepoint_deleting(F_line *line, F_point *prev_point, F_point *selected_point)
{
    F_point	   *next_point;

    next_point = selected_point->next;
    mask_toggle_linemarker(line);
    draw_line(line, ERASE);	/* erase the line */
    if (prev_point == NULL)
	line->points = next_point;
    else
	prev_point->next = next_point;
    draw_line(line, PAINT);
    mask_toggle_linemarker(line);
    clean_up();
    set_modifiedflag();
    set_action_object(F_DELETE_POINT, O_POLYLINE);
    set_latestline(line);
    set_last_prevpoint(prev_point);
    set_last_selectedpoint(selected_point);
    set_last_nextpoint(next_point);
}
