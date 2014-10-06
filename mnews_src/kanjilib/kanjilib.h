/*
 *
 *  ���������饤�֥��
 *
 *  Copyright 1990-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Kanji Convert Library
 *  Sub system  : library define
 *  File        : kanjilib.h
 *  Version     : 2.46
 *  First Edit  : 1990-11/01
 *  Last  Edit  : 1997-11/24
 *  Author      : MSR24 ��� ��
 *
 */

#ifndef	__KANJILIB_H__
#define	__KANJILIB_H__

#ifdef	WINDOWS
#include	<windows.h>
#endif	/* WINDOWS */

/*
 * ����������
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
 * ʸ�����å�
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
 * JIS �����ؼ�������
 */

#define	JIS_IN1		"\033$@"	/* JIS X0208-1978	*/
#define	JIS_IN2		"\033$B"	/* JIS X0208-1983	*/
#define	JIS_IN3		"\033$(B"	/* JIS X0208-1983	*/
#define	JIS_X0201	"\033(I"	/* JIS X0201(����)	*/
#define	JIS_X0212	"\033$(D"	/* JIS X0212-1990(�������) */
#define	JIS_IN		JIS_IN2

/*
 * JIS ���޻��ؼ�������
 */

#define	JIS_OUT1	"\033(J"	/* JIS X0201-1976 (left-hand part) */
#define	JIS_OUT2	"\033(B"	/* ASCII		*/
#define	JIS_OUT3	"\033(H"	/* ̤����		*/
#define	JIS_OUT		JIS_OUT2

/*
 * JIS-X0201 SHIFT-IN/OUT
 */

#define	X0201_IN_CODE	'\016'
#define	X0201_OUT_CODE	'\017'
#define	X0201_CODE_MASK	0x1000

/*
 * ESC ���� JIS �����ؼ�/���޻��ؼ�������
 */

#define	DJIS_IN1	"$@"		/* JIS X0208-1978	*/
#define	DJIS_IN2	"$B"		/* JIS X0208-1983	*/
#define	DJIS_IN3	"$(B"		/* JIS X0208-1983	*/
#define	DJIS_X0201	"(I"		/* JIS X0201(����)	*/
#define	DJIS_X0212	"$(D"		/* JIS X0212-1990(�������) */
#define	DJIS_OUT1	"(J"		/* JIS X0201-1976 (left-hand part) */
#define	DJIS_OUT2	"(B"		/* ASCII		*/
#define	DJIS_OUT3	"(H"		/* ̤����		*/

/*
 * EUC �����ɥ��å�
 */

#define	EUC_SS2		0x8e		/* JIS X0201(����)	*/
#define	EUC_SS3		0x8f		/* JIS X0212(�������)	*/

#ifdef	WINDOWS
#ifdef	BUILD_KANJILIB_DLL
#define	KANJILIBFUNC __stdcall __export
#else	/* !BUILD_KANJILIB_DLL */
#define	KANJILIBFUNC __stdcall __import
#endif	/* !BUILD_KANJILIB_DLL */
#else	/* !WINDOWS */
#define	KANJILIBFUNC
#endif	/* !WINDOWS */

extern KANJICODE	print_code;		/* ɽ������������	*/
extern char		*kanji_code_string[];	/* ����������̾		*/
extern char		jis_in[];		/* �����ؼ�������	*/
extern char		jis_out[];		/* ���޻��ؼ�������	*/
extern int		jis_in_len;		/* �����ؼ�������Ĺ	*/
extern int		jis_out_len;		/* ���޻��ؼ�������Ĺ	*/
extern unsigned int	char_mask;		/* ʸ������		*/

#ifdef	__STDC__
extern void KANJILIBFUNC jis_mode(int);	/* �����饤�֥������		*/
extern void KANJILIBFUNC jis_to_sjis(char*, char*);
					/* JIS��SJIS �Ѵ�		*/
extern void KANJILIBFUNC jis_to_euc(char*, char*);
					/* JIS��EUC �Ѵ�		*/
extern void KANJILIBFUNC euc_to_jis(char*, char*);
					/* EUC��JIS �Ѵ�		*/
