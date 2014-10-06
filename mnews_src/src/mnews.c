/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Main program
 *  File        : mnews.c
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-12/23
 *  Author      : MSR24 宅間 顯
 *
 */

/*
 * Turbo-C でスタックサイズの拡張
 * main の前に入れないと都合が悪いらしいので。
 */

#ifdef	__TURBOC__
unsigned int	_stklen = 0x7FFF;
#endif	/* __TURBOC__ */

#include	"compat.h"
#include	"nntplib.h"
#include	"smtplib.h"
#include	"pop3lib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"termlib.h"
#include	"mimelib.h"
#include	"mnews.h"
#include	"group.h"
#include	"article.h"
#include	"pager.h"
#include	"mh.h"
#include	"mark.h"
#include	"mailsend.h"
#ifdef	MIME
#include	"mimepart.h"
#endif	/* MIME */

#ifdef	JNAMES
int		jnOpen();	/* JNAMES データベースのオープン	*/
int		jnClose();	/* JNAMES データベースのクローズ	*/
unsigned char	*jnFetch();	/* JNAMES データベースの検索		*/

short	jnames;				/* JNAMES 使用フラグ	*/
#endif	/* JNAMES */
#ifdef	MOUSE
#undef	putchar
int		putchar();
int		mouse_button, mouse_column, mouse_row, mouse_off = 0;
#endif	/* MOUSE */

static void	get_comment();		/* コメント取得		*/
#ifdef	LARGE
static int	create_dir();		/* ディレクトリ作成	*/
#endif	/* LARGE */
#ifdef	MIME
static void	split_euc_str();	/* EUC 文字列分割	*/
#endif	/* MIME */

char	*new_mail_string[] = {		/* 新着メール		*/
  "        ",
  "NEW MAIL"
  };
short	head_lines;			/* ヘッダライン行数	*/
short	mode_lines;			/* モードライン行数	*/

static char	prog_name[PATH_BUFF];	/* プログラム名		*/
static FILE	*error_fp = (FILE*)NULL;/* エラーログファイル	*/
static char	*nntp_server_ptr;	/* NNTP サーバ名(FQDN)	*/
static char	*smtp_server_ptr;	/* SMTP サーバ名(FQDN)	*/
static char	*pop3_server_ptr;	/* POP3 サーバ名(FQDN)	*/
static char	write_file[MAX_WRITE_FILE][PATH_BUFF];
					/* 書き込みファイル	*/
static char	emerge_file[PATH_BUFF];	/* 緊急マークファイル	*/

/*
 * グループリスト
 */

GROUP_LIST	*group_list = (GROUP_LIST*)NULL;
BOGUS_LIST	*bogus_list = (BOGUS_LIST*)NULL;

/*
 * パスワードフィールド(ホームディレクトリ取得に使用)
 */

#ifndef	MSDOS
struct passwd		*passwd;
#endif	/* !MSDOS */

