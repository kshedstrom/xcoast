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

#include "paintop.h"

typedef struct {
    unsigned int    x, y, z;
    caddr_t	   *m;
}		MprData;

#define mpr_static(name,x,y,z,pix)	\
XImage name	= \
{ \
(x),		/* width */ \
(y),		/* height */ \
0,		/* offset */ \
XYBitmap,	/* format */ \
(char *)(pix),	/* data pointer */ \
MSBFirst,	/* data byte order LSB or MSB first */ \
8,		/* quant of scanline */ \
LSBFirst,	/* bitmap bit order LSB or MSBFirst */ \
8,		/* bitmap pad */ \
(z),		/* depth */ \
(x+7)/8,	/* bytes-per-line */ \
1,		/* bits per pizel */ \
0,		/* red_mask */ \
0,		/* z arrangement green_mask */ \
0,		/* z arrangement blue_mask */ \
NULL		/* object data pointer for extension */ \
}

#define NUMCOLORS 8
extern char    *colorNames[NUMCOLORS + 1];
extern Boolean	all_colors_available;
extern float	ZOOM_FACTOR;

/* resources structure */

typedef struct _appres {
    char           *iconGeometry;
    Boolean	    DEBUG;
    Boolean	    RHS_PANEL;
    Boolean	    INVERSE;
    Boolean	    TRACKING;
    Boolean	    SCALABLEFONTS;	/* hns 5 Nov 91 */
    char	   *normalFont;
    char	   *boldFont;
    char	   *buttonFont;
    float	    tmp_width;
    float	    tmp_height;
    int		    internalborderwidth;
    Pixel	    color[NUMCOLORS];
    Boolean	    monochrome;

}		appresStruct, *appresPtr;
extern appresStruct appres;

typedef struct {
    int		    x, y;
}		pr_size;

typedef struct {
    unsigned int    r_width, r_height, r_left, r_top;
}		RectRec;

typedef struct {
    int		    type;
    char	   *label;
    caddr_t	    info;
}		MenuItemRec;

struct Menu {
    int		    m_imagetype;
#define MENU_IMAGESTRING	0x00	/* imagedata is char * */
#define MENU_GRAPHIC		0x01	/* imagedata is pixrect * */
    caddr_t	    m_imagedata;
    int		    m_itemcount;
    MenuItemRec	   *m_items;
    struct Menu	   *m_next;
    caddr_t	    m_data;
};

typedef struct Menu MenuRec;

typedef XImage	PIXRECTREC;
typedef XImage *PIXRECT;
typedef XFontStruct *PIX_FONT;
typedef MprData MPR_DATA;
typedef Widget	TOOL;
typedef Widget	TOOLSW;
typedef pr_size PR_SIZE;
typedef RectRec RECT;

extern Window	canvas_win, msg_win, sideruler_win, topruler_win;

extern Cursor	cur_cursor;
extern Cursor	arrow_cursor, bull_cursor, buster_cursor, crosshair_cursor,
		null_cursor, pencil_cursor, pick15_cursor, pick9_cursor,
		panel_cursor, l_arrow_cursor, lr_arrow_cursor, r_arrow_cursor,
		u_arrow_cursor, ud_arrow_cursor, d_arrow_cursor, wait_cursor;

extern TOOL	tool;
extern XtAppContext tool_app;

extern TOOLSW	canvas_sw,
		msg_form, msg_panel, name_panel, cmd_panel, mode_panel,
		d_label, e_label, mousefun,
		ind_viewp, ind_panel,	/* indicator panel */
		unitbox_sw, sideruler_sw, topruler_sw;

extern Display *tool_d;
extern Screen  *tool_s;
extern int	tool_sn;

extern GC	gc, bold_gc, button_gc, ind_button_gc, mouse_button_gc,
		color_gc, blank_gc, ind_blank_gc, mouse_blank_gc, gccache[NUMOPS],
		tr_gc, tr_xor_gc, tr_erase_gc,  /* for the rulers */
		sr_gc, sr_xor_gc, sr_erase_gc;

extern XColor	x_fg_color, x_bg_color;
extern Boolean	writing_bitmap;
extern unsigned long but_fg, but_bg;
extern unsigned long ind_but_fg, ind_but_bg;
extern unsigned long mouse_but_fg, mouse_but_bg;

/* will be filled in with environment variable XFIGTMPDIR */
extern char    *TMPDIR;

struct icon {
    short	    ic_width, ic_height;	/* overall icon dimensions */
    PIXRECT	    ic_background;	/* background pattern (mem pixrect) */
    RECT	    ic_gfxrect; /* where the graphic goes */
    PIXRECT	    ic_mpr;	/* the graphic (a memory pixrect) */
    RECT	    ic_textrect;/* where text goes */
    char	   *ic_text;	/* the text */
    PIX_FONT	    ic_font;	/* Font with which to display text */
    int		    ic_flags;
};

/* flag values */
#define ICON_BKGRDPAT	0x02	/* use ic_background to prepare image */
#define ICON_BKGRDGRY	0x04	/* use std gray to prepare image */
#define ICON_BKGRDCLR	0x08	/* clear to prepare image */
#define ICON_BKGRDSET	0x10	/* set to prepare image */
#define ICON_FIRSTPRIV	0x0100	/* start of private flags range */
#define ICON_LASTPRIV	0x8000	/* end of private flags range */
