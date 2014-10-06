/*
 *
 *  MIME 処理ライブラリ
 *
 *  Copyright 1993-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : MIME Manage Library
 *  Sub system  : MIME encode/decode routine
 *  File        : mimelib.c
 *  Version     : 0.64
 *  First Edit  : 1993-08/09
 *  Last  Edit  : 1997-11/30
 *  Author      : MSR24 宅間 顯
 *
 */

#ifdef	WINDOWS
#define	BUILD_MIMELIB_DLL	/* mimelib.h をインクルードする前に必ず定義 */
#include	<windows.h>
#include	<ctype.h>
#include	"field.h"
#include	"mimelib.h"
#include	"kanjilib.h"
#define	MIME
#define	BUFF_SIZE	1024
#else	/* !WINDOWS */
#include	"compat.h"
#include	"kanjilib.h"
#ifdef	MNEWS
#include	"nntplib.h"
#include	"field.h"
#include	"mnews.h"
#endif	/* MNEWS */
#include	"mimelib.h"
#endif	/* !WINDOWS */

#ifdef	MIME
#ifdef	WINDOWS
static DWORD	error_code = 0;
#endif	/* WINDOWS */
#define	mime_charB64(c)  (((c) < 0 || (c) > 0x7F) ? -1 : \
			  mime_base64_index[(c)])

/*
 * MIME メインタイプ
 */

static char	*mime_mtype[] ={
  MIME_MTYPE_TEXT,
  MIME_MTYPE_APP,
  MIME_MTYPE_AUDIO,
  MIME_MTYPE_IMAGE,
  MIME_MTYPE_MSG,
  MIME_MTYPE_VIDEO,
  MIME_MTYPE_EXT,
  MIME_MTYPE_MULTI,
};

/*
 * MIME サブタイプ
 */

static char	*mime_stype[] ={
  MIME_STYPE_PLAIN,
  MIME_STYPE_RICH,
  MIME_STYPE_MIX,
  MIME_STYPE_PARA,
  MIME_STYPE_DIGEST,
  MIME_STYPE_ALT,
  MIME_STYPE_RFC,
  MIME_STYPE_PART,
  MIME_STYPE_EXT,
  MIME_STYPE_GIF,
  MIME_STYPE_JPEG,
  MIME_STYPE_BASIC,
  MIME_STYPE_MPEG,
  MIME_STYPE_OCT,
  MIME_STYPE_PS,
};

/*
 * MIME 転送エンコードタイプ
 */

static char	*mime_ttype[] ={
  MIME_TRANS_7BIT,
  MIME_TRANS_8BIT,
  MIME_TRANS_QP,
  MIME_TRANS_BASE64,
  MIME_TRANS_BIN,
};

/*
 * MIME BASE64 インデックス
 */

static char	mime_base64_index[] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
};

#ifdef	notdef
#define	mime_charB16(c)  (((c) < 0 || (c) > 0x7F) ? -1 : \
			  mime_base16_index[(c)])

/*
 * MIME BASE16 インデックス
 */

static char	mime_base16_index[] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};
#endif	/* notdef */

/*
 * MIME BASE64 ベース
 */

