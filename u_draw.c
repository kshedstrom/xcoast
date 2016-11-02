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
#include "paintop.h"
#include "u_bound.h"
#include "u_create.h"
#include "w_canvas.h"
#include "w_drawprim.h"
#include "w_zoom.h"

typedef unsigned char byte;

/************** POLYGON/CURVE DRAWING FACILITIES ****************/

static int	npoints;
static int	max_points;
static XPoint  *points;
static int	allocstep;


extern int put_msg(const char*, ...);
extern int fprintf(FILE *, const char *, ...);
extern int pw_lines(Window w, XPoint *points, int npoints, int op, int line_width, int line_style, float style_val, int color);
extern int line_bound(F_line *l, double *xmin, double *ymin, double *xmax, double *ymax);
extern int pw_point(Window w, double x, double y, int line_width, int op, int color);
extern int spline_bound(F_spline *s, double *xmin, double *ymin, double *xmax, double *ymax);
void draw_intspline(F_spline *s, int op);
void bezier_spline(float a0, float b0, float a1, float b1, float a2, float b2, float a3, float b3);
void clear_stack(void);
void push(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
int pop (float *x1, float *y1, float *x2, float *y2, float *x3, float *y3, float *x4, float *y4);

static		Boolean
init_point_array(int init_size, int step_size)
{
    npoints = 0;
    max_points = init_size;
    allocstep = step_size;
    if (max_points > MAXNUMPTS) {
	put_msg("Too many points, recompile with MAXNUMPTS > %d in w_drawprim.h",
		MAXNUMPTS);
	max_points = MAXNUMPTS;
    }
    if ((points = (XPoint *) malloc(max_points * sizeof(XPoint))) == 0) {
	fprintf(stderr, "xcoast: insufficient memory to allocate point array\n");
	return False;
    }
    return True;
}

static	void
add_point(double x, double y)
{
    if (npoints >= max_points) {
	XPoint	       *tmp_p;

	max_points += allocstep;
	if (max_points >= MAXNUMPTS)
	    return;	/* stop; it is not closing */

	if ((tmp_p = (XPoint *) realloc(points,
					max_points * sizeof(XPoint))) == 0) {
	    fprintf(stderr,
		    "xcoast: insufficient memory to reallocate point array\n");
	    return;
	}
	points = tmp_p;
    }
    points[npoints].x = (short) ZOOMX(x);
    points[npoints].y = (short) ZOOMY(y);
    npoints++;
}

static void
draw_point_array(Window w, int op, int line_width, int line_style, float style_val, int color)
{
    pw_lines(w, points, npoints, op,
	     line_width, line_style, style_val, color);
    free(points);
}

/*********************** LINE ***************************/

void draw_line(F_line *line, int op)
{
    F_point	   *point;
    double	    x, y;
    double	    xmin, ymin, xmax, ymax;

    line_bound(line, &xmin, &ymin, &xmax, &ymax);
    if (!overlapping(ZOOMX(xmin), ZOOMY(ymin), ZOOMX(xmax), ZOOMY(ymax),
		     clip_xmin, clip_ymin, clip_xmax, clip_ymax))
	return;

    /* get first point and coordinates */
    point = line->points;
    x = point->x;
    y = point->y;

    /* is it a single point? */
    if (line->points->next == NULL) {
	pw_point(canvas_win, x, y, line->thickness, op, line->color);
	return;
    }

    /* accumulate the points in an array - start with 50 */
    if (!init_point_array(50, 50))
	return;

    for (point = line->points; point != NULL; point = point->next) {
	x = point->x;
	y = point->y;
	add_point(x, y);
    }

    draw_point_array(canvas_win, op, line->thickness, line->style,
		     line->style_val, line->color);
}

/********************** COASTLINE **************************/

void draw_coastline(F_coast *coast, int op)
{
    F_point         *point;
    double          x, y;

    /* draw outline */

    /* get first point and coordinates */
    point = coast->points;
    x = point->x;
    y = point->y;

    /* is it a single point? */
    if (coast->points->next == NULL) {
	pw_point(canvas_win, x, y, 1, op, BLACK);
	return;
    }

    /* accumulate the points in an array - start with 50 */
    if (!init_point_array(50, 50))
	return;

    for (point = coast->points; point != NULL; point = point->next) {
	x = point->x;
	y = point->y;
	add_point(x, y);
    }

    draw_point_array(canvas_win, op, 1, SOLID_LINE,
		     0.0, BLACK);
}

/********************** GRIDLINES **************************/

void draw_grids(F_grid *grid, int op)
{
    F_point         *point;
    double          x, y;

    /* draw outline */

    /* get first point and coordinates */
    point = grid->points;
    x = point->x;
    y = point->y;

    /* is it a single point? */
    if (grid->points->next == NULL) {
	pw_point(canvas_win, x, y, 1, op, BLACK);
	return;
    }

    /* accumulate the points in an array - start with 50 */
    if (!init_point_array(50, 50))
	return;

    for (point = grid->points; point != NULL; point = point->next) {
	x = point->x;
	y = point->y;
	add_point(x, y);
    }

    draw_point_array(canvas_win, op, 1, DOTTED_LINE,
		     4.0, BLACK);
}

/*********************** SPLINE ***************************/

void draw_spline(F_spline *spline, int op)
{
    double	    xmin, ymin, xmax, ymax;

    spline_bound(spline, &xmin, &ymin, &xmax, &ymax);
    if (!overlapping(ZOOMX(xmin), ZOOMY(ymin), ZOOMX(xmax), ZOOMY(ymax),
		     clip_xmin, clip_ymin, clip_xmax, clip_ymax))
	return;

    if (int_spline(spline))
	draw_intspline(spline, op);
}

void draw_intspline(F_spline *s, int op)
{
    F_point	   *p1, *p2;
    F_control	   *cp1, *cp2;

    p1 = s->points;
    cp1 = s->controls;
    cp2 = cp1->next;

    if (!init_point_array(300, 200))
	return;

    for (p2 = p1->next, cp2 = cp1->next; p2 != NULL;
	 p1 = p2, cp1 = cp2, p2 = p2->next, cp2 = cp2->next) {
	bezier_spline((float) p1->x, (float) p1->y, cp1->rx, cp1->ry,
		      cp2->lx, cp2->ly, (float) p2->x, (float) p2->y);
    }

    add_point(p1->x, p1->y);

    draw_point_array(canvas_win, op, s->thickness, s->style,
		     s->style_val, s->color);
}

/********************* CURVES FOR SPLINES *****************************

	The following spline drawing routine is from

	"An Algorithm for High-Speed Curve Generation"
	by George Merrill Chaikin,
	Computer Graphics and Image Processing, 3, Academic Press,
	1974, 346-349.

	and

	"On Chaikin's Algorithm" by R. F. Riesenfeld,
	Computer Graphics and Image Processing, 4, Academic Press,
	1975, 304-310.

***********************************************************************/

#define		half(z1, z2)	((z1+z2)/2.0)
#define		THRESHOLD	5

/*
 * the style parameter doesn't work for splines because we use small line
 * segments
 */

void bezier_spline(float a0, float b0, float a1, float b1, float a2, float b2, float a3, float b3)
{
    register float  tx, ty;
    float	    x0, y0, x1, y1, x2, y2, x3, y3;
    float	    sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, xmid, ymid;

    clear_stack();
    push(a0, b0, a1, b1, a2, b2, a3, b3);

    while (pop(&x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3)) {
	int pop (float *x1, float *y1, float *x2, float *y2, float *x3, float *y3, float *x4, float *y4);
	if (fabs(x0 - x3) < THRESHOLD && fabs(y0 - y3) < THRESHOLD) {
	    add_point((double)x0, (double)y0);
	} else {
	    tx = half(x1, x2);
	    ty = half(y1, y2);
	    sx1 = half(x0, x1);
	    sy1 = half(y0, y1);
	    sx2 = half(sx1, tx);
	    sy2 = half(sy1, ty);
	    tx2 = half(x2, x3);
	    ty2 = half(y2, y3);
	    tx1 = half(tx2, tx);
	    ty1 = half(ty2, ty);
	    xmid = half(sx2, tx1);
	    ymid = half(sy2, ty1);

	    push(xmid, ymid, tx1, ty1, tx2, ty2, x3, y3);
	    push(x0, y0, sx1, sy1, sx2, sy2, xmid, ymid);
	}
    }
}

/* utilities used by spline drawing routines */

#define		STACK_DEPTH		20

typedef struct stack {
    float	    x1, y1, x2, y2, x3, y3, x4, y4;
}
		Stack;

static Stack	stack[STACK_DEPTH];
static Stack   *stack_top;
static int	stack_count;

void clear_stack(void)
{
    stack_top = stack;
    stack_count = 0;
}

void push(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    stack_top->x1 = x1;
    stack_top->y1 = y1;
    stack_top->x2 = x2;
    stack_top->y2 = y2;
    stack_top->x3 = x3;
    stack_top->y3 = y3;
    stack_top->x4 = x4;
    stack_top->y4 = y4;
    stack_top++;
    stack_count++;
}

int
pop(float *x1, float *y1, float *x2, float *y2, float *x3, float *y3, float *x4, float *y4)
{
    if (stack_count == 0)
	return (0);
    stack_top--;
    stack_count--;
    *x1 = stack_top->x1;
    *y1 = stack_top->y1;
    *x2 = stack_top->x2;
    *y2 = stack_top->y2;
    *x3 = stack_top->x3;
    *y3 = stack_top->y3;
    *x4 = stack_top->x4;
    *y4 = stack_top->y4;
    return (1);
}
