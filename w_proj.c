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
#include "w_drawprim.h"		/* for char_height */
#include "w_dir.h"
#include "w_mousefun.h"
#include "w_util.h"
#include "w_setup.h"

extern char    *panel_get_value(Widget widg);
extern void     map_init(void), draw_coast(void);
extern String	text_translations;

static char    *proj_items[] = {
    "Mercator       ", "Conformal Conic", "Stereographic  ",
    "Fast Mercator  "};
static char    *lat_string[] = {
    "Latitude =    ", "Latitude #1 = ", "Latitude =    ",
    "Latitude =    "};
static char    *rota_string[] = {
    "Rotation =    ", "Latitude #2 = ", "Rotation =    ",
    "Rotation =    "};

DeclareStaticArgs(13);

/************************ PROJECTION PANEL ***********************/

extern Widget   make_popup_menu(char **entries, Cardinal nent, Widget parent, XtCallbackProc callback);
extern Atom     wm_delete_window;
static int      fig_proj_setting;
static void     set_sensitive(void);
static double   lat_setting, long_setting, rota_setting;
static Widget   cancel;
static Widget   proj_type_panel, proj_type_menu;
static Widget   lat_label, lat_panel, long_label, long_panel;
static Widget   rota_label, rota_panel;
static Widget   xmin_panel, ymin_panel, xmax_panel, ymax_panel;
static Widget	unit_popup, form, set, beside, below, label;


extern int put_msg (const char*, ...);

void
init_proj(void)
{
    if (strlen(cur_projection))
	iprj = 0;
}
    
static String   unit_translations =
        "<Message>WM_PROTOCOLS: QuitProj()\n";
static void     proj_panel_cancel(Widget w, XButtonEvent *ev), proj_panel_set(Widget w, XButtonEvent *ev);
static XtActionsRec     unit_actions[] =
{
    {"QuitProj", (XtActionProc) proj_panel_cancel},
    {"SetProj", (XtActionProc) proj_panel_set},
};

/* handle unit/scale settings */

static void
proj_panel_dismiss(void)
{
    XtDestroyWidget(unit_popup);
    XtSetSensitive(unitbox_sw, True);
}

/* ARGSUSED */
static void
proj_panel_cancel(Widget w, XButtonEvent *ev)
{
    proj_panel_dismiss();
}

/* ARGSUSED */
static void
proj_panel_set(Widget w, XButtonEvent *ev)
{
    iprj = fig_proj_setting;
    strncpy(cur_projection,
		proj_items[iprj],
		sizeof(cur_projection));
    put_msg("Projection chosen = %s", cur_projection);
    lat_setting = atof(panel_get_value(lat_panel));
    plat = lat_setting;
    long_setting = atof(panel_get_value(long_panel));
    plon = long_setting;
    rota_setting = atof(panel_get_value(rota_panel));
    rota = rota_setting;
    proj_panel_dismiss();
    rlat1 = atof(panel_get_value(ymin_panel));
    rlat2 = atof(panel_get_value(ymax_panel));
    rlon1 = atof(panel_get_value(xmin_panel));
    rlon2 = atof(panel_get_value(xmax_panel));
    map_init();
    draw_coast();
}

static void
set_sensitive(void)
{
    char	    buf[32];

    XtVaSetValues(lat_label,
		XtNlabel, lat_string[fig_proj_setting], NULL);
    XtVaSetValues(rota_label,
		XtNlabel, rota_string[fig_proj_setting], NULL);
    sprintf(buf, "%.1lf", rota_setting);
    XtVaSetValues(rota_panel, XtNinsertPosition, strlen(buf),
			XtNstring, buf, NULL);
    XtSetSensitive(rota_label, True);
    XtSetSensitive(rota_panel, True);
    sprintf(buf, "%.1lf", lat_setting);
    XtVaSetValues(lat_panel, XtNinsertPosition, strlen(buf),
			XtNstring, buf, NULL);
    XtSetSensitive(lat_label, True);
    XtSetSensitive(lat_panel, True);
}

