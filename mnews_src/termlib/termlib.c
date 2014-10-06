/*
 *
 *  端末制御ライブラリ
 *
 *  Copyright 1990-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Terminal Control Library
 *  Sub system  : library routine
 *  File        : termlib.c
 *  Version     : 1.67
 *  First Edit  : 1990-11/01
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 宅間 顯
 *
 */

#include	"compat.h"
#include	"termlib.h"
#ifdef	MNEWS
#include	"kanjilib.h"
#endif	/* MNEWS */

#if	defined(USE_TERMIOS) && !defined(TERMIO)
#define	TERMIO
#endif	/* USE_TERMIOS && !TERMIO */

#ifndef	sun
#include <sys/ioctl.h>	/* for TIOCGWINSZ */
#endif	/* !sun */
#ifndef	MSDOS
#ifdef	TERMIO
#ifdef	USE_TERMIOS
#include	<termios.h>
#else	/* !USE_TERMIOS */
#include	<termio.h>
#endif	/* !USE_TERMIOS */
#ifndef	VDISABLE
#ifdef	_POSIX_VDISABLE
#define	VDISABLE	_POSIX_VDISABLE
#else	/* !_POSIX_VDISABLE */
#define	VDISABLE	0377
#endif	/* !_POSIX_VDISABLE */
#endif	/* !VDISABLE */
#ifdef	TERM_HPUX
/*
 * HP-UX is weird -- must use old interface for some control chars
 */
#include <bsdtty.h>
#endif	/* TERM_HPUX */
#else	/* !TERMIO */
#ifdef	TERMINFO
#include	<curses.h>
#include	<term.h>
#else	/* !TERMINFO */
#include	<sgtty.h>
#endif	/* !TERMINFO */
#endif	/* !TERMIO */
#include	<sys/wait.h>
#endif	/* !MSDOS */
#ifdef	OS2
#include	<sys/video.h>
#endif	/* OS2 */

#ifdef	__STDC__
static void	window_resize(void);	/* ウィンドウリサイズ	*/
static void	kbd_abort(void);	/* キーボード割り込み	*/
#else	/* !__STDC__ */
static void	window_resize();	/* ウィンドウリサイズ	*/
static void	kbd_abort();		/* キーボード割り込み	*/
#endif	/* !__STDC__ */

#ifdef	TERMCAP
static char	*term_cl;	/* Clear screen			*/
static char	*term_cd;	/* Clear to end of display	*/
static char	*term_ce;	/* Clear to end of line		*/
static char	*term_ho;	/* Home cursor			*/
static char	*term_cm;	/* Cursor motion		*/
static char	*term_so;	/* Stand out mode start		*/
static char	*term_se;	/* Stand out mode end		*/
static char	*term_us;	/* Underscore mode start	*/
static char	*term_ue;	/* Underscore mode end		*/
static char	*term_mr;	/* Reverse mode			*/
static char	*term_mh;	/* Half bright mode  (dim)	*/
static char	*term_md;	/* Extra bright mode (bold)	*/
static char	*term_mb;	/* Blinking mode		*/
static char	*term_me;	/* Turn off all attributes	*/
static char	*term_bl;	/* Audible bell			*/
static char	*term_cr;	/* Carriage return		*/
static char	*term_do;	/* Down one line		*/
static char	*term_u;	/* Upline			*/
static char	*term_le;	/* Move cursor left		*/
static char	*term_nd;	/* Non-destructive space (right)*/
static char	*term_ic;	/* Insert character		*/
static char	*term_al;	/* Add blank line(scroll down)	*/
static char	*term_ti;	/* String to begin programs	*/
				/* that use termcap		*/
static char	*term_te;	/* String to end programs that	*/
				/* use termcap			*/
#ifdef	COLOR
static char	*term_af;	/* Set foreground color		*/
static char	*term_ab;	/* Set background color		*/
#endif	/* COLOR */
static char	*term_TY;	/* Terminal Kanji code		*/
int		term_xn_nam;	/* Don't wrap at end of line	*/

#undef	putchar
int	putchar();		/* putchar must be a function (not macro!) */

/*
 * termcap グローバル変数
 */

#ifndef	SYSV_CURSES
extern char	*UP;		/* upline		*/
extern char	*BC;		/* backspace character	*/
extern char	PC;		/* pad character	*/
extern short	ospeed;		/* terminal output speed*/
#endif	/* !SYSV_CURSES */
#endif	/* TERMCAP */

int		term_columns = DEFAULT_COLUMNS;	/* 端末行数		*/
int		term_lines = DEFAULT_LINES;	/* 端末桁数		*/
int		term_resize_flag = 0;		/* 端末再描画フラグ	*/
int		term_abort_flag = 0;		/* 端末キー中断フラグ	*/

#ifdef	TERMIO
#ifdef	USE_TERMIOS
static struct termios	tty_new;		/* 端末情報(変更後)	*/
static struct termios	tty_org;		/* 端末情報(変更前)	*/
#else	/* !USE_TERMIOS */
static struct termio	tty_new;		/* 端末情報(変更後)	*/
static struct termio	tty_org;		/* 端末情報(変更前)	*/
#endif	/* !USE_TERMIOS */
#ifdef	TERM_HPUX
static struct ltchars	ltchars_new, ltchars_org;
#endif	/* TERM_HPUX */
#else	/* !TERMIO */
static struct sgttyb	tty_new;		/* 端末情報(変更後)	*/
static struct sgttyb	tty_org;		/* 端末情報(変更前)	*/
static struct tchars	tchars_new, tchars_org;
static struct ltchars	ltchars_new, ltchars_org;
#ifdef	TIOCLGET
static	int		lmode_new, lmode_org;
#endif	/* TIOCLGET */
#endif	/* !TERMIO */
#ifdef	TERMCAP
static unsigned char	tcbuf[1024];		/* 1024 でなければならない */
#endif	/* TERMCAP */
#ifdef	notdef
static int		term_nl = 1;		/* 改行モード		*/
static int		term_echo = 1;		/* エコーモード		*/
static int		term_cbreak = 0;	/* CBREAK モード	*/
static int		term_raw = 0;		/* RAW モード		*/
static int		ldisc_mode;		/* 端末モード		*/
static int		ldisc_org;		/* 端末モード		*/
#endif	/* notdef */
static int		init_flag = 0;		/* 初期化フラグ		*/

