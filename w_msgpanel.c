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
#include "paintop.h"
#include "u_elastic.h"
#include "w_canvas.h"
#include "w_drawprim.h"
#include "w_util.h"
#include "w_setup.h"
#include <stdarg.h>

/********************* EXPORTS *******************/

void		put_msg(const char*, ...);

/************************  LOCAL ******************/

#define		BUF_SIZE		128
static char	prompt[BUF_SIZE];

DeclareStaticArgs(12);


extern int vsprintf (char *, const char *, va_list);

void
init_msg(TOOL tool, char *filename)
{
    /* first make a form to put the two widgets in */
    FirstArg(XtNwidth, MSGFORM_WD);
    NextArg(XtNheight, MSGFORM_HT);
    NextArg(XtNfromVert, cmd_panel);
    NextArg(XtNvertDistance, -INTERNAL_BW);
    NextArg(XtNdefaultDistance, 0);
    NextArg(XtNborderWidth, 0);
    msg_form = XtCreateManagedWidget("msg_form", formWidgetClass, tool,
				      Args, ArgCount);
    /* setup the file name widget first */
    FirstArg(XtNresizable, True);
    NextArg(XtNfont, bold_font);
    NextArg(XtNlabel, (filename!=NULL? filename: DEF_NAME));
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNborderWidth, INTERNAL_BW);
    name_panel = XtCreateManagedWidget("file_name", labelWidgetClass, msg_form,
				      Args, ArgCount);
    /* now the message panel */
    FirstArg(XtNfont, roman_font);
    NextArg(XtNstring, "\0");
    NextArg(XtNfromHoriz, name_panel);
    NextArg(XtNhorizDistance, -INTERNAL_BW);
    NextArg(XtNtop, XtChainTop);
    NextArg(XtNbottom, XtChainTop);
    NextArg(XtNborderWidth, INTERNAL_BW);
    NextArg(XtNdisplayCaret, False);
    msg_panel = XtCreateManagedWidget("message", asciiTextWidgetClass, msg_form,
				      Args, ArgCount);
}

void
setup_msg(void)
{
    Dimension ht;

    /* set the height of the message panel to the height of the file name panel */
    XtUnmanageChild(msg_panel);
    FirstArg(XtNheight, &ht);
    GetValues(name_panel);
    FirstArg(XtNheight, ht);
    SetValues(msg_panel);
    /* set the MSGFORM_HT variable so the mouse panel can be resized to fit */
    MSGFORM_HT = ht;
    XtManageChild(msg_panel);
    if (msg_win == 0)
	msg_win = XtWindow(msg_panel);
    XDefineCursor(tool_d, msg_win, null_cursor);
}

/*
 * Update the current filename in the name_panel widget (it will resize
 * automatically) and resize the msg_panel widget to fit in the remaining 
 * space, by getting the width of the command panel and subtract the new 
 * width of the name_panel to get the new width of the message panel
 */
void
update_cur_filename(char *newname)
{
	Dimension namwid;

	XtUnmanageChild(msg_form);
	XtUnmanageChild(msg_panel);
	XtUnmanageChild(name_panel);
	strcpy(cur_filename,newname);


	FirstArg(XtNlabel, newname);
	SetValues(name_panel);
	/* get the new size of the name_panel */
	FirstArg(XtNwidth, &namwid);
	GetValues(name_panel);
	MSGPANEL_WD = MSGFORM_WD-namwid;
	/* resize the message panel to fit with the new width of the name panel */
	FirstArg(XtNwidth, MSGPANEL_WD);
	SetValues(msg_panel);
	XtManageChild(msg_panel);
	XtManageChild(name_panel);

	/* now resize the whole form */
	FirstArg(XtNwidth, MSGFORM_WD);
	SetValues(msg_form);
	XtManageChild(msg_form);
}

/* VARARGS1 */
void put_msg(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vsprintf(prompt, format, ap );
    va_end(ap);
    FirstArg(XtNstring, prompt);
    SetValues(msg_panel);
}
