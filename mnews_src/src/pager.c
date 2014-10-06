/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1992-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Pager routine
 *  File        : pager.c
 *  Version     : 1.21
 *  First Edit  : 1992-08/05
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 宅間 顯
 *
 */

#ifdef	PAGER
#include	"compat.h"
#include	"kanjilib.h"
#include	"termlib.h"
#ifdef	MNEWS
#include	"nntplib.h"
#include	"field.h"
#include	"mnews.h"
#include	"group.h"
#include	"article.h"
#endif	/* MNEWS */
#ifdef	MLESS
#ifdef	COLOR
#include	"nntplib.h"
#include	"field.h"
#endif	/* COLOR */
#include	"mless.h"
#endif	/* MLESS */
#ifdef	MANEKIN
#include	"manekin.h"
#endif	/* MANEKIN */
#include	"pager.h"

static int	prev_line();		/* 前行移動			*/
static int	next_line();		/* 次行移動			*/
static int	search_line();		/* 指定行移動			*/
static void	count_line();		/* 行位置取得			*/
static int	prev_position();	/* 前行取得			*/
static int	next_position();	/* 次行取得			*/
static int	forward_search();	/* 前方検索			*/
static int	backward_search();	/* 後方検索			*/
static void	view_all();		/* 全体表示			*/
static int	view_top();		/* 上段表示			*/
static int	view_bottom();		/* 下段表示			*/
static int	fread_line();		/* ファイル 1 行入力		*/
static int	fgets_prev();		/* ファイル直前 1 行入力	*/
static int	kanji_strposcpy();	/* 漢字文字列コピー(表示幅)	*/
static char	*kanji_strindex();	/* 文字列検索			*/
#if	defined(MNEWS) && defined(SAVE_PAGER)
static int	save_file();		/* ファイルセーブ		*/
#endif	/* (MNEWS && SAVE_PAGER) */
#if	defined(MNEWS) && defined(LARGE)
static int	print_file();		/* ファイル印刷			*/
#endif	/* (MNEWS && LARGE) */
#ifdef	COLOR
static int	check_field();		/* ヘッダチェック		*/
#endif	/* COLOR */

extern char	tmp_dir[];		/* 作業ディレクトリ		*/
extern short	japanese;		/* 日本語モード			*/
#ifdef	COLOR
extern short	color_mode;		/* カラーモード			*/
#endif	/* COLOR */

int		last_key = 0;		/* 最後に入力したキー		*/
char		guide_message[SMALL_BUFF];

static struct show_file {
  char		*file_name;
  FILE		*fp;
  int		line_number;
  int		valid_height;
  long		top_position, end_position, search_position, max_position;
  CHARSET	charset;
#ifdef	CTRL_L
  int		stop_flag;
#endif	/* CTRL_L */
#ifdef	COLOR
  long		field_position;
#endif	/* COLOR */
} file_struct[MAX_SHOW_FILE + 1];	/* + 1 はヘルプ画面用	*/

static int	line_number;
static int	valid_height;
static FILE	*fp;
static long	top_position, end_position, search_position;
static CHARSET	charset;
static int	kanji_strwidth = 0;	/* 漢字文字幅		*/
#ifdef	CTRL_L
static int	stop_flag;
#endif	/* CTRL_L */
#ifdef	COLOR
static long	field_position;
#endif	/* COLOR */

static char	*pager_jmessage[] = {
#ifdef	MNEWS
  "ページャーモード",
  "\n  [参照]",
#else	/* !MNEWS */
  "ヘルプモード\n",
#endif	/* !MNEWS */
  "\tb または ^B     前画面を表示します。(行数指定可能)",
#ifdef	MNEWS
  "\t^F              次画面を表示します。(行数指定可能)",
#else	/* !MNEWS */
  "\tf または ^F     次画面を表示します。(行数指定可能)",
#endif	/* !MNEWS */
  "\tSPACE           次画面を表示し、最後なら終了します。(行数指定可能)",
  "\tk または ^P     前の行を表示します。",
  "\te, j, ^N または RETURN",
  "\t                次の行を表示します。",
  "\tg または <      先頭を表示します。(行番号指定可能)",
  "\tG または >      最後を表示します。(行番号指定可能)",
#ifdef	MNEWS
  "\t^U              前の半ページを表示します。(行数指定可能)",
  "\t^D              次の半ページを表示します。(行数指定可能)",
  "\tv または V      全てのヘッダとともに記事を再表示します。",
  "\t                ('V' は MIME デコード対応)",
  "\t^               リファレンスを参照します。(ニュースモード時のみ)",
#ifndef	SMALL
  "\t*               Message-ID の記事を参照します。",
  "\t                (ニュースモード時のみ)",
#endif	/* !SMALL */
  "\n  [移動]",
  "\tp               前の未読記事を表示します。",
  "\tn               次の未読記事を表示します。",
  "\tP               前の記事を表示します。",
  "\tN               次の記事を表示します。",
  "\n  [マーク]",
  "\tD               マークして前の未読記事を表示します。",
  "\td               マークして次の未読記事を表示します。",
  "\tU               マーク解除して前の記事を表示します。",
  "\tu               マーク解除して次の記事を表示します。",
#ifndef	SMALL
  "\tK               同一サブジェクトの記事を既読マークします。",
  "\t                (ニュース/BOARD モード時のみ)",
#endif	/* !SMALL */
  "\n  [投稿]",
  "\tf               記事にフォローします。",
  "\t                (ニュース/BOARD モード時のみ)",
  "\tF               記事を引用してフォローします。",
  "\t                (ニュース/BOARD モード時のみ)",
  "\n  [送信]",
  "\tr               記事に返信します。",
  "\tR               記事を引用して返信します。",
  "\t= または f      記事を転送します。",
  "\t                (f はメールモード時のみ)",
#ifdef	LARGE
  "\t~               記事を回送します。",
#endif	/* LARGE */
  "\n  [セーブ]",
  "\ts               既読セーブします。",
  "\t                (ファイル名を入力して下さい。ファイルが存在する時は",
  "\t                 y:アペンド n:中止 o:上書きを選んで下さい)",
#ifdef	MH
  "\tO               記事を MH フォルダにセーブします。",
  "\t                (MH フォルダ名を入力して下さい)",
#endif	/* MH */
  "\n  [検索]",
#else	/* !MNEWS */
  "\tu または ^U     前の半ページを表示します。(行数指定可能)",
  "\td または ^D     次の半ページを表示します。(行数指定可能)",
  "\tp               後方に PATTERN を次検索します。",
  "\tn               前方に PATTERN を次検索します。",
  "\tP               前のファイルを参照します。",
  "\tN               次のファイルを参照します。",
#endif	/* !MNEWS */
  "\t/PATTERN        前方に PATTERN を検索します。",
  "\t\\PATTERN        後方に PATTERN を検索します。",
#ifdef	MNEWS
  "\n  [表示]",
  "\t^L              再表示します。",
#else	/* !MNEWS */
  "\tR, r または ^L  再表示します。",
#endif	/* !MNEWS */
  "\t?               ヘルプ表示します。",
  "\tA               表示をコード無変換で行ないます。",
  "\tE               表示を EUC コードで行ないます。",
  "\tJ               表示を JIS コードで行ないます。",
  "\tS               表示を SJIS コードで行ないます。",
#ifndef	MNEWS
  "\t=               行番号を表示します。",
#endif	/* !MNEWS */
#ifdef	MNEWS
#ifdef	LARGE
  "\n  [印刷]",
  "\t@               記事を印刷します。",
#endif	/* LARGE */
  "\n  [終了]",
#endif	/* MNEWS */
  "\to, q または Q   ページャを終了します。",
  "\t^Z              サスペンドします。",
  (char*)NULL,
};

