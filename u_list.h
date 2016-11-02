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

void		list_delete_line(F_line **line_list, F_line *line);
void		list_delete_spline(F_spline **spline_list, F_spline *spline);
void		list_delete_compound(F_compound **list, F_compound *compound);

void		list_add_line(F_line **line_list, F_line *l);
void		list_add_coast(F_coast **coast_list, F_coast *t);
void		list_add_grid(F_grid **grid_list, F_grid *g);
void		list_add_spline(F_spline **spline_list, F_spline *s);
void		list_add_compound(F_compound **list, F_compound *c);

F_line	       *last_line(F_line *list);
F_coast	       *last_coast(F_coast *list);
F_grid	       *last_grid(F_grid *list);
F_spline       *last_spline(F_spline *list);
F_compound     *last_compound(F_compound *list);
F_point	       *last_point(F_point *list);

F_line	       *prev_line(F_line *list, F_line *line);
F_spline       *prev_spline(F_spline *list, F_spline *spline);
F_compound     *prev_compound(F_compound *list, F_compound *compound);
F_point	       *prev_point(F_point *list, F_point *point);

void		delete_line(F_line *old_l);
void		delete_spline(F_spline *old_s);
void		delete_compound();

void		add_line(F_line *new_l);
void		add_coast(F_coast *new_t);
void		add_grid(F_grid *new_g);
void		add_spline(F_spline *new_s);

void		change_line(F_line *old_l, F_line *new_l);
void		change_spline(F_spline *old_s, F_spline *new_s);
