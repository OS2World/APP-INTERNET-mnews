/*
 *
 *  ü������饤�֥��
 *
 *  Copyright 1990-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Terminal Control Library
 *  Sub system  : library routine [for MS-DOS/OS-2/X680x0 only]
 *  File        : termdos.c
 *  Version     : 1.67
 *  First Edit  : 1994-03/15
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 ��� ��
 *
 */

#include	"compat.h"
#include	"termlib.h"
#ifdef	MNEWS
#include	"kanjilib.h"
#endif	/* MNEWS */
#ifdef	OS2
#include	<sys/video.h>
#endif	/* OS2 */
#ifdef	X68K
#include	<sys/iocs.h>
#endif	/* !X68K */

#ifdef	__WIN32__
#include	<windows.h>
#endif	/* __WIN32__ */
#ifdef	__STDC__
static void	window_resize(void);	/* ������ɥ��ꥵ����	*/
static void	kbd_abort(void);	/* �����ܡ��ɳ�����	*/
#else	/* !__STDC__ */
static void	window_resize();	/* ������ɥ��ꥵ����	*/
static void	kbd_abort();		/* �����ܡ��ɳ�����	*/
#endif	/* !__STDC__ */

int		term_columns = DEFAULT_COLUMNS;	/* ü���Կ�		*/
int		term_lines = DEFAULT_LINES;	/* ü�����		*/
int		term_resize_flag = 0;		/* ü��������ե饰	*/
int		term_abort_flag = 0;		/* ü���������ǥե饰	*/

static int		term_nl = 1;		/* ���ԥ⡼��		*/
static int		term_echo = 1;		/* �������⡼��		*/
static int		term_cbreak = 0;	/* CBREAK �⡼��	*/
static int		term_raw = 0;		/* RAW �⡼��		*/
static int		init_flag = 0;		/* ������ե饰		*/
#ifdef	__WIN32__
static HANDLE hin = NULL, hout = NULL;
static void win32_console_init(void);
static DWORD win32_console_color(int attr, DWORD old);
static DWORD current_attr;

#define WIN32_ATTR_COLOR_MASK \
	(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY|\
	 BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY)
#define WIN32_ATTR_DEFAULT \
	(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY)
#endif	/* __WIN32__ */

/*
 * ü�������
 */

#ifdef	__STDC__
int	term_init(int mode)
#else	/* !__STDC__ */
int	term_init(mode)
     int	mode;
#endif	/* !__STDC__ */
{
  switch (mode) {
  case 0:
#ifdef	OS2
    v_init();
#endif	/* OS2 */
#ifdef	WIN32_CONSOLE
    win32_console_init();
#endif	/* WIN32_CONSOLE */
    term_columns = DEFAULT_COLUMNS;
    term_lines   = DEFAULT_LINES;
    term_resize_flag = 0;
    term_abort_flag = 0;
#ifdef	SIGWINCH
    signal(SIGWINCH, window_resize);
#endif	/* SIGWINCH */
    window_resize();
    if (!init_flag) {
      init_flag = 1;
    }
    break;
  case 3:
    term_reset_raw();
    term_reset_cbreak();
    term_set_echo();
    term_set_nl();
    /* break ������ */
  case 1:
    if (init_flag) {
    }
    break;
  case 2:
    if (term_cbreak) {
      term_set_cbreak();
    }
    if (term_raw) {
      term_set_raw();
    }
    if (!term_nl) {
      term_reset_nl();
    }
    if (!term_echo) {
      term_reset_echo();
    }
    break;
  default:
    break;
  }
  return(0);
}

/*
 * ���̥��ꥢ
 */

#ifdef	__STDC__
void	term_clear_all(void)
#else	/* !__STDC__ */
void	term_clear_all()
#endif	/* !__STDC__ */
{
#ifdef	OS2
  v_clear();
  v_gotoxy(0, 0);
#else	/* !OS2 */
#ifdef	WIN32_CONSOLE
  DWORD	count;
  COORD	coord;
  coord.X = coord.Y = 0;

  FillConsoleOutputCharacter(hout, ' ', term_columns*term_lines, coord,
			     &count);
  nFillConsoleOutputAttribute(hout, WIN32_ATTR_DEFAULT,
			      term_columns * term_lines, coord, &count);
  SetConsoleCursorPosition(hout, coord);
#else	/* !WIN32_CONSOLE */
  printf(CLEAR_ALL_STRING);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
#endif	/* !OS2 */
}

