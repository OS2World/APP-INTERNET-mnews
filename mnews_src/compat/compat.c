/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1993-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Compatible routine
 *  File        : compat.c
 *  Version     : 1.21
 *  First Edit  : 1993-12/10
 *  Last  Edit  : 1997-11/24
 *  Author      : MSR24 宅間 顯
 *
 */

#include	<stdio.h>
#if	!defined(LUNA) && !defined(SUNOS_40) && !defined(SUNOS_3X) && !defined(SVR3)
#include	<stdlib.h>
#endif	/* (!LUNA && !SUNOS_40 && !SUNOS_3X && !SVR3) */
#include        <sys/types.h>
#include	<ctype.h>
#ifdef	__STDC__
#include	<stdarg.h>
#else	/* !__STDC__ */
#include	<varargs.h>
#endif	/* !__STDC__ */
#ifdef	MSC
#include	<malloc.h>
#endif	/* MSC */
#ifdef	__TURBOC__
#include	<alloc.h>
#endif	/* __TURBOC__ */

#ifndef	BUFF_SIZE
#define	BUFF_SIZE	1024
#endif	/* !BUFF_SIZE */

#if	defined(LUNA) || defined(SUNOS_41) || defined(SUNOS_40) || defined(SUNOS_3X)
#define	DONT_HAVE_MEMMOVE
#endif	/* (LUNA || SUNOS_41 || SUNOS_40 || SUNOS_3X) */


/*
 * 文字列比較(大文字小文字区別なし)
 * strncasecmp がないマシン用
 */

#if	defined(LUNA) || defined(SVR3) || defined(SVR4) || defined(SUNOS_40) || defined(SUNOS_3X) || (defined(MSDOS) && !defined(__GO32__) && !defined(X68K))
#ifndef	STRCASECMP
#define	Toupper(c)	((unsigned char) (islower(c) ? toupper(c) : (c)))

int	strcasecmp(s1, s2)
	char	*s1;
	char	*s2;
{
  for ( ; ((unsigned char)Toupper(*s1) == (unsigned char)Toupper(*s2));
       s1++ , s2++) {
    if (*s1 == '\0') {
      return(0);
    }
  }
  return(Toupper(*s1) - Toupper(*s2));
}

int	strncasecmp(s1, s2, n)
	char	*s1;
	char	*s2;
	int	n;
{
  for ( ; (n > 1 &&
	   ((unsigned char)Toupper(*s1) == (unsigned char)Toupper(*s2)));
       s1++ , s2++ , n--) {
    if (*s1 == '\0') {
      return(0);
    }
  }
  return(Toupper(*s1) - Toupper(*s2));
}
#endif	/* !STRCASECMP */
#endif	/* (LUNA || SVR3 || SVR4 || SUNOS_40 || SUNOS_3X || (MSDOS && !__GO32__ && !X68K)) */

#ifdef	PRINTF_ZERO
#ifdef	__STDC__
int	printf2(char *fmt, ...)
#else	/* !__STDC__ */
int	printf2(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff[BUFF_SIZE];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  vsprintf(buff, fmt, ap);
  printf("%s", buff);
  va_end(ap);
  return(strlen(buff));
}

#ifdef	__STDC__
int	fprintf2(FILE *fp, char *fmt, ...)
#else	/* !__STDC__ */
int	fprintf2(fp, fmt, va_alist)
     FILE	*fp;
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff[BUFF_SIZE];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  vsprintf(buff, fmt, ap);
  fprintf(fp, "%s", buff);
  va_end(ap);
  return(strlen(buff));
}
#endif	/* PRINTF_ZERO */

#ifdef	SVR3
#ifndef	RENAME
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>

int	rename(src, dist)
     char	*src;
     char	*dist;
{
  struct stat	s1,s2;
  extern int	errno;
  
  if (stat(src, &s1) < 0) {
    return(-1);
  }

  /*
   * src はディレクトリか?
   */

  if ((s1.st_mode & S_IFMT) == S_IFDIR) {
    errno = ENOTDIR;
    return(-1);
  }

  if (stat(dist, &s2) >= 0) {

    /*
     * dist はディレクトリか?
     */

    if ((s2.st_mode & S_IFMT) == S_IFDIR) {
      errno = EISDIR;
      return(-1);
    }
    if (unlink(dist) < 0) {
      return(-1);
    }
  }
  if (link(src, dist) < 0) {
    return(-1);
  }
  if (unlink(src) < 0) {
    return(-1);
  }
  return(0);
}
#endif	/* !RENAME */

