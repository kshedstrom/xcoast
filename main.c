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
#include "version.h"
#include "patchlevel.h"
#include "resources.h"
#include "object.h"
#include "mode.h"
#include "u_fonts.h"
#include "w_cursor.h"
#include "w_drawprim.h"
#include "w_mousefun.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"
#include <stdlib.h>     /* for getenv */

/************** EXTERNAL functions **************/

extern void	quit(Widget w), undo(void), redisplay_canvas(void), delete_all_cmd(void);
extern void	popup_file_panel(Widget w), popup_proj_panel(void);
extern void	do_load(Widget w, XButtonEvent *ev);
extern void	do_save(Widget w);

extern void	setup_cmd_panel(void);
extern void	X_error_handler();
extern void	error_handler();
extern void	my_quit();
extern void	resize_mousefun(void);
extern int	ignore_exp_cnt;

char		data_file[200];

#include "xcoast.icon.X"
Pixmap		xcoast_icon;

static char	tool_name[100];

/************** FIG options ******************/

static char    *filename = NULL;
static char    *datafile = NULL;

static Boolean	true = True;
static Boolean	false = False;
static int	zero = 0;

/* actions so that we may install accelerators at the top level */
static XtActionsRec   main_actions[] =
{
    {"Quit", (XtActionProc) quit},
    {"Delete_all", (XtActionProc) delete_all_cmd},
    {"Undo", (XtActionProc) undo},
    {"Redraw", (XtActionProc) redisplay_canvas},
    {"File", (XtActionProc) popup_file_panel},
	{"LoadFile", (XtActionProc) do_load},
	{"SaveFile", (XtActionProc) do_save},
    {"Projection", (XtActionProc) popup_proj_panel},
};

static XtResource application_resources[] = {
    {"iconGeometry",  "IconGeometry",  XtRString,  sizeof(char *),
    XtOffset(appresPtr,iconGeometry), XtRString, (caddr_t) NULL},
    {XtNjustify, XtCJustify, XtRBoolean, sizeof(Boolean),
    XtOffset(appresPtr, RHS_PANEL), XtRBoolean, (caddr_t) & false},
    {"debug", "Debug", XtRBoolean, sizeof(Boolean),
    XtOffset(appresPtr, DEBUG), XtRBoolean, (caddr_t) & false},
    {"pwidth", XtCWidth, XtRFloat, sizeof(float),
    XtOffset(appresPtr, tmp_width), XtRInt, (caddr_t) & zero},
    {"pheight", XtCHeight, XtRFloat, sizeof(float),
    XtOffset(appresPtr, tmp_height), XtRInt, (caddr_t) & zero},
    {XtNreverseVideo, XtCReverseVideo, XtRBoolean, sizeof(Boolean),
    XtOffset(appresPtr, INVERSE), XtRBoolean, (caddr_t) & false},
    {"trackCursor", "Track", XtRBoolean, sizeof(Boolean),
    XtOffset(appresPtr, TRACKING), XtRBoolean, (caddr_t) & true},
    {"boldFont", "Font", XtRString, sizeof(char *),
    XtOffset(appresPtr, boldFont), XtRString, (caddr_t) NULL},
    {"normalFont", "Font", XtRString, sizeof(char *),
    XtOffset(appresPtr, normalFont), XtRString, (caddr_t) NULL},
    {"buttonFont", "Font", XtRString, sizeof(char *),
    XtOffset(appresPtr, buttonFont), XtRString, (caddr_t) NULL},
    {"internalborderwidth", "InternalBorderWidth", XtRInt, sizeof(int),
    XtOffset(appresPtr, internalborderwidth), XtRInt, (caddr_t) & zero},
    {"scalablefonts", "ScalableFonts", XtRBoolean, sizeof(Boolean),
    XtOffset(appresPtr, SCALABLEFONTS), XtRBoolean, (caddr_t) & false},
    {"color0", "Color0", XtRPixel, sizeof(Pixel),
    XtOffset(appresPtr, color[0]), XtRString, (caddr_t) "black"},
    {"color1", "Color1", XtRPixel, sizeof(Pixel),
    XtOffset(appresPtr, color[1]), XtRString, (caddr_t) "blue"},
    {"color2", "Color2", XtRPixel, sizeof(Pixel),
    XtOffset(appresPtr, color[2]), XtRString, (caddr_t) "green"},
    {"color3", "Color3", XtRPixel, sizeof(Pixel),
    XtOffset(appresPtr, color[3]), XtRString, (caddr_t) "cyan"},
    {"color4", "Color4", XtRPixel, sizeof(Pixel),
    XtOffset(appresPtr, color[4]), XtRString, (caddr_t) "red"},
    {"color5", "Color5", XtRPixel, sizeof(Pixel),
    XtOffset(appresPtr, color[5]), XtRString, (caddr_t) "magenta"},
    {"color6", "Color6", XtRPixel, sizeof(Pixel),
    XtOffset(appresPtr, color[6]), XtRString, (caddr_t) "yellow"},
    {"color7", "Color7", XtRPixel, sizeof(Pixel),
    XtOffset(appresPtr, color[7]), XtRString, (caddr_t) "white"},
    {"monochrome", "Monochrome", XtRBoolean, sizeof(Boolean),
    XtOffset(appresPtr, monochrome), XtRBoolean, (caddr_t) & false},
};

