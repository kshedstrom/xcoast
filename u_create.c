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
 *
 */

#include "fig.h"
#include "resources.h"
#include "object.h"
#include "u_create.h"

extern int	cur_linewidth;

/* LOCAL */

static char	Err_mem[] = "Running out of memory.";

/************************ POINTS *************************/


extern int put_msg (const char*, ...);
extern void free_points (F_point *first_point);
extern void free_linestorage (F_line *l);
extern void free_splinestorage (F_spline *s);

F_point	       *
create_point(void)
{
    F_point	   *p;

    if ((p = (F_point *) malloc(POINT_SIZE)) == NULL)
	put_msg(Err_mem);
    return (p);
}

F_control      *
create_cpoint(void)
{
    F_control	   *cp;

    if ((cp = (F_control *) malloc(CONTROL_SIZE)) == NULL)
	put_msg(Err_mem);
    return (cp);
}

F_point	       *
copy_points(F_point *orig_pt)
{
    F_point	   *new_pt, *prev_pt, *first_pt;

    if ((new_pt = create_point()) == NULL)
	return (NULL);

    first_pt = new_pt;
    *new_pt = *orig_pt;
    new_pt->next = NULL;
    prev_pt = new_pt;
    for (orig_pt = orig_pt->next; orig_pt != NULL; orig_pt = orig_pt->next) {
	if ((new_pt = create_point()) == NULL) {
	    free_points(first_pt);
	    return (NULL);
	}
	prev_pt->next = new_pt;
	*new_pt = *orig_pt;
	new_pt->next = NULL;
	prev_pt = new_pt;
    }
    return (first_pt);
}

/************************ LINES *************************/

F_line	       *
create_line(void)
{
    F_line	   *l;

    if ((l = (F_line *) malloc(LINOBJ_SIZE)) == NULL)
	put_msg(Err_mem);
    l->tagged = 0;
    l->next = NULL;
    l->points = NULL;
    return (l);
}

F_line	       *
copy_line(F_line *l)
{
    F_line	   *line;

    if ((line = create_line()) == NULL)
	return (NULL);

    *line = *l;
    line->next = NULL;
    line->points = copy_points(l->points);
    if (NULL == line->points) {
	put_msg(Err_mem);
	free_linestorage(line);
	return (NULL);
    }
    return (line);
}

/************************ COASTS *************************/

F_coast	       *
create_coast(void)
{
    F_coast	   *t;

    if ((t = (F_coast *) malloc(CSTOBJ_SIZE)) == NULL)
	put_msg(Err_mem);
    t->next = NULL;
    t->points = NULL;
    return (t);
}

/************************ GRIDS **************************/

F_grid	       *
create_grid(void)
{
    F_grid	   *g;

    if ((g = (F_grid *) malloc(GRDOBJ_SIZE)) == NULL)
	put_msg(Err_mem);
    g->next = NULL;
    g->points = NULL;
    return (g);
}

/************************ SPLINES *************************/

F_spline       *
create_spline(void)
{
    F_spline	   *s;

    if ((s = (F_spline *) malloc(SPLOBJ_SIZE)) == NULL)
	put_msg(Err_mem);
    s->tagged = 0;
    return (s);
}

F_spline       *
copy_spline(F_spline *s)
{
    F_spline	   *spline;
    F_control	   *new_cp, *orig_cp, *last_cp;

    if ((spline = create_spline()) == NULL)
	return (NULL);

    *spline = *s;
    spline->next = NULL;
    spline->points = copy_points(s->points);
    if (NULL == spline->points) {
	put_msg(Err_mem);
	free_splinestorage(spline);
	return (NULL);
    }
    spline->controls = NULL;
    if (s->controls == NULL)
	return (spline);

    if ((new_cp = create_cpoint()) == NULL) {
	free_splinestorage(spline);
	return (NULL);
    }
    new_cp->next = NULL;
    last_cp = spline->controls = new_cp;
    orig_cp = s->controls;
    *new_cp = *orig_cp;
    for (orig_cp = orig_cp->next; orig_cp != NULL; orig_cp = orig_cp->next) {
	if ((new_cp = create_cpoint()) == NULL) {
	    free_splinestorage(spline);
	    return (NULL);
	}
	last_cp->next = new_cp;
	new_cp->next = NULL;
	*new_cp = *orig_cp;
	last_cp = new_cp;
    }
    return (spline);
}
