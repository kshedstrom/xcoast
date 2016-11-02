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

#include <X11/Xos.h>

#if XtSpecificationRelease > 4
#include <X11/Xfuncs.h>
#else

/* The following is just a copy of X11/Xosdefs.h and X11/Xfuncs.h (and copyright
   notice).  I include it here so xfig can still be compiled under X11R4, since
   these files only comes with R5, and I'd like people to still be able to 
   compile xfig under R4. */

/*
 * O/S-dependent (mis)feature macro definitions
 *
 * $XConsortium: Xosdefs.h,v 1.7 91/07/19 23:22:19 rws Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _XOSDEFS_H_
#define _XOSDEFS_H_

/*
 * X_NOT_STDC_ENV means does not have ANSI C header files.  Lack of this
 * symbol does NOT mean that the system has stdarg.h.
 *
 * X_NOT_POSIX means does not have POSIX header files.  Lack of this
 * symbol does NOT mean that the POSIX environment is the default.
 * You may still have to define _POSIX_SOURCE to get it.
 */

#ifdef NOSTDHDRS
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif

#ifdef sony
#ifndef SYSTYPE_SYSV
#define X_NOT_POSIX
#endif
#endif

#ifdef UTEK
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif

#ifdef CRAY
#define X_NOT_POSIX
#endif

#ifdef vax
#ifndef ultrix			/* assume vanilla BSD */
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
#endif

#ifdef luna
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif

#ifdef Mips
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
  
#ifdef USL
#ifdef SYSV /* (release 3.2) */
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
#endif

#ifdef SYSV386
#ifdef SYSV
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
#endif

#ifdef MOTOROLA
#ifdef SYSV
#define X_NOT_STDC_ENV
#endif
#endif

#endif /* _XOSDEFS_H_ */
/*
 * $XConsortium: Xfuncs.h,v 1.8 91/04/17 09:27:52 rws Exp $
 * 
 * Copyright 1990 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

#ifndef _XFUNCS_H_
#define _XFUNCS_H_

#ifdef X_USEBFUNCS
void bcopy();
void bzero();
int bcmp();
#else
#if (__STDC__ && !defined(X_NOT_STDC_ENV) && !defined(sun) && !defined(macII)) || defined(SVR4) || defined(hpux) || defined(_IBMR2)
#include <string.h>
#define bcopy(b1,b2,len) memmove(b2, b1, (size_t)(len))
#define bzero(b,len) memset(b, 0, (size_t)(len))
#define bcmp(b1,b2,len) memcmp(b1, b2, (size_t)(len))
#else
#ifdef sgi
#include <bstring.h>
#else
#ifdef SYSV
#include <memory.h>
#if defined(_XBCOPYFUNC) && !defined(macII)
#define bcopy _XBCOPYFUNC
#define _XNEEDBCOPYFUNC
#endif
void bcopy();
#define bzero(b,len) memset(b, 0, len)
#define bcmp(b1,b2,len) memcmp(b1, b2, len)
#else /* bsd */
void bcopy(const void *, void *, int);
void bzero(void *, size_t);
int bcmp(const void *, const void *, int);
#endif /* SYSV */
#endif /* sgi */
#endif /* __STDC__ and relatives */
#endif /* X_USEBFUNCS */

#endif /* _XFUNCS_H_ */

#endif /* XtSpecificationRelease > 4 */

#ifndef X_NOT_STDC_ENV
#include <string.h>
#ifndef index
#define index(s,c) (strchr((s),(c)))
#endif
#ifndef rindex
#define rindex(s,c) (strrchr((s),(c)))
#endif
#else  /* X_NOT_STDC_ENV IS defined */
#ifdef SYSV
#include <string.h>
#define index strchr
#define rindex strrchr
#else  /* NOT SYSV */
#include <strings.h>
#define strchr index
#define strrchr rindex
#endif  /* SYSV */
#endif  /* X_NOT_STDC_ENV */

#include <sys/stat.h>

#if defined(__convex__) && defined(__STDC__)
#define S_IFDIR _S_IFDIR
#define S_IWRITE _S_IWRITE
#endif

#ifndef SYSV
#ifndef SVR4
#include <fcntl.h>
#endif
#endif

#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

extern int	errno;

#include <math.h>	/* for sin(), cos() etc */

#if defined(SYS) && defined(SYSV386)
#if defined(__STDC__)
#ifdef ISC
extern double atof(char const *);
#endif  /* ISC */
#ifdef SCO
extern double atof(const char *);
#else  /* NOT SCO */
extern double atof();
#endif /* SCO */
#else  /* NOT __STDC__ */
extern double atof();
#endif /* __STDC__ */
#else  /* NOT defined(SYS) && defined(SYSV386) */
#ifdef X_NOT_STDC_ENV
#if defined(sun) && !defined(sparc) || defined(titan)
extern double atof();
extern char *getenv();
#endif /* (sun) !(sparc) (titan) */
#else  /* NOT X_NOT_STDC_ENV */
#include <stdlib.h>	/* for atof() and getenv() */
#endif /* X_NOT_STDC_ENV */
#endif /* defined(SYS) && defined(SYSV386) */

#if defined(SYSV) || defined(SVR4) || defined(__osf__) || defined(USE_DIRENT)
#define u_int uint
#define USE_DIRENT
#define DIRSTRUCT	struct dirent
#else
#define DIRSTRUCT	struct direct
#endif

/* define PATH_MAX if not already defined */
/* taken from the X11R5 server/os/osfonts.c file */
#ifndef X_NOT_POSIX
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#if !defined(sun) || defined(sparc)
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif /* !defined(sun) || defined(sparc) */
#endif /* _POSIX_SOURCE */
#endif /* X_NOT_POSIX */

#ifndef PATH_MAX
#include <sys/param.h>
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif /* MAXPATHLEN */
#endif /* PATH_MAX */

#ifndef M_PI
#define M_PI	3.14159265358979323846
#define M_PI_2	1.57079632679489661923
#endif

#ifndef M_2PI
#define M_2PI	6.28318530717958647692
#endif

#define		min2(a, b)	(((a) < (b)) ? (a) : (b))
#define		max2(a, b)	(((a) > (b)) ? (a) : (b))
#define		min3(a,b,c)	((((a<b)?a:b)<c)?((a<b)?a:b):c)
#define		max3(a,b,c)	((((a>b)?a:b)>c)?((a>b)?a:b):c)
#define		round(a)	(int)(((a)<0.0)?(a)-.5:(a)+.5)
#define		signof(a)	(((a) < 0) ? -1 : 1)

#define		DEF_NAME	"unnamed.fig"

/* include only Xlib.h and Intrinsic.h here - use figx.h for widget stuff */

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#ifdef NOSTRSTR
extern char *strstr();
#endif
