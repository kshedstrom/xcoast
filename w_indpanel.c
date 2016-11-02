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
#include "object.h"
#include "mode.h"
#include "paintop.h"
#include "u_fonts.h"
#include "w_drawprim.h"
#include "w_icons.h"
#include "w_indpanel.h"
#include "w_mousefun.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"

extern Atom     wm_delete_window;
extern char    *panel_get_value(Widget widg);
void		show_zoom(ind_sw_info *sw);
void		show_gridsize(ind_sw_info *sw);
extern int	cur_updatemask;
extern Widget	make_popup_menu(char **entries, Cardinal nent, Widget parent, XtCallbackProc callback);

/**************	    local variables and routines   **************/

static int	cur_indmask = I_MIN1;

static String	set_translations = 
	"<Key>Return: SetValue()";
static void	nval_panel_set(Widget w, XButtonEvent *ev);
static XtActionsRec set_actions[] =
{
    {"SetValue", (XtActionProc) nval_panel_set},
};
static String   nval_translations =
        "<Message>WM_PROTOCOLS: QuitNval()\n";
static void	nval_panel_cancel(Widget w, XButtonEvent *ev);
static XtActionsRec     nval_actions[] =
{
    {"QuitNval", (XtActionProc) nval_panel_cancel},
};
static String   choice_translations =
        "<Message>WM_PROTOCOLS: QuitChoice()\n";
static void     choice_panel_cancel(Widget w, XButtonEvent *ev);
static XtActionsRec     choice_actions[] =
{
    {"QuitChoice", (XtActionProc) choice_panel_cancel},
};

DeclareStaticArgs(15);

/* declarations for choice buttons */
static void	inc_choice(ind_sw_info *sw), dec_choice(ind_sw_info *sw);
static void	show_linestyle(ind_sw_info *sw);
static void	show_gridmode(ind_sw_info *sw);

/* declarations for value buttons */
static void	show_linewidth(ind_sw_info *sw), inc_linewidth(ind_sw_info *sw), dec_linewidth(ind_sw_info *sw);
static void	show_color(ind_sw_info *sw), next_color(ind_sw_info *sw), prev_color(ind_sw_info *sw);
static void	inc_zoom(ind_sw_info *sw), dec_zoom(ind_sw_info *sw);
static void	inc_grid(ind_sw_info *sw), dec_grid(ind_sw_info *sw);

static void	note_state(Widget w, XtPointer closure, XEvent *ev, Boolean *continue_to_dispatch);

static char	indbuf[12];
static float	old_display_zoomscale = -1.0;
static int	old_gridsize = -1;

#define		DEF_IND_SW_HT		32
#define		DEF_IND_SW_WD		64
#define		FONT_IND_SW_WD		(40+PS_FONTPANE_WD)
#define		NARROW_IND_SW_WD	56
#define		WIDE_IND_SW_WD		76
#define		XWIDE_IND_SW_WD		86

/* indicator switch definitions */

static choice_info gridmode_choices[] = {
    {GRID_0, &none_ic,},
    {GRID_1, &grid1_ic,},
};

#define NUM_GRIDMODE_CHOICES (sizeof(gridmode_choices)/sizeof(choice_info))

static choice_info linestyle_choices[] = {
    {SOLID_LINE, &solidline_ic,},
    {DASH_LINE, &dashline_ic,},
    {DOTTED_LINE, &dottedline_ic,},
};

#define NUM_LINESTYLE_CHOICES (sizeof(linestyle_choices)/sizeof(choice_info))

choice_info	color_choices[NUMCOLORS + 1];

#define I_CHOICE	0
#define I_IVAL		1
#define I_FVAL		2

#define		inc_action(z)	(z->inc_func)(z)
#define		dec_action(z)	(z->dec_func)(z)
#define		show_action(z)	(z->show_func)(z)

ind_sw_info	ind_switches[] = {
    {I_FVAL, I_ZOOM, "Zoom", "Scale", NARROW_IND_SW_WD,
	NULL, &display_zoomscale, inc_zoom, dec_zoom, show_zoom,},
    {I_CHOICE, I_GRIDMODE, "Grid", "Mode", DEF_IND_SW_WD,
	&gridmode, NULL, inc_choice, dec_choice, show_gridmode,
	gridmode_choices, NUM_GRIDMODE_CHOICES, NUM_GRIDMODE_CHOICES,},
    {I_IVAL, I_GRIDSIZE, "Grid", "Scale", NARROW_IND_SW_WD,
	&gridsize, NULL, inc_grid, dec_grid, show_gridsize,},
    {I_CHOICE, I_COLOR, "Color", "", WIDE_IND_SW_WD,
	(int *) &cur_color, NULL, next_color, prev_color, show_color,
	color_choices, NUMCOLORS + 1, (NUMCOLORS + 1) / 2},
    {I_IVAL, I_LINEWIDTH, "Line", "Width", NARROW_IND_SW_WD,
	&cur_linewidth, NULL, inc_linewidth, dec_linewidth, show_linewidth,},
    {I_CHOICE, I_LINESTYLE, "Line", "Style", DEF_IND_SW_WD,
	&cur_linestyle, NULL, inc_choice, dec_choice, show_linestyle,
	linestyle_choices, NUM_LINESTYLE_CHOICES, NUM_LINESTYLE_CHOICES,},
};

