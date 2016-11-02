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
#include "resources.h"
#include "mode.h"
#include "object.h"
#include "u_coords.h"
#include "w_setup.h"

#define COAST_FILE	1
#define METERS_FILE	2
#define PROJ_FILE	3
#define LATLON_FILE	4

extern int		num_object;

extern double		pix_per_unit, meters_per_unit;
extern double		u_min, v_min;
extern double		u_max, v_max;
extern double		screen_max_x, screen_max_y;


static int write_one_file (char *file_name, int file_type);
extern int ok_to_write (char *file_name, char *op_name);
extern int put_msg (const char*, ...);
static int write_objects (FILE *fp);
static int write_meter_objects (FILE *fp);
static int write_latlon_objects (FILE *fp);
static int write_proj (FILE *fp);
extern int fprintf (FILE *, const char *, ...);
static void write_line (FILE *fp, F_line *l);
static void write_spline (FILE *fp, F_spline *s);
extern int fclose (FILE *);
static void write_meters_line (FILE *fp, F_line *l);
static void write_meters_spline (FILE *fp, F_spline *s);
static void write_latlon_line (FILE *fp, F_line *l);
static void write_latlon_spline (FILE *fp, F_spline *s);

int write_file(char *file_name)
{
    int  err_no = 0;
    char file_2[202];
    char file_3[202];
    char file_4[202];

    err_no = write_one_file(file_name, COAST_FILE);
    if (err_no != 0) return(err_no);

    strcpy(file_2, file_name);
    strcat(file_2, ".M");
    err_no = write_one_file(file_2, METERS_FILE);
    if (err_no != 0) return(err_no);

    strcpy(file_3, file_name);
    strcat(file_3, ".proj");
    err_no = write_one_file(file_3, PROJ_FILE);
    if (err_no != 0) return(err_no);

    strcpy(file_4, file_name);
    strcat(file_4, ".uv");
    err_no = write_one_file(file_4, LATLON_FILE);
    return (err_no);
}

static int
write_one_file(char *file_name, int file_type)
{
    FILE	   *fp;

    if (!ok_to_write(file_name, "SAVE"))
	return (-1);

    if ((fp = fopen(file_name, "w")) == NULL) {
	put_msg("Couldn't open file %s, %s", file_name, strerror(errno));
	return (-1);
    }
    num_object = 0;
    switch (file_type) {
    case COAST_FILE:
    	if (write_objects(fp)) {
	    put_msg("Error writing file %s, %s", file_name, strerror(errno));
	    return (-1);
        }
        put_msg("%d object(s) saved in \"%s\"", num_object, file_name);
        return (0);
    case METERS_FILE:
        if (write_meter_objects(fp)) {
            put_msg("Error writing file %s, %s",file_name,
                        strerror(errno));
            return(-1);
        }
        return(0);
    case LATLON_FILE:
        if (write_latlon_objects(fp)) {
            put_msg("Error writing file %s, %s",file_name,
                        strerror(errno));
            return(-1);
        }
        return(0);
    case PROJ_FILE:
        if (write_proj(fp)) {
            put_msg("Error writing file %s, %s",file_name,
                        strerror(errno));
            return(-1);
        }
        return(0);
    }
    return(0);
}

static int
write_objects(FILE *fp)
{
    extern char	    file_header[];
    F_line	   *l;
    F_spline	   *s;

    /*
     * Number 2 means that the origin (0,0) is at the upper left corner of
     * the screen (2nd quadrant)
     */

    put_msg("Writing . . .");
    fprintf(fp, "%s\n", file_header);
    fprintf(fp, "%d %d\n", PIX_PER_CM, 2);
    fprintf(fp, "%d %lf %lf %lf\n", iprj, plat, plon_save, rota);
    fprintf(fp, "%lf %lf %lf %lf\n", rlat1, rlon1, rlat2, rlon2);

    for (l = objects.lines; l != NULL; l = l->next) {
	num_object++;
	write_line(fp, l);
    }
    for (s = objects.splines; s != NULL; s = s->next) {
	num_object++;
	write_spline(fp, s);
    }
    if (ferror(fp)) {
	fclose(fp);
	return (-1);
    }
    if (fclose(fp) == EOF)
	return (-1);
    return (0);
}