/* ARGSUSED */
static void
proj_type_select(Widget w, XtPointer new_unit, XtPointer garbage)
{
    FirstArg(XtNlabel, XtName(w));
    SetValues(proj_type_panel);
    fig_proj_setting = (int) new_unit;
    set_sensitive();
}

void
popup_proj_panel(void)
{
    Position	    x_val, y_val;
    Dimension	    width, height;
    char	    buf[32], buf1[32], buf2[32], buf3[32];
    char	    buf4[10], buf5[10], buf6[10], buf7[10];
    static int      actions_added=0;

    fig_proj_setting = iprj;
    lat_setting = plat;
    long_setting = plon_save;
    rota_setting = rota;

    FirstArg(XtNwidth, &width);
    NextArg(XtNheight, &height);
    GetValues(tool);
    /* position the popup 2/3 in from left and 1/3 down from top */
    XtTranslateCoords(tool, (Position) (2 * width / 3), (Position) (height / 3),
		      &x_val, &y_val);

    unit_popup = XtVaCreatePopupShell("xfig_set_proj_panel",
		transientShellWidgetClass, tool,
    		XtNx, x_val,
    		XtNy, y_val,
    		XtNwidth, 300,
		NULL);
    XtOverrideTranslations(unit_popup,
                       XtParseTranslationTable(unit_translations));
    if (!actions_added) {
        XtAppAddActions(tool_app, unit_actions, XtNumber(unit_actions));
	actions_added = 1;
    }

    form = XtVaCreateManagedWidget("form",
		formWidgetClass, unit_popup,
		NULL);

    sprintf(buf, "     Map Projections / Limits");
    label = XtVaCreateManagedWidget(buf,
		labelWidgetClass, form,
    		XtNborderWidth, 0,
		NULL);

    /* make ruler units menu */

    beside = XtVaCreateManagedWidget("Projection =  ",
		labelWidgetClass, form,
    		XtNfromVert, label,
    		XtNborderWidth, 0,
		NULL);

    proj_type_panel = XtVaCreateManagedWidget(proj_items[iprj],
		menuButtonWidgetClass, form,
    		XtNfromVert, label,
    		XtNfromHoriz, beside,
		NULL);
    below = proj_type_panel;
    proj_type_menu = make_popup_menu(proj_items, XtNumber(proj_items),
                                     proj_type_panel, proj_type_select);

    /* make latitude, longitude, rotation panels */

    lat_label = XtVaCreateManagedWidget(lat_string[iprj],
		labelWidgetClass, form,
    		XtNfromVert, below,
    		XtNborderWidth, 0,
		NULL);

    sprintf(buf1, "%.1lf", plat); 
    lat_panel = XtVaCreateManagedWidget("latitude",
		asciiTextWidgetClass, form,
    		XtNfromVert, below,
    		XtNfromHoriz, lat_label,
		XtNborderWidth, INTERNAL_BW,
		XtNstring, buf1,
		XtNinsertPosition, strlen(buf1),
		XtNeditType, "append",
		NULL);
    below = lat_panel;

    long_label = XtVaCreateManagedWidget("Longitude =   ",
		labelWidgetClass, form,
    		XtNfromVert, below,
    		XtNborderWidth, 0,
		NULL);

    sprintf(buf2, "%.1lf", plon_save); 
    long_panel = XtVaCreateManagedWidget("longitude",
		asciiTextWidgetClass, form,
    		XtNfromVert, below,
    		XtNfromHoriz, long_label,
		XtNborderWidth, INTERNAL_BW,
		XtNstring, buf2,
		XtNinsertPosition, strlen(buf2),
		XtNeditType, "append",
		NULL);
    below = long_panel;

    rota_label = XtVaCreateManagedWidget(rota_string[iprj],
		labelWidgetClass, form,
    		XtNfromVert, below,
    		XtNborderWidth, 0,
		NULL);

    sprintf(buf3, "%.1lf", rota); 
    rota_panel = XtVaCreateManagedWidget("rotation",
		asciiTextWidgetClass, form,
    		XtNfromVert, below,
    		XtNfromHoriz, rota_label,
		XtNborderWidth, INTERNAL_BW,
		XtNstring, buf3,
		XtNinsertPosition, strlen(buf3),
		XtNeditType, "append",
		NULL);
    below = rota_panel;

    /* find domain size */

    label = XtVaCreateManagedWidget("Lower left Latitude =   ",
		labelWidgetClass, form,
    		XtNfromVert, below,
    		XtNborderWidth, 0,
		NULL);

    sprintf(buf4, "%.1lf", rlat1); 
    ymin_panel = XtVaCreateManagedWidget("minmax",
		asciiTextWidgetClass, form,
    		XtNfromVert, below,
    		XtNfromHoriz, label,
		XtNborderWidth, INTERNAL_BW,
		XtNstring, buf4,
		XtNinsertPosition, strlen(buf4),
		XtNeditType, "append",
		NULL);
    below = ymin_panel;

    label = XtVaCreateManagedWidget("Lower left Longitude =  ",
		labelWidgetClass, form,
    		XtNfromVert, below,
    		XtNborderWidth, 0,
		NULL);

    sprintf(buf5, "%.1lf", rlon1); 
    xmin_panel = XtVaCreateManagedWidget("minmax",
		asciiTextWidgetClass, form,
    		XtNfromVert, below,
    		XtNfromHoriz, label,
		XtNborderWidth, INTERNAL_BW,
		XtNstring, buf5,
		XtNinsertPosition, strlen(buf5),
		XtNeditType, "append",
		NULL);
    below = xmin_panel;

    label = XtVaCreateManagedWidget("Upper right Latitude =  ",
		labelWidgetClass, form,
    		XtNfromVert, below,
    		XtNborderWidth, 0,
		NULL);

    sprintf(buf6, "%.1lf", rlat2); 
    ymax_panel = XtVaCreateManagedWidget("minmax",
		asciiTextWidgetClass, form,
    		XtNfromVert, below,
    		XtNfromHoriz, label,
		XtNborderWidth, INTERNAL_BW,
		XtNstring, buf6,
		XtNinsertPosition, strlen(buf6),
		XtNeditType, "append",
		NULL);
    below = ymax_panel;

    label = XtVaCreateManagedWidget("Upper right Longitude = ",
		labelWidgetClass, form,
    		XtNfromVert, below,
    		XtNborderWidth, 0,
		NULL);

    sprintf(buf7, "%.1lf", rlon2); 
    xmax_panel = XtVaCreateManagedWidget("minmax",
		asciiTextWidgetClass, form,
    		XtNfromVert, below,
    		XtNfromHoriz, label,
		XtNborderWidth, INTERNAL_BW,
		XtNstring, buf7,
		XtNinsertPosition, strlen(buf7),
		XtNeditType, "append",
		NULL);
    below = xmax_panel;

    /* standard cancel/set buttons */

    cancel = XtVaCreateManagedWidget("cancel",
		commandWidgetClass, form,
    		XtNlabel, "cancel",
    		XtNfromVert, below,
    		XtNborderWidth, INTERNAL_BW,
		NULL);
    XtAddEventHandler(cancel, ButtonReleaseMask, (Boolean) 0,
		      (XtEventHandler)proj_panel_cancel, (XtPointer) NULL);

    set = XtVaCreateManagedWidget("set",
		commandWidgetClass, form,
    		XtNlabel, "set",
    		XtNfromVert, below,
    		XtNfromHoriz, cancel,
    		XtNborderWidth, INTERNAL_BW,
		NULL);
    XtAddEventHandler(set, ButtonReleaseMask, (Boolean) 0,
		      (XtEventHandler)proj_panel_set, (XtPointer) NULL);

    XtPopup(unit_popup, XtGrabExclusive);

    set_sensitive();

    (void) XSetWMProtocols(XtDisplay(unit_popup), XtWindow(unit_popup),
                           &wm_delete_window, 1);
}