#define		NUM_IND_SW	(sizeof(ind_switches) / sizeof(ind_sw_info))

/* button selection event handler */
static void	sel_ind_but(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch);

/* arguments for the update indicator boxes in the indicator buttons */

static Arg	upd_args[] = 
{
    /* 0 */ {XtNwidth, (XtArgVal) 8},
    /* 1 */ {XtNheight, (XtArgVal) 8},
    /* 2 */ {XtNborderWidth, (XtArgVal) 1},
    /* 3 */ {XtNtop, (XtArgVal) XtChainTop},
    /* 4 */ {XtNright, (XtArgVal) XtChainRight},
    /* 5 */ {XtNstate, (XtArgVal) True},
    /* 6 */ {XtNvertDistance, (XtArgVal) 0},
    /* 7 */ {XtNhorizDistance, (XtArgVal) 0},
    /* 8 */ {XtNlabel, (XtArgVal) " "},
    /* 9 */ {XtNhighlightThickness, (XtArgVal) 0},
};

static XtActionsRec ind_actions[] =
{
    {"EnterIndSw", (XtActionProc) draw_mousefun_ind},
    {"LeaveIndSw", (XtActionProc) clear_mousefun},
};

static String	ind_translations =
"<EnterWindow>:EnterIndSw()highlight()\n\
    <LeaveWindow>:LeaveIndSw()unhighlight()\n";


void update_indpanel(int mask);
void update_current_settings(void);
void popup_nval_panel(ind_sw_info *isw);
void popup_choice_panel(ind_sw_info *isw);
extern int pw_vector(Window w, double x1, double y1, double x2, double y2,
    int op, int line_width, int line_style, float style_val, int color);
extern void put_msg(const char*, ...);
extern void setup_grid(void);
extern void reset_rulers(void);
extern void redisplay_rulers(void);
extern void redisplay_canvas(void);

static float     grid_deg[21] = { 10., 5., 3.33, 2.5, 2., 
			          1.25, 1., .67, .5, .25 };
static int       grid_choices[10] = { 1, 2, 3, 4, 5, 8, 10, 15, 20, 40};
static int       grid_choice;

void
init_ind_panel(TOOL tool)
{
    int		i;
    ind_sw_info	*sw;

    /* does he want to always see ALL of the indicator buttons? */
    cur_indmask = I_ALL;	/* yes */
    i = DEF_IND_SW_HT+2*INTERNAL_BW;	/* one row high when showing all buttons */

    /* make a scrollable viewport in case all the buttons don't fit */

    ind_viewp = XtVaCreateWidget("ind_viewport",
		viewportWidgetClass, tool,
    		XtNallowHoriz, True,
    		XtNwidth, INDPANEL_WD,
    		XtNheight, i,
    		XtNborderWidth, 0,
    		XtNresizable, False,
    		XtNfromVert, canvas_sw,
    		XtNvertDistance, -INTERNAL_BW,
    		XtNtop, XtChainBottom,
    		XtNbottom, XtChainBottom,
    		XtNleft, XtChainLeft,
    		XtNright, XtChainRight,
    		XtNuseBottom, True,
		NULL);


    ind_panel = XtVaCreateManagedWidget("ind_panel",
		boxWidgetClass, ind_viewp,
    		XtNwidth, INDPANEL_WD,
    		XtNheight, i,
    		XtNhSpace, 0,
    		XtNvSpace, 0,
    		XtNresizable, True,
    		XtNborderWidth, 0,
    		XtNorientation, XtorientVertical,  /* use two rows */
		NULL);

    /* start with all components affected by update */
    cur_updatemask = I_UPDATEMASK;

    XtAppAddActions(tool_app, ind_actions, XtNumber(ind_actions));

    for (i = 0; i < NUM_IND_SW; ++i) {
	sw = &ind_switches[i];

	sw->formw = XtVaCreateWidget("button_form",
		formWidgetClass, ind_panel,
		XtNwidth, sw->sw_width,
		XtNheight, DEF_IND_SW_HT,
		XtNdefaultDistance, 0,
		XtNborderWidth, INTERNAL_BW,
		NULL);

	/* make an update button in the upper-right corner of the main button */
	if (sw->func & I_UPDATEMASK)
	    {
	    upd_args[7].value = sw->sw_width
					- upd_args[0].value
					- 2*upd_args[2].value;
	    sw->updbut = XtCreateWidget("update", toggleWidgetClass,
			     sw->formw, upd_args, XtNumber(upd_args));
	    sw->update = True;
	    XtAddEventHandler(sw->updbut, ButtonReleaseMask, (Boolean) 0,
			     note_state, (XtPointer) sw);
	    }

	/* now create the command button */
	sw->button = XtVaCreateManagedWidget("button",
		commandWidgetClass, sw->formw,
     		XtNlabel, (XtArgVal) "        ",
     		XtNwidth, (XtArgVal) sw->sw_width,
     		XtNheight, (XtArgVal) DEF_IND_SW_HT,
     		XtNresizable, (XtArgVal) False,
     		XtNborderWidth, (XtArgVal) 0,
     		XtNresize, (XtArgVal) False,	/* keeps buttons from being
					 * resized when there are not
					 * a multiple of three of them */
     		XtNbackgroundPixmap, (XtArgVal) NULL,
		NULL);
	/* map this button if it is needed */
	if (sw->func & cur_indmask)
	    XtManageChild(sw->formw);

	/* allow left & right buttons */
	/* (callbacks pass same data for ANY button) */
	XtAddEventHandler(sw->button, ButtonReleaseMask, (Boolean) 0,
			  sel_ind_but, (XtPointer) sw);
	XtOverrideTranslations(sw->button,
			       XtParseTranslationTable(ind_translations));
    }
    update_indpanel(cur_indmask);
}

