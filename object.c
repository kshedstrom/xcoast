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
#include "mode.h"
#include "resources.h"
#include "object.h"
#include "paintop.h"

/************************  Objects  **********************/

F_compound	objects = {0, { 0, 0 }, {0, 0 }, NULL, NULL, NULL, NULL};

/************  global object pointers ************/

F_line	       *cur_l, *new_l, *old_l;
F_spline       *cur_s, *new_s, *old_s;
F_grid	       *cur_g, *new_g, *old_g;
F_compound     *new_c, *old_c;
F_point	       *first_point, *cur_point;

/*************** object attribute settings ***********/

/*  Lines  */
int		cur_linewidth = 2;
int		cur_linestyle = SOLID_LINE;
float		cur_dashlength = DEF_DASHLENGTH;
float		cur_dotgap = DEF_DOTGAP;
float		cur_styleval = 0.0;
Color		cur_color = DEFAULT_COLOR;
