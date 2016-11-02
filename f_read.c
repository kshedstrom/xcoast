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
#include "figx.h"
#include "resources.h"
#include "object.h"
#include "mode.h"
#include "u_fonts.h"
#include "u_create.h"
#include "version.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_zoom.h"
#include <stdarg.h>

/* file popup information */
extern Widget	file_popup;	/* the popup itself */
extern Boolean	file_up;

/* so w_file.c can access */
Boolean	file_msg_is_popped=False;
Widget	file_msg_popup;

static Widget	file_msg_panel,
		file_msg_win, file_msg_dismiss;
static Boolean	first_file_msg;
static int	file_msg_length=0;
static char    *read_file_name;
static char	tmpstr[100];
static Boolean	old_file = 0;

static String	file_msg_translations =
	"<Message>WM_PROTOCOLS: DismissFileMsg()\n";
static void file_msg_panel_dismiss(Widget w, XButtonEvent *ev);
static XtActionsRec	file_msg_actions[] =
{
    {"DismissFileMsg", (XtActionProc) file_msg_panel_dismiss},
};

static char	Err_incomp[] = "Incomplete %s object at line %d.";

static F_line	  *read_lineobject(FILE *fp);
static F_spline   *read_splineobject(FILE *fp);

#define		BUF_SIZE		1024

DeclareStaticArgs(10);
char		buf[BUF_SIZE];
int		line_no;
int		num_object;
int		proto;		/* file protocol*10 */


void file_msg(const char *format, ...);
extern void put_msg(const char*, ...);
int readfp_fig(FILE *fp, F_compound *obj);
extern int sscanf(const char *, const char *, ...);
int read_objects(FILE *fp, F_compound *obj);
extern int fclose(FILE *);
int get_line(FILE *fp);
extern void map_init(void);
void skip_comment(FILE *fp);
extern int fscanf(FILE *, const char *, ...);
extern void free_linestorage(F_line *l);
void skip_line(FILE *fp);
extern void free_splinestorage(F_spline *s);
extern int fgetc(FILE *);
extern int ungetc(int, FILE *);
void popup_file_msg(void);
extern int fprintf(FILE *, const char *, ...);

void read_fail_message(char *file, int err)
{
    if (err == 0)		/* Successful read */
	return;
#ifdef ENAMETOOLONG
    else if (err == ENAMETOOLONG)
	file_msg("File name \"%s\" is too long", file);
#endif
    else if (err == ENOENT)
	file_msg("File \"%s\" does not exist", file);
    else if (err == ENOTDIR)
	file_msg("A name in the path \"%s\" is not a directory", file);
    else if (err == EACCES)
	file_msg("Read access to file \"%s\" is blocked", file);
    else if (err == EISDIR)
	file_msg("File \"%s\" is a directory", file);
    else if (err == -2) {
	file_msg("File \"%s\" is empty", file);
    } else if (err == -1) {
	/* Format error; relevant error message is already delivered */
    } else
	file_msg("File \"%s\" is not accessable; %s", file, strerror(err));
}

/**********************************************************
Read_fig returns :

     0 : successful read.
    -1 : File is in incorrect format
    -2 : File is empty
err_no : if file can not be read for various reasons

The resolution (ppi) and the coordinate system (coord_sys) are
stored in obj->nwcorner.x and obj->nwcorner.y respectively.
The coordinate system is 1 for lower left at 0,0 and
2 for upper left at 0,0
>>> xcoast only uses 2 for the coordinate system. <<<
**********************************************************/

int read_fig(char *file_name, F_compound *obj)
{
    FILE	   *fp;

    line_no = 0;
    read_file_name = file_name;
    first_file_msg = True;
    if ((fp = fopen(file_name, "r")) == NULL)
	return (errno);
    else {
	put_msg("Reading objects from \"%s\" ...", file_name);
	return (readfp_fig(fp, obj));
    }
}

int readfp_fig(FILE *fp, F_compound *obj)
{
    int		    status;
    float	    fproto;
    char	    tmpstr[10];
    char           *ind;

    num_object = 0;
    bzero(obj, COMOBJ_SIZE);
    if (fgets(buf, BUF_SIZE, fp) == 0)	/* version */
	return -2;
    if (strncmp(buf, "#XCOAST", 6) == 0) { /* first line */
	if ((ind = index(buf, ' ')) == 0)	/* assume 1.0 */
	    proto = 10;
	else {
	    sscanf((ind + 1), "%f", &fproto);
	    proto = (fproto + .01) * 10;	/* protocol version*10 */
	}
	/* if file protocol != current protocol, give message */
	sprintf(tmpstr,"%.1f",fproto);
	if (strcmp(tmpstr,PROTOCOL_VERSION) != 0) {
	    file_msg("Converting figure from %s format to %s",tmpstr,PROTOCOL_VERSION);
	    old_file = 1;
	}
	status = read_objects(fp, obj);
    } 

    fclose(fp);
    return (status);
}

