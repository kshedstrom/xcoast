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


void free_line (F_line **list);
void free_spline (F_spline **list);
void free_coast (F_coast **list);
void free_grid (F_grid **list);
void free_linestorage (F_line *l);
void free_coaststorage (F_coast *t);
void free_gridstorage (F_grid *t);
void free_splinestorage (F_spline *s);
void free_points (F_point *first_point);

void free_compound(F_compound **list)
{
    F_compound	   *c, *compound;

    for (c = *list; c != NULL;) {
	compound = c;
	c = c->next;
	free_line(&compound->lines);
	free_spline(&compound->splines);
	free_coast(&compound->coasts);
	free_grid(&compound->grids);
	free((char *) compound);
    }
    *list = NULL;
}

void free_line(F_line **list)
{
    F_line	   *l, *line;

    for (l = *list; l != NULL;) {
	line = l;
	l = l->next;
	free_linestorage(line);
    }
    *list = NULL;
}

void free_coast(F_coast **list)
{
    F_coast *t, *coast;

    for (t = *list; t != NULL;) {
        coast = t;
        t = t->next;
        free_coaststorage(coast);
    }
    *list = NULL;
}

void free_grid(F_grid **list)
{
    F_grid *g, *grid;

    for (g = *list; g != NULL;) {
        grid = g;
        g = g->next;
        free_gridstorage(grid);
    }
    *list = NULL;
}

void free_spline(F_spline **list)
{
    F_spline	   *s, *spline;

    for (s = *list; s != NULL;) {
	spline = s;
	s = s->next;
	free_splinestorage(spline);
    }
    *list = NULL;
}

/* free up all the GC's before leaving xfig */

void free_GCs(void)
{
	int i;

/*	XFreeGC(tool_d, gc);   */
	XFreeGC(tool_d, bold_gc);
	XFreeGC(tool_d, button_gc);
	XFreeGC(tool_d, color_gc);
	XFreeGC(tool_d, ind_button_gc);
	XFreeGC(tool_d, ind_blank_gc);
	XFreeGC(tool_d, blank_gc);
	XFreeGC(tool_d, mouse_blank_gc);
	XFreeGC(tool_d, mouse_button_gc);
	XFreeGC(tool_d, tr_gc);
	XFreeGC(tool_d, tr_erase_gc);
	XFreeGC(tool_d, tr_xor_gc);
	XFreeGC(tool_d, sr_gc);
	XFreeGC(tool_d, sr_erase_gc);
	XFreeGC(tool_d, sr_xor_gc);

	for (i=0; i<NUMOPS; i++) {
		XFreeGC(tool_d, gccache[i]);
	}
}

void free_splinestorage(F_spline *s)
{
    F_control	   *a, *b;

    free_points(s->points);
    for (a = s->controls; a != NULL; a = b) {
	b = a->next;
	free((char *) a);
    }
    free((char *) s);
}

void free_linestorage(F_line *l)
{
    free_points(l->points);
    free((char *) l);
}

void free_coaststorage(F_coast *t)
{
    free_points(t->points);
    free((char*)t);
}

void free_gridstorage(F_grid *g)
{
    free_points(g->points);
    free((char*)g);
}

void free_points(F_point *first_point)
{
    F_point	   *p, *q;

    for (p = first_point; p != NULL; p = q) {
	q = p->next;
	free((char *) p);
    }
}
