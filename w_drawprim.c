/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985 by Supoj Sutanthavibul
 * Copyright (c) 1992 by Brian V. Smith
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both the copyright
 * notice and this permission notice appear in supporting documentation. 
 * No representations are made about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or implied warranty."
 */

/*
 * This file provides some drawing primitives which make use of the
 * underlying low-level windowing system operations.
 *
 * The file is divided into routines for:
 *
 * GRAPHICS CONTEXTS (which are used by all the following)
 * FONTS
 * LINES
 * SHADING
 */

/* IMPORTS */

#include "fig.h"
#include "figx.h"
#include "resources.h"
#include "paintop.h"
#include "mode.h"
#include "object.h"
#include "u_fonts.h"
#include "w_canvas.h"
#include "w_drawprim.h"
#include "w_icons.h"		/* for none_ic in init_fill_pm */
#include "w_indpanel.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"

/* EXPORTS */

PIX_FONT	bold_font;
PIX_FONT	roman_font;
PIX_FONT	button_font;

/* LOCAL */

static Pixel	gc_color[NUMOPS];
static XRectangle clip[1];

#define MAXNAMES 35

extern int fprintf (FILE *, const char *, ...);
void set_line_stuff (int width, int style, float style_val, int op, int color);

void init_font(void)
{
    if (appres.boldFont == NULL || *appres.boldFont == '\0')
	appres.boldFont = BOLD_FONT;
    if (appres.normalFont == NULL || *appres.normalFont == '\0')
	appres.normalFont = NORMAL_FONT;
    if (appres.buttonFont == NULL || *appres.buttonFont == '\0')
	appres.buttonFont = BUTTON_FONT;

    roman_font = XLoadQueryFont(tool_d, appres.normalFont);
    if ((bold_font = XLoadQueryFont(tool_d, appres.boldFont)) == 0) {
	fprintf(stderr, "Can't load font: %s, using %s\n",
		appres.boldFont, appres.normalFont);
	bold_font = XLoadQueryFont(tool_d, appres.normalFont);
    }
    if ((button_font = XLoadQueryFont(tool_d, appres.buttonFont)) == 0) {
	fprintf(stderr, "Can't load font: %s, using %s\n",
		appres.buttonFont, appres.normalFont);
	button_font = XLoadQueryFont(tool_d, appres.normalFont);
    }
}

/* LINES */

static int	gc_thickness[NUMOPS], gc_line_style[NUMOPS];

static		GC
makegc(int op, Pixel fg, Pixel bg)
{
    register GC	    ngc;
    XGCValues	    gcv;
    unsigned long   gcmask;

    gcv.font = roman_font->fid;
    gcv.join_style = JoinMiter;
    gcmask = GCJoinStyle | GCFunction | GCForeground | GCBackground | GCFont;
    switch (op) {
    case PAINT:
	gcv.foreground = fg;
	gcv.background = bg;
	gcv.function = GXcopy;
	break;
    case ERASE:
	gcv.foreground = bg;
	gcv.background = bg;
	gcv.function = GXcopy;
	break;
    case INV_PAINT:
	gcv.foreground = fg ^ bg;
	gcv.background = bg;
	gcv.function = GXxor;
	break;
    case MERGE:
	gcv.foreground = fg;
	gcv.background = bg;
	gcv.function = GXor;
	break;
    }

    ngc = XCreateGC(tool_d, XtWindow(canvas_sw), gcmask, &gcv);
    XCopyGC(tool_d, gc, ~(gcmask), ngc);	/* add main gc's values to
						 * the new one */
    return (ngc);
}

void init_gc(void)
{
    int		    i;

    gccache[PAINT] = makegc(PAINT, x_fg_color.pixel, x_bg_color.pixel);
    gccache[ERASE] = makegc(ERASE, x_fg_color.pixel, x_bg_color.pixel);
    gccache[INV_PAINT] = makegc(INV_PAINT, x_fg_color.pixel, x_bg_color.pixel);
    gccache[MERGE] = makegc(MERGE, x_fg_color.pixel, x_bg_color.pixel);

    for (i = 0; i < NUMOPS; i++) {
	gc_color[i] = -1;
	gc_thickness[i] = -1;
	gc_line_style[i] = -1;
    }
}

void pw_vector(Window w, double x1, double y1, double x2, double y2, int op, int line_width, int line_style, float style_val, int color)
{
    if (line_width == 0)
	return;
    set_line_stuff(line_width, line_style, style_val, op, color);
    if (line_style == PANEL_LINE)
	XDrawLine(tool_d, w, gccache[op], (int)x1, (int)y1, (int)x2, (int)y2);
    else
	zXDrawLine(tool_d, w, gccache[op], (int)x1, (int)y1, (int)x2, (int)y2);
}