/* ARGSUSED */
static void
note_state(Widget w, XtPointer closure, XEvent *ev, Boolean *continue_to_dispatch)
{
    ind_sw_info *sw = (ind_sw_info *) closure;
    XButtonEvent *event = &ev->xbutton;

    if (event->button != Button1)
	return;

    /* toggle update status of this indicator */
    sw->update = !sw->update;
    if (sw->update)
	cur_updatemask |= sw->func;	/* turn on update status */
    else
	cur_updatemask &= ~sw->func;	/* turn off update status */
}

void manage_update_buts(void)
{
    int		    i;
    for (i = 0; i < NUM_IND_SW; ++i)
	if (ind_switches[i].func & I_UPDATEMASK)
	    XtManageChild(ind_switches[i].updbut);
}
		
void unmanage_update_buts(void)
{
    int		    i;
    for (i = 0; i < NUM_IND_SW; ++i)
	if (ind_switches[i].func & I_UPDATEMASK)
	    XtUnmanageChild(ind_switches[i].updbut);
}
		
void setup_ind_panel(void)
{
    int		    i;
    ind_sw_info	   *isw;
    Display	   *d = tool_d;
    Screen	   *s = tool_s;
    Pixmap	    p;

    /* get the foreground and background from the indicator widget */
    /* and create a gc with those values */
    ind_button_gc = XCreateGC(tool_d, XtWindow(ind_panel), (unsigned long) 0, NULL);
    FirstArg(XtNforeground, &ind_but_fg);
    NextArg(XtNbackground, &ind_but_bg);
    GetValues(ind_switches[0].button);
    XSetBackground(tool_d, ind_button_gc, ind_but_bg);
    XSetForeground(tool_d, ind_button_gc, ind_but_fg);
    XSetFont(tool_d, ind_button_gc, button_font->fid);

    /* also create gc with fore=background for blanking areas */
    ind_blank_gc = XCreateGC(tool_d, XtWindow(ind_panel), (unsigned long) 0, NULL);
    XSetBackground(tool_d, ind_blank_gc, ind_but_bg);
    XSetForeground(tool_d, ind_blank_gc, ind_but_bg);

    /* create a gc for the color 'palette' */
    color_gc = XCreateGC(tool_d, XtWindow(ind_panel), (unsigned long) 0, NULL);

    SetValues(ind_viewp);

    for (i = 0; i < NUM_IND_SW; ++i) {
	isw = &ind_switches[i];

	p = XCreatePixmap(d, XtWindow(isw->button), isw->sw_width,
			  DEF_IND_SW_HT, DefaultDepthOfScreen(s));
	XFillRectangle(d, p, ind_blank_gc, 0, 0,
		       isw->sw_width, DEF_IND_SW_HT);
	XDrawImageString(d, p, ind_button_gc, 3, 12, isw->line1, strlen(isw->line1));
	XDrawImageString(d, p, ind_button_gc, 3, 25, isw->line2, strlen(isw->line2));

	isw->normalPM = (XtArgVal) p;
	XtVaSetValues(isw->button,
	     XtNbackgroundPixmap, (XtArgVal) p,
	     NULL);
	XtInstallAllAccelerators(isw->button, tool);
    }
    XtInstallAllAccelerators(ind_panel, tool);

    XDefineCursor(d, XtWindow(ind_panel), arrow_cursor);
    update_current_settings();

    FirstArg(XtNmappedWhenManaged, True);
    SetValues(ind_panel);
}

