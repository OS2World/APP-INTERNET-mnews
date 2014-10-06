/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1993-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Compatible define
 *  File        : compat.h
 *  Version     : 1.21
 *  First Edit  : 1993-11/18
 *  Last  Edit  : 1997-11/24
 *  Author      : MSR24 宅間 顯
 *
 */

#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#if	!defined(LUNA) && !defined(SUNOS_40) && !defined(SUNOS_3X) && !defined(SVR3)
#include	<stdlib.h>
#endif	/* (!LUNA && !SUNOS_40 && !SUNOS_3X && !SVR3) */
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#ifdef	_POSIX_SOURCE
#define	S_IEXEC		S_IXUSR
#define	S_IWRITE	S_IWUSR
#define	S_IREAD		S_IRUSR
#endif	/* _POSIX_SOURCE */
#if	defined(SVR3) || defined(SVR4)
#include	<memory.h>
#endif	/* (SVR3 || SVR4) */

#if	defined(LUNA) || defined(SUNOS_41) || defined(SUNOS_40) || defined(SUNOS_3X)
extern char	*memmove();
extern char	*memset();
#endif	/* (LUNA || SUNOS_41 || SUNOS_40 || SUNOS_3X) */

#ifndef	include_dirent
#define	include_dirent

#ifdef	MSDOS
#define	strnamecmp	strcasecmp
#define	strnnamecmp	strncasecmp
#define	CMD_BUFF	128
#include	<conio.h>
#ifdef	__TURBOC__
#define	strcasecmp	stricmp
#define	strncasecmp	strnicmp
#define	cprintf		printf
#ifdef	__BORLANDC__
#include	<process.h>	/* getpid() */
#else	/* !__BORLANDC__ */
#include	<dir.h>
int	getpid();
#endif	/* !__BORLANDC__ */
#endif	/* __TURBOC__ */
#if	defined(__BORLANDC__) || defined(__GO32__) || defined(X68K)
#include	<dirent.h>
#endif	/* (__BORLANDC__ || __GO32__ || X68K) */
#else	/* !MSDOS */
#define	CMD_BUFF	1024
#define	strnamecmp	strcmp
#define	strnnamecmp	strncmp
#define	cprintf		printf
#ifdef	OS2
#include	<conio.h>
#define	strcasecmp	stricmp
#define	strncasecmp	strnicmp
#endif	/* OS2 */
#include	<pwd.h>
#include	<sys/time.h>
#if	defined(NEXT) || defined(LUNA) || defined(SUNOS_3X)
#include	<sys/dir.h>
#else	/* !(NEXT || LUNA || SUNOS_3X) */
#include	<dirent.h>
#endif	/* !(NEXT || LUNA || SUNOS_3X) */
#ifndef	NEXT
#include	<unistd.h>
#endif	/* !NEXT */
#endif	/* !MSDOS */
#endif	/* !include_dirent */

#include	<errno.h>
#include	<time.h>
#ifdef	LUNA
extern int	errno;		/* errno.h にこれがないなんて… */
#endif	/* LUNA */

#ifdef	__STDC__
#include	<stdarg.h>
#else	/* !__STDC__ */
#include	<varargs.h>
#endif	/* !__STDC__ */

#ifdef	RESOLVE
#include	<netdb.h>
#endif	/* RESOLVE */

#if	defined(SVR3) || defined(SVR4) || defined(MSDOS) || defined(OS2)
#define	TERMIO
#endif	/* (SVR3 || SVR4 || MSDOS || OS2) */

#ifndef	PRINTF_ZERO
#define	printf2			printf
#define	fprintf2		fprintf
#endif	/* !PRINTF_ZERO */

#if	defined(SVR3) || defined(IRIX) || defined(OS2)
#define	vfork()	fork()
#endif	/* (SVR3 || IRIX || OS2) */

#ifdef	ALPHA
typedef int	TIMEVALUE;
typedef	long	CASTPTR;
#else	/* !ALPHA */
typedef long	TIMEVALUE;
#if	defined(MSDOS) && !defined(__GO32__)
typedef	long	CASTPTR;
#else	/* !(MSDOS || !__GO32__) */
typedef	int	CASTPTR;
#endif	/* !(MSDOS || !__GO32__) */
#endif	/* !ALPHA */

#include	<signal.h>

#ifdef	MSDOS
#define	HAVE_FILES
#ifdef	X68K
typedef struct dirent	DIR_PTR;