/*
 * バックアップファイルのパス名生成
 * (ファイル名に14文字制限があるマシン用)
 */

void	create_backup_SVR3(bak, org) 
     char	*bak;
     char	*org;
{ 
  register int	i, j;

  sprintf(bak, "%s~", org); 
  j = strlen(org);
  if (j-- < 14) {
    return;
  }
  for (i = j; i >= 0 && org[i] != '/'; i--) {
  }
  if ((j - i) < 14) {
    return;
  }
  strcpy(&bak[i + 14], "~");
} 
#endif	/* SVR3 */

#ifdef	DONT_HAVE_MEMMOVE
/*
 * メモリ移動
 *
 * 機  能:メモリを移動(コピー)する。
 * 引  数:char		*ptr1;	移動先
 *	  char		*ptr2;	移動元
 *	  size_t	length;	移動サイズ
 * 戻り値:char*		元の移動先
 * 備  考:memcpy との相違は memmove はオーバラップを許す点である。
 */

#ifdef	notdef
char	*memmove(ptr1, ptr2, length)
     char	*ptr1, *ptr2;
     int	length;
{
  char   *bf;
  int    i, fg;
  
  fg = 0;
  if ((unsigned long)ptr1 < (unsigned long)ptr2) {
    if (((unsigned long)ptr1 + (unsigned long)length)
	> (unsigned long)ptr2) {
      fg = 1;
    }
  } else if ((unsigned long)ptr2 < (unsigned long)ptr1) {
    if (((unsigned long)ptr2 + (unsigned long)length)
	> (unsigned long)ptr1) {
      fg = 2;
    }
  } else {
    return((int) ptr1);
  }
  
  switch (fg) {
  case 0:	/* area is'nt overrap */
    memcpy(ptr1, ptr2, length);
    break;
  case 1:	/* dist-area is first */
  case 2:	/* src-area is first */
    bf = (char *)malloc(length);
    if (bf == (char*)NULL) {
      fprintf(stderr, "Can't allocate memory for memmove.\n");
      return(-1);
    }
    memcpy(bf, ptr2, length);
    memcpy(ptr1, bf, length);
    free(bf);
    break;
  }
  return(ptr1);
}
#else	/* !notdef */
char	*memmove(ptr1, ptr2, length)
     char	*ptr1;
     char	*ptr2;
     size_t	length;
{
  char	*ptr3;
  int	overlap = 0;
  
  ptr3 = ptr1;
  if ((unsigned long)ptr1 < (unsigned long)ptr2) {
    if (((unsigned long)ptr1 + (unsigned long)length) > (unsigned long)ptr2) {
      overlap = 1;
    }
  } else if ((unsigned long)ptr2 < (unsigned long)ptr1) {
    if (((unsigned long)ptr2 + (unsigned long)length) > (unsigned long)ptr1) {
      overlap = 2;
    }
  } else {
    return(ptr1);
  }
  
  switch (overlap) {
  case 0:	/* オーバラップなし	*/
    memcpy(ptr1, ptr2, length);
    break;
  case 1:	/* 移動先 < 移動元 */
    while (length--) {
      *ptr1++ = *ptr2++;
    }
    break;
  case 2:	/* 移動先 > 移動元 */
    ptr1 += length;
    ptr2 += length;
    while (length--) {
      *--ptr1 = *--ptr2;
    }
    break;
  }
  return(ptr3);
}
#endif	/* !notdef */
#endif	/* DONT_HAVE_MEMMOVE */

#ifdef	DONT_HAVE_MEMSET
/*
 * メモリ補填
 *
 * 機  能:メモリをデータで補填(フィル)する。
 * 引  数:char		*ptr1;	補填先
 *	  int		data;	データ
 *	  size_t	length;	補填サイズ
 * 戻り値:char* 型	元の補填先を返す。
 */

char	*memset(ptr1, data, length)
     char	*ptr1;
     int	data;
     size_t	length;
{
  char	*ptr2;

  ptr2 = ptr1;
  while (length--) {
    *ptr1++ = data;
  }
  return(ptr2);
}
#endif	/* DONT_HAVE_MEMSET */

#ifdef	MSDOS
/*
 * バックアップファイルのパス名生成
 * (ファイル名に8文字制限があるマシン用)
 */

