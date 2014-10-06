/*
 *
 *  漢字処理ライブラリ
 *
 *  Copyright 1990-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Kanji Convert Library
 *  Sub system  : library routine
 *  File        : kanjilib.c
 *  Version     : 2.46
 *  First Edit  : 1990-11/01
 *  Last  Edit  : 1997-11/24
 *  Author      : MSR24 宅間 顯
 *
 */

#include	<stdio.h>
#include	<ctype.h>
#ifdef	__STDC__
#include	<stdarg.h>
#else	/* !__STDC__ */
#include	<varargs.h>
#endif	/* !__STDC__ */
#include	"kanjilib.h"

#if	defined(KCNV) || defined(MNEWS)
#define	CHECK_CHAR
#endif	/* (KCNV || MNEWS) */

#ifdef	notdef
static int	bsprintf();		/* フォーマット出力	*/
#endif	/* notdef */
#ifdef	DISPLAY_CTRL
#ifdef	__STDC__
static void	str_to_str(char*, char*);
static void	str_to_ascii(char*, char*);
static void	str_to_jis(char*, char*);
static void	str_to_sjis(char*, char*);
static void	str_to_euc(char*, char*);
#else	/* !__STDC__ */
static void	str_to_str();
static void	str_to_ascii();
static void	str_to_jis();
static void	str_to_sjis();
static void	str_to_euc();
#endif	/* !__STDC__ */
#else	/* !DISPLAY_CTRL */
#define	str_to_str(s1, s2)	strcpy(s1, s2)
#define	str_to_ascii(s1, s2)	strcpy(s1, s2)
#define	str_to_jis(s1, s2)	strcpy(s1, s2)
#define	str_to_sjis(s1, s2)	strcpy(s1, s2)
#define	str_to_euc(s1, s2)	strcpy(s1, s2)
#endif	/* !DISPLAY_CTRL */

KANJICODE	print_code;			/* 漢字コード		*/

char		jis_in[8]   = JIS_IN;		/* 漢字指示コード	*/
char		jis_out[8]  = JIS_OUT;		/* ローマ字指示コード	*/
int		jis_in_len  = sizeof(JIS_IN) - 1;
						/* 漢字指示コード長	*/
int		jis_out_len = sizeof(JIS_OUT) - 1;
						/* ローマ字指示コード長	*/
#ifdef	CHECK_CHAR
unsigned int	char_mask = 0;			/* 文字種別		*/
#endif	/* CHECK_CHAR */

char		*kanji_code_string[] = {	/* 漢字コード名		*/
  "ASCII",
  "JIS",
  "SJIS",
  "EUC",
  "UNKNOWN",
  "ERROR",
};

#ifdef	notdef
static char	small_hex_char[] = "0123456789abcdef";
#endif	/* notdef */
#ifdef	DISPLAY_CTRL
static char	large_hex_char[] = "0123456789ABCDEF";
#endif	/* DISPLAY_CTRL */
#ifdef	CHECK_CHAR
static unsigned int	jis_char_mask = 0;
static unsigned int	sjis_char_mask = 0;
static unsigned int	euc_char_mask = 0;
#endif	/* CHECK_CHAR */

/*
 * 漢字ライブラリ初期化
 */

#ifdef	__STDC__
void KANJILIBFUNC jis_mode(int mode)	/* 0:旧JIS, 0以外:新JIS */
#else	/* !__STDC__ */
void	jis_mode(mode)
     int	mode;	/* 0:旧JIS, 0以外:新JIS */
#endif	/* !__STDC__ */
{
  if (mode) {
    strcpy(jis_in, JIS_IN2);
    strcpy(jis_out, JIS_OUT2);
  } else {
    strcpy(jis_in, JIS_IN1);
    strcpy(jis_out, JIS_OUT1);
  }
}

/*
 * JIS→SJIS 漢字コード変換
 */

#ifdef	__STDC__
void KANJILIBFUNC jis_to_sjis(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
void	jis_to_sjis(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register CHARSET	charset = ROMAN_CHAR;
  
  while (*ptr2) {
    if (*ptr2 == ESC_CODE) {
      if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	
	/*	漢字指示	*/
	
	charset = KANJI_CHAR;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_IN3, 4)) {
	
	/*	漢字指示	*/
	
	charset = KANJI_CHAR;
	ptr2 += 4;
      } else if ((!strncmp(ptr2, JIS_OUT1, 3))
		 || (!strncmp(ptr2, JIS_OUT2, 3))) {
	
	/*	ローマ字指示	*/
	
	charset = ROMAN_CHAR;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_X0201, 3)) {

	/*	片仮名指示	*/

#ifdef	SUPPORT_X0201
	charset = KANA_CHAR;
	ptr2 += 3;
#else	/* !SUPPORT_X0201 */
#ifdef	DISPLAY_CTRL
	*ptr1++ = '^';
	*ptr1++ = '[';
	ptr2++;
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
#endif	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	charset = ROMAN_CHAR;
#endif	/* !SUPPORT_X0201 */
      } else if (!strncmp(ptr2, JIS_X0212, 4)) {
	
	/*	補助漢字指示	*/
	
#ifdef	DELETE_X0212
	charset = ROMAN_CHAR;
#else	/* !DELETE_X0212 */
	charset = HOJO_CHAR;
#endif	/* !DELETE_X0212 */
	ptr2 += 4;
      } else {
#ifdef	DISPLAY_CTRL
	*ptr1++ = '^';
	*ptr1++ = '[';
	ptr2++;
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
#endif	/* !DISPLAY_CTRL */
	while (*ptr2) {
	  if ((*((unsigned char*)ptr2) < 0x30) ||
	      (*((unsigned char*)ptr2) > 0x7e)) {
	    if (*((unsigned char*)ptr2) < ' ') {
	      *ptr1++ = '^';
	      *ptr1++ = '@' + *ptr2++;
	    } else {
	      *ptr1++ = *ptr2++;
	    }
	  } else {
	    *ptr1++ = *ptr2++;
	    break;
	  }
	}
      }
#ifdef	SUPPORT_X0201
    } else if (*ptr2 == X0201_IN_CODE) {
      charset = KANA_CHAR;
      ptr2++;
    } else if (*ptr2 == X0201_OUT_CODE) {
      charset = ROMAN_CHAR;
      ptr2++;
#endif	/* !SUPPORT_X0201 */
    } else {
      switch (charset) {
      case KANA_CHAR:
	*ptr1++ = (*ptr2++) | 0x80;
	break;
      case KANJI_CHAR:
	*ptr1++ = *ptr2++;
	*ptr1   = *ptr2++;
	if (!(*((unsigned char*)ptr1 - 1) % 2)) {
	  (*ptr1) += 0x7d;
	} else {
	  (*ptr1) += 0x1f;
	}
	if (*((unsigned char*)ptr1) >= 0x7f) {
	  (*ptr1) += 1;
	}
	*(ptr1 - 1) =
	  ((unsigned int)(*((unsigned char*)ptr1 - 1) - 0x21)) / 2 + 0x81;
	if (*((unsigned char*)ptr1 - 1) > 0x9f) {
	  (*(ptr1 - 1)) += 0x40;
	}
	ptr1++;
	break;
      case HOJO_CHAR:		/* サポートできていません。	*/
	*ptr1++ = *ptr2++;
	break;
      case ROMAN_CHAR:
      default:
	*ptr1++ = *ptr2++;
	break;
      }
    }
  }
  *ptr1 = '\0';
}

/*
 * JIS→EUC 漢字コード変換
 */

#ifdef	__STDC__
void KANJILIBFUNC jis_to_euc(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
void	jis_to_euc(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register CHARSET	charset = ROMAN_CHAR;
  
  while (*ptr2) {
    if (*ptr2 == ESC_CODE) {
      if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	
	/*	漢字指示	*/
	
	charset = KANJI_CHAR;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_IN3, 4)) {
	
	/*	漢字指示	*/
	
	charset = KANJI_CHAR;
	ptr2 += 4;
      } else if ((!strncmp(ptr2, JIS_OUT1, 3))
		 || (!strncmp(ptr2, JIS_OUT2, 3))) {
	
	/*	ローマ字指示	*/
	
	charset = ROMAN_CHAR;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_X0201, 3)) {

	/*	片仮名指示	*/

#ifdef	SUPPORT_X0201
	charset = KANA_CHAR;
	ptr2 += 3;
#else	/* !SUPPORT_X0201 */
#ifdef	DISPLAY_CTRL
	*ptr1++ = '^';
	*ptr1++ = '[';
	ptr2++;
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
#endif	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	charset = ROMAN_CHAR;
#endif	/* !SUPPORT_X0201 */
      } else if (!strncmp(ptr2, JIS_X0212, 4)) {
	
	/*	補助漢字指示	*/
	
#ifdef	DELETE_X0212
	charset = ROMAN_CHAR;
#else	/* !DELETE_X0212 */
	charset = HOJO_CHAR;
#endif	/* !DELETE_X0212 */
	ptr2 += 4;
      } else {
#ifdef	DISPLAY_CTRL
	*ptr1++ = '^';
	*ptr1++ = '[';
	ptr2++;
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
#endif	/* !DISPLAY_CTRL */
	while (*ptr2) {
	  if ((*((unsigned char*)ptr2) < 0x30) ||
	      (*((unsigned char*)ptr2) > 0x7e)) {
	    if (*((unsigned char*)ptr2) < ' ') {
	      *ptr1++ = '^';
	      *ptr1++ = '@' + *ptr2++;
	    } else {
	      *ptr1++ = *ptr2++;
	    }
	  } else {
	    *ptr1++ = *ptr2++;
	    break;
	  }
	}
      }
#ifdef	SUPPORT_X0201
    } else if (*ptr2 == X0201_IN_CODE) {
      charset = KANA_CHAR;
      ptr2++;
    } else if (*ptr2 == X0201_OUT_CODE) {
      charset = ROMAN_CHAR;
      ptr2++;
#endif	/* !SUPPORT_X0201 */
#ifdef	DISPLAY_CTRL
    } else if (*((unsigned char*)ptr2) < ' ') {
      if ((*ptr2 == '\t') || (*ptr2 == '\n') || (*ptr2 == '\r') ||
	  (*ptr2 == 0x0c)) {
	*ptr1++ = *ptr2++;
      } else {
	*ptr1++ = '^';
	*ptr1++ = '@' + *ptr2++;
      }
#endif	/* DISPLAY_CTRL */
    } else {
      switch (charset) {
      case KANA_CHAR:
	*((unsigned char*)ptr1++) = EUC_SS2;
	*ptr1++ = (*ptr2++) | 0x80;
	break;
      case HOJO_CHAR:
	*((unsigned char*)ptr1++) = EUC_SS3;
      case KANJI_CHAR:
	*ptr1++ = (*ptr2++ | 0x80);
	*ptr1++ = (*ptr2++ | 0x80);
	break;
      case ROMAN_CHAR:
      default:
	*ptr1++ = *ptr2++;
	break;
      }
    }
  }
  *ptr1 = '\0';
}