#include	<sys/dos.h>
#include	<sys/iocs.h>
#include	<unistd.h>
#define	FILE_ATTR		(_DOS_IFREG | _DOS_IHIDDEN | _DOS_IRDONLY)
#define	DIR_ATTR		_DOS_IFDIR
#define	HIDE_ATTR		_DOS_IHIDDEN
#define	dos_files(f, d, a)	_dos_files((d), (f), (a))
#define	dos_nfiles(d)		_dos_nfiles((d))
typedef struct _filbuf	FILES_STRUCT;
#define	SLASH_CHAR	'/'
#define	large_malloc	malloc
#define	large_realloc	realloc
#define	large_free	free
#else	/* !X68K */
#include	<dos.h>
#ifdef	DOS_PIPE
#include	"lib.h"
#else	/* !DOS_PIPE */
FILE	*popen();
int	pclose();
#endif	/* !DOS_PIPE */
#define	SLASH_CHAR	'\\'
#ifdef	__GO32__
#define	large_malloc	malloc
#define	large_realloc	realloc
#define	large_free	free
#define	cprintf		printf
typedef struct dirent	DIR_PTR;
#undef	HAVE_FILES
#else	/* !__GO32__ */
extern void	*large_malloc();
extern void	*large_realloc();
extern void	large_free();
#endif	/* !__GO32__ */
#ifdef	MSC
#define	FILE_ATTR		(_A_NORMAL | _A_RDONLY | _A_HIDDEN)
#define	DIR_ATTR		_A_SUBDIR
#define	HIDE_ATTR		_A_HIDDEN
#define	dos_files(f, d, a)	_dos_findfirst((f), (a), (d))
#define	dos_nfiles(d)		_dos_findnext((d))
typedef struct find_t	FILES_STRUCT;
#endif	/* MSC */
#ifdef	__TURBOC__
#define	FILE_ATTR		(_A_NORMAL | _A_RDONLY | _A_HIDDEN)
#define	DIR_ATTR		_A_SUBDIR
#define	HIDE_ATTR		_A_HIDDEN
#define	dos_files(f, d, a)	findfirst((f), (d), (a))
#define	dos_nfiles(d)		findnext((d))
typedef struct ffblk	FILES_STRUCT;
#ifdef	__BORLANDC__
#define	HAVE_OPENDIR
typedef struct dirent DIR_PTR;
#endif	/* __BORLANDC__ */
#endif	/* __TURBOC__ */
/*
 * MS-DOS 版で opendir を使いたい人は以下の #undef を有効にして
 * 下さい。
 */
/*
#undef	HAVE_FILES
*/

#ifndef	HAVE_FILES
#ifndef	__BORLANDC__
#ifndef	__GO32__
#include	"msd_dir.h"
#define	signal(t, f)
#endif	/* __GO32__ */
#endif	/* !__BORLANDC__ */
#endif	/* !HAVE_FILES */
#endif	/* !X68K */
#define	kill(p, t)
#define	chmod(f, p)
#define	PATH_BUFF	128
#else	/* !MSDOS */
#define	HAVE_OPENDIR
#if	defined(NEXT) || defined(LUNA) || defined(SUNOS_3X)
typedef struct direct	DIR_PTR;
#else	/* !(NEXT || LUNA || SUNOS_3X) */
typedef struct dirent	DIR_PTR;
#endif	/* !(NEXT || LUNA || SUNOS_3X) */

#define	large_malloc	malloc
#define	large_realloc	realloc
#define	large_free	free
#ifdef	OS2
#define	SLASH_CHAR	'\\'
#else	/* !OS2 */
#define	SLASH_CHAR	'/'
#endif	/* !OS2 */
#define	PATH_BUFF	256
#endif	/* !MSDOS */

#ifndef	S_ISDIR
#define	S_ISDIR(stat)		(((stat) & S_IFMT) == S_IFDIR)
#endif	/* !S_ISDIR */

#ifdef	MSDOS
#define	create_backup_name(bak, org)	create_backup_DOS(bak, org)
#else	/* !MSDOS */
#ifdef	SVR3
#define	create_backup_name(bak, org)	create_backup_SVR3(bak, org)
#else	/* !SVR3 */
#define	create_backup_name(bak, org)	sprintf(bak, "%s~", org)
#endif	/* !SVR3 */
#endif	/* !MSDOS */
#ifdef	__STDC__
extern char	*separater(char*);
#else	/* !__STDC__ */
extern char	*separater();
#endif	/* !__STDC__ */

#ifndef	BUFF_SIZE
#define	BUFF_SIZE	1024
#endif	/* !BUFF_SIZE */
#ifndef	SMALL_BUFF
#define	SMALL_BUFF	256
#endif	/* !SMALL_BUFF */
