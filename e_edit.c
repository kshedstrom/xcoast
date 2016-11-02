/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985 by Supoj Sutanthavibul
 * Change function implemented by Frank Schmuck (schmuck@svax.cs.cornell.edu)
 * X version by Jon Tombs <jon@uk.ac.oxford.robots>
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
#include "mode.h"
#include "object.h"
#include "paintop.h"
#include "u_fonts.h"
#include "u_search.h"
#include "u_list.h"
#include "u_create.h"
#include "w_canvas.h"
#include "w_drawprim.h"
#include "w_icons.h"
#include "w_util.h"
#include "w_mousefun.h"

extern char    *panel_get_value(Widget widg);
Widget		make_popup_menu(char **entries, Cardinal nent, Widget parent, XtCallbackProc callback);

void	Quit(Widget widget, XtPointer client_data, XtPointer call_data);

void		panel_set_value(Widget widg, char *val);
static Widget	popup;
static Widget	below, beside;

DeclareStaticArgs(12);
static char	buf[64];

/* don't allow newlines in text until we handle multiple line texts */
String		text_translations =
	"<Key>Return: no-op(RingBell)\n\
	Ctrl<Key>J: no-op(RingBell)\n\
	Ctrl<Key>M: no-op(RingBell)\n\
	Ctrl<Key>X: EmptyTextKey()\n\
	Ctrl<Key>U: multiply(4)\n";

/*
static void     edit_cancel();
static XtActionsRec     edit_actions[] =
{
    {"DoneEdit", (XtActionProc) edit_cancel},
};

static void
edit_cancel(w, ev)
    Widget          w;
    XButtonEvent   *ev;
{
    cancel_button(w, NULL, NULL);
}
*/


/*
 * make a popup menu with "nent" button entries (labels) that call "callback"
 * when pressed
 */

Widget
make_popup_menu(char **entries, Cardinal nent, Widget parent, XtCallbackProc callback)
{
    Widget	    pop_menu, entry;
    int		    i;

    pop_menu = XtCreatePopupShell("menu", simpleMenuWidgetClass, parent,
				  NULL, ZERO);

    for (i = 0; i < nent; i++) {
	entry = XtCreateManagedWidget(entries[i], smeBSBObjectClass, pop_menu,
				      NULL, ZERO);
	XtAddCallback(entry, XtNcallback, callback, (XtPointer) i);
    }
    return pop_menu;
}

static
void int_panel(int x, Widget parent, char *label, Widget *pi_x)
{
    FirstArg(XtNfromVert, below);
    NextArg(XtNlabel, label);
    NextArg(XtNborderWidth, 0);
    beside = XtCreateManagedWidget(label, labelWidgetClass, parent, Args, ArgCount);

    sprintf(buf, "%d", x);
    ArgCount = 1;
    NextArg(XtNstring, buf);
    NextArg(XtNinsertPosition, strlen(buf));
    NextArg(XtNfromHoriz, beside);
    NextArg(XtNeditType, "append");
    NextArg(XtNwidth, 40);
    *pi_x = XtCreateManagedWidget(label, asciiTextWidgetClass, parent, Args, ArgCount);
    below = *pi_x;
}

/* ARGSUSED */
void
Quit(Widget widget, XtPointer client_data, XtPointer call_data)
{
    XtDestroyWidget(popup);
}

char	       *
panel_get_value(Widget widg)
{
    char	   *val;

    FirstArg(XtNstring, &val);
    GetValues(widg);
    return val;

}

void panel_clear_value(Widget widg)
{
    FirstArg(XtNstring, " ");
    NextArg(XtNinsertPosition, 0);
    SetValues(widg);
}

void
panel_set_value(Widget widg, char *val)
{
    FirstArg(XtNstring, val);
    /* I don't know why this doesn't work? */
    /* NextArg(XtNinsertPosition, strlen(val)); */
    SetValues(widg);
    XawTextSetInsertionPoint(widg, strlen(val));
}

void clear_text_key(Widget w)
{
	panel_set_value(w, "");
}
