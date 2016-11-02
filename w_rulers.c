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
#include "paintop.h"
#include "w_drawprim.h"
#include "w_mousefun.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"

/*
 * The following will create rulers the same size as the initial screen size.
 * if the user resizes the xfig window, the rulers won't have numbers there.
 * Should really reset the sizes if the screen resizes.
 */

/*
 * set maximum ruler size:
 * color servers for Vaxstations screw up the pixmaps if the rulers are
 * made too big (they can't handle pixmaps much larger than the screen)
 */

#define			INCH_MARK		8
#define			HALF_MARK		8
#define			QUARTER_MARK		6
#define			SIXTEENTH_MARK		4

#define			TRM_WID			16
#define			TRM_HT			8
#define			SRM_WID			8
#define			SRM_HT			16

extern	void	pan_origin(void);

static int	lasty = -100, lastx = -100;
static int	troffx = -8, troffy = -10;
static int	orig_zoomoff;
static int	last_drag_x, last_drag_y;
static unsigned char	tr_marker_image[16] = {
    0xFE, 0xFF,		/* ***************  */
    0x04, 0x40,		/*  *           *  */
    0x08, 0x20,		/*   *         *  */
    0x10, 0x10,		/*    *       *  */
    0x20, 0x08,		/*     *     *  */
    0x40, 0x04,		/*      *   *  */
    0x80, 0x02,		/*       * *  */
    0x00, 0x01,		/*        *  */
};
static		mpr_static(trm_pr, TRM_WID, TRM_HT, 1, tr_marker_image);
static int	srroffx = 2, srroffy = -7;
static unsigned char	srr_marker_image[16] = {
    0x80,		/*        *  */
    0xC0,		/*       **  */
    0xA0,		/*      * *  */
    0x90,		/*     *  *  */
    0x88,		/*    *   *  */
    0x84,		/*   *    *  */
    0x82,		/*  *     *  */
    0x81,		/* *      *  */
    0x82,		/*  *     *  */
    0x84,		/*   *    *  */
    0x88,		/*    *   *  */
    0x90,		/*     *  *  */
    0xA0,		/*      * *  */
    0xC0,		/*       **  */
    0x80,		/*        *  */
    0x00
};
static		mpr_static(srrm_pr, SRM_WID, SRM_HT, 1, srr_marker_image);

static int	srloffx = -10, srloffy = -7;
static unsigned char	srl_marker_image[16] = {
    0x01,		/* *	      */
    0x03,		/* **	      */
    0x05,		/* * *	      */
    0x09,		/* *  *	      */
    0x11,		/* *   *      */
    0x21,		/* *    *     */
    0x41,		/* *     *    */
    0x81,		/* *      *   */
    0x41,		/* *     *    */
    0x21,		/* *    *     */
    0x11,		/* *   *      */
    0x09,		/* *  *	      */
    0x05,		/* * *	      */
    0x03,		/* **	      */
    0x01,		/* *	      */
    0x00
};
static		mpr_static(srlm_pr, SRM_WID, SRM_HT, 1, srl_marker_image);

static Pixmap	toparrow_pm = 0, sidearrow_pm = 0;
static Pixmap	topruler_pm = 0, sideruler_pm = 0;

DeclareStaticArgs(14);

static void	topruler_selected(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams);
static void	topruler_exposed(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams);
static void	sideruler_selected(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams);
static void	sideruler_exposed(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams);


void redisplay_topruler(void);
void redisplay_sideruler(void);
void redisplay_canvas(void);
void setup_topruler(void);
void setup_sideruler(void);
void reset_topruler(void);
void reset_sideruler(void);
void set_siderulermark(double y);
void set_toprulermark(double x);
void erase_siderulermark(void);
void erase_toprulermark(void);
extern void pan_left(void);
extern void pan_right(void);
extern void pan_up(void);
extern void pan_down(void);