static void
write_line(FILE *fp, F_line *l)
{
    F_point	   *p;
    int		   npts;

    if (l->points == NULL)
	return;
    /* count number of points and put it in the object */
    for (npts=0, p = l->points; p != NULL; p = p->next)
        npts++;
    fprintf(fp, "%d %d %d %d %d %.3f %d\n",
	    O_POLYLINE, l->type, l->style, l->thickness,
	 l->color, l->style_val, npts);

    fprintf(fp, "\t");
    npts=0;
    for (p = l->points; p != NULL; p = p->next) {
	fprintf(fp, " %lf %lf", p->x, p->y);
	if (++npts >= 6 && p->next != NULL)
		{
		fprintf(fp,"\n\t");
		npts=0;
		}
    };
    fprintf(fp, "\n");
}

static void
write_spline(FILE *fp, F_spline *s)
{
    F_control	   *cp;
    F_point	   *p;
    int		   npts;

    if (s->points == NULL)
	return;
    /* count number of points and put it in the object */
    for (npts=0, p = s->points; p != NULL; p = p->next)
        npts++;
    fprintf(fp, "%d %d %d %d %d %.3f %d\n",
	    O_SPLINE, s->type, s->style, s->thickness,
	    s->color, s->style_val, npts);
    fprintf(fp, "\t");
    npts=0;
    for (p = s->points; p != NULL; p = p->next) {
	fprintf(fp, " %lf %lf", p->x, p->y);
        if (++npts >= 6 && p->next != NULL) {
                fprintf(fp,"\n\t");
                npts=0;
        }
    };
    fprintf(fp, "\n");

    if (s->controls == NULL)
	return;
    fprintf(fp, "\t");
    npts=0;
    for (cp = s->controls; cp != NULL; cp = cp->next) {
	fprintf(fp, " %.3f %.3f %.3f %.3f",
		cp->lx, cp->ly, cp->rx, cp->ry);
	if (++npts >= 2 && cp->next != NULL)
		{
		fprintf(fp,"\n\t");
		npts=0;
		}
    };
    fprintf(fp, "\n");
}

static int
write_proj(FILE *fp)
{
    char            ch[3];

    switch (iprj) {
        case F_PROJ_CONIC:
            strcpy(ch, "LC");
            break;
        case F_PROJ_STEREO:
            strcpy(ch, "ST");
            break;
        case F_PROJ_MERC:
	case F_PROJ_FAST_MERC:
            strcpy(ch, "ME");
            break;
    }
    fprintf(fp, "      character*2   JPRJ, JLTS\n");
    fprintf(fp, "      real          PLAT, PLONG, ROTA, P1, P2, P3, P4, XOFF, YOFF\n");
    fprintf(fp, "      integer       JGRD\n");
    fprintf(fp, "      parameter  (  JPRJ = '%s', PLAT = %lf, &\n",ch,plat);
    fprintf(fp, "     &              PLONG = %lf, ROTA = %lf, &\n",plon_save,rota);
    fprintf(fp, "     &              JLTS = 'CO', P1 = %lf, &\n",rlat1);
    fprintf(fp, "     &              P2 = %lf, P3 = %lf, &\n",rlon1,rlat2);
    fprintf(fp, "     &              P4 = %lf, JGRD = 5)\n",rlon2);
    fprintf(fp, "      parameter  (  XOFF = 0.,  YOFF = 0.)\n");
    if (ferror(fp)) {
	fclose(fp);
	return(-1);
    }
    if (fclose(fp)==EOF) return(-1);
    return(0);
}

static int
write_meter_objects(FILE *fp)
{
    F_line          *l;
    F_spline        *s;

    for (l = objects.lines; l != NULL; l = l-> next) {
        num_object++;
        write_meters_line(fp, l);
    }
    for (s = objects.splines; s != NULL; s = s-> next) {
        num_object++;
        write_meters_spline(fp, s);
    }
    if (ferror(fp)) {
	fclose(fp);
	return(-1);
    }
    if (fclose(fp)==EOF) return(-1);
    return(0);
}

