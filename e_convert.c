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
#include "u_list.h"
#include "u_search.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_mousefun.h"

static void	init_convert(char *p, int type, int x, int y, int px, int py);


void line_2_spline (F_line *l);
void spline_2_line (F_spline *s);
extern int num_points (F_point *points);
extern void put_msg (const char*, ...);
extern int create_control_list (F_spline *s);
extern void free_splinestorage (F_spline *s);
extern void remake_control_points (F_spline *s);
extern void mask_toggle_linemarker (F_line *l);
extern void draw_line (F_line *line, int op);
extern void free_linestorage (F_line *l);
extern void draw_spline (F_spline *spline, int op);
extern void mask_toggle_splinemarker (F_spline *s);
extern void clean_up (void);
extern void set_action_object (int action, int object);
extern void set_latestspline (F_spline *spline);
extern void set_latestline (F_line *line);

void convert_selected(void)
{
    set_mousefun("spline<->line", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    init_searchproc_left(init_convert);
    canvas_leftbut_proc = object_search_left;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(pick15_cursor);
}

/* ARGSUSED */
static
void init_convert(char *p, int type, int x, int y, int px, int py)
{
    switch (type) {
    case O_POLYLINE:
	cur_l = (F_line *) p;
	(void)line_2_spline(cur_l);
	break;
    case O_SPLINE:
	cur_s = (F_spline *) p;
	(void)spline_2_line(cur_s);
	break;
    default:
	return;
    }
}

void line_2_spline(F_line *l)
{
    F_spline	   *s;

    if (num_points(l->points) < 3) {
	extern int num_points (F_point *points);
	put_msg("Can't CONVERT this line into a spline: insufficient points");
	return;
    }
    if ((s = create_spline()) == NULL)
	return;

    s->type = T_OPEN_INTERP;
    s->style = l->style;
    s->thickness = l->thickness;
    s->color = l->color;
    s->style_val = l->style_val;
    s->points = l->points;
    s->controls = NULL;
    s->next = NULL;

    if (-1 == create_control_list(s)) {
	extern int create_control_list (F_spline *s);
	free_splinestorage(s);
	return;
    }
    remake_control_points(s);

    /* now we have finished creating the spline, we can get rid of the line */
    /* first off the screen */
    mask_toggle_linemarker(l);
    draw_line(l, ERASE);
    list_delete_line(&objects.lines, l);
    l->points = NULL;
    /* now get rid of the rest */
    free_linestorage(l);

    /* now put back the new spline */
    draw_spline(s, PAINT);
    mask_toggle_splinemarker(s);
    list_add_spline(&objects.splines, s);
    clean_up();
    set_action_object(F_CONVERT, O_POLYLINE);
    set_latestspline(s);
    return;
}

void spline_2_line(F_spline *s)
{
    F_line	   *l;

    /* Now we turn s into a line */
    if ((l = create_line()) == NULL)
	return;

    l->type = T_POLYLINE;
    l->style = s->style;
    l->thickness = s->thickness;
    l->color = s->color;
    l->style_val = s->style_val;
    l->points = s->points;

    /* now we have finished creating the line, we can get rid of the spline */
    /* first off the screen */
    mask_toggle_splinemarker(s);
    draw_spline(s, ERASE);
    list_delete_spline(&objects.splines, s);
    s->points = NULL;
    /* now get rid of the rest */
    free_splinestorage(s);

    /* and put in the new line */
    draw_line(l, PAINT);
    mask_toggle_linemarker(l);
    list_add_line(&objects.lines, l);
    clean_up();
    set_action_object(F_CONVERT, O_SPLINE);
    set_latestline(l);
    return;
}
