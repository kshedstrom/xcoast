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
#include "u_list.h"
#include "w_zoom.h"

#define TOLERANCE (zoomscale>1?3:(int)(3/zoomscale))

static void	(*manipulate) ();
static void	(*handlerproc_left) ();
static void	(*handlerproc_middle) ();
static void	(*handlerproc_right) ();
static int	type;
static long	objectcount;
static long	n;
static int	csr_x, csr_y;

static F_point	point1, point2;

static F_line  *l;
static F_spline *s;
static F_compound *c;


extern int anyline_in_mask(void);
extern int validline_in_mask(F_line *l);
extern int close_to_vector(double x1, double y1, double x2, double y2, double xp, double yp, int d, float dd, int *px, int *py);
extern int anyspline_in_mask(void);
extern int validspline_in_mask(F_spline *s);
void toggle_objecthighlight(void);
extern void toggle_linehighlight(F_line *l);
extern void toggle_splinehighlight(F_spline *s);
extern void toggle_csrhighlight(int x, int y);

char
next_line_found(double x, double y, int tolerance, int *px, int *py, unsigned int shift)
{				/* return the pointer to lines object if the
				 * search is successful otherwise return
				 * NULL.  The value returned via (px, py) is
				 * the closest point on the vector to point
				 * (x, y)					 */

    F_point	   *point;
    double	    x1, y1, x2, y2;
    float	    tol2;

    tol2 = (float) tolerance *tolerance;

    if (!anyline_in_mask())
	return (0);
    if (l == NULL)
	l = last_line(objects.lines);
    else if (shift)
	l = prev_line(objects.lines, l);

    for (; l != NULL; l = prev_line(objects.lines, l))
	if (validline_in_mask(l)) {
	    extern int validline_in_mask (F_line *l);
	    n++;
	    point = l->points;
	    x1 = point->x;
	    y1 = point->y;
	    if (fabs(x - x1) <= tolerance && fabs(y - y1) <= tolerance) {
		*px = x1;
		*py = y1;
		return (1);
	    }
	    for (point = point->next; point != NULL; point = point->next) {
		x2 = point->x;
		y2 = point->y;
		if (close_to_vector(x1, y1, x2, y2, x, y, tolerance, tol2,
				    px, py))
		    return (1);
		x1 = x2;
		y1 = y2;
	    }
	}
    return (0);
}

char
next_spline_found(int x, int y, int tolerance, int *px, int *py, unsigned int shift)
{				/* return the pointer to lines object if the
				 * search is successful otherwise return
				 * NULL.  */

    F_point	   *point;
    int		    x1, y1, x2, y2;
    float	    tol2;

    if (!anyspline_in_mask())
	return (0);
    if (s == NULL)
	s = last_spline(objects.splines);
    else if (shift)
	s = prev_spline(objects.splines, s);

    tol2 = (float) tolerance *tolerance;

    for (; s != NULL; s = prev_spline(objects.splines, s))
	if (validspline_in_mask(s)) {
	    extern int validspline_in_mask (F_spline *s);
	    n++;
	    point = s->points;
	    x1 = point->x;
	    y1 = point->y;
	    for (point = point->next; point != NULL; point = point->next) {
		x2 = point->x;
		y2 = point->y;
		if (close_to_vector(x1, y1, x2, y2, x, y, tolerance, tol2,
				    px, py))
		    return (1);
		x1 = x2;
		y1 = y2;
	    }
	}
    return (0);
}

void show_objecthighlight(void)
{
    if (highlighting)
	return;
    highlighting = 1;
    toggle_objecthighlight();
}

void erase_objecthighlight(void)
{
    if (!highlighting)
	return;
    highlighting = 0;
    toggle_objecthighlight();
    if (type == -1) {
	l = NULL;
	type = O_POLYLINE;
    }
}

void toggle_objecthighlight(void)
{
    switch (type) {
    case O_POLYLINE:
	toggle_linehighlight(l);
	break;
    case O_SPLINE:
	toggle_splinehighlight(s);
	break;
    default:
	toggle_csrhighlight(csr_x, csr_y);
    }
}

static void
init_search(void)
{
    if (highlighting)
	erase_objecthighlight();
    else {
	objectcount = 0;
	if (anyline_in_mask())
	    for (l = objects.lines; l != NULL; l = l->next)
		if (validline_in_mask(l))
		    objectcount++;
	if (anyspline_in_mask())
	    for (s = objects.splines; s != NULL; s = s->next)
		if (validspline_in_mask(s))
		    objectcount++;
	l = NULL;
	type = O_POLYLINE;
    }
}