static XrmOptionDescRec options[] =
{
    {"-iconGeometry", ".iconGeometry", XrmoptionSepArg, (caddr_t) NULL},
    {"-right", ".justify", XrmoptionNoArg, "True"},
    {"-left", ".justify", XrmoptionNoArg, "False"},
    {"-debug", ".debug", XrmoptionNoArg, "True"},
    {"-pwidth", ".pwidth", XrmoptionSepArg, 0},
    {"-pheight", ".pheight", XrmoptionSepArg, 0},
    {"-inverse", ".reverseVideo", XrmoptionNoArg, "True"},
    {"-notrack", ".trackCursor", XrmoptionNoArg, "False"},
    {"-track", ".trackCursor", XrmoptionNoArg, "True"},
    {"-boldFont", ".boldFont", XrmoptionSepArg, 0},
    {"-normalFont", ".normalFont", XrmoptionSepArg, 0},
    {"-buttonFont", ".buttonFont", XrmoptionSepArg, 0},
    {"-scalablefonts", ".scalablefonts", XrmoptionNoArg, "True"},
    {"-noscalablefonts", ".scalablefonts", XrmoptionNoArg, "False"},
    {"-monochrome", ".monochrome", XrmoptionNoArg, "True"},
    {"-internalBW", ".internalborderwidth", XrmoptionSepArg, 0},
    {"-internalBorderWidth", ".internalborderwidth", XrmoptionSepArg, 0},
};

Atom wm_delete_window;

static XtCallbackRec callbacks[] =
{
    {NULL, NULL},
};

static Arg	form_args[] =
{
    {XtNcallback, (XtArgVal) callbacks},
    {XtNinput, (XtArgVal) True},
    {XtNdefaultDistance, (XtArgVal) 0},
    {XtNresizable, (XtArgVal) False},
};

static void	check_for_resize(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams);
static void	check_colors(void);
XtActionsRec	form_actions[] =
{
    {"ResizeForm", (XtActionProc) check_for_resize},
    {"Quit", (XtActionProc) my_quit},
};

extern void clear_text_key(Widget w);
static XtActionsRec text_panel_actions[] =
{
    {"EmptyTextKey", (XtActionProc) clear_text_key} ,
};

static String	form_translations =
			"<ConfigureNotify>:ResizeForm()\n";
static String	tool_translations =
			"<Message>WM_PROTOCOLS:Quit()\n";

#define NCHILDREN	9
static TOOL	form;


