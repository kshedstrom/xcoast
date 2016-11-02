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

/*********************** IMPORTS ************************/

#include "fig.h"
#include "figx.h"
#include "resources.h"
#include "mode.h"
#include "paintop.h"
#include <X11/keysym.h>
#include "u_bound.h"
#include "w_canvas.h"
#include "w_cursor.h"
#include "w_mousefun.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"
#ifndef SYSV
#include "sys/time.h"
#endif
#include <X11/Xatom.h>

extern void 	erase_rulermark(void);
extern void 	erase_objecthighlight(void);

/************** LOCAL STRUCTURE ***************/

typedef struct _CompKey CompKey;

struct _CompKey {
    unsigned char   key;
    unsigned char   first;
    unsigned char   second;
    CompKey	   *next;
};

/*********************** EXPORTS ************************/

void		(*canvas_kbd_proc) ();
void		(*canvas_locmove_proc) ();
void		(*canvas_leftbut_proc) ();
void		(*canvas_middlebut_proc) ();
void		(*canvas_middlebut_save) ();
void		(*canvas_rightbut_proc) ();
void		(*return_proc) ();
void		null_proc(void);
double		clip_xmin, clip_ymin, clip_xmax, clip_ymax;
int		clip_width, clip_height;
double		cur_x, cur_y;

/*********************** LOCAL ************************/

static void	canvas_selected(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams);

int		ignore_exp_cnt = 2;	/* we get 2 expose events at startup */


extern int redisplay_region (int xmin, int ymin, int xmax, int ymax);
extern void init_grid (void);
extern int reset_clip_window (void);
extern int set_rulermark (double x, double y);
extern int zoom_selected (int x, int y, unsigned int button);

void null_proc(void)
{
    /* almost does nothing */
    if (highlighting)
	erase_objecthighlight();
}

/* ARGSUSED */
static void
canvas_exposed(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams)
{
    static int	    xmin = 9999, xmax = -9999, ymin = 9999, ymax = -9999;
    XExposeEvent   *xe = (XExposeEvent *) event;
    register int    tmp;

    if (xe->x < xmin)
	xmin = xe->x;
    if (xe->y < ymin)
	ymin = xe->y;
    if ((tmp = xe->x + xe->width) > xmax)
	xmax = tmp;
    if ((tmp = xe->y + xe->height) > ymax)
	ymax = tmp;
    if (xe->count > 0)
	return;

    /* kludge to stop getting extra redraws at start up */
    if (ignore_exp_cnt)
	ignore_exp_cnt--;
    else
	redisplay_region(xmin, ymin, xmax, ymax);
    xmin = 9999, xmax = -9999, ymin = 9999, ymax = -9999;
}

XtActionsRec	canvas_actions[] =
{
    {"EventCanv", (XtActionProc) canvas_selected},
    {"ExposeCanv", (XtActionProc) canvas_exposed},
    {"EnterCanv", (XtActionProc) draw_mousefun_canvas},
    {"LeaveCanv", (XtActionProc) clear_mousefun},
    {"EraseRulerMark", (XtActionProc) erase_rulermark},
};

/* need the ~Meta for the EventCanv action so that the accelerators still work */
static String	canvas_translations =
"<Motion>:EventCanv()\n\
    Any<BtnDown>:EventCanv()\n\
    <EnterWindow>:EnterCanv()\n\
    <LeaveWindow>:LeaveCanv()EraseRulerMark()\n\
    ~Meta<Key>:EventCanv()\n\
    <Expose>:ExposeCanv()\n";