/*
 * EUC→JIS 漢字コード変換
 */

#ifdef	__STDC__
void KANJILIBFUNC euc_to_jis(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
void	euc_to_jis(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register CHARSET	charset = ROMAN_CHAR;

  while (*ptr2) {
    if (*ptr2 & 0x80) {
#ifdef	SUPPORT_X0201
      if (*((unsigned char*)ptr2) == EUC_SS2) {
	if (charset != KANA_CHAR) {

	  /*	片仮名指示	*/

	  charset = KANA_CHAR;
	  strcpy(ptr1, JIS_X0201);
	  ptr1 += 3;
	}
	ptr2++;
	*ptr1++ = *ptr2++ & 0x7f;
	continue;
      }
#endif	/* SUPPORT_X0201 */
      if (*((unsigned char*)ptr2) == EUC_SS3) {
#ifdef	DELETE_X0212
	ptr2 += 4;
#else	/* !DELETE_X0212 */
	if (charset != HOJO_CHAR) {

	  /*	補助漢字指示	*/

	  charset = HOJO_CHAR;
	  strcpy(ptr1, JIS_X0212);
	  ptr1 += 4;
	}
	ptr2++;
	*ptr1++ = *ptr2++ & 0x7f;
	*ptr1++ = *ptr2++ & 0x7f;
#endif	/* !DELETE_X0212 */
	continue;
      }
      if (charset != KANJI_CHAR) {
	
	/*	漢字指示	*/

	charset = KANJI_CHAR;
	strcpy(ptr1, jis_in);
	ptr1 += jis_in_len;
      }
      *ptr1++ = *ptr2++ & 0x7f;
      *ptr1++ = *ptr2++ & 0x7f;
#ifdef	SUPPORT_X0201
    } else if (*ptr2 == X0201_IN_CODE) {
      charset = KANA_CHAR;
      ptr2++;
      strcpy(ptr1, JIS_X0201);
      ptr1 += 3;
      while (*ptr2) {
	if (*ptr2 == X0201_OUT_CODE) {
	  ptr2++;
	  break;
	}
	*ptr1++ = *ptr2++;
      }
    } else if (*ptr2 == X0201_OUT_CODE) {
      ptr2++;
#endif	/* !SUPPORT_X0201 */
    } else {
      if (charset) {

	/*	ローマ字指示	*/

	charset = ROMAN_CHAR;
	strcpy(ptr1, jis_out);
	ptr1 += jis_out_len;
      }
      *ptr1++ = *ptr2++;
    }
  }
  if (charset) {

    /*	ローマ字指示	*/

    strcpy(ptr1, jis_out);
  } else {
    *ptr1 = '\0';
  }
}

/*
 * EUC→SJIS 漢字コード変換
 */

#ifdef	__STDC__
void KANJILIBFUNC euc_to_sjis(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
void	euc_to_sjis(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  while (*ptr2) {
    if (*ptr2 & 0x80) {
#ifdef	SUPPORT_X0201
      if (*((unsigned char*)ptr2) == EUC_SS2) {

	/*	片仮名	*/

	ptr2++;
	*ptr1++ = *ptr2++;
	continue;
      }
#endif	/* SUPPORT_X0201 */

      /*	漢字	*/

      *ptr1++ = *ptr2++ & 0x7f;
      *ptr1   = *ptr2++ & 0x7f;
      if (*((unsigned char*)ptr1 - 1) & 1) {
	*ptr1 += 0x1f;
      } else {
	*ptr1 += 0x7d;
      }
      if (*((unsigned char*)ptr1) >= 0x7f) {
	*ptr1 += 1;
      }
      ptr1--;
      *ptr1 = ((unsigned int)(*ptr1 - 0x21)) / 2 + 0x81;
      if (*((unsigned char*)ptr1) > 0x9f) {
	*ptr1 += 0x40;
      }
      ptr1 += 2;
#ifdef	SUPPORT_X0201
    } else if (*ptr2 == X0201_IN_CODE) {
      ptr2++;
      while (*ptr2) {
	if (*ptr2 == X0201_OUT_CODE) {
	  ptr2++;
	  break;
	}
	*ptr1++ = *ptr2++ | 0x80;
      }
    } else if (*ptr2 == X0201_OUT_CODE) {
      ptr2++;
#endif	/* !SUPPORT_X0201 */
    } else {

      /*	非漢字	*/

      *ptr1++ = *ptr2++;
    }
  }
  *ptr1 = '\0';
}

/*
 * SJIS→JIS 漢字コード変換
 */

#ifdef	__STDC__
void KANJILIBFUNC sjis_to_jis(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
void	sjis_to_jis(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register CHARSET	charset = ROMAN_CHAR;
  
  while (*ptr2) {
    if (*ptr2 & 0x80) {
#ifdef	SUPPORT_X0201
      if ((*((unsigned char*)ptr2) >= 0xa0) &&
	  (*((unsigned char*)ptr2) < 0xe0)) {
	if (charset != KANA_CHAR) {

	  /*	片仮名指示	*/

	  charset = KANA_CHAR;
	  strcpy(ptr1, JIS_X0201);
	  ptr1 += 3;
	}
	*ptr1++ = *ptr2++ & 0x7f;
	continue;
      }
#endif	/* SUPPORT_X0201 */
      if (charset != KANJI_CHAR) {

	/*	漢字指示	*/

	charset = KANJI_CHAR;
	strcpy(ptr1, jis_in);
	ptr1 += jis_in_len;
      }
      if (*((unsigned char*)ptr2) <= 0x9f) {
	*ptr1++ = ((*ptr2++) - 0x71) * 2 + 1;
      } else {
	*ptr1++ = ((*ptr2++) - 0xb1) * 2 + 1;
      }
      if (*((unsigned char*)ptr2) > 0x7f) {
	*ptr1 = (*ptr2++) - 1;
      } else {
	*ptr1 = *ptr2++;
      }
      if (*((unsigned char*)ptr1) >= 0x9e) {
	*(ptr1) -= 0x7d;
	*(ptr1 - 1) += 1;
      } else {
	*(ptr1) -= 0x1f;
      }
      ptr1++;
#ifdef	SUPPORT_X0201
    } else if (*ptr2 == X0201_IN_CODE) {
      charset = KANA_CHAR;
      ptr2++;
      strcpy(ptr1, JIS_X0201);
      ptr1 += 3;
      while (*ptr2) {
	if (*ptr2 == X0201_OUT_CODE) {
	  ptr2++;
	  break;
	}
	*ptr1++ = *ptr2++;
      }
    } else if (*ptr2 == X0201_OUT_CODE) {
      ptr2++;
#endif	/* !SUPPORT_X0201 */
    } else {
      if (charset) {

	/*	ローマ字指示	*/

	charset = ROMAN_CHAR;
	strcpy(ptr1, jis_out);
	ptr1 += jis_out_len;
      }
      *ptr1++ = *ptr2++;
    }
  }
  if (charset) {

    /*	ローマ字指示	*/

    strcpy(ptr1, jis_out);
    ptr1 += jis_out_len;
  }
  *ptr1 = '\0';
}

/*
 * SJIS→EUC 漢字コード変換
 */

#ifdef	__STDC__
void KANJILIBFUNC sjis_to_euc(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
void	sjis_to_euc(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  while (*ptr2) {
    if (*ptr2 & 0x80) {
#ifdef	SUPPORT_X0201
      if ((*((unsigned char*)ptr2) >= 0xa0) &&
	  (*((unsigned char*)ptr2) < 0xe0)) {
	*((unsigned char*)ptr1++) = EUC_SS2;
	*ptr1++ = *ptr2++;
	continue;
      }
#endif	/* SUPPORT_X0201 */
      if (*((unsigned char*)ptr2) <= 0x9f) {
	*ptr1++ = ((*ptr2++) - 0x71) * 2 + 1;
      } else {
	*ptr1++ = ((*ptr2++) - 0xb1) * 2 + 1;
      }
      if (*((unsigned char*)ptr2) > 0x7f) {
	*ptr1 = (*ptr2++) - 1;
      } else {
	*ptr1 = *ptr2++;
      }
      if (*((unsigned char*)ptr1) >= 0x9e) {
	*(ptr1) -= 0x7d;
	*(ptr1 - 1) += 1;
      } else {
	*(ptr1) -= 0x1f;
      }
      *(ptr1 - 1) |= 0x80;
      *(ptr1) |= 0x80;
      ptr1++;
#ifdef	SUPPORT_X0201
    } else if (*ptr2 == X0201_IN_CODE) {
      ptr2++;
      while (*ptr2) {
	if (*ptr2 == X0201_OUT_CODE) {
	  ptr2++;
	  break;
	}
	*((unsigned char*)ptr1++) = EUC_SS2;
	*ptr1++ = *ptr2++ | 0x80;
      }
    } else if (*ptr2 == X0201_OUT_CODE) {
      ptr2++;
#endif	/* !SUPPORT_X0201 */
    } else {
      *ptr1++ = *ptr2++;
    }
  }
  *ptr1 = '\0';
}

/*
 * JIS→JIS 漢字コード変換(エスケープシーケンス統一)
 */

#ifdef	__STDC__
void KANJILIBFUNC jis_to_jis(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
void	jis_to_jis(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register CHARSET	charset1 = ROMAN_CHAR;
  register CHARSET	charset2 = ROMAN_CHAR;
  
  while (*ptr2) {
    if (*ptr2 == ESC_CODE) {
      if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	
	/*	漢字指示	*/
	
	charset1 = KANJI_CHAR;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_IN3, 4)) {
	
	/*	漢字指示	*/
	
	charset1 = KANJI_CHAR;
	ptr2 += 4;
      } else if ((!strncmp(ptr2, JIS_OUT1, 3))
		 || (!strncmp(ptr2, JIS_OUT2, 3))) {
	
	/*	ローマ字指示	*/
	
	charset1 = ROMAN_CHAR;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_X0201, 3)) {

	/*	片仮名指示	*/

#ifdef	SUPPORT_X0201
	charset1 = KANA_CHAR;
	ptr2 += 3;
#else	/* !SUPPORT_X0201 */
#ifdef	DISPLAY_CTRL
	*ptr1++ = '^';
	*ptr1++ = '[';
	ptr2++;
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
#endif	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	charset1 = ROMAN_CHAR;
#endif	/* !SUPPORT_X0201 */
      } else if (!strncmp(ptr2, JIS_X0212, 4)) {
	
	/*	補助漢字指示	*/
	
#ifdef	DELETE_X0212
	charset1 = ROMAN_CHAR;
#else	/* !DELETE_X0212 */
	charset1 = HOJO_CHAR;
#endif	/* !DELETE_X0212 */
	ptr2 += 4;
      } else {
#ifdef	DISPLAY_CTRL
	*ptr1++ = '^';
	*ptr1++ = '[';
	ptr2++;
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
#endif	/* !DISPLAY_CTRL */
	while (*ptr2) {
	  if ((*((unsigned char*)ptr2) < 0x30) ||
	      (*((unsigned char*)ptr2) > 0x7e)) {
	    if (*((unsigned char*)ptr2) < ' ') {
	      *ptr1++ = '^';
	      *ptr1++ = '@' + *ptr2++;
	    } else {
	      *ptr1++ = *ptr2++;
	    }
	  } else {
	    *ptr1++ = *ptr2++;
	    break;
	  }
	}
      }
#ifdef	SUPPORT_X0201
    } else if (*ptr2 == X0201_IN_CODE) {
      charset1 = KANA_CHAR;
      ptr2++;
    } else if (*ptr2 == X0201_OUT_CODE) {
      charset1 = ROMAN_CHAR;
      ptr2++;
#endif	/* !SUPPORT_X0201 */
    } else {
      if (charset1) {
	if (charset1 != charset2) {
	  switch (charset1) {
	  case KANA_CHAR:
	    strcpy(ptr1, JIS_X0201);
	    ptr1 += 3;
	    break;
	  case KANJI_CHAR:
	    strcpy(ptr1, jis_in);
	    ptr1 += jis_in_len;
	    break;
	  case HOJO_CHAR:
	    strcpy(ptr1, JIS_X0212);
	    ptr1 += 4;
	    break;
	  default:
	    break;
	  }
	  charset2 = charset1;
	}
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
      } else {
	if (charset2) {
	  strcpy(ptr1, jis_out);
	  ptr1 += jis_out_len;
	  charset2 = ROMAN_CHAR;
	}
	*ptr1++ = *ptr2++;
      }
    }
  }
  if (charset2) {
    strcpy(ptr1, jis_out);
  } else {
    *ptr1 = '\0';
  }
}

/*
 * JIS 漢字文字列コピー(指定幅)
 */

#ifdef	__STDC__
int KANJILIBFUNC jis_strncpy(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	jis_strncpy(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  return(kanji_strncpy(ptr1, ptr2, count));
}

/*
 * EUC 漢字文字列コピー(指定幅)
 */

#ifdef	__STDC__
int KANJILIBFUNC euc_strncpy(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	euc_strncpy(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  char	*ptr3;
  
  ptr3 = ptr2;
  while (count > 0) {
    if (!*ptr2) {
      break;
    }
    if (*ptr2 & 0x80) {
      if (*((unsigned char*)ptr2) == EUC_SS3) {
#ifdef	DELETE_X0212
	ptr2++;
	if (*ptr2) {
	  ptr2++;
	  if (*ptr2) {
	    ptr2++;
	  }
	}
#else	/* !DELETE_X0212 */
	if (count < 3) {
	  count = 0;
	} else {
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count -= 3;
	}
#endif	/* !DELETE_X0212 */
      } else {
	if (count < 2) {
	  count = 0;
	} else {
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count -= 2;
	}
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
    }
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * SJIS 漢字文字列コピー(指定幅)
 */

#ifdef	__STDC__
int KANJILIBFUNC sjis_strncpy(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	sjis_strncpy(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  char	*ptr3;
  
  ptr3 = ptr2;
  while (count > 0) {
    if (!*ptr2) {
      break;
    }
    if (*ptr2 & 0x80) {
      if (count < 2) {
	count = 0;
      } else {
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	count -= 2;
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
    }
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * JIS 漢字文字列コピー(指定幅パディング有)
 */

#ifdef	__STDC__
int KANJILIBFUNC jis_strncpy2(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	jis_strncpy2(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  return(kanji_strncpy2(ptr1, ptr2, count));
}

/*
 * EUC 漢字文字列コピー(指定パディング有)
 */

#ifdef	__STDC__
int KANJILIBFUNC euc_strncpy2(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	euc_strncpy2(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  char	*ptr3;
  
  ptr3 = ptr2;
  while (count > 0) {
    if (!*ptr2) {
      break;
    } else if (*ptr2 & 0x80) {
      if (*((unsigned char*)ptr2) == EUC_SS3) {
#ifdef	DELETE_X0212
	ptr2++;
	if (*ptr2) {
	  ptr2++;
	  if (*ptr2) {
	    ptr2++;
	  }
	}
#else	/* !DELETE_X0212 */
	if (count < 3) {
	  break;
	} else {
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count -= 3;
	}
#endif	/* !DELETE_X0212 */
      } else {
	if (count < 2) {
	  break;
	} else {
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count -= 2;
	}
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
    }
  }
  while (count-- > 0) {
    *ptr1++ = ' ';
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * SJIS 漢字文字列コピー(指定パディング有)
 */

#ifdef	__STDC__
int KANJILIBFUNC sjis_strncpy2(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	sjis_strncpy2(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  char	*ptr3;
  
  ptr3 = ptr2;
  while (count > 0) {
    if (!*ptr2) {
      break;
    }
    if (*ptr2 & 0x80) {
      if (count < 2) {
	break;
      } else {
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	count -= 2;
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
    }
  }
  while (count-- > 0) {
    *ptr1++ = ' ';
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * JIS 漢字文字列コピー(指定幅,TAB 展開付)
 */

#ifdef	__STDC__
int KANJILIBFUNC jis_tab_strncpy(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	jis_tab_strncpy(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  return(kanji_tab_strncpy(ptr1, ptr2, count));
}

/*
 * EUC 文字列コピー(指定幅,TAB 展開付)
 */

#ifdef	__STDC__
int KANJILIBFUNC euc_tab_strncpy(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	euc_tab_strncpy(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  char		*ptr3;
  register int	index;
  
  ptr3 = ptr2;
  index = 0;
  while (count > 0) {
    if (!*ptr2) {
      break;
    }
    if (*ptr2 & 0x80) {
      if (*((unsigned char*)ptr2) == EUC_SS3) {
#ifdef	DELETE_X0212
	ptr2++;
	if (*ptr2) {
	  ptr2++;
	  if (*ptr2) {
	    ptr2++;
	  }
	}
#else	/* !DELETE_X0212 */
	if (count < 3) {
	  count = 0;
	} else {
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count -= 3;
	}
#endif	/* !DELETE_X0212 */
      } else {
	if (count < 2) {
	  count = 0;
	} else {
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count -= 2;
	  index += 2;
	}
      }
    } else if (*((unsigned char*)ptr2) < ' ') {
      if (*ptr2 == '\t') {
	do {
	  *ptr1++ = ' ';
	  count--;
	} while ((++index % 8) && (count > 0));
	ptr2++;
      } else {
#ifdef	DISPLAY_CTRL
	if (count < 2) {
	  count = 0;
	} else {
	  *ptr1++ = '^';
	  *ptr1++ = '@' + *ptr2++;
	  count -= 2;
	  index += 2;
	}
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
	count--;
	index++;
#endif	/* !DISPLAY_CTRL */
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
      index++;
    }
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * SJIS 漢字文字列コピー(指定幅,TAB 展開付)
 */

#ifdef	__STDC__
int KANJILIBFUNC sjis_tab_strncpy(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	sjis_tab_strncpy(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  char		*ptr3;
  register int	index;
  
  ptr3 = ptr2;
  index = 0;
  while (count > 0) {
    if (!*ptr2) {
      break;
    }
    if (*ptr2 & 0x80) {
      if (count < 2) {
	count = 0;
      } else {
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	count -= 2;
	index += 2;
      }
    } else if (*((unsigned char*)ptr2) < ' ') {
      if (*ptr2 == '\t') {
	do {
	  *ptr1++ = ' ';
	  count--;
	} while ((++index % 8) && (count > 0));
	ptr2++;
      } else {
#ifdef	DISPLAY_CTRL
	if (count < 2) {
	  count = 0;
	} else {
	  *ptr1++ = '^';
	  *ptr1++ = '@' + *ptr2++;
	  count -= 2;
	  index += 2;
	}
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
	count--;
	index++;
#endif	/* !DISPLAY_CTRL */
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
      index++;
    }
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * JIS 漢字文字列表示
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	jis_printf(char *fmt, int arg1, int arg2, int arg3, int arg4,
		   int arg5, int arg6)
#else	/* !__STDC__ */
void	jis_printf(fmt, arg1, arg2, arg3, arg4, arg5, arg6)
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6;
#endif	/* !__STDC__ */
{
  jis_fprintf(stdout, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	jis_printf(char *fmt, ...)
#else	/* __STDC__ */
void	jis_printf(fmt, va_alist)
     char	*fmt;
#endif	/* __STDC__ */
{
  va_list	ap;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
#ifdef	notdef
  bsprintf(buff1, fmt, ap);
#else	/* !notdef */
  vsprintf(buff1, fmt, ap);
#endif	/* !notdef */
  switch (print_code) {
  case JIS_CODE:
    jis_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    jis_to_sjis(buff2, buff1);
    break;
  case EUC_CODE:
    jis_to_euc(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  printf("%s", buff2);
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * EUC 漢字文字列表示
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	euc_printf(char *fmt, int arg1, int arg2, int arg3, int arg4,
		   int arg5, int arg6)
#else	/* !__STDC__ */
void	euc_printf(fmt, arg1, arg2, arg3, arg4, arg5, arg6)
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6;
#endif	/* !__STDC__ */
{
  euc_fprintf(stdout, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	euc_printf(char *fmt, ...)
#else	/* !__STDC__ */
void	euc_printf(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
#ifdef	notdef
  bsprintf(buff1, fmt, ap);
#else	/* !notdef */
  vsprintf(buff1, fmt, ap);
#endif	/* !notdef */
  switch (print_code) {
  case JIS_CODE:
    euc_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    euc_to_sjis(buff2, buff1);
    break;
  case EUC_CODE:
    str_to_str(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  printf("%s", buff2);
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * SJIS 漢字文字列表示
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	sjis_printf(char *fmt, int arg1, int arg2, int arg3, int arg4,
		    int arg5, int arg6)
#else	/* !__STDC__ */
void	sjis_printf(fmt, arg1, arg2, arg3, arg4, arg5, arg6)
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6;
#endif	/* !__STDC__ */
{
  euc_fprintf(stdout, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	sjis_printf(char *fmt, ...)
#else	/* !__STDC__ */
void	sjis_printf(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
#ifdef	notdef
  bsprintf(buff1, fmt, ap);
#else	/* !notdef */
  vsprintf(buff1, fmt, ap);
#endif	/* !notdef */
  switch (print_code) {
  case JIS_CODE:
    sjis_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    str_to_str(buff2, buff1);
    break;
  case EUC_CODE:
    sjis_to_euc(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  printf("%s", buff2);
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * JIS 漢字文字列出力
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	jis_fprintf(FILE *fp, char *fmt, int arg1, int arg2, int arg3,
		    int arg4, int arg5, int arg6)
#else	/* !__STDC__ */
void	jis_fprintf(fp, fmt, arg1, arg2, arg3, arg4, arg5, arg6)
     FILE	*fp;
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  
  sprintf(buff1, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
  switch (print_code) {
  case JIS_CODE:
    jis_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    jis_to_sjis(buff2, buff1);
    break;
  case EUC_CODE:
    jis_to_euc(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
}
#else	/* DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	jis_fprintf(FILE *fp, char *fmt, ...)
#else	/* !__STDC__ */
void	jis_fprintf(fp, fmt, va_alist)
     FILE	*fp;
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  
#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
#ifdef	notdef
  bsprintf(buff1, fmt, ap);
#else	/* !notdef */
  vsprintf(buff1, fmt, ap);
#endif	/* !notdef */
  switch (print_code) {
  case JIS_CODE:
    jis_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    jis_to_sjis(buff2, buff1);
    break;
  case EUC_CODE:
    jis_to_euc(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
  va_end(ap);
}
#endif	/* DONT_HAVE_DOPRNT */

/*
 * EUC 漢字文字列出力
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	euc_fprintf(FILE *fp, char *fmt, int arg1, int arg2, int arg3,
		    int arg4, int arg5, int arg6)
#else	/* !__STDC__ */
void	euc_fprintf(fp, fmt, arg1, arg2, arg3, arg4, arg5, arg6)
     FILE	*fp;
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  
  sprintf(buff1, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
  switch (print_code) {
  case JIS_CODE:
    euc_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    euc_to_sjis(buff2, buff1);
    break;
  case EUC_CODE:
    str_to_str(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
}
#else	/* DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	euc_fprintf(FILE *fp, char *fmt, ...)
#else	/* !__STDC__ */
void	euc_fprintf(fp, fmt, va_alist)
     FILE	*fp;
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  
#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
#ifdef	notdef
  bsprintf(buff1, fmt, ap);
#else	/* !notdef */
  vsprintf(buff1, fmt, ap);
#endif	/* !notdef */
  switch (print_code) {
  case JIS_CODE:
    euc_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    euc_to_sjis(buff2, buff1);
    break;
  case EUC_CODE:
    str_to_str(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
  va_end(ap);
}
#endif	/* DONT_HAVE_DOPRNT */

/*
 * SJIS 漢字文字列出力
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	sjis_fprintf(FILE *fp, char *fmt, int arg1, int arg2, int arg3,
		     int arg4, int arg5, int arg6)
#else	/* !__STDC__ */
void	sjis_fprintf(fp, fmt, arg1, arg2, arg3, arg4, arg5, arg6)
     FILE	*fp;
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  
  sprintf(buff1, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
  switch (print_code) {
  case JIS_CODE:
    sjis_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    str_to_str(buff2, buff1);
    break;
  case EUC_CODE:
    sjis_to_euc(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
}
#else	/* DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	sjis_fprintf(FILE *fp, char *fmt, ...)
#else	/* !__STDC__ */
void	sjis_fprintf(fp, fmt, va_alist)
     FILE	*fp;
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  
#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
#ifdef	notdef
  bsprintf(buff1, fmt, ap);
#else	/* !notdef */
  vsprintf(buff1, fmt, ap);
#endif	/* !notdef */
  switch (print_code) {
  case JIS_CODE:
    sjis_to_jis(buff2, buff1);
    break;
  case SJIS_CODE:
    str_to_str(buff2, buff1);
    break;
  case EUC_CODE:
    sjis_to_euc(buff2, buff1);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
  va_end(ap);
}
#endif	/* DONT_HAVE_DOPRNT */

#ifdef	notdef
/*
 * フォーマット出力
 */

#ifdef	__STDC__
static int	bsprintf(char *ptr, char *fmt, int *ap)
#else	/* !__STDC__ */
static int	bsprintf(ptr, fmt, ap)
     char	*ptr;
     char	*fmt;
     int	*ap;
#endif	/* !__STDC__ */
{
  static char	buff[16];
  int		length;
  unsigned int	number;
  char		*top;
  char		*str;
  char		pad_char;

  top = ptr;
  while (*fmt) {
    switch (*fmt) {
    case '%':
      pad_char = ' ';
      fmt++;
      length = 0;
      if (*fmt == '0') {
	fmt++;
	pad_char = '0';
      }
      while (isdigit(*fmt)) {
	length = length * 10 + (*fmt++) - '0';
      }
      if (length > 10) {
	length = 10;
      }
      switch (*fmt) {
      case 0:
	fmt--;
	break;
      case 'c':		/* 文字		*/
      case 'C':
	*ptr++ = (char)(*ap++);
	break;
      case 'd':
      case 'i':
      case 'D':
      case 'I':		/* 10進符合つき	*/
      case 'u':
      case 'U':		/* 10進符合なし	*/
	str = buff;
	number = (unsigned int)*ap++;
	if ((*fmt != 'U') && (*fmt != 'u')) {
	  if ((int)number < 0) {
	    *ptr++ = '-';
	    number = -((int)number);
	    length--;
	  }
	}
	do {
	  *str++ = (number % 10) + '0';
	  number /= 10;
	  length--;
	} while (number);
	while (length-- > 0) {
	  *str++ = pad_char;
	}
	do {
	  *ptr++ = *--str;
	} while (str != buff);
	break;
      case 'x':
      case 'X':		/* 16進		*/
	str = buff;
	number = (unsigned int)*ap++;
	if (*fmt == 'X') {
	  do {
	    *str++ = large_hex_char[number % 0x10];
	    number >>= 4;
	    length--;
	  } while (number);
	} else {
	  do {
	    *str++ = small_hex_char[number % 0x10];
	    number >>= 4;
	    length--;
	  } while (number);
	}
	while (length-- > 0) {
	  *str++ = pad_char;
	}
	do {
	  *ptr++ = *--str;
	} while (str != buff);
	break;
      case 's':		/* 文字列	*/
      case 'S':
	str = (char*)*ap++;
	if (length) {
	  length -= strlen(str);
	  while (length-- > 0) {
	    *ptr++ = pad_char;
	  }
	}
	while (*str) {
	  *ptr++ = *str++;
	}
	break;
      default:
	*ptr++ = *fmt;
	break;
      }
      fmt++;
      break;
    case '\\':
      switch (*(++fmt)) {
      case 'n':
	*ptr++ = '\n';
	break;
      case 'r':
	*ptr++ = '\r';
	break;
      case 't':
	*ptr++ = '\t';
	break;
      default:
	*ptr++ = *fmt;
	break;
      }
      fmt++;
      break;
    default:
      *ptr++ = *fmt++;
      break;
    }
  }
  *ptr = '\0';
  return((int)(ptr - top));
}
#endif	/* notdef */

#ifdef	DONT_HAVE_VSPRINTF
#ifdef	__STDC__
int	vsprintf(char *str, char *fmt, va_list ap)
#else	/* !__STDC__ */
int	vsprintf(str, fmt, ap)
     char	*str;
     char	*fmt;
     va_list	ap;
#endif	/* !__STDC__ */
{
  FILE	ff;
  int	length;
  
  ff._ptr = str;
  ff._flag = _IOSTRG | _IOWRT;
  ff._cnt = 32767;
  length = _doprnt(fmt, ap, &ff);
  *ff._ptr = 0;
  return(length);
}
#endif	/* DONT_HAVE_VSPRINTF */

#ifdef	notdef
/*
 * 漢字コード判別
 * (JIS-X0201 を考慮しない)
 */

#ifdef	__STDC__
KANJICODE KANJILIBFUNC discriminate_code(char *ptr)
#else	/* !__STDC__ */
KANJICODE	discriminate_code(ptr)
     char	*ptr;
#endif	/* !__STDC__ */
{
  register int	status = 0;

  while (*ptr) {
    if (*ptr == ESC_CODE) {
      if ((!strncmp(ptr, JIS_IN1, 3)) || (!strncmp(ptr, JIS_IN2, 3)) ||
	  (!strncmp(ptr, JIS_IN3, 4)) ||
#ifdef	SUPPORT_X0201
	  (!strncmp(ptr, JIS_X0201, 3)) ||
#endif	/* SUPPORT_X0201 */
	  (!strncmp(ptr, JIS_X0212, 4)) ||
	  (!strncmp(ptr, JIS_OUT1, 3)) || (!strncmp(ptr, JIS_OUT2, 3))) {
	return(JIS_CODE);
      } else {
	ptr++;
      }
    } else if (*((unsigned char*)ptr) > 0x80) {
#ifdef	SUPPORT_X0201
      if ((*ptr == EUC_SS2) || (*ptr == EUC_SS3)) {
	return(EUC_CODE);
      }
#endif	/* SUPPORT_X0201 */
      if (((*((unsigned char*)ptr) > 0xa0) &&
	   (*((unsigned char*)ptr) < 0xe0)) ||
	  (*((unsigned char*)ptr) > 0xfc)) {
	return(EUC_CODE);
      } else if (*((unsigned char*)ptr) >= 0xe0) {
	ptr++;
	if (*((unsigned char*)ptr) <= 0xa0) {
	  if ((*((unsigned char*)ptr + 1) < 0x40) ||
	      (*((unsigned char*)ptr + 1) == 0x7f) ||
	      (*((unsigned char*)ptr + 1) > 0xfc)) {
	    return(UNKNOWN_CODE);
	  } else {
	    return(SJIS_CODE);
	  }
	} else if (*((unsigned char*)ptr) > 0xfc) {
	  return(EUC_CODE);
	} else {
	  ptr++;
	  status = (int)UNKNOWN_CODE;
	}
      } else {
	if ((*((unsigned char*)ptr + 1) < 0x40) ||
	    (*((unsigned char*)ptr + 1) == 0x7f) ||
	    (*((unsigned char*)ptr + 1) > 0xfc)) {
	  return(UNKNOWN_CODE);
	} else {
	  return(SJIS_CODE);
	}
      }
    } else {
      ptr++;
    }
  }
  if (status) {
    return((KANJICODE)status);
  }
  return(ASCII_CODE);
}
#else	/* !notdef */
/*
 * 漢字コード判別
 * (JIS-X0201 を考慮しない)
 */

#ifdef	__STDC__
KANJICODE KANJILIBFUNC discriminate_code(char *ptr)
#else	/* !__STDC__ */
KANJICODE	discriminate_code(ptr)
     char	*ptr;
#endif	/* !__STDC__ */
{
  char	*ptr1;
  int	status;

  status = (1 << ASCII_CODE);
  ptr1 = ptr;
  while (*ptr) {
    if (*ptr == ESC_CODE) {
      if ((!strncmp(ptr, JIS_IN1, 3)) || (!strncmp(ptr, JIS_IN2, 3)) ||
	  (!strncmp(ptr, JIS_IN3, 4)) ||
	  (!strncmp(ptr, JIS_OUT1, 3)) || (!strncmp(ptr, JIS_OUT2, 3))) {
	status |= (1 << JIS_CODE);
      }
      ptr++;
      while (*ptr) {	/* エスケープシーケンスをスキップ */
	if ((*((unsigned char*)ptr) < 0x30) ||
	    (*((unsigned char*)ptr) > 0x7e)) {
	  ptr++;
	} else {
	  ptr++;
	  break;
	}
      }
    } else {
      if (*((unsigned char*)ptr) >= 0x80) {
	status |= ((1 << EUC_CODE) | (1 << SJIS_CODE));
	status &= ~(1 << ASCII_CODE);
      }
      ptr++;
    }
  }
  ptr = ptr1;
  while (*ptr) {
    if (*((unsigned char*)ptr) < 0x80) {
      ptr++;
    } else if ((*((unsigned char*)ptr) == EUC_SS2) ||
	       (*((unsigned char*)ptr) == EUC_SS3)) {
      status &= ~(1 << EUC_CODE);
      break;
    } else if ((*((unsigned char*)ptr) <= 0xa0) ||
	       (*((unsigned char*)ptr) >= 0xff)) {
      status &= ~(1 << EUC_CODE);
      break;
    } else {
      ptr++;
      if (*((unsigned char*)ptr) <= 0xa0) {
	status &= ~(1 << EUC_CODE);
	break;
      }
      ptr++;
    }
  }
  ptr = ptr1;
  while (*ptr) {
    if (*((unsigned char*)ptr) < 0x7f) {
      ptr++;
    } else if ((*((unsigned char*)ptr) >= 0xa0) &&
	       (*((unsigned char*)ptr) <= 0xdf)) {
      status &= ~(1 << SJIS_CODE);
      break;
    } else if ((*ptr == 0x7f) || (*((unsigned char*)ptr) == 0x80) ||
	       (*((unsigned char*)ptr) >= 0xf0)) {
      status &= ~(1 << SJIS_CODE);
      break;
    } else {
      ptr++;
      if ((*((unsigned char*)ptr) < 0x40) || (*ptr == 0x7f) ||
	  (*((unsigned char*)ptr) > 0xfc)) {
	status &= ~(1 << SJIS_CODE);
	break;
      }
      ptr++;
    }
  }
  if (!(status & (1 << ASCII_CODE))) {
    status &= ~(1 << JIS_CODE);
  }
  if (status & (1 << EUC_CODE)) {
    if (status & (1 << SJIS_CODE)) {
      return(UNKNOWN_CODE);
    }
    return(EUC_CODE);
  } else if (status & (1 << SJIS_CODE)) {
    return(SJIS_CODE);
  } else if (status & (1 << JIS_CODE)) {
    return(JIS_CODE);
  } else if (status & (1 << ASCII_CODE)) {
    return(ASCII_CODE);
  }
  return(ERROR_CODE);
}
#endif	/* !notdef */

#ifdef	SUPPORT_X0201
#ifdef	notdef
/*
 * 漢字コード判別
 * (JIS-X0201 を考慮する)
 */

#ifdef	__STDC__
KANJICODE KANJILIBFUNC discriminate_code2(char *ptr)
#else	/* !__STDC__ */
KANJICODE	discriminate_code2(ptr)
     char	*ptr;
#endif	/* !__STDC__ */
{
  register int	status = 0;

  while (*ptr) {
    if (*ptr == ESC_CODE) {
      if ((!strncmp(ptr, JIS_IN1, 3)) || (!strncmp(ptr, JIS_IN2, 3)) ||
	  (!strncmp(ptr, JIS_IN3, 4)) ||
#ifdef	SUPPORT_X0201
	  (!strncmp(ptr, JIS_X0201, 3)) ||
#endif	/* SUPPORT_X0201 */
	  (!strncmp(ptr, JIS_X0212, 4)) ||
	  (!strncmp(ptr, JIS_OUT1, 3)) || (!strncmp(ptr, JIS_OUT2, 3))) {
	return(JIS_CODE);
      } else {
	ptr++;
      }
    } else if (*((unsigned char*)ptr) > 0x80) {
#ifdef	SUPPORT_X0201
      if (*((unsigned char*)ptr) == EUC_SS2) {
	if ((*((unsigned char*)ptr + 1) > 0xa0) &&
	    (*((unsigned char*)ptr + 1) < 0xe0)) {
	  status = UNKNOWN_CODE;
	  ptr += 2;
	  continue;
	}
      } else if ((*((unsigned char*)ptr) > 0xa0) &&
		 (*((unsigned char*)ptr) < 0xe0)) {
	if (*((unsigned char*)ptr + 1) <= 0xa0) {
	  return(SJIS_CODE);
	} else if (*((unsigned char*)ptr + 1) > 0xfc) {
	  return(EUC_CODE);
	} else {
	  status = UNKNOWN_CODE;
	  ptr += 2;
	  continue;
	}
      }
#endif	/* SUPPORT_X0201 */
      if (((*((unsigned char*)ptr) > 0xa0) &&
	   (*((unsigned char*)ptr) < 0xe0)) ||
	  (*((unsigned char*)ptr) > 0xfc)) {
	return(EUC_CODE);
      } else if (*((unsigned char*)ptr) >= 0xe0) {
	ptr++;
	if (*((unsigned char*)ptr) <= 0xa0) {
	  if ((*((unsigned char*)ptr + 1) < 0x40) ||
	      (*((unsigned char*)ptr + 1) == 0x7f) ||
	      (*((unsigned char*)ptr + 1) > 0xfc)) {
	    return(UNKNOWN_CODE);
	  } else {
	    return(SJIS_CODE);
	  }
	} else if (*((unsigned char*)ptr) > 0xfc) {
	  return(EUC_CODE);
	} else {
	  ptr++;
	  status = (int)UNKNOWN_CODE;
	}
      } else {
	if ((*((unsigned char*)ptr + 1) < 0x40) ||
	    (*((unsigned char*)ptr + 1) == 0x7f) ||
	    (*((unsigned char*)ptr + 1) > 0xfc)) {
	  return(UNKNOWN_CODE);
	} else {
	  return(SJIS_CODE);
	}
      }
#ifdef	notdef
    } else if ((*ptr == X0201_IN_CODE) || (*ptr == X0201_OUT_CODE)) {
      return(SJIS_CODE);
#endif	/* notdef */
    } else {
      ptr++;
    }
  }
  if (status) {
    return((KANJICODE)status);
  }
  return(ASCII_CODE);
}
#else	/* !notdef */
/*
 * 漢字コード判別
 * (JIS-X0201 を考慮する)
 */

#ifdef	__STDC__
KANJICODE KANJILIBFUNC discriminate_code2(char *ptr)
#else	/* !__STDC__ */
KANJICODE	discriminate_code2(ptr)
     char	*ptr;
#endif	/* !__STDC__ */
{
  char	*ptr1;
  int	status;

  status = (1 << ASCII_CODE);
  ptr1 = ptr;
  while (*ptr) {
    if (*ptr == ESC_CODE) {
      if ((!strncmp(ptr, JIS_IN1, 3)) || (!strncmp(ptr, JIS_IN2, 3)) ||
	  (!strncmp(ptr, JIS_IN3, 4))) {
	status |= (1 << JIS_CODE);
#ifdef	CHECK_CHAR
	jis_char_mask |= KANJI_MASK;
#endif	/* CHECK_CHAR */
      } else if ((!strncmp(ptr, JIS_OUT1, 3)) ||
		 (!strncmp(ptr, JIS_OUT2, 3))) {
	status |= (1 << JIS_CODE);
      } else if (!strncmp(ptr, JIS_X0201, 3)) {
	status |= (1 << JIS_CODE);
#ifdef	CHECK_CHAR
	jis_char_mask |= KANA_MASK;
#endif	/* CHECK_CHAR */
      } else if (!strncmp(ptr, JIS_X0212, 4)) {
	status |= (1 << JIS_CODE);
#ifdef	CHECK_CHAR
	jis_char_mask |= HOJO_MASK;
#endif	/* CHECK_CHAR */
      }
      ptr++;
      while (*ptr) {	/* エスケープシーケンスをスキップ */
	if ((*((unsigned char*)ptr) < 0x30) ||
	    (*((unsigned char*)ptr) > 0x7e)) {
	  ptr++;
	} else {
	  ptr++;
	  break;
	}
      }
    } else if ((*ptr == X0201_IN_CODE) || (*ptr == X0201_OUT_CODE)) {
      status |= X0201_CODE_MASK;
      ptr++;
#ifdef	CHECK_CHAR
      jis_char_mask |= KANA_MASK;
#endif	/* CHECK_CHAR */
    } else {
      if (*((unsigned char*)ptr) >= 0x80) {
	status |= ((1 << EUC_CODE) | (1 << SJIS_CODE));
	status &= ~(1 << ASCII_CODE);
#ifdef	CHECK_CHAR
      } else {
	jis_char_mask |= ROMAN_MASK;
#endif	/* CHECK_CHAR */
      }
      ptr++;
    }
  }
  ptr = ptr1;
  while (*ptr) {
    if (*((unsigned char*)ptr) < 0x80) {
      ptr++;
#ifdef	CHECK_CHAR
      euc_char_mask |= ROMAN_MASK;
#endif	/* CHECK_CHAR */
    } else if (*((unsigned char*)ptr) == EUC_SS2) {
      ptr++;
      if ((*((unsigned char*)ptr) < 0xa0) ||
	  (*((unsigned char*)ptr) >= 0xe0)) {
	status &= ~(1 << EUC_CODE);
	break;
      }
      ptr++;
#ifdef	CHECK_CHAR
      euc_char_mask |= KANA_MASK;
#endif	/* CHECK_CHAR */
    } else if (*((unsigned char*)ptr) == EUC_SS3) {
      ptr++;
      if (*((unsigned char*)ptr) <= 0xa0) {
	status &= ~(1 << EUC_CODE);
	break;
      }
      ptr++;
      if (*((unsigned char*)ptr) <= 0xa0) {
	status &= ~(1 << EUC_CODE);
	break;
      }
      ptr++;
#ifdef	CHECK_CHAR
      euc_char_mask |= HOJO_MASK;
#endif	/* CHECK_CHAR */
    } else if ((*((unsigned char*)ptr) <= 0xa0) ||
	       (*((unsigned char*)ptr) >= 0xff)) {
      status &= ~(1 << EUC_CODE);
      break;
    } else {
      ptr++;
      if (*((unsigned char*)ptr) <= 0xa0) {
	status &= ~(1 << EUC_CODE);
	break;
      }
      ptr++;
#ifdef	CHECK_CHAR
      euc_char_mask |= KANJI_MASK;
#endif	/* CHECK_CHAR */
    }
  }
  ptr = ptr1;
  while (*ptr) {
    if (*((unsigned char*)ptr) < 0x7f) {
      ptr++;
#ifdef	CHECK_CHAR
      sjis_char_mask |= ROMAN_MASK;
#endif	/* CHECK_CHAR */
    } else if ((*((unsigned char*)ptr) >= 0xa0) &&
	       (*((unsigned char*)ptr) <= 0xdf)) {
      ptr++;
#ifdef	CHECK_CHAR
      sjis_char_mask |= KANA_MASK;
#endif	/* CHECK_CHAR */
    } else if ((*ptr == 0x7f) || (*((unsigned char*)ptr) == 0x80) ||
	       (*((unsigned char*)ptr) >= 0xf0)) {
      status &= ~(1 << SJIS_CODE);
      break;
    } else {
      ptr++;
      if ((*((unsigned char*)ptr) < 0x40) || (*ptr == 0x7f) ||
	  (*((unsigned char*)ptr) > 0xfc)) {
	status &= ~(1 << SJIS_CODE);
	break;
      }
      ptr++;
#ifdef	CHECK_CHAR
      sjis_char_mask |= KANJI_MASK;
#endif	/* CHECK_CHAR */
    }
  }
  if (!(status & (1 << ASCII_CODE))) {
    status &= ~(1 << JIS_CODE);
  }
  if ((status & X0201_CODE_MASK) && (!(status & ((1 << JIS_CODE) |
						 (1 << SJIS_CODE) |
						 (1 << EUC_CODE))))) {
#ifdef	CHECK_CHAR
    char_mask = sjis_char_mask;
#endif	/* CHECK_CHAR */
    return(SJIS_CODE);
  }
  if (status & (1 << EUC_CODE)) {
    if (status & (1 << SJIS_CODE)) {
      return(UNKNOWN_CODE);
    }
#ifdef	CHECK_CHAR
    char_mask = euc_char_mask;
#endif	/* CHECK_CHAR */
    return(EUC_CODE);
  } else if (status & (1 << SJIS_CODE)) {
#ifdef	CHECK_CHAR
    char_mask = sjis_char_mask;
#endif	/* CHECK_CHAR */
    return(SJIS_CODE);
  } else if (status & (1 << JIS_CODE)) {
#ifdef	CHECK_CHAR
    char_mask = jis_char_mask;
#endif	/* CHECK_CHAR */
    return(JIS_CODE);
  } else if (status & (1 << ASCII_CODE)) {
#ifdef	CHECK_CHAR
    char_mask = jis_char_mask;
#endif	/* CHECK_CHAR */
    return(ASCII_CODE);
  }
  return(ERROR_CODE);
}
#endif	/* !notdef */
#endif	/* SUPPORT_X0201 */

/*
 * 文字種別情報クリア
 */

#ifdef	CHECK_CHAR
#ifdef	__STDC__
void	reset_char_mask(void)
#else	/* !__STDC__ */
void	reset_char_mask()
#endif	/* !__STDC__ */
{
  jis_char_mask = sjis_char_mask = euc_char_mask = 0;
}
#endif	/* CHECK_CHAR */

/*
 * 漢字コード自動判別→JIS 漢字コード文字列変換
 */

#ifdef	__STDC__
KANJICODE KANJILIBFUNC to_jis(char *ptr1, char *ptr2, KANJICODE default_code)
#else	/* !__STDC__ */
KANJICODE	to_jis(ptr1, ptr2, default_code)
     char	*ptr1;
     char	*ptr2;
     KANJICODE	default_code;
#endif	/* !__STDC__ */
{
#ifdef	SUPPORT_X0201
  switch (discriminate_code2(ptr2)) {
#else	/* !SUPPORT_X0201 */
  switch (discriminate_code(ptr2)) {
#endif	/* !SUPPORT_X0201 */
  case ASCII_CODE:
    str_to_jis(ptr1, ptr2);
    return(ASCII_CODE);
    break;
  case JIS_CODE:
    jis_to_jis(ptr1, ptr2);
    return(JIS_CODE);
    break;
  case SJIS_CODE:
    sjis_to_jis(ptr1, ptr2);
    return(SJIS_CODE);
    break;
  case EUC_CODE:
    euc_to_jis(ptr1, ptr2);
    return(EUC_CODE);
    break;
  default:
    break;
  }
  switch (default_code) {
  case ASCII_CODE:
    str_to_jis(ptr1, ptr2);
    return(ASCII_CODE);
    break;
  case JIS_CODE:
    jis_to_jis(ptr1, ptr2);
    return(JIS_CODE);
    break;
  case SJIS_CODE:
    sjis_to_jis(ptr1, ptr2);
    return(SJIS_CODE);
    break;
  case EUC_CODE:
    euc_to_jis(ptr1, ptr2);
    return(EUC_CODE);
    break;
  default:
    break;
  }
  str_to_jis(ptr1, ptr2);
  return(UNKNOWN_CODE);
}

/*
 * 漢字コード自動判別→SJIS 漢字コード文字列変換
 */

#ifdef	__STDC__
KANJICODE KANJILIBFUNC to_sjis(char *ptr1, char *ptr2, KANJICODE default_code)
#else	/* !__STDC__ */
KANJICODE	to_sjis(ptr1, ptr2, default_code)
     char	*ptr1;
     char	*ptr2;
     KANJICODE	default_code;
#endif	/* !__STDC__ */
{
#ifdef	SUPPORT_X0201
  switch (discriminate_code2(ptr2)) {
#else	/* !SUPPORT_X0201 */
  switch (discriminate_code(ptr2)) {
#endif	/* !SUPPORT_X0201 */
  case ASCII_CODE:
    str_to_sjis(ptr1, ptr2);
    return(ASCII_CODE);
    break;
  case JIS_CODE:
    jis_to_sjis(ptr1, ptr2);
    return(JIS_CODE);
    break;
  case SJIS_CODE:
    str_to_str(ptr1, ptr2);
    return(SJIS_CODE);
    break;
  case EUC_CODE:
    euc_to_sjis(ptr1, ptr2);
    return(EUC_CODE);
    break;
  default:
    break;
  }
  switch (default_code) {
  case ASCII_CODE:
    str_to_sjis(ptr1, ptr2);
    return(ASCII_CODE);
    break;
  case JIS_CODE:
    jis_to_sjis(ptr1, ptr2);
    return(JIS_CODE);
    break;
  case SJIS_CODE:
    str_to_sjis(ptr1, ptr2);
    return(SJIS_CODE);
    break;
  case EUC_CODE:
    euc_to_sjis(ptr1, ptr2);
    return(EUC_CODE);
    break;
  default:
    break;
  }
  str_to_sjis(ptr1, ptr2);
  return(UNKNOWN_CODE);
}

/*
 * 漢字コード自動判別→EUC 漢字コード文字列変換
 */

#ifdef	__STDC__
KANJICODE KANJILIBFUNC to_euc(char *ptr1, char *ptr2, KANJICODE default_code)
#else	/* !__STDC__ */
KANJICODE	to_euc(ptr1, ptr2, default_code)
     char	*ptr1;
     char	*ptr2;
     KANJICODE	default_code;
#endif	/* !__STDC__ */
{
#ifdef	SUPPORT_X0201
  switch (discriminate_code2(ptr2)) {
#else	/* !SUPPORT_X0201 */
  switch (discriminate_code(ptr2)) {
#endif	/* !SUPPORT_X0201 */
  case ASCII_CODE:
    str_to_euc(ptr1, ptr2);
    return(ASCII_CODE);
    break;
  case JIS_CODE:
    jis_to_euc(ptr1, ptr2);
    return(JIS_CODE);
    break;
  case SJIS_CODE:
    sjis_to_euc(ptr1, ptr2);
    return(SJIS_CODE);
    break;
  case EUC_CODE:
    str_to_euc(ptr1, ptr2);
    return(EUC_CODE);
    break;
  default:
    break;
  }
  switch (default_code) {
  case ASCII_CODE:
    str_to_euc(ptr1, ptr2);
    return(ASCII_CODE);
    break;
  case JIS_CODE:
    jis_to_euc(ptr1, ptr2);
    return(JIS_CODE);
    break;
  case SJIS_CODE:
    sjis_to_euc(ptr1, ptr2);
    return(SJIS_CODE);
    break;
  case EUC_CODE:
    str_to_euc(ptr1, ptr2);
    return(EUC_CODE);
    break;
  default:
    break;
  }
  str_to_euc(ptr1, ptr2);
  return(UNKNOWN_CODE);
}

/*
 * 漢字文字列コピー(指定幅)
 */

#ifdef	__STDC__
int KANJILIBFUNC kanji_strncpy(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	kanji_strncpy(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  register int	kanji = 0;
  char		*ptr3;
  
  ptr3 = ptr2;
  while (count > 0) {
    if (!*ptr2) {
      break;
    } else if (*ptr2 == ESC_CODE) {
      if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	
	/*	漢字指示	*/
	
	kanji = 1;
	strcpy(ptr1, jis_in);
	ptr1 += jis_in_len;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_IN3, 4)) {
	
	/*	漢字指示	*/
	
	kanji = 1;
	strcpy(ptr1, jis_in);
	ptr1 += jis_in_len;
	ptr2 += 4;
      } else if ((!strncmp(ptr2, JIS_OUT1, 3))
		 || (!strncmp(ptr2, JIS_OUT2, 3))) {
	
	/*	ローマ字指示	*/
	
	kanji = 0;
	strcpy(ptr1, jis_out);
	ptr1 += jis_out_len;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_X0212, 4)) {
	
	/*	補助漢字指示	*/
	
	kanji = 2;
	strcpy(ptr1, JIS_X0212);
	ptr1 += 4;
	ptr2 += 4;
      } else {
	*ptr1++ = *ptr2++;
	count--;
      }
    } else if (kanji || (*ptr2 & 0x80)) {
      if (count < 2) {
	count = 0;
      } else {
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	count -= 2;
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
    }
  }
  if (kanji) {
    strcpy(ptr1, jis_out);
    ptr1 += jis_out_len;
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * 漢字文字列コピー(指定幅,TAB 展開付)
 */

#ifdef	__STDC__
int KANJILIBFUNC kanji_tab_strncpy(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	kanji_tab_strncpy(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  register int	kanji = 0;
  char		*ptr3;
  register int	index;
  
  ptr3 = ptr2;
  index = 0;
  while (count > 0) {
    if (!*ptr2) {
      break;
    } else if (*ptr2 == ESC_CODE) {
      if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	
	/*	漢字指示	*/
	
	kanji = 1;
	strcpy(ptr1, jis_in);
	ptr1 += jis_in_len;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_IN3, 4)) {
	
	/*	漢字指示	*/
	
	kanji = 1;
	strcpy(ptr1, jis_in);
	ptr1 += jis_in_len;
	ptr2 += 4;
      } else if ((!strncmp(ptr2, JIS_OUT1, 3))
		 || (!strncmp(ptr2, JIS_OUT2, 3))) {
	
	/*	ローマ字指示	*/
	
	kanji = 0;
	strcpy(ptr1, jis_out);
	ptr1 += jis_out_len;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_X0212, 4)) {
	
	/*	補助漢字指示	*/
	
	kanji = 2;
	strcpy(ptr1, JIS_X0212);
	ptr1 += 4;
	ptr2 += 4;
      } else {
	*ptr1++ = *ptr2++;
	count--;
      }
    } else if (kanji || (*ptr2 & 0x80)) {
      if (count < 2) {
	count = 0;
      } else {
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	count -= 2;
	index += 2;
      }
    } else if (*((unsigned char*)ptr2) < ' ') {
      if (*ptr2 == '\t') {
	do {
	  *ptr1++ = ' ';
	  count--;
	} while ((++index % 8) && (count > 0));
	ptr2++;
      } else {
#ifdef	DISPLAY_CTRL
	if (count < 2) {
	  count = 0;
	} else {
	  *ptr1++ = '^';
	  *ptr1++ = '@' + *ptr2++;
	  count -= 2;
	  index += 2;
	}
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
	count--;
	index++;
#endif	/* !DISPLAY_CTRL */
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
      index++;
    }
  }
  if (kanji) {
    strcpy(ptr1, jis_out);
    ptr1 += jis_out_len;
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * 漢字文字列コピー(指定幅,パディング有)
 */

#ifdef	__STDC__
int KANJILIBFUNC kanji_strncpy2(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	kanji_strncpy2(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  register int	kanji = 0;
  char		*ptr3;

  ptr3 = ptr2;
  while (count > 0) {
    if (!*ptr2) {
      break;
    } else if (*ptr2 == ESC_CODE) {
      if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	
	/*	漢字指示	*/
	
	kanji = 1;
	strcpy(ptr1, jis_in);
	ptr1 += jis_in_len;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_IN3, 4)) {
	
	/*	漢字指示	*/
	
	kanji = 1;
	strcpy(ptr1, jis_in);
	ptr1 += jis_in_len;
	ptr2 += 4;
      } else if ((!strncmp(ptr2, JIS_OUT1, 3))
		 || (!strncmp(ptr2, JIS_OUT2, 3))) {
	
	/*	ローマ字指示	*/
	
	kanji = 0;
	strcpy(ptr1, jis_out);
	ptr1 += jis_out_len;
	ptr2 += 3;
      } else if (!strncmp(ptr2, JIS_X0212, 4)) {
	
	/*	補助漢字指示	*/
	
	kanji = 2;
	strcpy(ptr1, JIS_X0212);
	ptr1 += 4;
	ptr2 += 4;
      } else {
	*ptr1++ = *ptr2++;
	count--;
      }
    } else if (kanji || (*ptr2 & 0x80)) {
      if (count < 2) {
	break;
      } else {
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	count -= 2;
      }
    } else {
      *ptr1++ = *ptr2++;
      count--;
    }
  }
  if (kanji) {
    strcpy(ptr1, jis_out);
    ptr1 += jis_out_len;
  }
  while (count-- > 0) {
    *ptr1++ = ' ';
  }
  *ptr1 = '\0';
  return(ptr2 - ptr3);
}

/*
 * 漢字文字列コピー(指定位置)
 */

#ifdef	__STDC__
int KANJILIBFUNC kanji_strcpy2(char *ptr1, char *ptr2, int count)
#else	/* !__STDC__ */
int	kanji_strcpy2(ptr1, ptr2, count)
     char	*ptr1;
     char	*ptr2;
     int	count;
#endif	/* !__STDC__ */
{
  register int	kanji = 0;

  while (count > 0) {
    while (*ptr2) {
      if (*ptr2 == ESC_CODE) {
	if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	  kanji = 1;
	  ptr2 += 3;
	} else if (!strncmp(ptr2, JIS_IN3, 4)) {
	  ptr2 += 4;
	  kanji = 1;
	} else if ((!strncmp(ptr2, JIS_OUT1, 3))
		   || (!strncmp(ptr2, JIS_OUT2, 3))) {
	  ptr2 += 3;
	  kanji = 0;
	} else if (!strncmp(ptr2, JIS_X0212, 4)) {
	  ptr2 += 4;
	  kanji = 2;
	} else {
	  ptr2++;
	  count--;
	}
      } else if (kanji || (*ptr2 & 0x80)) {
	ptr2 += 2;
	count -= 2;
      } else {
	ptr2++;
	count--;
      }
    }
  }
  switch (kanji) {
  case 1:
    sprintf((char*)ptr1, "%s%s", jis_in, ptr2);
    break;
  case 2:
#ifdef	DELETE_X0212
    strcpy(ptr1, ptr2);
#else	/* !DELETE_X0212 */
    sprintf((char*)ptr1, "%s%s", JIS_X0212, ptr2);
#endif	/* !DELETE_X0212 */
    break;
  default:
    strcpy(ptr1, ptr2);
    break;
  }
  return(strlen(ptr2));
}

/*
 * 漢字文字列カウント
 */

#ifdef	__STDC__
int KANJILIBFUNC kanji_strlen(char *ptr)
#else	/* !__STDC__ */
int	kanji_strlen(ptr)
     char	*ptr;
#endif	/* !__STDC__ */
{
  register int	count = 0;

  while (*ptr) {
    if (*ptr == ESC_CODE) {
      if ((!strncmp(ptr, JIS_IN1, 3)) || (!strncmp(ptr, JIS_IN2, 3))) {
	ptr += 3;
#ifdef	SUPPORT_X0201
      } else if (!strncmp(ptr, JIS_X0201, 3)) {
	ptr += 3;
#endif	/* SUPPORT_X0201 */
      } else if ((!strncmp(ptr, JIS_IN3, 4))
		 || (!strncmp(ptr, JIS_X0212, 4))) {
	ptr += 4;
      } else if ((!strncmp(ptr, JIS_OUT1, 3))
		 || (!strncmp(ptr, JIS_OUT2, 3))) {
	ptr += 3;
      } else {
	ptr++;
	count++;
      }
    } else {
      ptr++;
      count++;
    }
  }
  return(count);
}

/*
 * 漢字文字列表示
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	kanji_printf(int default_code, char *fmt, int arg1, int arg2,
		     int arg3, int arg4, int arg5, int arg6)
#else	/* !__STDC__ */
void	kanji_printf(default_code, fmt, arg1, arg2, arg3, arg4, arg5, arg6)
     int	default_code;
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6;
#endif	/* !__STDC__ */
{
  kanji_fprintf(default_code, stdout, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	kanji_printf(int default_code, char *fmt, ...)
#else	/* !__STDC__ */
void	kanji_printf(default_code, fmt, va_alist)
     int	default_code;
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  
#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
#ifdef	notdef
  bsprintf(buff1, fmt, ap);
#else	/* !notdef */
  vsprintf(buff1, fmt, ap);
#endif	/* !notdef */
  switch (print_code) {
  case JIS_CODE:
    to_jis(buff2, buff1, default_code);
    break;
  case SJIS_CODE:
    to_sjis(buff2, buff1, default_code);
    break;
  case EUC_CODE:
    to_euc(buff2, buff1, default_code);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  printf("%s", buff2);
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * 漢字文字列出力
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	kanji_fprintf(int default_code, FILE *fp, char *fmt,
		      int arg1, int arg2, int arg3, int arg4,
		      int arg5, int arg6)
#else	/* !__STDC__ */
void	kanji_fprintf(default_code, fp, fmt, arg1, arg2, arg3, arg4,
		      arg5, arg6)
     int	default_code;
     FILE	*fp;
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];

  sprintf(buff1, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
  switch (print_code) {
  case JIS_CODE:
    to_jis(buff2, buff1, default_code);
    break;
  case SJIS_CODE:
    to_sjis(buff2, buff1, default_code);
    break;
  case EUC_CODE:
    to_euc(buff2, buff1, default_code);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	kanji_fprintf(int default_code, FILE *fp, char *fmt, ...)
#else	/* !__STDC__ */
void	kanji_fprintf(default_code, fp, fmt, va_alist)
     int	default_code;
     FILE	*fp;
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  
#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
#ifdef	notdef
  bsprintf(buff1, fmt, ap);
#else	/* !notdef */
  vsprintf(buff1, fmt, ap);
#endif	/* !notdef */
  switch (print_code) {
  case JIS_CODE:
    to_jis(buff2, buff1, default_code);
    break;
  case SJIS_CODE:
    to_sjis(buff2, buff1, default_code);
    break;
  case EUC_CODE:
    to_euc(buff2, buff1, default_code);
    break;
  default:
    str_to_str(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

#ifdef	MNEWS
/*
 * ESC 欠落 JIS 文字列復元
 */

#ifdef	__STDC__
void KANJILIBFUNC recover_jis(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
void	recover_jis(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  char			*ptr3;
  char			*ptr4;
  register CHARSET	charset = ROMAN_CHAR;

  if ((!ptr1) || (!ptr2)) {
    return;
  }
  ptr3 = ptr1;
  ptr4 = ptr2;
  while (*ptr2) {
#ifdef	notdef
    if (charset &&
	((!strncmp(ptr2, DJIS_OUT1, 2)) || (!strncmp(ptr2, DJIS_OUT2, 2)))) {
      strcpy(ptr1, jis_out);
      charset = ROMAN_CHAR;
      ptr1 += jis_out_len;
      ptr2 += 2;
      continue;
    }
    if ((!strncmp(ptr2, DJIS_IN1, 2)) || (!strncmp(ptr2, DJIS_IN2, 2))) {
      charset = KANJI_CHAR;
      strcpy(ptr1, jis_in);
      ptr1 += jis_in_len;
      ptr2 += 2;
    } else if (!strncmp(ptr2, DJIS_IN3, 3)) {
      charset = KANJI_CHAR;
      strcpy(ptr1, jis_in);
      ptr1 += jis_in_len;
      ptr2 += 3;
#ifdef	SUPPORT_X0201
    } else if (!strncmp(ptr2, DJIS_X0201, 2)) {
      charset = KANA_CHAR;
      strcpy(ptr1, JIS_X0201);
      ptr1 += 3;
      ptr2 += 2;
#endif	/* SUPPORT_X0201 */
    } else if (!strncmp(ptr2, DJIS_X0212, 3)) {
      charset = HOJO_CHAR;
      strcpy(ptr1, JIS_X0212);
      ptr1 += 4;
      ptr2 += 3;
    } else if (*ptr2 == ESC_CODE) {
      strcpy(ptr3, ptr4);
      return;
    } else {
      switch (charset) {
      case KANJI_CHAR:
      case HOJO_CHAR:
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	break;
      case ROMAN_CHAR:
      case KANA_CHAR:
      default:
	*ptr1++ = *ptr2++;
	break;
      }
    }
#else	/* !notdef */
    if (charset &&
	((!strncmp(ptr2, DJIS_OUT1, 2)) || (!strncmp(ptr2, DJIS_OUT2, 2)))) {
      strcpy(ptr1, jis_out);
      charset = ROMAN_CHAR;
      ptr1 += jis_out_len;
      ptr2 += 2;
      continue;
    } else if ((charset != KANJI_CHAR) &&
	       ((!strncmp(ptr2, DJIS_IN1, 2))
		|| (!strncmp(ptr2, DJIS_IN2, 2)))) {
      charset = KANJI_CHAR;
      strcpy(ptr1, jis_in);
      ptr1 += jis_in_len;
      ptr2 += 2;
    } else if ((charset != KANJI_CHAR) && (!strncmp(ptr2, DJIS_IN3, 3))) {
      charset = KANJI_CHAR;
      strcpy(ptr1, jis_in);
      ptr1 += jis_in_len;
      ptr2 += 3;
#ifdef	SUPPORT_X0201
    } else if ((charset != KANA_CHAR) && (!strncmp(ptr2, DJIS_X0201, 2))) {
      charset = KANA_CHAR;
      strcpy(ptr1, JIS_X0201);
      ptr1 += 3;
      ptr2 += 2;
#endif	/* SUPPORT_X0201 */
    } else if ((charset != HOJO_CHAR) && (!strncmp(ptr2, DJIS_X0212, 3))) {
      charset = HOJO_CHAR;
      strcpy(ptr1, JIS_X0212);
      ptr1 += 4;
      ptr2 += 3;
    } else if (*ptr2 == ESC_CODE) {
      strcpy(ptr3, ptr4);
      return;
    } else {
      switch (charset) {
      case KANJI_CHAR:
      case HOJO_CHAR:
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	break;
      case ROMAN_CHAR:
      case KANA_CHAR:
      default:
	*ptr1++ = *ptr2++;
	break;
      }
    }
#endif	/* !notdef */
  }
  if (charset) {
    strcpy(ptr1, jis_out);
  } else {
    *ptr1 = '\0';
  }
}
#endif	/* MNEWS */

#ifdef	DISPLAY_CTRL
/*
 * 文字列コピー
 */

#ifdef	__STDC__
static void	str_to_str(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
static void	str_to_str(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register CHARSET	charset = ROMAN_CHAR;
  
  while (*ptr2) {
    if (*ptr2 == ESC_CODE) {
      if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	
	/*	漢字指示	*/
	
	charset = KANJI_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
      } else if (!strncmp(ptr2, JIS_IN3, 4)) {
	
	/*	漢字指示	*/
	
	charset = KANJI_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
      } else if ((!strncmp(ptr2, JIS_OUT1, 3))
		 || (!strncmp(ptr2, JIS_OUT2, 3))) {
	
	/*	ローマ字指示	*/
	
	charset = ROMAN_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
#ifdef	SUPPORT_X0201
      } else if (!strncmp(ptr2, JIS_X0201, 3)) {

	/*	片仮名指示	*/

	charset = KANA_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
#endif	/* SUPPORT_X0201 */
      } else if (!strncmp(ptr2, JIS_X0212, 4)) {
	
	/*	補助漢字指示	*/
	
#ifdef	DELETE_X0212
	charset = ROMAN_CHAR;
	ptr2 += 4;
#else	/* !DELETE_X0212 */
	charset = HOJO_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
#endif	/* !DELETE_X0212 */
      } else {
#ifdef	DISPLAY_CTRL
	*ptr1++ = '^';
	*ptr1++ = '[';
	ptr2++;
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
#endif	/* !DISPLAY_CTRL */
	while (*ptr2) {
	  if ((*((unsigned char*)ptr2) < 0x30) ||
	      (*((unsigned char*)ptr2) > 0x7e)) {
	    if (*((unsigned char*)ptr2) < ' ') {
	      *ptr1++ = '^';
	      *ptr1++ = '@' + *ptr2++;
	    } else {
	      *ptr1++ = *ptr2++;
	    }
	  } else {
	    *ptr1++ = *ptr2++;
	    break;
	  }
	}
      }
    } else if ((*((unsigned char*)ptr2) < ' ') && (*ptr2 != '\t') &&
	       (*ptr2 != '\n') && (*ptr2 != '\r') && (*ptr2 != 0x0c)) {
      *ptr1++ = '^';
      *ptr1++ = '@' + *ptr2++;
    } else {
      if (charset != ROMAN_CHAR) {
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
      } else {
	*ptr1++ = *ptr2++;
      }
    }
  }
  if (charset != ROMAN_CHAR) {
    strcpy(ptr1, jis_out);
  } else {
    *ptr1 = '\0';
  }
}

/*
 * ASCII 文字列コピー
 */

#ifdef	__STDC__
static void	str_to_ascii(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
static void	str_to_ascii(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  while (*ptr2) {
    if (((*((unsigned char*)ptr2) >= ' ') &&
	 (*((unsigned char*)ptr2) < 0x7f)) || (*ptr2 == '\t') ||
	(*ptr2 == '\n') || (*ptr2 == '\r') || (*ptr2 == 0x0c)) {
      *ptr1++ = *ptr2++;
    } else if (*((unsigned char*)ptr2) < ' ') {
      *ptr1++ = '^';
      *ptr1++ = '@' + *ptr2++;
    } else {
      *ptr1++ = '\\';
      *ptr1++ = 'x';
      *ptr1++ = large_hex_char[(*((unsigned char*)ptr2) >> 4)];
      *ptr1++ = large_hex_char[*ptr2 & 0x0F];
      ptr2++;
    }
  }
  *ptr1 = '\0';
}

/*
 * JIS 文字列コピー
 */

#ifdef	__STDC__
static void	str_to_jis(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
static void	str_to_jis(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register CHARSET	charset = ROMAN_CHAR;
  
  while (*ptr2) {
    if (*ptr2 == ESC_CODE) {
      if ((!strncmp(ptr2, JIS_IN1, 3)) || (!strncmp(ptr2, JIS_IN2, 3))) {
	
	/*	漢字指示	*/
	
	charset = KANJI_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
      } else if (!strncmp(ptr2, JIS_IN3, 4)) {
	
	/*	漢字指示	*/
	
	charset = KANJI_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
      } else if ((!strncmp(ptr2, JIS_OUT1, 3))
		 || (!strncmp(ptr2, JIS_OUT2, 3))) {
	
	/*	ローマ字指示	*/
	
	charset = ROMAN_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
#ifdef	SUPPORT_X0201
      } else if (!strncmp(ptr2, JIS_X0201, 3)) {

	/*	片仮名指示	*/

	charset = KANA_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
#endif	/* SUPPORT_X0201 */
      } else if (!strncmp(ptr2, JIS_X0212, 4)) {
	
	/*	補助漢字指示	*/
	
#ifdef	DELETE_X0212
	charset = ROMAN_CHAR;
	ptr2 += 4;
#else	/* !DELETE_X0212 */
	charset = HOJO_CHAR;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
#endif	/* !DELETE_X0212 */
      } else {
#ifdef	DISPLAY_CTRL
	*ptr1++ = '^';
	*ptr1++ = '[';
	ptr2++;
#else	/* !DISPLAY_CTRL */
	*ptr1++ = *ptr2++;
#endif	/* !DISPLAY_CTRL */
	while (*ptr2) {
	  if ((*((unsigned char*)ptr2) < 0x30) ||
	      (*((unsigned char*)ptr2) > 0x7e)) {
	    if (*((unsigned char*)ptr2) < ' ') {
	      *ptr1++ = '^';
	      *ptr1++ = '@' + *ptr2++;
	    } else {
	      *ptr1++ = *ptr2++;
	    }
	  } else {
	    *ptr1++ = *ptr2++;
	    break;
	  }
	}
      }
    } else if ((*((unsigned char*)ptr2) < ' ') && (*ptr2 != '\t') &&
	       (*ptr2 != '\n') && (*ptr2 != '\r') && (*ptr2 != 0x0c)) {
      *ptr1++ = '^';
      *ptr1++ = '@' + *ptr2++;
    } else if (*((unsigned char*)ptr2) >= 0x7F) {
      *ptr1++ = '\\';
      *ptr1++ = 'x';
      *ptr1++ = large_hex_char[(*((unsigned char*)ptr2) >> 4)];
      *ptr1++ = large_hex_char[*ptr2 & 0x0F];
      ptr2++;
    } else {
      if (charset != ROMAN_CHAR) {
	*ptr1++ = *ptr2++;
	*ptr1++ = *ptr2++;
      } else {
	*ptr1++ = *ptr2++;
      }
    }
  }
  if (charset != ROMAN_CHAR) {
    strcpy(ptr1, jis_out);
  } else {
    *ptr1 = '\0';
  }
}

/*
 * SJIS 文字列コピー
 */

#ifdef	__STDC__
static void	str_to_sjis(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
static void	str_to_sjis(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  while (*ptr2) {
    if (((*((unsigned char*)ptr2) >= ' ') &&
	 (*((unsigned char*)ptr2) < 0x7f)) || (*ptr2 == '\t') ||
	(*ptr2 == '\n') || (*ptr2 == '\r') || (*ptr2 == 0x0c) ||
	((*((unsigned char*)ptr2) >= 0xa0) &&
	 (*((unsigned char*)ptr2) <= 0xdf))) {
      *ptr1++ = *ptr2++;
    } else if (*((unsigned char*)ptr2) < ' ') {
      *ptr1++ = '^';
      *ptr1++ = '@' + *ptr2++;
    } else {
      *ptr1++ = '\\';
      *ptr1++ = 'x';
      *ptr1++ = large_hex_char[(*((unsigned char*)ptr2) >> 4)];
      *ptr1++ = large_hex_char[*ptr2 & 0x0F];
      ptr2++;
    }
  }
  *ptr1 = '\0';
}

/*
 * EUC 文字列コピー
 */

#ifdef	__STDC__
static void	str_to_euc(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
static void	str_to_euc(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  while (*ptr2) {
    if (((*((unsigned char*)ptr2) >= ' ') &&
	 (*((unsigned char*)ptr2) < 0x7f)) ||
	((*((unsigned char*)ptr2) >= 0xa0) &&
	 (*((unsigned char*)ptr2) < 0xff)) ||
	(*((unsigned char*)ptr2) == EUC_SS2) ||
	(*((unsigned char*)ptr2) == EUC_SS3) ||
	(*ptr2 == '\t') || (*ptr2 == '\n') || (*ptr2 == '\r') ||
	(*ptr2 == 0x0c)) {
      *ptr1++ = *ptr2++;
    } else if (*((unsigned char*)ptr2) < ' ') {
      *ptr1++ = '^';
      *ptr1++ = '@' + *ptr2++;
    } else {
      *ptr1++ = '\\';
      *ptr1++ = 'x';
      *ptr1++ = large_hex_char[(*((unsigned char*)ptr2) >> 4)];
      *ptr1++ = large_hex_char[*ptr2 & 0x0F];
      ptr2++;
    }
  }
  *ptr1 = '\0';
}
#endif	/* DISPLAY_CTRL */
#ifdef	WINDOWS
//========================================================
// DllEntryPoint
//--------------------------------------------------------
// Win32 DLLの初期化・後処理 関数
//========================================================

BOOL CALLBACK DllEntryPoint(HINSTANCE hInstDLL, DWORD fdwReason,
			    LPVOID reserved)
{
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
    jis_mode(1);		/* default ... 新JIS */
    break;
  }
  return(TRUE);
}
#endif	/* WINDOWS */
