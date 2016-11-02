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

extern PIX_FONT bold_font;
extern PIX_FONT roman_font;
extern PIX_FONT button_font;

/* Maximum number of points for polygons etc */
#define		MAXNUMPTS	10000

#define		NORMAL_FONT	"fixed"
#define		BOLD_FONT	"8x13bold"
#define		BUTTON_FONT	"6x13"

#define		char_height(font) \
		((font)->max_bounds.ascent + (font)->max_bounds.descent)

#define		rot_char_width(rotfont)	((rotfont)->width)
#define		rot_char_height(rotfont) \
		((rotfont)->max_ascent + (rotfont)->max_descent)

#define		rot_char_advance(font,char) \
		    (((font)->per_char)?\
		    ((font)->per_char[(char)-(font)->min_char].width):\
		    ((font)->width))

#define set_x_color(gc,col) XSetForeground(tool_d,gc,\
	(!all_colors_available? (col==WHITE?x_bg_color.pixel:x_fg_color.pixel): \
	(col<0||col>=NUMCOLORS)? x_fg_color.pixel:appres.color[col]))

#define x_color(col)\
	(!all_colors_available? (col==WHITE?x_bg_color.pixel:x_fg_color.pixel): \
	(col<0||col>=NUMCOLORS)? x_fg_color.pixel:appres.color[col])