void redisplay_rulers(void)
{
    redisplay_topruler();
    redisplay_sideruler();
}

void setup_rulers(void)
{
    setup_topruler();
    setup_sideruler();
}

void reset_rulers(void)
{
    reset_topruler();
    reset_sideruler();
}

void set_rulermark(double x, double y)
{
    if (appres.TRACKING) {
	set_siderulermark(y);
	set_toprulermark(x);
    }
}

void erase_rulermark(void)
{
    if (appres.TRACKING) {
	erase_siderulermark();
	erase_toprulermark();
    }
}

static int      TWOMM = (PIX_PER_CM / 5);
static int      ONEMM = (PIX_PER_CM / 10);

/************************* UNITBOX ************************/

XtActionsRec	unitbox_actions[] =
{
    {"EnterUnitBox", (XtActionProc) draw_mousefun_unitbox},
    {"LeaveUnitBox", (XtActionProc) clear_mousefun},
    {"HomeRulers", (XtActionProc) pan_origin},
};

static String	unitbox_translations =
"<EnterWindow>:EnterUnitBox()\n\
    <LeaveWindow>:LeaveUnitBox()\n\
    <Btn1Down>:HomeRulers()\n";

void
init_unitbox(TOOL tool)
{

    unitbox_sw = XtVaCreateWidget("unitbox",
		labelWidgetClass, tool,
    		XtNwidth, SIDERULER_WD,
    		XtNheight, RULER_WD,
    		XtNfont, button_font,
		XtNlabel, "home",
    		XtNfromHoriz, canvas_sw,
    		XtNhorizDistance, -INTERNAL_BW,
    		XtNfromVert, msg_form,
    		XtNvertDistance, -INTERNAL_BW,
    		XtNresizable, False,
    		XtNtop, XtChainTop,
    		XtNbottom, XtChainTop,
    		XtNleft, XtChainLeft,
    		XtNright, XtChainLeft,
    		XtNborderWidth, INTERNAL_BW,
		NULL);
    XtAppAddActions(tool_app, unitbox_actions, XtNumber(unitbox_actions));
    XtOverrideTranslations(unitbox_sw,
			   XtParseTranslationTable(unitbox_translations));
    return;
}

/************************* TOPRULER ************************/

XtActionsRec	topruler_actions[] =
{
    {"EventTopRuler", (XtActionProc) topruler_selected},
    {"ExposeTopRuler", (XtActionProc) topruler_exposed},
    {"EnterTopRuler", (XtActionProc) draw_mousefun_topruler},
    {"LeaveTopRuler", (XtActionProc) clear_mousefun},
};

static String	topruler_translations =
"Any<BtnDown>:EventTopRuler()\n\
    Any<BtnUp>:EventTopRuler()\n\
    <Btn2Motion>:EventTopRuler()\n\
    Meta <Btn3Motion>:EventTopRuler()\n\
    <EnterWindow>:EnterTopRuler()\n\
    <LeaveWindow>:LeaveTopRuler()\n\
    <Expose>:ExposeTopRuler()\n";

/* ARGSUSED */
static void
topruler_selected(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams)
{
    XButtonEvent   *be = (XButtonEvent *) event;

    switch (event->type) {
    case ButtonPress:
	if (be->button == Button3 && be->state & Mod1Mask) {
	    be->button = Button2;
	}
	switch (be->button) {
	case Button1:
	    XDefineCursor(tool_d, topruler_win, l_arrow_cursor);
	    break;
	case Button2:
	    XDefineCursor(tool_d, topruler_win, bull_cursor);
	    orig_zoomoff = zoomxoff;
	    last_drag_x = event->x;
	    break;
	case Button3:
	    XDefineCursor(tool_d, topruler_win, r_arrow_cursor);
	    break;
	}
	break;
    case ButtonRelease:
	if (be->button == Button3 && be->state & Mod1Mask) {
	    be->button = Button2;
	}
	switch (be->button) {
	case Button1:
	    pan_left();
	    break;
	case Button2:
	    if (orig_zoomoff != zoomxoff)
		redisplay_canvas();
	    break;
	case Button3:
	    pan_right();
	    break;
	}
	XDefineCursor(tool_d, topruler_win, lr_arrow_cursor);
	break;
    case MotionNotify:
	if (event->x != last_drag_x)
	    if ((zoomxoff != 0) || (event->x < last_drag_x)) {
                zoomxoff -= ((event->x - last_drag_x)/zoomscale*
				(event->state&ShiftMask?5.0:1.0));
                if (zoomxoff < 0)
                    zoomxoff = 0; 
                reset_topruler(); 
                redisplay_topruler();
            }
	last_drag_x = event->x;
	break;
    }
}

