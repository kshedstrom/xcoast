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

extern float	zoomscale;
extern float	display_zoomscale;
extern int	zoomxoff;
extern int	zoomyoff;

/* convert Fig units to pixels at current zoom */
#define ZOOMX(x) round(zoomscale*((x)-zoomxoff))
#define ZOOMY(y) round(zoomscale*((y)-zoomyoff))

/* convert pixels to Fig units at current zoom */
#define BACKX(x) round(x/zoomscale+zoomxoff)
#define BACKY(y) round(y/zoomscale+zoomyoff)

#define zXDrawPoint(d,w,gc,x,y) XDrawPoint(d,w,gc,\
	      ZOOMX(x),ZOOMY(y))
#define zXDrawLine(d,w,gc,x1,y1,x2,y2)\
    XDrawLine(d,w,gc,ZOOMX(x1),ZOOMY(y1), \
	      ZOOMX(x2),ZOOMY(y2))
#define zXDrawLines(d,w,gc,p,n,m)\
    {int i;\
     XPoint *pp=(XPoint *) malloc(n * sizeof(XPoint)); \
     for(i=0;i<n;i++){pp[i].x=(short)ZOOMX(p[i].x);\
		pp[i].y=(short)ZOOMY(p[i].y);}\
     XDrawLines(d,w,gc,pp,n,m);\
     free(pp); \
    }
