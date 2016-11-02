/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Brian V. Smith
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
#include "w_cursor.h"
#include "w_drawprim.h"		/* for char_height */
#include "w_dir.h"
#include "w_util.h"
#include "w_setup.h"

extern Boolean	file_msg_is_popped;
extern Widget	file_msg_popup;

extern String	text_translations;
static char	load_msg[] = "The current figure is modified.\nDo you want to discard it and load the new file?";
static char	buf[40];

DeclareStaticArgs(12);
static Widget	file_status, num_objects;
static Widget	cfile_lab, cfile_text;
static Widget	cancel, save, merge, load;
static Widget	file_w;
static Position xposn, yposn;
static String	file_name_translations =
	"<Key>Return: load()\n";
static void	file_panel_cancel(Widget w, XButtonEvent *ev), do_merge(Widget w, XButtonEvent *ev);
void		do_load(Widget w, XButtonEvent *ev);
int		do_save(Widget w);
static XtActionsRec	file_name_actions[] =
{
    {"load", (XtActionProc) do_load},
};
static String	file_translations =
	"<Message>WM_PROTOCOLS: DismissFile()\n";
static XtActionsRec	file_actions[] =
{
    {"DismissFile", (XtActionProc) file_panel_cancel},
    {"cancel", (XtActionProc) file_panel_cancel},
    {"load", (XtActionProc) do_load},
    {"save", (XtActionProc) do_save},
    {"merge", (XtActionProc) do_merge},
};

/* Global so w_dir.c can access us */

Widget		file_panel,	/* so w_dir can access the scrollbars */
		file_popup,	/* the popup itself */
		file_selfile,	/* selected file widget */
		file_mask,	/* mask widget */
		file_dir,	/* current directory widget */
		file_flist,	/* file list wiget */
		file_dlist;	/* dir list wiget */

Boolean		file_up = False;


extern int emptyname (char *name);
extern int emptyname_msg (char *name, char *msg);
extern int merge_file (char *file);
extern int emptyfigure (void);
extern int popup_query (int query_type, char *message);
extern int change_directory (char *path);
extern int load_file (char *file);
extern int update_cur_filename (char *newname);
extern int emptyfigure_msg (char *msg);
extern int write_file (char *file_name);
void create_file_panel (Widget w);
extern void Rescan (Widget widget, XEvent *event, String *params, Cardinal *num_params);
extern int object_count (F_compound *list);

static void
file_panel_dismiss(void)
{
    FirstArg(XtNstring, "\0");
    SetValues(file_selfile);	/* clear Filename string */
    XtPopdown(file_popup);
    XtSetSensitive(file_w, True);
    file_up = False;
}

/* ARGSUSED */
static void
do_merge(Widget w, XButtonEvent *ev)
{
    char	    filename[100];
    char	   *fval, *dval;

    FirstArg(XtNstring, &fval);
    GetValues(file_selfile);	/* check the ascii widget for a filename */
    if (emptyname(fval))
	fval = cur_filename;	/* "Filename" widget empty, use current filename */

    if (emptyname_msg(fval, "MERGE"))
	return;

    FirstArg(XtNstring, &dval);
    GetValues(file_dir);

    strcpy(filename, dval);
    strcat(filename, "/");
    strcat(filename, fval);
    if (merge_file(filename) == 0)
	file_panel_dismiss();
}

/* ARGSUSED */
void
do_load(Widget w, XButtonEvent *ev)
{
    char	   *fval, *dval;

    if (file_popup) {
	FirstArg(XtNstring, &dval);
	GetValues(file_dir);
	FirstArg(XtNstring, &fval);
	GetValues(file_selfile);        /* check the ascii widget for a filename */
	if (emptyname(fval))
	    fval = cur_filename;        /* "Filename" widget empty, use current filename */

	if (emptyname_msg(fval, "LOAD"))
	    return;

	if (!emptyfigure() && figure_modified) {
	    extern int emptyfigure (void);
	    XtSetSensitive(load, False);
	    if (!popup_query(QUERY_YES, load_msg)) {
		extern int popup_query (int query_type, char *message);
		XtSetSensitive(load, True);
		return;
	    }
	    XtSetSensitive(load, True);
	}
	if (change_directory(dval) == 0) {
	    extern int change_directory (char *path);
	    if (load_file(fval) == 0) {
		extern int load_file (char *file);
		FirstArg(XtNlabel, fval);
		SetValues(cfile_text);          /* set the current filename */
		if (fval != cur_filename)
		    update_cur_filename(fval);      /* and update cur_filename */
		file_panel_dismiss();
	    }
	}
    } else {
	(void) load_file(cur_filename);
    }
}