static char	*pager_message[] = {
#ifndef	SMALL
#ifdef	MNEWS
  "PAGER MODE",
  "\n  [READ]",
#else	/* !MNEWS */
  "HELP MODE\n",
#endif	/* !MNEWS */
  "\tb or ^B      previous page.(You can specify lines.)",
#ifdef	MNEWS
  "\t^F           next page.(You can specify lines.)",
#else	/* !MNEWS */
  "\tf or ^F      next page.(You can specify lines.)",
#endif	/* !MNEWS */
  "\tSPACE        next page,last then quit.(You can specify lines.)",
  "\tk or ^P      previous line.",
  "\te, j, ^N or RETURN",
  "\t             next line.",
  "\tg or <       top screen.(You can specify line-number.)",
  "\tG or >       last screen.(You can specify line-number.)",
#ifdef	MNEWS
  "\t^U           show previous half page.(You can specify lines.)",
  "\t^D           show next half page.(You can specify lines.)",
  "\tv or V       read article with all header.",
  "\t             ('V' decode MIME header.)",
  "\t^            read referece article.(When NEWS mode only)",
  "\t*            read Message-ID article.(When NEWS mode only)",
  "\n  [MOVE]",
  "\tp            previous unread article.",
  "\tn            next unread article.",
  "\tP            previous article.",
  "\tN            next article.",
  "\n  [MARK]",
  "\tD            mark article and move to previous unread.",
  "\td            mark article and move to next unread.",
  "\tU            unmark article and move to previous.",
  "\tu            unmark article and move to next.",
  "\tK            mark same subject as read.(When NEWS/BOARD mode only)",
  "\n  [POST]",
  "\tf            follow.(When NEWS/BOARD mode only)",
  "\tF            follow with original article.(When NEWS/BOARD mode only)",
  "\n  [SEND]",
  "\tr            reply.",
  "\tR            reply with original article.",
  "\t= or f       forward.(f can use mail mode only)",
#ifdef	LARGE
  "\t~            relay.",
#endif	/* LARGE */
  "\n  [SAVE]",
  "\ts            save article.",
  "\t             (Please input file name.If file exists, please select",
  "\t              y:append n:abort or o:overwrite.)",
#ifdef	MH
  "\tO            save article in MH folder.",
  "\t             (Please Input MH folder name.)",
#endif	/* MH */
  "\n  [SEARCH]",
#else	/* !MNEWS */
  "\tu or ^U      show previous half page.",
  "\td or ^D      show next half page.",
  "\tp            backward next search PATTERN.",
  "\tn            forward next search PATTERN.",
  "\tP            show previous file.",
  "\tN            show next file.",
#endif	/* !MNEWS */
  "\t\\PATTERN     backward search PATTERN.",
  "\t/PATTERN     forward search PATTERN.",
#ifdef	MNEWS
  "\n  [DISPLAY]",
  "\t^L           redisplay screen.",
#ifdef	LARGE
  "\t@            print out.",
#endif	/* LARGE */
#else	/* !MNEWS */
  "\tR, r or ^L   redisplay screen.",
#endif	/* !MNEWS */
  "\t?            show help screen.",
  "\tA            print Japanese no conversion.",
  "\tE            print Japanese EUC code.",
  "\tJ            print Japanese JIS code.",
  "\tS            print Japanese SJIS code.",
#ifndef	MNEWS
  "\t=            show line number.",
#endif	/* !MNEWS */
#ifdef	MNEWS
#ifdef	LARGE
  "\n  [PRINT]",
  "\t@            print out article.",
#endif	/* LARGE */
  "\n  [EXIT]",
#endif	/* MNEWS */
  "\to, q or Q    quit pager.",
  "\t^Z           suspend.",
#endif	/* !SMALL */
  (char*)NULL,
};

/*
 * ファイル参照
 */

int	view_file(file_name, offset)
     char	*file_name;
     int	offset;
{
  return(view_files(&file_name, 1, offset));
}

/*
 * 複数ファイル参照
 */

