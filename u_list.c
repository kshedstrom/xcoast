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
#include "mode.h"
#include "resources.h"
#include "object.h"
#include "paintop.h"
#include "u_create.h"
#include "u_list.h"
#include "u_elastic.h"
#include "u_undo.h"


extern int clean_up (void);
extern int set_latestline (F_line *line);
extern int set_action_object (int action, int object);
extern int set_latestspline (F_spline *spline);

void
list_delete_line(F_line **line_list, F_line *line)
{
    F_line	   *q, *r;

    if (*line_list == NULL)
	return;
    if (line == NULL)
	return;

    for (q = r = *line_list; r != NULL; q = r, r = r->next) {
	if (r == line) {
	    if (r == *line_list)
		*line_list = (*line_list)->next;
	    else
		q->next = r->next;
	    break;
	}
    }
    line->next = NULL;
}

void
list_delete_spline(F_spline **spline_list, F_spline *spline)
{
    F_spline	   *q, *r;

    if (*spline_list == NULL)
	return;
    if (spline == NULL)
	return;

    for (q = r = *spline_list; r != NULL; q = r, r = r->next) {
	if (r == spline) {
	    if (r == *spline_list)
		*spline_list = (*spline_list)->next;
	    else
		q->next = r->next;
	    break;
	}
    }
    spline->next = NULL;
}

void
list_delete_compound(F_compound **list, F_compound *compound)
{
    F_compound	   *c, *cc;

    if (*list == NULL)
	return;
    if (compound == NULL)
	return;

    for (cc = c = *list; c != NULL; cc = c, c = c->next) {
	if (c == compound) {
	    if (c == *list)
		*list = (*list)->next;
	    else
		cc->next = c->next;
	    break;
	}
    }
    compound->next = NULL;
}

void
list_add_line(F_line **line_list, F_line *l)
{
    F_line	   *ll;

    l->next = NULL;
    if ((ll = last_line(*line_list)) == NULL)
	*line_list = l;
    else
	ll->next = l;
}

void
list_add_coast(F_coast **coast_list, F_coast *t)
{
    F_coast	   *tt;

    t->next = NULL;
    if ((tt = last_coast(*coast_list)) == NULL)
	*coast_list = t;
    else
	tt->next = t;
}

void
list_add_grid(F_grid **grid_list, F_grid *g)
{
    F_grid	   *gg;

    g->next = NULL;
    if ((gg = last_grid(*grid_list)) == NULL)
	*grid_list = g;
    else
	gg->next = g;
}

void
list_add_spline(F_spline **spline_list, F_spline *s)
{
    F_spline	   *ss;

    s->next = NULL;
    if ((ss = last_spline(*spline_list)) == NULL)
	*spline_list = s;
    else
	ss->next = s;
}

void
list_add_compound(F_compound **list, F_compound *c)
{
    F_compound	   *cc;

    c->next = NULL;
    if ((cc = last_compound(*list)) == NULL)
	*list = c;
    else
	cc->next = c;
}

void
delete_line(F_line *old_l)
{
    list_delete_line(&objects.lines, old_l);
    clean_up();
    set_latestline(old_l);
    set_action_object(F_DELETE, O_POLYLINE);
    set_modifiedflag();
}

void
delete_spline(F_spline *old_s)
{
    list_delete_spline(&objects.splines, old_s);
    clean_up();
    set_latestspline(old_s);
    set_action_object(F_DELETE, O_SPLINE);
    set_modifiedflag();
}

void
add_line(F_line *new_l)
{
    list_add_line(&objects.lines, new_l);
    clean_up();
    set_latestline(new_l);
    set_action_object(F_ADD, O_POLYLINE);
    set_modifiedflag();
}

void
add_coast(F_coast *new_t)
{
    list_add_coast(&objects.coasts, new_t);
    clean_up();
    set_modifiedflag();
}

void
add_grid(F_grid *new_g)
{
    list_add_grid(&objects.grids, new_g);
    clean_up();
    set_modifiedflag();
}

void
add_spline(F_spline *new_s)
{
    list_add_spline(&objects.splines, new_s);
    clean_up();
    set_latestspline(new_s);
    set_action_object(F_ADD, O_SPLINE);
    set_modifiedflag();
}

void
change_line(F_line *old_l, F_line *new_l)
{
    list_delete_line(&objects.lines, old_l);
    list_add_line(&objects.lines, new_l);
    clean_up();
    old_l->next = new_l;
    set_latestline(old_l);
    set_action_object(F_CHANGE, O_POLYLINE);
    set_modifiedflag();
}