int
read_objects(FILE *fp, F_compound *obj)
{
    F_line	   *l, *ll = NULL;
    F_spline	   *s, *ls = NULL;
    int		    object, ppi, coord_sys;

    line_no++;
    if (get_line(fp) < 0) {
	int get_line (FILE *fp);
	file_msg("File is truncated");
	return (-1);
    }
    if (2 != sscanf(buf, "%d%d\n", &ppi, &coord_sys)) {
	extern int sscanf (const char *, const char *, ...);
	file_msg("Incomplete data at line %d", line_no);
	return (-1);
    }
    obj->nwcorner.x = ppi;		/* hack because now always 30 */
    obj->nwcorner.y = coord_sys;
    if (get_line(fp) < 0) {
        put_msg("File is truncated");
        return(-1);
    }
    if (4 != sscanf(buf, "%d%lf%lf%lf\n", &iprj, &plat, &plon, &rota)) {
        put_msg("Incomplete data at line %d", line_no);
        return(-1);
    }
    if (get_line(fp) < 0) {
        put_msg("File is truncated");
        return(-1);
    }
    if (4 != sscanf(buf, "%lf%lf%lf%lf\n", &rlat1, &rlon1, &rlat2, &rlon2)) {
        put_msg("Incomplete data at line %d", line_no);
        return(-1);
    }
    map_init();

    while (get_line(fp) > 0) {
	if (1 != sscanf(buf, "%d", &object)) {
	    file_msg("Incorrect format at line %d", line_no);
	    return (-1);
	}
	switch (object) {
	case O_POLYLINE:
	    if ((l = read_lineobject(fp)) == NULL)
		return (-1);
	    if (ll)
		ll = (ll->next = l);
	    else
		ll = obj->lines = l;
	    num_object++;
	    break;
	case O_SPLINE:
	    if ((s = read_splineobject(fp)) == NULL)
		return (-1);
	    if (ls)
		ls = (ls->next = s);
	    else
		ls = obj->splines = s;
	    num_object++;
	    break;
	default:
	    file_msg("Incorrect object code at line %d", line_no);
	    return (-1);
	}			/* switch */
    }				/* while */
    if (feof(fp))
	return (0);
    else
	return (errno);
}				/* read_objects */

static F_line  *
read_lineobject(FILE *fp)
{
    F_line	   *l;
    F_point	   *p, *q;
    int		    n, npts;
    double	    x, y;

    if ((l = create_line()) == NULL)
	return (NULL);

    l->points = NULL;
    l->next = NULL;

    if (old_file) {
        n = sscanf(buf, "%*d%d%d%d%d%f",
		   &l->type, &l->style, &l->thickness, &l->color,
	      &l->style_val);
	npts = 10000000;
    } else {		/* old xcoast did not have color, etc */
        n = sscanf(buf, "%*d%d%d%d%d%f%d",
		   &l->type, &l->style, &l->thickness, &l->color,
	      &l->style_val, &npts);
    }
    skip_comment(fp);

    /* points start on new line */
    line_no++;
    if ((p = create_point()) == NULL)
	return (NULL);

    l->points = p;
    p->next = NULL;

    /* read first point */
    if (fscanf(fp, "%lf%lf", &p->x, &p->y) != 2) {
	file_msg(Err_incomp, "line", line_no);
	free_linestorage(l);
	return (NULL);
    }
    if (old_file) {
	p->x *= ZOOM_FACTOR;
	p->y *= ZOOM_FACTOR;
    }
    /* read subsequent points */
    for (--npts; npts > 0; --npts) {
	if (fscanf(fp, "%lf%lf", &x, &y) != 2) {
	    file_msg(Err_incomp, "line", line_no);
	    free_linestorage(l);
	    return (NULL);
	}
	if (old_file && x > 9998)
	    break;
	if ((q = create_point()) == NULL) {
	    free_linestorage(l);
	    return (NULL);
	}
	if (old_file) {
	    x *= ZOOM_FACTOR;
	    y *= ZOOM_FACTOR;
	}
	q->x = x;
	q->y = y;
	q->next = NULL;
	p->next = q;
	p = q;
    }
    skip_line(fp);
    return (l);
}