extern void fix_converters (void);
extern void init_font (void);
extern void setup_sizes (int new_canv_wd, int new_canv_ht);
extern void init_cmd_panel (Widget tool);
extern void init_msg (TOOL tool, char *filename);
extern void init_mode_panel (TOOL tool);
extern void init_topruler (TOOL tool);
extern void init_canvas (TOOL tool);
extern void init_unitbox (TOOL tool);
extern void init_proj (void);
extern void init_sideruler (TOOL tool);
extern void init_ind_panel (TOOL tool);
extern void init_gc (void);
extern void setup_msg (void);
extern void setup_canvas (void);
extern void setup_rulers (void);
extern void setup_mode_panel (void);
extern void setup_ind_panel (void);
extern void get_directory (char *direct);
extern void put_msg (const char*, ...);
extern void load_file (char *file);
extern void update_cur_filename (char *newname);
extern void app_flush (void);
extern void resize_topruler (void);
extern void resize_sideruler (void);

int main(int argc, char **argv)
{
    TOOL	    children[NCHILDREN];
    int		    ichild;
    int		    init_canv_wd, init_canv_ht;
    XWMHints	   *wmhints;
    char	    i;
    Dimension	    w, h;

    DeclareArgs(5);

    /* get the TMPDIR environment variable for temporary files */
    if ((TMPDIR = getenv("XFIGTMPDIR"))==NULL)
      TMPDIR = "/tmp";

    (void) sprintf(tool_name, " XCOAST %s(.%s)  (Protocol: %s)",
		   XCOAST_VERSION, PATCHLEVEL, PROTOCOL_VERSION);
    (void) strcat(file_header, PROTOCOL_VERSION);
    tool = XtAppInitialize(&tool_app, (String) "XCoast",
			   (XrmOptionDescList) options,
			   (Cardinal) XtNumber(options),
#if XtSpecificationRelease < 5
			   (Cardinal *) & argc,
			   (String *) argv,
#else
			   &argc,
			   argv,
#endif
			   (String *) NULL,
#if XtSpecificationRelease < 5
			   (String *) NULL, 
#else
			   (ArgList) NULL,
#endif
			   (Cardinal) 0);


    /* install actions to get to the functions with accelerators */
    XtAppAddActions(tool_app, main_actions, XtNumber(main_actions));

    fix_converters();
    XtGetApplicationResources(tool, &appres, application_resources,
			      XtNumber(application_resources), NULL, 0);

    datafile = getenv("XCOASTDATA");
    for (i=1; i < argc; i++) {
	if (*argv[i] != '-') {	/* search for non - name */
	    filename = argv[i];
	    break;
	} else {
            if (strcmp(argv[i],"-data") ==0) {
                i++;
                datafile = argv[i];
            }
        }
    }
    if (datafile != NULL) 
        strcpy(data_file,datafile);
    else
        strcpy(data_file,"coast0.lines");
    tool_d = XtDisplay(tool);
    tool_s = XtScreen(tool);
    tool_sn = DefaultScreen(tool_d);

    if (appres.iconGeometry != (char *) 0) {
        int scr, x, y, junk;

        for(scr = 0;
            tool_s != ScreenOfDisplay(tool_d, scr);
            scr++);

        XGeometry(tool_d, scr, appres.iconGeometry,
                  "", 0, 0, 0, 0, 0, &x, &y, &junk, &junk);
        XtVaSetValues(tool,
	       XtNiconX, x,
	       XtNiconY, y,
	       NULL);
    }

    ZOOM_FACTOR = PIX_PER_CM/DISPLAY_PIX_PER_CM;
    display_zoomscale = 1.0;
    zoomscale=display_zoomscale/ZOOM_FACTOR;

    if (CellsOfScreen(tool_s) == 2 && appres.INVERSE) {
	XrmValue	value;
	XrmDatabase	newdb = (XrmDatabase) 0, old;

	value.size = sizeof("White");
	value.addr = "White";
	XrmPutResource(&newdb, "xfig*borderColor", "String",
		       &value);
	value.size = sizeof("White");
	value.addr = "White";
	XrmPutResource(&newdb, "xfig*foreground", "String",
		       &value);
	value.size = sizeof("Black");
	value.addr = "Black";
	XrmPutResource(&newdb, "xfig*background", "String",
		       &value);
	old = XtDatabase(tool_d);
	XrmMergeDatabases(newdb, &old);

	/* now set the tool part, since its already created */
	FirstArg(XtNborderColor, WhitePixelOfScreen(tool_s));
	NextArg(XtNforeground, WhitePixelOfScreen(tool_s));
	NextArg(XtNbackground, BlackPixelOfScreen(tool_s));
	SetValues(tool);
    }
    init_font();

    gc = DefaultGC(tool_d, tool_sn);
    bold_gc = DefaultGC(tool_d, tool_sn);
    button_gc = DefaultGC(tool_d, tool_sn);

    /* set the roman and bold fonts for the message windows */
    XSetFont(tool_d, gc, roman_font->fid);
    XSetFont(tool_d, bold_gc, bold_font->fid);
    XSetFont(tool_d, button_gc, button_font->fid);

    /*
     * check if the NUMCOLORS drawing colors could be allocated and have
     * different palette entries
     */
    check_colors();

    init_cursor();
    form = XtCreateManagedWidget("form", formWidgetClass, tool,
				 form_args, XtNumber(form_args));

    if (INTERNAL_BW == 0)
	INTERNAL_BW = (int) appres.internalborderwidth;
    if (INTERNAL_BW <= 0)
	INTERNAL_BW = DEF_INTERNAL_BW;

    SW_PER_ROW = SW_PER_ROW_PORT;
    SW_PER_COL = SW_PER_COL_PORT;
    init_canv_wd = appres.tmp_width * PIX_PER_CM/ZOOM_FACTOR;
    init_canv_ht = appres.tmp_height * PIX_PER_CM/ZOOM_FACTOR;

    if (init_canv_wd == 0)
	init_canv_wd = DEF_CANVAS_SIZE;

    if (init_canv_ht == 0)
	init_canv_ht = DEF_CANVAS_SIZE;

    if ((init_canv_ht < DEF_CANVAS_SIZE) ||
	(HeightOfScreen(tool_s) < DEF_CANVAS_SIZE)) {
	SW_PER_ROW = SW_PER_ROW_LAND;
	SW_PER_COL = SW_PER_COL_LAND;
    }
    setup_sizes(init_canv_wd, init_canv_ht);
    init_cmd_panel(form);
    init_msg(form,filename);
    init_mousefun(form);
    init_mode_panel(form);
    init_topruler(form);
    init_canvas(form);
    init_unitbox(form);
    init_proj();
    init_sideruler(form);
    init_ind_panel(form);

    ichild = 0;
    children[ichild++] = cmd_panel;	/* command buttons */
    children[ichild++] = mousefun;	/* labels for mouse fns */
    children[ichild++] = msg_form;	/* message window form */
    children[ichild++] = mode_panel;	/* current mode */
    children[ichild++] = topruler_sw;	/* top ruler */
    children[ichild++] = unitbox_sw;	/* box containing units */
    children[ichild++] = sideruler_sw;	/* side ruler */
    children[ichild++] = canvas_sw;	/* main drawing canvas */
    children[ichild++] = ind_viewp;	/* current settings indicators */

    /*
     * until the following XtRealizeWidget() is called, there are NO windows
     * in existence
     */

    XtManageChildren(children, NCHILDREN);
    XtRealizeWidget(tool);

    wm_delete_window = XInternAtom(XtDisplay(tool), "WM_DELETE_WINDOW", False);
    (void) XSetWMProtocols(XtDisplay(tool), XtWindow(tool),
                         &wm_delete_window, 1);

    xcoast_icon = XCreateBitmapFromData(tool_d, XtWindow(tool),
		    (char *) xcoast_bits, xcoast_width, xcoast_height);

    FirstArg(XtNtitle, tool_name);
    NextArg(XtNiconPixmap, xcoast_icon);
    SetValues(tool);
    /* Set the input field to true to allow keyboard input */
    wmhints = XGetWMHints(tool_d, XtWindow(tool));
    wmhints->flags |= InputHint;/* add in input hint */
    wmhints->input = True;
    XSetWMHints(tool_d, XtWindow(tool), wmhints);
    XFree((char *) wmhints);

    if (appres.RHS_PANEL) {	/* side button panel is on right size */
	FirstArg(XtNfromHoriz, 0);
	NextArg(XtNhorizDistance, SIDERULER_WD + INTERNAL_BW);
	SetValues(topruler_sw);

	FirstArg(XtNfromHoriz, 0);
	NextArg(XtNhorizDistance, 0);
	NextArg(XtNfromVert, topruler_sw);
	NextArg(XtNleft, XtChainLeft);	/* chain to left of form */
	NextArg(XtNright, XtChainLeft);
	SetValues(sideruler_sw);

	FirstArg(XtNfromHoriz, 0);
	NextArg(XtNhorizDistance, 0);
	NextArg(XtNfromVert, msg_form);
	NextArg(XtNleft, XtChainLeft);	/* chain to left of form */
	NextArg(XtNright, XtChainLeft);
	SetValues(unitbox_sw);

	/* relocate the side button panel */
	XtUnmanageChild(mode_panel);
	XtUnmanageChild(canvas_sw);
	FirstArg(XtNfromHoriz, canvas_sw);	/* panel right of canvas */
	NextArg(XtNhorizDistance, -INTERNAL_BW);
	NextArg(XtNfromVert, mousefun);
	NextArg(XtNleft, XtChainRight);
	NextArg(XtNright, XtChainRight);
	SetValues(mode_panel);
	FirstArg(XtNfromHoriz, sideruler_sw);	/* panel right of canvas */
	SetValues(canvas_sw);
	XtManageChild(canvas_sw);
	XtManageChild(mode_panel);
    }

    init_gc();
    setup_cmd_panel();
    setup_msg();
    setup_canvas();
    setup_rulers();
    setup_mode_panel();
    setup_mousefun();
    setup_ind_panel();
    get_directory(cur_dir);

    /* install the accelerators - cmd_panel, ind_panel and mode_panel
	accelerators are installed in their respective setup_xxx procedures */
    XtInstallAllAccelerators(canvas_sw, tool);
    XtInstallAllAccelerators(mousefun, tool);
    XtInstallAllAccelerators(msg_form, tool);
    XtInstallAllAccelerators(topruler_sw, tool);
    XtInstallAllAccelerators(sideruler_sw, tool);
    XtInstallAllAccelerators(unitbox_sw, tool);

    FirstArg(XtNwidth, &w);
    NextArg(XtNheight, &h);
    GetValues(tool);
    TOOL_WD = (int) w;
    TOOL_HT = (int) h;
    XtAppAddActions(tool_app, form_actions, XtNumber(form_actions));
    XtAppAddActions(tool_app, text_panel_actions, XtNumber(text_panel_actions));
    XtOverrideTranslations(tool, XtParseTranslationTable(tool_translations));
    XtOverrideTranslations(form, XtParseTranslationTable(form_translations));

    XSetErrorHandler(X_error_handler);
    XSetIOErrorHandler((XIOErrorHandler) X_error_handler);
    (void) signal(SIGHUP, error_handler);
    (void) signal(SIGFPE, error_handler);
#ifndef NO_SIBGUS
    (void) signal(SIGBUS, error_handler);
#endif
    (void) signal(SIGSEGV, error_handler);
    (void) signal(SIGINT, SIG_IGN);	/* in case user accidentally types
					 * ctrl-c */

    put_msg("READY, please select a mode or load a file");

    /*
     * decide on filename for cut buffer: first try users HOME directory to
     * allow cutting and pasting between sessions, if this fails create
     * unique filename in /tmp dir
     */

    if (filename == NULL)
	strcpy(cur_filename, DEF_NAME);
    else
	load_file(filename);
    update_cur_filename(cur_filename);

    app_flush();

    XtAppMainLoop(tool_app);
    return(0);
}

