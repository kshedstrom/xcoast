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
#include "u_coords.h"
#include "w_indpanel.h"

int		cur_mode = F_NULL;
int		pan_jump = 60;    /* 20 mm */
int		gridmode = GRID_0;
int		gridsize = 1;  /* lines per ten degrees */
int		action_on = 0;
int		highlighting = 0;
int		aborting = 0;
int		figure_modified = 0;
Boolean		warnexist = True;

/******************** global mode variables ***************************/

int		num_point;
int		min_num_points;

/************************** Mode Settings *****************************/

int		cur_objmask = M_NONE;
int		cur_updatemask = I_UPDATEMASK;
char		cur_projection[32] = "";

/************************* Map Projection *****************************/

int		iprj = F_PROJ_MERC;
double          plat = 0.0;
double          plon = -44.0;
double          plon_save = -44.0;
double          rota = 0.0;
double          rlat1 = -5.0;
double          rlon1 = -90.;
double          rlat2 = 60.;
double          rlon2 = 20.;

/*********************** Coordinate Settings **************************/

struct ltln	display_center;
double		display_radius;

/************************** File Settings *****************************/

char		cur_dir[1024];
char		cur_filename[200] = "";
char		save_filename[200] = "";	/* to undo load */
char		file_header[32] = "#XCOAST ";

/*************************** routines ***********************/

void
reset_modifiedflag(void)
{
    figure_modified = 0;
}

void
set_modifiedflag(void)
{
    figure_modified = 1;
}

void
set_action_on(void)
{
    action_on = 1;
}

void
reset_action_on(void)
{
    action_on = 0;
}