void
do_object_search(int x, int y, unsigned int shift)
       		         
                          	/* Shift Key Status from XEvent */
{
    int		    px, py;
    char	    found = 0;

    init_search();
    for (n = 0; n < objectcount;) {
	switch (type) {
	case O_POLYLINE:
	    found = next_line_found(x, y, TOLERANCE, &px, &py, shift);
	    break;
	case O_SPLINE:
	    found = next_spline_found(x, y, TOLERANCE, &px, &py, shift);
	    break;
	}

	if (found)
	    break;

	switch (type) {
	case O_POLYLINE:
	    type = O_SPLINE;
	    s = NULL;
	    break;
	case O_SPLINE:
	    type = O_COMPOUND;
	    c = NULL;
	    break;
	case O_COMPOUND:
	    type = O_POLYLINE;
	    l = NULL;
	    break;
	}
    }
    if (!found) {		/* nothing found */
	csr_x = x;
	csr_y = y;
	type = -1;
	show_objecthighlight();
    } else if (shift) {		/* show selected object */
	show_objecthighlight();
    } else {			/* user selected an object */
	erase_objecthighlight();
	switch (type) {
	case O_POLYLINE:
	    manipulate(l, type, x, y, px, py);
	    break;
	case O_SPLINE:
	    manipulate(s, type, x, y, px, py);
	    break;
	case O_COMPOUND:
	    manipulate(c, type, x, y, px, py);
	    break;
	}
    }
}

void object_search_left(int x, int y, unsigned int shift)
       		         
                          	/* Shift Key Status from XEvent */
{
    manipulate = handlerproc_left;
    do_object_search(x, y, shift);
}

void object_search_middle(int x, int y, unsigned int shift)
       		         
                          	/* Shift Key Status from XEvent */
{
    manipulate = handlerproc_middle;
    do_object_search(x, y, shift);
}

void object_search_right(int x, int y, unsigned int shift)
       		         
                          	/* Shift Key Status from XEvent */
{
    manipulate = handlerproc_right;
    do_object_search(x, y, shift);
}

char
next_spline_point_found(double x, double y, int tol, F_point **p, F_point **q, unsigned int shift)
{
    if (!anyspline_in_mask())
	return (0);
    if (s == NULL)
	s = last_spline(objects.splines);
    else if (shift)
	s = prev_spline(objects.splines, s);

    for (; s != NULL; s = prev_spline(objects.splines, s))
	if (validspline_in_mask(s)) {
	    extern int validspline_in_mask (F_spline *s);
	    n++;
	    *p = NULL;
	    for (*q = s->points; *q != NULL; *p = *q, *q = (*q)->next) {
		if (fabs((*q)->x - x) <= tol && fabs((*q)->y - y) <= tol)
		    return (1);
	    }
	}
    return (0);
}

char
next_line_point_found(int x, int y, int tol, F_point **p, F_point **q, unsigned int shift)
{
    F_point	   *a, *b;

    if (!anyline_in_mask())
	return (0);
    if (l == NULL)
	l = last_line(objects.lines);
    else if (shift)
	l = prev_line(objects.lines, l);

    for (; l != NULL; l = prev_line(objects.lines, l))
	if (validline_in_mask(l)) {
	    extern int validline_in_mask (F_line *l);
	    n++;
	    for (a = NULL, b = l->points; b != NULL; a = b, b = b->next) {
		if (fabs(b->x - x) <= tol && fabs(b->y - y) <= tol) {
		    *p = a;
		    *q = b;
		    return (1);
		}
	    }
	}
    return (0);
}

void
init_searchproc_left(void (*handlerproc) (/* ??? */))
{
    handlerproc_left = handlerproc;
}

void
init_searchproc_middle(void (*handlerproc) (/* ??? */))
{
    handlerproc_middle = handlerproc;
}

void
do_point_search(int x, int y, unsigned int shift)
       		         
                          	/* Shift Key Status from XEvent */
{
    F_point	   *px, *py;
    char	    found = 0;

    px = &point1;
    py = &point2;
    init_search();
    for (n = 0; n < objectcount;) {
	switch (type) {
	case O_POLYLINE:
	    found = next_line_point_found(x, y, TOLERANCE, &px, &py, shift);
	    break;
	case O_SPLINE:
	    found = next_spline_point_found(x, y, TOLERANCE, &px, &py, shift);
	    break;
	}
	if (found) {
	    if (shift)
		show_objecthighlight();
	    break;
	}
	switch (type) {
	case O_POLYLINE:
	    type = O_SPLINE;
	    s = NULL;
	    break;
	case O_SPLINE:
	    type = O_POLYLINE;
	    l = NULL;
	    break;
	}
    }
    if (!found) {
	csr_x = x;
	csr_y = y;
	type = -1;
	show_objecthighlight();
    } else if (shift) {
	show_objecthighlight();
    } else {
	erase_objecthighlight();
	switch (type) {
	case O_POLYLINE:
	    manipulate(l, type, x, y, px, py);
	    break;
	case O_SPLINE:
	    manipulate(s, type, x, y, px, py);
	    break;
	}
    }
}

void point_search_left(int x, int y, unsigned int shift)
       		         
                          	/* Shift Key Status from XEvent */
{
    manipulate = handlerproc_left;
    do_point_search(x, y, shift);
}

void point_search_middle(int x, int y, unsigned int shift)
       		         
                          	/* Shift Key Status from XEvent */
{
    manipulate = handlerproc_middle;
    do_point_search(x, y, shift);
}
