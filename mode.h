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

#define		F_NULL			0
#define	    FIRST_DRAW_MODE	    F_INTSPLINE
#define		F_POLYLINE		6
#define		F_INTSPLINE		12
#define	    FIRST_EDIT_MODE	    F_MOVE_POINT
#define		F_ADD			33
#define		F_DELETE		36
#define		F_MOVE_POINT		37
#define		F_DELETE_POINT		38
#define		F_ADD_POINT		39
#define		F_CONVERT		45
#define		F_CHANGE		46
#define		F_UPDATE		47
#define		F_ZOOM			49
#define		F_LOAD			50

extern int	cur_mode;
extern char	cur_projection[32];

/* map projection */
#define		F_PROJ_MERC		0
#define		F_PROJ_CONIC		1
#define		F_PROJ_STEREO		2
#define		F_PROJ_FAST_MERC	3
extern int	iprj;
extern double	plat, plon, plon_save, rota;
extern double	rlat1, rlon1, rlat2, rlon2;

/* grid mode */
#define		GRID_0			0
#define		GRID_1			1
extern int	gridmode;
extern int	gridsize;

/* point position */
#define		P_ANY			0
#define		P_GRID1			1
#define		P_GRID2			2

/* misc */
extern int	action_on;
extern int	highlighting;
extern int	aborting;
extern int	figure_modified;
extern Boolean	warnexist;

extern void	reset_modifiedflag(void);
extern void	set_modifiedflag(void);
extern void	reset_action_on(void);
extern void	set_action_on(void);

/********************** global mode variables *************************/

extern int	num_point;
extern int	min_num_points;

/*************************** Mode Settings ****************************/

extern int	cur_objmask;
extern int	cur_updatemask;

/************************* Coordinate Settings ************************/

extern struct ltln	display_center;
extern double		display_radius;

/*************************** File Settings ****************************/

extern char	cur_dir[];
extern char	cur_filename[];
extern char	save_filename[];  /* to undo load */
extern char	file_header[];