int	view_files(file_names, number, offset)
     char	*file_names[];
     int	number;
     int	offset;
{
  struct stat	stat_buff;
  char		key_buff[SMALL_BUFF];
  char		search_buff[SMALL_BUFF];
  char		help_file[PATH_BUFF];
  char		guide_buff[BUFF_SIZE];
  char		*file_name;
  char		*error_msg;
  char		**message;
#ifndef	DEBUG
  char		*ptr;
#endif	/* !DEBUG */
  int		key_code;
  int		key_index;
  int		loop;
  int		status;
  int		file;
  int		old_number;
  int		help_mode;
  long		max_position;
  int		reverse_flag;

  last_key = 0;
#ifdef	CTRL_L
  stop_flag = 0;
#endif	/* CTRL_L */
  charset = ROMAN_CHAR;
  if (number >= MAX_SHOW_FILE) {
    print_fatal("Too many files.");
    number = MAX_SHOW_FILE;
  }

  /*
   * ファイルオープン,初期化
   */

  sprintf(help_file, "%s%cmless_H.%d", tmp_dir, SLASH_CHAR, (int)getpid());
  for (loop = file = 0; loop < number + 1; loop++) {
    if (loop == number) {
      file_name = help_file;
      fp = (FILE*)NULL;
    } else {
      file_name = file_names[loop];
#if	defined(MSDOS) || defined(OS2)
      fp = fopen(file_name, "rb");
#else	/* !(MSDOS || OS2) */
      fp = fopen(file_name, "r");
#endif	/* !(MSDOS || OS2) */
      if (fp == (FILE*)NULL) {
	print_fatal("Can't open \"%s\".", file_names[loop]);
	continue;
      }
    }
    file_struct[file].fp              = fp;
    file_struct[file].file_name       = file_name;
    file_struct[file].line_number     = 0;
    file_struct[file].valid_height    = -1;
    file_struct[file].top_position    = 0L;
    file_struct[file].end_position    = 0L;
    file_struct[file].search_position = -1L;
    file_struct[file].charset         = ROMAN_CHAR;
#ifdef	CTRL_L
    file_struct[file].stop_flag       = 0;
#endif	/* CTRL_L */
#ifdef	COLOR
    if ((loop == number) || (!strcmp(guide_message, "HELP"))) {
      file_struct[file].field_position = -2L;
    } else {
      file_struct[file].field_position = -1L;
    }
#endif	/* COLOR */
    file++;
  }
  if (--file <= 0) {
    print_fatal("No valid file.");
    return(-1);
  }

  /*
   * ファイル参照
   */

  number = 0;
  help_mode = 0;
  while (1) {
    old_number = number;
    file_name       = file_struct[number].file_name;
    fp              = file_struct[number].fp;
    line_number     = file_struct[number].line_number;
    valid_height    = file_struct[number].valid_height;
    top_position    = file_struct[number].top_position;
    end_position    = file_struct[number].end_position;
    search_position = file_struct[number].search_position;
    charset         = file_struct[number].charset;
    max_position    = -2L;	/* -1 は end_position と比較するので不可 */
#ifdef	CTRL_L
    stop_flag       = file_struct[number].stop_flag;
#endif	/* CTRL_L */
#ifdef	COLOR
    field_position  = file_struct[number].field_position;
#endif	/* COLOR */
    if ((number == file) && (!file_struct[number].fp)) {
      fp = fopen(file_name, "w");
      if (japanese) {
	message = pager_jmessage;
      } else {
	message = pager_message;
      }
      while (*message) {
	fprintf(fp, "%s\n", *message);
	message++;
      }
#if	defined(MSDOS) || defined(OS2)
      fp = freopen(file_name, "rb", fp);
#else	/* !(MSDOS || OS2) */
      fp = freopen(file_name, "r", fp);
#endif	/* !(MSDOS || OS2) */
      if (!stat(file_name, &stat_buff)) {
	max_position = (long)stat_buff.st_size;
      }
      unlink(file_name);
      if ((file_struct[number].fp = fp) == (FILE*)NULL) {
	number = 0;
	continue;
      }
    }
    error_msg = (char*)NULL;
    if (valid_height < 0) {
      valid_height = 0;
      search_line(offset);
    }
    view_all();
    loop = 1;
    key_index = 0;
    key_buff[0] = search_buff[0] = '\0';
    search_position = -1L;
    while (loop > 0) {
      term_locate(0, term_lines - 1);
      term_clear_end();
      reverse_flag = 1;
      if (error_msg) {
	strcpy(guide_buff, error_msg);
	error_msg = (char*)NULL;
      } else {
	if (key_index > 0) {
	  reverse_flag = 0;
	  strcpy(guide_buff, key_buff);
	} else {
#ifdef	DEBUG
#ifdef	CTRL_L
	  sprintf(guide_buff,
		  "(LN=%d,TP=%d,EP=%d,SF=%d,SP=%d,VH=%d,ST=%d,SB=%s)",
		  line_number, (int)top_position, (int)end_position, stop_flag,
		  (int)search_position, valid_height, status, search_buff);
#else	/* !CTRL_L */
	  sprintf(guide_buff,
		  "(LN=%d,TP=%d,EP=%d,SP=%d,VH=%d,ST=%d,SB=%s)",
		  line_number, (int)top_position, (int)end_position,
		  (int)search_position, valid_height, status, search_buff);
#endif	/* !CTRL_L */
#else	/* !DEBUG */
	  if (line_number <= 0) {
	    count_line();
	  }
	  if (end_position < 0L) {
	    next_line(0);
	  }
	  if (max_position < 0L) {
	    if (!stat(file_name, &stat_buff)) {
	      max_position = (long)stat_buff.st_size;
	    }
	  }
	  if (guide_message[0]) {
	    ptr = guide_buff;
	    *ptr++ = '[';
#ifdef	SJIS_SRC
	    switch (print_code) {
	    case JIS_CODE:
	      sjis_to_jis(ptr, guide_message);
	      break;
	    case EUC_CODE:
	      jis_to_euc(ptr, guide_message);
	      break;
	    case SJIS_CODE:
	    default:
	      strcpy(ptr, guide_message);
	      break;
	    }
#else	/* !SJIS_SRC */
	    switch (print_code) {
	    case JIS_CODE:
	      euc_to_jis(ptr, guide_message);
	      break;
	    case SJIS_CODE:
	      euc_to_sjis(ptr, guide_message);
	      break;
	    case EUC_CODE:
	    default:
	      strcpy(ptr, guide_message);
	      break;
	    }
#endif	/* !SJIS_SRC */
	    while (*ptr) {
	      ptr++;
	    }
	    sprintf(ptr, "] line:%d byte:%ld", line_number, end_position);
	  } else {
	    sprintf(guide_buff, "%s line:%d byte:%ld",
		    file_name, line_number, end_position);
	  }
	  ptr = guide_buff;
	  while (*ptr) {
	    ptr++;
	  }
	  if (max_position > 0L) {
	    sprintf(ptr, "/%ld", (long)max_position);
	    while (*ptr) {
	      ptr++;
	    }
	  }
	  if (feof(fp) || (end_position == max_position)) {
	    strcpy(ptr, " (END)");
	  } else {
	    if (max_position <= 0L) {
	      strcpy(ptr, " (UNKNOWN)");
	    } else {
	      sprintf(ptr, " %d%%", (int)(end_position * 100L / max_position));
	    }
	  }
#endif	/* !DEBUG */
	}
      }
      guide_buff[term_columns] = '\0';
      if (reverse_flag) {
#ifdef	COLOR
	if (color_mode) {
	  term_attrib(color_code[GUIDE_COLOR]);
	} else {
	  term_attrib(REVERSE_ATTRIB);
	}
	printf("%s", guide_buff);
	term_attrib(RESET_ATTRIB);
#else	/* !COLOR */
	term_attrib(REVERSE_ATTRIB);
	printf("%s", guide_buff);
	term_attrib(RESET_ATTRIB);
#endif	/* !COLOR */
      } else {
	printf("%s", guide_buff);
      }
      fflush(stdout);
#ifdef	MNEWS
      last_key = key_code = get_key(MOUSE_MODE_MASK);
#else	/* !MNEWS */
      last_key = key_code = get_key(0);
#endif	/* !MNEWS */
#ifdef	MOUSE
      if (mouse_button == 0) {
        int	line;

	line = mouse_row - 1;
	if (line < term_lines / 2) {
	  last_key = key_code = 0x02;	/* ^B */
	} else {
	  last_key = key_code = 0x06;	/* ^F */
	}
      } else if (mouse_button == 1) {
        last_key = key_code = ' ';
      } else if (mouse_button == 2) {
        last_key = key_code = 'o';
      }
#endif	/* MOUSE */
      if (isdigit(key_code)) {
	if (key_index < 8) {
	  key_buff[key_index++] = key_code;
	  key_buff[key_index] = '\0';
	}
      } else if (((key_code == 0x7f) ||
		  (key_code == 0x08)) && (key_index > 0)) {
	key_buff[--key_index] = '\0';
      } else {
	offset = atoi(key_buff);
	switch (key_code) {
#ifdef	MNEWS
#ifdef	LARGE
	case '@':		/* 印刷			*/
	  print_file(file_name);
	  view_all();
	  break;
#endif	/* LARGE */
#ifdef	SAVE_PAGER
	case 's':		/* セーブ		*/
	  save_file(file_name);
	  view_all();
	  break;
#ifndef	SMALL
	case '*':		/* Message-ID 参照	*/
#endif	/* !SMALL */
	  term_home();
	  term_clear_line();
#else	/* !SAVE_PAGER */
	case 's':		/* セーブ		*/
#ifndef	SMALL
	case '*':		/* Message-ID 参照	*/
#endif	/* !SMALL */
	  term_home();
	  term_clear_line();
#endif	/* !SAVE_PAGER */
#endif	/* MNEWS */
	case 'Q':		/* 終了			*/
	case 'q':
	case 'o':
#ifdef	MNEWS
	case 'p':
	case 'P':
	case 'n':
	case 'N':
	case 'd':
	case 'u':
	case 'y':
	case 'z':
	case 'D':
	case 'U':
	case 'Y':
	case 'Z':
	case 'v':
	case 'V':
	case '^':
	case 'F':
	case 'f':
	case 'R':
	case 'r':
	case '=':
#ifdef	LARGE
	case '~':
#endif	/* LARGE */
#ifndef	SMALL
	case 'K':
#endif	/* !SMALL */
#ifdef	MH
	case 'O':
#endif	/* MH */
#endif	/* MNEWS */
	  if (help_mode) {
	    number = help_mode - 1;
	    help_mode = 0;
	    loop = -1;
	  } else {
	    loop = 0;
	  }
	  break;
#ifndef	MNEWS
	case 'P':		/* 前ファイル参照	*/
	  if (!help_mode) {
	    if (number > 0) {
	      number--;
	      loop = -1;
	    } else {
	      error_msg = "No previous file.";
	    }
	  }
	  break;
	case 'N':		/* 次ファイル参照	*/
	  if (!help_mode) {
	    if (number < (file - 1)) {
	      number++;
	      loop = -1;
	    } else {
	      error_msg = "No next file.";
	    }
	  }
	  break;
#endif	/* !MNEWS */
	case 0x02:		/* ^B 			*/
	case 'b':		/* 前ページ移動		*/
	  if (offset > 0) {
	    status = offset;
	  } else {
	    status = term_lines - 1;
	  }
	  status = prev_line(status);
	  if (status) {
	    if (status >= (term_lines - 1)) {
	      view_all();
	    } else {
	      view_top(status);
	    }
	  }
	  break;
	case ' ':		/* 次ページ移動		*/
	  if (feof(fp) || (end_position == max_position)) {
	    if (help_mode) {
	      number = help_mode - 1;
	      help_mode = 0;
	      loop = -1;
	    } else {
	      if (number < (file - 1)) {
		number++;
		loop = -1;
	      } else {
		loop = 0;
	      }
	    }
	    break;
	  }
	  /* break 不要 */
	case 0x06:		/* ^F			*/
#ifndef	MNEWS
	case 'f':
#endif	/* !MNEWS */
	  if (offset > 0) {
	    status = offset;
	  } else {
	    status = term_lines - 1;
	  }
	  status = view_bottom(status);
	  break;
	case 0x10:		/* ^P			*/
	case 'k':
	  status = prev_line(1);
	  if (status) {
	    view_top(status);
	  }
	  break;
	case 'e':
	case 'j':
	case 0x0e:		/* ^N			*/
	case '\015':		/* RETURN		*/
	case '\n':
	  view_bottom(1);
	  break;
	case 'g':
	case '<':		/* 先頭に移動		*/
	  if (offset > 0) {
	    search_line(offset);
	  } else {
	    top_position = 0L;
	    end_position = -1L;
	    line_number = 1;
	  }
	  view_all();
	  break;
	case 'G':
	case '>':		/* 最後に移動		*/
	  if (offset > 0) {
	    search_line(offset);
	    view_all();
	  } else {
	    top_position = -1L;
	    end_position = max_position;
	    line_number = -1;
	    prev_line(0);
	    view_top(term_lines - 1);
	  }
	  break;
#ifndef	MNEWS
	case 'u':
#endif	/* !MNEWS */
	case 0x15:		/* ^U 前の半ページ	*/
	  if (offset > 0) {
	    status = offset;
	  } else {
	    status = (term_lines - 1) / 2;
	  }
	  status = prev_line(status);
	  if (status) {
	    if (status >= (term_lines - 1)) {
	      view_all();
	    } else {
	      view_top(status);
	    }
	  }
	  break;
#ifndef	MNEWS
	case '=':		/* 行番号表示		*/
	  count_line();
	  break;
	case 'd':
#endif	/* !MNEWS */
	case 0x04:		/* ^D 次の半ページ	*/
	  if (offset > 0) {
	    status = offset;
	  } else {
	    status = (term_lines - 1) / 2;
	  }
	  view_bottom(status);
	  break;
	case 0x1a:		/* ^Z サスペンド	*/
	  suspend();
	  
	  /* break せず ^L のつもりで続いて再描画 */
	  
#ifndef	MNEWS
	case 'R':
	case 'r':
#endif	/* !MNEWS */
	case 0x0c:		/* ^L 再描画		*/
	  view_all();
	  break;
	case '\\':		/* 後方検索		*/
	case '/':		/* 前方検索		*/
	  key_index = 0;
	  key_buff[0] = '\0';
	  while (1) {
	    term_locate(0, term_lines - 1);
	    printf("%c%s", key_code, key_buff);
	    term_clear_end();
#if	defined(MSDOS) || defined(OS2)
	    offset = getch();
#else	/* !(MSDOS || OS2) */
#ifdef	YOUBIN
	    offset = youbin_getchar();
#else	/* !YOUBIN */
	    offset = getchar();
#endif	/* !YOUBIN */
#endif	/* !(MSDOS || OS2) */
	    if (offset == EOF) {
	      break;
	    }
	    if ((offset == '\015') || (offset == '\n')) { /* RETURN	*/
	      break;
	    } else if (((offset == 0x7f) || (offset == 0x08))
		       && (key_index > 0)) {
	      key_buff[--key_index] = '\0';
	      if (key_index <= 0) {
		search_buff[0] = '\0';
		break;
	      }
	    } else if (offset >= ' ') {
	      if (key_index < term_columns - 4) {
		key_buff[key_index++] = offset;
		key_buff[key_index] = '\0';
	      }
	    }
	  }
	  if (key_index > 0) {
	    search_position = -1L;
	    strcpy(search_buff, key_buff);
	  }
	  /* break 不要 */
#ifndef	MNEWS
	case 'p':		/* 前方次検索		*/
	case 'n':		/* 後方次検索		*/
#endif	/* !MNEWS */
	  if (search_buff[0]) {
	    if ((key_code == '/') || (key_code == 'n')) {
	      status = forward_search(search_buff);
	    } else {
	      status = backward_search(search_buff);
	    }
	    if (status) {
	      view_all();
	    } else {
	      error_msg = japanese ? "見つかりませんでした。" :
		"Search failed.";
	    }
	  }
	  break;
	case '?':		/* ヘルプ表示		*/
	  if (!help_mode) {
	    help_mode = number + 1;
	    number = file;
	    loop = -1;
	  } else {
	    error_msg = "Already help mode.";
	  }
	  break;
	case 'A':		/* ASCII モード		*/
	  print_code = ASCII_CODE;
	  view_all();
	  break;
	case 'E':		/* EUC モード		*/
	  print_code = EUC_CODE;
	  view_all();
	  break;
	case 'J':		/* JIS モード		*/
	  print_code = JIS_CODE;
	  view_all();
	  break;
	case 'S':		/* SJIS モード		*/
	  print_code = SJIS_CODE;
	  view_all();
	  break;
	default:
	  break;
	}
	key_index = 0;
	key_buff[0] = '\0';
      }
    }
    if (!loop) {
      break;
    }
    if (old_number != file) {
      file_struct[old_number].line_number     = line_number;
      file_struct[old_number].valid_height    = valid_height;
      file_struct[old_number].top_position    = top_position;
      file_struct[old_number].end_position    = end_position;
      file_struct[old_number].search_position = search_position;
      file_struct[old_number].charset         = charset;
#ifdef	CTRL_L
      file_struct[old_number].stop_flag       = stop_flag;
#endif	/* CTRL_L */
#ifdef	COLOR
      file_struct[old_number].field_position  = field_position;
#endif	/* COLOR */
    }
  }
  term_locate(0, term_lines - 1);
  term_clear_end();
  for (loop = 0; loop <= file; loop++) {
    if (file_struct[loop].fp) {
      fclose(file_struct[loop].fp);
    }
  }
  return(0);
}

