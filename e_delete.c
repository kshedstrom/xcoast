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
#include "u_search.h"
#include "u_list.h"
#include "u_undo.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_mousefun.h"
#include "w_setup.h"

static void	init_delete(char *p, int type, int x, int y, int px, int py);


extern void redisplay_line (F_line *l);
extern void redisplay_spline (F_spline *s);
extern void clean_up (void);
extern void set_action_object (int action, int object);
extern void set_latestobjects (F_compound *objects);

void delete_selected(void)
{
    set_mousefun("delete object", "", "del to cut buf");
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    init_searchproc_left(init_delete);
    canvas_leftbut_proc = object_search_left;
    canvas_rightbut_proc = object_search_right;
    canvas_middlebut_proc = null_proc;
    set_cursor(buster_cursor);
    reset_action_on();
}

/* ARGSUSED */
static
void init_delete(char *p, int type, int x, int y, int px, int py)
{
    switch (type) {
    case O_POLYLINE:
	cur_l = (F_line *) p;
	delete_line(cur_l);
	(void)redisplay_line(cur_l);
	break;
    case O_SPLINE:
	cur_s = (F_spline *) p;
	delete_spline(cur_s);
	(void)redisplay_spline(cur_s);
	break;
    default:
	return;
    }
}

void delete_all(void)
{
    clean_up();
    set_action_object(F_DELETE, O_ALL_OBJECT);

    /*
     * Aggregate assignment between variables is allowed, but not from
     * constant (weird!?)
     */

    set_latestobjects(&objects);

    objects.lines = NULL;
    objects.splines = NULL;
    objects.coasts = NULL;
    objects.grids = NULL;

    object_tails.lines = NULL;
    object_tails.splines = NULL;
    object_tails.coasts = NULL;
    object_tails.grids = NULL;
}
