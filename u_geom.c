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

/*
 * Routines dealing with geometry under the following headings:
 *	COMPUTE NORMAL, CLOSE TO VECTOR,
 *	COMPUTE ANGLE, COMPUTE DIRECTION.
 */

#include "fig.h"
#include "resources.h"
#include "object.h"

/*************************** COMPUTE NORMAL **********************

Input arguments :
	(x1,y1)(x2,y2) : the vector
	direction : direction of the normal vector to (x1,y1)(x2,y2)
Output arguments :
	(*x,*y)(x2,y2) : a normal vector.
Return value : none

******************************************************************/

void compute_normal(float x1, float y1, int x2, int y2, int direction, int *x, int *y)
{
    if (direction) {		/* counter clockwise  */
	*x = round(x2 - (y2 - y1));
	*y = round(y2 - (x1 - x2));
    } else {
	*x = round(x2 + (y2 - y1));
	*y = round(y2 + (x1 - x2));
    }
}

/******************** CLOSE TO VECTOR **************************

Input arguments:
	(x1,y1)(x2,y2) : the vector
	(xp,yp) : the point
	d : tolerance (max. allowable distance from the point to the vector)
	dd : d * d
Output arguments:
	(*px,*py) : a point on the vector which is not far from (xp,yp)
		by more than d. Normally the vector (*px,*py)(xp,yp)
		is normal to vector (x1,y1)(x2,y2) except when (xp,yp)
		is within d from (x1,y1) or (x2,y2), in which cases,
		(*px,*py) = (x1,y1) or (x2,y2) respectively.
Return value :
	0 : No point on the vector is within d from (xp, yp)
	1 : (*px, *py) is such a point.

******************************************************************/

int close_to_vector(double x1, double y1, double x2, double y2, double xp, double yp, int d, float dd, int *px, int *py)
{
    int		    xmin, ymin, xmax, ymax;
    float	    x, y, slope, D2, dx, dy;

    if (abs(xp - x1) <= d && abs(yp - y1) <= d) {
	*px = x1;
	*py = y1;
	return (1);
    }
    if (abs(xp - x2) <= d && abs(yp - y2) <= d) {
	*px = x2;
	*py = y2;
	return (1);
    }
    if (x1 < x2) {
	xmin = x1 - d;
	xmax = x2 + d;
    } else {
	xmin = x2 - d;
	xmax = x1 + d;
    }
    if (xp < xmin || xmax < xp)
	return (0);

    if (y1 < y2) {
	ymin = y1 - d;
	ymax = y2 + d;
    } else {
	ymin = y2 - d;
	ymax = y1 + d;
    }
    if (yp < ymin || ymax < yp)
	return (0);

    if (x2 == x1) {
	x = x1;
	y = yp;
    } else if (y1 == y2) {
	x = xp;
	y = y1;
    } else {
	slope = ((float) (x2 - x1)) / ((float) (y2 - y1));
	y = (slope * (xp - x1 + slope * y1) + yp) / (1 + slope * slope);
	x = ((float) x1) + slope * (y - y1);
    }
    dx = ((float) xp) - x;
    dy = ((float) yp) - y;
    D2 = dx * dx + dy * dy;
    if (D2 < dd) {
	*px = (int) (x + .5);
	*py = (int) (y + .5);
	return (1);
    }
    return (0);
}