void
erase_toprulermark(void)
{
    XClearArea(tool_d, topruler_win, ZOOMX(lastx) + troffx,
	       TOPRULER_HT + troffy, trm_pr.width,
	       trm_pr.height, False);
}

void
set_toprulermark(double x)
{
    XClearArea(tool_d, topruler_win, ZOOMX(lastx) + troffx,
	       TOPRULER_HT + troffy, trm_pr.width,
	       trm_pr.height, False);
    XCopyArea(tool_d, toparrow_pm, topruler_win, tr_xor_gc,
	      0, 0, trm_pr.width, trm_pr.height,
	      ZOOMX(x) + troffx, TOPRULER_HT + troffy);
    lastx = x;
}

/* ARGSUSED */
static void
topruler_exposed(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams)
{
    if (((XExposeEvent *) event)->count > 0)
	return;
    redisplay_topruler();
}

void
redisplay_topruler(void)
{
    XClearWindow(tool_d, topruler_win);
}

void
init_topruler(TOOL tool)
{
    FirstArg(XtNwidth, TOPRULER_WD);
    NextArg(XtNheight, TOPRULER_HT);
    NextArg(XtNlabel, "");
    NextArg(XtNfromHoriz, mode_panel);
    NextArg(XtNhorizDistance, -INTERNAL_BW);
    NextArg(XtNfromVert, msg_form);
    NextArg(XtNvertDistance, -INTERNAL_BW);
    NextArg(XtNresizable, False);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    NextArg(XtNborderWidth, INTERNAL_BW);

    topruler_sw = XtCreateWidget("topruler", labelWidgetClass, tool,
				 Args, ArgCount);

    XtAppAddActions(tool_app, topruler_actions, XtNumber(topruler_actions));
    XtOverrideTranslations(topruler_sw,
			   XtParseTranslationTable(topruler_translations));
    return;
}

void
setup_topruler(void)
{
    unsigned long   bg, fg;
    XGCValues	    gcv;
    unsigned long   gcmask;

    topruler_win = XtWindow(topruler_sw);
    gcv.font = roman_font->fid;
    gcmask = GCFunction | GCForeground | GCBackground | GCFont;

    /* set up the GCs */
    FirstArg(XtNbackground, &bg);
    NextArg(XtNforeground, &fg);
    GetValues(topruler_sw);

    gcv.foreground = bg;
    gcv.background = bg;
    gcv.function = GXcopy;
    tr_erase_gc = XCreateGC(tool_d, topruler_win, gcmask, &gcv);

    gcv.foreground = fg;
    tr_gc = XCreateGC(tool_d, topruler_win, gcmask, &gcv);
    /*
     * The arrows will be XORed into the rulers. We want the foreground color
     * in the arrow to result in the foreground or background color in the
     * display. so if the source pixel is fg^bg, it produces fg when XOR'ed
     * with bg, and bg when XOR'ed with bg. If the source pixel is zero, it
     * produces fg when XOR'ed with fg, and bg when XOR'ed with bg.
     */
    /* first make a temporary xor gc */
    gcv.foreground = fg ^ bg;
    gcv.background = (unsigned long) 0;
    gcv.function = GXcopy;
    tr_xor_gc = XCreateGC(tool_d, topruler_win, gcmask, &gcv);

    /* make pixmaps for top ruler arrow */
    toparrow_pm = XCreatePixmap(tool_d, topruler_win, trm_pr.width,
				trm_pr.height, DefaultDepthOfScreen(tool_s));
    XPutImage(tool_d, toparrow_pm, tr_xor_gc, &trm_pr, 0, 0, 0, 0,
	      trm_pr.width, trm_pr.height);
    XFreeGC(tool_d, tr_xor_gc);

    /* now make the real xor gc */
    gcv.background = bg;
    gcv.function = GXxor;
    tr_xor_gc = XCreateGC(tool_d, topruler_win, gcmask, &gcv);

    XDefineCursor(tool_d, topruler_win, lr_arrow_cursor);

    topruler_pm = XCreatePixmap(tool_d, topruler_win,
				TOPRULER_WD, TOPRULER_HT,
				DefaultDepthOfScreen(tool_s));

    reset_topruler();
}

