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
#include "u_undo.h"
#include "w_cursor.h"
#include "w_setup.h"

extern int	num_object;


extern int read_fig(char *file_name, F_compound *obj);
extern int clean_up(void);
extern int update_cur_filename(char *newname);
extern void redisplay_canvas(void);
extern int put_msg(const char*, ...);
extern int set_action(int action);
extern int read_fail_message(char *file, int err);
extern int compound_bound(F_compound *compound, double *xmin, double *ymin, double *xmax, double *ymax);
extern int tail(F_compound *ob, F_compound *tails);
extern int append_objects(F_compound *l1, F_compound *l2, F_compound *tails);
extern int redisplay_zoomed_region(double xmin, double ymin, double xmax, double ymax);
extern int set_action_object(int action, int object);

int
load_file(char *file)
{
    int		    s;
    F_compound	    c;

    c.lines = NULL;
    c.splines = NULL;
    c.coasts = NULL;
    c.grids = NULL;
    c.next = NULL;
    set_temp_cursor(wait_cursor);
    s = read_fig(file, &c);
    if (s == 0) {		/* Successful read */
	clean_up();
	(void) strcpy(save_filename, cur_filename);
	update_cur_filename(file);
	saved_objects = objects;
	objects = c;
	redisplay_canvas();
	put_msg("Current figure \"%s\" (%d objects)", file, num_object);
	set_action(F_LOAD);
	reset_cursor();
	reset_modifiedflag();
	return (0);
    } else if (s == ENOENT) {
	clean_up();
	saved_objects = objects;
	objects = c;
	redisplay_canvas();
	put_msg("Current figure \"%s\" (new file)", file);
	(void) strcpy(save_filename, cur_filename);
	update_cur_filename(file);
	set_action(F_LOAD);
	reset_cursor();
	reset_modifiedflag();
	return (0);
    }
    read_fail_message(file, s);
    reset_modifiedflag();
    reset_cursor();
    return (1);
}

int
merge_file(char *file)
{
    F_compound	    c;
    int		    s;

    c.lines = NULL;
    c.splines = NULL;
    c.next = NULL;
    set_temp_cursor(wait_cursor);

    s = read_fig(file, &c);
    if (s == 0) {		/* Successful read */
	double		xmin, ymin, xmax, ymax;

	compound_bound(&c, &xmin, &ymin, &xmax, &ymax);
	clean_up();
	saved_objects = c;
	tail(&objects, &object_tails);
	append_objects(&objects, &saved_objects, &object_tails);
	redisplay_zoomed_region(xmin, ymin, xmax, ymax);
	put_msg("%d object(s) read from \"%s\"", num_object, file);
	set_action_object(F_ADD, O_ALL_OBJECT);
	reset_cursor();
	set_modifiedflag();
	return (0);
    }
    read_fail_message(file, s);
    reset_cursor();
    return (1);
}
