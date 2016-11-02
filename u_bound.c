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
#include "u_bound.h"

#define		Ninety_deg		M_PI_2
#define		One_eighty_deg		M_PI
#define		Two_seventy_deg		(M_PI + M_PI_2)
#define		Three_sixty_deg		(M_PI + M_PI)
#define		half(z1 ,z2)		((z1+z2)/2.0)

/* macro which rounds DOWN the coordinates depending on point positioning mode */
#define		floor_coords(x) \

/* macro which rounds UP the coordinates depending on point positioning mode */
#define		ceil_coords(x) \

static void	points_bound(F_point *points, int half_wd, double *xmin, double *ymin, double *xmax, double *ymax);
static void	int_spline_bound(F_spline *s, double *xmin, double *ymin, double *xmax, double *ymax);


void line_bound(F_line *l, double *xmin, double *ymin, double *xmax, double *ymax);
void spline_bound(F_spline *s, double *xmin, double *ymin, double *xmax, double *ymax);

void compound_bound(F_compound *compound, double *xmin, double *ymin, double *xmax, double *ymax)
{
    F_spline	   *s;
    F_line	   *l;
    int		    first = 1;
    double	    bx, by, sx, sy;
    double	    llx, lly, urx, ury;

    for (l = compound->lines; l != NULL; l = l->next) {
	line_bound(l, &sx, &sy, &bx, &by);
	if (first) {
	    first = 0;
	    llx = sx;
	    lly = sy;
	    urx = bx;
	    ury = by;
	} else {
	    llx = min2(llx, sx);
	    lly = min2(lly, sy);
	    urx = max2(urx, bx);
	    ury = max2(ury, by);
	}
    }

    for (s = compound->splines; s != NULL; s = s->next) {
	spline_bound(s, &sx, &sy, &bx, &by);
	if (first) {
	    first = 0;
	    llx = sx;
	    lly = sy;
	    urx = bx;
	    ury = by;
	} else {
	    llx = min2(llx, sx);
	    lly = min2(lly, sy);
	    urx = max2(urx, bx);
	    ury = max2(ury, by);
	}
    }

    /* round the corners to the current positioning grid */
    floor_coords(llx);
    floor_coords(lly);
    ceil_coords(urx);
    ceil_coords(ury);
    *xmin = llx;
    *ymin = lly;
    *xmax = urx;
    *ymax = ury;
}

void line_bound(F_line *l, double *xmin, double *ymin, double *xmax, double *ymax)
{
    points_bound(l->points, (l->thickness / 2), xmin, ymin, xmax, ymax);
}

void spline_bound(F_spline *s, double *xmin, double *ymin, double *xmax, double *ymax)
{
    int_spline_bound(s, xmin, ymin, xmax, ymax);
}

static void
int_spline_bound(F_spline *s, double *xmin, double *ymin, double *xmax, double *ymax)
{
    F_point	   *p1, *p2;
    F_control	   *cp1, *cp2;
    float	    x0, y0, x1, y1, x2, y2, x3, y3, sx1, sy1, sx2, sy2;
    float	    tx, ty, tx1, ty1, tx2, ty2;
    float	    sx, sy, bx, by;
    int		    half_wd;

    half_wd = s->thickness / 2;
    p1 = s->points;
    sx = bx = p1->x;
    sy = by = p1->y;
    cp1 = s->controls;
    for (p2 = p1->next, cp2 = cp1->next; p2 != NULL;
	 p1 = p2, cp1 = cp2, p2 = p2->next, cp2 = cp2->next) {
	x0 = p1->x;
	y0 = p1->y;
	x1 = cp1->rx;
	y1 = cp1->ry;
	x2 = cp2->lx;
	y2 = cp2->ly;
	x3 = p2->x;
	y3 = p2->y;
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

	sx = min2(x0, sx);
	sy = min2(y0, sy);
	sx = min2(sx1, sx);
	sy = min2(sy1, sy);
	sx = min2(sx2, sx);
	sy = min2(sy2, sy);
	sx = min2(tx1, sx);
	sy = min2(ty1, sy);
	sx = min2(tx2, sx);
	sy = min2(ty2, sy);
	sx = min2(x3, sx);
	sy = min2(y3, sy);

	bx = max2(x0, bx);
	by = max2(y0, by);
	bx = max2(sx1, bx);
	by = max2(sy1, by);
	bx = max2(sx2, bx);
	by = max2(sy2, by);
	bx = max2(tx1, bx);
	by = max2(ty1, by);
	bx = max2(tx2, bx);
	by = max2(ty2, by);
	bx = max2(x3, bx);
	by = max2(y3, by);
    }
    *xmin = round(sx) - half_wd;
    *ymin = round(sy) - half_wd;
    *xmax = round(bx) + half_wd;
    *ymax = round(by) + half_wd;
}

static void
points_bound(F_point *points, int half_wd, double *xmin, double *ymin, double *xmax, double *ymax)
{
    double	    bx, by, sx, sy;
    F_point	   *p;

    bx = sx = points->x;
    by = sy = points->y;
    for (p = points->next; p != NULL; p = p->next) {
	sx = min2(sx, p->x);
	sy = min2(sy, p->y);
	bx = max2(bx, p->x);
	by = max2(by, p->y);
    }
    half_wd *= ZOOM_FACTOR;
    *xmin = sx - half_wd;
    *ymin = sy - half_wd;
    *xmax = bx + half_wd;
    *ymax = by + half_wd;
}

int
overlapping(double xmin1, double ymin1, double xmax1, double ymax1, double xmin2, double ymin2, double xmax2, double ymax2)
{
    if (xmin1 < xmin2)
	if (ymin1 < ymin2)
	    return (int)(xmax1 >= xmin2 && ymax1 >= ymin2);
	else
	    return (int)(xmax1 >= xmin2 && ymin1 <= ymax2);
    else if (ymin1 < ymin2)
	return (int)(xmin1 <= xmax2 && ymax1 >= ymin2);
    else
	return (int)(xmin1 <= xmax2 && ymin1 <= ymax2);
}