/* ARGSUSED */
static void
check_for_resize(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams)
{
    XConfigureEvent *xc = (XConfigureEvent *) event;
    Dimension	    b;
    int		    dx, dy;

    DeclareArgs(3);

    if (xc->width == TOOL_WD && xc->height == TOOL_HT)
	return;			/* no size change */
    dx = xc->width - TOOL_WD;
    dy = xc->height - TOOL_HT;
    TOOL_WD = xc->width;
    TOOL_HT = xc->height;
    setup_sizes(CANVAS_WD + dx, CANVAS_HT + dy);

    XawFormDoLayout(form, False);
    ignore_exp_cnt++;		/* canvas is resized twice - redraw only once */

    FirstArg(XtNborderWidth, &b);
    /* first redo the top panels */
    GetValues(cmd_panel);
    XtResizeWidget(cmd_panel, CMDPANEL_WD, CMDPANEL_HT, b);
    GetValues(mousefun);
    XtResizeWidget(mousefun, MOUSEFUN_WD, MOUSEFUN_HT, b);
    XtUnmanageChild(mousefun);
    resize_mousefun();
    XtManageChild(mousefun);	/* so that it shifts with msg_panel */
    /* resize the message form by setting the current filename */
    update_cur_filename(cur_filename);

    /* now redo the center area */
    XtUnmanageChild(mode_panel);
    FirstArg(XtNheight, (MODEPANEL_SPACE + 1) / 2);
    SetValues(d_label);
    FirstArg(XtNheight, (MODEPANEL_SPACE) / 2);
    SetValues(e_label);
    XtManageChild(mode_panel);	/* so that it adjusts properly */

    FirstArg(XtNborderWidth, &b);
    GetValues(canvas_sw);
    XtResizeWidget(canvas_sw, CANVAS_WD, CANVAS_HT, b);
    GetValues(topruler_sw);
    XtResizeWidget(topruler_sw, TOPRULER_WD, TOPRULER_HT, b);
    resize_topruler();
    GetValues(sideruler_sw);
    XtResizeWidget(sideruler_sw, SIDERULER_WD, SIDERULER_HT, b);
    resize_sideruler();
    XtUnmanageChild(sideruler_sw);
    XtManageChild(sideruler_sw);/* so that it shifts with canvas */
    XtUnmanageChild(unitbox_sw);
    XtManageChild(unitbox_sw);	/* so that it shifts with canvas */

    XawFormDoLayout(form, True);
}