void
change_spline(F_spline *old_s, F_spline *new_s)
{
    list_delete_spline(&objects.splines, old_s);
    list_add_spline(&objects.splines, new_s);
    clean_up();
    old_s->next = new_s;
    set_latestspline(old_s);
    set_action_object(F_CHANGE, O_SPLINE);
    set_modifiedflag();
}

void tail(F_compound *ob, F_compound *tails)
{
    F_line	   *l;
    F_spline	   *s;

    if (NULL != (l = ob->lines))
	for (; l->next != NULL; l = l->next);
    if (NULL != (s = ob->splines))
	for (; s->next != NULL; s = s->next);

    tails->lines = l;
    tails->splines = s;
}

/*
 * Make pointers in tails point to the last element of each list of l1 and
 * Append the lists in l2 after those in l1. The tails pointers must be
 * defined prior to calling append.
 */
void append_objects(F_compound *l1, F_compound *l2, F_compound *tails)
{
    if (tails->lines)
	tails->lines->next = l2->lines;
    else
	l1->lines = l2->lines;
    if (tails->splines)
	tails->splines->next = l2->splines;
    else
	l1->splines = l2->splines;
    if (tails->coasts)
	tails->coasts->next = l2->coasts;
    else
	l1->coasts = l2->coasts;
    if (tails->grids)
	tails->grids->next = l2->grids;
    else
	l1->grids = l2->grids;
}

/* Cut is the dual of append. */

void cut_objects(F_compound *objects, F_compound *tails)
{
    if (tails->lines)
	tails->lines->next = NULL;
    else
	objects->lines = NULL;
    if (tails->splines)
	tails->splines->next = NULL;
    else
	objects->splines = NULL;
    if (tails->coasts)
	tails->coasts->next = NULL;
    else
	objects->coasts = NULL;
    if (tails->grids)
	tails->grids->next = NULL;
    else
	objects->grids = NULL;
}

void append_point(double x, double y, F_point **point)
{
    F_point	   *p;

    if ((p = create_point()) == NULL)
	return;

    p->x = x;
    p->y = y;
    p->next = NULL;
    (*point)->next = p;
    *point = p;
}

int num_points(F_point *points)
{
    int		    n;
    F_point	   *p;

    for (p = points, n = 0; p != NULL; p = p->next, n++);
    return (n);
}

F_line	       *
last_line(F_line *list)
{
    F_line	   *ll;

    if (list == NULL)
	return NULL;

    for (ll = list; ll->next != NULL; ll = ll->next);
    return ll;
}

F_coast	       *
last_coast(F_coast *list)
{
    F_coast	   *tt;

    if (list == NULL)
	return NULL;

    for (tt = list; tt->next != NULL; tt = tt->next);
    return tt;
}

F_grid	       *
last_grid(F_grid *list)
{
    F_grid	   *gg;

    if (list == NULL)
	return NULL;

    for (gg = list; gg->next != NULL; gg = gg->next);
    return gg;
}

F_spline       *
last_spline(F_spline *list)
{
    F_spline	   *ss;

    if (list == NULL)
	return NULL;

    for (ss = list; ss->next != NULL; ss = ss->next);
    return ss;
}

F_compound     *
last_compound(F_compound *list)
{
    F_compound	   *tt;

    if (list == NULL)
	return NULL;

    for (tt = list; tt->next != NULL; tt = tt->next);
    return tt;
}

F_point	       *
last_point(F_point *list)
{
    F_point	   *tt;

    if (list == NULL)
	return NULL;

    for (tt = list; tt->next != NULL; tt = tt->next);
    return tt;
}

F_point              *
prev_point(F_point *list, F_point *point)
{
    F_point      *csr;

    if (list == point)
	return NULL;

    for (csr = list; csr->next != point; csr = csr->next);
    return csr;
}

F_compound     *
prev_compound(F_compound *list, F_compound *compound)
{
    F_compound	   *csr;

    if (list == compound)
	return NULL;

    for (csr = list; csr->next != compound; csr = csr->next);
    return csr;
}

F_line	       *
prev_line(F_line *list, F_line *line)
{
    F_line	   *csr;

    if (list == line)
	return NULL;

    for (csr = list; csr->next != line; csr = csr->next);
    return csr;
}

F_spline       *
prev_spline(F_spline *list, F_spline *spline)
{
    F_spline	   *csr;

    if (list == spline)
	return NULL;

    for (csr = list; csr->next != spline; csr = csr->next);
    return csr;
}

int
object_count(F_compound *list)
{
    register int    cnt;
    F_line	   *l;
    F_spline	   *s;

    cnt = 0;
    for (l = list->lines; l != NULL; l = l->next, cnt++);
    for (s = list->splines; s != NULL; s = s->next, cnt++);
    return (cnt);
}
