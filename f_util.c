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
#include "object.h"
#include "mode.h"
#include "w_util.h"


extern int put_msg (const char*, ...);
int get_directory (char *direct);
extern int popup_query (int query_type, char *message);

int
emptyname(char *name)
{
    if (*name == '\0') {
	return (1);
    } else {
	return (0);
    }
}

int
emptyname_msg(char *name, char *msg)
{
    int		    returnval;

    if (returnval = emptyname(name))
	put_msg("No file name specified, %s command ignored", msg);
    return (returnval);
}

int
emptyfigure(void)
{
    if (objects.lines != NULL)
	return (0);
    if (objects.splines != NULL)
	return (0);
    if (objects.coasts != NULL)
	return (0);
    return (1);
}

int
emptyfigure_msg(char *msg)
{
    int		    returnval;

    if (returnval = emptyfigure())
	put_msg("Empty figure, %s command ignored", msg);
    return (returnval);
}

int
change_directory(char *path)
{
    if (path == NULL) {
	*cur_dir = '\0';
	return (0);
    }
    if (chdir(path) == -1) {
	put_msg("Can't go to directory %s, : %s", path, strerror(errno));
	return (1);
    }
    if (get_directory(cur_dir)) /* get cwd */
	return (0);
    else
	return (1);
}

#define SYSV
int get_directory(char *direct)
{
#if defined(SYSV) || defined(SVR4)
    extern char	   *getcwd();

#else
    extern char	   *getwd(char *);

#endif

#if defined(SYSV) || defined(SVR4)
    if (getcwd(direct, 1024) == NULL) {	/* get current working dir */
	put_msg("Can't get current directory");
#else
    if (getwd(direct) == NULL) {/* get current working dir */
	put_msg("%s", direct);	/* err msg is in directory */
#endif
	*direct = '\0';
	return (int)NULL;
    }
    return 1;
}

#ifndef S_IWUSR
#define S_IWUSR 0000200
#endif
#ifndef S_IWGRP
#define S_IWGRP 0000020
#endif
#ifndef S_IWOTH
#define S_IWOTH 0000002
#endif

int
ok_to_write(char *file_name, char *op_name)
{
    struct stat	    file_status;
    char	    string[180];

    if (stat(file_name, &file_status) == 0) {	/* file exists */
	if (file_status.st_mode & S_IFDIR) {
	    put_msg("\"%s\" is a directory", file_name);
	    return (0);
	}
	if (file_status.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) {
	    /* writing is permitted by SOMEONE */
	    if (access(file_name, W_OK)) {
		put_msg("Write permission for \"%s\" is denied", file_name);
		return (0);
	    } else {
		if (warnexist) {
		    sprintf(string, "\"%s\" already exists.\nDo you want to overwrite it?", file_name);
		    if (!popup_query(QUERY_YES, string)) {
			extern int popup_query (int query_type, char *message);
			put_msg("%s cancelled", op_name);
			return (0);
		    }
		}
		/* !warnexist */
		else {
			return(1);
		}
	    }
	} else {
	    put_msg("\"%s\" is read only", file_name);
	    return (0);
	}
    } else {
	if (errno != ENOENT)
	    return (0);		/* file does exist but stat fails */
    }

    return (1);
}
