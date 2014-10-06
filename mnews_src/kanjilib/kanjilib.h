/*
 *
 *  漢字処理ライブラリ
 *
 *  Copyright 1990-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Kanji Convert Library
 *  Sub system  : library define
 *  File        : kanjilib.h
 *  Version     : 2.46
 *  First Edit  : 1990-11/01
 *  Last  Edit  : 1997-11/24
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	__KANJILIB_H__
#define	__KANJILIB_H__

#ifdef	WINDOWS
#include	<windows.h>
#endif	/* WINDOWS */

/*
 * 漢字コード
 */

typedef enum {
  ASCII_CODE,
  JIS_CODE,
  SJIS_CODE,
  EUC_CODE,
  UNKNOWN_CODE,
  ERROR_CODE
} KANJICODE;

/*
 * 文字セット
 */

typedef enum {
  ROMAN_CHAR = 0,
  KANA_CHAR,
  KANJI_CHAR,
  HOJO_CHAR
} CHARSET;

#define	ROMAN_MASK	(1 << ROMAN_CHAR)
#define	KANA_MASK	(1 << KANA_CHAR)
#define	KANJI_MASK	(1 << KANJI_CHAR)
#define	HOJO_MASK	(1 << HOJO_CHAR)

#ifndef	BUFF_SIZE
#define	BUFF_SIZE	1024
#endif	/* BUFF_SIZE */

#define	ESC_CODE	'\033'

/*
 * JIS 漢字指示コード
 */

#define	JIS_IN1		"\033$@"	/* JIS X0208-1978	*/
#define	JIS_IN2		"\033$B"	/* JIS X0208-1983	*/
#define	JIS_IN3		"\033$(B"	/* JIS X0208-1983	*/
#define	JIS_X0201	"\033(I"	/* JIS X0201(カナ)	*/
#define	JIS_X0212	"\033$(D"	/* JIS X0212-1990(補助漢字) */
#define	JIS_IN		JIS_IN2

/*
 * JIS ローマ字指示コード
 */

#define	JIS_OUT1	"\033(J"	/* JIS X0201-1976 (left-hand part) */
#define	JIS_OUT2	"\033(B"	/* ASCII		*/
#define	JIS_OUT3	"\033(H"	/* 未使用		*/
#define	JIS_OUT		JIS_OUT2

/*
 * JIS-X0201 SHIFT-IN/OUT
 */

#define	X0201_IN_CODE	'\016'
#define	X0201_OUT_CODE	'\017'
#define	X0201_CODE_MASK	0x1000

/*
 * ESC 欠落 JIS 漢字指示/ローマ字指示コード
 */

#define	DJIS_IN1	"$@"		/* JIS X0208-1978	*/
#define	DJIS_IN2	"$B"		/* JIS X0208-1983	*/
#define	DJIS_IN3	"$(B"		/* JIS X0208-1983	*/
#define	DJIS_X0201	"(I"		/* JIS X0201(カナ)	*/
#define	DJIS_X0212	"$(D"		/* JIS X0212-1990(補助漢字) */
#define	DJIS_OUT1	"(J"		/* JIS X0201-1976 (left-hand part) */
#define	DJIS_OUT2	"(B"		/* ASCII		*/
#define	DJIS_OUT3	"(H"		/* 未使用		*/

/*
 * EUC コードセット
 */

#define	EUC_SS2		0x8e		/* JIS X0201(カナ)	*/
#define	EUC_SS3		0x8f		/* JIS X0212(補助漢字)	*/

#ifdef	WINDOWS
#ifdef	BUILD_KANJILIB_DLL
#define	KANJILIBFUNC __stdcall __export
#else	/* !BUILD_KANJILIB_DLL */
#define	KANJILIBFUNC __stdcall __import
#endif	/* !BUILD_KANJILIB_DLL */
#else	/* !WINDOWS */
#define	KANJILIBFUNC
#endif	/* !WINDOWS */