extern void KANJILIBFUNC euc_to_sjis(char*, char*);
					/* EUC��SJIS �Ѵ�		*/
extern void KANJILIBFUNC sjis_to_jis(char*, char*);
					/* SJIS��JIS �Ѵ�		*/
extern void KANJILIBFUNC sjis_to_euc(char*, char*);
					/* SJIS��EUC �Ѵ�		*/
extern void KANJILIBFUNC jis_to_jis(char*, char*);
					/* JIS��JIS �Ѵ�(ESC����)	*/
extern int KANJILIBFUNC jis_strncpy(char*, char*, int);
					/* JIS ����ʸ���󥳥ԡ�(������) */
extern int KANJILIBFUNC euc_strncpy(char*, char*, int);
					/* EUC ����ʸ���󥳥ԡ�(������) */
extern int KANJILIBFUNC sjis_strncpy(char*, char*, int);
					/* SJIS ����ʸ���󥳥ԡ�(������)*/
extern int KANJILIBFUNC jis_strncpy2(char*, char*, int);
					/* JIS ����ʸ���󥳥ԡ�(������) */
extern int KANJILIBFUNC euc_strncpy2(char*, char*, int);
					/* EUC ����ʸ���󥳥ԡ�(������) */
extern int KANJILIBFUNC sjis_strncpy2(char*, char*, int);
					/* SJIS ����ʸ���󥳥ԡ�(������)*/
extern int KANJILIBFUNC jis_tab_strncpy(char*, char*, int);
					/* JIS ����ʸ���󥳥ԡ�(������) */
extern int KANJILIBFUNC euc_tab_strncpy(char*, char*, int);
					/* EUC ����ʸ���󥳥ԡ�(������) */
extern int KANJILIBFUNC sjis_tab_strncpy(char*, char*, int);
					/* SJIS ����ʸ���󥳥ԡ�(������) */
extern void	jis_printf(char*, ...);	/* JIS ����ʸ����ɽ��		*/
extern void	euc_printf(char*, ...);	/* EUC ����ʸ����ɽ��		*/
extern void	sjis_printf(char*, ...);/* SJIS ����ʸ����ɽ��		*/
extern void	jis_fprintf(FILE*, char*, ...);
					/* JIS ����ʸ�������		*/
extern void	euc_fprintf(FILE*, char*, ...);
					/* EUC ����ʸ�������		*/
extern void	sjis_fprintf(FILE*, char*, ...);
					/* SJIS ����ʸ�������		*/
extern KANJICODE KANJILIBFUNC discriminate_code(char*);
					/* ����������Ƚ��		*/
extern KANJICODE KANJILIBFUNC discriminate_code2(char*);
					/* ����������Ƚ��		*/
extern void	reset_char_mask(void);	/* ʸ�����̾��󥯥ꥢ		*/
extern KANJICODE KANJILIBFUNC to_sjis(char*, char*, KANJICODE);
					/* ��ưȽ�̢�SJIS ʸ�����Ѵ�	*/
extern KANJICODE KANJILIBFUNC to_euc(char*, char*, KANJICODE);
					/* ��ưȽ�̢�EUC ʸ�����Ѵ�	*/
extern KANJICODE KANJILIBFUNC to_jis(char*, char*, KANJICODE);
					/* ��ưȽ�̢�JIS ʸ�����Ѵ�	*/
extern int KANJILIBFUNC kanji_strncpy(char*, char*, int);
					/* ����ʸ���󥳥ԡ�(������)	*/
extern int KANJILIBFUNC kanji_strcpy2(char*, char*, int);
					/* ����ʸ���󥳥ԡ�(�������)	*/
extern int KANJILIBFUNC kanji_strncpy2(char*, char*, int);
					/* ����ʸ���󥳥ԡ�(������PAD)	*/
extern int KANJILIBFUNC kanji_tab_strncpy(char*, char*, int);
					/* ����ʸ���󥳥ԡ�(������,	*/
					/* TAB Ÿ����)			*/
extern int KANJILIBFUNC kanji_strlen(char*);
					/* ����ʸ���󥫥����		*/