/*
 * 端末初期化
 */

#ifdef	__STDC__
int	term_init(int mode)
#else	/* !__STDC__ */
int	term_init(mode)
     int	mode;
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  static char	cap[256];
  char		*cp = cap;
  char		*term_name;
#endif	/* TERMCAP */

  switch (mode) {
  case 0:
    term_columns = DEFAULT_COLUMNS;
    term_lines   = DEFAULT_LINES;
    term_resize_flag = 0;
    term_abort_flag = 0;
#ifdef	SIGWINCH
    signal(SIGWINCH, window_resize);
#endif	/* SIGWINCH */
#ifdef	TERMCAP
    if ((term_name = (char*)getenv("TERM")) == NULL) {
      fprintf(stderr, "Environment variable TERM not defined.\n");
      return(1);
    }
#ifdef	TERMINFO
#ifdef	notdef
    if (setupterm(term_name, 0, &terr) == ERR) {
#else	/* !notdef */
    if (setupterm(term_name, 1, (int*)NULL) == ERR) {
#endif	/* !notdef */
      fprintf(stderr, "Unknown terminal type %s\n", term_name);
      return(1);
    }
#else	/* !TERMINFO */
    if (tgetent(tcbuf, term_name) < 1) {
      fprintf(stderr, "Unknown terminal type %s\n", term_name);
      return(1);
    }
#endif	/* !TERMINFO */
    window_resize();
#ifdef	TERMINFO
    term_cl = clear_screen;
    term_cd = clr_eos;
    term_ce = clr_eol;
    term_ho = cursor_home;
    term_cm = cursor_address;
    term_so = enter_standout_mode;
    term_se = exit_standout_mode;
    term_us = enter_underline_mode;
    term_ue = exit_underline_mode;
    term_mr = enter_reverse_mode;
    term_mh = enter_dim_mode;
    term_md = enter_bold_mode;
    term_mb = enter_blink_mode;
    term_me = exit_attribute_mode;
    term_bl = bell;
    term_cr = carriage_return;
    term_do = cursor_down;
    term_u  = cursor_up;
    term_le = cursor_left;
    term_nd = cursor_right;
    term_ic = insert_character;
    term_al = insert_line;
#ifdef	COLOR
    term_af = set_a_foreground;
    term_ab = set_a_background;
#endif	/* COLOR */
    term_ti = (char*)NULL;
    term_te = (char*)NULL;
    term_TY = (char*)NULL;
    term_xn_nam = !auto_right_margin || eat_newline_glitch;
#else	/* ! TERMINFO */
    term_cl = tgetstr("cl", &cp);
    term_cd = tgetstr("cd", &cp);
    term_ce = tgetstr("ce", &cp);
    term_ho = tgetstr("ho", &cp);
    term_cm = tgetstr("cm", &cp);
    term_so = tgetstr("so", &cp);
    term_se = tgetstr("se", &cp);
    term_us = tgetstr("us", &cp);
    term_ue = tgetstr("ue", &cp);
    term_mr = tgetstr("mr", &cp);
    term_mh = tgetstr("mh", &cp);
    term_md = tgetstr("md", &cp);
    term_mb = tgetstr("mb", &cp);
    term_me = tgetstr("me", &cp);
    term_bl = tgetstr("bl", &cp);
    term_cr = tgetstr("cr", &cp);
    term_do = tgetstr("do", &cp);
    term_u  = tgetstr("up", &cp);
    term_le = tgetstr("le", &cp);
    term_nd = tgetstr("nd", &cp);
    term_ic = tgetstr("ic", &cp);
    term_al = tgetstr("al", &cp);
    term_TY = tgetstr("TY", &cp);
    term_ti = tgetstr("ti", &cp);
    term_te = tgetstr("te", &cp);
#ifdef	COLOR
    term_af = tgetstr("AF", &cp);
    term_ab = tgetstr("AB", &cp);
#endif	/* COLOR */
    term_xn_nam = !tgetflag("am") || tgetflag("xn");
    if (!term_al) {
      term_al = tgetstr("sr", &cp);
    }
#endif	/* ! TERMINFO */

#ifndef	SYSV_CURSES
    /*
     * Is there any termcap library which is missing in
     * these variables?
     */
#ifdef	TERMINFO
    {
      char *p;
      if (p = pad_char) {
	PC = p[0];			/* Padding character */
      } else {
	PC = '\0';
      }
    }
    BC = key_backspace;			/* Backspace string */
#else	/* ! TERMINFO */
    {
      char	*p;

      p = tgetstr("pc", &cp);
      if (p) {
	PC = p[0];			/* Padding character */
      } else {
	PC = '\0';
      }
    }
    BC = tgetstr("bc", &cp);		/* Backspace string */
#endif	/* ! TERMINFO */
    UP = term_u;			/* Upline string */
#endif	/* !SYSV_CURSES */

    if (!(term_cl && term_cm) || !term_al) {
      fprintf(stderr, "This terminal seems not so capable as to run mnews.\n");
      return(1);
    }
    if (!term_le) {			/* ^H */
      term_le = "\010";
    }
    if (!term_u) {			/* ^K */
      term_u  = "\013";
    }
    if (!term_nd) {			/* ^L */
      term_nd = "\014";
    }
    if (!term_cr) {			/* ^M */
      term_cr = "\015";
    }
    if (!term_so || !term_se) {
      term_so = term_se = "";
    }

    /*
     * term_do (down 1 line) は最下行でスクロールする必要がある。
     * しかしスクロールしないものがあるので ^J を使用する。
     */

    if ((!term_do) || (term_do[strlen(term_do) - 1] != '\012')) {
					/* ^J */
      term_do = "\012";
    }

#ifdef	MNEWS
    /*
     * "TY" エントリを参照してデフォルトの表示コードを決定する。
     * 例.	TY=euc or TY=ujis	->  EUC code
     *		TY=jis			->  JIS code
     *		TY=sjis			->  SJIS code
     */

    if (term_TY) {
      if (!strcmp(term_TY, "euc") || !strcmp(term_TY, "ujis")) {
	print_code = EUC_CODE;
      } else if (!strcmp(term_TY, "jis")) {
	print_code = JIS_CODE;
      } else if (!strcmp(term_TY, "sjis")) {
	print_code = SJIS_CODE;
      }
    }
#endif	/* MNEWS */
#else	/* !TERMCAP */
    window_resize();
#endif	/* !TERMCAP */
    if (!init_flag) {
#ifdef	TERMIO
#ifdef	USE_TERMIOS
      tcgetattr(0, &tty_org);
#else	/* !USE_TERMIOS */
      ioctl(0, TCGETA, &tty_org);
#endif	/* !USE_TERMIOS */
      tty_new = tty_org;

      /*
       * set new mode
       */
      /* output flags */
      tty_new.c_oflag &= ~(0
#ifdef	ONLCR
			   | ONLCR
#endif	/* ONLCR */
#ifdef	OPOST
			   | OPOST
#else	/* !OPOST */
			   | OLCUC | OCRNL | ONLRET
#endif	/* !OPOST */
#ifdef	OXTABS
			   | OXTABS
#endif	/* OXTABS */
#ifdef	XTABS
			   | XTABS
#endif	/* XTABS */
#ifdef	TAB3
			   | TAB3
#endif	/* TAB3 */
			   );

      /* local flags */
      tty_new.c_lflag |= ISIG;	/* enable terminal signal */
      tty_new.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL
#ifdef	XCASE
			   | XCASE
#endif	/* XCASE */
#ifdef	ECHOCTL
			   | ECHOCTL
#endif	/* ECHOCTL */
#ifdef	ECHOPRT
			   | ECHOPRT
#endif	/* ECHOPRT */
#ifdef	ECHOKE
			   | ECHOKE
#endif	/* ECHOKE */
			   );

      /* input flags */
      tty_new.c_iflag &= ~(ICRNL | IGNCR | INLCR
#ifdef	ISTRIP
			   | ISTRIP
#endif	/* ISTRIP */
			   );

      tty_new.c_cc[VMIN] = 1;	/* read(2) returns with one char */
      tty_new.c_cc[VTIME] = 0;	/* no timeout */

#ifdef	TERM_INTR_CHAR
      tty_new.c_cc[VINTR] = TERM_INTR_CHAR;
#endif	/* TERM_INTR_CHAR */
      tty_new.c_cc[VQUIT] = VDISABLE;
#ifdef	USE_TERMIOS
#ifdef	VDISCARD
      tty_new.c_cc[VDISCARD] = VDISABLE;
#endif	/* VDISCARD */
#ifdef	VFLUSHO
      tty_new.c_cc[VFLUSHO] = VDISABLE;	/* IRIX 4 */
#endif	/* VFLUSHO */
#ifdef	V_FLUSH
      tty_new.c_cc[V_FLUSH] = VDISABLE;	/* MIPS RISC/os4 */
#endif	/* V_FLUSH */
#ifdef	VDISCRD
      tty_new.c_cc[VDISCRD] = VDISABLE;	/* AIX 3 */
#endif	/* VDISCRD */
#ifdef	VLNEXT
      tty_new.c_cc[VLNEXT] = VDISABLE;
#endif	/* VLNEXT */
#ifdef	V_LNEXT
      tty_new.c_cc[V_LNEXT] = VDISABLE;	/* MIPS RISC/os4 */
#endif	/* V_LNEXT */
#ifdef	VSTATUS
      tty_new.c_cc[VSTATUS] = VDISABLE;
#endif	/* VSTATUS */
#ifdef	VSUSP
      tty_new.c_cc[VSUSP] = VDISABLE;
#endif	/* VSUSP */
#ifdef	VDSUSP
      tty_new.c_cc[VDSUSP] = VDISABLE;
#endif	/* VDSUSP */
#ifdef	V_DSUSP
      tty_new.c_cc[V_DSUSP] = VDISABLE;	/* MIPS RISC/os4 */
#endif	/* V_DSUSP */
#endif	/* USE_TERMIOS */
#ifdef	TERM_HPUX
      /* wired but required... */
      ioctl(1, TIOCGLTC, &ltchars_org);
      ltchars_new = ltchars_org;
      ltchars_new.t_suspc = -1;
      ltchars_new.t_dsuspc = -1;
      ltchars_new.t_flushc = -1;
      ltchars_new.t_lnextc = -1;
#endif	/* TERM_HPUX */

#else	/* !TERMIO */

      /*
       * for 4.2BSD
       */

      ioctl(0, TIOCGETP, &tty_org);
      tty_new = tty_org;
      tty_new.sg_flags &= ~ (RAW | ECHO | XTABS | CRMOD);
      tty_new.sg_flags |= (CBREAK
#ifdef	PASS8
			   | PASS8	/* this may not work (use LPASS8) */
#endif	/* PASS8 */
			   );

      ioctl(0, TIOCGETC, (char*)&tchars_org);
      tchars_new = tchars_org;
#ifdef	TERM_INTR_CHAR
      tchars_new.t_intrc = TERM_INTR_CHAR;
#endif	/* TERM_INTR_CHAR */
      tchars_new.t_quitc = -1;

      ioctl(0, TIOCGLTC, (char*)&ltchars_org);
      ltchars_new = ltchars_org;
      ltchars_new.t_flushc = -1;
      ltchars_new.t_lnextc = -1;
      ltchars_new.t_suspc  = -1;
      ltchars_new.t_dsuspc = -1;

#ifdef	TIOCLGET
      ioctl(0, TIOCLGET, (char*)&lmode_org);
      lmode_new = lmode_org;
#ifdef	LPASS8
      lmode_new |= LPASS8;
#endif	/* LPASS8 */
#ifdef	LPASS8OUT
      lmode_new |= LPASS8OUT;
#endif	/* LPASS8OUT */
#endif	/* TIOCLGET */
#endif	/* !TERMIO */

      /*
       * This is for termcap library
       */
#ifdef	TERMCAP
#ifndef	SYSV_CURSES
#ifdef	TERMIO
#ifdef	USE_TERMIOS
      {
	int	ospd = cfgetospeed(&tty_org);
#if	B9600 == 9600
	/*
	 * new tty speed interface -- convert to termcap's
	 */
	static const unsigned short	spd_cnv[] = {
	  0, 50, 75, 110, 134, 150, 200, 300, 600, 1200,
	  1800, 2400, 4800, 9600, 19200, 38400
	  };
	short	s;

	for (s = 0; s < sizeof spd_cnv / sizeof(unsigned short) - 1; s++) {
	  if (ospd <= spd_cnv[s]) {
	    break;
	  }
	}
	ospeed = s;
#else	/* B9600 != 9600 */
	ospeed = ospd;	/* no conversion required */
#endif	/* B9600 != 9600 */
      }
#else	/* !USE_TERMIOS */
      ospeed = tty_org.c_cflag & CBAUD;
#endif	/* !USE_TERMIOS */
#else	/* !TERMIO */
#ifdef	notdef
      ospeed = tty_org.sg_ospeed;
#endif	/* notdef */
#endif	/* !TERMIO */
#endif	/* !SYSV_CURSES */

      if (term_ti) {
	tputs(term_ti, 1, putchar);
      }
#endif	/* TERMCAP */
      init_flag = 1;
    }
    /* break は不要 */
  case 2:
#ifdef	TERMIO
#ifdef	USE_TERMIOS
    tcsetattr(0, TCSAFLUSH, &tty_new);
#else	/* !USE_TERMIOS */
    ioctl(0, TCSETA, &tty_new);
#endif	/* !USE_TERMIOS */
#ifdef	TERM_HPUX
    ioctl(0, TIOCSLTC, (char*)&ltchars_new);
#endif	/* TERM_HPUX */
#else	/* !TERMIO */
    ioctl(0, TIOCSETP, &tty_new);
    ioctl(0, TIOCSETC, (char*)&tchars_new);
    ioctl(0, TIOCSLTC, (char*)&ltchars_new);
#ifdef	TIOCLGET
    ioctl(0, TIOCLSET, (char*)&lmode_new);
#endif	/* TIOCLGET */
#if	notdef
    ioctl(0, TIOCSETD, &ldisc_new);
#endif	/* notdef */
#endif	/* !TERMIO */
    break;
  default:
    break;
  case 3:
#if	notdef
    term_reset_raw();
    term_reset_cbreak();
    term_set_echo();
    term_set_nl();
#endif	/* notdef */
    /* break は不要 */
  case 1:
    if (init_flag) {
#ifdef	TERMIO
#ifdef	USE_TERMIOS
      tcsetattr(0, TCSAFLUSH, &tty_org);
#else	/* !USE_TERMIOS */
      ioctl(0, TCSETA, &tty_org);
#endif	/* !USE_TERMIOS */
#ifdef	TERM_HPUX
      ioctl(0, TIOCSLTC, (char*)&ltchars_org);
#endif	/* TERM_HPUX */
#else	/* !TERMIO */
      ioctl(0, TIOCSETP, &tty_org);
      ioctl(0, TIOCSETC, (char*)&tchars_org);
      ioctl(0, TIOCSLTC, (char*)&ltchars_org);
#ifdef	TIOCLGET
      ioctl(0, TIOCLSET, (char*)&lmode_org);
#endif	/* TIOCLGET */
#if	notdef
      ioctl(0, TIOCSETD, &ldisc_org);
#endif	/* notdef */
#endif	/* !TERMIO */
#ifdef	TERMCAP
      if (term_te) {
	tputs(term_te, 1, putchar);
      }
#endif	/* TERMCAP */
    }
    break;
  }
  return(0);
}