void update_indpanel(int mask)
{
    register int    i;
    register ind_sw_info *isw;

    cur_indmask = mask;
    XtUnmanageChild(ind_panel);
    for (isw = ind_switches, i = 0; i < NUM_IND_SW; isw++, i++) {
	if (isw->func & cur_indmask) {
	    XtManageChild(isw->formw);
	} else {
	    XtUnmanageChild(isw->formw);
	}
    }
    XtManageChild(ind_panel);
}

/* come here when a button is pressed in the indicator panel */

/* ARGSUSED */
static void
sel_ind_but(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch)
{
    XButtonEvent xbutton;
    ind_sw_info *isw = (ind_sw_info *) closure;
    xbutton = event->xbutton;
    if ((xbutton.button == Button2)  ||
              (xbutton.button == Button3 && xbutton.state & Mod1Mask)) { /* middle button */
	dec_action(isw);
    } else if (xbutton.button == Button3) {	/* right button */
	inc_action(isw);
    } else {			/* left button */
	if (isw->type == I_IVAL || isw->type == I_FVAL)
	    popup_nval_panel(isw);
	else if (isw->type == I_CHOICE)
	    popup_choice_panel(isw);
    }
}

static
void update_string_pixmap(ind_sw_info *isw, char *buf, int xpos, int ypos)
{
    XDrawImageString(tool_d, isw->normalPM, ind_button_gc,
		     xpos, ypos, buf, strlen(buf));
    /*
     * Fool the toolkit by changing the background pixmap to 0 then giving it
     * the modified one again.	Otherwise, it sees that the pixmap ID is not
     * changed and doesn't actually draw it into the widget window
     */
    XtVaSetValues(isw->button,
	 XtNbackgroundPixmap, (XtArgVal) 0,
	 NULL);

    /* put the pixmap in the widget background */
    XtVaSetValues(isw->button,
	 XtNbackgroundPixmap, (XtArgVal) isw->normalPM,
	 NULL);
}

static
void update_choice_pixmap(ind_sw_info *isw, int mode)
{
    choice_info	   *tmp_choice;
    register Pixmap p;

    /* put the pixmap in the widget background */
    p = isw->normalPM;
    tmp_choice = isw->choices + mode;
    XPutImage(tool_d, p, ind_button_gc, tmp_choice->icon, 0, 0, 32, 0, 32, 32);
    /*
     * Fool the toolkit by changing the background pixmap to 0 then giving it
     * the modified one again.	Otherwise, it sees that the pixmap ID is not
     * changed and doesn't actually draw it into the widget window
     */
    XtVaSetValues(isw->button,
	 XtNbackgroundPixmap, (XtArgVal) 0,
	 NULL);

    /* put the pixmap in the widget background */
    XtVaSetValues(isw->button,
	 XtNbackgroundPixmap, (XtArgVal) p,
	 NULL);
}

/********************************************************

	auxiliary functions

********************************************************/

static Widget	choice_popup;
static ind_sw_info *choice_i;
static Widget	nval_popup, form, cancel, set, beside, below, newvalue,
		label;
static Widget	dash_length, dot_gap;
static ind_sw_info *nval_i;

/* handle choice settings */

static void
choice_panel_dismiss(void)
{
    XtDestroyWidget(choice_popup);
    XtSetSensitive(choice_i->button, True);
}

/* ARGSUSED */
static void
choice_panel_cancel(Widget w, XButtonEvent *ev)
{
    choice_panel_dismiss();
}

/* ARGSUSED */
static void
choice_panel_set(Widget w, choice_info *sel_choice, XButtonEvent *ev)
{
    (*choice_i->i_varadr) = sel_choice->value;
    show_action(choice_i);

    /* auxiliary info */
    switch (choice_i->func) {
    case I_LINESTYLE:
	/* dash length */
	cur_dashlength = (float) atof(panel_get_value(dash_length));
	if (cur_dashlength <= 0.0)
	    cur_dashlength = DEF_DASHLENGTH;
	/* dot gap */
	cur_dotgap = (float) atof(panel_get_value(dot_gap));
	if (cur_dotgap <= 0.0)
	    cur_dotgap = DEF_DOTGAP;
	break;
    }

    choice_panel_dismiss();
}

