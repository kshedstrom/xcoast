/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1991 by Paul King
 *
 * "Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both the copyright
 * notice and this permission notice appear in supporting documentation. 
 * No representations are made about the suitability of this software for 
 * any purpose.  It is provided "as is" without express or implied warranty."
 */

/* indicator button selection */

#define I_GRIDMODE	0x000008
#define I_GRIDSIZE	0x000010
#define I_LINEWIDTH	0x000080
#define I_LINESTYLE	0x000100
#define I_FONT		0x001000
#define I_ZOOM		0x004000
#define I_COLOR		0x020000

#define I_NONE		0x000000
#define I_ALL		0x3fffff
#define I_MIN1		(I_GRIDMODE | I_GRIDSIZE | I_ZOOM)
#define I_MIN2		(I_MIN1)
#define I_MIN3		(I_MIN2)
#define I_ADDMOVPT	(I_MIN2)
#define I_LINE0		(I_LINESTYLE | I_LINEWIDTH | I_COLOR)
#define I_OPEN		(I_MIN2 | I_LINE0)
#define I_OBJECT	(I_MIN1 | I_LINE0)
/* for checking which parts to update */
#define I_UPDATEMASK	(I_OBJECT & ~I_GRIDMODE & ~I_GRIDSIZE & ~I_ZOOM)

typedef struct choice_struct {
    int		    value;
    PIXRECT	    icon;
    Pixmap	    normalPM,blackPM;
}		choice_info;

typedef struct ind_sw_struct {
    int		    type;	/* one of I_CHOICE .. I_FVAL */
    int		    func;
    char	    line1[14], line2[6];
    int		    sw_width;
    int		   *i_varadr;
    float	   *f_varadr;
    void	    (*inc_func) ();
    void	    (*dec_func) ();
    void	    (*show_func) ();
    choice_info	   *choices;	/* specific to I_CHOICE */
    int		    numchoices; /* specific to I_CHOICE */
    int		    sw_per_row; /* specific to I_CHOICE */
    Bool	    update;	/* whether this object component is updated by update */
    TOOL	    button;
    TOOL	    formw;
    TOOL	    updbut;
    Pixmap	    normalPM;
}		ind_sw_info;

#define ZOOM_SWITCH_INDEX	0	/* used by w_zoom.c */
extern ind_sw_info ind_switches[];
