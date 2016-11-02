/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Henning Spruth
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both the copyright
 * notice and this permission notice appear in supporting documentation. 
 * No representations are made about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or implied warranty."
 */

#include "fig.h"
#include "resources.h"
#include "mode.h"
#include "w_zoom.h"

extern int	pan_jump;


extern void reset_topruler(void);
extern void redisplay_topruler(void);
extern void reset_sideruler(void);
extern void redisplay_sideruler(void);
extern void redisplay_canvas(void);
extern void setup_sideruler(void);

void pan_left(void)
{
    zoomxoff += pan_jump;
    reset_topruler();
    redisplay_topruler();
    redisplay_canvas();
}

void pan_right(void)
{
    if (zoomxoff == 0)
	return;
    zoomxoff -= pan_jump;
    if (zoomxoff < 0)
	zoomxoff = 0;
    reset_topruler();
    redisplay_topruler();
    redisplay_canvas();
}

void pan_up(void)
{
    zoomyoff += pan_jump;
    reset_sideruler();
    redisplay_sideruler();
    redisplay_canvas();
}

void pan_down(void)
{
    if (zoomyoff == 0)
	return;
    zoomyoff -= pan_jump;
    if (zoomyoff < 0)
	zoomyoff = 0;
    reset_sideruler();
    redisplay_sideruler();
    redisplay_canvas();
}

void pan_origin(void)
{
    if (zoomxoff == 0 && zoomyoff == 0)
	return;
    if (zoomyoff != 0) {
	zoomyoff = 0;
	setup_sideruler();
	redisplay_sideruler();
    }
    if (zoomxoff != 0) {
	zoomxoff = 0;
	reset_topruler();
	redisplay_topruler();
    }
    redisplay_canvas();
}
