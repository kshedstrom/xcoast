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
#include "mode.h"
#include "resources.h"

#define MAXERRORS 6
#define MAXERRMSGLEN 512

static int	error_cnt = 0;


extern int fprintf (FILE *, const char *, ...);
void emergency_quit (void);
extern int emptyfigure (void);
extern int emergency_save (char *file_name);
extern void quit (Widget w);
extern void free_GCs (void);

void
error_handler(int err_sig)
{
    switch (err_sig) {
    case SIGHUP:
	fprintf(stderr, "\nxcoast: SIGHUP signal trapped\n");
	break;
    case SIGFPE:
	fprintf(stderr, "\nxcoast: SIGFPE signal trapped\n");
	break;
#ifndef NO_SIBGUS
    case SIGBUS:
	fprintf(stderr, "\nxcoast: SIGBUS signal trapped\n");
	break;
#endif
    case SIGSEGV:
	fprintf(stderr, "\nxcoast: SIGSEGV signal trapped\n");
	break;
    }
    emergency_quit();
}

/* ARGSUSED */
void X_error_handler(Display *d, XErrorEvent *err_ev)
{
    char	    err_msg[MAXERRMSGLEN];

    XGetErrorText(tool_d, (int) (err_ev->error_code), err_msg, MAXERRMSGLEN - 1);
    (void) fprintf(stderr,
	   "xcoast: X error trapped - error message follows:\n%s\n", err_msg);
    emergency_quit();
}

void emergency_quit(void)
{
    if (++error_cnt > MAXERRORS) {
	fprintf(stderr, "xcoast: too many errors - giving up.\n");
	exit(-1);
    }
    signal(SIGHUP, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
#ifndef NO_SIBGUS
    signal(SIGBUS, SIG_DFL);
#endif
    signal(SIGSEGV, SIG_DFL);

    aborting = 1;
    if (figure_modified && !emptyfigure()) {
	extern int emptyfigure (void);
	fprintf(stderr, "xcoast: attempting to save figure\n");
	if (emergency_save("xcoast.SAVE") == -1)
	    if (emergency_save(strcat(TMPDIR,"/xcoast.SAVE")) == -1)
		fprintf(stderr, "xcoast: unable to save figure\n");
    } else
	fprintf(stderr, "xcoast: figure empty or not modified - exiting\n");

    quit(tool);
}

/* ARGSUSED */
void my_quit(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    extern Atom wm_delete_window;
    if (event && event->type == ClientMessage &&
	event->xclient.data.l[0] != wm_delete_window)
    {
	return;
    }
    /* free all the GC's */
    free_GCs();
    emergency_quit();
}
