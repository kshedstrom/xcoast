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
#include "figx.h"
#include "resources.h"
#include "mode.h"
#include "w_canvas.h"		/* for null_proc() */
#include "w_drawprim.h"
#include "w_mousefun.h"
#include "w_util.h"
#include "w_setup.h"

extern void	erase_objecthighlight(void);
extern int	emptyfigure(void);
extern int	do_save(Widget w);
extern void	undo(void), redisplay_canvas(void);
extern void	popup_file_panel(Widget w);
extern void	popup_proj_panel(void);

void		init_cmd_panel(Widget tool);
void		setup_cmd_panel(void);

/* internal features and definitions */

#define cmd_action(z)		(z->cmd_func)(z->widget)
#define quick_action(z)		(z->quick_func)(z->widget)
#define shift_quick_action(z)	(z->shift_quick_func)(z->widget)

/* prototypes */
static void	sel_cmd_but(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch);
static void	enter_cmd_but(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch);
void		quit(Widget w);
void		delete_all_cmd(void);

/* cmd panel definitions */
#define CMD_LABEL_LEN 16
typedef struct cmd_switch_struct {
    char          label[CMD_LABEL_LEN];       /* label on the button */
    char          cmd_name[CMD_LABEL_LEN];    /* command name for resources */
    void          (*cmd_func) ();             /* mouse button 1 func */
    void	  (*quick_func) ();           /* mouse button 3 func */
    void	  (*shift_quick_func) ();     /* shift-mouse button 3 func */
    char          mousefun_l[CMD_LABEL_LEN];  /* label for mouse 1 func */
    char          mousefun_r[CMD_LABEL_LEN];  /* label for mouse 3 func */
    Widget        widget;                     /* widget */
}             cmd_sw_info;

/* command panel of switches below the lower ruler */
cmd_sw_info cmd_switches[] = {
    {"Quit", "quit", quit, null_proc, null_proc, "Quit", ""},
    {"Delete ALL", "delete_all", delete_all_cmd, null_proc,
                          null_proc, "Delete All", ""},
    {"Undo", "undo", undo, null_proc, null_proc, "Undo", ""},
    {"Redraw", "redraw", redisplay_canvas, null_proc,
			  null_proc, "Redraw", ""},
    {"Map proj...", "mapproj", popup_proj_panel, null_proc,
			  null_proc, "Popup", ""},
    {"File...", "file", popup_file_panel, do_save, null_proc,
			  "Popup", "Save Shortcut"},
};

#define		NUM_CMD_SW  (sizeof(cmd_switches) / sizeof(cmd_sw_info))

static XtActionsRec cmd_actions[] =
{
    {"LeaveCmdSw", (XtActionProc) clear_mousefun},
    {"quit", (XtActionProc) quit},
    {"delete_all", (XtActionProc) delete_all_cmd},
    {"undo", (XtActionProc) undo},
    {"redraw", (XtActionProc) redisplay_canvas},
    {"mapproj", (XtActionProc) popup_proj_panel},
    {"file", (XtActionProc) popup_file_panel},
};

static String	cmd_translations =
"<Btn1Down>:set()\n\
    <Btn1Up>:unset()\n\
    <LeaveWindow>:LeaveCmdSw()reset()\n";

DeclareStaticArgs(11);


extern void put_msg (const char*, ...);
extern int popup_query (int query_type, char *message);
extern void free_GCs (void);
extern void delete_all (void);

int
num_cmd_sw(void)
{
    return (NUM_CMD_SW);
}