void popup_choice_panel(ind_sw_info *isw)
{
    Position	    x_val, y_val;
    Dimension	    width, height;
    char	    buf[32];
    choice_info	   *tmp_choice;
    Pixmap	    p;
    Pixel	    form_fg;
    register int    i;
    static int      actions_added=0;

    choice_i = isw;
    XtSetSensitive(choice_i->button, False);

    FirstArg(XtNwidth, &width);
    NextArg(XtNheight, &height);
    GetValues(tool);
    /* position the popup 1/3 in from left and 2/3 down from top */
    XtTranslateCoords(tool, (Position) (width / 3), (Position) (2 * height / 3),
		      &x_val, &y_val);


    choice_popup = XtVaCreatePopupShell("xfig_set_indicator_panel",
		transientShellWidgetClass, tool,
    		XtNx, x_val,
    		XtNy, y_val,
    		XtNresize, False,
    		XtNresizable, False,
    		XtNtitle, "Xcoast: Set indicator panel",
		NULL);
    XtOverrideTranslations(choice_popup,
                       XtParseTranslationTable(choice_translations));
    if (!actions_added) {
        XtAppAddActions(tool_app, choice_actions, XtNumber(choice_actions));
	actions_added = 1;
    }

    form = XtCreateManagedWidget("form", formWidgetClass, choice_popup, NULL, 0);

    sprintf(buf, "%s %s", isw->line1, isw->line2);
    label = XtVaCreateManagedWidget(buf, labelWidgetClass, form,
    		XtNborderWidth, 0,
		NULL);

    cancel = XtVaCreateManagedWidget("cancel",
		commandWidgetClass, form,
    		XtNlabel, "cancel",
    		XtNfromVert, label,
    		XtNresize, False,
    		XtNresizable, False,
    		XtNheight, 32,
    		XtNborderWidth, INTERNAL_BW,
		NULL);
    XtAddEventHandler(cancel, ButtonReleaseMask, (Boolean) 0,
		      (XtEventHandler)choice_panel_cancel, (XtPointer) NULL);

    tmp_choice = isw->choices;

    for (i = 0; i < isw->numchoices; tmp_choice++, i++) {
	if (isw->func == I_COLOR) {
	    p = 0;
	    tmp_choice->value = (i >= NUMCOLORS ? DEFAULT_COLOR : i);
	} else
	    p = XCreatePixmapFromBitmapData(tool_d, XtWindow(ind_panel),
			    (char *) tmp_choice->icon->data, tmp_choice->icon->width,
			   tmp_choice->icon->height, ind_but_fg, ind_but_bg,
					    DefaultDepthOfScreen(tool_s));
	if (i % isw->sw_per_row == 0) {
	    if (i == 0)
		below = label;
	    else
		below = beside;
	    beside = cancel;
	}
	FirstArg(XtNfromVert, below);
	NextArg(XtNfromHoriz, beside);
	if (isw->func != I_COLOR) {
	    NextArg(XtNbackgroundPixmap, p);
	    NextArg(XtNwidth, tmp_choice->icon->width);
	    NextArg(XtNheight, tmp_choice->icon->height);
	} else {		/* Color popup menu */
	    NextArg(XtNheight, 32);
	    NextArg(XtNwidth, 64);
	    if (i < NUMCOLORS && i >= 0) {	/* it's a proper color */
		if (all_colors_available) {
		    XColor	    col;

		    col.pixel = appres.color[i];
		    XQueryColor(tool_d, DefaultColormapOfScreen(tool_s), &col);
		    if ((0.3 * col.red + 0.59 * col.green + 0.11 * col.blue) < 0.5 * (255 << 8))
			form_fg = appres.color[WHITE];
		    else
			form_fg = appres.color[BLACK];
		    NextArg(XtNforeground, form_fg);
		    NextArg(XtNbackground, appres.color[i]);
		}
		NextArg(XtNlabel, colorNames[i + 1]);
	    } else {		/* it's the default color */
		NextArg(XtNforeground, x_fg_color.pixel);
		NextArg(XtNlabel, colorNames[0]);
	    }
	}
	NextArg(XtNresize, False);
	NextArg(XtNresizable, False);
	NextArg(XtNborderWidth, INTERNAL_BW);
	beside = XtCreateManagedWidget(" ", commandWidgetClass,
				       form, Args, ArgCount);
	XtAddEventHandler(beside, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)choice_panel_set, (XtPointer) tmp_choice);
    }

    /* auxiliary info */
    switch (isw->func) {
    case I_LINESTYLE:
	/* dash length */
	label = XtVaCreateManagedWidget("default_dash_length",
			labelWidgetClass, form,
			XtNfromVert, beside,
			XtNborderWidth, 0,
			XtNlabel, "Default dash length =",
			NULL);
	sprintf(buf, "%1.1f", cur_dashlength);
	dash_length = XtVaCreateManagedWidget(buf,
			asciiTextWidgetClass, form,
			XtNfromVert, beside,
			XtNborderWidth, INTERNAL_BW,
			XtNfromHoriz, label,
			XtNstring, buf,
			XtNinsertPosition, strlen(buf),
			XtNeditType, "append",
			XtNwidth, 40,
			NULL);
	/* dot gap */
	label = XtVaCreateManagedWidget("default_dot_gap",
			labelWidgetClass, form,
			XtNfromVert, dash_length,
			XtNborderWidth, 0,
			XtNlabel, "    Default dot gap =",
			NULL);
	sprintf(buf, "%1.1f", cur_dotgap);
	dot_gap = XtVaCreateManagedWidget(buf,
			asciiTextWidgetClass, form,
			XtNfromVert, dash_length,
			XtNborderWidth, INTERNAL_BW,
			XtNfromHoriz, label,
			XtNstring, buf,
			XtNinsertPosition, strlen(buf),
			XtNeditType, "append",
			XtNwidth, 40,
			NULL);
	break;
    }

    XtPopup(choice_popup, XtGrabExclusive);
    (void) XSetWMProtocols(XtDisplay(choice_popup), XtWindow(choice_popup),
                           &wm_delete_window, 1);

}

