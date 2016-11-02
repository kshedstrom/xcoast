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
#include "u_create.h"
#include "u_elastic.h"
#include "u_list.h"
#include "u_undo.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_mousefun.h"

/* local routine declarations */

static F_point *moved_point;

static void	init_linepointmoving(void);
static void	init_splinepointmoving(void);

static void	relocate_linepoint(F_line *line, int x, int y, F_point *moved_point, F_point *left_point);
static void	relocate_splinepoint(F_spline *s, int x, int y, F_point *moved_point);

static void	init_move_point(char *obj, int type, int x, int y, F_point *p, F_point *q);
static void	init_arb_move_point(char *obj, int type, int x, int y, F_point *p, F_point *q);
static void	init_stretch_move_point(char *obj, int type, int x, int y, F_point *p, F_point *q);

static void	fix_movedsplinepoint(int x, int y);
static void	fix_movedlinepoint(int x, int y);

static void	cancel_movedsplinepoint(void);
static void	cancel_movedlinepoint(void);


extern void clean_up (void);
extern void set_latestspline (F_spline *spline);
extern void set_action_object (int action, int object);
extern void draw_spline (F_spline *spline, int op);
extern void remake_control_points (F_spline *s);
extern void set_latestline (F_line *line);
extern void draw_line (F_line *line, int op);
extern void update_markers(int mask);
extern int new_objmask;

void move_point_selected(void)
{
    set_mousefun("move point", "horiz/vert move", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    init_searchproc_left(init_arb_move_point);
    init_searchproc_middle(init_stretch_move_point);
    canvas_leftbut_proc = point_search_left;
    canvas_middlebut_proc = point_search_middle;
    canvas_rightbut_proc = null_proc;
    set_cursor(pick9_cursor);
}

static
void init_arb_move_point(char *obj, int type, int x, int y, F_point *p, F_point *q)
{
    constrained = MOVE_ARB;
    init_move_point(obj, type, x, y, p, q);
    set_mousefun("new posn", "", "cancel");
    draw_mousefun_canvas();
    canvas_middlebut_proc = null_proc;
}

static
void init_stretch_move_point(char *obj, int type, int x, int y, F_point *p, F_point *q)
{
    constrained = MOVE_HORIZ_VERT;
    init_move_point(obj, type, x, y, p, q);
    set_mousefun("", "new posn", "cancel");
    draw_mousefun_canvas();
    canvas_middlebut_proc = canvas_leftbut_proc;
    canvas_leftbut_proc = null_proc;
}

/* ARGSUSED */
static
void init_move_point(char *obj, int type, int x, int y, F_point *p, F_point *q)
{
    left_point = p;
    moved_point = q;
    switch (type) {
    case O_POLYLINE:
	cur_l = (F_line *) obj;
	right_point = q->next;
	init_linepointmoving();
	break;
    case O_SPLINE:
	cur_s = (F_spline *) obj;
	right_point = q->next;
	init_splinepointmoving();
	break;
    default:
	return;
    }
}

static
void wrapup_movepoint(void)
{
    reset_action_on();
    move_point_selected();
    draw_mousefun_canvas();
}

/**************************  spline  *******************************/

static
void init_splinepointmoving(void)
{
    set_action_on();
    /* turn off all markers */
    update_markers(0);
    from_x = cur_x = moved_point->x;
    from_y = cur_y = moved_point->y;
    set_temp_cursor(crosshair_cursor);
    elastic_linelink();
    canvas_locmove_proc = reshaping_line;
    canvas_leftbut_proc = fix_movedsplinepoint;
    canvas_rightbut_proc = cancel_movedsplinepoint;
}

static
void cancel_movedsplinepoint(void)
{
    elastic_linelink();
    /* turn back on all relevant markers */
    update_markers(new_objmask);
    wrapup_movepoint();
}

static
void fix_movedsplinepoint(int x, int y)
{
    (*canvas_locmove_proc) (x, y);
    elastic_linelink();
    old_s = copy_spline(cur_s);
    clean_up();
    set_latestspline(old_s);
    set_action_object(F_CHANGE, O_SPLINE);
    old_s->next = cur_s;
    relocate_splinepoint(cur_s, cur_x, cur_y, moved_point);
    /* turn back on all relevant markers */
    update_markers(new_objmask);
    wrapup_movepoint();
}

static
void relocate_splinepoint(F_spline *s, int x, int y, F_point *moved_point)
{
    set_temp_cursor(wait_cursor);
    draw_spline(s, ERASE);	/* erase old spline */
    moved_point->x = x;
    moved_point->y = y;
    if (int_spline(s))
	remake_control_points(s);
    draw_spline(s, PAINT);	/* draw spline with moved point */
    set_modifiedflag();
    reset_cursor();
}

/***************************  line  ********************************/

static
void init_linepointmoving(void)
{

    set_action_on();
    /* turn off all markers */
    update_markers(0);
    from_x = cur_x = moved_point->x;
    from_y = cur_y = moved_point->y;
    set_temp_cursor(crosshair_cursor);
    elastic_linelink();
    canvas_locmove_proc = reshaping_line;
    canvas_leftbut_proc = fix_movedlinepoint;
    canvas_rightbut_proc = cancel_movedlinepoint;
}

static
void cancel_movedlinepoint(void)
{
    elastic_linelink();
    /* turn back on all relevant markers */
    update_markers(new_objmask);
    wrapup_movepoint();
}

static
void fix_movedlinepoint(int x, int y)
{
    (*canvas_locmove_proc) (x, y);
    elastic_linelink();
    /* make a copy of the original and save as unchanged object */
    old_l = copy_line(cur_l);
    clean_up();
    set_latestline(old_l);
    set_action_object(F_CHANGE, O_POLYLINE);
    old_l->next = cur_l;
    /* now change the original to become the new object */
    relocate_linepoint(cur_l, cur_x, cur_y, moved_point, left_point);
    /* turn back on all relevant markers */
    update_markers(new_objmask);
    wrapup_movepoint();
}

/* ARGSUSED */
static
void relocate_linepoint(F_line *line, int x, int y, F_point *moved_point, F_point *left_point)
{
    draw_line(line, ERASE);
    moved_point->x = x;
    moved_point->y = y;
    set_modifiedflag();
    draw_line(line, PAINT);
}