/*
 * 前行移動
 */

static int	prev_line(number)
     int	number;
{
  int	status;

  status = 0;
  if (top_position < 0L) {
    if (end_position >= 0L) {
      fseek(fp, end_position, 0);
      prev_position(valid_height);
      top_position = ftell(fp);
    }
  } else {
    fseek(fp, top_position, 0);
  }
#ifdef	LOG_DEBUG
  print_fatal("ENTER prev_line() TOP=%d, END=%d",
	      (int)top_position, (int)end_position);
#endif	/* LOG_DEBUG */
  if (top_position >= 0L) {
    status = prev_position(number);
    if (status) {
      top_position = ftell(fp);
      end_position = -1L;
      line_number = -1;
    }
  } else {
    print_fatal("Program error at prev_line.");
  }
#ifdef	LOG_DEBUG
  print_fatal("RETURN prev_line() TOP=%d, END=%d",
	      (int)top_position, (int)end_position);
#endif	/* LOG_DEBUG */
  return(status);
}

/*
 * 次行移動
 */

static int	next_line(number)
     int	number;
{
  int	status;

  status = 0;
  if (end_position < 0L) {
    if (top_position >= 0L) {
      fseek(fp, top_position, 0);
      next_position(valid_height);
      end_position = ftell(fp);
    }
  } else {
    fseek(fp, end_position, 0);
  }
#ifdef	LOG_DEBUG
  print_fatal("ENTER next_line() TOP=%d, END=%d",
	      (int)top_position, (int)end_position);
#endif	/* LOG_DEBUG */
  if (end_position >= 0L) {
    status = next_position(number);
    if (status) {
      top_position = -1L;
      end_position = ftell(fp);
      line_number = -1;
    }
  } else {
    print_fatal("Program error at next_line.");
  }
#ifdef	LOG_DEBUG
  print_fatal("RETURN next_line() TOP=%d, END=%d",
	      (int)top_position, (int)end_position);
#endif	/* LOG_DEBUG */
  return(status);
}

/*
 * 指定行移動
 */

static int	search_line(number)
     int	number;
{
  char	buff[BUFF_SIZE];
  int	status;
#ifdef	COLOR
  int	color = RESET_ATTRIB;
#endif	/* COLOR */

  status = 0;
  fseek(fp, 0L, 0);
  line_number = 1;
  while (--number > 0) {
    if (fread_line(buff, sizeof(buff), fp)) {
      break;
    }
#ifdef	COLOR
    color = check_field(buff, (long)ftell(fp));
#endif	/* COLOR */
    line_number++;
    status++;
  }
#ifdef	COLOR
  if (color_mode) {
    term_attrib(color);
  }
#endif	/* COLOR */
  top_position = ftell(fp);
  end_position = -1L;
  return(status);
}

/*
 * 行位置取得
 */

static void	count_line()
{
  char	buff[BUFF_SIZE];

  line_number = 0;
  prev_line(0);
  fseek(fp, 0L, 0);
  line_number = 0;
  while (!fread_line(buff, sizeof(buff), fp)) {
    line_number++;
    if (top_position < ftell(fp)) {
      break;
    }
  }
}

/*
 * 前行取得(下位関数)
 */