/* handle integer and floating point settings */

static void
nval_panel_dismiss(void)
{
    XtDestroyWidget(nval_popup);
    XtSetSensitive(nval_i->button, True);
}

/* ARGSUSED */
static void
nval_panel_cancel(Widget w, XButtonEvent *ev)
{
    nval_panel_dismiss();
}

/* ARGSUSED */
static void
nval_panel_set(Widget w, XButtonEvent *ev)
{
    int		    new_i_value;
    float	    new_f_value;


    if (nval_i->type == I_IVAL)
	    {
	    new_i_value = atoi(panel_get_value(newvalue));
	    (*nval_i->i_varadr) = new_i_value;
	    }
    else
	    {
	    new_f_value = atof(panel_get_value(newvalue));
	    (*nval_i->f_varadr) = new_f_value;
	    }
    nval_panel_dismiss();
    show_action(nval_i);
}

void popup_nval_panel(ind_sw_info *isw)
{
    Position	    x_val, y_val;
    Dimension	    width, height;
    char	    buf[32];
    static int      actions_added=0;

    nval_i = isw;
    XtSetSensitive(nval_i->button, False);

    FirstArg(XtNwidth, &width);
    NextArg(XtNheight, &height);
    GetValues(tool);
    /* position the popup 1/3 in from left and 2/3 down from top */
    XtTranslateCoords(tool, (Position) (width / 3), (Position) (2 * height / 3),
		      &x_val, &y_val);

    nval_popup = XtVaCreatePopupShell("xfig_set_indicator_panel",
			transientShellWidgetClass, tool,
    			XtNx, x_val,
    			XtNy, y_val,
    			XtNwidth, 240,
			NULL);
    XtOverrideTranslations(nval_popup,
                       XtParseTranslationTable(nval_translations));
    if (!actions_added) {
        XtAppAddActions(tool_app, nval_actions, XtNumber(nval_actions));
	actions_added = 1;
    }

    form = XtCreateManagedWidget("form", formWidgetClass, nval_popup, NULL, 0);

    sprintf(buf, "%s %s", isw->line1, isw->line2);
    label = XtVaCreateManagedWidget(buf, labelWidgetClass, form,
    			XtNborderWidth, 0,
			NULL);

    newvalue = XtVaCreateManagedWidget("value",
			labelWidgetClass, form,
    			XtNfromVert, label,
    			XtNborderWidth, 0,
    			XtNlabel, "Value =",
			NULL);
    /* int or float? */
    if (isw->type == I_IVAL)
	    sprintf(buf, "%d", (*isw->i_varadr));
    else
	    sprintf(buf, "%4.2lf", (*isw->f_varadr));
    newvalue = XtVaCreateManagedWidget(buf,
			asciiTextWidgetClass, form,
    			XtNfromVert, label,
    			XtNborderWidth, INTERNAL_BW,
    			XtNfromHoriz, newvalue,
    			XtNstring, buf,
    			XtNinsertPosition, strlen(buf),
    			XtNeditType, "append",
    			XtNwidth, 40,
			NULL);

    /* add translation and action to set value on carriage return */
    XtAppAddActions(tool_app, set_actions, XtNumber(set_actions));
    XtOverrideTranslations(newvalue, XtParseTranslationTable(set_translations));

    cancel = XtVaCreateManagedWidget("cancel",
			commandWidgetClass, form,
    			XtNlabel, "cancel",
    			XtNfromVert, newvalue,
    			XtNborderWidth, INTERNAL_BW,
			NULL);
    XtAddEventHandler(cancel, ButtonReleaseMask, (Boolean) 0,
		      (XtEventHandler)nval_panel_cancel, (XtPointer) NULL);

    set = XtVaCreateManagedWidget("set",
			commandWidgetClass, form,
    			XtNlabel, "set",
    			XtNfromVert, newvalue,
    			XtNfromHoriz, cancel,
    			XtNborderWidth, INTERNAL_BW,
			NULL);
    XtAddEventHandler(set, ButtonReleaseMask, (Boolean) 0,
		      (XtEventHandler)nval_panel_set, (XtPointer) NULL);

    XtPopup(nval_popup, XtGrabExclusive);
    (void) XSetWMProtocols(XtDisplay(nval_popup), XtWindow(nval_popup),
                           &wm_delete_window, 1);
}

/********************************************************

	commands to change indicator settings

********************************************************/

void update_current_settings(void)
{
    int		    i;
    ind_sw_info	   *isw;

    for (i = 0; i < NUM_IND_SW; ++i) {
	isw = &ind_switches[i];
	show_action(isw);
    }
}