void
resize_topruler(void)
{
    XFreePixmap(tool_d, topruler_pm);
    topruler_pm = XCreatePixmap(tool_d, topruler_win,
				TOPRULER_WD, TOPRULER_HT,
				DefaultDepthOfScreen(tool_s));

    reset_topruler();
}

void
reset_topruler(void)
{
    register int    i;
    register Pixmap p = topruler_pm;
    char	    number[4];
    int		    X0;
    float	    skip;

    /* top ruler, adjustments for digits are kludges based on 6x13 char */
    XFillRectangle(tool_d, p, tr_erase_gc, 0, 0, TOPRULER_WD, TOPRULER_HT);

    skip = 1;  
    if (display_zoomscale < 0.2)
        skip = 10;
    else if (display_zoomscale < 0.3)
                skip = 4;
    else if (display_zoomscale < 0.7)  
                skip = 2;
    else if (display_zoomscale >= 5.0)
                skip = 0.5;

    X0 = BACKX(0);
    X0 -= (X0 % TWOMM);
        for (i = X0; i <= X0+round(TOPRULER_WD/zoomscale); i += ONEMM) {
            if ((int)(i/skip) % PIX_PER_CM == 0) {
                if (1.0*i/PIX_PER_CM == (int)(i/PIX_PER_CM))
                    sprintf(number, "%-d", (int)(i / PIX_PER_CM));
                else
                    sprintf(number, "%-.1f", (float)(1.0 * i / PIX_PER_CM));
                XDrawString(tool_d, p, tr_gc, ZOOMX(i) - 3,
                        TOPRULER_HT - INCH_MARK - 5, number, strlen(number));
            }
            if (i % PIX_PER_CM == 0)
                XDrawLine(tool_d, p, tr_gc, ZOOMX(i), TOPRULER_HT - 1, ZOOMX(i),
                          TOPRULER_HT - INCH_MARK - 1);
            else if (i % TWOMM == 0 && display_zoomscale >= 0.3)
                XDrawLine(tool_d, p, tr_gc, ZOOMX(i), TOPRULER_HT - 1, ZOOMX(i),
                          TOPRULER_HT - QUARTER_MARK - 1);
    }
    /* change the pixmap ID to fool the intrinsics to actually set the pixmap */
    FirstArg(XtNbackgroundPixmap, 0);
    SetValues(topruler_sw);
    FirstArg(XtNbackgroundPixmap, p);
    SetValues(topruler_sw);
}

/************************* SIDERULER ************************/

XtActionsRec	sideruler_actions[] =
{
    {"EventSideRuler", (XtActionProc) sideruler_selected},
    {"ExposeSideRuler", (XtActionProc) sideruler_exposed},
    {"EnterSideRuler", (XtActionProc) draw_mousefun_sideruler},
    {"LeaveSideRuler", (XtActionProc) clear_mousefun},
};