char	*month_string[] = {		/* 月名			*/
  "???", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
char	*day_string[] = {		/* 曜日名		*/
  "???", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
char	*day_jstring[] = {	/* 曜日名(日本語)	*/
  "??", "日", "月", "火", "水", "木", "金", "土"
  };

/*
 * マルチフォロー/リプライ関連
 */

int	multi_list[MAX_MULTI_NUMBER];
int	multi_number = 0;

/*
 * 表示ヘッダ
 */

static char	*print_field[] = {
  X_NSUBJ_FIELD,		/* このフィールドは先頭にすること!	*/
  DATE_FIELD,			/* このフィールドは 2 番目にすること!	*/
#ifdef	MIME
  CONTENT_TYPE_FIELD,		/* このフィールドは 3 番目にすること!	*/
  CONTENT_TRANS_FIELD,		/* このフィールドは 4 番目にすること!	*/
  CONTENT_DPOS_FIELD,		/* このフィールドは 5 番目にすること!	*/
#endif	/* MIME */
  FROM_FIELD,
  TO_FIELD,
  APP_TO_FIELD,
  CC_FIELD,
  BCC_FIELD,
  REPLY_FIELD,
  IN_REPLY_FIELD,
  SUBJECT_FIELD,
  GROUP_FIELD,
  FOLLOWUP_FIELD,
  DIST_FIELD,
  ORGAN_FIELD,
};

/*
 * 汎用ヘルプメッセージ
 */

static char	*always_jmessage[] = {
  "\n  [表示]",
  "\t^L              再表示します。",
  "\tw               ワイドモードを切替えます。",
  "\tx               英語/日本語モードを切替えます。",
  "\t?               ヘルプ表示します。",
  "\tA               表示をコード無変換で行ないます。",
  "\tE               表示を EUC コードで行ないます。",
  "\tJ               表示を JIS コードで行ないます。",
  "\tS               表示を SJIS コードで行ないます。",
  "\n  [終了]",
  "\t^Z              サスペンドします。",
  "\tQ               システムを終了します。",
  (char*)NULL,
};

static char	*group_jmessage[] = {
  "\n  [その他]",
#ifdef	MIME
  "\tM               MIME エンコード/デコード処理モードを切替えます。",
#endif	/* MIME */
#ifdef	MOUSE
  "\t`               マウスを ON/OFF します。",
#endif	/* MOUSE */
  (char*)NULL,
};

static char	*subject_jmessage[] = {
#ifndef	SMALL
  "\n  [検索]",
  "\t/               サブジェクトを前方検索します。",
  "\t                (検索したい文字列を入力して下さい)",
  "\t\\               サブジェクトを後方検索します。",
  "\t                (検索したい文字列を入力して下さい)",
#endif	/* !SMALL */
#ifdef	LARGE
  "\t(               記事内容を前方検索します。",
  "\t                (検索したい文字列を入力して下さい)",
  "\t)               記事内容を後方検索します。",
  "\t                (検索したい文字列を入力して下さい)",
  "\n  [印刷]",
  "\t@               記事を印刷します。",
  "\t                (y/n で確認を求めてきます)",
#endif	/* LARGE */
  "\n  [その他]",
#ifndef	SMALL
  "\t|               記事をパイプ実行します。",
  "\t                (パイプ実行するコマンドを入力して下さい)",
#endif	/* !SMALL */
  "\th               記事一覧フォーマットを切替えます。",
#ifdef	MOUSE
  "\t`               マウスを ON/OFF します。",
#endif	/* MOUSE */
  (char*)NULL,
};

static char	*always_message[] = {
#ifndef	SMALL
  "\n  [DISPLAY]",
  "\t^L           redisplay screen.",
  "\tw            toggle wide mode.",
  "\tx            toggle English/Japanese mode.",
  "\t?            show help screen.",
  "\tA            print Japanese no conversion.",
  "\tE            print Japanese EUC code.",
  "\tJ            print Japanese JIS code.",
  "\tS            print Japanese SJIS code.",
  "\n  [EXIT]",
  "\t^Z           suspend.",
  "\tQ            quit system.",
#endif	/* !SMALL */
  (char*)NULL,
};

static char	*group_message[] = {
#ifdef	MIME
  "\n  [ETC]",
  "\tM            change MIME encode/decode mode.",
#endif	/* MIME */
#ifdef	MOUSE
  "\t`            toggle mouse ON/OFF.",
#endif	/* MOUSE */
  (char*)NULL,
};

static char	*subject_message[] = {
#ifndef	SMALL
  "\n  [SEARCH]",
  "\t/            forward search subjects.",
  "\t             (Please input search pattern.)",
  "\t\\            backward search subjects.",
  "\t             (Please input search pattern.)",
#ifdef	LARGE
  "\t(            forward search articles.",
  "\t             (Please input search pattern.)",
  "\t)            backward search articles.",
  "\t             (Please input search pattern.)",
  "\n  [PRINT]",
  "\t@            print out article.",
  "\t             (Please make sure y/n.)",
#endif	/* LARGE */
  "\n  [ETC]",
  "\t|            execute pipe command.",
  "\th            change article summary format.",
#endif	/* !SMALL */
#ifdef	MOUSE
  "\t`            toggle mouse ON/OFF.",
#endif	/* MOUSE */
  (char*)NULL,
};

/*
 * メイン関数
 */

int	main(argc, argv)
     int	argc;
     char	*argv[];
{
  int		i, j;
  NNTP_ERR_CODE	nntp_status;
  SMTP_ERR_CODE	smtp_status;
  POP3_ERR_CODE	pop3_status;
  char		*ptr;
  char		file_name[PATH_BUFF];
  KANJICODE	opt_print_code;
#ifdef	JNAMES
  char		database_file[PATH_BUFF];
#endif	/* JNAMES */
  static struct connect_option {
  char		option;
  char		**server_pptr;
  short		*connect_ptr;
  char		*service_name;
  } connect_option[] = {
    {'D',	&nntp_server_ptr,	&nntp_mode,	"NNTP"},
    {'M',	&smtp_server_ptr,	&smtp_mode,	"SMTP"},
    {'P',	&pop3_server_ptr,	&pop3_mode,	"POP3"},
  };

  nntp_server_ptr = smtp_server_ptr = pop3_server_ptr = (char*)NULL;
  opt_print_code = UNKNOWN_CODE;
  
  ptr = separater(argv[0]);
  if (ptr != (char*)NULL) {
    strcpy(prog_name, ptr + 1);
  } else {
    strcpy(prog_name, argv[0]);
  }
#ifndef	MSDOS
  if (!(passwd = (struct passwd*)getpwuid(geteuid()))) {
    fprintf(stderr, "Can't get password entry.\n");
    exit(1);
  }
#endif	/* !MSDOS */
  
  /*
   * コンフィグレーション
   */

  tcp_init();
  net_config();
  for (i = 1; i < argc; i++) {
    ptr = argv[i];
    if ((*ptr == '-') || (*ptr == '/')) {
      ptr++;
      while (*ptr) {
	for (j = 0;
	     j < (sizeof(connect_option) / sizeof(struct connect_option));
	     j++) {
	  if (connect_option[j].option == *ptr) {
	    ptr++;
	    if (!*ptr) {
	      if (++i >= argc) {
		fprintf(stderr, "%s server not specified.\n",
			connect_option[j].service_name);
		exit(1);
	      }
	      ptr = argv[i];
	    }
	    if (*ptr == '-') {
	      *connect_option[j].connect_ptr =
		!*connect_option[j].connect_ptr;
	      ptr++;
	    } else {
	      *connect_option[j].connect_ptr = 1;
	    }
	    if (*ptr) {
	      *connect_option[j].server_pptr = ptr;
	    }
	    j = -1;
	    break;
	  }
	}
	if (j < 0) {
	  break;
	}
	if (*ptr == 'h') {
	  usage(1);
	  exit(0);
	} else if (*ptr == 'v') {
	  usage(0);
	  exit(0);
	} else if (*ptr++ == 'n') {
	  break;
	}
      }
    }
  }
  nntp_connect_mode = nntp_mode;
  nntp_select_server(nntp_server_ptr);
  i = env_config();
  if (i) {
    fprintf(stderr, "%s : Configuration error.\n", prog_name);
    fflush(stderr);
    if (i > 0) {
      exit(1);
    } else {
      sleep(ERROR_SLEEP);
    }
  }
#ifndef	MIME
  mime_format = 0;
#endif	/* !MIME */
  set_mime_decode_func();
  select_name[0] = jump_name[0] = '\0';

  /*
   * スイッチの解釈
   */
  
  for (i = 1; i < argc; i++) {
    ptr = argv[i];
    if ((*ptr == '-') || (*ptr == '/')) {
      ptr++;
      while (*ptr) {
	for (j = 0;
	     j < (sizeof(connect_option) / sizeof(struct connect_option));
	     j++) {
	  if (connect_option[j].option == *ptr) {
	    *connect_option[j].connect_ptr = 1;
	    ptr++;
	    if (!*ptr) {
	      i++;
	      ptr = argv[i];
	    }
	    if (*ptr == '-') {
	      *connect_option[j].connect_ptr =
		!*connect_option[j].connect_ptr;
	      ptr++;
	    } else {
	      *connect_option[j].connect_ptr = 1;
	    }
	    j = -1;
	    break;
	  }
	}
	if (j < 0) {
	  break;
	}
	if (*ptr == 'n') {
	  ptr++;
	  if (*ptr) {
	    strcpy(jump_name, ptr);
	  } else {
	    if (++i >= argc) {
	      fprintf(stderr, "News group name not specified.\n");
	      exit(1);
	    } else {
	      strcpy(jump_name, argv[i]);
	    }
	  }
	  break;
	}
	switch (*ptr++) {
	case 'a':
	  opt_print_code = ASCII_CODE;
	  break;
	case 'b':
	  bell_mode = !bell_mode;
	  break;
#ifdef	COLOR
	case 'c':
	  color_mode = !color_mode;
	  break;
#endif	/* COLOR */
	case 'e':
	  opt_print_code = EUC_CODE;
	  break;
	case 'j':
	  opt_print_code = JIS_CODE;
	  break;
#ifdef	notdef
	case 'l':
	  news_article_mask = !news_article_mask;
	  break;
#endif	/* notdef */
	case 'm':
	  mail_mode = !mail_mode;
	  break;
	case 'p':
	  pager_mode = !pager_mode;
	  break;
	case 'q':
	  quiet_mode = !quiet_mode;
	  break;
	case 's':
	  opt_print_code = SJIS_CODE;
	  break;
	case 'w':
	  wide_mode = !wide_mode;
	  break;
	case 'x':
	  japanese = !japanese;
	  if (japanese) {
	    x_nsubj_mode = x_nsubj_org;
	  } else {
	    x_nsubj_mode = 0;
	  }
	  break;
	case 'E':
	  error_mode = !error_mode;
	  break;
	case 'G':
	  gnus_mode = !gnus_mode;
	  break;
	case 'L':
	  group_mask = !group_mask;
	  break;
	case 'N':
	  nntp_mode = !nntp_mode;
	  break;
#ifdef	notdef
	case 'T':
	  news_thread_mode = !news_thread_mode;
	  break;
#endif	/* notdef */
	default:
	  fprintf(stderr, "Bad option.\n");
	  exit(1);
	  break;
	}
      }
    } else {
      fprintf(stderr, "Bad option.\n");
      exit(1);
    }
  }

  /*
   * 端末の初期化
   */

  term_disable_abort();	/* should be done before changing tty mode */
#ifdef	COLOR
  if (!color_mode) {
    setup_mono();
  }
#else	/* !COLOR */
  setup_mono();
#endif	/* !COLOR */
  head_lines = HEAD_LINES - wide_mode * 2;
  mode_lines = MODE_LINES - wide_mode * 2;
  if (term_init(0)) {
    fprintf(stderr, "Terminal initialize error.\n");
    exit(1);
  }
  if (term_lines < MODE_LINES + 4) {
    term_init(3);
    fprintf(stderr, "Too narrow screen height.\n");
    exit(1);
  }
  if (term_columns < 80) {
    term_init(3);
    fprintf(stderr, "Too narrow screen width.\n");
    exit(1);
  }
  if (opt_print_code != UNKNOWN_CODE) {
    print_code = opt_print_code;
  }

  /*
   * kill で殺された場合の対応
   */

  for (i = 0; i < MAX_WRITE_FILE; i++) {
    write_file[i][0] = '\0';
  }
  emerge_file[0] = '\0';
#ifdef	SIGTERM
  signal(SIGTERM, force_exit);
#endif	/* SIGTERM */
#ifdef	SIGHUP
  signal(SIGHUP, force_exit);
#endif	/* SIGHUP */
#ifdef	OS2
  signal(SIGBREAK, force_exit);
#endif	/* OS2 */

  if (error_mode) {
    sprintf(file_name, "%s%c%s", dot_dir, SLASH_CHAR, ERROR_FILE);
    error_fp = fopen(file_name, "w");
  }
  nntp_connect_mode = 2;	/* print_title で nntp_noop を防ぐため */
  print_title();
#ifdef	PRINT_TITLE
  i = term_lines / 2 - 2;
  if (i + 4 >= term_lines) {
    i = 1;
  }
  term_locate(0, i);
  if (japanese) {
#ifdef	SJIS_SRC
    sjis_printf("このソフトウェアについて、もし何かトラブルがありましたら\r\n");
    sjis_printf("どうか私まで知らせてください。\r\n\r\n");
    sjis_printf("御要望、御意見、バグ報告は大歓迎です。");
#else	/* !SJIS_SRC */
    euc_printf("このソフトウェアについて、もし何かトラブルがありましたら\r\n");
    euc_printf("どうか私まで知らせてください。\r\n\r\n");
    euc_printf("御要望、御意見、バグ報告は大歓迎です。");
#endif	/* !SJIS_SRC */
  } else {
    printf("If you have any trouble with this software, please let me\r\n");
    printf("know. I will fix your problems in the next release.\r\n\r\n");
    printf("Comments, suggestions, and bug reports are welcome.");
  }
  fflush(stdout);
#endif	/* PRINT_TITLE */
#ifdef	YOUBIN
  youbin_init(get_mail_server());
#endif	/* YOUBIN */
  nntp_connect_mode = nntp_mode;
  nntp_select_server(nntp_server_ptr);
  smtp_connect_mode = smtp_mode;
  smtp_select_server(smtp_server_ptr);
  pop3_connect_mode = pop3_mode;
  pop3_rpop_mode = rpop_mode;
  pop3_select_server(pop3_server_ptr);
#ifdef	notdef
  term_set_raw();
#ifdef	MIPS
  term_set_cbreak();
#endif	/* MIPS */
  term_reset_echo();
  term_reset_nl();
#endif	/* notdef */

  if (mail_mode) {
    nntp_status = NNTP_ERR_RSV;
  } else {
    nntp_status = open_nntp_server(0);
  }
  smtp_status = smtp_open();
  if (smtp_status != SMTP_OK) {
    if (smtp_status != SMTP_ERR_RSV) {
      print_mode_line(japanese ? "SMTP/MTAP サーバと接続できませんでした。" :
		    "Can't connect mail SMTP/MTAP server.");
      term_bell();
      sleep(ERROR_SLEEP);
    }
  } else {
    smtp_close();
  }
  pop3_status = pop3_open();
  if (pop3_status != POP3_OK) {
    if (pop3_status != POP3_ERR_RSV) {
      print_mode_line(japanese ? "POP3/MSPL サーバと接続できませんでした。" :
		    "Can't connect POP3/MSPL server.");
      term_bell();
      sleep(ERROR_SLEEP);
    }
  } else {
    pop3_close();
  }
#ifdef	JNAMES
  if (getenv("JNAMES")) {
    strcpy(database_file, getenv("JNAMES"));
  } else {
    database_file[0] = '\0';
  }
  if ((jnames = jnOpen(database_file, jnames_file)) < 0) {
    if (jnames == -1) {
      print_warning("Can't open jnames data base.");
    }
  }
#endif	/* JNAMES */
  if (nntp_status == NNTP_OK) {
    print_mode_line(japanese ? "ニュースグループリストを取得中です。" :
		    "Retrieving news group list.");
    nntp_list();
    strcpy(file_name, mark_file);
    news_read_initfile(file_name);
    strcpy(mark_file, file_name);
  }
  sprintf(emerge_file, "%s.emg", mark_file);
  while (1) {
    i = news_group_menu();
    if (!jump_name[0]) {
      break;
    }
  }

  /*
   * 終了処理
   */

  if (nntp_status == NNTP_OK) {
    nntp_close();
  }
  if (article_list != (ARTICLE_LIST*)NULL) {
    free(article_list);
  }
#ifndef	DEBUG
  term_clear_all();
#endif	/* !DEBUG */
#ifdef	JNAMES
  if (jnames >= 0) {
    jnClose();
  }
#endif	/* JNAMES */
#ifdef	YOUBIN
  youbin_end();
#endif	/* YOUBIN */
  if ((nntp_status == NNTP_OK) && (i >= 0)) {
    news_save_initfile(mark_file);
  }
  if (error_fp != (FILE*)NULL) {
    fclose(error_fp);
  }
  term_init(3);
  fflush(stdout);
#ifdef	OLD_ANNEX
  sleep(1);
#endif	/* OLD_ANNEX */
  term_attrib(RESET_ATTRIB);
  printf("\n");
  tcp_exit();
  return(0);
}

/*
 * ヘルプ表示(使用方法)
 */

void	help(jmessage, message, mask)
     char	**jmessage;
     char	**message;
     int	mask;		/*
				 * GLOBAL_MODE_MASK :A,E,J,S,x,m,w,^Z
				 * GROUP_MODE_MASK  :M
				 * SUBJECT_MODE_MASK:h
				 */
{
  FILE		*fp;
  char		tmp_file[PATH_BUFF];
  char		**pptr1;
  unsigned char	skip;

  create_temp_name(tmp_file, "H");
  fp = fopen2(tmp_file, "w");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't create output file.");
    return;
  }
  if (japanese) {
    message = jmessage;
  }
  while (*message) {
    skip = (unsigned char)**message;
    if (skip < '\t') {
      message++;
      break;
    }
#ifdef	SJIS_SRC
    sjis_fprintf(fp, "%s\n", *message++);
#else	/* !SJIS_SRC */
    euc_fprintf(fp, "%s\n", *message++);
#endif	/* !SJIS_SRC */
  }
  pptr1 = message;
  if (mask & GLOBAL_MODE_MASK) {
    message = japanese ? always_jmessage : always_message;
    if (skip) {
      message++;
    }
    while (*message) {
#ifdef	SJIS_SRC
      sjis_fprintf(fp, "%s\n", *message++);
#else	/* !SJIS_SRC */
      euc_fprintf(fp, "%s\n", *message++);
#endif	/* !SJIS_SRC */
    }
  }
  if (mask & GROUP_MODE_MASK) {
    message = japanese ? group_jmessage : group_message;
    while (*message) {
#ifdef	SJIS_SRC
      sjis_fprintf(fp, "%s\n", *message++);
#else	/* !SJIS_SRC */
      euc_fprintf(fp, "%s\n", *message++);
#endif	/* !SJIS_SRC */
    }
  }
  if (mask & SUBJECT_MODE_MASK) {
    message = japanese ? subject_jmessage : subject_message;
    while (*message) {
#ifdef	SJIS_SRC
      sjis_fprintf(fp, "%s\n", *message++);
#else	/* !SJIS_SRC */
      euc_fprintf(fp, "%s\n", *message++);
#endif	/* !SJIS_SRC */
    }
  }
  message = pptr1;
  while (*message) {
#ifdef	SJIS_SRC
    sjis_fprintf(fp, "%s\n", *message++);
#else	/* !SJIS_SRC */
    euc_fprintf(fp, "%s\n", *message++);
#endif	/* !SJIS_SRC */
  }
  fclose(fp);
  exec_pager((char*)NULL, tmp_file, 0, "HELP");
  funlink2(tmp_file);
}

/*
 * タイトル表示
 */

void	print_title()
{
#ifdef	TERM_REGION_CLR
  register int	i;
#endif	/* TERM_REGION_CLR */
  int		mode;

  mode = 2;
  if (!mail_mode) {
    if (nntp_noop() == NNTP_OK) {
      mode = nntp_connect_mode;
    }
  }
#ifdef	TERM_REGION_CLR
  term_locate(0, 3 - wide_mode * 2);
  for (i = 0; i < (term_lines - mode_lines); i++) {
    term_clear_line();
    term_down();
  }
#else	/* !TERM_REGION_CLR */
  term_clear_all();
#endif	/* !TERM_REGION_CLR */
  if (!wide_mode) {
#ifdef	COLOR
    term_attrib(color_code[TITLE_COLOR]);
#else	/* !COLOR */
    term_attrib(REVERSE_ATTRIB);
#endif	/* !COLOR */
    print_full_line("Mini News Reader Ver %-13.13s Copyright (C) By A.Takuma [%s][%s][%s]",
		    MNEWS_VERSION,
		    mail_mode ? "MAIL" : nntp_connect_string[mode],
		    smtp_connect_string[smtp_connect_mode],
		    pop3_connect_string[pop3_connect_mode]);
    term_locate(0, 1);
    term_attrib(RESET_ATTRIB);
  }
}

/*
 * モードライン表示(EUC 文字列専用)
 */

#ifdef	DONT_HAVE_DOPRNT
void	print_mode_line(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
{
  char		buff1[SMALL_BUFF];
  char		buff2[SMALL_BUFF];

  sprintf(buff1, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
#ifdef	SJIS_SRC
  sjis_strncpy2(buff2, buff1, term_columns - 1);
#else	/* !SJIS_SRC */
  euc_strncpy2(buff2, buff1, term_columns - 1);
#endif	/* !SJIS_SRC */
#ifdef	COLOR
  term_attrib(color_code[GUIDE_COLOR]);
#else	/* !COLOR */
  term_attrib(REVERSE_ATTRIB);
#endif	/* !COLOR */
  term_locate(term_columns - 2, term_lines - 1);
  term_fill_right();
  term_locate(0, term_lines - 1);
#ifdef	SJIS_SRC
  sjis_printf("%s", buff2);
#else	/* !SJIS_SRC */
  euc_printf("%s", buff2);
#endif	/* !SJIS_SRC */
  term_attrib(RESET_ATTRIB);
  fflush(stdout);
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	print_mode_line(char *fmt, ...)
#else	/* !__STDC__ */
void	print_mode_line(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[SMALL_BUFF];
  char		buff2[SMALL_BUFF];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  vsprintf(buff1, fmt, ap);
#ifdef	SJIS_SRC
  sjis_strncpy2(buff2, buff1, term_columns - 1);
#else	/* !SJIS_SRC */
  euc_strncpy2(buff2, buff1, term_columns - 1);
#endif	/* !SJIS_SRC */
#ifdef	COLOR
  term_attrib(color_code[GUIDE_COLOR]);
#else	/* !COLOR */
  term_attrib(REVERSE_ATTRIB);
#endif	/* !COLOR */
  term_locate(term_columns - 2, term_lines - 1);
  term_fill_right();
  term_locate(0, term_lines - 1);
#ifdef	SJIS_SRC
  sjis_printf("%s", buff2);
#else	/* !SJIS_SRC */
  euc_printf("%s", buff2);
#endif	/* !SJIS_SRC */
  term_attrib(RESET_ATTRIB);
  fflush(stdout);
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * 1 行フル表示(EUC 文字列専用)
 */

#ifdef	DONT_HAVE_DOPRNT
void	print_full_line(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
{
  char		buff1[SMALL_BUFF];
  char		buff2[SMALL_BUFF];

  sprintf(buff1, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
#ifdef	SJIS_SRC
  sjis_strncpy2(buff2, buff1, term_columns);
  sjis_printf("%s", buff2);
#else	/* !SJIS_SRC */
  euc_strncpy2(buff2, buff1, term_columns);
  euc_printf("%s", buff2);
#endif	/* !SJIS_SRC */
  fflush(stdout);
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	print_full_line(char *fmt, ...)
#else	/* !__STDC__ */
void	print_full_line(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[SMALL_BUFF];
  char		buff2[SMALL_BUFF];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  vsprintf(buff1, fmt, ap);
#ifdef	SJIS_SRC
  sjis_strncpy2(buff2, buff1, term_columns);
  sjis_printf("%s", buff2);
#else	/* !SJIS_SRC */
  euc_strncpy2(buff2, buff1, term_columns);
  euc_printf("%s", buff2);
#endif	/* !SJIS_SRC */
  fflush(stdout);
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * 1 行入力(漢字対応版、出力は EUC 固定)
 * このルーチンはまだ JIS-X0201 かな及び補助漢字に対応していません。
 */

void	input_line(mask, ptr1, ptr2, ptr3)
     int	mask;	/*
			 * 0:何もしない(0:以外なら末尾の空白を除去する)
			 * INPUT_SPCCUT_MASK:空白を除去する
			 * INPUT_EXPAND_MASK:~/ を展開/
			 * INPUT_COMP_MASK  :補完機能が働かせる
			 * INPUT_FOLDER_MASK:フォルダとして補完機能が働かせる
			 * INPUT_SHADOW_MASK:入力文字を隠す
			 */
     char	*ptr1;
     char	*ptr2;
     char	*ptr3;	/* 入力バッファ */
{
#ifdef	SMALL
  term_locate(0, 1 - wide_mode);
#ifdef	SJIS_SRC
  sjis_printf(japanese ? ptr1 : ptr2);
#else	/* !SJIS_SRC */
  euc_printf(japanese ? ptr1 : ptr2);
#endif	/* !SJIS_SRC */
  term_clear_line();
  print_mode_line(japanese ? "RETURN:確定" : "RET:ok");
  term_init(1);
  term_locate(strlen(japanese ? ptr1 : ptr2), 1 - wide_mode);
  *ptr3 = '\0';
  fgets(ptr3, SMALL_BUFF, stdin);
  term_init(2);
  while (*ptr3) {
    if (*ptr3 == '\n') {
      *ptr3 = '\0';
      break;
    } else {
      ptr3++;
    }
  }
#else	/* !SMALL */
#ifdef	COMPLETION
  DIR_PTR	*dp;
  DIR		*dir_ptr;
#endif	/* COMPLETION */
  static char	hist[SMALL_BUFF] = "";	/* static にすること */
  char		buff1[SMALL_BUFF];
  char		buff2[SMALL_BUFF];
#ifdef	COMPLETION
  char		buff3[SMALL_BUFF];
#endif	/* COMPLETION */
  char		yank[SMALL_BUFF];
  char		*ptr4;
#ifdef	COMPLETION
  char		*ptr5;
  int		completion;		/* 補完フラグ兼ファイル番号 */
#endif	/* COMPLETION */
  register int	offset, position, length, i;
  int		key, key2;
  int		refresh;
  int		kanji;

#ifdef	COMPLETION
  completion = 0;
#endif	/* COMPLETION */
  strcpy(buff1, ptr3);
  if (mask & (INPUT_FOLDER_MASK | INPUT_COMP_MASK)) {
#ifdef	COMPLETION
    completion = 1;
#endif	/* COMPLETION */
    if (mask & INPUT_FOLDER_MASK) {
      buff1[0] = '\0';
    }
  }
#ifdef	COMPLETION
  buff3[0] = '\0';
#endif	/* COMPLETION */
  offset = strlen(japanese ? ptr1 : ptr2);
  position = length = strlen(buff1);
  yank[0] = '\0';
  refresh = -1;
  while (position >= 0) {
    if (refresh) {
      if (refresh > 0) {
	print_title();
	if (!wide_mode) {
	  term_locate(0, 2);
#ifdef	COLOR
	  term_attrib(color_code[CATEGORY_COLOR]);
#else	/* !COLOR */
	  term_attrib(REVERSE_ATTRIB);
#endif	/* !COLOR */
	  print_full_line("");
	  term_attrib(RESET_ATTRIB);
	}
      }
      term_locate(0, 1 - wide_mode);
#ifdef	SJIS_SRC
      sjis_printf(japanese ? ptr1 : ptr2);
#else	/* !SJIS_SRC */
      euc_printf(japanese ? ptr1 : ptr2);
#endif	/* !SJIS_SRC */
      print_mode_line(japanese ?
		      "^B:後退 ^F:前進 ^A:行頭 ^E:行末 ^D:削除 ^U:クリア ^N,^P:直前入力 RETURN:確定" :
		      "^B:backward ^F:forward ^A:top ^E:end ^D:delete ^U:clear ^N,^P:previous RET:ok");
      refresh = 0;
    }
    buff1[length] = '\0';
    term_locate(offset, 1 - wide_mode);
    if (!(mask & INPUT_SHADOW_MASK)) {
      euc_printf("%s", buff1);
    }
    term_clear_line();
    term_locate(offset + position, 1 - wide_mode);
    key = get_key(LINE_MODE_MASK);
    if ((key == '\t') || (key == ' ')) {	/* SPC or TAB	*/
#ifdef	COMPLETION
      if (completion) {

	/*
	 * ファイル/フォルダ名補完処理
	 */

	if ((buff3[0] == '~') && (buff3[1] == SLASH_CHAR)) {
	  sprintf(buff2, "%s%s", home_dir, &buff3[1]);
	} else {
	  strcpy(buff2, buff3);
	  if (mask & INPUT_FOLDER_MASK) {
	    if (buff3[0] != SLASH_CHAR) {
	      sprintf(buff2, "%s%c%s", ptr3, SLASH_CHAR, buff3);
	    }
	  }
	}
	ptr4 = (char*)strrchr(buff2, (char)SLASH_CHAR);
	if (ptr4 != (char*)NULL) {
	  if (ptr4 == buff2) {
	    buff2[0] = SLASH_CHAR;
	    buff2[1] = '\0';
	  } else {
	    *ptr4 = '\0';
	  }
	} else  {
	  strcpy(buff2, ".");
	}
	ptr4 = (char*)strrchr(buff1, (char)SLASH_CHAR);
	if (ptr4 != (char*)NULL) {
	  ptr4++;
	} else {
	  ptr4 = buff1;
	}
	ptr5 = (char*)strrchr(buff3, (char)SLASH_CHAR);
	if (ptr5 != (char*)NULL) {
	  ptr5++;
	} else {
	  ptr5 = buff3;
	}
	if ((dir_ptr = opendir(buff2)) != (DIR*)NULL) {
	  for (i = 0; i < completion; i++) {
	    if (readdir(dir_ptr) == (DIR_PTR*)NULL) {
	      break;
	    }
	  }
	  while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
	    i++;
	    strcpy(buff2, dp->d_name);
	    if ((mask & INPUT_FOLDER_MASK) &&
		(buff2[strlen(buff2) - 1] == '~')) {
	      continue;
	    }
	    if (strcmp(buff2, ".") && strcmp(buff2, "..")) {
#ifdef	MSDOS
	      if (!strncasecmp(ptr5, buff2, strlen(ptr5))) {
#else	/* !MSDOS */
	      if (!strncmp(ptr5, buff2, strlen(ptr5))) {
#endif	/* !MSDOS */
		completion = i;
		i = 0;
		strcpy(ptr4, buff2);
		length = strlen(buff1);
		if (length > (term_columns - offset - 1)) {
		  length = term_columns - offset - 1;
		}
		position = length;
		break;
	      }
	    }
	  }
	  if (i) {
	    term_bell();
	    completion = 1;
	  }
	  closedir(dir_ptr);
	}
	key = 0;
      } else {
	key = ' ';	/* TAB はスペースと解釈	*/
      }
#else	/* !COMPLETION */
      key = ' ';	/* TAB はスペースと解釈	*/
#endif	/* !COMPLETION */
    }
    switch (key) {
    case 0:		/* ^@			*/
      break;
    case 0x01:		/* ^A			*/
      position = 0;
      break;
    case 0x02:		/* ^B			*/
      if (position > 0) {
	if (buff1[--position] & 0x80) {
	  position--;
	}
      }
      break;
    case 0x04:		/* ^D			*/
      if ((length > 0) && (position < length)) {
	if (buff1[position] & 0x80) {
	  length -= 2;
	  for (i = position; i < length; i++) {
	    buff1[i] = buff1[i + 2];
	  }
	} else {
	  length--;
	  for (i = position; i < length; i++) {
	    buff1[i] = buff1[i + 1];
	  }
	}
      }
      break;
    case 0x05:		/* ^E			*/
      position = length;
      break;
    case 0x06:		/* ^F			*/
      if (position < length) {
	if (buff1[++position] & 0x80) {
	  position++;
	}
      }
      break;
    case 0x07:		/* ^G			*/
      *ptr3 = '\0';
      return;
      /* break 不要 */
    case 0x7f:		/* DEL			*/
    case 0x08:		/* ^H			*/
      if (position > 0) {
	position--;
	if (buff1[position] & 0x80) {
	  position--;
	  for (i = position; i < length; i++) {
	    buff1[i] = buff1[i + 2];
	  }
	  length -= 2;
	} else {
	  for (i = position; i < length; i++) {
	    buff1[i] = buff1[i + 1];
	  }
	  length--;
	}
      }
      break;
    case 0x0b:		/* ^K			*/
      strcpy(yank, &buff1[position]);
      length = position;
      break;
    case 0x0c:		/* ^L 再描画		*/
      term_init(2);
      refresh = 1;
      break;
    case 0x0e:		/* ^N 直前の行と入替	*/
    case 0x10:		/* ^P 直前の行と入替	*/
      strcpy(buff2, buff1);
      strcpy(buff1, hist);
      strcpy(hist, buff2);
      position = length = strlen(buff1);
      break;
    case '\015':	/* RETURN		*/
    case '\n':
      position = -1;
      break;
    case 0x15:		/* ^U			*/
      position = length = 0;
      break;
    case 0x19:		/* ^Y ヤンク		*/
      ptr4 = yank;
      while (*ptr4) {
	if (*ptr4 & 0x80) {
	  if (length < (term_columns - offset - 2)) {
	    length += 2;
	    for (i = length; i > position + 1; i--) {
	      buff1[i] = buff1[i - 2];
	    }
	    buff1[position++] = *ptr4++;
	    buff1[position++] = *ptr4++;
	  } else {
	    break;
	  }
	} else {
	  if (length < (term_columns - offset - 1)) {
	    length++;
	    for (i = length; i > position; i--) {
	      buff1[i] = buff1[i - 1];
	    }
	    buff1[position++] = *ptr4++;
	  } else {
	    break;
	  }
	}
      }
      break;
    case 0x1a:		/* ^Z サスペンド	*/
      suspend();
      refresh = 1;
      break;
    default:		/* 文字挿入		*/
      if ((key >= ' ') && (key <= 0x7f)) {
	if (length < (term_columns - offset - 1)) {
	  length++;
	  for (i = length; i > position; i--) {
	    buff1[i] = buff1[i - 1];
	  }
	  buff1[position++] = key;
	}
      } else {
	kanji = 0;
#ifdef	notdef
	if (key & 0x80) {
	  key2 = get_key(LINE_MODE_MASK);
	  if (key2 & 0x80) {
	    kanji = 1;
	  }
#else	/* !notdef */
	if ((input_code == (int)EUC_CODE) && (key >= 0xa1)) {
	  key2 = get_key(LINE_MODE_MASK);
	  if (key2 >= 0xa1) {
	    kanji = 1;
	  }
	} else if ((input_code == (int)SJIS_CODE) && (key > 0x80)) {
	  key2 = get_key(LINE_MODE_MASK);
	  if (key2 >= 0x40) {
	    kanji = 1;
	  }
#endif	/* !notdef */
	} else if (key == '\033') {
	  if (get_key(LINE_MODE_MASK) == '$') {
	    key = get_key(LINE_MODE_MASK);
	    if ((key == '@') || (key == 'B')) {
	      kanji = 2;
	    } else if (key == '(') {
	      if (get_key(LINE_MODE_MASK) == 'B') {
		kanji = 2;
	      }
	    }
	  }
	}
	while (kanji) {
	  if (kanji == 1) {
	    kanji = 0;
	  } else if (kanji == 2) {
	    key = get_key(LINE_MODE_MASK);
	    if (key == '\033') {
	      if (get_key(LINE_MODE_MASK) == '(') {
		key = get_key(LINE_MODE_MASK);
	      }
	      break;
	    }
	    if (key >= ' ') {
	      key |= 0x80;
	      key2 = get_key(LINE_MODE_MASK);
	      if (key2 >= ' ') {
		key2 |= 0x80;
	      } else {
		break;
	      }
	    } else {
	      break;
	    }
	  }
	  if ((length < (term_columns - offset - 2))
	      && (input_code != (int)ASCII_CODE)) {
	    length += 2;
	    for (i = length; i > position + 1; i--) {
	      buff1[i] = buff1[i - 2];
	    }
	    if ((kanji != 2) && (input_code == (int)SJIS_CODE)) {
	      sprintf(buff2, "%c%c", key, key2);
	      sjis_to_euc(&buff2[4], buff2);
	      buff1[position++] = buff2[4];
	      buff1[position++] = buff2[5];
	    } else {
	      buff1[position++] = key;
	      buff1[position++] = key2;
	    }
	  }
	}
      }
      break;
    }
#ifdef	COMPLETION
    if (key) {
      strncpy(buff3, buff1, length);
      buff3[length] = '\0';
      if (completion) {
	completion = 1;
      }
    }
#endif	/* COMPLETION */
  }
  buff1[length] = '\0';
  strcpy(hist, buff1);
  ptr4 = buff1;
  if (mask & (INPUT_SPCCUT_MASK | INPUT_EXPAND_MASK)) {
    while ((*ptr4 == ' ') || (*ptr4 == '\t') || (*ptr4 == '\n')) {
      ptr4++;
    }
  }
  strcpy(buff2, ptr3);		/* フォルダパス名 */
#ifdef	MSDOS
  if ((mask & INPUT_EXPAND_MASK) && ((!strncmp(ptr4, "~/", 2)) ||
				     (!strncmp(ptr4, "~\\", 2)))) {
#else	/* !MSDOS */
  if ((mask & INPUT_EXPAND_MASK) && (!strncmp(ptr4, "~/", 2))) {
#endif	/* !MSDOS */
    ptr4++;
    sprintf(ptr3, "%s%s", home_dir, ptr4);
  } else {
    strcpy(ptr3, ptr4);
  }
  if (mask & (INPUT_SPCCUT_MASK | INPUT_EXPAND_MASK)) {
    ptr4 = ptr3;
    while ((*ptr4) && (*ptr4 != ' ') && (*ptr4 != '\t') && (*ptr4 != '\n')) {
      ptr4++;
    }
    *ptr4 = '\0';
  }
  term_init(2);
#ifdef	LARGE
  if (mask & INPUT_EXPAND_MASK) {
    strcpy(buff1, ptr3);
    ptr4 = separater(buff1);
    if (ptr4 != (char*)NULL) {
      if (*ptr4 == ':') {
	return;			/* ドライブは create できない */
      }
      *ptr4 = '\0';
      if (create_dir(buff1, 1)) {
	*ptr3 = '\0';
      }
    }
  }
#endif	/* LARGE */
#endif	/* !SMALL */
}

#ifdef	USE_SELECT_ABORT
/*
 * 中断チェック
 */

int	check_abort()
{
  if (term_select()) {
#if	defined(MSDOS) || defined(OS2)
    return(getch() == 0x07);
#else	/* !(MSDOS || OS2) */
#ifdef	YOUBIN
    return(youbin_getchar() == 0x07);
#else	/* !YOUBIN */
    return(getchar() == 0x07);
#endif	/* !YOUBIN */
#endif	/* !(MSDOS || OS2) */
  }
  return(0);
}
#endif	/* USE_SELECT_ABORT */

/*
 * Y/N 確認(EUC 文字列専用)
 *
 * 戻り値:
 *		CARE	FILE		JUMP	MIME	SEND	MARK
 *	0:	No	No		No	No	No	No
 *	1:	Yes	Yes(Append)	Yes	Yes	Yes	Read
 *	2:	---	Overwrite	Quit	---	Edit	Delete
 *	3:	---	---		---	Mime	Mime	---
 *	4:	---	---		---	---	Draft	---
 */

#ifdef	DONT_HAVE_DOPRNT
int	yes_or_no(mode, fmt1, fmt2, arg1, arg2, arg3, arg4, arg5, arg6,
		  arg7, arg8)
     yn_mode	mode;
     char	*fmt1;
     char	*fmt2;
     int	arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
{
  char		buff1[SMALL_BUFF];
  int		refresh;
  int		status;
  int		length;

  status = -1;
  refresh = -1;
  while (status < 0) {
    if (refresh) {
      if (refresh > 0) {
	print_title();
      }
      sprintf(buff1, japanese ? fmt1 : fmt2, arg1, arg2, arg3, arg4,
	      arg5, arg6, arg7, arg8);
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
int	yes_or_no(yn_mode mode, char *fmt1, char *fmt2, ...)
#else	/* !__STDC__ */
int	yes_or_no(mode, fmt1, fmt2, va_alist)
     yn_mode	mode;
     char	*fmt1;
     char	*fmt2;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff1[SMALL_BUFF];
  int		refresh;
  int		status;
  int		length;
  int		key;

#ifdef	__STDC__
  va_start(ap, fmt2);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  status = -1;
  refresh = -1;
  while (status < 0) {
    if (refresh) {
      if (refresh > 0) {
	print_title();
      }
      vsprintf(buff1, japanese ? fmt1 : fmt2, ap);
#endif	/* !DONT_HAVE_DOPRNT */
      switch ((int)mode) {
      case (int)NORMAL_YN_MODE:
      case (int)CARE_YN_MODE:
	strcat(buff1, " [y/n]:");
	break;
      case (int)FILE_YN_MODE:
	strcat(buff1, " [y/n/o(verwrite)]:");
	break;
      case (int)JUMP_YN_MODE:
	strcat(buff1, " [y/n/q(uit)]:");
	break;
      case (int)MIME_YN_MODE:
#ifdef	MIME
	strcat(buff1, " [y/n/m(ime)]:");
#else	/* !MIME */
	strcat(buff1, " [y/n]:");
#endif	/* !MIME */
	break;
      case (int)SEND_YN_MODE:
#ifdef	MIME
#ifdef	LARGE_XXX
	strcat(buff1, " [y/n/e(dit)/m(ime)/d(raft)]:");
#else	/* !LARGE */
	strcat(buff1, " [y/n/e(dit)/m(ime)]:");
#endif	/* !LARGE */
#else	/* !MIME */
#ifdef	LARGE_XXX
	strcat(buff1, " [y/n/e(dit)/d(raft)]:");
#else	/* !LARGE */
	strcat(buff1, " [y/n/e(dit)]:");
#endif	/* !LARGE */
#endif	/* !MIME */
	break;
      case (int)MARK_YN_MODE:
	strcat(buff1, " [y/n/d(elete)]:");
	break;
      default:
	break;
      }
      length = strlen(buff1) + 1;
      if (length > (term_columns - 1)) {
	length = term_columns - 1;
      }
      print_mode_line(buff1);
      term_locate(length, term_lines - 1);
      fflush(stdout);
      refresh = 0;
    }
    key = get_key(YN_MODE_MASK | MOUSE_MODE_MASK);
#ifdef	MOUSE
    if (mouse_button == 0) {
      key = 'y';
    } else if (mouse_button == 1) {
      key = ' ';
    } else if (mouse_button == 2) {
      key = 'n';
    }
#endif	/* MOUSE */
    switch (key) {
    case ' ':		/* SPACE	*/
      if ((mode != CARE_YN_MODE) && (mode != SEND_YN_MODE)) {
	status = 1;
      } else {
	term_bell();
      }
      break;
    case 'y':
    case 'Y':
      status = 1;
      break;
    case '\015':	/* RETURN	*/
    case '\n':
      if ((mode != CARE_YN_MODE) && (mode != SEND_YN_MODE)) {
	status = 0;
      } else {
	term_bell();
      }
      break;
    case 'n':
    case 'N':
    case 0x07:
      status = 0;
      break;
    case 'o':		/* Overwrite	*/
    case 'O':
      if (mode == FILE_YN_MODE) {
	status = 2;
      } else {
	term_bell();
      }
      break;
    case 'q':		/* Quit		*/
    case 'Q':
      if (mode == JUMP_YN_MODE) {
	status = 2;
      } else {
	term_bell();
      }
      break;
    case 'e':		/* Edit		*/
    case 'E':
      if (mode == SEND_YN_MODE) {
	status = 2;
      } else {
	term_bell();
      }
      break;
#ifdef	MIME
    case 'm':		/* Mime		*/
    case 'M':
      if ((mode == MIME_YN_MODE) || (mode == SEND_YN_MODE)) {
	if (mime_format == 2) {
	  mime_format = 1;
	} else {
	  mime_format = 2;
	}
	set_mime_decode_func();
	term_clear_all();
	status = 3;
      } else {
	term_bell();
      }
      break;
#endif	/* MIME */
    case 'd':		/* Draft,Delete	*/
    case 'D':
      if (mode == MARK_YN_MODE) {
	status = 2;
#ifdef	LARGE
      } else if (mode == SEND_YN_MODE) {
	status = 4;
#endif	/* LARGE */
      } else {
	term_bell();
      }
      break;
    case 0x0c:		/* ^L 再描画	*/
      term_init(2);
      refresh = 1;
      break;
    case 0x1a:		/* ^Z サスペンド*/
      suspend();
      refresh = 1;
      break;
    default:
      term_bell();
      break;
    }
  }
#ifndef	DONT_HAVE_DOPRNT
  va_end(ap);
#endif	/* !DONT_HAVE_DOPRNT */
  return(status);
}

#ifndef	SMALL
/*
 * 検索文字列入力
 */

char	*input_search_string(direction)
     int	direction;
{
  static char	search_string[SMALL_BUFF] = "";

  if (!direction) {
    input_line(0, "前方検索したい文字列を入力して下さい:",
	       "Input forward search string:", search_string);
  } else {
    input_line(0, "後方検索したい文字列を入力して下さい:",
	       "Input backward search string:", search_string);
  }
  if (*search_string) {
    print_mode_line(japanese ? "検索中です。(^C:中断)" : "Searching.(^C:abort)");
    return(search_string);
  }
  print_mode_line(japanese ? "検索を中止しました。" : "Search aborted.");
  return((char*)NULL);
}
#endif	/* !SMALL */

#if	defined(NEWSPOST) || defined(MAILSEND)
/*
 * フィールドフォーマット出力
 * 注意.フォーマット文字列は EUC コード専用。
 */

void	field_fprintf(fp, fmt, from_field, subject, message_id,
		      group_name, date_field)
     FILE	*fp;
     char	*fmt;
     char	*from_field;
     char	*subject;
     char	*message_id;
     char	*group_name;
     char	*date_field;
{
  short	year, month, date, day, hour, minute, second;
  char	buff1[BUFF_SIZE];
  char	buff2[BUFF_SIZE];
  char	from[MAX_FIELD_LEN];
  char	real[MAX_FIELD_LEN];
  char	comment[BUFF_SIZE];
  char	zone_name[MAX_FIELD_LEN];
  char	*ptr1, *ptr2, *ptr3, *ptr4;
  int	c;
  int	my_flag;

  get_real_adrs(from_field, buff1);
  strcpy(real, buff1);
  get_comment(from_field, comment);
  my_flag = check_my_adrs(buff1);
  if (japanese) {
#ifdef	JNAMES
    get_jadrs(from, buff1);
#else	/* !JNAMES */
    strcpy(from, buff1);
#endif	/* !JNAMES */
  } else {
    strcpy(from, buff1);
  }
  ptr1 = ptr3 = buff1;
  convert_article_date(date_field, &year, &month, &date, &day, &hour,
		       &minute, &second, zone_name);
  while (*fmt) {
    *ptr1 = '\0';
    switch (*fmt) {
    case '%':
      c = 0;
      fmt++;
      ptr2 = ptr4 = (char*)NULL;
      switch (*fmt) {
      case 'J':			/* 差出人(ロングフォーマット)	*/
	ptr4 = strchr(from, ' ');
	/* break 不要 */
      case 'F':			/* 差出人(ロングフォーマット)	*/
	ptr2 = from;
	c = my_flag;
	break;
      case 'c':			/* 差出人(コメント,MIME 対応)	*/
	if (*comment) {
	  mime_decode_func(buff2, comment, default_code);
	  ptr2 = buff2;
	  c = my_flag;
	  break;
	}
	/* break 不要 */
      case 'j':			/* 差出人(ショートフォーマット)	*/
      case 'f':			/* 差出人(ショートフォーマット)	*/
	strcpy(buff2, from);
	ptr2 = buff2;
	while (*ptr2) {
	  if (*ptr2 == '@') {
	    *ptr2 = '\0';
	    break;
	  }
	  ptr2++;
	}
	ptr2 = buff2;
	if (*fmt == 'j') {
	  ptr4 = strchr(buff2, ' ');
	}
	c = my_flag;
	break;
      case 'R':			/* 差出人(ロングフォーマット)	*/
	ptr2 = real;
	break;
      case 'r':			/* 差出人(ショートフォーマット)	*/
	strcpy(buff2, real);
	ptr2 = buff2;
	while (*ptr2) {
	  if (*ptr2 == '@') {
	    *ptr2 = '\0';
	    break;
	  }
	  ptr2++;
	}
	ptr2 = buff2;
	break;
      case 'N':			/* ニュースグループ名		*/
	ptr2 = group_name;
	break;
      case 'y':			/* 年(西暦下 2 桁)		*/
	sprintf(buff2, "%02d", year % 100);
	ptr2 = buff2;
	break;
      case 'Y':			/* 年(西暦 4 桁)		*/
	sprintf(buff2, "%04d", year);
	ptr2 = buff2;
	break;
      case 'm':			/* 月				*/
	sprintf(buff2, "%02d", month);
	ptr2 = buff2;
	break;
      case 'b':			/* 月名				*/
	sprintf(buff2, "%s", month_string[month]);
	ptr2 = buff2;
	break;
      case 'd':			/* 日				*/
	sprintf(buff2, "%02d", date);
	ptr2 = buff2;
	break;
      case 'h':			/* 時(12 時間制)		*/
	sprintf(buff2, "%02d", hour % 12);
	ptr2 = buff2;
	break;
      case 'H':			/* 時(24 時間制)		*/
	sprintf(buff2, "%02d", hour);
	ptr2 = buff2;
	break;
      case 'M':			/* 分				*/
	sprintf(buff2, "%02d", minute);
	ptr2 = buff2;
	break;
      case 'S':			/* 秒				*/
	sprintf(buff2, "%02d", second);
	ptr2 = buff2;
	break;
      case 'w':			/* 曜日(英語)			*/
	strcpy(buff2, day_string[day]);
	ptr2 = buff2;
	break;
      case 'W':			/* 曜日(漢字)			*/
#ifdef	SJIS_SRC
	sjis_to_euc(buff2, day_jstring[day]);
#else	/* !SJIS_SRC */
	strcpy(buff2, day_jstring[day]);
#endif	/* !SJIS_SRC */
	ptr2 = buff2;
	break;
      case 's':			/* サブジェクト			*/
	mime_decode_func(buff2, subject, default_code);
	ptr2 = buff2;
	break;
      case 'I':			/* メッセージ ID		*/
	ptr2 = message_id;
	break;
      case 'p':			/* AM/PM			*/
	if (hour >= 12) {
	  ptr2 = "PM";
	} else {
	  ptr2 = "AM";
	}
	break;
      case 'P':			/* 午前/午後			*/
	if (hour >= 12) {
	  ptr2 = "午後";
	} else {
	  ptr2 = "午前";
	}
#ifdef	SJIS_SRC
	sjis_to_euc(buff2, ptr2);
	ptr2 = buff2;
#endif	/* SJIS_SRC */
	break;
      case 'z':			/* タイムゾーン			*/
	ptr2 = zone_name;
	break;
      default:			/* その他			*/
	*ptr1++ = '%';
	*ptr1++ = *fmt++;
	continue;
	/* break 不要 */
      }
      fmt++;
      if (c && (*fmt == '{')) {
	ptr2 = (char*)NULL;
      }
      if (ptr2) {
	if (*ptr2) {
	  while (*ptr2) {
	    if (ptr2 == ptr4) {
	      break;
	    }
	    *ptr1++ = *ptr2++;
	  }
	} else {
	  ptr2 = (char*)NULL;
	}
      }
      if (*fmt == '{') {	/* 拡張フォーマット */
	fmt++;
	if (ptr2) {
	  ptr2 = fmt;
	}
	while (*fmt) {
	  if (*fmt == '}') {
	    fmt++;
	    break;
	  } else if (*fmt == ',') {
	    fmt++;
	    if (!ptr2) {
	      ptr2 = fmt;
	    }
	  } else if (*fmt++ == '\\') {
	    if (*fmt) {
	      fmt++;
	    }
	  }
	}
	if (ptr2) {
	  while (*ptr2) {
	    if ((*ptr2 == '}') || (*ptr2 == ',')) {
	      break;
	    } else if (*ptr2 == '\\') {
	      ptr2++;
	      if (isdigit(*ptr2)) {
		c = atoi(ptr2);
		while (isdigit(*ptr2)) {
		  ptr2++;
		}
		if (strlen(ptr3) < c) {
		  if (*ptr2) {
		    ptr2++;
		  }
		  continue;
		}
	      }
	      switch (*ptr2) {
	      case 'n':
		*ptr1++ = '\n';
		ptr3 = ptr1;
		ptr2++;
		break;
	      case 't':
		*ptr1++ = '\t';
		ptr2++;
		break;
	      default:
		*ptr1++ = *ptr2++;
		/* break 不要 */
	      case '\0':
		break;
	      }
	    } else {
	      *ptr1++ = *ptr2++;
	    }
	  }
	}
      }
      break;
    case '\\':
      fmt++;
      if (isdigit(*fmt)) {
	c = atoi(fmt);
	while (isdigit(*fmt)) {
	  fmt++;
	}
	if (strlen(ptr3) < c) {
	  if (*fmt) {
	    fmt++;
	  }
	  break;
	}
      }
      switch (*fmt) {
      case 'n':
	*ptr1++ = '\n';
	ptr3 = ptr1;
	fmt++;
	break;
      case 't':
	*ptr1++ = '\t';
	fmt++;
	break;
      default:
	*ptr1++ = *fmt++;
	/* break 不要 */
      case '\0':
	break;
      }
      break;
    default:
      *ptr1++ = *fmt++;
      break;
    }
  }
  *ptr1 = '\0';
  edit_fprintf(fp, "%s", buff1);
}

/*
 * エディタ用漢字文字列出力
 * 注意.フォーマット文字列は SHIFT-JIS は使用不可。
 */

#ifdef	DONT_HAVE_DOPRNT
void	edit_fprintf(fp, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
     FILE	*fp;
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
{
  char	buff1[BUFF_SIZE];
  char	buff2[BUFF_SIZE];
  
  sprintf(buff1, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  switch (edit_code) {
  case JIS_CODE:
    to_jis(buff2, buff1, EUC_CODE);
    break;
  case SJIS_CODE:
    to_sjis(buff2, buff1, EUC_CODE);
    break;
  case EUC_CODE:
    to_euc(buff2, buff1, EUC_CODE);
    break;
  default:
    strcpy(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	edit_fprintf(FILE *fp, char *fmt, ...)
#else	/* !__STDC__ */
void	edit_fprintf(fp, fmt, va_alist)
     FILE	*fp;
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char	buff1[BUFF_SIZE];
  char	buff2[BUFF_SIZE];
  
#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  vsprintf(buff1, fmt, ap);
  switch (edit_code) {
  case JIS_CODE:
    to_jis(buff2, buff1, EUC_CODE);
    break;
  case SJIS_CODE:
    to_sjis(buff2, buff1, EUC_CODE);
    break;
  case EUC_CODE:
    to_euc(buff2, buff1, EUC_CODE);
    break;
  default:
    strcpy(buff2, buff1);
    break;
  }
  fprintf(fp, "%s", buff2);
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */
#endif	/* (NEWSPOST || MAILSEND) */

/*
 * サスペンド
 */

void	suspend()
{
  term_init(1);
  term_locate(0, term_lines - 1);
  fflush(stdout);
#ifdef	MSDOS
  system("");
#else	/* !MSDOS */
#ifdef	SIGTSTP
  kill(0, SIGTSTP);
#else	/* !SIGTSTP */
#ifdef	SIGSTOP
  kill(getpid(), SIGSTOP);
#else	/* !SIGSTOP */
  term_system((char*)NULL);
#endif	/* !SIGSTOP */
#endif	/* !SIGTSTP */
  sleep(1);
#endif	/* !MSDOS */
  term_init(2);
}

/*
 * 警告エラー表示
 */

#ifdef	DONT_HAVE_DOPRNT
void	print_warning(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
{
  char		buff[SMALL_BUFF];
#ifdef	COLOR
  short		color;
#endif	/* COLOR */

  if (!quiet_mode) {
    sprintf(buff, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
#ifdef	COLOR
    color = color_code[GUIDE_COLOR];
    color_code[GUIDE_COLOR] = color_code[ERROR_COLOR];
#endif	/* COLOR */
    print_mode_line("%s", buff);
#ifdef	COLOR
    color_code[GUIDE_COLOR] = color;
#endif	/* COLOR */
    if (error_fp != (FILE*)NULL) {
      fprintf(error_fp, "%s : Warning : ", prog_name);
      fprintf(error_fp, "%s\n", buff);
      fflush(error_fp);
    }
  }
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	print_warning(char *fmt, ...)
#else	/* !__STDC__ */
void	print_warning(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff[SMALL_BUFF];
#ifdef	COLOR
  short		color;
#endif	/* COLOR */

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  if (!quiet_mode) {
    vsprintf(buff, fmt, ap);
#ifdef	COLOR
    color = color_code[GUIDE_COLOR];
    color_code[GUIDE_COLOR] = color_code[ERROR_COLOR];
#endif	/* COLOR */
    print_mode_line("%s", buff);
#ifdef	COLOR
    color_code[GUIDE_COLOR] = color;
#endif	/* COLOR */
    if (error_fp != (FILE*)NULL) {
      fprintf(error_fp, "%s : Warning : ", prog_name);
      fprintf(error_fp, "%s\n", buff);
      fflush(error_fp);
    }
  }
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * 致命的エラー表示
 */

#ifdef	DONT_HAVE_DOPRNT
void	print_fatal(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
{
  char		buff[SMALL_BUFF];
#ifdef	COLOR
  short		color;
#endif	/* COLOR */

  sprintf(buff, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
#ifdef	COLOR
  color = color_code[GUIDE_COLOR];
  color_code[GUIDE_COLOR] = color_code[ERROR_COLOR];
#endif	/* COLOR */
  print_mode_line("%s", buff);
#ifdef	COLOR
  color_code[GUIDE_COLOR] = color;
#endif	/* COLOR */
  if (error_fp != (FILE*)NULL) {
    fprintf(error_fp, "%s : ", prog_name);
    fprintf(error_fp, "%s\n", buff);
    fflush(error_fp);
  }
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	print_fatal(char *fmt, ...)
#else	/* !__STDC__ */
void	print_fatal(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff[SMALL_BUFF];
#ifdef	COLOR
  short		color;
#endif	/* COLOR */

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  vsprintf(buff, fmt, ap);
#ifdef	COLOR
  color = color_code[GUIDE_COLOR];
  color_code[GUIDE_COLOR] = color_code[ERROR_COLOR];
#endif	/* COLOR */
  print_mode_line("%s", buff);
#ifdef	COLOR
  color_code[GUIDE_COLOR] = color;
#endif	/* COLOR */
  if (error_fp != (FILE*)NULL) {
    fprintf(error_fp, "%s : ", prog_name);
    fprintf(error_fp, "%s\n", buff);
    fflush(error_fp);
  }
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * 文字列検索
 */

char	*strindex(str1 ,str2)
     char	*str1;
     char	*str2;
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

/*
 * ファイルから 1 行入力(最後の行に改行がついていなければ付加する)
 */

int	fgets2(ptr, size, fp)
     char	*ptr;
     int	size;
     FILE	*fp;
{
  register int	code;
  register int	status;
  register int	first;

  status = 1;
  first = 1;
  if (feof(fp)) {
    status = 0;
  } else {
    while (size > 0) {
      switch (code = fgetc(fp)) {
      case 0x1A:	/* ^Z */
      case EOF:
	if (first) {
	  status = 0;
	} else {
	  *ptr++ = '\n';
	}
	size = 0;
	break;
      case '\n':
	*ptr++ = code;
	size = 0;
	break;
      case '\r':	/* バイナリモードでテキストファイルをリード時無視 */
	break;
      default:
	*ptr++ = code;
	size--;
      }
      first = 0;
    }
  }
  *ptr = '\0';
  return(status);
}

/*
 * 漢字コード変換
 * 戻り値(負数:エラー,0:正常終了,正数:正常終了(表示済み))
 */

int	convert_code(file1, file2, code, mode)
     char	*file1;
     char	*file2;
     KANJICODE	code;
     int	mode;		/*
				 * 0:無変換
				 * 1:ヘッダ削除+コード変換(MIME 処理)
				 * 2:コード変換(MIME 処理)
				 * 3:コード変換(MIME 非処理)
				 * 4:コード変換(MIME TEXT のみ処理)
				 */
{
  FILE		*fp1, *fp2;
  short		year, month, date, day, hour, minute, second;
  char		buff1[ALIAS_BUFF];
  char		buff2[ALIAS_BUFF];
  char		buff3[ALIAS_BUFF];
  char		*ptr1, *ptr2, *ptr3;
#ifdef	MIME
  char		*ptr4, *ptr5;
  char		boundary[SMALL_BUFF];
  char		file_name[PATH_BUFF];
  MIME_MTYPE	mime_mtype;
  MIME_STYPE	mime_stype;
  MIME_TTYPE	mime_ttype;
  char		c;
  int		status;
#endif	/* MIME */
  int		mode2;
  register int	i, j;

  fp1 = fopen(file1, "r");
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't read input file.");
    return(-1);
  }
  fp2 = fopen2(file2, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't create output file.");
    fclose(fp1);
    return(-1);
  }
  mode2 = mode;

  /*
   * ヘッダ部分の変換
   */

 MULTI:
  mode = mode2;		/* 一見無駄だが goto してくるので必要!	*/
  j = -1;
  buff3[0] = '\0';
#ifdef	MIME
  mime_mtype = M_MTYPE_UNKNOWN;
  mime_stype = M_STYPE_UNKNOWN;
  mime_ttype = M_TTYPE_UNKNOWN;
  boundary[0] = '\0';
  file_name[0] = '\0';
#endif	/* MIME */
  while (1) {
    if (fgets(buff1, sizeof(buff1), fp1) == (char*)NULL) {
      buff1[0] = '\0';	/* ヘッダ中に終端に達したらセパレータとみなす	*/
    }
    if (mode) {
      ptr1 = (char*)NULL;
      if ((buff1[0] == ' ') || (buff1[0] == '\t')) {
	if (j < 0) {
	  continue;
	}
	if ((int)(strlen(buff3) + strlen(buff1)) < (sizeof(buff3) - 1)) {
	  strcat(buff3, buff1);
	} else {
	  j = -1;	/* バッファオーバー時は継続行をカット	*/
	}
      } else {
	if (buff3[0]) {
	  ptr1 = buff2;
	  switch (j) {
	  case 0:				/* X-Nsubject:		*/
	    if (x_nsubj_mode) {
	      recover_jis(buff2, buff3);
	    } else {
	      ptr1 = (char*)NULL;
	    }
	    break;
	  case 1:				/* Date:		*/
	    if (jst_mode) {

	      /*
	       * Date: を GMT →JST 変換してフォーマットを統一して表示
	       */

	      convert_article_date(&buff3[sizeof(DATE_FIELD) - 1],
				   &year, &month, &date, &day,
				   &hour, &minute, &second, buff2);
	      strcpy(buff3, buff2);
	      sprintf(buff2, "%s %s, %d %s %04d %02d:%02d:%02d %s\n",
		      DATE_FIELD, day_string[day], date, month_string[month],
		      year, hour, minute, second, buff3);
	    } else {
	      strcpy(buff2, buff3);
	    }
	    break;
	  case 2:				/* Content-Type:	*/
#ifdef	MIME
	    if (mime_format) {
	      ptr3 = mime_get_type(buff3 + strlen(CONTENT_TYPE_FIELD),
				   &mime_mtype, &mime_stype);
	      if (ptr3 != (char*)NULL) {
		if ((mode == 4) && (mime_mtype != M_MTYPE_TEXT)) {
		  mime_mtype = M_MTYPE_UNKNOWN;
		}
		if (mime_mtype == M_MTYPE_MULTI) {	/* multipart	*/
		  if (!mime_get_param(ptr3, MIME_BOUNDARY, boundary)) {
		    fclose(fp1);
		    fclose(fp2);
		    mime_part_menu(file1, boundary, -1);
		    return(1);
		  }
		} else if (mime_mtype == M_MTYPE_APP) {	/* application	*/
		  mime_get_param(ptr3, MIME_NAME, file_name);
#ifdef	notdef
		} else if ((mime_mtype == M_MTYPE_MSG) &&
			   (mime_stype == M_STYPE_PART)) {
						/* message/partial	*/
		  mime_mtype = M_MTYPE_UNKNOWN;
		  mime_stype = M_STYPE_UNKNOWN;
		  while (fgets(buff1, sizeof(buff1), fp1)) {
		    if ((!buff1[0]) || (buff1[0] == '\n')) {
		      break;
		    }
		  }
		  fseek(fp2, 0L, 0);
		  j = -1;
		  buff3[0] = '\0';
		  continue;
#endif	/* notdef */
		}
	      }
	    }
	    if (mode == 1) {
	      ptr1 = (char*)NULL;
	    } else {
	      strcpy(buff2, buff3);
	    }
#else	/* !MIME */
	    ptr1 = (char*)NULL;
#endif	/* !MIME */
	    break;
	  case 3:			/* Content-Transfer-Encoding:	*/
#ifdef	MIME
	    if (mime_format) {
	      mime_get_ttype(buff3 + strlen(CONTENT_TRANS_FIELD),
			     &mime_ttype);
	    }
	    if (mode == 1) {
	      ptr1 = (char*)NULL;
	    } else {
	      strcpy(buff2, buff3);
	    }
#else	/* !MIME */
	    ptr1 = (char*)NULL;
#endif	/* !MIME */
	    break;
	  case 4:				/* Content-Disposition:	*/
#ifdef	MIME
	    if (mime_format) {
	      mime_get_param(buff3 + strlen(CONTENT_DPOS_FIELD),
			     MIME_FILENAME, file_name);
	    }
	    if (mode == 1) {
	      ptr1 = (char*)NULL;
	    } else {
	      strcpy(buff2, buff3);
	    }
#else	/* !MIME */
	    ptr1 = (char*)NULL;
#endif	/* !MIME */
	    break;
	  default:
#ifdef	MIME
	    if (mime_format && (mode != 3)) {
	      i = 0;
	      ptr2 = buff3;
	      ptr3 = buff2;
	      ptr5 = ptr3;
	      while (*ptr2) {
		ptr4 = ptr2;
		while (*ptr4) {
		  if (*ptr4 == '\n') {
		    *ptr4++ = '\0';
		    while ((*ptr4 == '\t') || (*ptr4 == ' ')) {
		      ptr4++;
		    }
		    break;
		  } else {
		    ptr4++;
		  }
		}
		i |= mime_decode(ptr2, ptr3);
		if (kanji_strlen(ptr5) >= MAX_FIELD_COLUMN) {
		  if (ptr3 != buff2) {
		    *ptr3++ = '\n';
		    ptr5 = ptr3;
		    mime_decode(ptr2, ptr3);
		  }
		}
		while (*ptr3) {
		  ptr3++;
		}
		ptr2 = ptr4;
	      }
	      if (i) {			/* エンコードフィールドの場合	*/
		*ptr3++ = '\n';
		*ptr3 = '\0';
	      } else {			/* プレーンなフィールドの場合	*/
		ptr4 = buff3;
		ptr3 = buff2;
		while (ptr2 != ptr4) {
		  if (*ptr4 == '\0') {
		    *ptr3++ = '\n';
		    ptr4++;
		  } else {
		    *ptr3++ = *ptr4++;
		  }
		}
		*ptr3 = '\0';
	      }
	    } else {
	      strcpy(buff2, buff3);
	    }
#else	/* !MIME */
	    strcpy(buff2, buff3);
#endif	/* !MIME */
	    break;
	  }
	}
	if ((!buff1[0]) || (buff1[0] == '\n')) {
	  mode = -1;
	  if (ptr1 != (char*)NULL) {
	    strcat(ptr1, buff1);
	  } else {
	    ptr1 = buff1;
	  }
	} else if (!strcmp(buff1, MULTI_END_MARK)) {
	  strcpy(buff1, "\n");
	  mode = -2;
	  if (ptr1 != (char*)NULL) {
	    strcat(ptr1, buff1);
	  } else {
	    ptr1 = buff1;
	  }
	} else {
	  if (mode == 1) {
	    j = -1;
	  } else {
	    j = 999;
	  }
	  for (i = 0; i < sizeof(print_field) / sizeof(char*); i++) {
	    if (!strncasecmp(buff1, print_field[i],
			     strlen(print_field[i]))) {
	      j = i;
	    }
	  }
	  if (j < 0) {
	    buff3[0] = '\0';
	  } else {
	    strcpy(buff3, buff1);
	  }
	}
      }
    } else {
      if ((!buff1[0]) || (buff1[0] == '\n')) {
	mode = -1;
      }
      ptr1 = buff1;
    }
    if (ptr1 != (char*)NULL) {
      if (ptr1 == buff1) {
	ptr2 = buff2;
      } else {
	ptr2 = buff1;
      }
      switch (code) {
      case JIS_CODE:
	to_jis(ptr2, ptr1, default_code);
	break;
      case SJIS_CODE:
	to_sjis(ptr2, ptr1, default_code);
	break;
      case EUC_CODE:
	to_euc(ptr2, ptr1, default_code);
	break;
      default:
	ptr2 = ptr1;
	break;
      }
      fputs(ptr2, fp2);
    }
    if (mode == -2) {		/* 本文がないマルチマーク記事	*/
      fputs(MULTI_END_MARK, fp2);
      goto MULTI;
    } else if (mode < 0) {
      break;
    }
  }

  /*
   * 本文部分の変換
   */

#ifdef	MIME
  if ((mime_mtype != M_MTYPE_UNKNOWN) && (mime_mtype != M_MTYPE_TEXT)) {
    status = mime_manage(fp1, mime_mtype, mime_stype, mime_ttype, file_name);
    if (status) {
      fclose(fp1);
      fclose(fp2);
      return(status);
    }
    mime_ttype = M_TTYPE_UNKNOWN;
  }
#endif	/* MIME */
  buff2[0] = '\0';
  while (fgets(buff1, sizeof(buff1), fp1)) {
    strcpy(buff3, buff2);
    ptr1 = buff3;
    while (*ptr1) {
      ptr1++;
    }
#ifdef	MIME
    switch (mime_ttype) {
    case M_TTYPE_QP:	/* quoted-printable	*/
      mime_decodeQP(buff1, ptr1);
      split_euc_str(buff3);
      break;
    case M_TTYPE_B64:	/* base64		*/
      mime_decodeB64(buff1, ptr1);
      split_euc_str(buff3);
      break;
    default:		/* 7/8bit,binary,etc.	*/
      strcpy(ptr1, buff1);
      break;
    }
#else	/* !MIME */
    strcpy(ptr1, buff1);
#endif	/* !MIME */
    ptr1 = buff3;
    if (!strcmp(ptr1, MULTI_END_MARK)) {
      fputs(ptr1, fp2);
      goto MULTI;
    }
    while (*ptr1) {
      ptr2 = strchr(ptr1, '\n');
      if (ptr2 != (char*)NULL) {
	ptr2++;
	c = *ptr2;
	*ptr2 = '\0';
	ptr3 = buff2;
	switch (code) {
	case JIS_CODE:
	  to_jis(ptr3, ptr1, default_code);
	  break;
	case SJIS_CODE:
	  to_sjis(ptr3, ptr1, default_code);
	  break;
	case EUC_CODE:
	  to_euc(ptr3, ptr1, default_code);
	  break;
	default:
	  ptr3 = ptr1;
	  break;
	}
	fputs(ptr3, fp2);
	*ptr2 = c;
	ptr1 = ptr2;
      } else {
	break;
      }
    }
    strcpy(buff2, ptr1);
  }
  fclose(fp1);
  fclose(fp2);
  return(0);
}

#ifdef	MIME
/*
 * 長い EUC 文字列を強制的に改行する。
 */

static void	split_euc_str(ptr)
     char	*ptr;
{
  register int	length;

  length = 0;
  while (*ptr) {
    if (length >= BUFF_SIZE - MAX_ENCODE_COLUMN) {
      memcpy(ptr + 1, ptr, strlen(ptr) + 1);
      *ptr = '\n';
      break;
    }
    if (*ptr & 0x80) {
      if (*((unsigned char*)ptr) == EUC_SS3) {
	ptr += 3;
	length += 3;
      } else {
	ptr += 2;
	length += 2;
      }
    } else {
      ptr++;
      length++;
    }
  }
}
#endif	/* MIME */

/*
 * 記事日付フォーマット変換
 */

void	convert_article_date(dates, year, month, date, day,
			     hour, minute, second, zone_name)
     char	*dates;
     short	*year;
     short	*month;
     short	*date;
     short	*day;
     short	*hour;
     short	*minute;
     short	*second;
     char	*zone_name;
{
  register short	i;
  int			offset;
  int			jst_flag;

  *year   = 0;
  *month  = 0;
  *date   = 0;
  *day    = 0;
  *hour   = 0;
  *minute = 0;
  *second = 0;

  while ((*dates == ' ') || (*dates == '\t')) {
    dates++;
  }
  if (!*dates) {
    return;
  }
  for (i = 0; i < sizeof(day_string) / sizeof(char*); i++) {
    if (!strncasecmp(dates, day_string[i], 3)) {
      *day = i;
      i = -1;
      break;
    }
  }
  if (i < 0) {
    dates = next_param(dates);
  }
  for (i = 0; i < sizeof(month_string) / sizeof(char*); i++) {
    if (!strncasecmp(dates, month_string[i], 3)) {
      *month = i;		/* 月	*/
      i = -1;
      break;
    }
  }
  if (i < 0) {			/* (Date,) Month Day Time Year Zone */	
    dates = next_param(dates);
    *date = atoi(dates);	/* 日	*/
    while (*dates) {
      if (*dates++ == ' ') {
	break;
      }
    }
    *hour = atoi(dates);	/* 時	*/
    while (*dates) {
      if (*dates++ == ':') {
	break;
      }
    }
    *minute = atoi(dates);	/* 分	*/
    while (isdigit(*dates)) {
      dates++;
    }
    if (*dates == ':') {
      dates++;
      *second = atoi(dates);	/* 秒	*/
    }
    dates = next_param(dates);
    *year = atoi(dates);	/* 年	*/
    if ((*year > 0) && (*year < 1000)) {
      *year += 1900;
    }
    dates = next_param(dates);
  } else {			/* Date, Day Month Year Time Zone */
    *date = atoi(dates);	/* 日	*/
    dates = next_param(dates);
    for (i = 0; i < sizeof(month_string) / sizeof(char*); i++) {
      if (!strncasecmp(dates, month_string[i], 3)) {
	*month = i;		/* 月	*/
	break;
      }
    }
    dates = next_param(dates);
    *year = atoi(dates);	/* 年	*/
    if ((*year > 0) && (*year < 100)) {
      *year += 1900;
    }
    dates = next_param(dates);
    *hour = atoi(dates);	/* 時	*/
    while (*dates) {
      if (*dates++ == ':') {
	break;
      }
    }
    *minute = atoi(dates);	/* 分	*/
    while (isdigit(*dates)) {
      dates++;
    }
    if (*dates == ':') {
      dates++;
      *second = atoi(dates);	/* 秒	*/
    }
    dates = next_param(dates);
  }
  strcpy(zone_name, dates);
  jst_flag = 0;
  if (!strncmp(dates, "GMT", 3)) {
    dates += 3;
  }
  if (*dates == '+') {
    dates++;
    offset = atoi(dates);
    if (jst_mode) {
      *hour   -= offset / 100;
      *minute -= offset % 100;
    }
    jst_flag = 1;
  } else if (*dates == '-') {
    dates++;
    offset = atoi(dates);
    if (jst_mode) {
      *hour   += offset / 100;
      *minute += offset % 100;
    }
    jst_flag = 1;
  } else if (*dates < ' ') {
    jst_flag = 1;
  }
  if (jst_flag) {
    strcpy(zone_name, "JST");
    gmt_to_jst(year, month, date, day, hour, minute, second);
  }
  dates = zone_name;
  while (*dates) {
    if (*dates == '\n') {
      *dates = '\0';
    }
    dates++;
  }
}

/*
 * GMT 時間の JST 時間変換
 */

int	gmt_to_jst(year, month, date, day, hour, minute, second)
     short	*year;
     short	*month;
     short	*date;
     short	*day;
     short	*hour;
     short	*minute;
     short	*second;
{
#ifdef	notdef
#if	defined(SUNOS_41) || defined(SUNOS_40) || defined(SUNOS_3X)
  struct tm	t1;
  struct tm	*t2;
  TIMEVALUE	gmt_time;
  
  if (!jst_mode) {
    return(0);
  }
  t1.tm_sec = *second;
  t1.tm_min = *minute;
  t1.tm_hour = *hour;
  t1.tm_year = *year - 1900;
  t1.tm_mon = *month - 1;
  t1.tm_mday = *date;
  t1.tm_wday = 0;
  t1.tm_yday = 0;
  t1.tm_isdst = 0;
  t1.tm_zone = 0;
  t1.tm_gmtoff = 0;
  gmt_time = timegm(&t1);
  t2 = localtime(&gmt_time);
  if (t2 != (struct tm*)NULL) {
    *year = t2->tm_year + 1900;
    *month = t2->tm_mon + 1;
    *date = t2->tm_mday;
    *hour = t2->tm_hour;
    *minute = t2->tm_min;
    *second = t2->tm_sec;
    *day = t2->tm_wday + 1;
  }
#else	/* !(SUNOS_41 || SUNOS_40 || SUNOS_3X) */
  if (!jst_mode) {
    return(0);
  }
  if ((*hour = *hour + JST_OFFSET) >= 24) {
    *hour = *hour - 24;
    *date = *date + 1;
    switch (*month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
      if (*date > 31) {
	*month = 2;
	*date = 1;
      }
      break;
    case 2:
      if ((((*year % 4) == 0) && (*year % 100)) || ((*year % 400) == 0)) {
	if (*date > 29) {
	  *month = 3;
	  *date =1;
	}
      } else if (*date > 28) {
	*month = 3;
	*date =1;
      }
      break;
    case 4:
    case 6:
    case 9:
    case 11:
      if (*date > 30) {
	*month = 2;
	*date = 1;
      }
      break;
    case 12:
      if (*date > 31) {
	*year = *year + 1;
	*month = 1;
	*date = 1;
      }
      break;
    default:
      print_fatal("Illegal GMT time.");
      break;
    }
  }
#endif	/* !(SUNOS_41 || SUNOS_40 || SUNOS_3X) */
#else	/* !notdef */
  struct tm	*t1;
  time_t	gmt_time, date_time;
  int		i;
  
  if (!jst_mode) {
    return(0);
  }
  i = (*year - 1) / 4 - 492;
  date_time = 60L * 60L * 24L;
  gmt_time = (*year - 1970) * (date_time * 365L) + i * date_time;
  for (i = 1; i < *month; i++) {
    switch (i) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      gmt_time += 31L * (60L * 60L * 24L);
      break;
    case 2:
      if ((((*year % 4) == 0) && (*year % 100)) || ((*year % 400) == 0)) {
	gmt_time += 29L * date_time;
      } else {
	gmt_time += 28L * date_time;
      }
      break;
    case 4:
    case 6:
    case 9:
    case 11:
      gmt_time += 30L * date_time;
      break;
    default:
      print_fatal("Illegal GMT time.");
      break;
    }
  }
  gmt_time += (*date - 1) * date_time + *hour * (60L * 60L)
    + *minute * 60 + *second;
  t1 = localtime(&gmt_time);
  if (t1 != (struct tm*)NULL) {
    *year = t1->tm_year + 1900;
    *month = t1->tm_mon + 1;
    *date = t1->tm_mday;
    *hour = t1->tm_hour;
    *minute = t1->tm_min;
    *second = t1->tm_sec;
    *day = t1->tm_wday + 1;
  }
#endif	/* !notdef */
  return(1);
}

/*
 * 次のパラメータ取得
 */

char	*next_param(ptr)
     char	*ptr;
{
  while (*ptr) {
    if ((*ptr == ' ') || (*ptr =='\t')) {
      while ((*ptr == ' ') || (*ptr =='\t')) {
	ptr++;
      }
      break;
    }
    ptr++;
  }
  return(ptr);
}

/*
 * コメント取得
 */

static void	get_comment(str1, str2)
     char	*str1;
     char	*str2;
{
  int	status;
  char	*ptr1, *ptr2, *ptr3, *ptr4;

  status = 1;
  ptr1 = str1;
  ptr2 = str2;
  ptr3 = (char*)NULL;
  while (status) {
    switch (*ptr1) {
    case '(':
      ptr1++;
      status = 1;
      while (status) {
	if (!(*ptr1)) {
	  break;
	}
	switch (*ptr2++ = *ptr1++) {
	case '(':
	  status++;
	  break;
	case ')':
	  if (!--status) {
	    ptr2--;
	  }
	  break;
	default:
	  break;
	}
      }
      if (status) {
	print_warning("Unexpected bracket found in mail address.");
      }
      status = 0;
      break;
    case '<':
      ptr3 = ptr1;
      status = 0;
      break;
    case '\"':
      ptr1++;
      while (*ptr1) {
	if (*ptr1++ == '\"') {
	  break;
	}
      }
      break;
    case ',':
    case '\n':
    case '\0':
      status = 0;
      break;
    case ' ':
    case '\t':
      ptr1++;
      break;
    default:
      ptr1++;
      break;
    }
  }
  if (ptr3) {
    ptr1 = str1;
    ptr4 = (char*)NULL;
    while (ptr1 != ptr3) {
      if ((*ptr1 == ' ') || (*ptr1 == '\t')) {
	if (!ptr4) {
	  ptr4 = ptr2;
	}
      } else {
	ptr4 = (char*)NULL;
      }
      *ptr2++ = *ptr1++;
    }
    if (ptr4) {
      ptr2 = ptr4;
    }
  }
  *ptr2 = '\0';
}

/*
 * 日本語アドレス取得
 */

int	get_jadrs(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
{
#ifdef	JNAMES
  char	buff[BUFF_SIZE];
  char	*ptr3, *ptr4;

  if ((jnames >= 0) && japanese) {
    ptr3 = (char*)jnFetch(FROM_JN_DOMAIN, ptr2);
    if (ptr3 == (char*)NULL) {
      ptr4 = strchr(ptr2, '@');
      if (ptr4 != (char*)NULL) {
	if (strindex(ptr4, domain_name)) {
	  strcpy(buff, ptr2);
	  ptr4 = buff;
	  while (*ptr4) {
	    if (*ptr4 == '@') {
	      strcpy(ptr4 + 1, domain_name);
	      break;
	    }
	    ptr4++;
	  }
	  ptr3 = (char*)jnFetch(FROM_JN_DOMAIN, buff);
	  if (ptr3 == (char*)NULL) {
	    *ptr4 = '\0';
	    ptr3 = (char*)jnFetch(FROM_JN_DOMAIN, buff);
	  }
	}
      } else {
	sprintf(buff, "%s@%s", ptr2, domain_name);
	ptr3 = (char*)jnFetch(FROM_JN_DOMAIN, buff);
      }
    }
    if (ptr3 != (char*)NULL) {
      strcpy(ptr1, ptr3);
      return(1);
    } else {
      strcpy(ptr1, ptr2);
    }
  } else {
    strcpy(ptr1, ptr2);
  }
#else	/* !JNAMES */
  strcpy(ptr1, ptr2);
#endif	/* !JNAMES */
  return(0);
}

/*
 * 日本語名取得
 */

void	get_jname(domain, ptr1, number)
     char	*domain;
     char	*ptr1;
     int	number;
{
#ifdef	JNAMES
  char	*ptr2;
#ifdef	SJIS_SRC
  char	buff[SMALL_BUFF];
#endif	/* SJIS_SRC */

  if ((jnames >= 0) && japanese) {
    ptr2 = (char*)jnFetch(domain, ptr1);
    if (ptr2) {
#ifdef	SJIS_SRC
      euc_to_sjis(buff, ptr2);
      sjis_strncpy(ptr1, buff, number);
#else	/* !SJIS_SRC */
      euc_strncpy(ptr1, ptr2, number);
#endif	/* !SJIS_SRC */
    } else {
      ptr1[number] = '\0';
    }
  } else {
    ptr1[number] = '\0';
  }
#else	/* !JNAMES */
  ptr1[number] = '\0';
#endif	/* !JNAMES */
}

/*
 * 自アドレスチェック
 */

int	check_my_adrs(real_adrs)
     char	*real_adrs;
{
  char	from_buff[MAX_FIELD_LEN];
#ifdef	LARGE
  char	*ptr;
#endif	/* LARGE */

  sprintf(from_buff, "%s@%s", user_name, domain_name);
  if (!strcmp(from_buff, real_adrs)) {
    return(1);
  }
#ifdef	LARGE
  ptr = second_adrs;
  while (*ptr) {
    ptr = get_real_adrs(ptr, from_buff);
    if (!strcmp(from_buff, real_adrs)) {
      return(1);
    }
  }
#endif	/* LARGE */
  return(0);
}

#ifdef	LARGE

/*
 * ディレクトリ作成
 */

static int	create_dir(dir_name, mode)
     char	*dir_name;
     int	mode;
{
  struct stat	stat_buff;
  char		buff[PATH_BUFF];
  char		*ptr;
  int		status;

  if (stat(dir_name, &stat_buff)) {
    if (mode) {
      if (!yes_or_no(CARE_YN_MODE, "\"%s\"を作成してよろしいですか?",
		     "Create \"%s\"?", dir_name)) {
	return(1);
      }
    }
    status = 0;
    strcpy(buff, dir_name);
    ptr = separater(buff);
    if (ptr != (char*)NULL) {
      if (*ptr == ':') {
	status = 1;			/* ドライブは create できない */
      } else {
	*ptr = '\0';
	status = create_dir(buff, 0);
      }
    }
    if (!status) {
      status = mkdir(dir_name, S_IREAD | S_IWRITE | S_IEXEC);
    }
    if (mode && status) {
      print_mode_line(japanese ? "\"%s\"が作成できませんでした。" :
		      "Can't create \"%s\".", dir_name);
      term_bell();
      sleep(ERROR_SLEEP);
    }
    return(status);
  }
  return(0);
}
#endif	/* LARGE */

/*
 * 1 文字キー入力
 */

int	get_key(mask)
     int	mask;		/*
				 * GLOBAL_MODE_MASK :A,E,J,S,x,m,w,^Z
				 * GROUP_MODE_MASK  :M
				 * SUBJECT_MODE_MASK:h
				 * LINE_MODE_MASK   :1行入力用
				 * YN_MODE_MASK     :Y/N 確認用
				 * NEWS_MODE_MASK   :mail_send に select_name
				 *                   を渡さない
				 * MOUSE_MODE_MASK  :マウス対応
				 */
{
  static int	escape = 0;	/* static 型であること */
#ifdef	MOUSE
  static int	pressed_button = -1;
  static int	button, column, mouse = 0;
#endif	/* MOUSE */
  register int	key;

#ifdef	MOUSE
  if (mouse_off) {
    mask &= ~MOUSE_MODE_MASK;
  }
  if (mask & MOUSE_MODE_MASK) {
    mouse_button = -1;
    term_enable_mouse();
  }
#endif	/* MOUSE */
  do {
#if	defined(MSDOS) || defined(OS2)
    key = getch();
#else	/* !(MSDOS || OS2) */
#ifdef	YOUBIN
    key = youbin_getchar();
#else	/* !YOUBIN */
    key = getchar();
#endif	/* !YOUBIN */
#endif	/* !(MSDOS || OS2) */
  } while ((key == '\021') || (key == '\023') ||	/* XON, XOFF	*/
	   ((key == EOF) && (errno == EINTR)));		/* signal	*/
  if (key == EOF) {
    force_exit();
    exit(1);
  }
#ifdef	MOUSE
  if (mouse) {
    switch (mouse) {
    case 1:
      button = key - ' ';
      mouse++;
      break;
    case 2:
      column = key - ' ';
      mouse++;
      break;
    case 3:
      mouse_button = button;
      mouse_column = column;
      mouse_row = key - ' ';
      mouse = 0;
      break;
    }
    key = '[';
  }
#endif	/* MOUSE */
  switch (escape) {
  case 0:
    if (key == 0x1b) {	/* ESC			*/
      escape = 1;
    }
    break;
  case 1:
    if ((key == '[') || (key == 'O')) {
      key = '[';	/* O はキーバインドで使うので [ に変換 */
      escape = 2;
    } else {
      escape = 0;
    }
    break;
  case 2:
    switch (key) {
#ifdef	MOUSE
    case 'M':
      mouse = 1;
      key = '[';
      break;
#endif	/* MOUSE */
    case 'A':
    case 'a':
      if (mask & LINE_MODE_MASK) {
	key = '\020';	/* ^P	*/
      } else if (!(mask & YN_MODE_MASK)) {
	key = 'k';
      }
      break;
    case 'B':
    case 'b':
      if (mask & LINE_MODE_MASK) {
	key = '\016';	/* ^N	*/
      } else if (!(mask & YN_MODE_MASK)) {
	key = 'j';
      }
      break;
    case 'C':
    case 'c':
      if (mask & LINE_MODE_MASK) {
	key = '\015';	/* RETURN	*/
      } else if (mask & YN_MODE_MASK) {
	key = 'y';
      } else {
	key = 'i';
      }
      break;
    case 'D':
    case 'd':
      if (mask & LINE_MODE_MASK) {
	key = '\025';	/* ^U		*/
      } else if (mask & YN_MODE_MASK) {
	key = 'n';
      } else {
	key = 'o';
      }
      break;
    default:
      break;
    }
    /* break 不要 */
  default:
    escape = 0;
    break;
  }
  if (!escape) {
    if (mask & GLOBAL_MODE_MASK) {
      switch (key) {
      case 'A':		/* ASCII モード		*/
	print_code = ASCII_CODE;
	key = 0x0c;
	break;
      case 'E':		/* EUC モード		*/
	print_code = EUC_CODE;
	key = 0x0c;
	break;
      case 'J':		/* JIS モード		*/
	print_code = JIS_CODE;
	key = 0x0c;
	break;
      case 'S':		/* SJIS モード		*/
	print_code = SJIS_CODE;
	key = 0x0c;
	break;
      case 'x':		/* 英語/日本語モード	*/
	japanese = !japanese;
	if (japanese) {
	  x_nsubj_mode = x_nsubj_org;
	} else {
	  x_nsubj_mode = 0;
	}
	key = 0x0c;
	break;
#ifdef	MAILSEND
      case 'm':		/* メール送信		*/
	if (mask & NEWS_MODE_MASK) {
	  mail_send("", "");
	} else {
	  mail_send("", select_name);
	}
	key = 0x0c;
	break;
#endif	/* MAILSEND */
      case 'w':		/* ワイドモード		*/
	wide_mode = !wide_mode;
	head_lines = HEAD_LINES - wide_mode * 2;
	mode_lines = MODE_LINES - wide_mode * 2;
	key = 0x0c;
	break;
      case 0x1a:	/* ^Z サスペンド	*/
	suspend();
	key = 0x0c;
	break;
      default:
	break;
      }
    }
#ifdef	MIME
    if (mask & GROUP_MODE_MASK) {
      if (key == 'M') {	/* MIMEフォーマット処理	*/
	mime_format = (mime_format + 1) % 3;
	set_mime_decode_func();
	key = 0x0c;
      }
    }
#endif	/* MIME */

    if (mask & SUBJECT_MODE_MASK) {
      if (key == 'h') {	/* 記事一覧フォーマット	*/
	article_format = (article_format + 1) % 3;
	key = 0x0c;
      }
    }
  }

#ifdef	MOUSE
  if (!(mask & LINE_MODE_MASK) && (key == '`')) {	/* マウス ON/OFF */
    mouse_off = !mouse_off;
    key = 0x0c;
  }
  if (mask & MOUSE_MODE_MASK) {
    if (mouse_button != -1) {
      if (mouse_button != 3) {	/* button pressed */
	pressed_button = mouse_button;
	mouse_button = -1;
      } else {			/* button released */
	mouse_button = pressed_button;
	pressed_button = -1;
      }
    }
    term_disable_mouse();
  }
#endif	/* MOUSE */
  return(key);
}

/*
 * 再描画位置計算
 */

int	get_top_position(top_position, current_position, max_position,
			 thread_mode, ptr)
     int	top_position;
     int	current_position;
     int	max_position;
     short	thread_mode;
     char	*ptr;
{
  static char	sort_mode_char[] = {'S', 'R', 'D'};	/* ソートモード	*/
  register int	i, j;

  i = term_lines - mode_lines;
  j = ((current_position - i / 4) / (i / 2)) * (i / 2);
  if ((current_position < top_position) ||
      (current_position >= (top_position + i)) || (top_position < 0)) {
    top_position = j;
  }
  if (max_position > 0) {
    if ((i = ((top_position + i - 1) * 100) / max_position) >= 100) {
      if (top_position == 0) {
	strcpy(ptr, "All");
      } else {
	strcpy(ptr, "Bot");
      }
    } else {
      if (top_position == 0) {
	strcpy(ptr, "Top");
      } else {
	sprintf(ptr, "%2d%%", i);
      }
    }
  } else {
    strcpy(ptr, "---");
  }
  ptr += 3;
  *ptr++ = '[';
  *ptr++ = kanji_code_string[(int)print_code][0];
  *ptr++ = ':';
  *ptr++ = mime_form_char;
  *ptr++ = ':';
  *ptr++ = thread_mode ? sort_mode_char[sort_rule] : '-';
  *ptr++ = ']';
  *ptr = '\0';
  return(top_position);
}

/*
 * 新着メールチェック
 */

int	check_new_mail()
{
#if	defined(MH) || defined(RMAIL) || defined(UCBMAIL)
  static int	new_mail = 0;
#ifdef	YOUBIN
  extern int	have_mail;
#endif	/* !YOUBIN */

  switch (msgchk_rule) {
  case 1:
#ifdef	YOUBIN
    youbin_poll();
    if (!have_mail) {
      new_mail = 0;
      return(0);
    }
#else	/* !YOUBIN */
    if (pop3_stat(1) != POP3_OK) {
      new_mail = 0;
      return(0);
    }
#endif	/* !YOUBIN */
    break;
  case 2:
#ifdef	MH
    if (!mh_chkmsg()) {
      new_mail = 0;
      return(0);
    }
#else	/* !MH */
    new_mail = 0;
    return(0);
#endif	/* !MH */
    break;
  case 0:
  default:
    return(-1);
    break;
  }
  if (!new_mail) {
    if (bell_mode) {
      term_bell();
    }
    new_mail = 1;
  }
  return(1);
#else	/* !(MH || RMAIL || UCBMAIL) */
  return(0);
#endif	/* !(MH || RMAIL || UCBMAIL) */
}

/*
 * ページャ実行
 */

int	exec_pager(show_file, conv_file, mode, str)
     char	*show_file;
     char	*conv_file;
     int	mode;		/*
				 * 0:無変換
				 * 1:ヘッダ削除+コード変換(MIME 処理)
				 * 2:コード変換(MIME 処理)
				 * 3:コード変換(MIME 非処理)
				 */
     char	*str;
{
  register int	status;
  char		buff[BUFF_SIZE];
  KANJICODE	temp_code;

  strcpy(guide_message, str);
  if (show_file) {
    if (pager_mode) {
      status = convert_code(show_file, conv_file, print_code, mode);
      if (status) {
	funlink2(conv_file);
	return(status < 0);
      }
    } else {
      if (mode) {
	/* 内蔵ページャーの漢字コード判定に任せる */
	status = convert_code(show_file, conv_file, EUC_CODE, mode);
	if (status) {
	  funlink2(conv_file);
	  return(status < 0);
	}
      } else {
	conv_file = show_file;
	show_file = (char*)NULL;
      }
    }
  }
  if (pager_mode) {
    term_clear_all();
#if	defined(MSDOS) || defined(OS2)
    sprintf(buff, "%s %s", pager, conv_file);
#else	/* !(MSDOS || OS2) */
    sprintf(buff, "exec %s %s", pager, conv_file);
#endif	/* !(MSDOS || OS2) */
    status = term_system(buff);
    if (status) {
      print_fatal("Pager error.");
    }
  } else {
    temp_code = default_code;
    default_code = EUC_CODE;
    status = view_file(conv_file, 0);
    default_code = temp_code;
  }
  if (show_file) {
    funlink2(conv_file);
  }
  return(status);
}

#if	defined(NEWSPOST) || defined(MAILSEND)
/*
 * エディタ起動
 */

int	exec_editor(edit_file, mode)
     char	*edit_file;
     int	mode;	/* 0:更新チェックなし,0以外:更新チェックあり */
{
  struct stat	stat_buff1, stat_buff2;
  int	status;
  char	buff[BUFF_SIZE];
#ifdef	SELECT_EDITOR
  char	buff2[BUFF_SIZE];
#endif	/* !SELECT_EDITOR */

#ifdef	MIPS
  term_init(2);
#endif	/* MIPS */
#ifdef	SELECT_EDITOR
  strcpy(buff2, edit_command);
  input_line(0, "エディタ:", "Editor:", buff2);
  if (!buff2[0]) {
    return(-1);
  }
#if	defined(MSDOS) || defined(OS2)
  sprintf(buff, "%s %s", buff2, edit_file);
#else	/* !(MSDOS || OS2) */
  sprintf(buff, "exec %s %s", buff2, edit_file);
#endif	/* !(MSDOS || OS2) */
#else	/* !SELECT_EDITOR */
#if	defined(MSDOS) || defined(OS2)
  sprintf(buff, "%s %s", edit_command, edit_file);
#else	/* !(MSDOS || OS2) */
  sprintf(buff, "exec %s %s", edit_command, edit_file);
#endif	/* !(MSDOS || OS2) */
#endif	/* !SELECT_EDITOR */
#ifdef	DEBUG
  fprintf(stderr, "execute %s\n", buff);
  status = 0;
#else	/* !DEBUG */
  if (!stat(edit_file, &stat_buff1)) {
    status = term_system(buff);
#ifndef	IGNORE_EDITOR
    if (mode && !stat(edit_file, &stat_buff2)) {
      if ((stat_buff1.st_mtime == stat_buff2.st_mtime)
	  && (stat_buff1.st_size == stat_buff2.st_size)) {
	print_mode_line(japanese ? "編集されていませんでした。" :
			"Not modified.");
	term_bell();
	sleep(ERROR_SLEEP);
	return(-1);
      }
    }
#endif	/* IGNORE_EDITOR */
  } else {
    status = -1;
  }
#endif	/* !DEBUG */
  term_clear_all();
  if (status) {
#ifdef	IGNORE_EDITOR
    status = 0;
#else	/* !IGNORE_EDITOR */
    print_fatal("Editor error.");
#endif	/* !IGNORE_EDITOR */
  }
  if (status) {
    print_mode_line(japanese ? "エディタの起動に失敗しました。" :
		    "Execute editor failed.");
    term_bell();
    sleep(ERROR_SLEEP);
    return(1);
  }
  return(0);	/* return(status) としてまとめてはいけない */
}
#endif	/* (NEWSPOST || MAILSEND) */

/*
 * NNTP サーバ接続
 */

NNTP_ERR_CODE	open_nntp_server(mode)
     int	mode;	/* 0:グループ初期化,0以外:グループ保留	*/
{
  NNTP_ERR_CODE	status;
  int		number;

  if (mail_mode) {
    return(NNTP_ERR_RSV);
  }
  if (mode) {
    number = group_number;
    status = nntp_open();
    group_number = number;
  } else {
    status = nntp_open();
  }
  switch (status) {
  case NNTP_ERR_AUTH:
    print_mode_line(japanese ? "ユーザ認証に失敗しました。" :
		    "Authentication failed.");
    break;
  case NNTP_ERR_SERV:
    print_mode_line(japanese ? "ニュースサーバがサービスを中止しています。" :
		    "News server service discontinued.");
    break;
  case NNTP_OK:
    break;
  default:
    print_mode_line(japanese ? "ニュースサーバと接続できませんでした。" :
		    "Can't connect news server.");
    break;
  }
  if (status != NNTP_OK) {
    nntp_free();
    group_number = 0;
    term_bell();
    sleep(ERROR_SLEEP);
  }
  return(status);
}

/*
 * Date: フィールドの生成
 */

void	create_date_field(ptr, tm)
     char	*ptr;
     struct tm	*tm;
{
  int		status;
#ifndef	DONT_HAVE_FTIME
  struct timeb	timeb;
#endif	/* !DONT_HAVE_FTIME */

  sprintf(ptr, "%s %s, %d %s %d %02d:%02d:%02d ", DATE_FIELD,
	  day_string[tm->tm_wday + 1],
	  tm->tm_mday,
	  month_string[tm->tm_mon + 1],
	  tm->tm_year,
	  tm->tm_hour,
	  tm->tm_min,
	  tm->tm_sec);
  while (*ptr) {
    ptr++;
  }
#ifdef	USE_TZNAME
#if	notdef
  strcpy(ptr, tm->tm_zone);
#else	/* !notdef */
  tzset();
  if (tm->tm_isdst > 0) {
    strcpy(ptr, tzname[1]);
  } else {
    strcpy(ptr, tzname[0]);
  }
#endif	/* !notdef */
#else	/* !USE_TZNAME */
#ifdef	DONT_HAVE_FTIME
#ifdef	SVR3
  status = timezone / 60;
#else	/* !SVR3 */
  status = (tm->tm_gmtoff) / 60;
#endif	/* !SVR3 */
#else	/* !DONT_HAVE_FTIME */
  ftime(&timeb);
  status = -timeb.timezone;
#endif	/* !DONT_HAVE_FTIME */
  if (status > 0) {
    sprintf(ptr, "+%02d%02d", status / 60, status % 60);
  } else if (status < 0) {
    status = -status;
    sprintf(ptr, "-%02d%02d", status / 60, status % 60);
  } else {
    strcpy(ptr, "GMT");
  }
#endif	/* !USE_TZNAME */
  while (*ptr) {
    ptr++;
  }
  *ptr++ = '\n';
  *ptr = '\0';
}

/*
 * パイプエラーハンドラ
 * (Broken Pipe)
 */

void	pipe_error()
{
#ifdef	SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif	/* SIGPIPE */
  print_fatal("Broken pipe.");
}

/*
 * 強制終了処理
 */

void	force_exit()
{
  static int	exit_lock = 0;
  register int	i;

  /*
   * 作業ファイルの削除
   */

  if (!exit_lock) {
    exit_lock = 1;
    if (emerge_file[0]) {
      news_save_initfile(emerge_file);
    }
    for (i = 0; i < MAX_WRITE_FILE; i++) {
      if (write_file[i][0]) {
	unlink(write_file[i]);
      }
    }
    term_init(1);
    exit(1);
  }
}

/*
 * ファイル作成(kill された時の削除に対応)
 */

FILE	*fopen2(file_name, mode)
     char	*file_name;
     char	*mode;
{
  FILE		*fp;
  int		status;
  register int	i;

  status = 1;
  fp = fopen(file_name, mode);
  if (fp != (FILE*)NULL) {
    chmod(file_name, S_IREAD | S_IWRITE);
    for (i = 0; i < MAX_WRITE_FILE; i++) {
      if (!write_file[i][0]) {
	strcpy(write_file[i], file_name);
	status = 0;
	break;
      }
    }
    if (status) {
      print_fatal("Program error.too many open files \"%s\".", file_name);
    }
  }
  return(fp);
}

/*
 * ファイル取消(kill された時の削除に対応)
 */

int	fclose2(file_name)
     char	*file_name;
{
  register int	i;

  for (i = 0; i < MAX_WRITE_FILE; i++) {
    if (!strcmp(write_file[i], file_name)) {
      write_file[i][0] = '\0';
      return(0);
    }
  }
  print_fatal("Program error.unexpected close file \"%s\".", file_name);
  return(1);
}

/*
 * ファイル削除(kill された時の削除に対応)
 */

int	funlink2(file_name)
     char	*file_name;
{
  register int	i;
  int		status;

  status = 1;
  for (i = 0; i < MAX_WRITE_FILE; i++) {
    if (!strcmp(write_file[i], file_name)) {
      write_file[i][0] = '\0';
      status = 0;
      break;
    }
  }
  if (status) {
    print_fatal("Program error.unexpected unlink file \"%s\".", file_name);
  }
  return(unlink(file_name));
}

/*
 * ヘルプ表示(起動方法)
 */

void	usage(mode)
     int	mode;
{
  fprintf(stderr, "Mini News Reader Version %s Copyright (C) %s By A.Takuma\n",
	  MNEWS_VERSION, MNEWS_DATE);
  if (mode) {
    fprintf(stderr, "Usage : %s [options]\n", prog_name);
    fprintf(stderr, "-e           print EUC mode.\n");
    fprintf(stderr, "-h           print help message.\n");
    fprintf(stderr, "-j           print JIS mode.\n");
    fprintf(stderr, "-m           toggle mail only mode.\n");
    fprintf(stderr, "-n newsgroup specify news group.\n");
    fprintf(stderr, "-s           print SJIS mode.\n");
    fprintf(stderr, "-v           print version.\n");
    fprintf(stderr, "-x           toggle Japanese/English mode.\n");
    fprintf(stderr, "-D server    specify NNTP server.\n");
    fprintf(stderr, "-M server    specify SMTP server.\n");
    fprintf(stderr, "-P server    specify POP3 server.\n");
    fprintf(stderr, "-N           toggle NNTP/NSPL mode.\n");
    fprintf(stderr, "See manual for other options.\n");
  } else {
#if	!(defined(MSDOS) || defined(OS2))
    fprintf(stderr, "Install host,user,date:%s,%s,%s\n", install_host,
	    install_user, install_date);
    fprintf(stderr, "Network      :%s\n", net_opt);
    fprintf(stderr, "Construction :%s\n", cons_opt);
    fprintf(stderr, "Configuration:%s\n", conf_opt);
    fprintf(stderr, "Architecture :%s\n", arch_opt);
#endif	/* !(MSDOS || OS2) */
    fprintf(stderr, "Domain name  :%s\n", domain_name);
  }
}
