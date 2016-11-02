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

#define		MOVE_ARB	0
#define		MOVE_HORIZ_VERT 1

#define		MSG_DIAM	1
#define		MSG_LENGTH	2

extern int	constrained;
extern double	fix_x, fix_y;
extern double	x1off, x2off, y1off, y2off;
extern double	from_x, from_y;
extern F_point *left_point, *right_point;

extern	void 	elastic_box(int x1, int y1, int x2, int y2);
extern	void 	elastic_movebox(void);
extern	void 	moving_box(double x, double y);

extern	void 	freehand_line(int x, int y);
extern	void 	elastic_moveline(F_point *pts);
extern	void 	elastic_line(void);
extern	void 	reshaping_line(int x, int y);
extern	void 	elastic_linelink(void);

extern	void 	adjust_pos(double curs_x, double curs_y, double orig_x, double orig_y, double *ret_x, double *ret_y);
