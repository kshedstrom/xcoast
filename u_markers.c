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
#include "w_zoom.h"

#define set_marker(win,x,y,w,h,z1,z2) \
	XDrawRectangle(tool_d,(win),gccache[INV_PAINT], \
	   ZOOMX(x)-((w-1)/2),ZOOMY(y)-((h-1)/2),(w),(h));

#ifdef notdef
/* not used in present implementation */
static u_int	marker_pattern[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

mpr_static(pmarker, 5, 5, 1, marker_pattern);
#endif


extern int set_line_stuff (int width, int style, float style_val, int op, int color);
void toggle_linemarker (F_line *l);
void toggle_splinemarker (F_spline *s);
void toggle_linehighlight (F_line *l);
void toggle_splinehighlight (F_spline *s);

void toggle_csrhighlight(int x, int y)
{
    set_line_stuff(1, RUBBER_LINE, 0.0, (INV_PAINT), DEFAULT_COLOR);
    set_marker(canvas_win, x - 2, y - 2, 5, 5, 0, 0);
    set_marker(canvas_win, x - 1, y - 1, 3, 3, 0, 0);
}

int anyline_in_mask(void)
{
    return (cur_objmask & M_POLYLINE_LINE);
}

int validline_in_mask(F_line *l)
{
    return ((l->type == T_POLYLINE) && (cur_objmask & M_POLYLINE_LINE));
}

int anyspline_in_mask(void)
{
    return (cur_objmask & M_SPLINE_O_INTERP);
}

int validspline_in_mask(F_spline *s)
{
    return ((s->type == T_OPEN_INTERP) && (cur_objmask & M_SPLINE_O_INTERP));
}

void mask_toggle_linemarker(F_line *l)
{
    if (validline_in_mask(l))
	toggle_linemarker(l);
}

void mask_toggle_splinemarker(F_spline *s)
{
    if (validspline_in_mask(s))
	toggle_splinemarker(s);
}

void toggle_markers_in_compound(F_compound *cmpnd)
{
    F_line	   *l;
    F_spline	   *s;
    register int    mask;

    mask = cur_objmask;
    if (mask & M_POLYLINE_LINE)
	for (l = cmpnd->lines; l != NULL; l = l->next) {
	    if ((l->type == T_POLYLINE) && (mask & M_POLYLINE_LINE))
		toggle_linemarker(l);
	}
    if (mask & M_SPLINE_O_INTERP)
	for (s = cmpnd->splines; s != NULL; s = s->next) {
	    if ((s->type == T_OPEN_INTERP) && (mask & M_SPLINE_O_INTERP))
		toggle_splinemarker(s);
	}
}

void update_markers(int mask)
{
    F_line	   *l;
    F_spline	   *s;
    register int    oldmask, newmask;

    oldmask = cur_objmask;
    newmask = mask;
    if ((oldmask & M_POLYLINE_LINE) != (newmask & M_POLYLINE_LINE))
	for (l = objects.lines; l != NULL; l = l->next) {
	    if ((l->type == T_POLYLINE) &&
	    ((oldmask & M_POLYLINE_LINE) != (newmask & M_POLYLINE_LINE)))
		toggle_linemarker(l);
	}
    if ((oldmask & M_SPLINE_O_INTERP) != (newmask & M_SPLINE_O_INTERP)) 
	for (s = objects.splines; s != NULL; s = s->next) {
	    if ((s->type == T_OPEN_INTERP) &&
		 ((oldmask & M_SPLINE_O_INTERP) !=
		  (newmask & M_SPLINE_O_INTERP)))
		toggle_splinemarker(s);
	}
    cur_objmask = newmask;
}

void toggle_linemarker(F_line *l)
{
    F_point	   *p;
    int		    fx, fy, x, y;

    set_line_stuff(1, RUBBER_LINE, 0.0, (INV_PAINT), DEFAULT_COLOR);
    p = l->points;
    fx = p->x;
    fy = p->y;
    for (p = p->next; p != NULL; p = p->next) {
	x = p->x;
	y = p->y;
	set_marker(canvas_win, x, y, 5, 5, 0, 0);
    }
    if (x != fx || y != fy || l->points->next == NULL) {
	set_marker(canvas_win, fx, fy, 5, 5, 0, 0);
    }
    if (l->tagged)
	toggle_linehighlight(l);
}

void toggle_linehighlight(F_line *l)
{
    F_point	   *p;
    int		    fx, fy, x, y;

    set_line_stuff(1, RUBBER_LINE, 0.0, (INV_PAINT), DEFAULT_COLOR);
    p = l->points;
    fx = p->x;
    fy = p->y;
    for (p = p->next; p != NULL; p = p->next) {
	x = p->x;
	y = p->y;
	set_marker(canvas_win, x, y, 1, 1, 0, 0);
	set_marker(canvas_win, x - 1, y - 1, 3, 3, 0, 0);
    }
    if (x != fx || y != fy) {
	set_marker(canvas_win, fx, fy, 1, 1, 0, 0);
	set_marker(canvas_win, fx - 1, fy - 1, 3, 3, 0, 0);
    }
}

void toggle_splinemarker(F_spline *s)
{
    F_point	   *p;
    int		    fx, fy, x, y;

    set_line_stuff(1, RUBBER_LINE, 0.0, (INV_PAINT), DEFAULT_COLOR);
    p = s->points;
    fx = p->x;
    fy = p->y;
    for (p = p->next; p != NULL; p = p->next) {
	x = p->x;
	y = p->y;
	set_marker(canvas_win, x, y, 5, 5, 0, 0);
    }
    if (x != fx || y != fy) {
	set_marker(canvas_win, fx, fy, 5, 5, 0, 0);
    }
    if (s->tagged)
	toggle_splinehighlight(s);
}

void toggle_splinehighlight(F_spline *s)
{
    F_point	   *p;
    int		    fx, fy, x, y;

    set_line_stuff(1, RUBBER_LINE, 0.0, (INV_PAINT), DEFAULT_COLOR);
    p = s->points;
    fx = p->x;
    fy = p->y;
    for (p = p->next; p != NULL; p = p->next) {
	x = p->x;
	y = p->y;
	set_marker(canvas_win, x, y, 1, 1, 0, 0);
	set_marker(canvas_win, x - 1, y - 1, 3, 3, 0, 0);
    }
    if (x != fx || y != fy) {
	set_marker(canvas_win, fx, fy, 1, 1, 0, 0);
	set_marker(canvas_win, fx - 1, fy - 1, 3, 3, 0, 0);
    }
}