/* ARGSUSED */
int
do_save(Widget w)
{
    char	   *fval, *dval;

    if (emptyfigure_msg("Save"))
	return (0);

    if (file_popup) {
	FirstArg(XtNstring, &fval);
	GetValues(file_selfile);	/* check the ascii widget for a filename */
	if (emptyname(fval)) {
	    extern int emptyname (char *name);
	    fval = cur_filename;	/* "Filename" widget empty, use current filename */
	    warnexist = False;          /* don't warn if this file exists */
	/* copy the name from the file_name widget to the current filename */
	} else
	    {
	    warnexist = True;
	    }

	if (emptyname_msg(fval, "Save"))
	    return (0);

	/* get the directory from the ascii widget */
	FirstArg(XtNstring, &dval);
	GetValues(file_dir);

	if (change_directory(dval) == 0) {
	    extern int change_directory (char *path);
	    XtSetSensitive(save, False);
	    if (write_file(fval) == 0) {
		extern int write_file (char *file_name);
		FirstArg(XtNlabel, fval);
		SetValues(cfile_text);
		if (strcmp(fval,cur_filename) != 0) {
		    update_cur_filename(fval);	/* update cur_filename */
		}
		reset_modifiedflag();
		file_panel_dismiss();
	    }
	    XtSetSensitive(save, True);
	}
    } else {
	/* not using popup => filename not changed so ok to write existing file */
	warnexist = False;			
	if (write_file(cur_filename) == 0)
	    reset_modifiedflag();
    }
    return (0);
}

/* ARGSUSED */
static void
file_panel_cancel(Widget w, XButtonEvent *ev)
{
    file_panel_dismiss();
}

void
popup_file_panel(Widget w)
{
    extern Atom     wm_delete_window;

    set_temp_cursor(wait_cursor);
    XtSetSensitive(w, False);
    file_up = True;

    if (!file_popup)
        create_file_panel(w);
    else
        Rescan(0, 0, 0, 0);

    FirstArg(XtNlabel, (figure_modified ? "      File Status: Modified    " :
                                          "      File Status: Not modified"));
    SetValues(file_status);
    sprintf(buf, "Number of Objects: %d", object_count(&objects));
    FirstArg(XtNlabel, buf);
    SetValues(num_objects);
    XtPopup(file_popup, XtGrabNonexclusive);
    (void) XSetWMProtocols(XtDisplay(file_popup), XtWindow(file_popup),
                         &wm_delete_window, 1);
    if (file_msg_is_popped)
        XtAddGrab(file_msg_popup, False, False);
    reset_cursor();
}