static F_spline *
read_splineobject(FILE *fp)
{
    F_spline	   *s;
    F_point	   *p, *q;
    F_control	   *cp, *cq;
    int		    c, n, npts;
    double	    x, y;
    float	    lx, ly, rx, ry;

    if ((s = create_spline()) == NULL)
	return (NULL);

    s->points = NULL;
    s->controls = NULL;
    s->next = NULL;

    if (old_file) {
        n = sscanf(buf, "%*d%d%d%d%d%f",
		   &s->type, &s->style, &s->thickness, &s->color,
	      &s->style_val);
	npts = 10000000;
    } else {		/* old xcoast did not have color, etc */
        n = sscanf(buf, "%*d%d%d%d%d%f%d",
		   &s->type, &s->style, &s->thickness, &s->color,
	      &s->style_val, &npts);
    }
    skip_comment(fp);
    line_no++;

    /* Read first point */
    if ((n = fscanf(fp, "%lf%lf", &x, &y)) != 2) {
	file_msg(Err_incomp, "spline", line_no);
	free_splinestorage(s);
	return (NULL);
    };
    if ((p = create_point()) == NULL) {
	free_splinestorage(s);
	return (NULL);
    }
    if (old_file) {
	x *= ZOOM_FACTOR;
	y *= ZOOM_FACTOR;
    }
    s->points = p;
    p->x = x;
    p->y = y;
    c = 1;
    for (--npts; npts > 0; --npts) {
	if (fscanf(fp, "%lf%lf", &x, &y) != 2) {
	    file_msg(Err_incomp, "spline", line_no);
	    p->next = NULL;
	    free_splinestorage(s);
	    return (NULL);
	};
	if (old_file && x > 9998)
	    break;
	if ((q = create_point()) == NULL) {
	    free_splinestorage(s);
	    return (NULL);
	}
	if (old_file) {
	    x *= ZOOM_FACTOR;
	    y *= ZOOM_FACTOR;
	}
	q->x = x;
	q->y = y;
	p->next = q;
	p = q;
	c++;
    }
    p->next = NULL;
    skip_line(fp);

    line_no++;
    skip_comment(fp);
    /* Read controls */
    if ((n = fscanf(fp, "%f%f%f%f", &lx, &ly, &rx, &ry)) != 4) {
	file_msg(Err_incomp, "spline", line_no);
	free_splinestorage(s);
	return (NULL);
    };
    if ((cp = create_cpoint()) == NULL) {
	free_splinestorage(s);
	return (NULL);
    }
    s->controls = cp;
    if (old_file) {
	lx *= ZOOM_FACTOR;
	ly *= ZOOM_FACTOR;
	rx *= ZOOM_FACTOR;
	ry *= ZOOM_FACTOR;
    }
    cp->lx = lx;
    cp->ly = ly;
    cp->rx = rx;
    cp->ry = ry;
    while (--c) {
	if (fscanf(fp, "%f%f%f%f", &lx, &ly, &rx, &ry) != 4) {
	    file_msg(Err_incomp, "spline", line_no);
	    cp->next = NULL;
	    free_splinestorage(s);
	    return (NULL);
	};
	if ((cq = create_cpoint()) == NULL) {
	    cp->next = NULL;
	    free_splinestorage(s);
	    return (NULL);
	}
	if (old_file) {
	    lx *= ZOOM_FACTOR;
	    ly *= ZOOM_FACTOR;
	    rx *= ZOOM_FACTOR;
	    ry *= ZOOM_FACTOR;
	}
	cq->lx = lx;
	cq->ly = ly;
	cq->rx = rx;
	cq->ry = ry;
	cp->next = cq;
	cp = cq;
    }
    cp->next = NULL;

    skip_line(fp);
    return (s);
}

int get_line(FILE *fp)
{
    while (1) {
	if (NULL == fgets(buf, BUF_SIZE, fp)) {
	    return (-1);
	}
	line_no++;
	if (*buf != '\n' && *buf != '#')	/* Skip empty and comment
						 * lines */
	    return (1);
    }
}

void skip_comment(FILE *fp)
{
    char	    c;

    while ((c = fgetc(fp)) == '#')
	skip_line(fp);
    if (c != '#')
	ungetc(c, fp);
}

void skip_line(FILE *fp)
{
    while (fgetc(fp) != '\n') {
	extern int fgetc (FILE *);
	if (feof(fp))
	    return;
    }
}

/* VARARGS1 */
void file_msg(const char *format, ...)
{
    va_list ap;
    XawTextBlock block;

    va_start(ap, format);
    popup_file_msg();
    if (first_file_msg)
	{
	first_file_msg = False;
	file_msg("---------------------");
	file_msg("File %s:",read_file_name);
	}
    vsprintf(tmpstr, format, ap);
    va_end(ap);
    strcat(tmpstr,"\n");
    /* append this message to the file message widget string */
    block.firstPos = 0;
    block.ptr = tmpstr;
    block.length = strlen(tmpstr);
    block.format = FMT8BIT;
    /* make editable to add new message */
    FirstArg(XtNeditType, XawtextEdit);
    SetValues(file_msg_win);
    /* insert the new message after the end */
    (void) XawTextReplace(file_msg_win,file_msg_length,file_msg_length,&block);
    (void) XawTextSetInsertionPoint(file_msg_win,file_msg_length);

    /* make read-only again */
    FirstArg(XtNeditType, XawtextRead);
    SetValues(file_msg_win);
    file_msg_length += block.length;
}

