/*
 *
 *  ü������饤�֥��
 *
 *  Copyright 1990-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Terminal Control Library
 *  Sub system  : library define
 *  File        : termlib.h
 *  Version     : 1.67
 *  First Edit  : 1990-11/01
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 ��� ��
 *
 */

#ifndef	__TERMLIB_H__
#define	__TERMLIB_H__

/*
 * ���������ץ�������
 */

#ifndef	TERMCAP
#define	CLEAR_ALL_STRING	"\033[H\033[2J"
#ifdef	OS2
#define	CLEAR_LINE_STRING	"\033[K"
#define	CLEAR_END_STRING	"\033[K"
#else	/* !OS2 */
#define	CLEAR_LINE_STRING	"\033[0K"
#define	CLEAR_END_STRING	"\033[0J"
#endif	/* !OS2 */
#define	LOCATE_STRING		"\033[%d;%dH"
#define	INS_LINE_STRING		"\033[L"
#define	INS_MULTILINE_STRING	"\033[%dL"
#define	INS_CHAR_STRING		"\033[@"
#define	UP_STRING		"\033[A"
#define	DOWN_STRING		"\033[B"
#define	LEFT_STRING		"\033[D"
#define	RIGHT_STRING		"\033[C"
#define	REVERSE_STRING		"\033[7m"
#define	NORMAL_STRING		"\033[m"
#define	ATTRIB_STRING		"\033[%dm"
#define	BELL_STRING		"\007"
#endif	/* !TERMCAP */

#define	TERM_BUFF	16

#ifdef	X68K
#define	DEFAULT_COLUMNS	96
#define	DEFAULT_LINES	31
#else	/* !X68K */
#define	DEFAULT_COLUMNS	80
#define	DEFAULT_LINES	24
#endif	/* !X68K */

/*
 * ���ǥ���
 */

#define	TERM_INTR_CHAR		'\3'	/* ^C	*/

/*
 * °��������
 */

#define	RESET_ATTRIB		0
#define	LIGHT_ATTRIB		1
#define	LINE_ATTRIB		4
#define	BLINK_ATTRIB		5
#define	REVERSE_ATTRIB		7
#define	HIDE_ATTRIB		8
#ifdef	X68K
#define	FG_BLACK1_ATTRIB	30
#define	FG_SKYBLUE1_ATTRIB	31
#define	FG_YELLOW1_ATTRIB	32
#define	FG_WHITE1_ATTRIB	33
#define	FG_BLACK2_ATTRIB	34
#define	FG_SKYBLUE2_ATTRIB	35
#define	FG_YELLOW2_ATTRIB	36
#define	FG_WHITE2_ATTRIB	37
#define	BG_BLACK1_ATTRIB	40
#define	BG_SKYBLUE1_ATTRIB	41
#define	BG_YELLOW1_ATTRIB	42
#define	BG_WHITE1_ATTRIB	43
#define	BG_BLACK2_ATTRIB	44
#define	BG_SKYBLUE2_ATTRIB	45
#define	BG_YELLOW2_ATTRIB	46
#define	BG_WHITE2_ATTRIB	47

#define	FG_BLACK_ATTRIB		30
#define	BG_BLACK_ATTRIB		40
#else	/* !X68K */
#define	FG_BLACK_ATTRIB		30
#define	FG_RED_ATTRIB		31
#define	FG_GREEN_ATTRIB		32
#define	FG_YELLOW_ATTRIB	33
#define	FG_BLUE_ATTRIB		34
#define	FG_PURPLE_ATTRIB	35
#define	FG_SKYBLUE_ATTRIB	36
#define	FG_WHITE_ATTRIB		37
#define	BG_BLACK_ATTRIB		40
#define	BG_RED_ATTRIB		41
#define	BG_GREEN_ATTRIB		42
#define	BG_YELLOW_ATTRIB	43
#define	BG_BLUE_ATTRIB		44
#define	BG_PURPLE_ATTRIB	45
#define	BG_SKYBLUE_ATTRIB	46
#define	BG_WHITE_ATTRIB		47
#endif	/* !X68K */

extern int	term_lines;			/* ü���Կ�		*/
extern int	term_columns;			/* ü�����		*/
extern int	term_resize_flag;		/* ü��������ե饰	*/
extern int	term_abort_flag;		/* ü���������ǥե饰	*/

extern char	*tgetstr();
extern int	tgetnum();
extern int	tgetent();

