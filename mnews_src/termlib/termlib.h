/*
 *
 *  端末制御ライブラリ
 *
 *  Copyright 1990-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Terminal Control Library
 *  Sub system  : library define
 *  File        : termlib.h
 *  Version     : 1.67
 *  First Edit  : 1990-11/01
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	__TERMLIB_H__
#define	__TERMLIB_H__

/*
 * エスケープシーケンス
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
 * 中断キー
 */

#define	TERM_INTR_CHAR		'\3'	/* ^C	*/

/*
 * 属性コード
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

extern int	term_lines;			/* 端末行数		*/
extern int	term_columns;			/* 端末桁数		*/
extern int	term_resize_flag;		/* 端末再描画フラグ	*/
extern int	term_abort_flag;		/* 端末キー中断フラグ	*/

extern char	*tgetstr();
extern int	tgetnum();
extern int	tgetent();

#ifdef	__STDC__
extern int	term_init(int);		/* 端末初期化			*/
extern void	term_clear_all(void);	/* 画面クリア			*/
extern void	term_clear_line(void);	/* 行末まで削除			*/
extern void	term_clear_end(void);	/* 画面端まで削除		*/
extern void	term_bell(void);	/* 警告ベル			*/
extern void	term_locate(int, int);	/* カーソル移動			*/
extern void	term_home(void);	/* カーソルホーム移動		*/
extern void	term_insert(void);	/* 行挿入			*/
extern void	term_insert_line(int);	/* 複数行挿入			*/
extern void	term_insert_char(void);	/* 文字挿入			*/
extern void	term_up(void);		/* カーソル上移動		*/
extern void	term_down(void);	/* カーソル下移動		*/
extern void	term_left(void);	/* カーソル左移動		*/
extern void	term_right(void);	/* カーソル右移動		*/
extern void	term_top(void);		/* カーソル行頭移動		*/
extern void	term_crlf(int);		/* 改行				*/
extern void	term_fill_right(void);	/* 右端に文字出力		*/
extern void	term_attrib(int);	/* 表示属性変更			*/
extern void	term_set_raw(void);	/* RAW モードセット		*/
extern void	term_reset_raw(void);	/* RAW モードリセット		*/
extern void	term_set_cbreak(void);	/* CBREAK モードセット		*/
extern void	term_reset_cbreak(void);/* CBREAK モードリセット	*/
extern void	term_set_echo(void);	/* ECHO モードセット		*/
extern void	term_reset_echo(void);	/* ECHO モードリセット		*/
extern void	term_set_nl(void);	/* NL モードセット		*/
extern void	term_reset_nl(void);	/* NL モードリセット		*/
extern void	term_enable_mouse(void);/* マウス許可			*/
extern void	term_disable_mouse(void);
					/* マウス禁止			*/
extern int	term_system(char*);	/* 端末一時解放実行		*/
extern int	term_select(void);	/* 端末セレクト			*/
extern void	term_enable_abort(void);/* キーボード割り込み許可	*/
extern void	term_disable_abort(void);
					/* キーボード割り込み禁止	*/
#else	/* !__STDC__ */
extern int	term_init();		/* 端末初期化			*/
extern void	term_clear_all();	/* 画面クリア			*/
extern void	term_clear_line();	/* 行末まで削除			*/
extern void	term_clear_end();	/* 画面端まで削除		*/
extern void	term_bell();		/* 警告ベル			*/
extern void	term_locate();		/* カーソル移動			*/
extern void	term_home();		/* カーソルホーム移動		*/
extern void	term_insert();		/* 行挿入			*/
extern void	term_insert_line();	/* 複数行挿入			*/
extern void	term_insert_char();	/* 文字挿入			*/
extern void	term_up();		/* カーソル上移動		*/
extern void	term_down();		/* カーソル下移動		*/
extern void	term_left();		/* カーソル左移動		*/
extern void	term_right();		/* カーソル右移動		*/
extern void	term_top();		/* カーソル行頭移動		*/
extern void	term_crlf();		/* 改行				*/
extern void	term_fill_right();	/* 右端に文字出力		*/
extern void	term_attrib();		/* 表示属性変更			*/
extern void	term_set_raw();		/* RAW モードセット		*/
extern void	term_reset_raw();	/* RAW モードリセット		*/
extern void	term_set_cbreak();	/* CBREAK モードセット		*/
extern void	term_reset_cbreak();	/* CBREAK モードリセット	*/
extern void	term_set_echo();	/* ECHO モードセット		*/
extern void	term_reset_echo();	/* ECHO モードリセット		*/
extern void	term_set_nl();		/* NL モードセット		*/
extern void	term_reset_nl();	/* NL モードリセット		*/
extern void	term_enable_mouse();	/* マウス許可			*/
extern void	term_disable_mouse();	/* マウス禁止			*/
extern int	term_system();		/* 端末一時解放実行		*/
extern int	term_select();		/* 端末セレクト			*/
extern void	term_enable_abort();	/* キーボード割り込み許可	*/
extern void	term_disable_abort();	/* キーボード割り込み禁止	*/
#endif	/* !__STDC__ */
#endif	/* !__TERMLIB_H__ */