static int	prev_position(number)
     int	number;
{
  char		buff1[BUFF_SIZE + 1];
  char		buff2[BUFF_SIZE];
#ifdef	notdef
  char		*ptr1 = buff1;
  int		status;
  int		first;
  int		noread;		/* flag for no fread() */
  register int	length1, length2, length3;
  long		last_pos;	/* last position */
  long		pos_save;	/* saved position */
  register long	position;

  status = 0;
  last_pos = position = ftell(fp);
  if (position <= 0L) {
    return(status);
  }

  charset = ROMAN_CHAR;
  noread = 0;
  while (number-- > 0 && position > 0L) {

    /*
     * 直前の行頭を検索する。
     */

    first = 1;
    while (position > 0L) {
      if (noread) {
	length1 = ptr1 - buff1;
	position -= length1;
	noread = 0;
      } else {
	if (position < (sizeof(buff1) - 1)) {
	  length1 = position;
	} else {
	  length1 = sizeof(buff1) - 1;
	}
	position -= length1;
	fseek(fp, position, 0);
	for (length2 = 0; length2 < length1; length2 += length3) {
	  length3 = fread(buff1 + length2, 1, length1 - length2, fp);
	  if (!length3) {
	    break;
	  }
	}
      }
      buff1[length1] = '\0';
      if (first && buff1[length1 - 1] == '\n') {
	buff1[length1 - 1] = '\0';
	last_pos--;
      }
      if ((ptr1 = strrchr(buff1, '\n'))) {
	ptr1++;				/* ptr1: top of line in buff1 */
	position += (ptr1 - buff1);	/* position: top of the line */
	break;
      }
      first = 0;
    }

    /*
     * positon = 行頭位置
     */

    noread = first;	/* OPTIMIZE: use old buffer if "first" */
    if (!noread) {
      fseek(fp, position, 0);
    }

    /*
     * find the position to display
     */

    length3 = 0;
    do {
      ptr1 += length3;

      /* terminal width < 1000 ... I think.... */

      if (!noread) {		/* OPTIMIZE */
	if ((position - last_pos) <=  (sizeof(buff1) - 1)) {
	  length1 = position - last_pos;
	  noread = 1;		/* OPTIMIZE: last read */
	} else {
	  length1 = sizeof(buff1) - 1;
	}
	for (length2 = 0; length2 < length1; length2 += length3) {
	  length3 = fread(buff1 + length2, 1, length1 - length2, fp);
	  if (!length3)
	    break;
        }
	ptr1 = buff1;
	buff1[length1] = '\0';
      }				/* OPTIMIZE */
      pos_save = position;
      length3 = kanji_strposcpy(buff2, ptr1, term_columns);
      position += length3;
    } while (position < last_pos);

    /*
     * OPTIMIZE: If some bytes remain in buff1, reuse them.
     */

    noread = (ptr1 > buff1);

    /*
     * now "position" is the display position
     */

    status++;
    last_pos = position = pos_save;
  }

  /*
   * move
   */

  fseek(fp, position, 0);
#else	/* !notdef */
  char		*ptr1, *ptr2;
  int		status;
  int		first;
  register int	length1, length2, length3;
  register long	position;

  status = 0;
  position = ftell(fp);
  if (position <= 0) {
    return(status);
  }

  /*
   * ここから下の処理は内蔵ページャーで最も難解な部分です。
   * 作者自体後から見る気も起こらないほどに醜いですねぇ。
   */

  charset = ROMAN_CHAR;
  length2 = 0;
  while (number-- > 0) {
#ifdef	LOG_DEBUG
    print_fatal("WHILE prev_position() NUM=%d POSITION=%d",
		number, position);
#endif	/* LOG_DEBUG */
    length1 = 0;
    first = 1;
    while (1) {
      if (!length1) {
	if (position <= 0L) {
	  position = 0L;
	  number = 0;
	  status++;
	  break;
	} else if (position > (long)(sizeof(buff1) - 1)) {
	  length1 = (sizeof(buff1) - 1);
	} else {
	  length1 = position;
	}
	position -= length1;
	fseek(fp, position, 0);
	length2 = 0;
	while (length2 < length1) {
	  length3 = fread(&buff1[length2], 1, length1 - length2, fp);
	  if (!length3) {
	    break;
	  }
	  length2 += length3;
	}
	if (buff1[length1 - 1] != '\n') {
	  buff1[length1] = '\0';
	  first = 0;
	}
      } else if (buff1[length1 - 1] == '\n') {	/* 未初期化参照不具合	*/
	if (first) {
	  buff1[--length1] = '\0';
	  first = 0;
	} else {
	  number++;
	  while (number-- > 0) {
	    length2 = 0;
	    ptr1 = &buff1[length1];
	    status++;
	    while (1) {
	      ptr2 = ptr1;
	      first = kanji_strposcpy(buff2, ptr1, term_columns);
	      ptr1 += first;
	      if (!*ptr1) {
		break;
	      }
	      length2 += first;
	    }
	    *ptr2 = '\0';
	    if (!buff1[length1]) {
	      break;
	    }
	  }
	  position += (length1 + length2);
	  break;
	}
      } else {
	if (--length1 == 0) {
	  if (position == 0L) {
	    number++;
	    while (number-- > 0) {
	      length2 = 0;
	      ptr1 = &buff1[length1];
	      status++;
	      while (1) {
		ptr2 = ptr1;
		first = kanji_strposcpy(buff2, ptr1, term_columns);
		ptr1 += first;
		if (!*ptr1) {
		  break;
		}
		length2 += first;
	      }
	      *ptr2 = '\0';
	      if (!buff1[length1]) {
		break;
	      }
	    }
	    position += (length1 + length2);
	    number = 0;
	    break;
	  }
	}
      }
    }
    fseek(fp, position, 0);
#ifdef	LOG_DEBUG
    print_fatal("WEND  prev_position() NUM=%d POSITION=%d",
		number, (int)position);
#endif	/* LOG_DEBUG */
  }
#endif	/* !notdef */
  return(status);
}

/*
 * 次行取得(下位関数)
 */

static int	next_position(number)
     int	number;
{
  register int	status;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		*ptr;
  long		position;

  status = 0;
  charset = ROMAN_CHAR;
  position = 0L;
  ptr = buff1;
  *ptr = '\0';
  while (number-- > 0) {
    if (!*ptr) {
      position = ftell(fp);
      ptr = buff1;
      if (fread_line(buff1, sizeof(buff1), fp)) {
	return(status);
      }
    }
    status++;
#ifdef	LOG_DEBUG
    print_fatal("WHILE next_position() NUM=%d POSITION=%d",
		number, (int)position);
#endif	/* LOG_DEBUG */
    ptr += kanji_strposcpy(buff2, ptr, term_columns);
    if (!*ptr) {
      position++;
    }
#ifdef	LOG_DEBUG
    print_fatal("WEND  next_position() NUM=%d POSITION=%d",
		number, (int)position);
#endif	/* LOG_DEBUG */
  }
  if (status) {
    fseek(fp, (long)(position + ptr - buff1), 0);
  }
  return(status);
}

/*
 * 前方検索
 */

static int	forward_search(string)
     char	*string;
{
  char	buff[BUFF_SIZE];
  int	status;

  status = 0;
  prev_line(0);
  if (search_position == top_position) {
    next_position(1);
  }
  while (1) {
    if (fread_line(buff, sizeof(buff), fp)) {
      break;
    }
    if (kanji_strindex(buff, string)) {
      prev_position(1);
      top_position = ftell(fp);
      end_position = -1L;
      status = 1;
      break;
    }
  }
  if (status) {
    search_position = top_position;
  } else {
    prev_line(0);
  }
  return(status);
}

/*
 * 後方検索
 */

static int	backward_search(string)
     char	*string;
{
  char	buff[BUFF_SIZE];
  int	status;

  status = 0;
  prev_line(0);
  while (!fgets_prev(buff, fp)) {
    if (kanji_strindex(buff, string)) {
      top_position = ftell(fp);
      end_position = -1L;
      status = 1;
      break;
    }
  }
  if (status) {
    search_position = top_position;
  } else {
    prev_line(0);
  }
  return(status);
}

/*
 * 全体表示
 */

static void	view_all()
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		*ptr1;
#ifdef	CTRL_L
  char		*ptr2;
