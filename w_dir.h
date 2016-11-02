/* This file is part of xdir, an X-based directory browser.
 *
 *	Created: 13 Aug 88
 *
 *	Win Treese
 *	Cambridge Research Lab
 *	Digital Equipment Corporation
 *	treese@crl.dec.com
 *
 *        COPYRIGHT 1990
 *      DIGITAL EQUIPMENT CORPORATION
 *       MAYNARD, MASSACHUSETTS
 *      ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF
   THIS SOFTWARE
 * FOR ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR
   IMPLIED
 * WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 *    Modified: 4 Dec 91 - Paul King (king@cs.uq.oz.au)
 */


/* From the C library. */

char	       *re_comp();

/* Useful constants. */

#define EOS	'\0'		/* End-of-string. */

#define NENTRIES	100	/* chunk size for allocating filename space */

/* Useful macros. */

#define streq(a, b)	(! strcmp((a), (b)))

extern void	create_dirinfo(Widget parent, Widget below, Widget *ret_beside, Widget *ret_below, Widget *mask_w, Widget *dir_w, Widget *flist_w, Widget *dlist_w);

/* Xdir function declarations. */

Boolean		MakeFileList(char *dir_name, char *mask, char ***dir_list, char ***file_list);
char	       *SaveString(char *string);
void		MakeFullPath(char *root, char *filename, char *pathname);
Boolean		IsDirectory(char *root, char *path);