/*
 * �����ޤǺ��
 */

#ifdef	__STDC__
void	term_clear_line(void)
#else	/* !__STDC__ */
void	term_clear_line()
#endif	/* !__STDC__ */
{
  printf(CLEAR_LINE_STRING);
  fflush(stdout);
}

/*
 * ����ü�ޤǺ��
 */

#ifdef	__STDC__
void	term_clear_end(void)
#else	/* !__STDC__ */
void	term_clear_end()
#endif	/* !__STDC__ */
{
  printf(CLEAR_END_STRING);
  fflush(stdout);
}

/*
 * �ٹ�٥�
 */

#ifdef	__STDC__
void	term_bell(void)
#else	/* !__STDC__ */
void	term_bell()
#endif	/* !__STDC__ */
{
  printf(BELL_STRING);
  fflush(stdout);
}

/*
 * ���������ư
 */

#ifdef	__STDC__
void	term_locate(int x, int y)
#else	/* !__STDC__ */
void	term_locate(x, y)
     int	x;
     int	y;
#endif	/* !__STDC__ */
{
#ifdef	OS2
  v_gotoxy(x, y);
#else	/* !OS2 */
#ifdef	WIN32_CONSOLE
  DWORD				count, ret;
  CONSOLE_SCREEN_BUFFER_INFO	csbi;

  GetConsoleScreenBufferInfo(hout, &csbi);
  count = term_columns - csbi.dwCursorPosition.X;
  FillConsoleOutputCharacter(hout, ' ', count, csbi.dwCursorPosition, &ret);
  FillConsoleOutputAttribute(hout, WIN32_ATTR_DEFAULT, count, csbi.
			     dwCursorPosition, &ret);
#else	/* !WIN32_CONSOLE */
  printf(LOCATE_STRING, y + 1, x + 1);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
#endif	/* !OS2 */
}

/*
 * ��������ۡ�����ְ�ư
 */

#ifdef	__STDC__
void	term_home(void)
#else	/* !__STDC__ */
void	term_home()
#endif	/* !__STDC__ */
{
  term_locate(0, 0);
}

/*
 * ������
 */

#ifdef	__STDC__
void	term_insert(void)
#else	/* !__STDC__ */
void	term_insert()
#endif	/* !__STDC__ */
{
#ifdef	OS2
  int	x, y;

  v_getxy(&x, &y);
  v_gotoxy(0 , y);
  v_insline(1);
  v_gotoxy(0 , y);
#else	/* !OS2 */
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;
  SMALL_RECT			rect;
  COORD				coord;
  CHAR_INFO			ch;
  DWORD				ret;

  GetConsoleScreenBufferInfo(hout, &csbi);
  rect.Left = 0;
  rect.Top = csbi.dwCursorPosition.Y;
  rect.Right = term_columns - 1;
  rect.Bottom = term_lines - 1;
  coord.X = 0;
  coord.Y = csbi.dwCursorPosition.Y + 1;
  ch.Char.AsciiChar = 0;
  ch.Attributes = 0;
  ScrollConsoleScreenBuffer(hout, &rect, NULL, coord, &ch);
  coord.X = 0;
  coord.Y = csbi.dwCursorPosition.Y;
  FillConsoleOutputCharacter(hout, ' ', term_columns, coord, &ret);
  FillConsoleOutputAttribute(hout, WIN32_ATTR_DEFAULT, term_columns,
			     coord, &ret);
#else	/* !WIN32_CONSOLE */
  printf(INS_LINE_STRING);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
#endif	/* !OS2 */
}

/*
 * ʣ��������
 */

#ifdef	__STDC__
void	term_insert_line(int line)
#else	/* !__STDC__ */
void	term_insert_line(line)
     int	line;
