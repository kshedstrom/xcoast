/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Henning Spruth
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both the copyright
 * notice and this permission notice appear in supporting documentation. 
 * No representations are made about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or implied warranty."
 */

#include <X11/keysym.h>
#include "fig.h"
#include "mode.h"
#include "resources.h"
#include "object.h"
#include "paintop.h"
#include "u_create.h"
#include "u_elastic.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_setup.h"
#include "w_zoom.h"
#include "w_indpanel.h"

extern int	pan_origin(void);
extern int	show_zoom(ind_sw_info *sw);

/* extern int		   gc_thickness[NUMOPS]; */

static void	do_zoom(int x, int y);
static void	cancel_zoom(void);
static void	init_zoombox_drawing(int x, int y);

static void	(*save_kbd_proc) ();
static void	(*save_locmove_proc) ();
static void	(*save_leftbut_proc) ();
static void	(*save_middlebut_proc) ();
static void	(*save_rightbut_proc) ();
static Cursor	save_cur_cursor;
static int	save_action_on;

float		zoomscale;	/* both zoomscales initialized in main() */
float		display_zoomscale;
int		zoomxoff = 0;
int		zoomyoff = 0;

static Boolean	zoom_in_progress = False;

/* used for private box drawing functions */
static int	my_fix_x, my_fix_y;
static int	my_cur_x, my_cur_y;


void
zoom_selected(int x, int y, unsigned int button)
{
    if (!zoom_in_progress) {
	switch (button) {
	case Button1:
	    init_zoombox_drawing(x, y);
	    break;
	case Button2:
	    pan_origin();
	    break;
	case Button3:
	    display_zoomscale = 1.0;
	    show_zoom(&ind_switches[ZOOM_SWITCH_INDEX]);
	    break;
	}
    } else if (button == Button1)
	do_zoom(x, y);
}


static void
my_box(int x, int y)
{
    elastic_box(my_fix_x, my_fix_y, my_cur_x, my_cur_y);
    my_cur_x = x;
    my_cur_y = y;
    elastic_box(my_fix_x, my_fix_y, my_cur_x, my_cur_y);
}


static void
init_zoombox_drawing(int x, int y)
{
    save_kbd_proc = canvas_kbd_proc;
    save_locmove_proc = canvas_locmove_proc;
    save_leftbut_proc = canvas_leftbut_proc;
    save_middlebut_proc = canvas_middlebut_proc;
    save_rightbut_proc = canvas_rightbut_proc;
    save_kbd_proc = canvas_kbd_proc;
    save_cur_cursor = cur_cursor;

    my_cur_x = my_fix_x = x;
    my_cur_y = my_fix_y = y;
    canvas_locmove_proc = moving_box;

    canvas_locmove_proc = my_box;
    canvas_leftbut_proc = do_zoom;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = cancel_zoom;
    elastic_box(my_fix_x, my_fix_y, my_cur_x, my_cur_y);
    set_temp_cursor(null_cursor);
    set_action_on();
    zoom_in_progress = True;
}

static void
do_zoom(int x, int y)
{
    int		    dimx, dimy;
    float	    scalex, scaley;

    elastic_box(my_fix_x, my_fix_y, my_cur_x, my_cur_y);
    zoomxoff = my_fix_x < x ? my_fix_x : x;
    zoomyoff = my_fix_y < y ? my_fix_y : y;
    dimx = abs(x - my_fix_x);
    dimy = abs(y - my_fix_y);
    if (zoomxoff < 0)
	zoomxoff = 0;
    if (zoomyoff < 0)
	zoomyoff = 0;
    if (dimx && dimy) {
	scalex = ZOOM_FACTOR * CANVAS_WD / (float) dimx;
	scaley = ZOOM_FACTOR * CANVAS_HT / (float) dimy;
	display_zoomscale = (scalex > scaley ? scaley : scalex);
	if (display_zoomscale <= 1.0)   /* keep to 0.1 increments */
	    display_zoomscale = (int)((display_zoomscale+0.09)*10.0)/10.0 - 0.1;

	show_zoom(&ind_switches[ZOOM_SWITCH_INDEX]);
    }
    /* restore state */
    canvas_kbd_proc = save_kbd_proc;
    canvas_locmove_proc = save_locmove_proc;
    canvas_leftbut_proc = save_leftbut_proc;
    canvas_middlebut_proc = save_middlebut_proc;
    canvas_rightbut_proc = save_rightbut_proc;
    canvas_kbd_proc = save_kbd_proc;
    set_cursor(save_cur_cursor);
    action_on = save_action_on;
    zoom_in_progress = False;
}

static void
cancel_zoom()
{
    elastic_box(my_fix_x, my_fix_y, my_cur_x, my_cur_y);
    /* restore state */
    canvas_kbd_proc = save_kbd_proc;
    canvas_locmove_proc = save_locmove_proc;
    canvas_leftbut_proc = save_leftbut_proc;
    canvas_middlebut_proc = save_middlebut_proc;
    canvas_rightbut_proc = save_rightbut_proc;
    canvas_kbd_proc = save_kbd_proc;
    set_cursor(save_cur_cursor);
    action_on = save_action_on;
    zoom_in_progress = False;
}