static String	sideruler_translations =
"Any<BtnDown>:EventSideRuler()\n\
    Any<BtnUp>:EventSideRuler()\n\
    <Btn2Motion>:EventSideRuler()\n\
    Meta <Btn3Motion>:EventSideRuler()\n\
    <EnterWindow>:EnterSideRuler()\n\
    <LeaveWindow>:LeaveSideRuler()\n\
    <Expose>:ExposeSideRuler()\n";

/* ARGSUSED */
static void
sideruler_selected(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams)
{
    XButtonEvent   *be = (XButtonEvent *) event;

    switch (event->type) {
    case ButtonPress:
	if (be->button == Button3 && be->state & Mod1Mask) {
	    be->button = Button2;
	}
	switch (be->button) {
	case Button1:
	    XDefineCursor(tool_d, sideruler_win, u_arrow_cursor);
	    break;
	case Button2:
	    XDefineCursor(tool_d, sideruler_win, bull_cursor);
	    orig_zoomoff = zoomyoff;
	    last_drag_y = event->y;
	    break;
	case Button3:
	    XDefineCursor(tool_d, sideruler_win, d_arrow_cursor);
	    break;
	}
	break;
    case ButtonRelease:
	if (be->button == Button3 && be->state & Mod1Mask) {
	    be->button = Button2;
	}
	switch (be->button) {
	case Button1:
	    pan_up();
	    break;
	case Button2:
	    if (orig_zoomoff != zoomxoff)
		redisplay_canvas();
	    break;
	case Button3:
	    pan_down();
	    break;
	}
	XDefineCursor(tool_d, sideruler_win, ud_arrow_cursor);
	break;
    case MotionNotify:
	if (event->y != last_drag_y)
	    if ((zoomyoff != 0) || (event->y < last_drag_y)) {
                zoomyoff -= ((event->y - last_drag_y)/zoomscale*
				(event->state&ShiftMask?5.0:1.0));
		if (zoomyoff < 0)
		    zoomyoff = 0;
		reset_sideruler();
		redisplay_sideruler();
	    }
	last_drag_y = event->y;
	break;
    }
}

/* ARGSUSED */
static void
sideruler_exposed(TOOL tool, XButtonEvent *event, String *params, Cardinal *nparams)
{
    if (((XExposeEvent *) event)->count > 0)
	return;
    redisplay_sideruler();
}

void
init_sideruler(TOOL tool)
{
    FirstArg(XtNwidth, SIDERULER_WD);
    NextArg(XtNheight, SIDERULER_HT);
    NextArg(XtNlabel, "");
    NextArg(XtNfromHoriz, canvas_sw);
    NextArg(XtNhorizDistance, -INTERNAL_BW);
    NextArg(XtNfromVert, topruler_sw);
    NextArg(XtNvertDistance, -INTERNAL_BW);
    NextArg(XtNresizable, False);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNleft, XtChainLeft);
    NextArg(XtNright, XtChainLeft);
    NextArg(XtNborderWidth, INTERNAL_BW);

    sideruler_sw = XtCreateWidget("sideruler", labelWidgetClass, tool,
				  Args, ArgCount);

    XtAppAddActions(tool_app, sideruler_actions, XtNumber(sideruler_actions));
    XtOverrideTranslations(sideruler_sw,
			   XtParseTranslationTable(sideruler_translations));
    return;
}

void
redisplay_sideruler(void)
{
    XClearWindow(tool_d, sideruler_win);
}