#endif	/* CTRL_L */
#ifdef	COLOR
  int		color = -1;
#endif	/* COLOR */
  register int	i, j;

  term_clear_all();
#ifdef	CTRL_L
  stop_flag = 0;
#endif	/* CTRL_L */
  ptr1 = buff1;
  *ptr1 = '\0';
  if (top_position < 0L) {
    prev_line(0);
  }
  valid_height = 0;
  charset = ROMAN_CHAR;
  fseek(fp, top_position, 0);
  end_position = top_position;
  for (i = 0; i < term_lines - 1; i++) {
    if (!*ptr1) {
      ptr1 = buff1;
      if (fread_line(buff1, sizeof(buff1), fp)) {
	term_clear_line();
	break;
      }
#ifdef	COLOR
      color = -1;
#endif	/* COLOR */
    }
#ifdef	CTRL_L
    ptr2 = ptr1;
    while (*ptr2) {
      if (*ptr2++ == 0x0c) {		/* ^L 検出	*/
	if (!*ptr2) {
	  stop_flag = valid_height + 1;
	}
	break;
      }
    }
#endif	/* CTRL_L */
    j = kanji_strposcpy(buff2, ptr1, term_columns);
    ptr1 += j;
    end_position += j;
#ifdef	COLOR
    if (color < 0) {
      color = check_field(buff2, end_position);
    }
    if (color_mode) {
      term_attrib(color);
    }
#endif	/* COLOR */
#ifdef	TERMCAP
#ifdef	EUC_ONLY
    printf("%s", buff2);
#else	/* !EUC_ONLY */
#ifdef	JIS_INPUT
    kanji_printf(default_code, "%s", buff2);
#else	/* !JIS_INPUT */
    euc_printf("%s", buff2);
#endif	/* !JIS_INPUT */
#endif	/* !EUC_ONLY */
    term_crlf(kanji_strwidth);
#else	/* !TERMCAP */
#ifdef	EUC_ONLY
    printf("%s\r\n", buff2);
#else	/* !EUC_ONLY */
#ifdef	JIS_INPUT
    kanji_printf(default_code, "%s\r\n", buff2);
#else	/* !JIS_INPUT */
    euc_printf("%s\r\n", buff2);
#endif	/* !JIS_INPUT */
#endif	/* !EUC_ONLY */
#endif	/* !TERMCAP */
    if (!*ptr1) {
      end_position++;
    }
    valid_height++;
#ifdef	CTRL_L
    if (stop_flag) {
      break;
    }
#endif	/* CTRL_L */
#ifdef	COLOR
    if (color_mode) {
      term_attrib(RESET_ATTRIB);
    }
#endif	/* COLOR */
  }
#ifdef	CTRL_L
  if (stop_flag) {
    end_position = -1L;
  }
#endif	/* !CTRL_L */
}

/*
 * 上段表示
 */

static int	view_top(number)
     int	number;
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		*ptr1;
#ifdef	CTRL_L
  char		*ptr2;
#endif	/* CTRL_L */
#ifdef	COLOR
  int		color = -1;
#endif	/* COLOR */
  int		status;
  register int	i;

  status = 0;
  if (top_position < 0L) {
    prev_line(0);
  }
  term_locate(0, 0);
  for (i = 0; i < number; i++) {
    term_insert();
  }
  term_locate(0, 0);
  ptr1 = buff1;
  *ptr1 = '\0';
  fseek(fp, top_position, 0);
  charset = ROMAN_CHAR;
  for (i = 0; i < number; i++) {
#ifdef	TERMCAP
    if (i) {
      term_crlf(kanji_strwidth);
    }
#endif	/* TERMCAP */
    if (valid_height < (term_lines - 1)) {
      valid_height++;
    }
#ifdef	CTRL_L
    if (stop_flag > 0) {
      if (++stop_flag > (term_lines - 1)) {
	stop_flag = 0;
      }
    }
#endif	/* CTRL_L */
    if (!*ptr1) {
      ptr1 = buff1;
      if (fread_line(buff1, sizeof(buff1), fp)) {
	term_clear_line();
#ifdef	TERMCAP
	term_top();
#else	/* !TERMCAP */
	printf("\r");
#endif	/* !TERMCAP */
	continue;
      }
#ifdef	COLOR
      color = -1;
#endif	/* COLOR */
    }
#ifdef	CTRL_L
    ptr2 = ptr1;
    while (*ptr2) {
      if (*ptr2++ == 0x0c) {		/* ^L 検出	*/
	if (!*ptr2) {
	  stop_flag = valid_height + 1;
	}
	break;
      }
    }
#endif	/* CTRL_L */
    ptr1 += kanji_strposcpy(buff2, ptr1, term_columns);
#ifdef	COLOR
    if (color < 0) {
      color = check_field(buff2, (long)ftell(fp));
    }
    if (color_mode) {
      term_attrib(color);
    }
#endif	/* COLOR */
#ifdef	TERMCAP
#ifdef	EUC_ONLY
    printf("%s", buff2);
#else	/* !EUC_ONLY */
#ifdef	JIS_INPUT
    kanji_printf(default_code, "%s", buff2);
#else	/* !JIS_INPUT */
    euc_printf("%s", buff2);
#endif	/* !JIS_INPUT */
#endif	/* !EUC_ONLY */
#else	/* !TERMCAP */
#ifdef	EUC_ONLY
    printf("%s\r\n", buff2);
#else	/* !EUC_ONLY */
#ifdef	JIS_INPUT
    kanji_printf(default_code, "%s\r\n", buff2);
#else	/* !JIS_INPUT */
    euc_printf("%s\r\n", buff2);
#endif	/* !JIS_INPUT */
#endif	/* !EUC_ONLY */
#endif	/* !TERMCAP */
    status++;
  }
#ifdef	COLOR
  if (color_mode) {
    term_attrib(RESET_ATTRIB);
  }
#endif	/* COLOR */
  return(status);
}

/*
 * 下段表示
 */

static int	view_bottom(number)
     int	number;
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		*ptr1;
#ifdef	CTRL_L
  char		*ptr2;
#endif	/* CTRL_L */
#ifdef	COLOR
  int		color = -1;
#endif	/* COLOR */
  int		status;
  register int	i, j;

  ptr1 = buff1;
  *ptr1 = '\0';
  status = 0;
  charset = ROMAN_CHAR;
#ifdef	CTRL_L
  if (stop_flag) {
    line_number = -1;
    if (number < stop_flag) {
      term_locate(0, term_lines - 1);
      term_clear_line();
      if (top_position < 0L) {
	prev_line(0);
      }
      fseek(fp, top_position, 0);
      next_position(number);
      end_position = -1L;
      top_position = ftell(fp);
      for (i = 0; i < number; i++) {
#ifdef	TERMCAP
	term_crlf(0);
#else	/* !TERMCAP */
	printf("\r\n");
#endif	/* !TERMCAP */
	valid_height--;
	stop_flag--;
	status++;
      }
    } else {
      if (end_position < 0L) {
	next_line(0);
      }
      top_position = end_position;
      end_position = -1L;
      view_all();
      status = valid_height;
    }
  } else {
#endif	/* CTRL_L */
    if (end_position < 0L) {
      next_line(0);
    }
    fseek(fp, end_position, 0);
    term_locate(0, valid_height);
    term_clear_line();
    for (i = 0; i < number; i++) {
#ifdef	CTRL_L
      if (stop_flag) {
#ifdef	TERMCAP
	term_crlf(0);
#else	/* !TERMCAP */
	printf("\r\n");
#endif	/* !TERMCAP */
	valid_height--;
	stop_flag--;
	continue;
      }
#endif	/* CTRL_L */
      if (!*ptr1) {
	ptr1 = buff1;
	if (fread_line(buff1, sizeof(buff1), fp)) {
	  if (scroll_mode) {
	    if (!i) {
	      return(status);
	    }
	    *ptr1 = '\0';
	    printf("\r\n");
	    valid_height--;
	    continue;
	  } else {
	    return(status);
	  }
	}
#ifdef	COLOR
	color = -1;
#endif	/* COLOR */
#ifdef	CTRL_L
	ptr2 = ptr1;
	while (*ptr2) {
	  if (*ptr2++ == 0x0c) {	/* ^L 検出	*/
	    if (!*ptr2) {
	      stop_flag = valid_height;
	    }
	    break;
	  }
	}
#endif	/* CTRL_L */
	top_position = -1L;
	line_number = -1;
      }
      j = kanji_strposcpy(buff2, ptr1, term_columns);
      ptr1 += j;
      end_position += j;
#ifdef	COLOR
      if (color < 0) {
	color = check_field(buff2, end_position);
      }
      if (color_mode) {
	term_attrib(color);
      }
#endif	/* COLOR */
#ifdef	TERMCAP
#ifdef	EUC_ONLY
      printf("%s", buff2);
#else	/* !EUC_ONLY */
#ifdef	JIS_INPUT
      kanji_printf(default_code, "%s", buff2);
#else	/* !JIS_INPUT */
      euc_printf("%s", buff2);
#endif	/* !JIS_INPUT */
#endif	/* !EUC_ONLY */
      term_crlf(kanji_strwidth);
#else	/* !TERMCAP */
#ifdef	EUC_ONLY
      printf("%s\r\n", buff2);
#else	/* !EUC_ONLY */
#ifdef	JIS_INPUT
      kanji_printf(default_code, "%s\r\n", buff2);
#else	/* !JIS_INPUT */
      euc_printf("%s\r\n", buff2);
#endif	/* !JIS_INPUT */
#endif	/* !EUC_ONLY */
#endif	/* !TERMCAP */
      if (!*ptr1) {
	end_position++;
      }
      status++;
#ifdef	COLOR
      if (color_mode) {
	term_attrib(RESET_ATTRIB);
      }
#endif	/* COLOR */
    }
#ifdef	CTRL_L
  }