/*
 * 画面クリア
 */

#ifdef	__STDC__
void	term_clear_all(void)
#else	/* !__STDC__ */
void	term_clear_all()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_cl) {
    tputs(term_cl, term_lines, putchar);
  }
#else	/* !TERMCAP */
  printf(CLEAR_ALL_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * 行末まで削除
 */

#ifdef	__STDC__
void	term_clear_line(void)
#else	/* !__STDC__ */
void	term_clear_line()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  register int	i;
  if (term_ce) {
    tputs(term_ce, 1, putchar);
  } else {
    term_top();
    for (i = term_columns - 1; i > 0; --i) {
      putc(' ', stdout);
    }
  }
#else	/* !TERMCAP */
  printf(CLEAR_LINE_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * 画面端まで削除
 */

#ifdef	__STDC__
void	term_clear_end(void)
#else	/* !__STDC__ */
void	term_clear_end()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_cd) {
    tputs(term_cd, 1, putchar);
  }
#else	/* !TERMCAP */
  printf(CLEAR_END_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * 警告ベル
 */

#ifdef	__STDC__
void	term_bell(void)
#else	/* !__STDC__ */
void	term_bell()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_bl) {
    tputs(term_bl, 1, putchar);
  }
#else	/* !TERMCAP */
  printf(BELL_STRING);
  fflush(stdout);
#endif	/* !TERMCAP */
}