/* command panel */
void
init_cmd_panel(Widget tool)
{
    register int    i;
    register cmd_sw_info *sw;
    Widget	    beside = NULL;

    FirstArg(XtNborderWidth, 0);
    NextArg(XtNdefaultDistance, 0);
    NextArg(XtNhorizDistance, 0);
    NextArg(XtNvertDistance, 0);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    cmd_panel = XtCreateWidget("commands", formWidgetClass, tool,
			       Args, ArgCount);
    XtAppAddActions(tool_app, cmd_actions, XtNumber(cmd_actions));

    FirstArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNfont, button_font);
    NextArg(XtNheight, CMDPANEL_HT - 2 * INTERNAL_BW);
    NextArg(XtNwidth, CMDPANEL_WD / NUM_CMD_SW - INTERNAL_BW);
    NextArg(XtNvertDistance, 0);
    NextArg(XtNhorizDistance, 0);
    for (i = 0; i < NUM_CMD_SW; ++i) {
	sw = &cmd_switches[i];
	NextArg(XtNlabel, sw->label);
	NextArg(XtNfromHoriz, beside);
	sw->widget = XtCreateManagedWidget(sw->cmd_name, commandWidgetClass,
					   cmd_panel, Args, ArgCount);
	/* setup callback and default actions */
	XtAddEventHandler(sw->widget, ButtonReleaseMask, (Boolean) 0,
			  sel_cmd_but, (XtPointer) sw);
	XtAddEventHandler(sw->widget, EnterWindowMask, (Boolean) 0,
			  enter_cmd_but, (XtPointer) sw);
	XtOverrideTranslations(sw->widget,
			       XtParseTranslationTable(cmd_translations));
	ArgCount -= 3;
	NextArg(XtNhorizDistance, -INTERNAL_BW);
	beside = sw->widget;
    }
}

void
setup_cmd_panel(void)
{
    register int    i;
    register cmd_sw_info *sw;

    XDefineCursor(tool_d, XtWindow(cmd_panel), arrow_cursor);

    for (i = 0; i < NUM_CMD_SW; ++i) {
	sw = &cmd_switches[i];
	FirstArg(XtNfont, button_font); /* label font */
	SetValues(sw->widget);
	/* install the keyboard accelerators for this command button 
	   from the resources */
	/*XtInstallAllAccelerators(sw->widget, tool);*/
    }
}

/* ARGSUSED */
static void
enter_cmd_but(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch)
{
    cmd_sw_info *sw = (cmd_sw_info *) closure;
    clear_mousefun();
    draw_mousefun(sw->mousefun_l, "", sw->mousefun_r);
}

/* ARGSUSED */
static void
sel_cmd_but(Widget widget, XtPointer closure, XEvent *event, Boolean *continue_to_dispatch)
{
    cmd_sw_info *sw = (cmd_sw_info *) closure;
    XButtonEvent button;
    
    button = event->xbutton;

    if ((button.button == Button2) ||
	(button.button == Button3 && button.state & Mod1Mask))
	    return;

    if (action_on) {
	put_msg("FINISH drawing current object first");
	return;
    } else if (highlighting)
	erase_objecthighlight();

    if (button.button == Button1)
	cmd_action(sw);
    else if (button.state & ShiftMask)
	shift_quick_action(sw);
    else
	quick_action(sw);
}

static char	quit_msg[] = "The current figure is modified.\nDo you want to save it before quitting?";

void
quit(Widget w)
{
    int		    qresult;

    if (!emptyfigure() && figure_modified && !aborting) {
	XtSetSensitive(w, False);
	if ((qresult = popup_query(QUERY_YESNO, quit_msg)) == RESULT_CANCEL) {
	    extern int popup_query (int query_type, char *message);
	    XtSetSensitive(w, True);
	    return;
	} else if (qresult == RESULT_YES) {
	    (void)do_save(w);
	    /*
	     * if saving was not successful, figure_modified is still true:
	     * do not quit!
	     */
	    if (figure_modified) {
		XtSetSensitive(w, True);
		return;
	    }
	}
    }

    /* free all the GC's */
    free_GCs();
    XtDestroyWidget(tool);
    exit(0);
}

void
delete_all_cmd(void)
{
    if (emptyfigure()) {
	put_msg("Figure already empty");
	return;
    }
    delete_all();
    put_msg("Immediate Undo will restore the figure");
    redisplay_canvas();
}
