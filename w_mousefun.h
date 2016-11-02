/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Paul King
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both the copyright
 * notice and this permission notice appear in supporting documentation. 
 * No representations are made about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or implied warranty."
 */

void		init_mousefun(TOOL tool);
void		setup_mousefun(void);
void		set_mousefun(char *left, char *middle, char *right);
void		draw_mousefun_mode(void);
void		draw_mousefun_ind(void);
void		draw_mousefun_unitbox(void);
void		draw_mousefun_topruler(void);
void		draw_mousefun_sideruler(void);
void		draw_mousefun_canvas(void);
void		draw_mousefun(char *left, char *middle, char *right);
void		clear_mousefun(void);
void		notused_middle(void);
void		clear_middle(void);
void		notused_right(void);
void		clear_right(void);