void pw_point(Window w, double x, double y, int line_width, int op, int color)
{
    /* pw_point doesn't use line_style or fill_style - maybe not needed */
    /* (needs color though - hns) */
    set_line_stuff(line_width, SOLID_LINE, 0.0, op, color);
    zXDrawPoint(tool_d, w, gccache[op], (int)x, (int)y);
}

void pw_lines(Window w, XPoint *points, int npoints, int op, int line_width, int line_style, float style_val, int color)
{
    register int i;
    register XPoint *p;

    if (line_width == 0)
	return;
    /* if the line has only one point or it has two points and those
       points are coincident AND we are drawing a DOTTED line, this
       kills Xsun and hangs other servers.  We will just call pw_point
       since it is only a point anyway */

    if ((npoints == 1) || (npoints == 2 &&
	 points[0].x == points[1].x && points[0].y == points[1].y)) {   
            pw_point(w, points[0].x, points[0].y, line_width, op, color);
            return;
    }

#ifdef FOO
    if (line_style == PANEL_LINE) {
#endif
        /* must use XPoint, not our zXPoint */
        p = (XPoint *) malloc(npoints * sizeof(XPoint));
        for (i=0; i<npoints; i++) {
            p[i].x = (short) points[i].x;
            p[i].y = (short) points[i].y;
        }
#ifdef FOO
    }
#endif
    set_line_stuff(line_width, line_style, style_val, op, color);
#ifdef FOO
    if (line_style == PANEL_LINE) {
#endif
        XDrawLines(tool_d, w, gccache[op], p, npoints, CoordModeOrigin);
        free((char *) p);
#ifdef FOO
    } else {
        zXDrawLines(tool_d, w, gccache[op], points, npoints, CoordModeOrigin);
    }
#endif
}

void set_clip_window(int xmin, int ymin, int xmax, int ymax)
{
    clip_xmin = clip[0].x = xmin;
    clip_ymin = clip[0].y = ymin;
    clip_xmax = xmax;
    clip_ymax = ymax;
    clip_width = clip[0].width = xmax - xmin;
    clip_height = clip[0].height = ymax - ymin;
    XSetClipRectangles(tool_d, gccache[PAINT], 0, 0, clip, 1, YXBanded);
    XSetClipRectangles(tool_d, gccache[INV_PAINT], 0, 0, clip, 1, YXBanded);
}

void reset_clip_window(void)
{
    set_clip_window(0, 0, CANVAS_WD, CANVAS_HT);
}

void set_line_stuff(int width, int style, float style_val, int op, int color)
{
    XGCValues	    gcv;
    unsigned long   mask;
    static unsigned char dash_list[2] = {-1, -1};

    switch (style) {
    case RUBBER_LINE:
	width = 0;
	break;
    case PANEL_LINE:
    case COAST_LINE:
	break;
    default:
/* don't want to have thicker lines when zoomed */
/*	width = round(display_zoomscale * width);   */
	break;
    }

    /* user zero-width lines for speed with SOLID lines */
    /* can't do this for dashed lines because server isn't */
    /* required to draw dashes for zero-width lines */
    if (width == 1 && style == SOLID_LINE)
	width = 0;

    /* if we're drawing to the bitmap instead of the canvas
       map colors white => white, all others => black */
    if (writing_bitmap)
	{
	if (color == WHITE)
		color = 0;
	else
		color = 1;
	}
    /* see if all gc stuff is already correct */

    if (width == gc_thickness[op] && style == gc_line_style[op] &&
	(writing_bitmap? color == gc_color[op] : x_color(color) == gc_color[op]) &&
	(style != DASH_LINE && style != DOTTED_LINE ||
	 dash_list[1] == (char) round(style_val * display_zoomscale)))
	return;			/* no need to change anything */

    gcv.line_width = width;
    mask = GCLineWidth | GCLineStyle | GCCapStyle;
    if (op == PAINT)
	mask |= GCForeground;
    gcv.line_style = (style == DASH_LINE || style == DOTTED_LINE) ?
	LineOnOffDash : LineSolid;
    gcv.cap_style = (style == DOTTED_LINE) ? CapRound : CapButt;
    gcv.foreground = (writing_bitmap? color : x_color(color));

    XChangeGC(tool_d, gccache[op], mask, &gcv);
    if (style == DASH_LINE || style == DOTTED_LINE) {
	if (style_val > 0.0) {	/* style_val of 0.0 causes problems */
	    /* length of ON/OFF pixels */
	    dash_list[0] = dash_list[1] =
			(char) round(style_val * display_zoomscale);
	    if (dash_list[0]==0)		/* take care for rounding to zero ! */
		dash_list[0]=dash_list[1]=1;

	    if (style == DOTTED_LINE)
		dash_list[0] = 1;	/* length of ON pixels for dotted */
	    XSetDashes(tool_d, gccache[op], 0, (char *) dash_list, 2);
	}
    }
    gc_thickness[op] = width;
    gc_line_style[op] = style;
    gc_color[op] = writing_bitmap? color : x_color(color);
}