/*
 * カーソル移動
 */

#ifdef	__STDC__
void	term_locate(int x, int y)
#else	/* !__STDC__ */
void	term_locate(x, y)
     int	x;
     int	y;
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_cm) {
#ifdef	TERMINFO
    tputs(tparm(term_cm, y, x), 1, putchar);
#else	/* !TERMINFO */
    tputs(tgoto(term_cm, x, y), 1, putchar);
#endif	/* !TERMINFO */
  }
#else	/* !TERMCAP */
  printf(LOCATE_STRING, y + 1, x + 1);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * カーソルホーム位置移動
 */

#ifdef	__STDC__
void	term_home(void)
#else	/* !__STDC__ */
void	term_home()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_ho) {
    tputs(term_ho, 1, putchar);
  } else {
    term_locate(0, 0);
  }
#else	/* !TERMCAP */
  term_locate(0, 0);
#endif	/* !TERMCAP */
}

/*
 * 行挿入
 */

#ifdef	__STDC__
void	term_insert(void)
#else	/* !__STDC__ */
void	term_insert()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_al) {
    tputs(term_al, 1, putchar);
  }
#else	/* !TERMCAP */
#ifdef	OS2
  v_init();
  v_insline(1);
#else	/* !OS2 */
  printf(INS_LINE_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !OS2 */
#endif	/* !TERMCAP */
}

