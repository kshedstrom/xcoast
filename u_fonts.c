/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Brian V. Smith
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both the copyright
 * notice and this permission notice appear in supporting documentation. 
 * No representations are made about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or implied warranty."
 */

#include <X11/Xlib.h>
#include "fig.h"
#include "resources.h"
#include "u_fonts.h"
#include "object.h"

/* printer font names for indicator window */

struct _xfstruct x_fontinfo[NUM_X_FONTS] = {
    {"-adobe-times-medium-r-*--", (struct xfont*) NULL},
    {"-adobe-times-medium-i-*--", (struct xfont*) NULL},
    {"-adobe-times-bold-r-*--", (struct xfont*) NULL},
    {"-adobe-times-bold-i-*--", (struct xfont*) NULL},
    {"-schumacher-clean-medium-r-*--", (struct xfont*) NULL},
    {"-schumacher-clean-medium-i-*--", (struct xfont*) NULL},
    {"-schumacher-clean-bold-r-*--", (struct xfont*) NULL},
    {"-schumacher-clean-bold-i-*--", (struct xfont*) NULL},
    {"-adobe-courier-medium-r-*--", (struct xfont*) NULL},
    {"-adobe-courier-medium-o-*--", (struct xfont*) NULL},
    {"-adobe-courier-bold-r-*--", (struct xfont*) NULL},
    {"-adobe-courier-bold-o-*--", (struct xfont*) NULL},
    {"-adobe-helvetica-medium-r-*--", (struct xfont*) NULL},
    {"-adobe-helvetica-medium-o-*--", (struct xfont*) NULL},
    {"-adobe-helvetica-bold-r-*--", (struct xfont*) NULL},
    {"-adobe-helvetica-bold-o-*--", (struct xfont*) NULL},
    {"-adobe-new century schoolbook-medium-r-*--", (struct xfont*) NULL},
    {"-adobe-new century schoolbook-medium-i-*--", (struct xfont*) NULL},
    {"-adobe-new century schoolbook-bold-r-*--", (struct xfont*) NULL},
    {"-adobe-new century schoolbook-bold-i-*--", (struct xfont*) NULL},
    {"-*-lucidabright-medium-r-*--", (struct xfont*) NULL},
    {"-*-lucidabright-medium-i-*--", (struct xfont*) NULL},
    {"-*-lucidabright-demibold-r-*--", (struct xfont*) NULL},
    {"-*-lucidabright-demibold-i-*--", (struct xfont*) NULL},
    {"-*-symbol-medium-r-*--", (struct xfont*) NULL},
    {"-*-zapfchancery-medium-i-*--", (struct xfont*) NULL},
    {"-*-zapfdingbats-*-*-*--", (struct xfont*) NULL},
};