void
setup_sideruler(void)
{
    unsigned long   bg, fg;
    XGCValues	    gcv;
    unsigned long   gcmask;

    sideruler_win = XtWindow(sideruler_sw);
    gcv.font = roman_font->fid;
    gcmask = GCFunction | GCForeground | GCBackground | GCFont;

    /* set up the GCs */
    FirstArg(XtNbackground, &bg);
    NextArg(XtNforeground, &fg);
    GetValues(sideruler_sw);

    gcv.foreground = bg;
    gcv.background = bg;
    gcv.function = GXcopy;
    sr_erase_gc = XCreateGC(tool_d, sideruler_win, gcmask, &gcv);

    gcv.foreground = fg;
    sr_gc = XCreateGC(tool_d, sideruler_win, gcmask, &gcv);
    /*
     * The arrows will be XORed into the rulers. We want the foreground color
     * in the arrow to result in the foreground or background color in the
     * display. so if the source pixel is fg^bg, it produces fg when XOR'ed
     * with bg, and bg when XOR'ed with bg. If the source pixel is zero, it
     * produces fg when XOR'ed with fg, and bg when XOR'ed with bg.
     */
    /* first make a temporary xor gc */
    gcv.foreground = fg ^ bg;
    gcv.background = (unsigned long) 0;
    gcv.function = GXcopy;
    sr_xor_gc = XCreateGC(tool_d, sideruler_win, gcmask, &gcv);

    /* make pixmaps for side ruler arrow */
    if (appres.RHS_PANEL) {
	sidearrow_pm = XCreatePixmap(tool_d, sideruler_win,
				     srlm_pr.width, srlm_pr.height,
				     DefaultDepthOfScreen(tool_s));
	XPutImage(tool_d, sidearrow_pm, sr_xor_gc, &srlm_pr, 0, 0, 0, 0,
		  srlm_pr.width, srlm_pr.height);
    } else {
	sidearrow_pm = XCreatePixmap(tool_d, sideruler_win,
				     srrm_pr.width, srrm_pr.height,
				     DefaultDepthOfScreen(tool_s));
	XPutImage(tool_d, sidearrow_pm, sr_xor_gc, &srrm_pr, 0, 0, 0, 0,
		  srrm_pr.width, srrm_pr.height);
    }
    XFreeGC(tool_d, sr_xor_gc);

    /* now make the real xor gc */
    gcv.background = bg;
    gcv.function = GXxor;
    sr_xor_gc = XCreateGC(tool_d, sideruler_win, gcmask, &gcv);

    XDefineCursor(tool_d, sideruler_win, ud_arrow_cursor);

    sideruler_pm = XCreatePixmap(tool_d, sideruler_win,
				 SIDERULER_WD, SIDERULER_HT,
				 DefaultDepthOfScreen(tool_s));

    reset_sideruler();
}

void
resize_sideruler(void)
{
    XFreePixmap(tool_d, sideruler_pm);
    sideruler_pm = XCreatePixmap(tool_d, sideruler_win,
				 SIDERULER_WD, SIDERULER_HT,
				 DefaultDepthOfScreen(tool_s));
    reset_sideruler();
}