static void
check_colors(void)
{
    int		    i, j;

    /* if monochrome resource is set, do not even check for colors */
    if (appres.monochrome) {
	all_colors_available = false;
	return;
    }
    all_colors_available = true;

    /* check if the drawing colors have different palette entries */
    for (i = 0; i < NUMCOLORS - 1; i++) {
	for (j = i + 1; j < NUMCOLORS; j++) {
	    if (appres.color[i] == appres.color[j]) {
		all_colors_available = false;
		break;
	    }
	}
	if (!all_colors_available)
	    break;
    }
}

/* useful when using ups */
void XSyncOn(void)
{
	XSynchronize(tool_d, True);
	XFlush(tool_d);
}

void XSyncOff(void)
{
	XSynchronize(tool_d, False);
	XFlush(tool_d);
}

#ifdef NOSTRSTR

char *strstr(s1, s2)
    char *s1, *s2;
{
    int len2;
    char *stmp;

    len2 = strlen(s2);
    for (stmp = s1; *stmp != NULL; stmp++)
	if (strncmp(stmp, s2, len2)==0)
	    return stmp;
    return NULL;
}
#endif
 
#ifdef NOSTRTOL
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strtol.c	5.3 (Berkeley) 5/17/90";
#endif /* LIBC_SCCS and not lint */

#include <ctype.h>
#include <errno.h>

#define	ULONG_MAX	0xffffffff	/* max value for an unsigned long */
#define	LONG_MAX	0x7fffffff	/* max value for a long */
#define	LONG_MIN	0x80000000	/* min value for a long */

/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long
strtol(nptr, endptr, base)
	char *nptr, **endptr;
	register int base;
{
	register char *s = nptr;
	register unsigned long acc;
	register int c;
	register unsigned long cutoff;
	register int neg = 0, any, cutlim;

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	do {
		c = *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10,
	 * cutoff will be set to 214748364 and cutlim to either
	 * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	 * a value > 214748364, or equal but the next digit is > 7 (or 8),
	 * the number is too big, and we will return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
	cutlim = cutoff % (unsigned long)base;
	cutoff /= (unsigned long)base;
	for (acc = 0, any = 0; c = *s++) {
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LONG_MIN : LONG_MAX;
		errno = ERANGE;
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = any ? s - 1 : nptr;
	return (acc);
}
#endif