#endif	/* !__STDC__ */
{
#ifdef	OS2
  int	x, y;

  v_getxy(&x, &y);
  v_gotoxy(0 , y);
  v_insline(line);
  v_gotoxy(0 , y);
#else	/* !OS2 */
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;
  SMALL_RECT			rect;
  COORD				coord;
  CHAR_INFO			ch;
  DWORD				ret;

  GetConsoleScreenBufferInfo(hout, &csbi);
  rect.Left = 0;
  rect.Top = csbi.dwCursorPosition.Y;
  rect.Right = term_columns - 1;
  rect.Bottom = term_lines - 1;
  coord.X = 0;
  coord.Y = csbi.dwCursorPosition.Y+line;
  ch.Char.AsciiChar = 0;
  ch.Attributes = 0;
  ScrollConsoleScreenBuffer(hout, &rect, NULL, coord, &ch);
  coord.X = 0;
  coord.Y = csbi.dwCursorPosition.Y;
  FillConsoleOutputCharacter(hout, ' ', line * term_columns, coord, &ret);
  FillConsoleOutputAttribute(hout, WIN32_ATTR_DEFAULT, line * term_columns,
			     coord, &ret);
#else	/* !WIN32_CONSOLE */
  printf(INS_MULTILINE_STRING, line);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
#endif	/* !OS2 */
}

/*
 * ʸ������
 */

#ifdef	__STDC__
void	term_insert_char(void)
#else	/* !__STDC__ */
void	term_insert_char()
#endif	/* !__STDC__ */
{
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;
  SMALL_RECT			rect;
  COORD				coord;
  CHAR_INFO			ch;

  GetConsoleScreenBufferInfo(hout, &csbi);
  rect.Left = csbi.dwCursorPosition.X;
  rect.Top = rect.Bottom = coord.Y = csbi.dwCursorPosition.Y;
  rect.Right = term_columns - 1;
  coord.X = csbi.dwCursorPosition.X + 1;
  
  ch.Char.AsciiChar = 0;
  ch.Attributes = 0;
  ScrollConsoleScreenBuffer(hout, &rect, NULL, coord, &ch);
#else	/* !WIN32_CONSOLE */
  printf(INS_CHAR_STRING);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
}

/*
 * ����������ư
 */

#ifdef	__STDC__
void	term_up(void)
#else	/* !__STDC__ */
void	term_up()
#endif	/* !__STDC__ */
{
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;

  GetConsoleScreenBufferInfo(hout, &csbi);
  csbi.dwCursorPosition.Y--;
  SetConsoleCursorPosition(hout, csbi.dwCursorPosition);
#else	/* !WIN32_CONSOLE */
  printf(UP_STRING);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
}

/*
 * �������벼��ư
 */

#ifdef	__STDC__
void	term_down(void)
#else	/* !__STDC__ */
void	term_down()
#endif	/* !__STDC__ */
{
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;

  GetConsoleScreenBufferInfo(hout, &csbi);
  csbi.dwCursorPosition.Y++;
  SetConsoleCursorPosition(hout, csbi.dwCursorPosition);
#else	/* !WIN32_CONSOLE */
  printf(DOWN_STRING);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
}

/*
 * �������뺸��ư
 */

#ifdef	__STDC__
void	term_left(void)
#else	/* !__STDC__ */
void	term_left()
#endif	/* !__STDC__ */
{
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;

  GetConsoleScreenBufferInfo(hout, &csbi);
  csbi.dwCursorPosition.X--;
  SetConsoleCursorPosition(hout, csbi.dwCursorPosition);
#else	/* !WIN32_CONSOLE */
  printf(LEFT_STRING);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
}

/*
 * �������뱦��ư
 */

#ifdef	__STDC__
void	term_right(void)
#else	/* !__STDC__ */
void	term_right()
#endif	/* !__STDC__ */
{
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;

  GetConsoleScreenBufferInfo(hout, &csbi);
  csbi.dwCursorPosition.X++;
  SetConsoleCursorPosition(hout, csbi.dwCursorPosition);
#else	/* !WIN32_CONSOLE */
  printf(RIGHT_STRING);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
}

/*
 * ���������Ƭ��ư
 */