/*
 * 複数行挿入
 */

#ifdef	__STDC__
void	term_insert_line(int line)
#else	/* !__STDC__ */
void	term_insert_line(line)
     int	line;
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_al) {
    tputs(term_al, line, putchar);
  }
#else	/* !TERMCAP */
#ifdef	OS2
  v_init();
  v_insline(line);
#else	/* !OS2 */
  printf(INS_MULTILINE_STRING, line);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !OS2 */
#endif	/* !TERMCAP */
}

/*
 * 文字挿入
 */

#ifdef	__STDC__
void	term_insert_char(void)
#else	/* !__STDC__ */
void	term_insert_char()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_ic) {
    tputs(term_ic, 1, putchar);
  }
#else	/* !TERMCAP */
  printf(INS_CHAR_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * カーソル上移動
 */

#ifdef	__STDC__
void	term_up(void)
#else	/* !__STDC__ */
void	term_up()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  tputs(term_u, 1, putchar);
#else	/* !TERMCAP */
  printf(UP_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * カーソル下移動
 */

#ifdef	__STDC__
void	term_down(void)
#else	/* !__STDC__ */
void	term_down()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  tputs(term_do, 1, putchar);
#else	/* !TERMCAP */
  printf(DOWN_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * カーソル左移動
 */

#ifdef	__STDC__
void	term_left(void)
#else	/* !__STDC__ */
void	term_left()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  tputs(term_le, 1, putchar);
#else	/* !TERMCAP */
  printf(LEFT_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * カーソル右移動
 */

#ifdef	__STDC__
void	term_right(void)
#else	/* !__STDC__ */
void	term_right()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  tputs(term_nd, 1, putchar);
#else	/* !TERMCAP */
  printf(RIGHT_STRING);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * カーソル行頭移動
 */

#ifdef	__STDC__
void	term_top(void)
#else	/* !__STDC__ */
void	term_top()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  tputs(term_cr, 1, putchar);
#else	/* !TERMCAP */
  putc('\r', stdout);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * 改行
 */

#ifdef	__STDC__
void	term_crlf(int pos)
#else	/* !__STDC__ */
void	term_crlf(pos)
     int	pos;
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (pos != term_columns || term_xn_nam) {
    tputs(term_cr, 1, putchar);
    tputs(term_do, 1, putchar);
  }
#else	/* !TERMCAP */
  printf("\r\n");
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

/*
 * 右端文字出力
 */

#ifdef	__STDC__
void	term_fill_right(void)
#else	/* !__STDC__ */
void	term_fill_right()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  if (term_xn_nam) {
    putc(' ', stdout);
    putc(' ', stdout);
  } else {
    putc(' ', stdout);
    term_left();
    term_insert_char();
  }
#else	/* !TERMCAP */
#ifdef	SUPPORT_KTX
  char	*ptr;

  ptr = getenv("TERM");
  if (ptr && !strncmp(ptr, "ktx-", 4)) {
    return;
  }
#endif	/* SUPPORT_KTX */
  putc(' ', stdout);
  term_left();
  term_insert_char();
#endif	/* !TERMCAP */
}

/*
 * 表示属性変更
 */

#ifdef	__STDC__
void	term_attrib(int attrib)
#else	/* !__STDC__ */
void	term_attrib(attrib)
     int	attrib;
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
#ifndef	TERMINFO
  char	buff[SMALL_BUFF];
#endif	/* !TERMINFO */

  switch (attrib) {
  case RESET_ATTRIB:
    tputs(term_me, 1, putchar);
    break;
  case LIGHT_ATTRIB:
    tputs(term_md, 1, putchar);
    break;
  case LINE_ATTRIB:
    tputs(term_us, 1, putchar);
    break;
  case BLINK_ATTRIB:
    tputs(term_mb, 1, putchar);
    break;
  case REVERSE_ATTRIB:
    tputs(term_mr, 1, putchar);
    break;
  case HIDE_ATTRIB:
    tputs(term_mh, 1, putchar);
    break;
#ifdef	COLOR
  default:
#ifdef	TERMINFO
    if (attrib >= FG_BLACK_ATTRIB && attrib <= FG_WHITE_ATTRIB) {
      tputs(tparm(term_af, attrib - FG_BLACK_ATTRIB), 1, putchar);
    } else if (attrib >= BG_BLACK_ATTRIB && attrib <= BG_WHITE_ATTRIB) {
      tputs(tparm(term_ab, attrib - BG_BLACK_ATTRIB), 1, putchar);
    }
#else	/* !TERMINFO */
    if (attrib >= FG_BLACK_ATTRIB && attrib <= FG_WHITE_ATTRIB) {
      if (term_af) {
	sprintf(buff, term_af, attrib - FG_BLACK_ATTRIB);
	tputs(buff, 1, putchar);
      }
    } else if (attrib >= BG_BLACK_ATTRIB && attrib <= BG_WHITE_ATTRIB) {
      if (term_ab) {
	sprintf(buff, term_ab, attrib - BG_BLACK_ATTRIB);
	tputs(buff, 1, putchar);
      }
    }
#endif	/* !TERMINFO */
    break;
#endif	/* COLOR */
  }
#else	/* !TERMCAP */
  printf(ATTRIB_STRING, attrib);
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* !TERMCAP */
}

#ifdef	notdef
/*
 * RAW モードセット
 */

#ifdef	__STDC__
void	term_set_raw(void)
#else	/* !__STDC__ */
void	term_set_raw()
#endif	/* !__STDC__ */
{
  term_raw = 1;
#ifdef	TERMIO
  tty_new.c_iflag &= ~(INLCR | ICRNL | ISTRIP | IXON | BRKINT);
  tty_new.c_oflag &= ~OPOST;
#ifdef	NEED_LNEXT
  tty_new.c_lflag &= ~(ICANON | ISIG | IEXTEN);
#else	/* !NEED_LNEXT */
  tty_new.c_lflag &= ~(ICANON | ISIG);
#endif	/* !NEED_LNEXT */
  tty_new.c_cc[VMIN] = 1;
  tty_new.c_cc[VTIME] = 0;
  ioctl(0, TCSETAF, &tty_new);
#else	/* !TERMIO */
  tty_new.sg_flags |= RAW;
  ioctl(0, TIOCSETP, &tty_new);
#endif	/* !TERMIO */
}

/*
 * RAW モードリセット
 */

#ifdef	__STDC__
void	term_reset_raw(void)
#else	/* !__STDC__ */
void	term_reset_raw()
#endif	/* !__STDC__ */
{
  term_raw = 0;
#ifdef	TERMIO
  tty_new.c_iflag |= (INLCR | ICRNL | IXON | BRKINT) | (tty_org.c_iflag & ISTRIP);
  tty_new.c_oflag |= OPOST;
#ifdef	NEED_LNEXT
  tty_new.c_lflag |= (ICANON | ISIG | IEXTEN);
#else	/* !NEED_LNEXT */
  tty_new.c_lflag |= (ICANON | ISIG);
#endif	/* !NEED_LNEXT */
  tty_new.c_cc[VMIN] = tty_org.c_cc[VMIN];
  tty_new.c_cc[VTIME] = tty_org.c_cc[VTIME];
#ifdef	USE_TERMIOS
  tcsetattr(0, TCSAFLUSH, &tty_new);
#else	/* !USE_TERMIOS */
  ioctl(0, TCSETAF, &tty_new);
#endif	/* !USE_TERMIOS */
#else	/* !TERMIO */
  tty_new.sg_flags &= ~RAW;
  ioctl(0, TIOCSETP, &tty_new);
#endif	/* !TERMIO */
}

/*
 * CBREAK モードセット
 */

#ifdef	__STDC__
void	term_set_cbreak(void)
#else	/* !__STDC__ */
void	term_set_cbreak()
#endif	/* !__STDC__ */
{
  term_cbreak = 1;
#ifdef	TERMIO
  tty_new.c_lflag &= ~ICANON;
  tty_new.c_cc[VMIN] = 1;
  tty_new.c_cc[VTIME] = 0;
#ifdef	USE_TERMIOS
  tcsetattr(0, TCSAFLUSH, &tty_new);
#else	/* !USE_TERMIOS */
  ioctl(0, TCSETAF, &tty_new);
#endif	/* !USE_TERMIOS */
#else	/* !TERMIO */
  tty_new.sg_flags |= CBREAK;
  ioctl(0, TIOCSETP, &tty_new);
#endif	/* !TERMIO */
}

/*
 * CBREAK モードリセット
 */

#ifdef	__STDC__
void	term_reset_cbreak(void)
#else	/* !__STDC__ */
void	term_reset_cbreak()
#endif	/* !__STDC__ */
{
  term_cbreak = 0;
#ifdef	TERMIO
  tty_new.c_lflag &= ICANON;
  tty_new.c_cc[VMIN] = tty_org.c_cc[VMIN];
  tty_new.c_cc[VTIME] = tty_org.c_cc[VTIME];
#ifdef	USE_TERMIOS
  tcsetattr(0, TCSAFLUSH, &tty_new);
#else	/* !USE_TERMIOS */
  ioctl(0, TCSETAF, &tty_new);
#endif	/* !USE_TERMIOS */
#else	/* !TERMIO */
  tty_new.sg_flags &= ~CBREAK;
  ioctl(0, TIOCSETP, &tty_new);
#endif	/* !TERMIO */
}

/*
 * ECHO モードセット
 */

#ifdef	__STDC__
void	term_set_echo(void)
#else	/* !__STDC__ */
void	term_set_echo()
#endif	/* !__STDC__ */
{
  term_echo = 1;
#ifdef	TERMIO
  tty_new.c_lflag &= ECHO;
#ifdef	USE_TERMIOS
  tcsetattr(0, TCSAFLUSH, &tty_new);
#else	/* !USE_TERMIOS */
  ioctl(0, TCSETAF, &tty_new);
#endif	/* !USE_TERMIOS */
#else	/* !TERMIO */
  tty_new.sg_flags |= ECHO;
  ioctl(0, TIOCSETP, &tty_new);
#endif	/* !TERMIO */
}

/*
 * ECHO モードリセット
 */

#ifdef	__STDC__
void	term_reset_echo(void)
#else	/* !__STDC__ */
void	term_reset_echo()
#endif	/* !__STDC__ */
{
  term_echo = 0;
#ifdef	TERMIO
  tty_new.c_lflag &= ~ECHO;
#ifdef	USE_TERMIOS
  tcsetattr(0, TCSAFLUSH, &tty_new);
#else	/* !USE_TERMIOS */
  ioctl(0, TCSETAF, &tty_new);
#endif	/* !USE_TERMIOS */
#else	/* !TERMIO */
  tty_new.sg_flags &= ~ECHO;
  ioctl(0, TIOCSETP, &tty_new);
#endif	/* !TERMIO */
}

/*
 * NL モードセット
 */

#ifdef	__STDC__
void	term_set_nl(void)
#else	/* !__STDC__ */
void	term_set_nl()
#endif	/* !__STDC__ */
{
  term_nl = 1;
#ifdef	TERMIO
  tty_new.c_iflag &= ~(INLCR | ICRNL);
  tty_new.c_oflag &= ~OPOST;
#ifdef	USE_TERMIOS
  tcsetattr(0, TCSAFLUSH, &tty_new);
#else	/* !USE_TERMIOS */
  ioctl(0, TCSETAF, &tty_new);
#endif	/* !USE_TERMIOS */
#else	/* !TERMIO */
  tty_new.sg_flags |= CRMOD;
  ioctl(0, TIOCSETP, &tty_new);
#endif	/* !TERMIO */
}

/*
 * NL モードリセット
 */

#ifdef	__STDC__
void	term_reset_nl(void)
#else	/* !__STDC__ */
void	term_reset_nl()
#endif	/* !__STDC__ */
{
  term_nl = 0;
#ifdef	TERMIO
  tty_new.c_iflag |= (INLCR | ICRNL);
  tty_new.c_oflag |= OPOST;
#ifdef	USE_TERMIOS
  tcsetattr(0, TCSAFLUSH, &tty_new);
#else	/* !USE_TERMIOS */
  ioctl(0, TCSETAF, &tty_new);
#endif	/* !USE_TERMIOS */
#else	/* !TERMIO */
  tty_new.sg_flags &= ~CRMOD;
  ioctl(0, TIOCSETP, &tty_new);
#endif	/* !TERMIO */
}
#endif	/* notdef */

/*
 * マウス許可
 */

#ifdef	__STDC__
void	term_enable_mouse(void)
#else	/* !__STDC__ */
void	term_enable_mouse()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  tputs("\033[?1000h", 1, putchar);
#else	/* !TERMCAP */
  printf("\033[?1000h");
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* TERMCAP */
}

/*
 * マウス禁止
 */

#ifdef	__STDC__
void	term_disable_mouse(void)
#else	/* !__STDC__ */
void	term_disable_mouse()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
  tputs("\033[?1000l", 1, putchar);
#else	/* !TERMCAP */
  printf("\033[?1000l");
#ifdef	NEED_FFLUSH
  fflush(stdout);
#endif	/* NEED_FFLUSH */
#endif	/* TERMCAP */
}