static char	mime_base64_basis[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#ifdef	SUPPORT_Q_ENCODE
static char	mime_base16_basis[] = "0123456789ABCDEF";
#endif	/* SUPPORT_Q_ENCODE */

static char	mime_especials[] = "()<>@,;:\\/[]?.=";

#ifdef	__STDC__
#ifdef	MNEWS
static char		*mime_encodeB64_bin(char*, char*, int);
static char		*mime_decodeB64_bin(char*, char*, char **, int*);
#endif	/* MNEWS */
static char		*mime_encodeB64_str(char*, char*);
static char		*mime_decodeB64_str(char*, char*, char **, int*);
static char		*mime_outputB64(char, char, char, int, char*);
static int		mime_boundaryB64(char*, char**, int*);
static char		*strcheck(char*, char*);
static char		*strcasecheck(char*, char*);
#ifndef	MNEWS
char			*strindex(char*, char*);
#endif	/* !MNEWS */
#else	/* !__STDC__ */
#ifdef	MNEWS
static char		*mime_encodeB64_bin();
static char		*mime_decodeB64_bin();
#endif	/* MNEWS */
static char		*mime_encodeB64_str();
static char		*mime_decodeB64_str();
static char		*mime_outputB64();
static int		mime_boundaryB64();
static char		*strcheck();
static char		*strcasecheck();
#ifndef	MNEWS
char			*strindex();
#endif	/* !MNEWS */
#endif	/* !__STDC__ */

#ifdef	MNEWS
/*
 * MIME エンコード(EUC 入力)
 */

#ifdef	__STDC__
int MIMELIBFUNC	mime_encode_jis(char *ptr1, char *ptr2, int hint_code)
#else	/* !__STDC__ */
int	mime_encode_jis(ptr1, ptr2, hint_code)
     char	*ptr1;
     char	*ptr2;
     int	hint_code;
#endif	/* !__STDC__ */
{
  char	buff[BUFF_SIZE];

  to_jis(buff, ptr2, hint_code);
  return(mime_encode(buff, ptr1));
}

/*
 * MIME デコード(EUC 出力)
 */

#ifdef	__STDC__
int MIMELIBFUNC	mime_decode_euc(char *ptr1, char *ptr2, int hint_code)
#else	/* !__STDC__ */
int	mime_decode_euc(ptr1, ptr2, hint_code)
     char	*ptr1;
     char	*ptr2;
     int	hint_code;
#endif	/* !__STDC__ */
{
  int	mime;
  char	buff[BUFF_SIZE];

  mime = mime_decode(ptr2, buff);
  to_euc(ptr1, buff, hint_code);
  return(mime);
}
#endif	/* MNEWS */

/*
 * MIME エンコード
 * (ISO-2022-JP2 対応は不完全)
 */

#ifdef	__STDC__
int MIMELIBFUNC	mime_encode(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
int	mime_encode(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  static struct sequence {
    char	*seq_str;
    int		char_num;
  } sequence[] = {
    {JIS_OUT1,	0},
    {JIS_OUT2,	0},
    {JIS_IN1,	2},
    {JIS_IN2,	2},
    {JIS_IN3,	2},
#ifdef	SUPPORT_JP2
    {JIS_X0212,	3},		/* JIS-X0212(補助漢字)	*/
    {"\033$A",	2},		/* GB2312-1980(Chinese)	*/
    {"\033$(C",	2},		/* KSC5601-1987(Korean)	*/
    {"\033.A",	2},		/* ISO-8859-1(Latin-1)	*/
    {"\033.B",	2},		/* ISO-8859-2(Latin-2)	*/
    {"\033.C",	2},		/* ISO-8859-3(Latin-3)	*/
    {"\033.D",	2},		/* ISO-8859-4(Latin-4)	*/
    {"\033.E",	2},		/* ISO-8859-5(Yawerty?)	*/
    {"\033.F",	2},		/* ISO-8859-6(Unknown)	*/
    {"\033.G",	2},		/* ISO-8859-7(Greek)	*/
    {"\033.H",	2},		/* ISO-8859-8(Unknown)	*/
    {"\033.I",	2},		/* ISO-8859-9(Latin-5)	*/
#endif	/* SUPPORT_JP2 */
  };
  int		mime;
  register int	count, i, j;
  int		lastchar;
  int		seq_id;
  char		*ptr3, *ptr4;
  char		buff[BUFF_SIZE];

  mime = count = lastchar = 0;
  ptr3 = ptr4 = (char*)NULL;
  seq_id = 0;
  while (*ptr1) {
    if ((*ptr1 == ' ') || (*ptr1 == '\t')) {

      /*
       * このフォールディング処理はちょっといい加減過ぎるかも。
       */

      if (count >= MAX_ENCODE_COLUMN) {
	*ptr2++ = '\n';
	*ptr2++ = '\t';
	lastchar = 0;
	count = 8;
      }
#ifdef	ENCODE_SPACE
      if (!ptr3) {
	ptr3 = ptr1;
	ptr4 = ptr2;
      }
#else	/* !ENCODE_SPACE */
      ptr3 = ptr4 = (char*)NULL;
#endif	/* !ENCODE_SPACE */
      lastchar = 0;
      *ptr2++ = *ptr1++;
      count++;
    } else if (*ptr1 == '\n') {
      lastchar = *ptr1;
      *ptr2++ = *ptr1++;
      ptr3 = (char*)NULL;
      count = 0;
    } else if ((*((unsigned char*)ptr1) >= ' ') &&
	       (*((unsigned char*)ptr1) <= 0x7F)) {
      lastchar = *ptr1;
      *ptr2++ = *ptr1++;
      ptr3 = (char*)NULL;
      count++;
    } else {
      mime = 1;
      if (ptr3) {
	ptr2 = ptr4;
      } else {
	ptr3 = ptr1;
      }
      seq_id = 0;

      /*
       * フォールディングを考慮しつつエンコードするいい加減なルーチン
       */

      while (*ptr1) {
	if (count >
	    (MAX_ENCODE_COLUMN - MIME_SEP_SIZE1 - MIME_SEP_SIZE2 - 12)) {
	  *ptr2++ = '\n';
	  *ptr2++ = '\t';
	  lastchar = 0;
	  count = 8;
	}
	if (lastchar) {
	  if (strchr(mime_especials, lastchar) == (char*)NULL) {
	    *ptr2++ = ' ';
	    count++;
	  }
	  lastchar = 0;
	}
#ifdef	WINDOWS
	wsprintf(ptr2, "%s%s%c%c%c", MIME_ENCODE_START,
		 MIME_CHARSET_ISO2022_1, MIME_SEP_CHAR, MIME_ENCODE_BASE64,
		 MIME_SEP_CHAR);
#else	/* !WINDOWS */
	sprintf(ptr2, "%s%s%c%c%c", MIME_ENCODE_START, MIME_CHARSET_ISO2022_1,
		MIME_SEP_CHAR, MIME_ENCODE_BASE64, MIME_SEP_CHAR);
#endif	/* !WINDOWS */
	while (*ptr2) {
	  ptr2++;
	  count++;
	}
	ptr3 = buff;
	if (seq_id) {
	  strcpy(ptr3, sequence[seq_id].seq_str);
	  while (*ptr3) {
	    ptr3++;
	  }
	}
	while (*ptr1) {
	  if ((count + ((int)(ptr3 - buff) * 4) / 3) >
	      (MAX_ENCODE_COLUMN - MIME_SEP_SIZE2 - 12)) {
	    break;
	  }
	  while (*ptr1 == ESC_CODE) {	/* 本当は if で十分な while 文 */
	    for (i = 0; i < sizeof(sequence) / sizeof(struct sequence); i++) {
	      j = strlen(sequence[i].seq_str);
	      if (!strncmp(ptr1, sequence[i].seq_str, j)) {
		strncpy(ptr3, ptr1, j);
		ptr1 += j;
		ptr3 += j;
		if (sequence[i].char_num) {
		  seq_id = i;
		} else {
		  seq_id = 0;
		}
		i = -1;
		break;
	      }
	    }
	    if (i >= 0) {
#ifdef	notdef
	      break;
#else	/* !notdef */
	      *ptr1 = '\0';
	      return(-1);		/* 不正なシーケンス	*/
#endif	/* !notdef */
	    }
	  }
	  if (seq_id) {			/* ここは ISO-2022-JP-2 のこと未考慮 */
	    *ptr3++ = *ptr1++;
	    *ptr3++ = *ptr1++;
	  } else {
	    if (*((unsigned char*)ptr1) > 0x7F) {
#ifdef	notdef
	      *ptr3++ = *ptr1++;	/* 予期せぬコード	*/
#else	/* !notdef */
	      *ptr2 = '\0';
	      return(-1);		/* 予期せぬコード	*/
#endif	/* !notdef */
	    } else if ((*ptr1 == ' ') || (*ptr1 == '\t')) {
#ifdef	ENCODE_SPACE
	      while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
		*ptr3++ = ptr1++;
	      }
#else	/* !ENCODE_SPACE */
	      break;
#endif	/* !ENCODE_SPACE */
	    } else {
	      break;			/* 通常文字		*/
	    }
	  }
	}
	if (seq_id) {
	  strcpy(ptr3, JIS_OUT);
	} else {
	  *ptr3 = '\0';
	}
	ptr3 = ptr2;
	ptr2 = mime_encodeB64_str(buff, ptr2);
	strcpy(ptr2, MIME_ENCODE_END);
	while (*ptr2) {
	  ptr2++;
	}
	count += strlen(ptr3);
	if (!seq_id) {
	  break;
	}
      }
      if (*ptr1 && (*ptr1 != ' ') && (*ptr1 != '\t')
	  && (!strchr(mime_especials, *ptr1))) {
	*ptr2++ = ' ';
	count++;
      }
      ptr3 = (char*)NULL;
    }
  }
  *ptr2 = '\0';
  return(mime);
}

/*
 * MIME デコード
 * (ISO-2022-JP2 対応は不完全)
 */

#ifdef	__STDC__
int MIMELIBFUNC mime_decode(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
int	mime_decode(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  int	mime;
  int	encode;
  char	*ptr3, *ptr4, *ptr5;
  char	buff[BUFF_SIZE];

  mime = 0;
  ptr4 = (char*)NULL;
  while (*ptr1) {
    ptr3 = strcheck(ptr1, MIME_ENCODE_START);
    if (ptr3 != (char*)NULL) {
      ptr5 = ptr3;
#ifdef	SUPPORT_JP2
      if ((!(ptr3 = strcasecheck(ptr5, MIME_CHARSET_ISO2022_1))) &&
	  (!(ptr3 = strcasecheck(ptr5, MIME_CHARSET_ISO2022_2)))) {
	if (!(ptr3 = strcasecheck(ptr5, MIME_CHARSET_ASCII))) {
	  ptr3 = strcasecheck(ptr5, MIME_CHARSET_ISO8859);
	  if (ptr3) {
	    if (isdigit(*ptr3)) {	/* いい加減なチェック */
	      ptr3++;
	    } else {
	      ptr3 = (char*)NULL;
	    }
	  }
	}
#else	/* !SUPPORT_JP2 */
      ptr3 = strcasecheck(ptr5, MIME_CHARSET_ISO2022_1);
      if (ptr3 == (char*)NULL) {
	ptr3 = strcasecheck(ptr5, MIME_CHARSET_ASCII);
#endif	/* !SUPPORT_JP2 */
      }
      if (ptr3) {
	if (*ptr3++ == MIME_SEP_CHAR) {
	  switch (islower(*ptr3) ? toupper(*ptr3) : *ptr3) {
#ifdef	SUPPORT_Q_DECODE
	  case MIME_ENCODE_Q:
	    encode = 1;
	    break;
#endif	/* SUPPORT_Q_DECODE */
	  case MIME_ENCODE_BASE64:
	    encode = 2;
	    break;
	  default:
	    encode = 0;
	    break;
	  }
	  ptr3++;
	  if (encode) {
	    if (*ptr3++ == MIME_SEP_CHAR) {
	      ptr1 = ptr3;
	      ptr3 = (char*)strindex(ptr1, MIME_ENCODE_END);
	      if (ptr3 != (char*)NULL) {
		strcpy(buff, ptr1);
		buff[ptr3 - ptr1] = '\0';
		mime = 1;
		if (ptr4) {
		  ptr2 = ptr4;
		  ptr4 = (char*)NULL;
		}
		if (encode == 1) {
#ifdef	SUPPORT_Q_DECODE
		  ptr2 = mime_decodeQ(buff, ptr2);
#endif	/* SUPPORT_Q_DECODE */
		} else {
		  ptr2 = mime_decodeB64_str(buff, ptr2, (char**)NULL, 0);
		}
		ptr1 = ptr3 + sizeof(MIME_ENCODE_END) - 1;
#ifdef	ENCODE_SPACE
		while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
		  ptr1++;
		}
#endif	/* ENCODE_SPACE */
		continue;
	      }
	    }
	  }
	}
      }
    }
#ifdef	ENCODE_SPACE
    if ((*ptr1 == ' ') || (*ptr1 == '\t')) {
      ptr4 = ptr2;
      while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
	*ptr2++ = *ptr1++;
      }
    } else {
      ptr4 = (char*)NULL;
      *ptr2++ = *ptr1++;
    }
#else	/* !ENCODE_SPACE */
    ptr4 = (char*)NULL;
    *ptr2++ = *ptr1++;
#endif	/* !ENCODE_SPACE */
  }
  *ptr2 = '\0';
  return(mime);
}

/*
 * MIME B64 エンコード
 */

#ifdef	__STDC__
char	*mime_encodeB64(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
char	*mime_encodeB64(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  return(mime_encodeB64_str(ptr1, ptr2));
}

/*
 * MIME B64 デコード
 */

#ifdef	__STDC__
char	*mime_decodeB64(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
char	*mime_decodeB64(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  return(mime_decodeB64_str(ptr1, ptr2, (char**)NULL, 0));
}

/*
 * MIME Q エンコード
 */

#ifdef	SUPPORT_Q_ENCODE
#ifdef	__STDC__
char	*mime_encodeQ(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
char	*mime_encodeQ(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  int	count = 0;

  while (*ptr1) {
    switch (*ptr1) {
    case '\t':
      *ptr2++ = *ptr1++;
      count++;
      break;
    case '\n':
      *ptr2++ = *ptr1++;
      count = 0;
      break;
    case ' ':
      *ptr2++ = '_';
      ptr1++;
      count++;
      break;
    default:
      if ((*((unsigned char*)ptr1) >= ' ') &&
	  (*((unsigned char*)ptr1) < 0x7F)) {
	*ptr2 = *ptr1++;
	count++;
	break;
      }
      /* break 不要 */
    case MIME_SEP_CHAR:
    case MIME_PAD_CHAR:
    case MIME_SPC_CHAR:
      *ptr2++ = MIME_PAD_CHAR;
      *ptr2++ = mime_base16_basis[(*ptr1 >> 4) & 0x0F];
      *ptr2++ = mime_base16_basis[*ptr1++ & 0x0F];
      count += 3;
      break;
    }
    if (count > (MAX_ENCODE_COLUMN - 3)) {
      *ptr2++ = '\n';
      count = 0;
    }
  }
  *ptr2 = '\0';
  return(ptr2);
}
#endif	/* SUPPORT_Q_ENCODE */

/*
 * MIME Q デコード
 */

#ifdef	SUPPORT_Q_DECODE
#ifdef	__STDC__
char	*mime_decodeQ(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
char	*mime_decodeQ(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  char	code;

  while (*ptr1) {
    if (*ptr1 == MIME_PAD_CHAR) {
      ptr1++;
      if (isdigit(*ptr1)) {
	code = (*ptr1++ - '0');
      } else if ((*ptr1 >= 'A') && (*ptr1 <= 'F')) {
	code = (*ptr1++ - 'A'+ 0xA);
      } else if ((*ptr1 >= 'a') && (*ptr1 <= 'f')) {
	code = (*ptr1++ - 'a'+ 0xA);
      } else {
#ifdef	WINDOWS
	error_code = MIME_ERROR_CODE_1;
#else	/* !WINDOWS */
	print_warning("Ignoring unrecognized code in MIME-Q.");
#endif	/* !WINDOWS */
	if (*ptr1) {
	  ptr1++;
	}
	continue;
      }
      code <<= 4;
      if (isdigit(*ptr1)) {
	code |= (*ptr1++ - '0');
      } else if ((*ptr1 >= 'A') && (*ptr1 <= 'F')) {
	code |= (*ptr1++ - 'A'+ 0xA);
      } else if ((*ptr1 >= 'a') && (*ptr1 <= 'f')) {
	code |= (*ptr1++ - 'a'+ 0xA);
      } else {
#ifdef	WINDOWS
	error_code = MIME_ERROR_CODE_1;
#else	/* !WINDOWS */
	print_warning("Ignoring unrecognized code in MIME-Q.");
#endif	/* !WINDOWS */
	if (*ptr1) {
	  ptr1++;
	}
	continue;
      }
      *ptr2++ = code;
    } else if (*ptr1 == MIME_SPC_CHAR) {
      ptr1++;
      *ptr2++ = ' ';
    } else {
      *ptr2++ = *ptr1++;
    }
  }
  *ptr2 = '\0';
  return(ptr2);
}
#endif	/* SUPPORT_Q_DECODE */

/*
 * MIME QP エンコード
 */

#ifdef	SUPPORT_QP_ENCODE
#ifdef	__STDC__
char	*mime_encodeQP(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
char	*mime_encodeQP(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  int	count = 0;

  while (*ptr1) {
    if (((*((unsigned char*)ptr1) >= ' ') && (*((unsigned char*)ptr1) < 0x7F))
	|| (*ptr1 == '\t')) {
      *ptr2++ = *ptr1++;
      count++;
    } else if (*ptr1 == '\n') {
      *ptr2 = *ptr1++;
      count = 0;
    } else {
      *ptr2++ = MIME_PAD_CHAR;
      *ptr2++ = mime_base16_basis[(*ptr1 >> 4) & 0x0F];
      *ptr2++ = mime_base16_basis[*ptr1++ & 0x0F];
      count += 3;
    }
    if (count > (MAX_ENCODE_COLUMN - 3)) {
      *ptr2++ = '\n';
      count = 0;
    }
  }
  *ptr2 = '\0';
  return(ptr2);
}
#endif	/* SUPPORT_QP_ENCODE */

/*
 * MIME QP デコード
 */

#ifdef	SUPPORT_QP_DECODE
#ifdef	__STDC__
char	*mime_decodeQP(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
char	*mime_decodeQP(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  char	code;

  while (*ptr1) {
    if (*ptr1 == MIME_PAD_CHAR) {
      ptr1++;
      if (isdigit(*ptr1)) {
	code = (*ptr1++ - '0');
      } else if ((*ptr1 >= 'A') && (*ptr1 <= 'F')) {
	code = (*ptr1++ - 'A'+ 0xA);
      } else if ((*ptr1 >= 'a') && (*ptr1 <= 'f')) {
	code = (*ptr1++ - 'a'+ 0xA);
      } else {
#ifdef	WINDOWS
	error_code = MIME_ERROR_CODE_1;
#else	/* !WINDOWS */
	print_warning("Ignoring unrecognized code in MIME-QP.");
#endif	/* !WINDOWS */
	if (*ptr1) {
	  ptr1++;
	}
	continue;
      }
      code <<= 4;
      if (isdigit(*ptr1)) {
	code |= (*ptr1++ - '0');
      } else if ((*ptr1 >= 'A') && (*ptr1 <= 'F')) {
	code |= (*ptr1++ - 'A'+ 0xA);
      } else if ((*ptr1 >= 'a') && (*ptr1 <= 'f')) {
	code |= (*ptr1++ - 'a'+ 0xA);
      } else {
#ifdef	WINDOWS
	error_code = MIME_ERROR_CODE_1;
#else	/* !WINDOWS */
	print_warning("Ignoring unrecognized code in MIME-QP.");
#endif	/* !WINDOWS */
	if (*ptr1) {
	  ptr1++;
	}
	continue;
      }
      *ptr2++ = code;
    } else {
      *ptr2++ = *ptr1++;
    }
  }
  *ptr2 = '\0';
  return(ptr2);
}
#endif	/* SUPPORT_QP_DECODE */

#ifdef	MNEWS
/*
 * MIME ファイルエンコード
 * (ファイルポインタはバイナリモードでオープンしておくこと)
 */

#ifdef	__STDC__
int MIMELIBFUNC mime_encode_file(MIME_TTYPE mime_ttype, FILE *fp1, FILE *fp2)
#else	/* !__STDC__ */
int	mime_encode_file(mime_ttype, fp1, fp2)
     MIME_TTYPE	mime_ttype;
     FILE	*fp1;
     FILE	*fp2;
#endif	/* !__STDC__ */
{
  char	buff1[BUFF_SIZE];
  char	buff2[BUFF_SIZE];
  char	*ptr;
  int	length1, length2, length3;

  length1 = 0;
  switch (mime_ttype) {
  case M_TTYPE_QP:
    if (!fgets(buff1, sizeof(buff1), fp1)) {
      return(0);
    }
    length1 = sizeof(buff1);
    mime_encodeQ(buff1, buff2);
    fputs(buff2, fp2);
    break;
  case M_TTYPE_B64:
    ptr = buff1;
    while (length1 < MIME_BIN_SIZE) {
      length2 = fread(ptr, 1, MIME_BIN_SIZE - length1, fp1);
      if (!length2) {
	if (!feof(fp1)) {
	  return(-1);
	}
	break;
      }
      length1 += length2;
      ptr += length2;
    }
    if (length1) {
      mime_encodeB64_bin(buff1, buff2, length1);
      fprintf(fp2, "%s\n", buff2);
    }
    break;
  case M_TTYPE_UNKNOWN:
  case M_TTYPE_7BIT:
  case M_TTYPE_8BIT:
  case M_TTYPE_BIN:
  default:
    ptr = buff1;
    while (length1 < sizeof(buff1)) {
      length2 = fread(ptr, 1, sizeof(buff1) - length1, fp1);
      if (!length2) {
	if (!feof(fp1)) {
	  return(-1);
	}
	break;
      }
      length1 += length2;
      ptr += length2;
    }
    ptr = buff1;
    length3 = length1;
    while (length3 > 0) {
      length2 = fwrite(ptr, 1, length3, fp2);
      if (!length2) {
	return(-1);
      }
      length3 -= length2;
      ptr += length2;
    }
    break;
  }
  return(length1);
}

/*
 * MIME ファイルデコード
 * (ファイルポインタはバイナリモードでオープンしておくこと)
 */

#ifdef	__STDC__
int MIMELIBFUNC mime_decode_file(MIME_TTYPE mime_ttype, FILE *fp1, FILE *fp2)
#else	/* !__STDC__ */
int	mime_decode_file(mime_ttype, fp1, fp2)
     MIME_TTYPE	mime_ttype;
     FILE	*fp1;
     FILE	*fp2;
#endif	/* !__STDC__ */
{
  char	buff1[BUFF_SIZE];
  char	buff2[BUFF_SIZE];
  char	*ptr;
  int	length1, length2;
  int	status;

  if (!fgets(buff1, sizeof(buff1), fp1)) {
    return(-1);
  }
  ptr = buff1;
  status = 0;
  switch (mime_ttype) {
  case M_TTYPE_QP:
    mime_decodeQ(buff1, buff2);
    if (fputs(buff2, fp2)) {
      status = 1;
    }
    break;
  case M_TTYPE_B64:
    if ((*ptr == '\n') || (*ptr == '\r') || (!*ptr)) {
      status = 1;
    } else {
      while (*ptr) {
	if (*ptr == MIME_PAD_CHAR) {
	  ptr++;
	  if ((*ptr == '\n') || (*ptr == '\r') || (!*ptr)) {
	    status = 1;
	    break;
	  }
	} else {
	  ptr++;
	}
      }
    }
    ptr = mime_decodeB64_bin(buff1, buff2, (char**)NULL, 0);
    length1 = ptr - buff2;
    ptr = buff2;
    while (length1 > 0) {
      length2 = fwrite(ptr, 1, length1, fp2);
      if (!length2) {
	return(-1);
      }
      ptr += length2;
      length1 -= length2;
    }
    break;
  case M_TTYPE_UNKNOWN:
  case M_TTYPE_7BIT:
  case M_TTYPE_8BIT:
  case M_TTYPE_BIN:
  default:
    if (fputs(buff1, fp2)) {
      status = 1;
    }
    break;
  }
  return(status);
}

/*
 * MIME BASE64 エンコード(バイナリ用)
 */

#ifdef	__STDC__
static char	*mime_encodeB64_bin(char *ptr1, char *ptr2, int size)
#else	/* !__STDC__ */
static char	*mime_encodeB64_bin(ptr1, ptr2, size)
     char	*ptr1;
     char	*ptr2;
     int	size;
#endif	/* !__STDC__ */
{
  char	code1, code2, code3;

  while (size-- > 0) {
    code1 = *ptr1++;
    if (size-- > 0) {
      code2 = *ptr1++;
      if (size-- > 0) {
	code3 = *ptr1++;
	ptr2 = mime_outputB64(code1, code2, code3, 0, ptr2);
      } else {
	ptr2 = mime_outputB64(code1, code2, 0, 1, ptr2);
	break;
      }
    } else {
      ptr2 = mime_outputB64(code1, 0, 0, 2, ptr2);
      break;
    }
  }
  *ptr2 = '\0';
  return(ptr2);
}

/*
 * MIME BASE64 デコード(バイナリ用)
 */

#ifdef	__STDC__
static char	*mime_decodeB64_bin(char *ptr1, char *ptr2,
				    char **boundaries, int *boundaryct)
#else	/* !__STDC__ */
static char	*mime_decodeB64_bin(ptr1, ptr2, boundaries, boundaryct)
     char	*ptr1;
     char	*ptr2;
     char	**boundaries;
     int	*boundaryct;
#endif	/* !__STDC__ */
{
  return(mime_decodeB64_str(ptr1, ptr2, boundaries, boundaryct));
}
#endif	/* MNEWS */

/*
 * MIME BASE64 エンコード(文字列用)
 */

#ifdef	__STDC__
static char	*mime_encodeB64_str(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
static char	*mime_encodeB64_str(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  char	code1, code2, code3;

  while (*ptr1) {
    code1 = *ptr1++;
    if (*ptr1) {
      code2 = *ptr1++;
      if (*ptr1) {
	code3 = *ptr1++;
	ptr2 = mime_outputB64(code1, code2, code3, 0, ptr2);
      } else {
	ptr2 = mime_outputB64(code1, code2, 0, 1, ptr2);
	break;
      }
    } else {
      ptr2 = mime_outputB64(code1, 0, 0, 2, ptr2);
      break;
    }
  }
  *ptr2 = '\0';
  return(ptr2);
}

/*
 * MIME BASE64 デコード(文字列用)
 */

#ifdef	__STDC__
static char	*mime_decodeB64_str(char *ptr1, char *ptr2,
				    char **boundaries, int *boundaryct)
#else	/* !__STDC__ */
static char	*mime_decodeB64_str(ptr1, ptr2, boundaries, boundaryct)
     char	*ptr1;
     char	*ptr2;
     char	**boundaries;
     int	*boundaryct;
#endif	/* !__STDC__ */
{
  char	buff[BUFF_SIZE];
  char	*ptr3;
  int	code1, code2, code3, code4;
  int	newline = 1;
  int	finish = 0;

  while (*ptr1) {
    code1 = *ptr1++;
    if (isspace(code1)) {
      if (code1 == '\n') {
	newline = 1;
      } else {
	newline = 0;
      }
      continue;
    }
    if (newline && boundaries && (code1 == '-')) {
      ptr1--;
      ptr3 = buff;
      while (*ptr1) {
	*ptr3++ = *ptr1;
	if (*ptr1++ == '\n') {
	  break;
	}
      };
      *ptr3 = '\0';
      if (boundaries && (buff[0] == '-') && (buff[1] == '-')
	  && mime_boundaryB64(buff, boundaries, boundaryct)) {
	*ptr2 = '\0';
	return(ptr2);
      }
#ifdef	WINDOWS
      error_code = MIME_ERROR_CODE_2;
#else	/* !WINDOWS */
      print_warning("Ignoring unrecognized boundary line in MIME-B64.");
#endif	/* !WINDOWS */
      continue;
    }
    if (finish) {
      continue;
    }
    newline = 0;
    do {
      code2 = *ptr1++;
    } while (code2 && isspace(code2));
    if (code2) {
      do {
	code3 = *ptr1++;
      } while (code3 && isspace(code3));
      if (code3) {
	do {
	  code4 = *ptr1++;
	} while (code4 && isspace(code4));
	if (code4) {
	  if ((code1 == MIME_PAD_CHAR) || (code2 == MIME_PAD_CHAR)) {
	    finish = 1;
	    continue;
	  }
	  code1 = mime_charB64(code1);
	  code2 = mime_charB64(code2);
	  *ptr2++ = ((code1 << 2) | ((code2 & 0x30) >> 4));
	  if (code3 == MIME_PAD_CHAR) {
	    finish = 1;
	  } else {
	    code3 = mime_charB64(code3);
	    *ptr2++ = (((code2 & 0x0F) << 4) | ((code3 & 0x3C) >> 2));
	    if (code4 == MIME_PAD_CHAR) {
	      finish = 1;
	    } else {
	      code4 = mime_charB64(code4);
	      *ptr2++ = (((code3 & 0x03) << 6) | code4);
	    }
	  }
	  continue;
	}
      }
    }
#ifdef	WINDOWS
    error_code = MIME_ERROR_CODE_3;
#else	/* !WINDOWS */
    print_warning("Unexpected EOL in MIME-B64.");
#endif	/* !WINDOWS */
    break;
  }
  *ptr2 = '\0';
  return(ptr2);
}

#ifdef	__STDC__
static char	*mime_outputB64(char code1, char code2,char code3,
				int pad, char *ptr)
#else	/* !__STDC__ */
static char	*mime_outputB64(code1, code2, code3, pad, ptr)
     char	code1;
     char	code2;
     char	code3;
     int	pad;
     char	*ptr;
#endif	/* !__STDC__ */
{
  *ptr++ = mime_base64_basis[code1 >> 2];
  *ptr++ = mime_base64_basis[((code1 & 0x3) << 4) | ((code2 & 0xF0) >> 4)];
  switch (pad) {
  case 2:
    *ptr++ = MIME_PAD_CHAR;
    *ptr++ = MIME_PAD_CHAR;
    break;
  case 1:
    *ptr++ = mime_base64_basis[((code2 & 0xF) << 2) | ((code3 & 0xC0) >> 6)];
    *ptr++ = MIME_PAD_CHAR;
    break;
  case 0:
  default:
    *ptr++ = mime_base64_basis[((code2 & 0xF) << 2) | ((code3 & 0xC0) >> 6)];
    *ptr++ = mime_base64_basis[code3 & 0x3F];
    break;
  }
  return(ptr);
}

#ifdef	__STDC__
static int	mime_boundaryB64(char *ptr, char **boundaries, int *boundaryct)
#else	/* !__STDC__ */
static int	mime_boundaryB64(ptr, boundaries, boundaryct)
     char	*ptr;
     char	**boundaries;
     int	*boundaryct;
#endif	/* !__STDC__ */
{
  char		buff[2048];
  register int	i;

  for (i = 0; i < *boundaryct; ++i) {
    if (!strncmp(ptr, boundaries[i], strlen(boundaries[i]))) {
      strcpy(buff, boundaries[i]);
      strcat(buff, "--\n");
      if (!strcmp(buff, ptr)) {
	*boundaryct = i;
      }
      return(1);
    }
  }
  return(0);
}

/*
 * MIME メイン/サブタイプ取得
 */

#ifdef	__STDC__
char	*mime_get_type(char *ptr, MIME_MTYPE *mtype_ptr, MIME_STYPE *stype_ptr)
#else	/* !__STDC__ */
char	*mime_get_type(ptr, mtype_ptr, stype_ptr)
     char	*ptr;
     MIME_MTYPE	*mtype_ptr;
     MIME_STYPE	*stype_ptr;
#endif	/* !__STDC__ */
{
  register int	i;

  *mtype_ptr = M_MTYPE_UNKNOWN;
  *stype_ptr = M_STYPE_UNKNOWN;
  while ((*ptr == ' ') || (*ptr == '\t')) {
    ptr++;
  }
  for (i = 0; i < (sizeof(mime_mtype) / sizeof(char*)); i++) {
    if (!strncasecmp(ptr, mime_mtype[i], strlen(mime_mtype[i]))) {
      *mtype_ptr = i + 1;
      ptr += strlen(mime_mtype[i]);
      while ((*ptr == ' ') || (*ptr == '\t')) {
	ptr++;
      }
      if (*ptr == '/') {
	ptr++;
      }
      while ((*ptr == ' ') || (*ptr == '\t')) {
	ptr++;
      }
      for (i = 0; i < (sizeof(mime_stype) / sizeof(char*)); i++) {
	if (!strncasecmp(ptr, mime_stype[i], strlen(mime_stype[i]))) {
	  *stype_ptr = i + 1;
	  ptr += strlen(mime_stype[i]);
	  while ((*ptr == ' ') || (*ptr == '\t')) {
	    ptr++;
	  }
	}
      }
      return(ptr);
    }
  }
  return((char*)NULL);
}

/*
 * MIME パラメータ取得
 */

#ifdef	__STDC__
int	mime_get_param(char *ptr, char *parameter, char *value)
#else	/* !__STDC__ */
int	mime_get_param(ptr, parameter, value)
     char	*ptr;
     char	*parameter;
     char	*value;
#endif	/* !__STDC__ */
{
  while (*ptr) {
    if ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\n') || (*ptr == '\r') ||
	(*ptr == MIME_PARAM_SEPARATER)) {
      ptr++;
    } else {
      if (!strncasecmp(ptr, parameter, strlen(parameter))) {
	ptr += strlen(parameter);
	if (*ptr == MIME_PARAM_QUOTE) {
	  ptr++;
	  while (*ptr && (*ptr != MIME_PARAM_QUOTE)) {
	    *value++ = *ptr++;
	  }
	} else {
	  while ((*ptr > ' ') && (*ptr != MIME_PARAM_SEPARATER)) {
	    *value++ = *ptr++;
	  }
	}
	*value = '\0';
	return(0);
      } else {
	while (*ptr && (*ptr != MIME_PARAM_SEPARATER)) {
	  if (*ptr == MIME_PARAM_QUOTE) {
	    ptr++;
	    while (*ptr) {
	      if (*ptr++ == MIME_PARAM_QUOTE) {
		break;
	      }
	    }
	  } else {
	    ptr++;
	  }
	}
      }
    }
  }
  return(1);
}

/*
 * MIME 転送エンコードタイプ取得
 */

#ifdef	__STDC__
int	mime_get_ttype(char *ptr, MIME_TTYPE *ttype_ptr)
#else	/* !__STDC__ */
int	mime_get_ttype(ptr, ttype_ptr)
     char	*ptr;
     MIME_TTYPE	*ttype_ptr;
#endif	/* !__STDC__ */
{
  register int	i;

  while ((*ptr == ' ') || (*ptr == '\t')) {
    ptr++;
  }
  for (i = 0; i < (sizeof(mime_ttype) / sizeof(char*)); i++) {
    if (!strncasecmp(ptr, mime_ttype[i], strlen(mime_ttype[i]))) {
      *ttype_ptr = i + 1;
      return(0);
    }
  }
  return(1);
}

/*
 * 文字列比較+ポインタ移動(大文字小文字区別)
 */

#ifdef	__STDC__
static char	*strcheck(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
static char	*strcheck(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register int	n;

  n = strlen(ptr2);
  if (strncmp(ptr1, ptr2, n)) {
    return((char*)NULL);
  }
  return(ptr1 + n);
}

/*
 * 文字列比較+ポインタ移動(大文字小文字区別なし)
 */

#ifdef	__STDC__
static char	*strcasecheck(char *ptr1, char *ptr2)
#else	/* !__STDC__ */
static char	*strcasecheck(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
#endif	/* !__STDC__ */
{
  register int	n;

  n = strlen(ptr2);
  if (strncasecmp(ptr1, ptr2, n)) {
    return((char*)NULL);
  }
  return(ptr1 + n);
}

#if	!(defined(MNEWS) || defined(MMH))
/*
 * 文字列検索
 */

#ifdef	__STDC__
char	*strindex(char *str1, char *str2)
#else	/* !__STDC__ */
char	*strindex(str1 ,str2)
     char	*str1;
     char	*str2;
#endif	/* !__STDC__ */
{
  int	length;

  length = strlen(str2);
  while (*str1) {
    if (!strncasecmp(str1, str2, length)) {
      return(str1);
    }
    str1++;
  }
  return((char*)NULL);
}
#endif	/* !(MNEWS || MMH) */

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
    error_code = 0;
    break;
  }
  return(TRUE);
}

//========================================================
// mime_GetLastError
//--------------------------------------------------------
// エラーコード取得
//--------------------------------------------------------
// 戻り値 DWORD: エラーコード
//========================================================

DWORD MIMELIBFUNC mime_GetLastError(void)
{
  return(error_code);
}
#endif	/* WINDOWS */
#endif	/* MIME */
