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

static void	create_intsplineobject(int x, int y);
static void	init_intspline_drawing(int x, int y);


extern void init_trace_drawing(double x, double y);
extern int get_intermediatepoint(int x, int y);
void make_control_points(F_spline *s);
extern int draw_intspline(F_spline *s, int op);
extern int spline_bound(F_spline *s, double *xmin, double *ymin, double *xmax, double *ymax);
void remake_control_points(F_spline *s);
void compute_cp(F_point *points, F_control *controls, int path);
void control_points(float x, float y, float l1, float l2, float theta1, float theta2, float t, F_control *cp);

void intspline_drawing_selected(void)
{
    set_mousefun("first point", "", "");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_leftbut_proc = init_intspline_drawing;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    set_cursor(arrow_cursor);
    reset_action_on();
}

static
void init_intspline_drawing(int x, int y)
{
    min_num_points = 3;
    init_trace_drawing((double)x, (double)y);
    canvas_middlebut_save = create_intsplineobject;
    return_proc = intspline_drawing_selected;
}

static
void create_intsplineobject(int x, int y)
{
    F_spline	   *spline;

    if ((double)x != fix_x || (double)y != fix_y ||
		num_point < min_num_points) {
	get_intermediatepoint(x, y);
    }
    elastic_line();
    if ((spline = create_spline()) == NULL) {
	if (num_point == 1) {
	    free((char *) cur_point);
	    cur_point = NULL;
	}
	free((char *) first_point);
	first_point = NULL;
	return;
    }
    spline->style = cur_linestyle;
    spline->thickness = cur_linewidth;
    spline->style_val = cur_styleval * (cur_linewidth + 1) / 2;
    spline->color = cur_color;
    spline->points = first_point;
    spline->controls = NULL;
    spline->next = NULL;
    cur_x = cur_y = fix_x = fix_y = 0;	/* used in elastic_moveline */
    elastic_moveline(spline->points);	/* erase control vector */
    spline->type = T_OPEN_INTERP;
    make_control_points(spline);
    draw_intspline(spline, PAINT);
    if (appres.DEBUG) {
	double		xmin, ymin, xmax, ymax;

	spline_bound(spline, &xmin, &ymin, &xmax, &ymax);
	elastic_box((int)xmin, (int)ymin, (int)xmax, (int)ymax);
    }
    add_spline(spline);
    intspline_drawing_selected();
    draw_mousefun_canvas();
}

/* Tension : 0 (min) -> 1 (max)	 */

int create_control_list(F_spline *s)
{
    F_point	   *p;
    F_control	   *cp;

    if ((cp = create_cpoint()) == NULL)
	return (-1);

    s->controls = cp;
    for (p = s->points->next; p != NULL; p = p->next) {
	if ((cp->next = create_cpoint()) == NULL)
	    return (-1);
	cp = cp->next;
    }
    cp->next = NULL;
    return (1);
}

void make_control_points(F_spline *s)
{
    if (-1 == create_control_list(s))
	return;

    remake_control_points(s);
}

void remake_control_points(F_spline *s)
{
    compute_cp(s->points, s->controls, OPEN_PATH);
}

#define		T		0.45
#define		_2xPI		6.2832
#define		_1dSQR2		0.7071
#define		_SQR2		1.4142

void compute_cp(F_point *points, F_control *controls, int path)
{
    F_control	   *cp, *cpn;
    F_point	   *p, *p2, *pk;/* Pk is the next-to-last point. */
    float	    dx, dy;
    float	    x1, y1, x2, y2, x3, y3;
    float	    l1, l2, theta1, theta2;

    x1 = points->x;
    y1 = points->y;
    pk = p2 = points->next;
    x2 = p2->x;
    y2 = p2->y;
    p = p2->next;
    x3 = p->x;
    y3 = p->y;

    dx = x1 - x2;
    dy = y2 - y1;
    l1 = sqrt((double) (dx * dx + dy * dy));
    if (l1 == 0.0)
	theta1 = 0.0;
    else
	theta1 = atan2((double) dy, (double) dx);
    dx = x3 - x2;
    dy = y2 - y3;
    l2 = sqrt((double) (dx * dx + dy * dy));
    if (l2 == 0.0)
	theta2 = 0.0;
    else
	theta2 = atan2((double) dy, (double) dx);
    /* -PI <= theta1, theta2 <= PI */
    if (theta1 < 0)
	theta1 += _2xPI;
    if (theta2 < 0)
	theta2 += _2xPI;
    /* 0 <= theta1, theta2 < 2PI */

    cp = controls->next;
    control_points(x2, y2, l1, l2, theta1, theta2, T, cp);
    /* control points for (x2, y2) */
    if (path == OPEN_PATH) {
	controls->lx = 0.0;
	controls->ly = 0.0;
	controls->rx = (x1 + 3 * cp->lx) / 4;
	controls->ry = (y1 + 3 * cp->ly) / 4;
	cp->lx = (3 * cp->lx + x2) / 4;
	cp->ly = (3 * cp->ly + y2) / 4;
    }
    while (1) {
	x2 = x3;
	y2 = y3;
	l1 = l2;
	if (theta2 >= M_PI)
	    theta1 = theta2 - M_PI;
	else
	    theta1 = theta2 + M_PI;
	if ((p = p->next) == NULL)
	    break;
	pk = pk->next;
	x3 = p->x;
	y3 = p->y;
	dx = x3 - x2;
	dy = y2 - y3;
	l2 = sqrt((double) (dx * dx + dy * dy));
	if (l2 == 0.0)
	    theta2 = 0.0;
	else
	    theta2 = atan2((double) dy, (double) dx);
	if (theta2 < 0)
	    theta2 += _2xPI;
	cp = cp->next;
	control_points(x2, y2, l1, l2, theta1, theta2, T, cp);
    };

    cpn = cp->next;
    cpn->lx = (3 * cp->rx + x2) / 4;
    cpn->ly = (3 * cp->ry + y2) / 4;
    cpn->rx = 0.0;
    cpn->ry = 0.0;
    cp->rx = (pk->x + 3 * cp->rx) / 4;
    cp->ry = (pk->y + 3 * cp->ry) / 4;
}

/*
 * The parameter t is the tension.  It must range in [0, 1]. The bigger the
 * value of t, the higher the tension.
 */

void control_points(float x, float y, float l1, float l2, float theta1, float theta2, float t, F_control *cp)
{
    float	    s, theta, r = 1 - t;

    /* 0 <= theta1, theta2 < 2PI */

    theta = (theta1 + theta2) / 2;

    if (theta1 > theta2) {
	s = sin((double) (theta - theta2));
	theta1 = theta + M_PI_2;
	theta2 = theta - M_PI_2;
    } else {
	s = sin((double) (theta2 - theta));
	theta1 = theta - M_PI_2;
	theta2 = theta + M_PI_2;
    }
    if (s > _1dSQR2)
	s = _SQR2 - s;
    s *= r;
    l1 *= s;
    l2 *= s;
    cp->lx = x + l1 * cos((double) theta1);
    cp->ly = y - l1 * sin((double) theta1);
    cp->rx = x + l2 * cos((double) theta2);
    cp->ry = y - l2 * sin((double) theta2);
}
