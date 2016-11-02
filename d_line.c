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
#include "u_list.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_mousefun.h"

/*************************** locally global variables *********************/

static void	init_line_drawing(int x, int y);

void		create_lineobject(int x, int y);
void		get_intermediatepoint(int x, int y);

/**********************	 polyline and polygon section  **********************/


void init_trace_drawing(double x, double y);
extern void free_points(F_point *first_point);
extern void win_setmouseposition(Window w, int x, int y);
extern void append_point(double x, double y, F_point **point);
extern void redisplay_line(F_line *line);

void line_drawing_selected(void)
{
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_leftbut_proc = init_line_drawing;
    canvas_rightbut_proc = null_proc;
    set_cursor(arrow_cursor);
    reset_action_on();
    set_mousefun("first point", "single point", "");
    min_num_points = 1;
    num_point = 0;
    fix_x = fix_y = -1;
    canvas_middlebut_proc = create_lineobject;
}

static
void init_line_drawing(int x, int y)
{
    init_trace_drawing((double)x, (double)y);
}

void cancel_line_drawing(void)
{
    elastic_line();
    cur_x = fix_x;
    cur_y = fix_y;
    if (cur_point != first_point)
	elastic_moveline(first_point);	/* erase control vector */
    free_points(first_point);
    return_proc();
    draw_mousefun_canvas();
}

void init_trace_drawing(double x, double y)
{
    if ((first_point = create_point()) == NULL)
	return;

    cur_point = first_point;
    set_action_on();
    cur_point->x = fix_x = cur_x = x;
    cur_point->y = fix_y = cur_y = y;
    cur_point->next = NULL;
    canvas_locmove_proc = freehand_line;
    canvas_leftbut_proc = get_intermediatepoint;
    canvas_middlebut_save = create_lineobject;
    canvas_rightbut_proc = cancel_line_drawing;
    return_proc = line_drawing_selected;
    num_point = 1;
    set_mousefun("next point", "", "cancel");
    if (num_point >= min_num_points - 1) {
	set_mousefun("next point", "final point", "cancel");
	canvas_middlebut_proc = canvas_middlebut_save;
    }
    draw_mousefun_canvas();
    set_temp_cursor(null_cursor);
    cur_cursor = null_cursor;
    elastic_line();
}

void get_intermediatepoint(int x, int y)
{
    (*canvas_locmove_proc) (x, y);
    num_point++;
    fix_x = cur_x;
    fix_y = cur_y;
    elastic_line();
    if (cur_cursor != null_cursor) {
    set_temp_cursor(null_cursor);
	cur_cursor = null_cursor;
    }
    win_setmouseposition(canvas_win, (int)cur_x, (int)cur_y);
    append_point(fix_x, fix_y, &cur_point);
    if (num_point == min_num_points - 1) {
	set_mousefun("next point", "final point", "cancel");
	draw_mousefun_canvas();
	canvas_middlebut_proc = canvas_middlebut_save;
    }
}

/* come here upon pressing middle button (last point of lineobject) */

void create_lineobject(int x, int y)
{
    F_line	   *line;
    int		    dot;

    if (num_point == 0) {
	if ((first_point = create_point()) == NULL) {
	    line_drawing_selected();
	    draw_mousefun_canvas();
	    return;
	}
	cur_point = first_point;
	first_point->x = fix_x = cur_x = (double)x;
	first_point->y = fix_y = cur_y = (double)y;
	first_point->next = NULL;
	num_point++;
    } else if ((double)x != fix_x || (double)y != fix_y) {
	get_intermediatepoint(x, y);
    }
    dot = (num_point == 1);
    elastic_line();
    if ((line = create_line()) == NULL) {
	line_drawing_selected();
	draw_mousefun_canvas();
	return;
    }
    line->type = T_POLYLINE;
    line->thickness = cur_linewidth;
    line->style = cur_linestyle;
    line->style_val = cur_styleval * (cur_linewidth + 1) / 2;
    line->color = cur_color;
    line->points = first_point;
    if (!dot) {
	cur_x = fix_x;
	cur_y = fix_y;
	elastic_moveline(first_point);	/* erase temporary outline */
    }
    redisplay_line(line);
    add_line(line);
    line_drawing_selected();
    draw_mousefun_canvas();
}
