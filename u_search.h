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

void		init_searchproc_left(void (*handlerproc) (/* ??? */));
void		init_searchproc_middle(void (*handlerproc) (/* ??? */));
void		init_searchproc_right();

void		point_search_left(int x, int y, unsigned int shift);
void		point_search_middle(int x, int y, unsigned int shift);
void		point_search_right();

void		object_search_left(int x, int y, unsigned int shift);
void		object_search_middle(int x, int y, unsigned int shift);
void		object_search_right(int x, int y, unsigned int shift);