static void
write_meters_line(FILE *fp, F_line *l)
{
    F_point *p;
    int num = 0;
    double myx, myy;

    if( l->points == NULL )
        return;
    for (p = l->points; p!= NULL; p = p->next) {
        num++;
    }
    fprintf(fp, "%d\n", num);
    for (p = l->points; p!= NULL; p = p->next) {
        myx = (double)(p->x)/pix_per_unit + u_min;
        myy = (screen_max_y -(double)(p->y))/pix_per_unit + v_min;
        myx *= meters_per_unit;
        myy *= meters_per_unit;
        fprintf(fp, "   %16.6lf %16.6lf\n", myx, myy);
    }
}

static void
write_meters_spline(FILE *fp, F_spline *s)
{
    F_point         *p;
    int num = 0;
    double myx, myy;

    if( s->points == NULL )
        return;
    for (p = s->points; p!= NULL; p = p->next) {
        num++;
    }
    fprintf(fp, "%d\n", num);
    for (p = s->points; p!= NULL; p = p->next) {
        myx = (double)(p->x)/pix_per_unit + u_min;
        myy = (screen_max_y -(double)(p->y))/pix_per_unit + v_min;
        myx *= meters_per_unit;
        myy *= meters_per_unit;
        fprintf(fp, "   %16.6lf %16.6lf\n", myx, myy);
    }
}

static int
write_latlon_objects(FILE *fp)
{
    F_line          *l;
    F_spline        *s;
    char            ch[3];

    switch (iprj) {
        case F_PROJ_CONIC:
            strcpy(ch, "LC");
            break;
        case F_PROJ_STEREO:
            strcpy(ch, "ST");
            break;
        case F_PROJ_MERC:
	case F_PROJ_FAST_MERC:
            strcpy(ch, "ME");
            break;
    }

/*
 * Initialize the mapping
 */

    fprintf(fp, "%s\n", ch);
    fprintf(fp, "%lf  %lf  %lf\n", plat, plon_save, rota);
    fprintf(fp, "%lf  %lf  %lf  %lf\n", rlat1, rlon1, rlat2, rlon2);
    for (l = objects.lines; l != NULL; l = l-> next) {
        num_object++;
        write_latlon_line(fp, l);
    }
    for (s = objects.splines; s != NULL; s = s-> next) {
        num_object++;
        write_latlon_spline(fp, s);
    }

    if (ferror(fp)) {
	fclose(fp);
	return(-1);
    }
    if (fclose(fp)==EOF) return(-1);
    return(0);
}

static void
write_latlon_line(FILE *fp, F_line *l)
{
    F_point *p;
    int num = 0;
    double myx, myy;
    double uu, vv;

    if( l->points == NULL )
        return;
    for (p = l->points; p!= NULL; p = p->next) {
        num++;
    }
    for (p = l->points; p!= NULL; p = p->next) {
        myx = (double)(p->x)/pix_per_unit + u_min;
        myy = (screen_max_y -(double)(p->y))/pix_per_unit + v_min;
        myx *= meters_per_unit;
        myy *= meters_per_unit;
        uu = myx*udeg*RAD2DEG/REarth;
        vv = myy*udeg*RAD2DEG/REarth;
        fprintf(fp, "%16.12lf   %16.12lf\n", uu, vv);
    }
    fprintf(fp, "1000.   1000.\n");
}

static void
write_latlon_spline(FILE *fp, F_spline *s)
{
    F_point         *p;
    int num = 0;
    double myx, myy;
    float uu, vv;

    if( s->points == NULL )
        return;
    for (p = s->points; p!= NULL; p = p->next) {
        num++;
    }
    for (p = s->points; p!= NULL; p = p->next) {
        myx = (double)(p->x)/pix_per_unit + u_min;
        myy = (screen_max_y -(double)(p->y))/pix_per_unit + v_min;
        myx *= meters_per_unit;
        myy *= meters_per_unit;
        uu = myx*udeg*RAD2DEG/REarth;
        vv = myy*udeg*RAD2DEG/REarth;
        fprintf(fp, "%16.12lf   %16.12lf\n", uu, vv);
    }
    fprintf(fp, "1000.   1000.\n");
}

int emergency_save(char *file_name)
{
    FILE	   *fp;

    if ((fp = fopen(file_name, "w")) == NULL)
	return (-1);
    num_object = 0;
    if (write_objects(fp))
	return (-1);
    (void) fprintf(stderr, "xcoast: %d object(s) saved in \"%s\"\n",
		   num_object, file_name);
    return (0);
}