/*
 * 端末一時解放実行
 */

#ifdef	__STDC__
int	term_system(char *str)
#else	/* !__STDC__ */
int	term_system(str)
     char	*str;
#endif	/* !__STDC__ */
{
  int		pstat;

#ifdef	USE_SYSTEM
  term_init(1);
  pstat = system(str);
  term_init(2);
  return(pstat);
#else	/* !USE_SYSTEM */
  int		pid;
  void		(*sig_int)(), (*sig_quit)(), (*sig_term)(), (*sig_pipe)(),
		(*sig_winch)(), (*sig_hup)();
#ifdef	OS2
  void		(*sig_break)();
#endif	/* OS2 */
  char		*shell;
  
  sig_int  = (void (*)())signal(SIGINT, SIG_IGN);
  sig_quit = (void (*)())signal(SIGQUIT, SIG_IGN);
  sig_term = (void (*)())signal(SIGTERM, SIG_IGN);
  sig_pipe = (void (*)())signal(SIGPIPE, SIG_IGN);
  sig_hup  = (void (*)())signal(SIGHUP, SIG_IGN);
#ifdef	SIGWINCH
  sig_winch = (void (*)())signal(SIGWINCH, SIG_IGN);
#endif	/* SIGWINCH */
#ifdef	OS2
  sig_break = (void (*)())signal(SIGBREAK, SIG_IGN);
#endif	/* OS2 */

  term_init(1);
  pid = vfork();
  if (pid == -1) {
    return(127);
  }
  if (!pid) {
    if (str) {
#ifdef	OS2
      execl("cmd.exe", "cmd.exe", "/C", str, (char*)NULL);
#else	/* !OS2 */
      execl("/bin/sh", "sh", "-c", str, (char*)NULL);
#endif	/* !OS2 */
    } else {
      if (!(shell = (char*)getenv("SHELL"))) {
#ifdef	OS2
	shell = "cmd.exe";
#else	/* !OS2 */
	shell = "/bin/sh";
#endif	/* !OS2 */
      }
      execl(shell, shell, (char*)NULL);
    }
    _exit(127);
  }
  wait((int *)&pstat);
  term_init(2);
  signal(SIGINT,  sig_int);
  signal(SIGQUIT, sig_quit);
  signal(SIGTERM, sig_term);
  signal(SIGPIPE, sig_pipe);
  signal(SIGHUP, sig_hup);
#ifdef	SIGWINCH
  signal(SIGWINCH, sig_winch);
#endif	/* SIGWINCH */
#ifdef	OS2
  signal(SIGBREAK, sig_break);
#endif	/* OS2 */
  
  return(pstat);
#endif	/* !USE_SYSTEM */
}