extern KANJICODE	print_code;		/* 表示漢字コード	*/
extern char		*kanji_code_string[];	/* 漢字コード名		*/
extern char		jis_in[];		/* 漢字指示コード	*/
extern char		jis_out[];		/* ローマ字指示コード	*/
extern int		jis_in_len;		/* 漢字指示コード長	*/
extern int		jis_out_len;		/* ローマ字指示コード長	*/
extern unsigned int	char_mask;		/* 文字種別		*/

#ifdef	__STDC__
extern void KANJILIBFUNC jis_mode(int);	/* 漢字ライブラリ初期化		*/
extern void KANJILIBFUNC jis_to_sjis(char*, char*);
					/* JIS→SJIS 変換		*/
extern void KANJILIBFUNC jis_to_euc(char*, char*);
					/* JIS→EUC 変換		*/
extern void KANJILIBFUNC euc_to_jis(char*, char*);
					/* EUC→JIS 変換		*/
extern void KANJILIBFUNC euc_to_sjis(char*, char*);
					/* EUC→SJIS 変換		*/
extern void KANJILIBFUNC sjis_to_jis(char*, char*);
					/* SJIS→JIS 変換		*/
extern void KANJILIBFUNC sjis_to_euc(char*, char*);
					/* SJIS→EUC 変換		*/
extern void KANJILIBFUNC jis_to_jis(char*, char*);
					/* JIS→JIS 変換(ESC統一)	*/
extern int KANJILIBFUNC jis_strncpy(char*, char*, int);
					/* JIS 漢字文字列コピー(指定幅) */
extern int KANJILIBFUNC euc_strncpy(char*, char*, int);
					/* EUC 漢字文字列コピー(指定幅) */
extern int KANJILIBFUNC sjis_strncpy(char*, char*, int);
					/* SJIS 漢字文字列コピー(指定幅)*/
extern int KANJILIBFUNC jis_strncpy2(char*, char*, int);
					/* JIS 漢字文字列コピー(指定幅) */
extern int KANJILIBFUNC euc_strncpy2(char*, char*, int);
					/* EUC 漢字文字列コピー(指定幅) */
extern int KANJILIBFUNC sjis_strncpy2(char*, char*, int);
					/* SJIS 漢字文字列コピー(指定幅)*/
extern int KANJILIBFUNC jis_tab_strncpy(char*, char*, int);
					/* JIS 漢字文字列コピー(指定幅) */
extern int KANJILIBFUNC euc_tab_strncpy(char*, char*, int);
					/* EUC 漢字文字列コピー(指定幅) */
extern int KANJILIBFUNC sjis_tab_strncpy(char*, char*, int);
					/* SJIS 漢字文字列コピー(指定幅) */
extern void	jis_printf(char*, ...);	/* JIS 漢字文字列表示		*/
extern void	euc_printf(char*, ...);	/* EUC 漢字文字列表示		*/
extern void	sjis_printf(char*, ...);/* SJIS 漢字文字列表示		*/
extern void	jis_fprintf(FILE*, char*, ...);
					/* JIS 漢字文字列出力		*/
extern void	euc_fprintf(FILE*, char*, ...);
					/* EUC 漢字文字列出力		*/
extern void	sjis_fprintf(FILE*, char*, ...);
					/* SJIS 漢字文字列出力		*/
extern KANJICODE KANJILIBFUNC discriminate_code(char*);
					/* 漢字コード判別		*/
extern KANJICODE KANJILIBFUNC discriminate_code2(char*);
					/* 漢字コード判別		*/
extern void	reset_char_mask(void);	/* 文字種別情報クリア		*/
extern KANJICODE KANJILIBFUNC to_sjis(char*, char*, KANJICODE);
					/* 自動判別→SJIS 文字列変換	*/
extern KANJICODE KANJILIBFUNC to_euc(char*, char*, KANJICODE);
					/* 自動判別→EUC 文字列変換	*/
extern KANJICODE KANJILIBFUNC to_jis(char*, char*, KANJICODE);
					/* 自動判別→JIS 文字列変換	*/
extern int KANJILIBFUNC kanji_strncpy(char*, char*, int);
					/* 漢字文字列コピー(指定幅)	*/
extern int KANJILIBFUNC kanji_strcpy2(char*, char*, int);
					/* 漢字文字列コピー(指定位置)	*/