#ifdef	__STDC__
void	term_top(void)
#else	/* !__STDC__ */
void	term_top()
#endif	/* !__STDC__ */
{
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;

  GetConsoleScreenBufferInfo(hout, &csbi);
  csbi.dwCursorPosition.X = 0;
  SetConsoleCursorPosition(hout, csbi.dwCursorPosition);
#else	/* !WIN32_CONSOLE */
  putc('\r', stdout);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
}

/*
 * ����
 */

#ifdef	__STDC__
void	term_crlf(int pos)
#else	/* !__STDC__ */
void	term_crlf(pos)
     int	pos;
#endif	/* !__STDC__ */
{
  printf("\r\n");
  fflush(stdout);
}

/*
 * ��üʸ������
 */

#ifdef	__STDC__
void	term_fill_right(void)
#else	/* !__STDC__ */
void	term_fill_right()
#endif	/* !__STDC__ */
{
  putc(' ', stdout);
  term_left();
  term_insert_char();
}

/*
 * ɽ��°���ѹ�
 */

#ifdef	__STDC__
void	term_attrib(int attrib)
#else	/* !__STDC__ */
void	term_attrib(attrib)
     int	attrib;
#endif	/* !__STDC__ */
{
#ifdef	WIN32_CONSOLE
  DWORD	newattr;

  newattr = win32_console_color(attrib, current_attr);
  if (newattr != current_attr) {
    SetConsoleTextAttribute(hout, current_attr = newattr);
  }
#else	/* !WIN32_CONSOLE */
  printf(ATTRIB_STRING, attrib);
  fflush(stdout);
#endif	/* !WIN32_CONSOLE */
}

/*
 * RAW �⡼�ɥ��å�
 */

#ifdef	__STDC__
void	term_set_raw(void)
#else	/* !__STDC__ */
void	term_set_raw()
#endif	/* !__STDC__ */
{
  term_raw = 1;
}

/*
 * RAW �⡼�ɥꥻ�å�
 */

#ifdef	__STDC__
void	term_reset_raw(void)
#else	/* !__STDC__ */
void	term_reset_raw()
#endif	/* !__STDC__ */
{
  term_raw = 0;
}

/*
 * CBREAK �⡼�ɥ��å�
 */

#ifdef	__STDC__
void	term_set_cbreak(void)
#else	/* !__STDC__ */
void	term_set_cbreak()
#endif	/* !__STDC__ */
{
  term_cbreak = 1;
}

/*
 * CBREAK �⡼�ɥꥻ�å�
 */

#ifdef	__STDC__
void	term_reset_cbreak(void)
#else	/* !__STDC__ */
void	term_reset_cbreak()
#endif	/* !__STDC__ */
{
  term_cbreak = 0;
}

/*
 * ECHO �⡼�ɥ��å�
 */

#ifdef	__STDC__
void	term_set_echo(void)
#else	/* !__STDC__ */
void	term_set_echo()
#endif	/* !__STDC__ */
{
  term_echo = 1;
}

/*
 * ECHO �⡼�ɥꥻ�å�
 */

#ifdef	__STDC__
void	term_reset_echo(void)
#else	/* !__STDC__ */
void	term_reset_echo()
#endif	/* !__STDC__ */
{
  term_echo = 0;
}

/*
 * NL �⡼�ɥ��å�
 */

#ifdef	__STDC__
void	term_set_nl(void)
#else	/* !__STDC__ */
void	term_set_nl()
#endif	/* !__STDC__ */
{
  term_nl = 1;
}

/*
 * NL �⡼�ɥꥻ�å�
 */

#ifdef	__STDC__
void	term_reset_nl(void)
#else	/* !__STDC__ */
void	term_reset_nl()
#endif	/* !__STDC__ */
{
  term_nl = 0;
}

/*
 * ü����������¹�
 */

#ifdef	__STDC__
int	term_system(char *str)
#else	/* !__STDC__ */
int	term_system(str)
     char	*str;
#endif	/* !__STDC__ */
{
  int		pstat;

  term_init(1);
  pstat = system(str);
  term_init(2);
  return(pstat);
}

/*
 * ü�����쥯��
 */

#ifdef	__STDC__
int	term_select(void)
#else	/* !__STDC__ */
int	term_select()
#endif	/* !__STDC__ */
{
  return(0);
}

