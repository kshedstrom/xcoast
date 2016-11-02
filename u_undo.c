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

/**************** IMPORTS ****************/

#include "fig.h"
#include "resources.h"
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "u_elastic.h"
#include "u_list.h"
#include "u_undo.h"
#include "w_setup.h"

/*************** EXPORTS *****************/

/*
 * Object_tails *usually* points to the last object in each linked list in
 * objects.  The exceptions occur when multiple objects are added to a figure
 * (e.g. file read, break compound, undo delete region).  In these cases,
 * the added objects are appended to the object lists (and saved_objects is
 * set up to point to the new objects) but object_tails is not changed.
 * This speeds up a subsequent undo operation which need only set
 * all the "next" fields of objects pointed to by object_tails to NULL.
 */

F_compound	saved_objects = {0, { 0, 0}, { 0, 0}, NULL, NULL, NULL, NULL};
F_compound	object_tails = {0, { 0, 0}, { 0, 0}, NULL, NULL, NULL, NULL};

/*************** LOCAL *****************/

static int	last_object;
static int	last_action = F_NULL;
static F_point *last_prev_point, *last_selected_point, *last_next_point;


void undo_add(void);
void undo_delete(void);
void undo_change(void);
void undo_load(void);
void undo_addpoint(void);
void undo_deletepoint(void);
void undo_convert(void);
extern int put_msg(const char*, ...);
extern int linepoint_deleting(F_line *line, F_point *prev_point, F_point *selected_point);
extern int splinepoint_deleting(F_spline *spline, F_point *prev_point, F_point *selected_point);
extern int linepoint_adding(F_line *line, F_point *left_point, F_point *added_point);
extern int splinepoint_adding(F_spline *spline, F_point *left_point, F_point *added_point, F_point *right_point);
extern int spline_2_line(F_spline *s);
extern int line_2_spline(F_line *l);
extern void redisplay_lines(F_line *l1, F_line *l2);
extern void redisplay_splines(F_spline *s1, F_spline *s2);
void set_action_object(int action, int object);
extern void redisplay_zoomed_region(double xmin, double ymin, double xmax, double ymax);
extern void redisplay_line(F_line *l);
extern void redisplay_spline(F_spline *s);
extern void redisplay_compound(F_compound *c);
extern int cut_objects(F_compound *objects, F_compound *tails);
extern int compound_bound(F_compound *compound, double *xmin, double *ymin, double *xmax, double *ymax);
extern int append_objects(F_compound *l1, F_compound *l2, F_compound *tails);
extern int update_cur_filename(char *newname);
extern void redisplay_canvas(void);
extern void free_compound(F_compound **list);
extern void free_line(F_line **list);
extern void free_spline(F_spline **list);
extern void free_coast(F_coast **list);
extern void free_grid(F_grid **list);

void
undo(void)
{
    switch (last_action) {
	case F_ADD:
	undo_add();
	break;
    case F_DELETE:
	undo_delete();
	break;
    case F_CHANGE:
	undo_change();
	break;
    case F_LOAD:
	undo_load();
	break;
    case F_ADD_POINT:
	undo_addpoint();
	break;
    case F_DELETE_POINT:
	undo_deletepoint();
	break;
    case F_CONVERT:
	undo_convert();
	break;
    default:
	put_msg("Nothing to UNDO");
	return;
    }
    put_msg("Undo complete");
}

void undo_addpoint(void)
{
    if (last_object == O_POLYLINE)
	linepoint_deleting(saved_objects.lines, last_prev_point,
			   last_selected_point);
    else
	splinepoint_deleting(saved_objects.splines, last_prev_point,
			     last_selected_point);
}

void undo_deletepoint(void)
{
    last_action = F_NULL;	/* to avoid doing a clean-up during adding */

    if (last_object == O_POLYLINE)
	linepoint_adding(saved_objects.lines, last_prev_point,
			 last_selected_point);
    else
	splinepoint_adding(saved_objects.splines, last_prev_point,
			   last_selected_point, last_next_point);
    last_next_point = NULL;
}

void undo_convert(void)
{
    switch (last_object) {
    case O_POLYLINE:
	spline_2_line(saved_objects.splines);
	break;
    case O_SPLINE:
	line_2_spline(saved_objects.lines);
	break;
    }
}

/*
 * saved_objects.xxxx contains a pointer to the original object,
 * saved_objects.xxxx->next points to the changed object.
 */

void undo_change(void)
{
    F_compound	    swp_comp;

    last_action = F_NULL;	/* to avoid a clean-up during "unchange" */
    switch (last_object) {
    case O_POLYLINE:
	new_l = saved_objects.lines;	/* the original */
	old_l = saved_objects.lines->next;	/* the changed object */
	change_line(old_l, new_l);
	redisplay_lines(new_l, old_l);
	break;
    case O_SPLINE:
	new_s = saved_objects.splines;
	old_s = saved_objects.splines->next;
	change_spline(old_s, new_s);
	redisplay_splines(new_s, old_s);
	break;
    case O_ALL_OBJECT:
	swp_comp = objects;
	objects = saved_objects;
	saved_objects = swp_comp;
	new_c = &objects;
	old_c = &saved_objects;
	set_action_object(F_CHANGE, O_ALL_OBJECT);
	set_modifiedflag();
	redisplay_zoomed_region(0, 0, CANVAS_WD, CANVAS_HT);
	break;
    }
}

