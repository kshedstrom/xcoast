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

#define		PIX_PER_CM		450
#define		DISPLAY_PIX_PER_CM	30
#define		PIX_PER_INCH		1200
#define		DISPLAY_PIX_PER_INCH	80

/* Square dimensions */
#define		DEF_CANVAS_SIZE		24*DISPLAY_PIX_PER_CM

#define		RULER_WD		24
#ifndef MAX_TOPRULER_WD
#define		MAX_TOPRULER_WD		1020
#endif
#ifndef MAX_SIDERULER_HT
#define		MAX_SIDERULER_HT	860
#endif
#define		MIN_MOUSEFUN_WD		240

#define		SW_PER_ROW_PORT 2	/* switches/row in mode panel */
#define		SW_PER_ROW_LAND 2	/* same for landscape mode */
#define		SW_PER_COL_PORT 19
#define		SW_PER_COL_LAND 19

#define		MODE_SW_HT	32	/* height of a mode switch icon */
#define		MODE_SW_WD	36	/* width of a mode switch icon */

#define		DEF_INTERNAL_BW		1
#define		POPUP_BW		2

extern int	TOOL_WD, TOOL_HT;
extern int	CMDPANEL_WD, CMDPANEL_HT;
extern int	MODEPANEL_WD;
extern int	MODEPANEL_SPACE;
extern int	MSGFORM_WD, MSGFORM_HT;
extern int	MSGPANEL_WD;
extern int	MOUSEFUN_WD, MOUSEFUN_HT;
extern int	INDPANEL_WD;
extern int	CANVAS_WD, CANVAS_HT;
extern int	INTERNAL_BW;
extern int	TOPRULER_WD, TOPRULER_HT;
extern int	SIDERULER_WD, SIDERULER_HT;
extern int	SW_PER_ROW, SW_PER_COL;