/*
 * ������ɥ��ꥵ����
 */

#ifdef	__STDC__
static void	window_resize(void)
#else	/* !__STDC__ */
static void	window_resize()
#endif	/* !__STDC__ */
{
#ifdef	OS2
  v_dimen(&term_columns, &term_lines);
#else	/* !OS2 */
#ifdef	WIN32_CONSOLE
  CONSOLE_SCREEN_BUFFER_INFO	csbi;

  GetConsoleScreenBufferInfo(hout, &csbi);
  term_lines = csbi.dwSize.Y;
  term_columns = csbi.dwSize.X;
#else	/* !WIN32_CONSOLE */
#ifdef	X68K
  long	data;
#endif	/* X68K */
  char	*ptr;

#ifdef	X68K
  data = _iocs_b_consol(-1, -1, -1, -1);
  term_columns = (data >> 16) + 1;
  term_lines = (data & 0xFFFF) + 1;
#else	/* !X68K */
#ifdef	GET_TERM_SIZE
  int	c;

  fputs("\x1b[99B\x1b[99C\x1b[99C\x1b[6n", stderr);
  c = getch();
  while (!isdigit(c)) {
    c = getch();
  }
  term_lines = c - '0';
  c = getch(); term_lines = term_lines * 10 + c - '0';
  c = getch();
  c = getch(); term_columns = c - '0';
  c = getch(); term_columns = term_columns * 10 + c - '0';
  c = getch();
#else	/* !GET_TERM_SIZE */
  term_columns = DEFAULT_COLUMNS;
  term_lines = DEFAULT_LINES;
#endif	/* !GET_TERM_SIZE */
#endif	/* !X68K */
  ptr = getenv("LINES");
  if (ptr != (char*)NULL) {
    term_lines = atoi(ptr);
  }
  ptr = getenv("COLUMNS");
  if (ptr != (char*)NULL) {
    term_columns = atoi(ptr);
  }
#endif	/* !WIN32_CONSOLE */
#endif	/* !OS2 */
}

/*
 * �����ܡ��ɳ����ߵ���
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
 * �����ܡ��ɳ����߶ػ�
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
 * �����ܡ��ɳ�����
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

#ifdef	WIN32_CONSOLE
static DWORD	win32_console_color(int attr, DWORD old)
{
  DWORD	c = 0;

  c |= FOREGROUND_INTENSITY;
  if (attr >= 30 && attr <= 37) {
    if ((attr-30) & 1) c |= FOREGROUND_RED;
    if ((attr-30) & 2) c |= FOREGROUND_GREEN;
    if ((attr-30) & 4) c |= FOREGROUND_BLUE;
    c |= old & ~(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
  } else if (attr >= 40 && attr <= 47) {
    if ((attr-40) & 1) c |= BACKGROUND_RED;
    if ((attr-40) & 2) c |= BACKGROUND_GREEN;
    if ((attr-40) & 4) c |= BACKGROUND_BLUE;
    c |= old & ~(BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
  } else if (attr == 0) {
    c = WIN32_ATTR_DEFAULT;
  } else if (attr == 1) {
    c |= FOREGROUND_INTENSITY;
  } else if (attr == 4) {
    c |= COMMON_LVB_UNDERSCORE;
  } else if (attr == 5) {
  } else if (attr == 7) {
    c |= COMMON_LVB_REVERSE_VIDEO;
  } else if (attr == 8) {
    c = 0;
  } else {
    c = WIN32_ATTR_DEFAULT;
  }
  return(c);
}

static int mousemode = 0;

static void win32_console_init(void)
{
  hin  = GetStdHandle(STD_INPUT_HANDLE);
  hout = GetStdHandle(STD_ERROR_HANDLE);

#ifdef	MOUSE
  SetConsoleMode(hin, ENABLE_PROCESSED_INPUT
		 | ENABLE_MOUSE_INPUT
		 | ENABLE_WINDOW_INPUT);
#else	/* !MOUSE */
  SetConsoleMode(hin, ENABLE_PROCESS_INPUT);
#endif	/* !MOUSE */
}
#endif	/* WIN32_CONSOLE */