/*
 * When a single object is created, it is appended to the appropriate list
 * in objects.	It is also placed in the appropriate list in saved_objects.
 *
 * When a number of objects are created (usually by reading them in from
 * a file or undoing a remove-all action), they are appended to the lists in
 * objects and also saved in saved_objects.  The pointers in object_tails
 * will be set to point to the last members of the lists in objects prior to
 * the appending.
 *
 * Note: The read operation will set the pointers in object_tails while the
 * remove-all operation will zero pointers in objects.
 */

void undo_add(void)
{
    double	    xmin, ymin, xmax, ymax;

    switch (last_object) {
    case O_POLYLINE:
	list_delete_line(&objects.lines, saved_objects.lines);
	redisplay_line(saved_objects.lines);
	break;
    case O_SPLINE:
	list_delete_spline(&objects.splines, saved_objects.splines);
	redisplay_spline(saved_objects.splines);
	break;
    case O_ALL_OBJECT:
	cut_objects(&objects, &object_tails);
	compound_bound(&saved_objects, &xmin, &ymin, &xmax, &ymax);
	redisplay_zoomed_region(xmin, ymin, xmax, ymax);
	break;
    }
    last_action = F_DELETE;
}

void undo_delete(void)
{
    switch (last_object) {
    case O_POLYLINE:
	list_add_line(&objects.lines, saved_objects.lines);
	redisplay_line(saved_objects.lines);
	break;
    case O_SPLINE:
	list_add_spline(&objects.splines, saved_objects.splines);
	redisplay_spline(saved_objects.splines);
	break;
    case O_ALL_OBJECT:
	append_objects(&objects, &saved_objects, &object_tails);
	redisplay_canvas();
    }
    last_action = F_ADD;
}

void undo_load(void)
{
    F_compound	    temp;
    char	    ctemp[128];

    temp = objects;
    objects = saved_objects;
    saved_objects = temp;
    strcpy(ctemp, cur_filename);
    update_cur_filename(save_filename);
    strcpy(save_filename, ctemp);
    redisplay_canvas();
    set_modifiedflag();
    last_action = F_LOAD;
}

/*
 * Clean_up should be called before committing a user's request. Clean_up
 * will attempt to free all the allocated memories which resulted from
 * delete/remove action.  It will set the last_action to F_NULL.  Thus this
 * routine should be before set_action_object(). if they are to be called in
 * the same routine.
 */
void clean_up(void)
{
    if (last_action == F_CHANGE) {
	switch (last_object) {
	case O_POLYLINE:
	    saved_objects.lines->next = NULL;
	    free_line(&saved_objects.lines);
	    break;
	case O_SPLINE:
	    saved_objects.splines->next = NULL;
	    free_spline(&saved_objects.splines);
	    break;
	}
    } else if (last_action == F_DELETE) {
	switch (last_object) {
	case O_POLYLINE:
	    free_line(&saved_objects.lines);
	    break;
	case O_SPLINE:
	    free_spline(&saved_objects.splines);
	    break;
	case O_ALL_OBJECT:
	    free_line(&saved_objects.lines);
	    free_spline(&saved_objects.splines);
	    free_coast(&saved_objects.coasts);
	    free_grid(&saved_objects.grids);
	    break;
	}
    } else if (last_action == F_DELETE_POINT) {
	free((char *) last_selected_point);
	last_prev_point = NULL;
	last_selected_point = NULL;
	last_next_point = NULL;
    } else if (last_action == F_ADD_POINT) {
	last_prev_point = NULL;
	last_selected_point = NULL;
    } else if (last_action == F_LOAD) {
	free_line(&saved_objects.lines);
	free_spline(&saved_objects.splines);
	free_coast(&saved_objects.coasts);
	free_grid(&saved_objects.grids);
    } else if (last_action == F_ADD) {
	saved_objects.lines = NULL;
	saved_objects.splines = NULL;
	saved_objects.coasts = NULL;
	saved_objects.grids = NULL;
    } else if (last_action == F_CONVERT) {
	saved_objects.splines = NULL;
	saved_objects.lines = NULL;
    }
    last_action = F_NULL;
}

void set_latestobjects(F_compound *objects)
{
    saved_objects = *objects;
}

void set_latestline(F_line *line)
{
    saved_objects.lines = line;
}

void set_latestspline(F_spline *spline)
{
    saved_objects.splines = spline;
}

void set_last_prevpoint(F_point *prev_point)
{
    last_prev_point = prev_point;
}

void set_last_selectedpoint(F_point *selected_point)
{
    last_selected_point = selected_point;
}

void set_last_nextpoint(F_point *next_point)
{
    last_next_point = next_point;
}

void set_action(int action)
{
    last_action = action;
}

void set_action_object(int action, int object)
{
    last_action = action;
    last_object = object;
}