extern int KANJILIBFUNC kanji_strncpy2(char*, char*, int);
					/* 漢字文字列コピー(指定幅PAD)	*/
extern int KANJILIBFUNC kanji_tab_strncpy(char*, char*, int);
					/* 漢字文字列コピー(指定幅,	*/
					/* TAB 展開付)			*/
extern int KANJILIBFUNC kanji_strlen(char*);
					/* 漢字文字列カウント		*/
extern void	kanji_printf(int, char*, ...);
					/* 漢字文字列表示		*/
extern void	kanji_fprintf(int, FILE*, char*, ...);
					/* 漢字文字列出力		*/
#ifdef	MNEWS
extern void		recover_jis(char*, char*);
					/* ESC 欠落 JIS 文字列復元	*/
#endif	/* MNEWS */
#else	/* !__STDC__ */
extern void	jis_mode();		/* 漢字ライブラリ初期化		*/
extern void	jis_to_sjis();		/* JIS→SJIS 変換		*/
extern void	jis_to_euc();		/* JIS→EUC 変換		*/
extern void	euc_to_jis();		/* EUC→JIS 変換		*/
extern void	euc_to_sjis();		/* EUC→SJIS 変換		*/
extern void	sjis_to_jis();		/* SJIS→JIS 変換		*/
extern void	sjis_to_euc();		/* SJIS→EUC 変換		*/
extern void	jis_to_jis();		/* JIS→JIS 変換(ESC統一)	*/
extern int	jis_strncpy();		/* JIS 漢字文字列コピー(指定幅) */
extern int	euc_strncpy();		/* EUC 漢字文字列コピー(指定幅) */
extern int	sjis_strncpy();		/* SJIS 漢字文字列コピー(指定幅)*/
extern int	jis_strncpy2();		/* JIS 漢字文字列コピー(指定幅) */
extern int	euc_strncpy2();		/* EUC 漢字文字列コピー(指定幅) */
extern int	sjis_strncpy2();	/* SJIS 漢字文字列コピー(指定幅)*/
extern int	jis_tab_strncpy();	/* JIS 文字列コピー(TAB展開)	*/
extern int	euc_tab_strncpy();	/* EUC 文字列コピー(TAB展開)	*/
extern int	sjis_tab_strncpy();	/* SJIS 文字列コピー(TAB展開)	*/
extern void	jis_printf();		/* JIS 漢字文字列表示		*/
extern void	euc_printf();		/* EUC 漢字文字列表示		*/
extern void	sjis_printf();		/* SJIS 漢字文字列表示		*/
extern void	jis_fprintf();		/* JIS 漢字文字列出力		*/
extern void	euc_fprintf();		/* EUC 漢字文字列出力		*/
extern void	sjis_fprintf();		/* SJIS 漢字文字列出力		*/
extern KANJICODE discriminate_code();	/* 漢字コード判別		*/
extern KANJICODE discriminate_code2();	/* 漢字コード判別		*/
extern void	reset_char_mask();	/* 文字種別情報クリア		*/
extern KANJICODE to_sjis();		/* 自動判別→SJIS 文字列変換	*/
extern KANJICODE to_euc();		/* 自動判別→EUC 文字列変換	*/
extern KANJICODE to_jis();		/* 自動判別→JIS 文字列変換	*/
extern int	kanji_strncpy();	/* 漢字文字列コピー(指定幅)	*/
extern int	kanji_strcpy2();	/* 漢字文字列コピー(指定位置)	*/
extern int	kanji_strncpy2();	/* 漢字文字列コピー(指定幅PAD)	*/
extern int	kanji_tab_strncpy();	/* 漢字文字列コピー(指定幅,	*/
					/* TAB 展開付)			*/
extern int	kanji_strlen();		/* 漢字文字列カウント		*/
extern void	kanji_printf();		/* 漢字文字列表示		*/
extern void	kanji_fprintf();	/* 漢字文字列出力		*/
#ifdef	MNEWS
extern void	recover_jis();		/* ESC 欠落 JIS 文字列復元	*/
#endif	/* MNEWS */
#endif	/* !__STDC__ */
#endif	/* !__KANJILIB_H__ */