extern void	kanji_printf(int, char*, ...);
					/* ����ʸ����ɽ��		*/
extern void	kanji_fprintf(int, FILE*, char*, ...);
					/* ����ʸ�������		*/
#ifdef	MNEWS
extern void		recover_jis(char*, char*);
					/* ESC ���� JIS ʸ��������	*/
#endif	/* MNEWS */
#else	/* !__STDC__ */
extern void	jis_mode();		/* �����饤�֥������		*/
extern void	jis_to_sjis();		/* JIS��SJIS �Ѵ�		*/
extern void	jis_to_euc();		/* JIS��EUC �Ѵ�		*/
extern void	euc_to_jis();		/* EUC��JIS �Ѵ�		*/
extern void	euc_to_sjis();		/* EUC��SJIS �Ѵ�		*/
extern void	sjis_to_jis();		/* SJIS��JIS �Ѵ�		*/
extern void	sjis_to_euc();		/* SJIS��EUC �Ѵ�		*/
extern void	jis_to_jis();		/* JIS��JIS �Ѵ�(ESC����)	*/
extern int	jis_strncpy();		/* JIS ����ʸ���󥳥ԡ�(������) */
extern int	euc_strncpy();		/* EUC ����ʸ���󥳥ԡ�(������) */
extern int	sjis_strncpy();		/* SJIS ����ʸ���󥳥ԡ�(������)*/
extern int	jis_strncpy2();		/* JIS ����ʸ���󥳥ԡ�(������) */
extern int	euc_strncpy2();		/* EUC ����ʸ���󥳥ԡ�(������) */
extern int	sjis_strncpy2();	/* SJIS ����ʸ���󥳥ԡ�(������)*/
extern int	jis_tab_strncpy();	/* JIS ʸ���󥳥ԡ�(TABŸ��)	*/
extern int	euc_tab_strncpy();	/* EUC ʸ���󥳥ԡ�(TABŸ��)	*/
extern int	sjis_tab_strncpy();	/* SJIS ʸ���󥳥ԡ�(TABŸ��)	*/
extern void	jis_printf();		/* JIS ����ʸ����ɽ��		*/
extern void	euc_printf();		/* EUC ����ʸ����ɽ��		*/
extern void	sjis_printf();		/* SJIS ����ʸ����ɽ��		*/
extern void	jis_fprintf();		/* JIS ����ʸ�������		*/
extern void	euc_fprintf();		/* EUC ����ʸ�������		*/
extern void	sjis_fprintf();		/* SJIS ����ʸ�������		*/
extern KANJICODE discriminate_code();	/* ����������Ƚ��		*/
extern KANJICODE discriminate_code2();	/* ����������Ƚ��		*/
extern void	reset_char_mask();	/* ʸ�����̾��󥯥ꥢ		*/
extern KANJICODE to_sjis();		/* ��ưȽ�̢�SJIS ʸ�����Ѵ�	*/
extern KANJICODE to_euc();		/* ��ưȽ�̢�EUC ʸ�����Ѵ�	*/
extern KANJICODE to_jis();		/* ��ưȽ�̢�JIS ʸ�����Ѵ�	*/
extern int	kanji_strncpy();	/* ����ʸ���󥳥ԡ�(������)	*/
extern int	kanji_strcpy2();	/* ����ʸ���󥳥ԡ�(�������)	*/
extern int	kanji_strncpy2();	/* ����ʸ���󥳥ԡ�(������PAD)	*/
extern int	kanji_tab_strncpy();	/* ����ʸ���󥳥ԡ�(������,	*/
					/* TAB Ÿ����)			*/
extern int	kanji_strlen();		/* ����ʸ���󥫥����		*/
extern void	kanji_printf();		/* ����ʸ����ɽ��		*/
extern void	kanji_fprintf();	/* ����ʸ�������		*/
#ifdef	MNEWS
extern void	recover_jis();		/* ESC ���� JIS ʸ��������	*/
#endif	/* MNEWS */
#endif	/* !__STDC__ */
#endif	/* !__KANJILIB_H__ */