/*
 * 端末セレクト
 */

#ifdef	__STDC__
int	term_select(void)
#else	/* !__STDC__ */
int	term_select()
#endif	/* !__STDC__ */
{
#ifndef	DONT_HAVE_SELECT
  fd_set		readfds;
  struct timeval	tv;

  FD_ZERO(&readfds);
  tv.tv_sec  = 0;
  tv.tv_usec = 0;
  FD_SET(0, &readfds);
  if (select(1, &readfds, NULL, NULL, &tv) > 0) {
    if (FD_ISSET(0, &readfds)) {
      return(1);
    }
  }
#endif	/* !DONT_HAVE_SELECT */
  return(0);
}

/*
 * ウィンドウリサイズ
 */

#ifdef	__STDC__
static void	window_resize(void)
#else	/* !__STDC__ */
static void	window_resize()
#endif	/* !__STDC__ */
{
#ifdef	TERMCAP
#ifdef	TIOCGWINSZ
  struct winsize	winsize;
#endif	/* TIOCGWINSZ */

  term_resize_flag = 1;
#ifdef	SIGWINCH
  signal(SIGWINCH, window_resize);
#endif	/* SIGWINCH */
#ifdef	TIOCGWINSZ
  if (ioctl(0, TIOCGWINSZ, &winsize) == 0) {
    if (winsize.ws_col > 0 && winsize.ws_row > 0) {
      term_columns = winsize.ws_col;
      term_lines = winsize.ws_row;
      return;
    }
  }
#endif	/* TIOCGWINSZ */
#ifdef	TERMINFO
  term_columns = columns;
  term_lines   = lines;
#else	/* !TERMINFO */
  term_columns = tgetnum("co");
  term_lines   = tgetnum("li");
#endif	/* !TERMINFO */
  if (term_columns <= 0) {
    term_columns = DEFAULT_COLUMNS;
  }
  if (term_lines <= 0) {
    term_lines = DEFAULT_LINES;
  }
#else	/* !TERMCAP */
#ifdef	OS2
  v_init();
  v_dimen(&term_columns, &term_lines);
#else	/* !OS2 */
#ifdef	TIOCGWINSZ
  struct winsize	winsize;
#endif	/* TIOCGWINSZ */
#ifndef	TIOCGWINSZ
  char			*ptr;
#endif	/* TIOCGWINSZ */

#ifdef	SIGWINCH
  signal(SIGWINCH, (void (*)())window_resize);
#endif	/* SIGWINCH */
  term_resize_flag = 1;
#ifdef	TIOCGWINSZ
  if (ioctl(0, TIOCGWINSZ, &winsize) < 0) {
    return;
  }
  if ((term_columns = winsize.ws_col) <= 0) {
    term_columns = DEFAULT_COLUMNS;
  }
  if ((term_lines   = winsize.ws_row) <= 0) {
    term_lines = DEFAULT_LINES;
  }
#else	/* !TIOCGWINSZ */
  term_columns = DEFAULT_COLUMNS;
  term_lines = DEFAULT_LINES;
  if (ptr = getenv("LINES")) {
    term_lines = atoi(ptr);
  }
  if (ptr = getenv("COLUMNS")) {
    term_columns = atoi(ptr);
  }
#endif	/* !TIOCGWINSZ */
#endif	/* !OS2 */
#endif	/* !TERMCAP */
}

/*
 * キーボード割り込み許可
 */

#ifdef	__STDC__
void	term_enable_abort(void)
#else	/* !__STDC__ */
void	term_enable_abort()
#endif	/* !__STDC__ */
{
  term_abort_flag = 0;
  signal(SIGINT, kbd_abort);
}

/*
 * キーボード割り込み禁止
 */

#ifdef	__STDC__
void	term_disable_abort(void)
#else	/* !__STDC__ */
void	term_disable_abort()
#endif	/* !__STDC__ */
{
  term_abort_flag = 0;
  signal(SIGINT, SIG_IGN);
}

/*
 * キーボード割り込み
 */

#ifdef	__STDC__
static void	kbd_abort(void)
#else	/* !__STDC__ */
static void	kbd_abort()
#endif	/* !__STDC__ */
{
  term_disable_abort();
  term_abort_flag = 1;
}