/* ARGSUSED */
void clear_file_message(Widget w, XButtonEvent *ev)
{
    XawTextBlock	block;
    int			replcode;

    if (!file_msg_popup)
	return;

    tmpstr[0]=' ';
    block.firstPos = 0;
    block.ptr = tmpstr;
    block.length = 1;
    block.format = FMT8BIT;

    /* make editable to clear message */
    FirstArg(XtNeditType, XawtextEdit);
    NextArg(XtNdisplayPosition, 0);
    SetValues(file_msg_win);

    /* replace all messages with one blank */
    replcode = XawTextReplace(file_msg_win,0,file_msg_length,&block);
    if (replcode == XawPositionError)
	fprintf(stderr,"XawTextReplace XawPositionError\n");
    else if (replcode == XawEditError)
	fprintf(stderr,"XawTextReplace XawEditError\n");

    /* make read-only again */
    FirstArg(XtNeditType, XawtextRead);
    SetValues(file_msg_win);
    file_msg_length = 0;
}

static Bool grabbed=False;

/* ARGSUSED */
static
void
file_msg_panel_dismiss(Widget w, XButtonEvent *ev)
{
	if ((grabbed) && (!file_up))
		XtAddGrab(file_msg_popup, False, False);
	XtPopdown(file_msg_popup);
	file_msg_is_popped=False;
}

void popup_file_msg(void)
{
	extern Atom wm_delete_window;

	if (file_msg_popup)
		{
		if (!file_msg_is_popped)
			{
			if (file_up)
				{
				XtPopup(file_msg_popup, XtGrabNonexclusive);
    				XSetWMProtocols(XtDisplay(file_msg_popup), 
						XtWindow(file_msg_popup),
			       			&wm_delete_window, 1);
				grabbed = True;
				}
			else
				{
				XtPopup(file_msg_popup, XtGrabNone);
    				XSetWMProtocols(XtDisplay(file_msg_popup), 
						XtWindow(file_msg_popup),
			       			&wm_delete_window, 1);
				grabbed = False;
				}
			}
		file_msg_is_popped = True;
		return;
		}

	file_msg_is_popped = True;
	FirstArg(XtNx, 0);
	NextArg(XtNy, 0);
	NextArg(XtNtitle, "Xcoast: File error messages");
	file_msg_popup = XtCreatePopupShell("xfig_file_msg",
					transientShellWidgetClass,
					tool, Args, ArgCount);
	XtOverrideTranslations(file_msg_popup,
			XtParseTranslationTable(file_msg_translations));
	XtAppAddActions(tool_app, file_msg_actions, XtNumber(file_msg_actions));

	file_msg_panel = XtCreateManagedWidget("file_msg_panel", formWidgetClass,
					   file_msg_popup, NULL, ZERO);
	FirstArg(XtNwidth, 500);
	NextArg(XtNheight, 200);
	NextArg(XtNeditType, XawtextRead);
	NextArg(XtNdisplayCaret, False);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNscrollHorizontal, XawtextScrollWhenNeeded);
	NextArg(XtNscrollVertical, XawtextScrollAlways);
	file_msg_win = XtCreateManagedWidget("file_msg_win", asciiTextWidgetClass,
					     file_msg_panel, Args, ArgCount);

	FirstArg(XtNlabel, "Dismiss");
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNfromVert, file_msg_win);
	file_msg_dismiss = XtCreateManagedWidget("dismiss", commandWidgetClass,
				       file_msg_panel, Args, ArgCount);
	XtAddEventHandler(file_msg_dismiss, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)file_msg_panel_dismiss, (XtPointer) NULL);

	FirstArg(XtNlabel, "Clear");
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNfromVert, file_msg_win);
	NextArg(XtNfromHoriz, file_msg_dismiss);
	file_msg_dismiss = XtCreateManagedWidget("clear", commandWidgetClass,
				       file_msg_panel, Args, ArgCount);
	XtAddEventHandler(file_msg_dismiss, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)clear_file_message, (XtPointer) NULL);

	if (file_up)
		{
		XtPopup(file_msg_popup, XtGrabNonexclusive);
    		XSetWMProtocols(XtDisplay(file_msg_popup), 
				XtWindow(file_msg_popup),
			       	&wm_delete_window, 1);
		grabbed = True;
		}
	else
		{
		XtPopup(file_msg_popup, XtGrabNone);
    		XSetWMProtocols(XtDisplay(file_msg_popup), 
				XtWindow(file_msg_popup),
			       	&wm_delete_window, 1);
		grabbed = False;
		}
}