void  create_backup_DOS(bak, org) 
     char *bak;
     char *org;
{ 
  register int    i, j;

  sprintf(bak, "%s~", org); 
  j = strlen(org);
  if (j-- < 8) {
    return;
  }
  for (i = j; i >= 0 && org[i] != '/' && org[i] != '\\'; i--) {
  }
  if ((j - i) < 8) {
    return;
  }
  strcpy(&bak[i + 8], "~");
} 

/*
 * getpid() は getpid 関数のアドレス % 100 で誤魔化す
 */

int	getpid()
{
  return(((int)getpid) % 100);
}

/*
 * sleep() は単純ループで誤魔化す
 */

#ifndef	X68K
int	sleep(second)
     int	second;
{
  register int	i, j, n;

  n = 0;
  for (i = 0; i < second; i++) {
    for (j = 0; j < 0x7FFF; j++) {
      n += j;
    }
  }
  return(n);
}

#ifndef	__GO32__
#ifndef	DOS_PIPE
FILE	*popen()
{
  return((FILE*)NULL);
}

int	pclose()
{
  return(1);
}
#endif	/* DOS_PIPE */

void	*large_malloc(alloc_size)
  unsigned long	alloc_size;
{
  if (alloc_size >= 65536L) {
    return((void*)NULL);
  }
#ifdef	__TURBOC__
#ifdef	__WIN32__
  return(malloc((size_t)alloc_size));
#else	/* !__WIN32__ */
  return(farmalloc((unsigned long)alloc_size));
#endif	/* !__WIN32__ */
#else	/* !__TURBOC__ */
  return(malloc((size_t)alloc_size));
#endif	/* !__TURBOC__ */
}

void	*large_realloc(large_ptr, alloc_size)
  char		*large_ptr;
  unsigned long	alloc_size;
{
  if (alloc_size >= 65536L) {
    return((void*)NULL);
  }
#ifdef	__TURBOC__
#ifdef	__WIN32__
  return(realloc(large_ptr, (size_t)alloc_size));
#else	/* !__WIN32__ */
  return(farrealloc(large_ptr, (unsigned long)alloc_size));
#endif	/* !__WIN32__ */
#else	/* !__TURBOC__ */
  return(realloc(large_ptr, (size_t)alloc_size));
#endif	/* !__TURBOC__ */
}

void	large_free(large_ptr)
  char	*large_ptr;
{
#ifdef	__TURBOC__
#ifdef	__WIN32__
  free(large_ptr);
#else	/* !__WIN32__ */
  farfree(large_ptr);
#endif	/* !__WIN32__ */
#else	/* !__TURBOC__ */
  free(large_ptr);
#endif	/* !__TURBOC__ */
}
#endif	/* !__GO32__ */
#endif	/* !X68K */
#endif	/* MSDOS */

#ifdef	__GO32__
/*
 * DJGPPのgetch()はATでしか動かないのでDOSコールを使う
 */
#include	<dos.h>
#define	DOS_coninput	0x07		/* direct console input		*/
int	getch()
{
  union	REGS	regs;
  int		key;

  regs.h.ah = DOS_coninput;
  intdos(&regs, &regs);
  key = (int)regs.h.al;
  return(key);
}
#endif	/* __GO32__ */

/*
 * パス区切検索
 */

#ifdef	__STDC__
char	*separater(char *ptr1)
#else	/* !__STDC__ */
char	*separater(ptr1)
     char	*ptr1;
#endif	/* !__STDC__ */
{
#if	defined(MSDOS) || defined(OS2)
  char	*ptr2 = (char*)NULL;

  while (*ptr1) {
#ifdef	X68K
    if (ismbblead(*ptr1)) {			/* 漢字の第1バイト	*/
#else	/* !X68K */
    if (((*((unsigned char*)ptr1) >= 0x81) &&
	 (*((unsigned char*)ptr1) <= 0x9f)) ||
	((*((unsigned char*)ptr1) >= 0xe0) &&
	 (*((unsigned char*)ptr1) <= 0xfc))) {	/* 漢字の第1バイト	*/
#endif	/* !X68K */
      ptr1 += 2;
    } else if ((*ptr1 == '/') || (*ptr1 == '\\') || (*ptr1 == ':')) {
      ptr2 = ptr1++;
    } else {
      ptr1++;
    }
  }
  return(ptr2);
#else	/* !(MSDOS || OS2) */
  return((char*)strrchr(ptr1, (char)'/'));
#endif	/* !(MSDOS || OS2) */
}