static
void dec_choice(ind_sw_info *sw)
{
    if (--(*sw->i_varadr) < 0)
	(*sw->i_varadr) = sw->numchoices - 1;
    show_action(sw);
}

static
void inc_choice(ind_sw_info *sw)
{
    if (++(*sw->i_varadr) > sw->numchoices - 1)
	(*sw->i_varadr) = 0;
    show_action(sw);
}

/* LINE WIDTH		 */

#define MAXLINEWIDTH 200

static
void dec_linewidth(ind_sw_info *sw)
{
    --cur_linewidth;
    show_linewidth(sw);
}

static
void inc_linewidth(ind_sw_info *sw)
{
    ++cur_linewidth;
    show_linewidth(sw);
}

static
void show_linewidth(ind_sw_info *sw)
{
    if (cur_linewidth > MAXLINEWIDTH)
	cur_linewidth = MAXLINEWIDTH;
    else if (cur_linewidth < 0)
	cur_linewidth = 0;

    /* erase by drawing wide, inverted (white) line */
    pw_vector(sw->normalPM, (double)(DEF_IND_SW_WD / 2 + 2),
	      (double)(DEF_IND_SW_HT / 2),
	      (double)(sw->sw_width - 2), (double)(DEF_IND_SW_HT / 2),
	      ERASE, DEF_IND_SW_HT, PANEL_LINE, 0.0, DEFAULT_COLOR);
    /* draw current line thickness into pixmap */
    if (cur_linewidth > 0)	/* don't draw line for zero-thickness */
	pw_vector(sw->normalPM, (double)(DEF_IND_SW_WD / 2 + 2),
		(double)(DEF_IND_SW_HT / 2),
		(double)(sw->sw_width - 2), (double)(DEF_IND_SW_HT / 2),
		PAINT, cur_linewidth, PANEL_LINE, 0.0, DEFAULT_COLOR);

    /*
     * Fool the toolkit by changing the background pixmap to 0 then giving it
     * the modified one again.	Otherwise, it sees that the pixmap ID is not
     * changed and doesn't actually draw it into the widget window
     */
    XtVaSetValues(sw->button,
	 XtNbackgroundPixmap, (XtArgVal) 0,
	 NULL);

    /* put the pixmap in the widget background */
    XtVaSetValues(sw->button,
	 XtNbackgroundPixmap, (XtArgVal) sw->normalPM,
	 NULL);
    put_msg("LINE Thickness = %d", cur_linewidth);
}

/* LINE STYLE		 */

static
void show_linestyle(ind_sw_info *sw)
{
    update_choice_pixmap(sw, cur_linestyle);
    switch (cur_linestyle) {
    case SOLID_LINE:
	cur_styleval = 0.0;
	put_msg("SOLID LINE STYLE (for POLYLINE)");
	break;
    case DASH_LINE:
	cur_styleval = cur_dashlength;
	put_msg("DASH LINE STYLE (for POLYLINE)");
	break;
    case DOTTED_LINE:
	cur_styleval = cur_dotgap;
	put_msg("DOTTED LINE STYLE (for POLYLINE)");
	break;
    }
}

/* GRID MODE	 */
static
void show_gridmode(ind_sw_info *sw)
{
    static int            prev_gridmode = -1;

    update_choice_pixmap(sw, gridmode);
    switch (gridmode) {
    case GRID_0:
	put_msg("No grid");
	break;
    case GRID_1:
	put_msg("Latitude-longitude grid");
	break;
    }
    if (gridmode != prev_gridmode)
	setup_grid();
    prev_gridmode = gridmode;
}

static
void dec_grid(ind_sw_info *sw)
{
    if (gridsize >= 40) {
	gridsize = 40;
	grid_choice = 9;
    } else {
	grid_choice++;
	gridsize = grid_choices[grid_choice];
    }
    show_gridsize(sw);
}

static
void inc_grid(ind_sw_info *sw)
{
    if (gridsize <= 1) {
	gridsize = 1;
	grid_choice = 0;
    } else {
	grid_choice--;
	gridsize = grid_choices[grid_choice];
    }
    show_gridsize(sw);
}

void show_gridsize(ind_sw_info *sw)
{

    if (gridsize < 1)
	gridsize = 1;
    else if (gridsize > 20)
	gridsize = 20;

    put_msg("Grid scale %d", gridsize);
    if (gridsize == old_gridsize)
	return;

    /* write the font size in the background pixmap */
    indbuf[0] = indbuf[1] = indbuf[2] = indbuf[3] = indbuf[4] = '\0';
    sprintf(indbuf, "%.2f", grid_deg[grid_choice]);
    update_string_pixmap(sw, indbuf, sw->sw_width - 24, 14);

    /* fix up the rulers and grid */
    reset_rulers();
    redisplay_rulers();
    setup_grid();
    old_gridsize = gridsize;
}

/* COLOR */