void
init_canvas(TOOL tool)
{
    XColor	   fixcolors[2];

    DeclareArgs(10);

    FirstArg(XtNlabel, "");
    NextArg(XtNwidth, CANVAS_WD);
    NextArg(XtNheight, CANVAS_HT);
    NextArg(XtNfromHoriz, mode_panel);
    NextArg(XtNhorizDistance, -INTERNAL_BW);
    NextArg(XtNfromVert, topruler_sw);
    NextArg(XtNvertDistance, -INTERNAL_BW);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNborderWidth, INTERNAL_BW);

    canvas_sw = XtCreateWidget("canvas", labelWidgetClass, tool,
			       Args, ArgCount);

    FirstArg(XtNforeground, &x_fg_color.pixel);
    NextArg(XtNbackground, &x_bg_color.pixel);
    GetValues(canvas_sw);

    /*
     * get the RGB values for recolor cursor use -- may want to have cursor
     * color resource
     */
    fixcolors[0] = x_fg_color;
    fixcolors[1] = x_bg_color;
    XQueryColors(tool_d, DefaultColormapOfScreen(tool_s), fixcolors, 2);
    x_fg_color = fixcolors[0];
    x_bg_color = fixcolors[1];

    /* now fix the global GC */
    XSetState(tool_d, gc, x_fg_color.pixel, x_bg_color.pixel, GXcopy,
	      AllPlanes);

    /* and recolor the cursors */
    recolor_cursors();

    canvas_leftbut_proc = null_proc;
    canvas_middlebut_proc = null_proc;
    canvas_rightbut_proc = null_proc;
    canvas_kbd_proc = canvas_locmove_proc = null_proc;
    XtAppAddActions(tool_app, canvas_actions, XtNumber(canvas_actions));
    XtAugmentTranslations(canvas_sw,
			   XtParseTranslationTable(canvas_translations));
    return;
}

void setup_canvas(void)
{
    canvas_win = XtWindow(canvas_sw);
    init_grid();
    reset_clip_window();
}

/* ARGSUSED */
static
void canvas_selected(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams)
{
    register int    x, y;
    KeySym	    key;
    static int	    sx = -10000, sy = -10000;
    XButtonPressedEvent *be = (XButtonPressedEvent *) event;
    XKeyPressedEvent *ke = (XKeyPressedEvent *) event;
    extern void     pan_up(void), pan_down(void), pan_left(void), pan_right(void);
    extern void     pan_origin(void);

    switch (event->type) {
    case MotionNotify:
#if defined(SMOOTHMOTION) || defined(OPENWIN)
	/* translate from zoomed coords to object coords */
	x = BACKX(event->x);
	y = BACKY(event->y);

	if (x == sx && y == sy)
	    return;
	sx = x;
	sy = y;
#else
	{
	    Window	    rw, cw;
	    int		    rx, ry, cx, cy;
	    unsigned int    mask;

	    XQueryPointer(event->display, event->window,
			  &rw, &cw,
			  &rx, &ry,
			  &cx, &cy,
			  &mask);
	    cx = BACKX(cx);
	    cy = BACKY(cy);

	    if (cx == sx && cy == sy)
		break;
	    x = sx = cx;	/* these are zoomed */
	    y = sy = cy;	/* coordinates!	    */
	}
#endif /* SMOOTHMOTION || OPENWIN */
	set_rulermark((double)x, (double)y);
	(*canvas_locmove_proc) (x, y);
	break;
    case ButtonPress:
	/* translate from zoomed coords to object coords */
	x = BACKX(event->x);
	y = BACKY(event->y);

	/* Convert Alt-Button3 to Button2 */
	if (be->button == Button3 && be->state & Mod1Mask) {
	    be->button = Button2;
	    be->state &= ~Mod1Mask;
	}

	/* call interactive zoom function if control key is pressed */
	if (be->state & ControlMask) {
	    zoom_selected(x, y, be->button);
	    break;
	}

	if (be->button == Button1)
	    (*canvas_leftbut_proc) (x, y, be->state & ShiftMask);
	else if (be->button == Button2)
	    (*canvas_middlebut_proc) (x, y, be->state & ShiftMask);
	else if (be->button == Button3)
	    (*canvas_rightbut_proc) (x, y, be->state & ShiftMask);
	break;
    case KeyPress:
	/* we might want to check action_on */
	/* if arrow keys are pressed, pan */
	key = XLookupKeysym(ke, 0);
	if (key == XK_Left ||
	    key == XK_Right ||
	    key == XK_Up ||
	    key == XK_Down ||
	    key == XK_Home ||
	    key == XK_Multi_key ||
	    key == XK_Alt_L) {
	    switch (key) {
	    case XK_Left:
		pan_left();
		break;
	    case XK_Right:
		pan_right();
		break;
	    case XK_Up:
		pan_up();
		break;
	    case XK_Down:
		pan_down();
		break;
	    case XK_Home:
		pan_origin();
		break;
	    }
	}
	break;
    }
}

void clear_canvas(void)
{
    XClearArea(tool_d, canvas_win, (int)clip_xmin, (int)clip_ymin,
	       clip_width, clip_height, False);
}