#endif	/* CTRL_L */
  return(status);
}

/*
 * ファイル 1 行入力(行末の LF を取り除く)
 */

static int	fread_line(ptr, size, fp)
     char	*ptr;
     int	size;
     FILE	*fp;
{
  if (!fgets(ptr, size, fp)) {
    *ptr = '\0';
    return(1);
  }
  while (*ptr) {
    if (*ptr == '\n') {
      *ptr = '\0';
      break;
    }
    ptr++;
  }
  return(0);
}

/*
 * ファイル直前の 1 行入力
 */

static int	fgets_prev(ptr, fp)
     char	*ptr;
     FILE	*fp;
{
  char		buff1[BUFF_SIZE + 1];
  int		first;
  register int	length1, length2;
  register long	position;

  position = ftell(fp);
  if (position <= 0) {
    return(1);
  }
  length1 = 0;
  first = 1;
  while (1) {
    if (!length1) {
      if (position <= 0L) {
	position = 0L;
	break;
      } else if (position > (sizeof(buff1) - 1)) {
	length1 = (sizeof(buff1) - 1);
      } else {
	length1 = position;
      }
      position -= length1;
      fseek(fp, position, 0);
      length2 = 0;
      while (length2 < length1) {
	length2 = length2 + fread(&buff1[length2], 1, length1 - length2, fp);
      }
      if (buff1[length1 - 1] != '\n') {
	buff1[length1] = '\0';
	first = 0;
      }
    } else if (buff1[length1] == '\n') {
      if (first) {
	buff1[length1] = '\0';
	first = 0;
	length1--;
      } else {
	length1++;
	position += length1;
	break;
      }
    } else {
      if (--length1 == 0) {
	if (position == 0L) {
	  break;
	}
      }
    }
  }
  fseek(fp, position, 0);
  strcpy(ptr, &buff1[length1]);
  return(0);
}

/*
 * 漢字文字列コピー(表示幅)
 */

static int	kanji_strposcpy(ptr1, ptr2, count)
     unsigned char	*ptr1;
     unsigned char	*ptr2;
     int		count;
{
  unsigned char	*ptr3, *ptr4;
  register int	index;

  index = 0;
  ptr3 = ptr2;
  ptr4 = ptr1;
#if	defined(MSDOS) || defined(OS2)
  count--;	/* MS-DOS で改行のずれを誤魔化す(自信なし)	*/
#endif	/* (MSDOS || OS2) */

  /*
   * この部分は JIS 漢字コードを処理するためにトリッキーな
   * 構造をとっています。
   */

  switch (charset) {
  case KANJI_CHAR:
    strncpy((char*)ptr1, JIS_IN, 3);
    ptr1 += 3;
    ptr4 = (unsigned char*)NULL;
    break;
  case KANA_CHAR:
    strncpy((char*)ptr1, JIS_X0201, 3);
    ptr1 += 3;
    ptr4 = (unsigned char*)NULL;
    break;
  case HOJO_CHAR:
    strncpy((char*)ptr1, JIS_X0212, 4);
    ptr1 += 4;
    ptr4 = (unsigned char*)NULL;
    break;
  case ROMAN_CHAR:
  default:
    break;
  }
  while (count > 0) {
    if (!*ptr2) {
      break;
    }
    if ((!strncmp((char*)ptr2, JIS_IN1, 3))
	|| (!strncmp((char*)ptr2, JIS_IN2, 3))) {
      if (count < 2) {
	break;
      }
      
      /*	漢字指示	*/
      
      charset = KANJI_CHAR;
      strcpy((char*)ptr1, JIS_IN);
      ptr1 += 3;
      ptr2 += 3;
      ptr4 = (unsigned char*)NULL;
    } else if (!strncmp((char*)ptr2, JIS_IN3, 4)) {
      if (count < 2) {
	break;
      }
      
      /*	漢字指示	*/
      
      charset = KANJI_CHAR;
      strcpy((char*)ptr1, JIS_IN);
      ptr1 += 3;
      ptr2 += 4;
      ptr4 = (unsigned char*)NULL;
#ifdef	SUPPORT_X0201
    } else if (!strncmp((char*)ptr2, JIS_X0201, 3)) {

      /*	片仮名指示	*/

      charset = KANA_CHAR;
      strcpy((char*)ptr1, JIS_X0201);
      ptr1 += 3;
      ptr2 += 3;
      ptr4 = (unsigned char*)NULL;
#endif	/* SUPPORT_X0201 */
    } else if (!strncmp((char*)ptr2, JIS_X0212, 4)) {
      if (count < 2) {
	break;
      }
      
      /*	漢字指示	*/
      
      charset = HOJO_CHAR;
      strcpy((char*)ptr1, JIS_X0212);
      ptr1 += 4;
      ptr2 += 4;
      ptr4 = (unsigned char*)NULL;
    } else if ((!strncmp((char*)ptr2, JIS_OUT1, 3)) ||
	       (!strncmp((char*)ptr2, JIS_OUT2, 3))) {

      /*	ローマ字指示	*/

      if (ptr1 != ptr4) {
	if (charset == ROMAN_CHAR) {
	  if (ptr4 != (unsigned char*)NULL) {
	    *ptr1 = '\0';
	    memmove(ptr4 + 3, ptr4, strlen((char*)ptr4));
	    strncpy((char*)ptr4, JIS_IN, 3);
	    ptr1 += 3;
	  }
	}
	strcpy((char*)ptr1, JIS_OUT);
	ptr1 += 3;
      }
      charset = ROMAN_CHAR;
      ptr2 += 3;
      ptr4 = (unsigned char*)NULL;
    } else if (*ptr2 & 0x80) {
#ifdef	SUPPORT_X0201
      if (default_code == SJIS_CODE) {
	if ((*ptr2 >= 0xa0) && (*ptr2 < 0xe0)) {
	  *ptr1++ = *ptr2++;
	  count--;
	  index++;
	} else {
	  if (*(ptr2 + 1) >= ' ') {
	    if (count < 2) {
	      count = 0;
	    } else {
	      *ptr1++ = *ptr2++;
	      *ptr1++ = *ptr2++;
	      count -= 2;
	      index += 2;
	    }
	  } else {
	    *ptr1++ = ' ';
	    ptr2++;
	    count--;
	    index++;
	  }
	}
      } else {
	switch (*ptr2) {
	case EUC_SS2:		/* JIS-X0201 かな	*/
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count--;
	  index++;
	  break;
	case EUC_SS3:		/* JIS-X0212 補助漢字	*/
	  if (*(ptr2 + 1) >= ' ') {
	    if (count < 2) {
	      count = 0;
	    } else {
	      *ptr1++ = *ptr2++;
	      *ptr1++ = *ptr2++;
	      *ptr1++ = *ptr2++;
	      count -= 2;
	      index += 2;
	    }
	  } else {
	    *ptr1++ = ' ';
	    ptr2++;
	    count--;
	    index++;
	  }
	  break;
	default:
	  if (*(ptr2 + 1) >= ' ') {
	    if (count < 2) {
	      count = 0;
	    } else {
	      *ptr1++ = *ptr2++;
	      *ptr1++ = *ptr2++;
	      count -= 2;
	      index += 2;
	    }
	  } else {
	    *ptr1++ = ' ';
	    ptr2++;
	    count--;
	    index++;
	  }
	  break;
	}
      }
#else	/* !SUPPORT_X0201 */
      if (*(ptr2 + 1) >= ' ') {
	if (count < 2) {
	  count = 0;
	} else {
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count -= 2;
	  index += 2;
	}
      } else {
	*ptr1++ = ' ';
	ptr2++;
	count--;
	index++;
      }
#endif	/* !SUPPORT_X0201 */
    } else if (*ptr2 == '\t') {
      do {
	*ptr1++ = ' ';
	index++;
	if (--count <= 0) {
	  break;
	}
      } while (index % 8);
      ptr2++;
#ifdef	DISPLAY_CTRL
#if	defined(MSDOS) || defined(OS2)
    } else if ((*ptr2 < ' ') && (*ptr2 != '\r')) {
				/* コントロールコード検出	*/
#else	/* !(MSDOS || OS2) */
    } else if (*ptr2 < ' ') {	/* コントロールコード検出	*/
#endif	/* !(MSDOS || OS2) */
#else	/* !DISPLAY_CTRL */
    } else if (*ptr2 == 0x0c) {	/* ^L 検出	*/
#endif	/* !DISPLAY_CTRL */
      if (count < 2) {
	count = 0;
      } else {
	*ptr1++ = '^';
	*ptr1++ = '@' + *ptr2++;
	count -= 2;
	index += 2;
      }
    } else {
      switch (charset) {
      case KANJI_CHAR:
      case HOJO_CHAR:
	if (count < 2) {
	  count = 0;
	} else {
	  *ptr1++ = *ptr2++;
	  *ptr1++ = *ptr2++;
	  count -= 2;
	  index += 2;
	}
	break;
      case ROMAN_CHAR:
      case KANA_CHAR:
      default:
	*ptr1++ = *ptr2++;
	count--;
	index++;
	break;
      }
    }
  }
  if (charset != ROMAN_CHAR) {
    if ((!strncmp((char*)ptr2, JIS_OUT1, 3))
	|| (!strncmp((char*)ptr2, JIS_OUT2, 3))) {
      charset = ROMAN_CHAR;
      ptr2 += 3;
    }
    strcpy((char*)ptr1, JIS_OUT);
    ptr1 += 3;
  }
  *ptr1 = '\0';
  kanji_strwidth = index;
  return(ptr2 - ptr3);
}