static
void next_color(ind_sw_info *sw)
{
    if (++cur_color >= NUMCOLORS)
	cur_color = DEFAULT_COLOR;
    show_color(sw);
}

static
void prev_color(ind_sw_info *sw)
{
    if (--cur_color < DEFAULT_COLOR)
	cur_color = NUMCOLORS - 1;
    show_color(sw);
}

static
void show_color(ind_sw_info *sw)
{
    int		    color;

    if (cur_color < 0 || cur_color >= NUMCOLORS) {
	cur_color == DEFAULT_COLOR;
	color = x_fg_color.pixel;
    } else
	color = all_colors_available ? appres.color[cur_color] : x_fg_color.pixel;

    put_msg("Color set to %s", colorNames[cur_color + 1]);
    XSetForeground(tool_d, color_gc, color);
    /* now fill the color rectangle with the new color */
    XFillRectangle(tool_d, sw->normalPM, color_gc, sw->sw_width - 29, 4, 26, 24);
    /*
     * write the widget background over old color name before writing new
     * name
     */
    /* first set the foreground color to the background for the fill */
    XSetForeground(tool_d, ind_button_gc, ind_but_bg);
    XFillRectangle(tool_d, sw->normalPM, ind_button_gc, 0, DEF_IND_SW_HT / 2,
		   sw->sw_width - 29, DEF_IND_SW_HT / 2);
    /* now restore the foreground in the gc */
    XSetForeground(tool_d, ind_button_gc, ind_but_fg);
    XDrawImageString(tool_d, sw->normalPM, ind_button_gc, 3, 25,
	      colorNames[cur_color + 1], strlen(colorNames[cur_color + 1]));
    XtVaSetValues(sw->button,
	 XtNbackgroundPixmap, (XtArgVal) 0,
	 NULL);

    /* put the pixmap in the widget background */
    XtVaSetValues(sw->button,
	 XtNbackgroundPixmap, (XtArgVal) sw->normalPM,
	 NULL);
}

/* ZOOM */

static
void inc_zoom(ind_sw_info *sw)
{
    if (display_zoomscale < (float) 0.1) {
        display_zoomscale = (int)(display_zoomscale * 150.0) + 1.0;
        display_zoomscale /= 100.0;
    } else if (display_zoomscale < 1.0) {
        if (display_zoomscale < 0.2)            
            display_zoomscale = 0.2;
        else
            display_zoomscale += 0.2; /* always quantized */
        display_zoomscale = (int)(display_zoomscale*5.0);
        display_zoomscale /= 5.0;
    } else
	display_zoomscale = (int)display_zoomscale + 1.0;
    show_zoom(sw);
}

static
void dec_zoom(ind_sw_info *sw)
{
        /* RER: It was a mistake to make these float instead of double... */
    if (display_zoomscale <= (float) 0.1) {
        display_zoomscale = ((int)(display_zoomscale * 100.0)) - 1.0;
        display_zoomscale /= 100.0;             
    } else if (display_zoomscale < (float) 0.3) {
        display_zoomscale = 0.1; /* always quantized */
    } else if (display_zoomscale <= (float) 1.0) {
        display_zoomscale -= 0.2; /* always quantized */ 
        display_zoomscale = (int)(display_zoomscale*5.0);
        display_zoomscale /= 5.0;
    } else {
        if (display_zoomscale != (int)display_zoomscale)
            display_zoomscale = (int)display_zoomscale;
        else
            display_zoomscale = (int)display_zoomscale - 1.0;
        if (display_zoomscale < (float) 1.0)
                display_zoomscale = 1.0;
    }
    show_zoom(sw);
}

void show_zoom(ind_sw_info *sw)
{
    if (display_zoomscale < 0.1)
	display_zoomscale = 0.1;

    if (display_zoomscale < 0.1)
	put_msg("Zoom scale %.2f", display_zoomscale);
    else        
	put_msg("Zoom scale %.1f", display_zoomscale);
    if (display_zoomscale == old_display_zoomscale)
	return;

    /* write the font size in the background pixmap */
    indbuf[0] = indbuf[1] = indbuf[2] = indbuf[3] = indbuf[4] = '\0';
    if (display_zoomscale == (int) display_zoomscale)
        sprintf(indbuf, "%s%.0f ",
                display_zoomscale<10.0?"  ":" ",display_zoomscale);
    else if (display_zoomscale < (float) 0.1)
    {   
            sprintf(indbuf, "%.2f\n", display_zoomscale);
            indbuf[0] = ' ';
    } 
    else
        sprintf(indbuf, "%s%.1f",
                display_zoomscale<10.0?" ":"",display_zoomscale);
    update_string_pixmap(sw, indbuf, sw->sw_width - 24, 14);

    zoomscale=display_zoomscale/ZOOM_FACTOR;

    /* fix up the rulers and grid */
    reset_rulers();
    redisplay_rulers();
    redisplay_canvas();
    old_display_zoomscale = display_zoomscale;
}
