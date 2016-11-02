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
#include "paintop.h"

void init_cursor(void)
{
    register Display *d = tool_d;
    cur_cursor		= arrow_cursor;  /* current cursor */

    arrow_cursor	= XCreateFontCursor(d, XC_left_ptr);
    bull_cursor		= XCreateFontCursor(d, XC_circle);
    buster_cursor	= XCreateFontCursor(d, XC_pirate);
    crosshair_cursor	= XCreateFontCursor(d, XC_crosshair);
    null_cursor		= XCreateFontCursor(d, XC_tcross);
    pencil_cursor	= XCreateFontCursor(d, XC_pencil);
    pick15_cursor	= XCreateFontCursor(d, XC_dotbox);
    pick9_cursor	= XCreateFontCursor(d, XC_hand1);
    wait_cursor		= XCreateFontCursor(d, XC_watch);
    panel_cursor	= XCreateFontCursor(d, XC_icon);
    lr_arrow_cursor	= XCreateFontCursor(d, XC_sb_h_double_arrow);
    l_arrow_cursor	= XCreateFontCursor(d, XC_sb_left_arrow);
    r_arrow_cursor	= XCreateFontCursor(d, XC_sb_right_arrow);
    ud_arrow_cursor	= XCreateFontCursor(d, XC_sb_v_double_arrow);
    u_arrow_cursor	= XCreateFontCursor(d, XC_sb_up_arrow);
    d_arrow_cursor	= XCreateFontCursor(d, XC_sb_down_arrow);
}

void recolor_cursors(void)
{
    register Display *d = tool_d;

    XRecolorCursor(d, arrow_cursor,     &x_fg_color, &x_bg_color);
    XRecolorCursor(d, bull_cursor,      &x_fg_color, &x_bg_color);
    XRecolorCursor(d, buster_cursor,    &x_fg_color, &x_bg_color);
    XRecolorCursor(d, crosshair_cursor, &x_fg_color, &x_bg_color);
    XRecolorCursor(d, null_cursor,      &x_fg_color, &x_bg_color);
    XRecolorCursor(d, pencil_cursor,    &x_fg_color, &x_bg_color);
    XRecolorCursor(d, pick15_cursor,    &x_fg_color, &x_bg_color);
    XRecolorCursor(d, pick9_cursor,     &x_fg_color, &x_bg_color);
    XRecolorCursor(d, wait_cursor,      &x_fg_color, &x_bg_color);
    XRecolorCursor(d, panel_cursor,     &x_fg_color, &x_bg_color);
    XRecolorCursor(d, l_arrow_cursor,   &x_fg_color, &x_bg_color);
    XRecolorCursor(d, r_arrow_cursor,   &x_fg_color, &x_bg_color);
    XRecolorCursor(d, lr_arrow_cursor,  &x_fg_color, &x_bg_color);
    XRecolorCursor(d, u_arrow_cursor,   &x_fg_color, &x_bg_color);
    XRecolorCursor(d, d_arrow_cursor,   &x_fg_color, &x_bg_color);
    XRecolorCursor(d, ud_arrow_cursor,  &x_fg_color, &x_bg_color);
}

void reset_cursor(void)
{
    XDefineCursor(tool_d, canvas_win, cur_cursor);
}

void set_temp_cursor(Cursor cursor)
{
    XDefineCursor(tool_d, canvas_win, cursor);
}

void set_cursor(Cursor cursor)
{
    cur_cursor = cursor;
    XDefineCursor(tool_d, canvas_win, cursor);
}