/*
 * 文字列検索(EUC に変換後比較)
 */

static char	*kanji_strindex(str1 ,str2)
     char	*str1;
     char	*str2;
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  register int	length;

  to_euc(buff1, str1, default_code);
  str1 = buff1;
  to_euc(buff2, str2, default_code);
  str2 = buff2;
  length = strlen(str2);
  while (*str1) {
    if (!strncasecmp(str1, str2, length)) {
      return(str1);
    }
    str1++;
  }
  return((char*)NULL);
}

#if	defined(MNEWS) && defined(SAVE_PAGER)
static int	save_file(view_name)
     char	*view_name;
{
  char		file_name[PATH_BUFF];
  KANJICODE	temp_code;
  int		status;
#ifdef	MSDOS
  char		*ptr;
#endif	/* MSDOS */

#ifdef	MSDOS
  sprintf(file_name, "%s%c", save_dir, SLASH_CHAR);
  ptr = file_name;
  while (*ptr) {
    ptr++;
  }
  strcpy(ptr, select_name);
  while (*ptr) {
    if (*ptr == NEWS_GROUP_SEPARATER) {
      *ptr++ = SLASH_CHAR;
    } else {
      ptr++;
    }
  }
#else	/* !MSDOS */
  sprintf(file_name, "%s%c%s", save_dir, SLASH_CHAR, select_name);
#endif	/* !MSDOS */
  term_home();
  term_clear_line();
  input_line(INPUT_EXPAND_MASK | INPUT_COMP_MASK,
	     "ファイル名を入力して下さい:", "Input file name:", file_name);
  if (!file_name[0]) {
    return(1);
  }
  print_mode_line(japanese ? "セーブ中です。" : "Saving.");
  temp_code = default_code;
  default_code = EUC_CODE;
  status = save_article(-1, view_name, file_name);
  default_code = temp_code;
  return(status);
}
#endif	/* (MNEWS && SAVE_PAGER) */

#if	defined(MNEWS) && defined(LARGE)
static int	print_file(view_name)
     char	*view_name;
{
  FILE		*fp1, *fp2;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  int		status;

  if (!yes_or_no(CARE_YN_MODE, "印刷してよろしいですか?",
		 "Print article?")) {
    return(1);
  }
  status = 1;
  fp1 = fopen(view_name, "r");
  if (fp1 != (FILE*)NULL) {
#ifdef	SIGPIPE
    signal(SIGPIPE, pipe_error);
#endif	/* SIGPIPE */
    fp2 = popen(lpr_command, "w");
    if (fp2 != (FILE*)NULL) {
      status = 0;
      while (fgets(buff1, sizeof(buff1), fp1)) {
	switch (lpr_code) {
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
	  strcpy(buff2, buff1);
	  break;
	}
	if (fputs(buff2, fp2) == EOF) {
	  status = 1;
	}
      }
      if (pclose(fp2)) {
	status = 1;
      }
    }
    fclose(fp1);
#ifdef	SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif	/* SIGPIPE */
  }
  return(status);
}
#endif	/* (MNEWS && LARGE) */

#ifdef	COLOR
/*
 * ヘッダチェック(カラーモード用)
 */

static int	check_field(ptr, position)
     char	*ptr;
     long	position;
{
  static struct color_field {
    char	*field;
    short	length;
    short	attrib;
  } color_field[] = {
    {FROM_FIELD,	sizeof(FROM_FIELD) - 1,		FROM2_COLOR},
    {TO_FIELD,		sizeof(TO_FIELD) - 1,		TO_COLOR},
    {CC_FIELD,		sizeof(CC_FIELD) - 1,		CC_COLOR},
    {DATE_FIELD,	sizeof(DATE_FIELD) - 1,		DATE2_COLOR},
    {SUBJECT_FIELD,	sizeof(SUBJECT_FIELD) - 1,	SUBJECT_COLOR},
    {X_NSUBJ_FIELD,	sizeof(X_NSUBJ_FIELD) - 1,	SUBJECT_COLOR},
    {GROUP_FIELD,	sizeof(GROUP_FIELD) - 1,	GROUP_COLOR},
  };
  static int	last_color;
  register int	i;

  if (!color_mode) {
    return(RESET_ATTRIB);
  }
  if ((field_position < 0L) || (position <= field_position)) {
    if (field_position < -1L) {
      return(RESET_ATTRIB);
    }
    if ((*ptr == ' ') || (*ptr == '\t')) {
      if (last_color) {
	return(last_color);
      }
    } else if (!*ptr) {
      field_position = position;
    } else {
      for (i = 0; i < (sizeof(color_field) / sizeof(struct color_field));
	   i++) {
	if (!strncasecmp(ptr, color_field[i].field, color_field[i].length)) {
	  last_color = color_code[color_field[i].attrib];
	  return(last_color);
	}
      }
    }
    last_color = 0;
  } else {
    while ((*ptr == ' ') || (*ptr == '\t')) {
      ptr++;
    }
    if ((*ptr == '>') || (*ptr == '|')) {
      return(color_code[REFER_COLOR]);
    }
  }
  return(RESET_ATTRIB);
}
#endif	/* COLOR */
#endif	/* PAGER */
