/* 
 *	FIG : Facility for Interactive Generation of figures
 *
 *	Copyright (c) 1985 by Supoj Sutanthavibul (supoj@sally.UTEXAS.EDU)
 *	January 1985.
 *	1st revision : Aug 1985.
 *
 *	%W%	%G%
*/
#include "fig.h"
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "resources.h"
#include "u_create.h"
#include "u_elastic.h"
#include "w_canvas.h"

extern F_compound	objects;
extern void             draw_mousefun_canvas(void);


extern void append_point(double x, double y, F_point **point);
extern void line_drawing_selected(void);
extern void pw_vector(Window w, double x1, double y1, double x2, double y2,
     int op, int line_width, int line_style, float style_val, int color);
extern void draw_coastline(F_coast *coast, int op);
extern void clean_up(void);
extern void add_coast(F_coast *new_t);

void get_coast_point(double x, double y)
{
    cur_x = fix_x = x;
    cur_y = fix_y = y;
    append_point(fix_x, fix_y, &cur_point);
    num_point++;
}

void init_coast_drawing(double x, double y)
{
    if ((first_point = create_point()) == NULL)
	return;

    cur_point = first_point;
    cur_point->x = fix_x = cur_x = x;
    cur_point->y = fix_y = cur_y = y;
    cur_point->next = NULL;
    num_point = 1;
}

void create_coastobject(void)
{
    F_coast	*coast;

    if ((coast = create_coast()) == NULL) {
	line_drawing_selected();
	draw_mousefun_canvas();
	return;
    }
    coast->points = first_point;
    coast->next = NULL;
    if (num_point == 1) {
	pw_vector(canvas_win, fix_x, fix_y, cur_x, cur_y, PAINT, 2,
	     SOLID_LINE, 0.0, BLACK);
    } else {
        draw_coastline(coast,PAINT);
	}
    clean_up();
    add_coast(coast);
}
