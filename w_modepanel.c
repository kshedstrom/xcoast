/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Paul King
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both the copyright
 * notice and this permission notice appear in supporting documentation. 
 * No representations are made about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or implied warranty."
 */

#include "fig.h"
#include "figx.h"
#include "resources.h"
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "w_drawprim.h"
#include "w_icons.h"
#include "w_indpanel.h"
#include "w_util.h"
#include "w_mousefun.h"
#include "w_setup.h"

extern void	erase_objecthighlight(void);

extern void	line_drawing_selected(void);
extern void	intspline_drawing_selected(void);
extern void	point_adding_selected(void);
extern void	delete_point_selected(void);
extern void	move_point_selected(void);
extern void	delete_selected(void);
extern void	convert_selected(void);
extern void	update_selected(void);
int             new_objmask;

/**************	    local variables and routines   **************/

#define MAX_MODEMSG_LEN 80
typedef struct mode_switch_struct {
    PIXRECT         icon;
    int             mode;
    void             (*setmode_func) ();
    int             objmask;
    int             indmask;
    char            modemsg[MAX_MODEMSG_LEN];
    TOOL            widget;
    Pixmap          normalPM, reversePM;
}               mode_sw_info;

#define		setmode_action(z)    (z->setmode_func)(z)

DeclareStaticArgs(13);
/* pointer to current mode switch */
static mode_sw_info *current = NULL;

/* button selection event handler */
static void     sel_mode_but(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch);
static void     turn_on(mode_sw_info *msw);

static mode_sw_info mode_switches[] = {
    {&intspl_ic, F_INTSPLINE, intspline_drawing_selected, M_NONE,
    I_OPEN, "INTERPOLATED SPLINE drawing",},
    {&line_ic, F_POLYLINE, line_drawing_selected, M_NONE,
    I_OPEN, "POLYLINE drawing",},
    {&movept_ic, F_MOVE_POINT, move_point_selected, M_NO_TEXT,
    I_ADDMOVPT, "MOVE POINTs",},
    {&addpt_ic, F_ADD_POINT, point_adding_selected, M_VARPTS_OBJECT,
    I_ADDMOVPT, "ADD POINTs (to lines and splines)",},
    {&deletept_ic, F_DELETE_POINT, delete_point_selected, M_VARPTS_OBJECT,
    I_MIN1, "DELETE POINTs (from lines and splines)",},
    {&delete_ic, F_DELETE, delete_selected, M_ALL,
    I_MIN1, "DELETE objects",},
    {&update_ic, F_UPDATE, update_selected, M_ALL,
    I_OBJECT, "UPDATE object <-> current settings",},
    {&convert_ic, F_CONVERT, convert_selected,
	(M_POLYLINE_LINE | M_SPLINE_INTERP), I_MIN1,
    "CONVERT lines into splines or vice versa",},
};

#define		NUM_MODE_SW	(sizeof(mode_switches) / sizeof(mode_sw_info))

static Arg      button_args[] =
{
     /* 0 */ {XtNlabel, (XtArgVal) "    "},
     /* 1 */ {XtNwidth, (XtArgVal) 0},
     /* 2 */ {XtNheight, (XtArgVal) 0},
     /* 3 */ {XtNresizable, (XtArgVal) False},
     /* 4 */ {XtNborderWidth, (XtArgVal) 0},
     /* 5 */ {XtNresize, (XtArgVal) False},	/* keeps buttons from being
						 * resized when there are not
						 * a multiple of three of
						 * them */
     /* 6 */ {XtNbackgroundPixmap, (XtArgVal) NULL},
};

static XtActionsRec mode_actions[] =
{
    {"EnterModeSw", (XtActionProc) draw_mousefun_mode},
    {"LeaveModeSw", (XtActionProc) clear_mousefun},
    {"PressMiddle", (XtActionProc) notused_middle},
    {"ReleaseMiddle", (XtActionProc) clear_middle},
    {"PressRight", (XtActionProc) notused_right},
    {"ReleaseRight", (XtActionProc) clear_right},
};

static String   mode_translations =
"<EnterWindow>:EnterModeSw()highlight()\n\
    <Btn1Down>:\n\
    <Btn1Up>:\n\
    <Btn2Down>:PressMiddle()\n\
    <Btn2Up>:ReleaseMiddle()\n\
    <Btn3Down>:PressRight()\n\
    <Btn3Up>:ReleaseRight()\n\
    <LeaveWindow>:LeaveModeSw()unhighlight()\n";


extern void put_msg (const char*, ...);
void turn_off_current (void);
extern void unmanage_update_buts (void);
extern void update_markers (int mask);

