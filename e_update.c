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
#include "mode.h"
#include "paintop.h"
#include "u_create.h"
#include "u_list.h"
#include "u_search.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_drawprim.h"
#include "w_indpanel.h"
#include "w_mousefun.h"

extern void	update_current_settings(void);
static void	init_update_object(char *p, int type, int x, int y, int px, int py);
static void	init_update_settings(char *p, int type, int x, int y, int px, int py);

#define	up_part(lv,rv,mask) \
		if (cur_updatemask & (mask)) \
		    (lv) = (rv)


extern void manage_update_buts(void);
extern void put_msg(const char*, ...);
extern void toggle_linemarker(F_line *l);
void update_line(F_line *line);
extern void toggle_splinemarker(F_spline *s);
void update_spline(F_spline *spline);
extern void draw_line(F_line *line, int op);
extern void draw_spline(F_spline *spline, int op);
void update_lines(F_line *lines);
void update_splines(F_spline *splines);
void update_compounds(F_compound *compounds);
extern void compound_bound(F_compound *compound, double *xmin, double *ymin, double *xmax, double *ymax);

void update_selected(void)
{
    set_mousefun("update object", "update settings", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    init_searchproc_left(init_update_object);
    init_searchproc_middle(init_update_settings);
    canvas_leftbut_proc = object_search_left;
    canvas_middlebut_proc = object_search_middle;
    canvas_rightbut_proc = null_proc;
    set_cursor(pick9_cursor);
    /* manage on the update buttons */
    manage_update_buts();
}

/* ARGSUSED */
static
void init_update_settings(char *p, int type, int x, int y, int px, int py)
{

    switch (type) {
    case O_COMPOUND:
	put_msg("There is no support for updating settings from a compound object");
	return;
    case O_POLYLINE:
	cur_l = (F_line *) p;
	up_part(cur_linewidth, cur_l->thickness, I_LINEWIDTH);
	up_part(cur_color, cur_l->color, I_COLOR);
	up_part(cur_linestyle, cur_l->style, I_LINESTYLE);
	up_part(cur_styleval, cur_l->style_val, I_LINESTYLE);
	break;
    case O_SPLINE:
	cur_s = (F_spline *) p;
	up_part(cur_linewidth, cur_s->thickness, I_LINEWIDTH);
	up_part(cur_color, cur_s->color, I_COLOR);
	up_part(cur_linestyle, cur_s->style, I_LINESTYLE);
	up_part(cur_styleval, cur_s->style_val, I_LINESTYLE);
	break;
    default:
	return;
    }
    update_current_settings();
    put_msg("Settings UPDATED");
}

/* ARGSUSED */
static
void init_update_object(char *p, int type, int x, int y, int px, int py)
{
    switch (type) {
    case O_POLYLINE:
	set_temp_cursor(wait_cursor);
	cur_l = (F_line *) p;
	toggle_linemarker(cur_l);
	new_l = copy_line(cur_l);
	update_line(new_l);
	change_line(cur_l, new_l);
	toggle_linemarker(new_l);
	break;
    case O_SPLINE:
	set_temp_cursor(wait_cursor);
	cur_s = (F_spline *) p;
	toggle_splinemarker(cur_s);
	new_s = copy_spline(cur_s);
	update_spline(new_s);
	change_spline(cur_s, new_s);
	toggle_splinemarker(new_s);
	break;
    default:
	return;
    }
    reset_cursor();
    put_msg("Object(s) UPDATED");
}

void update_line(F_line *line)
{
    draw_line(line, ERASE);
    up_part(line->thickness, cur_linewidth, I_LINEWIDTH);
    up_part(line->style, cur_linestyle, I_LINESTYLE);
    up_part(line->style_val, cur_styleval * (cur_linewidth + 1) / 2, 
	    I_LINESTYLE);
    up_part(line->color, cur_color, I_COLOR);
    draw_line(line, PAINT);
}

void update_spline(F_spline *spline)
{
    draw_spline(spline, ERASE);
    up_part(spline->thickness, cur_linewidth, I_LINEWIDTH);
    up_part(spline->style, cur_linestyle, I_LINESTYLE);
    up_part(spline->style_val, cur_styleval * (cur_linewidth + 1) / 2, 
	    I_LINESTYLE);
    up_part(spline->color, cur_color, I_COLOR);
    draw_spline(spline, PAINT);
}

void update_compound(F_compound *compound)
{
    update_lines(compound->lines);
    update_splines(compound->splines);
    compound_bound(compound, &compound->nwcorner.x, &compound->nwcorner.y,
		   &compound->secorner.x, &compound->secorner.y);
}

void update_compounds(F_compound *compounds)
{
    F_compound	   *c;

    for (c = compounds; c != NULL; c = c->next)
	update_compound(c);
}

void update_lines(F_line *lines)
{
    F_line	   *l;

    for (l = lines; l != NULL; l = l->next)
	update_line(l);
}

void update_splines(F_spline *splines)
{
    F_spline	   *s;

    for (s = splines; s != NULL; s = s->next)
	update_spline(s);
}
