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
#include "u_coords.h"
#include "u_create.h"
#include "u_elastic.h"
#include "w_canvas.h"

extern F_compound	objects;
extern void             draw_mousefun_canvas(void);

extern void append_point(double x, double y, F_point **point);
extern int line_drawing_selected(void);
extern int pw_vector(Window w, double x1, double y1, double x2, double y2,
     int op, int line_width, int line_style, float style_val, int color);
extern int draw_grids(F_grid *grid, int op);
extern int clean_up(void);
extern void add_grid(F_grid *new_t);
extern int set_latestgrid(F_grid *grid);

void get_grid_point(double x, double y)
{
    cur_x = fix_x = x;
    cur_y = fix_y = y;
    append_point(fix_x, fix_y, &cur_point);
    num_point++;
}

void init_grid_drawing(double x, double y)
{
    if ((first_point = create_point()) == NULL)
	return;

    cur_point = first_point;
    cur_point->x = fix_x = cur_x = x;
    cur_point->y = fix_y = cur_y = y;
    cur_point->next = NULL;
    num_point = 1;
}

void create_gridobject(void)
{
    F_grid	*grid;

    if ((grid = create_grid()) == NULL) {
	line_drawing_selected();
	draw_mousefun_canvas();
	return;
    }
    grid->points = first_point;
    grid->next = NULL;
    if (num_point == 1) {
	pw_vector(canvas_win, fix_x, fix_y, cur_x, cur_y, PAINT, 1,
	     SOLID_LINE, 0.0, BLACK);
    } else {
        draw_grids(grid,PAINT);
	}
    clean_up();
    add_grid(grid);
}
