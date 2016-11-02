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

#define		DEFAULT		      (-1)
#define		SOLID_LINE		0
#define		DASH_LINE		1
#define		DOTTED_LINE		2
#define		RUBBER_LINE		3
#define		PANEL_LINE		4
#define		COAST_LINE		5

#define		Color			int

#define		BLACK			0
#define		WHITE			7

typedef struct f_pattern {
    int		    w, h;
    int		   *p;
}
		F_pattern;

typedef struct f_point {
    double	    x, y;
    struct f_point *next;
}
		F_point;

typedef struct f_pos {
    double	    x, y;
}
		F_pos;

#define		OPEN_PATH		1
#define		DEF_DASHLENGTH		4
#define		DEF_DOTGAP		3

typedef struct f_line {
    int		    tagged;
    int		    type;
#define					T_POLYLINE	1
    int		    style;
    int		    thickness;
    Color	    color;
    float	    style_val;
    struct f_point *points;
    struct f_line  *next;
}
		F_line;

typedef struct f_coast {
    struct f_point *points;
    struct f_coast *next;
}
	 	F_coast;

typedef struct f_grid {
    struct f_point *points;
    struct f_grid  *next;
}
		F_grid;

typedef struct f_control {
    float	    lx, ly, rx, ry;
    struct f_control *next;
}
		F_control;

#define		int_spline(s)		(s->type & 0x2)

typedef struct f_spline {
    int		    tagged;
    int		    type;
#define					T_OPEN_INTERP	2
    int		    style;
    int		    thickness;
    Color	    color;
    float	    style_val;
    /*
     * For T_OPEN_NORMAL and T_CLOSED_NORMAL points are control points while
     * they are knots for T_OPEN_INTERP and T_CLOSED_INTERP whose control
     * points are stored in controls.
     */
    struct f_point *points;
    struct f_control *controls;
    struct f_spline *next;
}
		F_spline;

typedef struct f_compound {
    int		    tagged;
    struct f_pos    nwcorner;
    struct f_pos    secorner;
    struct f_line  *lines;
    struct f_coast *coasts;
    struct f_spline *splines;
    struct f_grid  *grids;
/*    struct f_compound *compounds; */
    struct f_compound *next;
}
		F_compound;

#define		POINT_SIZE		sizeof(struct f_point)
#define		CONTROL_SIZE		sizeof(struct f_control)
#define		LINOBJ_SIZE		sizeof(struct f_line)
#define		CSTOBJ_SIZE		sizeof(struct f_coast)
#define		GRDOBJ_SIZE		sizeof(struct f_grid)
#define		SPLOBJ_SIZE		sizeof(struct f_spline)
#define		COMOBJ_SIZE		sizeof(struct f_compound)

/**********************  object codes  **********************/

#define		O_POLYLINE		2
#define		O_SPLINE		3
#define		O_COMPOUND		6
#define		O_END_COMPOUND		-O_COMPOUND
#define		O_ALL_OBJECT		99

/********************* object masks for update  ************************/

#define M_NONE			0x000
#define M_POLYLINE_LINE		0x002
#define M_SPLINE_O_INTERP	0x020
#define M_COMPOUND		0x800

#define M_SPLINE_O	(M_SPLINE_O_INTERP)
#define M_SPLINE_INTERP (M_SPLINE_O_INTERP)
#define M_SPLINE	(M_SPLINE_INTERP)
#define M_POLYLINE	(M_POLYLINE_LINE)
#define M_VARPTS_OBJECT (M_POLYLINE_LINE | M_SPLINE)
#define M_OPEN_OBJECT	(M_POLYLINE_LINE | M_SPLINE_O )
#define M_OBJECT	(M_POLYLINE | M_SPLINE )
#define M_NO_TEXT	(M_POLYLINE | M_SPLINE | M_COMPOUND )
#define M_ALL		(M_OBJECT | M_COMPOUND)

/************************  Objects  **********************/

extern F_compound objects;

/************  global working pointers ************/

extern F_line  *cur_l, *new_l, *old_l;
extern F_spline *cur_s, *new_s, *old_s;
extern F_compound *new_c, *old_c;
extern F_grid  *cur_g, *new_g, *old_g;
extern F_point *first_point, *cur_point;

/*************** object attribute settings ***********/

/*  Lines  */
extern int	cur_linewidth;
extern int	cur_linestyle;
extern float	cur_dashlength;
extern float	cur_dotgap;
extern float	cur_styleval;
extern Color	cur_color;