void
init_mode_panel(TOOL tool)
{
    register int    i;
    register mode_sw_info *sw;

    FirstArg(XtNwidth, MODEPANEL_WD);
    NextArg(XtNhSpace, INTERNAL_BW);
    NextArg(XtNvSpace, INTERNAL_BW);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNfromVert, msg_form);
    NextArg(XtNvertDistance, -INTERNAL_BW);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    NextArg(XtNresizable, False);
    NextArg(XtNborderWidth, 0);
    NextArg(XtNmappedWhenManaged, False);

    mode_panel = XtCreateWidget("mode_panel", boxWidgetClass, tool,
				Args, ArgCount);

    XtAppAddActions(tool_app, mode_actions, XtNumber(mode_actions));

    for (i = 0; i < NUM_MODE_SW; ++i) {
	sw = &mode_switches[i];
	if (sw->mode == FIRST_DRAW_MODE) {
	    FirstArg(XtNwidth, MODE_SW_WD * SW_PER_ROW +
		     INTERNAL_BW * (SW_PER_ROW - 1));
	    NextArg(XtNborderWidth, 0);
	    NextArg(XtNresize, False);
	    NextArg(XtNheight, (MODEPANEL_SPACE + 1) / 2);
	    NextArg(XtNlabel, "Drawing\n modes");
	    d_label = XtCreateManagedWidget("label", labelWidgetClass,
					    mode_panel, Args, ArgCount);
	} else if (sw->mode == FIRST_EDIT_MODE) {
	    /* assume Args still set up from d_label */
	    ArgCount -= 2;
	    NextArg(XtNheight, (MODEPANEL_SPACE) / 2);
	    NextArg(XtNlabel, "Editing\n modes");
	    e_label = XtCreateManagedWidget("label", labelWidgetClass,
					    mode_panel, Args, ArgCount);
	}
	button_args[1].value = sw->icon->width;
	button_args[2].value = sw->icon->height;
	sw->widget = XtCreateManagedWidget("button", commandWidgetClass,
			    mode_panel, button_args, XtNumber(button_args));

	/* left button changes mode */
	XtAddEventHandler(sw->widget, ButtonPressMask, (Boolean) 0,
			  sel_mode_but, (XtPointer) sw);
	XtOverrideTranslations(sw->widget,
			       XtParseTranslationTable(mode_translations));
    }
    return;
}

/*
 * after panel widget is realized (in main) put some bitmaps etc. in it
 */

void setup_mode_panel(void)
{
    register int    i;
    register mode_sw_info *msw;
    register Display *d = tool_d;
    register Screen *s = tool_s;

    blank_gc = XCreateGC(tool_d, XtWindow(mode_panel), (unsigned long) 0, NULL);
    button_gc = XCreateGC(tool_d, XtWindow(mode_panel), (unsigned long) 0, NULL);
    FirstArg(XtNforeground, &but_fg);
    NextArg(XtNbackground, &but_bg);
    GetValues(mode_switches[0].widget);

    XSetBackground(tool_d, blank_gc, but_bg);
    XSetForeground(tool_d, blank_gc, but_bg);

    FirstArg(XtNfont, button_font);
    SetValues(d_label);
    SetValues(e_label);

    if (appres.INVERSE) {
	FirstArg(XtNbackground, WhitePixelOfScreen(tool_s));
    } else {
	FirstArg(XtNbackground, BlackPixelOfScreen(tool_s));
    }
    SetValues(mode_panel);

    for (i = 0; i < NUM_MODE_SW; ++i) {
	msw = &mode_switches[i];
	/* create normal bitmaps */
	msw->normalPM = XCreatePixmapFromBitmapData(d, XtWindow(msw->widget),
		       (char *) msw->icon->data, msw->icon->width, msw->icon->height,
				   but_fg, but_bg, DefaultDepthOfScreen(s));

	FirstArg(XtNbackgroundPixmap, msw->normalPM);
	SetValues(msw->widget);

	/* create reverse bitmaps */
	msw->reversePM = XCreatePixmapFromBitmapData(d, XtWindow(msw->widget),
		       (char *) msw->icon->data, msw->icon->width, msw->icon->height,
				   but_bg, but_fg, DefaultDepthOfScreen(s));
	/* install the accelerators in the buttons */
	XtInstallAllAccelerators(msw->widget, tool);
    }
    /* install the accelerators for the surrounding parts */
    XtInstallAllAccelerators(mode_panel, tool);
    XtInstallAllAccelerators(d_label, tool);
    XtInstallAllAccelerators(e_label, tool);

    XDefineCursor(d, XtWindow(mode_panel), arrow_cursor);
    FirstArg(XtNmappedWhenManaged, True);
    SetValues(mode_panel);
}

/* come here when a button is pressed in the mode panel */

/* ARGSUSED */
static void
sel_mode_but(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch)
{
    XButtonEvent    xbutton;
    mode_sw_info    *msw = (mode_sw_info *) closure;

    xbutton = event->xbutton;
    if (action_on) {
	put_msg("Please finish (or cancel) the current operation before changing modes");
	return;
    } else if (highlighting)
	erase_objecthighlight();
    if (xbutton.button == Button1) {	/* left button */
	turn_off_current();
	turn_on(msw);
	/* turn off the update boxes if not in update mode */
	if (msw->mode != F_UPDATE)
		unmanage_update_buts();
	put_msg(msw->modemsg);
	cur_mode = msw->mode;
	new_objmask = msw->objmask;
	update_markers(new_objmask);
	current = msw;
	setmode_action(msw);
    }
}

static void
turn_on(mode_sw_info *msw)
{
    FirstArg(XtNbackgroundPixmap, msw->reversePM);
    SetValues(msw->widget);
}

void turn_off_current(void)
{
    if (current) {
	FirstArg(XtNbackgroundPixmap, current->normalPM);
	SetValues(current->widget);
    }
}