/* ARGSUSED */
void create_file_panel(Widget w)
{
	Widget             file, beside, below;
	PIX_FONT           temp_font;
	static int	   actions_added=0;
	file_w = w;
	XtTranslateCoords(w, (Position) 0, (Position) 0, &xposn, &yposn);

	FirstArg(XtNx, xposn);
	NextArg(XtNy, yposn + 50);
	NextArg(XtNtitle, "Xcoast: File menu");
	file_popup = XtCreatePopupShell("xfig_file_menu",
					transientShellWidgetClass,
					tool, Args, ArgCount);
	XtOverrideTranslations(file_popup,
			   XtParseTranslationTable(file_translations));

	file_panel = XtCreateManagedWidget("file_panel", formWidgetClass,
					   file_popup, NULL, ZERO);

	FirstArg(XtNlabel, "");
	NextArg(XtNwidth, 400);
	NextArg(XtNjustify, XtJustifyLeft);
	NextArg(XtNborderWidth, 0);
	NextArg(XtNresize, False);
	file_status = XtCreateManagedWidget("file_status", labelWidgetClass,
					    file_panel, Args, ArgCount);

	FirstArg(XtNlabel, "");
	NextArg(XtNwidth, 400);
	NextArg(XtNfromVert, file_status);
	NextArg(XtNjustify, XtJustifyLeft);
	NextArg(XtNborderWidth, 0);
	NextArg(XtNresize, False);
	num_objects = XtCreateManagedWidget("num_objects", labelWidgetClass,
					    file_panel, Args, ArgCount);

	FirstArg(XtNlabel, " Current Filename:");
	NextArg(XtNfromVert, num_objects);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNjustify, XtJustifyLeft);
	NextArg(XtNborderWidth, 0);
	cfile_lab = XtCreateManagedWidget("cur_file_label", labelWidgetClass,
					  file_panel, Args, ArgCount);

	FirstArg(XtNlabel, cur_filename);
	NextArg(XtNfromVert, num_objects);
	NextArg(XtNfromHoriz, cfile_lab);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNjustify, XtJustifyLeft);
	NextArg(XtNborderWidth, 0);
	NextArg(XtNwidth, 250);
	cfile_text = XtCreateManagedWidget("cur_file_name", labelWidgetClass,
					   file_panel, Args, ArgCount);

	FirstArg(XtNlabel, "         Filename:");
	NextArg(XtNvertDistance, 15);
	NextArg(XtNfromVert, cfile_lab);
	NextArg(XtNborderWidth, 0);
	file = XtCreateManagedWidget("file_label", labelWidgetClass,
				     file_panel, Args, ArgCount);
	FirstArg(XtNfont, &temp_font);
	GetValues(file);

	FirstArg(XtNwidth, 350);
	NextArg(XtNheight, char_height(temp_font) * 2 + 4);
	NextArg(XtNeditType, "edit");
	NextArg(XtNstring, cur_filename);
	NextArg(XtNinsertPosition, strlen(cur_filename));
	NextArg(XtNfromHoriz, file);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNfromVert, cfile_lab);
	NextArg(XtNscrollHorizontal, XawtextScrollWhenNeeded);
	file_selfile = XtCreateManagedWidget("file_name", asciiTextWidgetClass,
					     file_panel, Args, ArgCount);
	XtOverrideTranslations(file_selfile,
			   XtParseTranslationTable(text_translations));

	if (!actions_added) {
	    XtAppAddActions(tool_app, file_actions, XtNumber(file_actions));
	    actions_added = 1;
	    /* add action to load file */
	    XtAppAddActions(tool_app, file_name_actions, XtNumber(file_name_actions));
	}

	/* make <return> in the filename window load the file */
	XtOverrideTranslations(file_selfile,
			   XtParseTranslationTable(file_name_translations));

	create_dirinfo(file_panel, file_selfile, &beside, &below,
		       &file_mask, &file_dir, &file_flist, &file_dlist);
	/* make <return> in the file list window load the file */
	XtOverrideTranslations(file_flist,
			   XtParseTranslationTable(file_name_translations));
	FirstArg(XtNlabel, "Cancel");
	NextArg(XtNvertDistance, 15);
	NextArg(XtNhorizDistance, 25);
	NextArg(XtNheight, 25);
	NextArg(XtNfromHoriz, beside);
	NextArg(XtNfromVert, below);
	NextArg(XtNborderWidth, INTERNAL_BW);
	cancel = XtCreateManagedWidget("cancel", commandWidgetClass,
				       file_panel, Args, ArgCount);
	XtAddEventHandler(cancel, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)file_panel_cancel, (XtPointer) NULL);

	FirstArg(XtNlabel, "Save");
	NextArg(XtNfromHoriz, cancel);
	NextArg(XtNfromVert, below);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNhorizDistance, 25);
	NextArg(XtNheight, 25);
	NextArg(XtNborderWidth, INTERNAL_BW);
	save = XtCreateManagedWidget("save", commandWidgetClass,
				     file_panel, Args, ArgCount);
	XtAddEventHandler(save, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)do_save, (XtPointer) NULL);

	FirstArg(XtNlabel, "Load");
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNfromHoriz, save);
	NextArg(XtNfromVert, below);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNhorizDistance, 25);
	NextArg(XtNheight, 25);
	load = XtCreateManagedWidget("load", commandWidgetClass,
				     file_panel, Args, ArgCount);
	XtAddEventHandler(load, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)do_load, (XtPointer) NULL);

	FirstArg(XtNlabel, "Merge Read");
	NextArg(XtNfromHoriz, load);
	NextArg(XtNfromVert, below);
	NextArg(XtNborderWidth, INTERNAL_BW);
	NextArg(XtNvertDistance, 15);
	NextArg(XtNhorizDistance, 25);
	NextArg(XtNheight, 25);
	merge = XtCreateManagedWidget("merge", commandWidgetClass,
				      file_panel, Args, ArgCount);
	XtAddEventHandler(merge, ButtonReleaseMask, (Boolean) 0,
			  (XtEventHandler)do_merge, (XtPointer) NULL);

	XtInstallAccelerators(file_panel, cancel);
	XtInstallAccelerators(file_panel, save);
	XtInstallAccelerators(file_panel, load);
	XtInstallAccelerators(file_panel, merge);
}
