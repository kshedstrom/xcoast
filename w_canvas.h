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

/************** DECLARE EXPORTS ***************/

extern void	(*canvas_kbd_proc) ();
extern void	(*canvas_locmove_proc) ();
extern void	(*canvas_leftbut_proc) ();
extern void	(*canvas_middlebut_proc) ();
extern void	(*canvas_middlebut_save) ();
extern void	(*canvas_rightbut_proc) ();
extern void	(*return_proc) ();
extern void	null_proc(void);
extern double	clip_xmin, clip_ymin, clip_xmax, clip_ymax;
extern int	clip_width, clip_height;
extern double	cur_x, cur_y;