#ifdef	__STDC__
extern int	term_init(int);		/* ü�������			*/
extern void	term_clear_all(void);	/* ���̥��ꥢ			*/
extern void	term_clear_line(void);	/* �����ޤǺ��			*/
extern void	term_clear_end(void);	/* ����ü�ޤǺ��		*/
extern void	term_bell(void);	/* �ٹ�٥�			*/
extern void	term_locate(int, int);	/* ���������ư			*/
extern void	term_home(void);	/* ��������ۡ����ư		*/
extern void	term_insert(void);	/* ������			*/
extern void	term_insert_line(int);	/* ʣ��������			*/
extern void	term_insert_char(void);	/* ʸ������			*/
extern void	term_up(void);		/* ����������ư		*/
extern void	term_down(void);	/* �������벼��ư		*/
extern void	term_left(void);	/* �������뺸��ư		*/
extern void	term_right(void);	/* �������뱦��ư		*/
extern void	term_top(void);		/* ���������Ƭ��ư		*/
extern void	term_crlf(int);		/* ����				*/
extern void	term_fill_right(void);	/* ��ü��ʸ������		*/
extern void	term_attrib(int);	/* ɽ��°���ѹ�			*/
extern void	term_set_raw(void);	/* RAW �⡼�ɥ��å�		*/
extern void	term_reset_raw(void);	/* RAW �⡼�ɥꥻ�å�		*/
extern void	term_set_cbreak(void);	/* CBREAK �⡼�ɥ��å�		*/
extern void	term_reset_cbreak(void);/* CBREAK �⡼�ɥꥻ�å�	*/
extern void	term_set_echo(void);	/* ECHO �⡼�ɥ��å�		*/
extern void	term_reset_echo(void);	/* ECHO �⡼�ɥꥻ�å�		*/
extern void	term_set_nl(void);	/* NL �⡼�ɥ��å�		*/
extern void	term_reset_nl(void);	/* NL �⡼�ɥꥻ�å�		*/
extern void	term_enable_mouse(void);/* �ޥ�������			*/
extern void	term_disable_mouse(void);
					/* �ޥ����ػ�			*/
extern int	term_system(char*);	/* ü����������¹�		*/
extern int	term_select(void);	/* ü�����쥯��			*/
extern void	term_enable_abort(void);/* �����ܡ��ɳ����ߵ���	*/
extern void	term_disable_abort(void);
					/* �����ܡ��ɳ����߶ػ�	*/
#else	/* !__STDC__ */
extern int	term_init();		/* ü�������			*/
extern void	term_clear_all();	/* ���̥��ꥢ			*/
extern void	term_clear_line();	/* �����ޤǺ��			*/
extern void	term_clear_end();	/* ����ü�ޤǺ��		*/
extern void	term_bell();		/* �ٹ�٥�			*/
extern void	term_locate();		/* ���������ư			*/
extern void	term_home();		/* ��������ۡ����ư		*/
extern void	term_insert();		/* ������			*/
extern void	term_insert_line();	/* ʣ��������			*/
extern void	term_insert_char();	/* ʸ������			*/
extern void	term_up();		/* ����������ư		*/
extern void	term_down();		/* �������벼��ư		*/
extern void	term_left();		/* �������뺸��ư		*/
extern void	term_right();		/* �������뱦��ư		*/
extern void	term_top();		/* ���������Ƭ��ư		*/
extern void	term_crlf();		/* ����				*/
extern void	term_fill_right();	/* ��ü��ʸ������		*/
extern void	term_attrib();		/* ɽ��°���ѹ�			*/
extern void	term_set_raw();		/* RAW �⡼�ɥ��å�		*/
extern void	term_reset_raw();	/* RAW �⡼�ɥꥻ�å�		*/
extern void	term_set_cbreak();	/* CBREAK �⡼�ɥ��å�		*/
extern void	term_reset_cbreak();	/* CBREAK �⡼�ɥꥻ�å�	*/
extern void	term_set_echo();	/* ECHO �⡼�ɥ��å�		*/
extern void	term_reset_echo();	/* ECHO �⡼�ɥꥻ�å�		*/
extern void	term_set_nl();		/* NL �⡼�ɥ��å�		*/
extern void	term_reset_nl();	/* NL �⡼�ɥꥻ�å�		*/
extern void	term_enable_mouse();	/* �ޥ�������			*/
extern void	term_disable_mouse();	/* �ޥ����ػ�			*/
extern int	term_system();		/* ü����������¹�		*/
extern int	term_select();		/* ü�����쥯��			*/
extern void	term_enable_abort();	/* �����ܡ��ɳ����ߵ���	*/
extern void	term_disable_abort();	/* �����ܡ��ɳ����߶ػ�	*/
#endif	/* !__STDC__ */
#endif	/* !__TERMLIB_H__ */
