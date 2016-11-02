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

extern F_line  *create_line(void);
extern F_coast *create_coast(void);
extern F_grid *create_grid(void);
extern F_spline *create_spline(void);
extern F_compound *create_compound();
extern F_point *create_point(void);
extern F_control *create_cpoint(void);

extern F_line  *copy_line(F_line *l);
extern F_spline *copy_spline(F_spline *s);

extern F_point *copy_points(F_point *orig_pt);