void
reset_sideruler(void)
{
    register int    i;
    register Pixmap p = sideruler_pm;
    char	    number[4];
    int		    Y0;
    float	    skip;

    /* side ruler, adjustments for digits are kludges based on 6x13 char */
    XFillRectangle(tool_d, p, sr_erase_gc, 0, 0, SIDERULER_WD,
		   (int) (SIDERULER_HT));

    skip = 1;
    if (display_zoomscale < 0.2)
        skip = 10;        
    else if (display_zoomscale < 0.3)
                skip = 4;
    else if (display_zoomscale < 0.7)
                skip = 2;
    else if (display_zoomscale >= 5.0)
                skip = 0.5;

    Y0 = BACKY(0);
    Y0 -= (Y0 % TWOMM);
    if (appres.RHS_PANEL) {
            for (i = Y0; i <= Y0+round(SIDERULER_HT/zoomscale); i++) {
                if (i % PIX_PER_CM == 0) {
                    XDrawLine(tool_d, p, sr_gc, SIDERULER_WD - INCH_MARK,
                              ZOOMY(i), SIDERULER_WD, ZOOMY(i));
                    if ((int)(i/skip) % PIX_PER_CM == 0) {
                        if (1.0*i/PIX_PER_CM == (int)(i / PIX_PER_CM))
                            sprintf(number, "%d", (int)(i / PIX_PER_CM));
                        else  
                            sprintf(number, "%.1f", (float)(1.0 * i / PIX_PER_CM));                 
                        XDrawString(tool_d, p, sr_gc,
                                SIDERULER_WD - INCH_MARK - 14, ZOOMY(i) + 3,
                                number, strlen(number));
                    }     
                } else if (i % TWOMM == 0 && display_zoomscale >= 0.3)
                    XDrawLine(tool_d, p, sr_gc,
                              SIDERULER_WD - QUARTER_MARK, ZOOMY(i),
                              SIDERULER_WD, ZOOMY(i));  
            }       
        } else {        
            for (i = Y0; i <= Y0+round(SIDERULER_HT/zoomscale); i++) {
                if (i % PIX_PER_CM == 0) {
                    XDrawLine(tool_d, p, sr_gc, 0, ZOOMY(i),
                              INCH_MARK - 1, ZOOMY(i));
                    if ((int)(i/skip) % PIX_PER_CM == 0) {
                        if (1.0*i/PIX_PER_CM == (int)(i / PIX_PER_CM))
                            sprintf(number, "%d", (int)(i / PIX_PER_CM));
                        else 
                            sprintf(number, "%.1f", (float)(1.0 * i / PIX_PER_CM));                           
                        XDrawString(tool_d, p, sr_gc, INCH_MARK + 3,
                                ZOOMY(i) + 3, number, strlen(number));
                    }         
                } else if (i % TWOMM == 0 && display_zoomscale >= 0.3)
                    XDrawLine(tool_d, p, sr_gc, 0, ZOOMY(i),
                              QUARTER_MARK - 1, ZOOMY(i));  
            }                    
    }
    /* change the pixmap ID to fool the intrinsics to actually set the pixmap */
    FirstArg(XtNbackgroundPixmap, 0);
    SetValues(sideruler_sw);
    FirstArg(XtNbackgroundPixmap, p);
    SetValues(sideruler_sw);
}

void
erase_siderulermark(void)
{
    if (appres.RHS_PANEL)
	XClearArea(tool_d, sideruler_win,
		   SIDERULER_WD + srloffx, ZOOMY(lasty) + srloffy,
		   srlm_pr.width, srlm_pr.height, False);
    else
	XClearArea(tool_d, sideruler_win,
		   srroffx, ZOOMY(lasty) + srroffy,
		   srlm_pr.width, srlm_pr.height, False);
}

void
set_siderulermark(double y)
{
    if (appres.RHS_PANEL) {
	/*
	 * Because the ruler uses a background pixmap, we can win here by
	 * using XClearArea to erase the old thing.
	 */
	XClearArea(tool_d, sideruler_win,
		   SIDERULER_WD + srloffx, ZOOMY(lasty) + srloffy,
		   srlm_pr.width, srlm_pr.height, False);
	XCopyArea(tool_d, sidearrow_pm, sideruler_win,
		  sr_xor_gc, 0, 0, srlm_pr.width,
		  srlm_pr.height, SIDERULER_WD + srloffx, ZOOMY(y) + srloffy);
    } else {
	/*
	 * Because the ruler uses a background pixmap, we can win here by
	 * using XClearArea to erase the old thing.
	 */
	XClearArea(tool_d, sideruler_win,
		   srroffx, ZOOMY(lasty) + srroffy,
		   srlm_pr.width, srlm_pr.height, False);
	XCopyArea(tool_d, sidearrow_pm, sideruler_win,
		  sr_xor_gc, 0, 0, srrm_pr.width,
		  srrm_pr.height, srroffx, ZOOMY(y) + srroffy);
    }
    lasty = y;
}
