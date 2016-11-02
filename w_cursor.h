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

extern void init_cursor(void);
extern void recolor_cursors(void);
extern void reset_cursor(void);
extern void set_temp_cursor(Cursor cursor);
extern void set_cursor(Cursor cursor);
