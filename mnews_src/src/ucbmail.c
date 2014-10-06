/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : UCB-mail support routine
 *  File        : ucbmail.c
 *  Version     : 1.21
 *  First Edit  : 1992-07/27
 *  Last  Edit  : 1997-11/11
 *  Author      : MSR24 宅間 顯
 *
 */

#ifdef	UCBMAIL
#include	"compat.h"
#include	"nntplib.h"
#include	"smtplib.h"
#include	"pop3lib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"termlib.h"
#include	"mnews.h"
#include	"group.h"
#include	"article.h"
#include	"ucbmail.h"
#include	"pager.h"
#include	"mailsend.h"

static int	ucbmail_select_folder();/* フォルダ選択			*/
static void	ucbmail_redraw_folder();/* フォルダ選択画面再描画	*/
static int	ucbmail_select_article();
					/* 記事選択			*/
static void	ucbmail_redraw_article();
					/* 記事選択画面再描画		*/
static int	ucbmail_prev_folder();	/* 前フォルダ番号取得		*/
static int	ucbmail_next_folder();	/* 次フォルダ番号取得		*/
static int	ucbmail_prev_unread_folder();
					/* 前未読フォルダ番号取得	*/
static int	ucbmail_next_unread_folder();
					/* 次未読フォルダ番号取得	*/
static int	ucbmail_prev_article();	/* 前記事番号取得		*/
static int	ucbmail_next_article();	/* 次記事番号取得		*/
static int	ucbmail_prev_unread_article();
					/* 前記事番号取得		*/
static int	ucbmail_next_unread_article();
					/* 次記事番号取得		*/
#ifdef	LARGE
static int	ucbmail_prev_mark_article();/* 前マーク記事番号取得	*/
static int	ucbmail_next_mark_article();/* 次マーク記事番号取得	*/
#endif	/* LARGE */
static void	ucbmail_search_folder();/* 全フォルダ検索		*/
static int	ucbmail_read();		/* 記事参照			*/
static int	ucbmail_extract();	/* 記事抽出			*/
static void	ucbmail_mark();		/* 記事マーク追加		*/
static void	ucbmail_unmark();	/* 記事マーク取消		*/
static void	ucbmail_count();	/* 記事数カウント		*/
static int	ucbmail_get_list();	/* 記事リスト取得		*/
static void	ucbmail_include();	/* 新着記事取り込み		*/
static void	ucbmail_update();	/* MBOX ファイル更新		*/
static int	ucbmail_create_folder();/* フォルダ作成			*/
static int	ucbmail_delete_folder();/* フォルダ削除			*/
static int	ucbmail_adjust_folder();/* フォルダ位置補正		*/
static int	ucbmail_search_name();	/* フォルダ名検索		*/

/*
 * UCB-mail フォルダリスト
 */

static int	ucbmail_folder_number;	/* UCB-mail フォルダ数		*/
static int	ucbmail_alloc_number;	/* UCB-mail フォルダ割当数	*/
static char	ucbmail_file[PATH_BUFF];/* UCB-mail フォルダファイル名	*/
static struct ucbmail_folder	*ucbmail_folder =
  (struct ucbmail_folder*)NULL;
static int	top_position;		/* 表示開始位置		*/
static int	ucbmail_article_number;	/* UCB-mail 記事数	*/
static int	ucbmail_modify;		/* UCB-mail 更新フラグ	*/

/*
 * UCB-mail ヘルプメッセージ
 */

static char	*ucbmail_folder_jmessage[] = {
  "UCB メールフォルダ選択モード",
  "\n  [選択]",
  "\ti または SPACE  フォルダを選択します。",
  "\n  [移動]",
  "\tk または ^P     前のフォルダに移動します。",
  "\tj または ^N     次のフォルダに移動します。",
  "\tp               前の未読フォルダに移動します。",
  "\tn               次の未読フォルダに移動します。",
  "\t^U または ^B    前画面のフォルダに移動します。",
  "\t^D, ^F または ^V",
  "\t                次画面のフォルダに移動します。",
  "\t<               先頭のフォルダに移動します。",
  "\t>               最後のフォルダに移動します。",
  "\tTAB             指定のフォルダにジャンプします。",
  "\t                (フォルダカテゴリまたはフォルダ名を入力して下さい)",
#ifdef	LARGE
  "\tI               新着用フォルダに移動します。",
#endif	/* LARGE */
  "\to, q または RETURN",
  "\t                フォルダ選択モードから抜けます。",
#ifdef	MAILSEND
  "\n  [送信]",
  "\tm               新たにメールを出します。",
#endif	/* MAILSEND */
#ifndef	SMALL
  "\n  [検索]",
  "\t/               フォルダ名を前方検索します。",
  "\t                (検索したい文字列を入力して下さい)",
  "\t\\               フォルダ名を後方検索します。",
  "\t                (検索したい文字列を入力して下さい)",
#endif	/* !SMALL */
  "",
  "\tC               新たにフォルダを作成します。",
  "\t                (フォルダ名を入力して下さい)",
  "\tDEL または BS   フォルダを削除します。",
  "\t                (y/n で確認を求めてきます)",
  (char*)NULL,
  };

static char	*ucbmail_article_jmessage[] = {
  "UCB メール記事選択モード",
  "\n  [参照]",
  "\tSPACE, i または .",
  "\t                記事を参照します。",
  "\tv または V      全てのヘッダとともに記事を参照します。",
#ifdef	MIME
  "\t                ('V' は MIME デコード対応)",
#endif	/* MIME */
  "\tp               前の未読記事を参照します。",
  "\tn               次の未読記事を参照します。",
  "\tP               前の記事を参照します。",
  "\tN               次の記事を参照します。",
  "\n  [移動]",
  "\tk または ^P     前の記事に移動します。",
  "\tj または ^N     次の記事に移動します。",
  "\t^U または ^B    前画面の記事に移動します。",
  "\t^D, ^F または ^V",
  "\t                次画面の記事に移動します。",
  "\t<               先頭の記事に移動します。",
  "\t>               最後の記事に移動します。",
  "\tTAB             指定の記事にジャンプします。",
  "\t                (記事番号を入力して下さい)",
#ifdef	LARGE
  "\t{               前の削除マーク記事に移動します。",
  "\t}               次の削除マーク記事に移動します。",
#endif	/* LARGE */
  "\to, q または RETURN",
  "\t                記事選択モードから抜けます。",
  "\n  [マーク]",
  "\tD               削除マークし前の記事へ移動します。",
  "\td               削除マークし次の記事へ移動します。",
  "\tU               削除マーク解除し前の記事へ移動します。",
  "\tu               削除マーク解除し次の記事へ移動します。",
  "\tY               既読マークし前の記事へ移動します。",
  "\ty               既読マークし次の記事へ移動します。",
  "\tZ               既読マーク解除し前の記事へ移動します。",
  "\tz               既読マーク解除し次の記事へ移動します。",
  "\tM               マルチマークを指定/解除し次の記事へ移動します。",
  "\tC               保存される全マークを解除し次の記事へ移動します。",
  "\tc               全記事をマークします。",
  "\t                (y:既読マーク n:中止 d:削除マーク を選択して下さい)",
#ifdef	MAILSEND
  "\n  [送信]",
  "\tm               新たにメールを出します。",
  "\tr               記事に返信します。",
  "\tR               記事を引用して返信します。",
  "\t= または f      記事を転送します。",
#ifdef	LARGE
  "\t~               記事を回送します。",
#endif	/* LARGE */
#endif	/* MAILSEND */
  "\n  [セーブ]",
  "\ts               記事をセーブします。",
  "\t                (ファイル名を入力して下さい。ファイルが存在する時は",
  "\t                 y:アペンド n:中止 o:上書きを選んで下さい)",
  "\n  [取り込み]",
  "\tI               新しいメールを取り込みます。",
#ifndef	SMALL
  "\tL               UCB-mail 形式の記事ファイルを取り込みます。",
  "\t                (ファイル名を入力して下さい)",
#endif	/* !SMALL */
  "\n  [削除/整理]",
  "\tDEL または BS   削除マークされた記事を削除します。",
  "\t                (y/n で確認を求めてきます)",
  "\t^               削除マークされた記事を別のフォルダに移動します。",
  "\t                (移動先フォルダ名を求めてきます)",
  "\n  [表示]",
  "\tl               既読/削除された記事の表示モードを切替えます。",
  "\tt               スレッドモードを切替えます。",
#ifdef	REF_SORT
  "\tT               ソート方法を切替えます。",
#endif	/* REF_SORT */
  (char*)NULL,
};

static char	*ucbmail_folder_message[] = {
#ifndef	SMALL
  "UCB-MAIL FOLDER SELECT MODE",
  "\n  [SELECT]",
  "\ti or SPACE   select folder.",
  "\n  [MOVE]",
  "\tk or ^P      previous folder.",
  "\tj or ^N      next folder.",
  "\tp            previous unread folder.",
  "\tn            next unread folder.",
  "\t^U or ^B     previous page folder.",
  "\t^D, ^F or ^V next page folder.",
  "\t<            top folder.",
  "\t>            last folder.",
  "\tTAB          jump to specified folder.",
  "\t             (Please input folder name.)",
#ifdef	LARGE
  "\tI            jump to new article folder.",
#endif	/* LARGE */
  "\to, q or RETURN",
  "\t             exit from folder select mode.",
#ifdef	MAILSEND
  "\n  [SEND]",
  "\tm            mail.",
#endif	/* MAILSEND */
  "\n  [SEARCH]",
  "\t/            forward search pattern.",
  "\t             (Please input search pattern.)",
  "\t\\            backward search pattern.",
  "\t             (Please input search pattern.)",
  "",
  "\tC            create new folder.",
  "\t             (Please input folder name.)",
  "\tDEL or BS    delete folder.",
  "\t             (Please make sure y/n.)",
#endif	/* !SMALL */
  (char*)NULL,
  };

static char	*ucbmail_article_message[] = {
#ifndef	SMALL
  "UCB-MAIL ARTICLE SELECT MODE",
  "\n  [READ]",
  "\tSPACE, i or .",
  "\t             read article.",
  "\tv or V       read article with all header.",
#ifdef	MIME
  "\t             ('V' decode MIME header.)",
#endif	/* MIME */
  "\tp            read previous unread article.",
  "\tn            read next unread article.",
  "\tP            read previous article.",
  "\tN            read next article.",
  "\n  [MOVE]",
  "\tk or ^P      previous article.",
  "\tj or ^N      next article.",
#ifdef	LARGE
  "\t{            previous delete mark article.",
  "\t}            next delete mark article.",
#endif	/* LARGE */
  "\t^U or ^B     previous page article.",
  "\t^D, ^F or ^V next page article.",
  "\t<            top article.",
  "\t>            last article.",
  "\tTAB          jump to specified article.",
  "\t             (Please input article number.)",
  "\to, q or return",
  "\t             exit from article select mode.",
  "\n  [MARK]",
  "\tD            mark as delete and move to previous.",
  "\td            mark as delete and move to next.",
  "\tU            clear delete mark and move to previous.",
  "\tu            clear delete mark and move to next.",
  "\tY            mark as read and move to previous.",
  "\ty            mark as read and move to next.",
  "\tZ            clear read mark and move to previous.",
  "\tz            clear read mark and move to next.",
  "\tM            toggle multi mark and move to next.",
  "\tC            clear all saved mark and move to next.",
  "\tc            mark all articles.",
  "\t             (Please select y:read n:abort or d:delete mark.)",
#ifdef	MAILSEND
  "\n  [SEND]",
  "\tm            mail.",
  "\tr            reply.",
  "\tR            reply with original article.",
  "\t= or f       forward.",
#ifdef	LARGE
  "\t~            relay.",
#endif	/* LARGE */
#endif	/* MAILSEND */
  "\n  [SAVE]",
  "\ts            save article.",
  "\t             (Please input file name.If file exists, please select",
  "\t              y:append n:abort or o:overwrite.)",
  "\n  [INCLUDE]",
  "\tI            include new mail.",
  "\tL            include UCB-mail format article file.",
  "\t             (Please input file name.)",
  "\n  [DELETE/REORDER]",
  "\tDEL or BS    remove delete marked article.",
  "\t             (Please make sure y/n.)",
  "\t^            refile delete marked article.",
  "\t             (Please input folder name.)",
  "\n  [DISPLAY]",
  "\tl            toggle print read/delete article.",
  "\tt            toggle thread mode.",
#ifdef	REF_SORT
  "\tT            change sort rule.",
#endif	/* REF_SORT */
#endif	/* !SMALL */
  (char*)NULL,
};

/*
 * UCB-mail モード初期化
 */

int	ucbmail_init()
{
  return(ucbmail_mode);
}

/*
 * UCB-mail メニュー
 */

int	ucbmail_menu()
{
  struct stat	stat_buff;
  int		status;

  select_name[0] = '\0';
  print_mode_line(japanese ? "検索中です。" : "Searching.");
  if ((!stat(ucbmail_mbox, &stat_buff)) && (stat_buff.st_mode & S_IFDIR)) {
    ucbmail_search_folder();
    while (1) {
      if (auto_inc_mode && auto_inc_folder[0] && (check_new_mail() > 0)) {
	strcpy(jump_name, auto_inc_folder);
      }
      status = ucbmail_select_folder();
      if (!jump_name[0]) {
	break;
      }
    }
    return(status);
  } else {
    ucbmail_folder_number = 0;
    strcpy(ucbmail_file, ucbmail_mbox);
    return(ucbmail_select_article(auto_inc_mode));
  }
}

/*
 * フォルダ選択画面再描画
 */

static void	ucbmail_redraw_folder(current_folder)
     int	current_folder;
{
  register int	i, j;
  char		buff[SMALL_BUFF];
  char		jname[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_folder,
				  ucbmail_folder_number, mail_thread_mode,
				  buff);
  if (i == top_position) {
    return;
  }
  print_title();
  if (wide_mode) {
    term_attrib(color_code[HEADER_COLOR]);
  } else {
    term_attrib(color_code[CATEGORY_COLOR]);
  }
  strcpy(jname, select_name);
#ifdef	JNAMES
  get_jname(UCBMAIL_JN_DOMAIN, jname, 40);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "フォルダ:%-48.48s       位置:%s" :
		  "Folder:%-48.48s     Position:%s",
		  jname, buff);
  if (!wide_mode) {
    j = (check_new_mail() > 0);
    term_locate(0, 2);
    term_attrib(RESET_ATTRIB);
    term_attrib(color_code[HEADER_COLOR]);
    print_full_line(japanese ?
		    "  最大    未読    フォルダ名                       [%-8.8s]" :
		    "  Max     Unread  Folder name                      [%-8.8s]",
		    new_mail_string[j]);
  }
  term_attrib(RESET_ATTRIB);
  for (i = 0, j = top_position; i < term_lines - mode_lines; i++, j++) {
    if (j >= ucbmail_folder_number) {
      break;
    }
    term_locate(0, head_lines + i);
#ifdef	COLOR
    term_attrib(color_code[NUMBER_COLOR]);
#endif	/* COLOR */
    cprintf(" %6d  %6d", ucbmail_folder[j].max_article,
	    ucbmail_folder[j].unread_article);
    strcpy(buff, ucbmail_folder[j].folder_name);
#ifdef	JNAMES
    get_jname(UCBMAIL_JN_DOMAIN, buff, term_columns - 24);
#else	/* !JNAMES */
    buff[term_columns - 24] = '\0';
#endif	/* !JNAMES */
#ifdef	COLOR
    term_attrib(color_code[CATEGORY_COLOR]);
#endif	/* COLOR */
#ifdef	SJIS_SRC
    kanji_printf(SJIS_CODE, "   %s", buff);
#else	/* !SJIS_SRC */
    kanji_printf(EUC_CODE, "   %s", buff);
#endif	/* !SJIS_SRC */
  }
  term_attrib(RESET_ATTRIB);
  print_mode_line(japanese ?
		  "?:ヘルプ j,n,^N:次行 k,p,^P:前行 SPACE,i:選択 o,q,RETURN:復帰 Q:終了" :
		  "?:help j,n,^N:next k,p,^P:previous SPACE,i:select o,q,RETURN:return Q:exit");
}

/*
 * フォルダ選択
 */

static int	ucbmail_select_folder()
{
  int		current_folder;		/* 選択中のフォルダ番号		*/
  char		buff1[BUFF_SIZE];
  int		loop;
  int		status;
  int		key;
  register int	i;

  top_position = -1;
  current_folder = 0;

  loop = 1;
  while (loop) {
    if (jump_name[0]) {
      key = 0;
      current_folder = -1;
      for (i = 0; i < ucbmail_folder_number; i++) {
	if (!strnamecmp(ucbmail_folder[i].folder_name, jump_name)) {
	  current_folder = i;
	  key = ' ';
	  break;
	}
	if (!strnnamecmp(ucbmail_folder[i].folder_name, jump_name,
			 strlen(jump_name))) {
	  if (current_folder < 0) {
	    current_folder = i;
	    key = ' ';
	  } else {
	    key = 0;
	  }
	}
      }
      if (current_folder < 0) {
	current_folder = 0;
      }
      jump_name[0] = '\0';
      if (!key) {
	continue;
      }
    } else {
      ucbmail_redraw_folder(current_folder);
      term_locate(16, head_lines + current_folder - top_position);
      key = get_key(NEWS_MODE_MASK | GLOBAL_MODE_MASK | GROUP_MODE_MASK |
		    MOUSE_MODE_MASK);
#ifdef	MOUSE
      if (mouse_button == 0) {
        int	line;

	line = mouse_row - 1 - (wide_mode ? 1 : 3);
	if (line < 0) {
	  key = 0x02;	/* ^B */
	} else if (line >= term_lines - mode_lines) {
	  key = 0x06;	/* ^F */
	} else {
	  line += top_position;
	  if (line < ucbmail_folder_number) {
	    current_folder = line;
	    key = 'i';
	  }
	}
      } else if (mouse_button == 1) {
	key = ' ';
      } else if (mouse_button == 2) {
	key = 'o';
      }
#endif	/* MOUSE */
    }
    switch (key) {
    case 0x0c:		/* ^L 再描画		*/
      term_init(2);
      top_position = -1;
      break;
    case 0x10:		/* ^P 前フォルダ移動	*/
    case 'k':
      current_folder = ucbmail_prev_folder(current_folder);
      break;
    case 0x0e:		/* ^N 次フォルダ移動	*/
    case 'j':
      current_folder = ucbmail_next_folder(current_folder);
      break;
    case 'p':		/* p 前未読フォルダ移動	*/
      current_folder = ucbmail_prev_unread_folder(current_folder);
      break;
    case 'n':		/* n 次未読フォルダ移動	*/
      current_folder = ucbmail_next_unread_folder(current_folder);
      break;
    case 'o':
    case 'q':
    case '\015':	/* RETURN		*/
    case '\n':
      loop = 0;
      break;
    case 0x02:		/* ^B			*/
    case 0x15:		/* ^U 前ページ		*/
      if ((current_folder -= (term_lines - mode_lines)) < 0) {
	current_folder = 0;
      }
      break;
    case 0x04:		/* ^D			*/
    case 0x06:		/* ^F			*/
    case 0x16:		/* ^V 次ページ		*/
      if ((current_folder += (term_lines - mode_lines))
	  >= ucbmail_folder_number) {
	if (ucbmail_folder_number > 0) {
	  current_folder = ucbmail_folder_number - 1;
	} else {
	  current_folder = 0;
	}
      }
      break;
    case '<':		/* 最初のフォルダ	*/
      current_folder = 0;
      break;
    case '>':		/* 最後のフォルダ	*/
      if (ucbmail_folder_number > 0) {
	current_folder = ucbmail_folder_number - 1;
      } else {
	current_folder = 0;
      }
      break;
    case '\t':		/* TAB 指定フォルダへジャンプ	*/
      strcpy(buff1, ucbmail_mbox);
      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
		 "フォルダ名を入力して下さい:",
		 "Input folder name:", buff1);
      if (buff1[0]) {
	strcpy(jump_name, buff1);
      }
      top_position = -1;
      break;
    case '?':		/* ヘルプ		*/
      help(ucbmail_folder_jmessage, ucbmail_folder_message,
	   GLOBAL_MODE_MASK | GROUP_MODE_MASK);
      top_position = -1;
      break;
    case 'Q':		/* 終了			*/
      return(1);
      /* break; */
    case 'C':		/* フォルダ作成		*/
      strcpy(buff1, ucbmail_mbox);
      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
		 "新規作成するフォルダ名を入力して下さい:",
		 "Input new folder name:", buff1);
      if (buff1[0]) {
	if (!ucbmail_create_folder(buff1)) {
	  if (ucbmail_folder_number > 0) {
	    strcpy(buff1, ucbmail_folder[current_folder].folder_name);
	    ucbmail_search_folder();
	    current_folder = ucbmail_adjust_folder(buff1);
	  } else {
	    ucbmail_search_folder();
	    current_folder = 0;
	  }
	}
      }
      top_position = -1;
      break;
    default:
      if (ucbmail_folder_number > 0) {
	switch (key) {
#ifdef	LARGE
	case 'I':		/* 新着用フォルダ移動	*/
	  if (auto_inc_folder[0]) {
	    strcpy(jump_name, auto_inc_folder);
	    return(1);
	  }
	  /* break 不要 */
#endif	/* LARGE */
	case ' ':
	case 'i':		/* フォルダ選択		*/
	  strcpy(buff1, select_name);
	  strcpy(select_name, ucbmail_folder[current_folder].folder_name);
	  sprintf(ucbmail_file, "%s%c%s", ucbmail_mbox, SLASH_CHAR,
		  ucbmail_folder[current_folder].folder_name);
	  status = ucbmail_select_article(auto_inc_mode &&
					  ((!auto_inc_folder[0]) ||
					   (!strnamecmp(auto_inc_folder,
			ucbmail_folder[current_folder].folder_name))));
	  strcpy(select_name, buff1);
	  strcpy(buff1, ucbmail_folder[current_folder].folder_name);
	  current_folder = 0;
	  for (i = 0; i < ucbmail_folder_number; i++) {
	    if (!strnnamecmp(ucbmail_folder[i].folder_name, buff1,
			     strlen(buff1))) {
	      current_folder = i;
	      break;
	    }
	  }
	  ucbmail_count(current_folder);
	  if (status) {
	    return(1);
	  }
	  top_position = -1;
	  break;
#ifndef	SMALL
	case '/':		/* 前方検索		*/
	  ucbmail_search_name(0, ucbmail_folder_number, &current_folder);
	  break;
	case '\\':		/* 後方検索		*/
	  ucbmail_search_name(1, ucbmail_folder_number, &current_folder);
	  break;
#endif	/* !SMALL */
	case 0x7f:		/* フォルダ削除		*/
	case 0x08:
	  if (ucbmail_folder[current_folder].max_article) {
	    print_mode_line(japanese ?
			    "フォルダ(%s)は空ではありません。" :
			    "Folder(%s) does not empty.",
			    ucbmail_folder[current_folder].folder_name);
	    term_bell();
	    sleep(ERROR_SLEEP);
	  } else {
	    if (yes_or_no(CARE_YN_MODE, "フォルダ(%s)を削除しますか?",
			  "Delete folder(%s)?",
			  ucbmail_folder[current_folder].folder_name)) {
	      if (!ucbmail_delete_folder(ucbmail_folder[current_folder].folder_name)) {
		strcpy(buff1, ucbmail_folder[current_folder].folder_name);
		ucbmail_search_folder();
		current_folder = ucbmail_adjust_folder(buff1);
	      }
	    }
	  }
	  top_position = -1;
	  break;
	default:
	  break;
	}
      }
      break;
    }
  }
  return(0);
}

/*
 * 記事選択画面再描画
 */

static void	ucbmail_redraw_article(current_folder, current_article)
     int	current_folder;
     int	current_article;
{
  register int	i;
  char		buff[SMALL_BUFF];
  char		jname[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_article,
				  ucbmail_article_number, mail_thread_mode,
				  buff);
  if (i == top_position) {
    return;
  }
  print_title();
  if (wide_mode) {
    term_attrib(color_code[HEADER_COLOR]);
  } else {
    term_attrib(color_code[CATEGORY_COLOR]);
  }
  if ((int)strlen(ucbmail_file) > (int)strlen(ucbmail_mbox)) {
    strcpy(jname, &ucbmail_file[strlen(ucbmail_mbox) + 1]);
  } else {
    strcpy(jname, ucbmail_file);
  }
#ifdef	JNAMES
  get_jname(UCBMAIL_JN_DOMAIN, jname, 42);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "メールボックス:%-42.42s       位置:%s" :
		  "Mail box:%-42.42s         Position:%s",
		  jname, buff);
  print_articles(top_position, current_folder, ucbmail_article_number,
		 (int (*)())NULL, "PANIC");
  print_mode_line(japanese ?
		  "?:ヘルプ j,^N:次行 k,^P:前行 SPACE,i,.:参照 o,q:復帰 Q:終了" :
		  "?:help j,^N:next k,^P:previous SPACE,i,.:read o,q:return Q:exit");
}

/*
 * 記事選択
 */

static int	ucbmail_select_article(auto_flag)
     int	auto_flag;
{
  register int	current_folder;		/* 選択中のフォルダ番号		*/
  int		current_article;	/* 選択中の記事番号		*/
  char		buff[BUFF_SIZE];
  char		refile_folder[BUFF_SIZE];
  int		loop;
  int		status;
  int		key;
  register int	i, j;

  current_folder = -1;
  jump_name[0] = '\0';
  for (i = 0; i < ucbmail_folder_number; i++) {
    if (!strnamecmp(select_name, ucbmail_folder[i].folder_name)) {
      current_folder = i;
      break;
    }
  }
  if ((current_folder < 0) && ucbmail_folder_number) {
    print_fatal("Unexpected folder selected.");
    return(0);
  }
  print_mode_line(japanese ? "検索中です。" : "Searching.");
  if (auto_flag) {	/* 自動取込機能		*/
    if ((check_new_mail() > 0)) {
      ucbmail_include((char*)NULL, 1);
    }
  }
  if (ucbmail_get_list() < 0) {
    return(0);
  }

  /*
   * 最初の未読記事までカレントを進める
   */

  current_article = 0;
  if (ucbmail_article_number > 0) {
    while (1) {
      if (current_article >= ucbmail_article_number - 1) {
	break;
      } else if (article_list[current_article].mark & READ_MARK) {
	current_article = ucbmail_next_unread_article(current_article);
	continue;
      } else {
	break;
      }
    }
  }
  top_position = -1;
  loop = 1;
  status = 0;
  while (loop) {
    ucbmail_redraw_article(current_folder, current_article);
    term_locate(article_format ? 6 : 11,
		head_lines + current_article - top_position);
    key = get_key(GLOBAL_MODE_MASK | SUBJECT_MODE_MASK | MOUSE_MODE_MASK);
#ifdef	MOUSE
    if (mouse_button == 0) {
      int	line;

      line = mouse_row - 1 - (wide_mode ? 1 : 3);
      if (line < 0) {
	key = 0x02;	/* ^B */
      } else if (line >= term_lines - mode_lines) {
	  key = 0x06;   /* ^F */
      } else {
	line += top_position;
	if (line < ucbmail_article_number) {
	  current_article = line;
	  key = 'i';
	}
      }
    } else if (mouse_button == 1) {
      key = ' ';
    } else if (mouse_button == 2) {
      key = 'o';
    }
#endif	/* MOUSE */
    switch (key) {
    case 0x0c:		/* ^L 再描画		*/
      term_init(2);
      top_position = -1;
      break;
    case 0x10:		/* ^P 前記事移動	*/
    case 'k':
      current_article = ucbmail_prev_article(current_article);
      break;
    case 0x0e:		/* ^N 次記事移動	*/
    case 'j':
      current_article = ucbmail_next_article(current_article);
      break;
#ifdef	LARGE
    case '{':		/* { 前マーク記事移動	*/
      current_article = ucbmail_prev_mark_article(current_article);
      break;
    case '}':		/* } 次マーク記事移動	*/
      current_article = ucbmail_next_mark_article(current_article);
      break;
#endif	/* LARGE */
    case '\015':	/* RETURN		*/
    case '\n':
    case 'o':
    case 'q':
      loop = 0;
      break;
    case 0x02:		/* ^B 			*/
    case 0x15:		/* ^U 前ページ		*/
      if ((current_article -= (term_lines - mode_lines)) < 0) {
	current_article = 0;
      }
      break;
    case 0x04:		/* ^D			*/
    case 0x06:		/* ^F			*/
    case 0x16:		/* ^V 次ページ		*/
      if ((current_article += (term_lines - mode_lines))
	  >= ucbmail_article_number) {
	if (ucbmail_article_number > 0) {
	  current_article = ucbmail_article_number - 1;
	} else {
	  current_article = 0;
	}
      }
      break;
    case '<':		/* 最初の記事		*/
      current_article = 0;
      break;
    case '>':		/* 最後の記事		*/
      if (ucbmail_article_number > 0) {
	current_article = ucbmail_article_number - 1;
      } else {
	current_article = 0;
      }
      break;
#ifndef	SMALL
    case 'L':		/* 記事ファイル取り込み	*/
      buff[0] = '\0';
      input_line(INPUT_EXPAND_MASK | INPUT_COMP_MASK,
		 "ファイル名を入力して下さい:",
		 "Input file name:", buff);
      if (!buff[0]) {
	top_position = -1;
	break;
      }
      /* break 不要	*/
#endif	/* !SMALL */
    case 'I':		/* 新着記事取り込み	*/
      ucbmail_update(0, (char*)NULL);
      if (key == 'I') {
	if (auto_inc_mode && auto_inc_folder[0]) {
	  i = check_new_mail();
	  if (i > 0) {
	    strcpy(jump_name, auto_inc_folder);
	    return(1);
	  } else if (!i) {
	    break;
	  }
	}
	ucbmail_include((char*)NULL, 1);
      } else {
	ucbmail_include(buff, 0);
      }
      i = ucbmail_article_number;
      ucbmail_get_list();
      current_article = i;
      if (current_article >= ucbmail_article_number) {
	if (ucbmail_article_number > 0) {
	  current_article = ucbmail_article_number - 1;
	} else {
	  current_article = 0;
	}
      }
      top_position = -1;
      break;
    case '?':		/* ヘルプ		*/
      help(ucbmail_article_jmessage, ucbmail_article_message,
	   GLOBAL_MODE_MASK | SUBJECT_MODE_MASK);
      top_position = -1;
      break;
    case 'Q':		/* 終了			*/
      loop = 0;
      status = 1;
      break;
    case 'l':		/* 既読記事パック/解除	*/
    case 't':		/* スレッドソート	*/
    case 'T':		/* スレッドルール	*/
      ucbmail_update(0, (char*)NULL);
      if (!change_sort_rule(-1, &ucbmail_article_number,
			    &current_article,
			    &mail_article_mask, &mail_thread_mode,
			    ucbmail_get_list, key)) {
	top_position = -1;
      }
      break;
    default:
      last_key = key;
      while (last_key) {
	key = last_key;
	last_key = 0;
	switch (key) {
	case 'p':		/* 前未読記事参照	*/
	case 'P':		/* 前記事参照		*/
	  j = current_article;
	  if (ucbmail_article_number > 0) {
	    if (key == 'p') {
	      current_article = ucbmail_prev_unread_article(current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(!current_article);
	    } else {
	      current_article = ucbmail_prev_article(current_article);
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    if (ucbmail_folder_number) {
	      i = current_folder;
	      while (1) {
		if (--i < 0) {
		  i = current_folder;
		  break;
		}
		if (ucbmail_folder[i].unread_article > 0) {
		  break;
		}
	      }
	      if (i != current_folder) {
		switch (yes_or_no(JUMP_YN_MODE,
				  "前の未読フォルダ(%s)を参照しますか?",
				  "Read previous unread folder(%s)?",
				  ucbmail_folder[i].folder_name)) {
		case 1:
		  ucbmail_update(0, (char*)NULL);
		  strcpy(jump_name, ucbmail_folder[i].folder_name);
		  return(1);
		  /* break; */
		case 2:
		  ucbmail_update(0, (char*)NULL);
		  return(0);
		  /* break; */
		default:
		  current_article = j;
		  break;
		}
		top_position = -1;
	      } else {
		current_article = j;
	      }
	    } else {
	      current_article = j;
	    }
	    key = 0;
	  }
	  status = 0;
	  break;
	case 'n':		/* 次未読記事参照	*/
	case 'N':		/* 次記事参照		*/
	  j = current_article;
	  if (ucbmail_article_number > 0) {
	    if (key == 'n') {
	      current_article = ucbmail_next_unread_article(current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(current_article == (ucbmail_article_number - 1));
	    } else {
	      current_article = ucbmail_next_article(current_article);
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    i = current_folder;
	    while (1) {
	      if (++i >= ucbmail_folder_number) {
		i = current_folder;
		break;
	      }
	      if (ucbmail_folder[i].unread_article > 0) {
		break;
	      }
	    }
	    if (i != current_folder) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"次の未読フォルダ(%s)を参照しますか?",
				"Read next unread folder(%s)?",
				ucbmail_folder[i].folder_name)) {
	      case 1:
		ucbmail_update(0, (char*)NULL);
		strcpy(jump_name, ucbmail_folder[i].folder_name);
		return(1);
		/* break; */
	      case 2:
		ucbmail_update(0, (char*)NULL);
		return(0);
		/* break; */
	      default:
		current_article = j;
		break;
	      }
	      top_position = -1;
	    } else {
	      current_article = j;
	    }
	    key = 0;
	  }
	  status = 0;
	  break;
	default:
	  break;
	}
	if (ucbmail_article_number > 0) {
	  i = 1;
	  switch (key) {
	  case 'V':		/* 記事参照		*/
	    i = 3;
	    /* break 不要 */
	  case 'v':
	    i--;
	    key = ' ';
	    /* break 不要 */
	  case ' ':
	  case 'p':
	  case 'n':
	  case 'P':
	  case 'N':
	  case 'i':
	  case '.':
	    if (!ucbmail_read(article_list[current_article].real_number, i)) {
	      switch (last_key) {
	      case 'D':
		ucbmail_mark(current_article, READ_MARK | DELETE_MARK);
		last_key = 'p';
		break;
	      case 'd':
		ucbmail_mark(current_article, READ_MARK | DELETE_MARK);
		last_key = 'n';
		break;
	      case 'Y':
		ucbmail_mark(current_article, READ_MARK);
		last_key = 'p';
		break;
	      case 'y':
		ucbmail_mark(current_article, READ_MARK);
		last_key = 'n';
		break;
	      case 'U':
		ucbmail_mark(current_article, READ_MARK);
		ucbmail_unmark(current_article, DELETE_MARK);
		last_key = 'p';
		break;
	      case 'u':
		ucbmail_mark(current_article, READ_MARK);
		ucbmail_unmark(current_article, DELETE_MARK);
		last_key = 'n';
		break;
	      case 'Z':
		ucbmail_unmark(current_article, READ_MARK);
		last_key = 'p';
		break;
	      case 'z':
		ucbmail_unmark(current_article, READ_MARK);
		last_key = 'n';
		break;
	      default:
		ucbmail_mark(current_article, READ_MARK);
		break;
	      }
	    }
	    if (pager_mode) {
	      switch (key) {
	      case 'p':
		current_article = ucbmail_prev_unread_article(current_article);
		break;
	      case 'n':
	      case ' ':
		current_article = ucbmail_next_unread_article(current_article);
		break;
	      case 'P':
		current_article = ucbmail_prev_article(current_article);
		break;
	      case 'N':
		current_article = ucbmail_next_article(current_article);
		break;
	      default:
		break;
	      }
	    }
	    if (last_key == ' ') {
	      last_key = 'n';
	    }
	    top_position = -1;
	    break;
	  case 'D':		/* 削除マーク,前記事移動	*/
	  case 'd':		/* 削除マーク,次記事移動	*/
	    ucbmail_mark(current_article, DELETE_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'D') {
	      current_article = ucbmail_prev_article(current_article);
	    } else {
	      current_article = ucbmail_next_article(current_article);
	    }
	    break;
	  case 'U':		/* 削除マーク解除,前記事移動	*/
	  case 'u':		/* 削除マーク解除,次記事移動	*/
	    ucbmail_unmark(current_article, DELETE_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'U') {
	      current_article = ucbmail_prev_article(current_article);
	    } else {
	      current_article = ucbmail_next_article(current_article);
	    }
	    break;
	  case 'Y':		/* 既読マーク,前記事移動	*/
	  case 'y':		/* 既読マーク,次記事移動	*/
	    ucbmail_mark(current_article, READ_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'Y') {
	      current_article = ucbmail_prev_article(current_article);
	    } else {
	      current_article = ucbmail_next_article(current_article);
	    }
	    break;
	  case 'Z':		/* 既読マーク解除,前記事移動	*/
	  case 'z':		/* 既読マーク解除,次記事移動	*/
	    ucbmail_unmark(current_article, READ_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'Z') {
	      current_article = ucbmail_prev_article(current_article);
	    } else {
	      current_article = ucbmail_next_article(current_article);
	    }
	    break;
	  case 'c':		/* 全記事マーク		*/
	    switch (yes_or_no(MARK_YN_MODE,
			      "全記事をマークしてよろしいですか?",
			      "Mark all articles.Are you sure?")) {
	    case 1:	/* READ		*/
	      j = READ_MARK;
	      break;
	    case 2:	/* DELETE	*/
	      j = DELETE_MARK;
	      break;
	    default:
	      j = 0;
	      break;
	    }
	    if (j) {
	      for (i = 0; i < ucbmail_article_number; i++) {
		ucbmail_mark(i, j);
	      }
	    }
	    top_position = -1;
	    break;
	  case '^':		/* マーク記事リファイル	*/
	    j = 0;
	    for (i = 0; i < ucbmail_article_number; i++) {
	      if (article_list[i].mark & DELETE_MARK) {
		j++;
	      }
	    }
	    if (j) {
	      strcpy(buff, ucbmail_mbox);
	      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
			 "移動先フォルダ名を入力して下さい:",
			 "Input destination folder name:", buff);
	      if (buff[0]) {
		j = 0;
		for (i = 0; i < ucbmail_folder_number; i++) {
		  if (!strnamecmp(ucbmail_folder[i].folder_name, buff)) {
		    j = 1;
		    break;
		  }
		}
		if (!j)  {
		  if (yes_or_no(NORMAL_YN_MODE,
				"フォルダ(%s)を新規作成しますか?",
				"Create folder(%s).Are you sure?",
				buff)) {
		    if (!ucbmail_create_folder(buff)) {
		      j = 1;
		    }
		  }
		}
		if (j) {
		  sprintf(refile_folder, "%s%c%s", ucbmail_mbox, SLASH_CHAR,
			  buff);
		  if (!strnamecmp(refile_folder, ucbmail_file)) {
		    print_mode_line(japanese ?
				    "移動元と移動先が同じフォルダです。" :
				    "Source and destination folders are identical.");
		    term_bell();
		    sleep(ERROR_SLEEP);
		  } else {
		    ucbmail_update(0, (char*)NULL);
		    ucbmail_update(1, refile_folder);
		    ucbmail_search_folder();
		    ucbmail_get_list();
		    if (current_article >= ucbmail_article_number) {
		      if (ucbmail_article_number > 0) {
			current_article = ucbmail_article_number - 1;
		      } else {
			current_article = 0;
		      }
		    }
		  }
		}
	      }
	      status = 0;
	      top_position = -1;
	    }
	    break;
	  case 0x7f:		/* マーク記事削除	*/
	  case 0x08:
	    j = 0;
	    for (i = 0; i < ucbmail_article_number; i++) {
	      if (article_list[i].mark & DELETE_MARK) {
		j++;
	      }
	    }
	    if (j) {
	      if (yes_or_no(CARE_YN_MODE,
			    "マークされた記事を全て削除してよろしいですか?",
			    "Delete all marked articles.Are you sure?")) {
		ucbmail_update(1, (char*)NULL);
		ucbmail_get_list();
		if (current_article >= ucbmail_article_number) {
		  if (ucbmail_article_number > 0) {
		    current_article = ucbmail_article_number - 1;
		  } else {
		    current_article = 0;
		  }
		}
	      }
	      status = 0;
	    }
	    top_position = -1;
	    break;
	  case 's':		/* 記事セーブ		*/
	    if (!multi_save(current_article, ucbmail_extract)) {
	      multi_add_mark(ucbmail_article_number, -1, current_article,
			     READ_MARK, ucbmail_mark);
	    }
	    top_position = -1;
	    break;
#ifndef	SMALL
	  case '/':		/* 前方検索		*/
	    search_subjects(0, ucbmail_article_number, &current_article,
			    (int (*)())NULL);
	    break;
	  case '\\':		/* 後方検索		*/
	    search_subjects(1, ucbmail_article_number, &current_article,
			    (int (*)())NULL);
	    break;
	  case '|':		/* 記事パイプ実行	*/
	    if (!multi_pipe(current_article, ucbmail_extract)) {
	      multi_add_mark(ucbmail_article_number, -1, current_article,
			     READ_MARK, ucbmail_mark);
	    }
	    top_position = -1;
	    break;
#endif	/* !SMALL */
	  case '\t':		/* TAB 指定記事へジャンプ	*/
	    buff[0] = '\0';
	    input_line(INPUT_SPCCUT_MASK, "記事番号を入力して下さい:",
		       "Input article number:", buff);
	    j = atoi(buff);
	    if ((j > 0) && buff[0]) {
	      for (i = 0; i < ucbmail_article_number; i++) {
		if (article_list[i].real_number == j) {
		  current_article = i;
		  break;
		}
	      }
	    }
	    top_position = -1;
	    break;
#ifdef	MAILSEND
	  case 'r':		/* メール返信		*/
	    create_temp_name(buff, "UR");
	    if (!ucbmail_extract(article_list[current_article].real_number,
				 buff)) {
	      mail_reply(buff, 0, select_name);
	      funlink2(buff);
	    }
	    top_position = -1;
	    break;
	  case 'R':		/* メール返信		*/
	    create_temp_name(buff, "UR");
	    if (!multi_extract(current_article, buff, ucbmail_extract)) {
	      if (!mail_reply(buff, REPLY_QUOTE_MASK, select_name)) {
		multi_add_mark(ucbmail_article_number, -1, current_article,
			       READ_MARK, ucbmail_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -1;
	    break;
	  case '=':		/* メール転送		*/
	  case 'f':
	    create_temp_name(buff, "UT");
	    if (!multi_extract(current_article, buff, ucbmail_extract)) {
	      if (!mail_forward(buff, select_name)) {
		multi_add_mark(ucbmail_article_number, -1, current_article,
			       READ_MARK, ucbmail_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -1;
	    break;
#ifdef	LARGE
	  case '~':		/* メール回送		*/
	    create_temp_name(buff, "UL");
	    if (!multi_extract(current_article, buff, ucbmail_extract)) {
	      if (!mail_relay(buff, select_name)) {
		multi_add_mark(ucbmail_article_number, -1, current_article,
			       READ_MARK, ucbmail_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -1;
	    break;
#endif	/* LARGE */
#endif	/* MAILSEND */
	  case 'C':		/* 全マーク解除		*/
	    ucbmail_unmark(current_article, READ_MARK | FORWARD_MARK
			   | ANSWER_MARK | DELETE_MARK);
	    if (article_list[current_article].mark & MULTI_MARK) {
	      multi_mark(top_position, current_article);
	    }
	    toggle_mark(top_position, current_article, 0);
	    current_article = ucbmail_next_article(current_article);
	    break;
	  case 'M':		/* マルチマーク設定/解除	*/
	    multi_mark(top_position, current_article);
	    current_article = ucbmail_next_article(current_article);
	    break;
#ifdef	LARGE
	  case '@':		/* 印刷			*/
	    if (!multi_print(current_article, ucbmail_extract)) {
	      multi_add_mark(ucbmail_article_number, -1, current_article,
			     READ_MARK, ucbmail_mark);
	    }
	    top_position = -1;
	    break;
	  case '(':		/* 前方記事検索		*/
	    search_articles(0, ucbmail_article_number, &current_article, 
			    ucbmail_extract);
	    break;
	  case ')':		/* 後方記事検索		*/
	    search_articles(1, ucbmail_article_number, &current_article, 
			    ucbmail_extract);
	    break;
#endif	/* LARGE */
	  default:
	    break;
	  }
	} else if (key == ' ') {
	  last_key = 'n';
	}
	switch (last_key) {
	case 'f':
	case '=':
	case '~':
	  print_title();
	case ' ':
	case 'p':
	case 'n':
	case 'P':
	case 'N':
	case 'v':
	case 'V':
	case 'r':
	case 'R':
	case 's':
	  break;
	default:
	  last_key = 0;
	}
      }
      break;
    }
  }
  ucbmail_update(0, (char*)NULL);
  return(status);
}

/*
 * 前フォルダ番号取得
 */

static int	ucbmail_prev_folder(current_folder)
     int	current_folder;
{
  if (--current_folder < 0) {
    current_folder = 0;
  }
  return(current_folder);
}

/*
 * 次フォルダ番号取得
 */

static int	ucbmail_next_folder(current_folder)
     int	current_folder;
{
  if (++current_folder >= ucbmail_folder_number) {
    if (ucbmail_folder_number > 0) {
      current_folder = ucbmail_folder_number - 1;
    } else {
      current_folder = 0;
    }
  }
  return(current_folder);
}

/*
 * 前未読フォルダ番号取得
 */

static int	ucbmail_prev_unread_folder(current_folder)
     int	current_folder;
{
  register int	i;

  i = current_folder;
  if (ucbmail_folder_number > 0) {
    while (--i >= 0) {
      if (ucbmail_folder[i].unread_article) {
	current_folder = i;
	break;
      }
    }
  }
  return(current_folder);
}

/*
 * 次未読フォルダ番号取得
 */

static int	ucbmail_next_unread_folder(current_folder)
     int	current_folder;
{
  register int	i;

  i = current_folder;
  if (ucbmail_folder_number > 0) {
    while (++i < ucbmail_folder_number) {
      if (ucbmail_folder[i].unread_article) {
	current_folder = i;
	break;
      }
    }
  }
  return(current_folder);
}

/*
 * 前記事番号取得
 */

static int	ucbmail_prev_article(current_article)
     int	current_article;
{
  if (--current_article < 0) {
    current_article = 0;
  }
  return(current_article);
}

/*
 * 次フォルダ記事取得
 */

static int	ucbmail_next_article(current_article)
     int	current_article;
{
  if (++current_article >= ucbmail_article_number) {
    if (ucbmail_article_number > 0) {
      current_article = ucbmail_article_number - 1;
    } else {
      current_article = 0;
    }
  }
  return(current_article);
}

/*
 * 前未読記事番号取得
 */

static int	ucbmail_prev_unread_article(current_article)
     int	current_article;
{
  term_enable_abort();
  while (current_article > 0) {
    if (!(article_list[--current_article].mark & READ_MARK)) {
      break;
    }
#ifndef	DONT_USE_ABORT
    if (check_abort()) {
      break;
    }
#endif	/* !DONT_USE_ABORT */
  }
  term_disable_abort();
  return(current_article);
}

/*
 * 次未読記事番号取得
 */

static int	ucbmail_next_unread_article(current_article)
     int	current_article;
{
  term_enable_abort();
  while (current_article < (ucbmail_article_number - 1)) {
    if (!(article_list[++current_article].mark & READ_MARK)) {
      break;
    }
#ifndef	DONT_USE_ABORT
    if (check_abort()) {
      break;
    }
#endif	/* !DONT_USE_ABORT */
  }
  term_disable_abort();
  return(current_article);
}

#ifdef	LARGE
/*
 * 前マーク記事番号取得
 */

static int	ucbmail_prev_mark_article(current_article)
     int	current_article;
{
  while (current_article > 0) {
    if (article_list[--current_article].mark & DELETE_MARK) {
      break;
    }
  }
  return(current_article);
}

/*
 * 次マーク記事番号取得
 */

static int	ucbmail_next_mark_article(current_article)
     int	current_article;
{
  while (current_article < (ucbmail_article_number - 1)) {
    if (article_list[++current_article].mark & DELETE_MARK) {
      break;
    }
  }
  return(current_article);
}
#endif	/* LARGE */

/*
 * 全フォルダ検索
 */

static void	ucbmail_search_folder()
{
  struct ucbmail_folder	*ucbmail_alloc_ptr;
  char			buff1[BUFF_SIZE];
  char			buff2[BUFF_SIZE];
  char			buff3[BUFF_SIZE];
#ifdef	HAVE_FILES
  FILES_STRUCT		dp;
#else	/* !HAVE_FILES */
  DIR_PTR		*dp;
  DIR			*dir_ptr;
  struct stat		stat_buff;
#endif	/* !HAVE_FILES */
  FILE			*fp;
  int			status;

  ucbmail_folder_number = ucbmail_alloc_number = 0;
  if (ucbmail_folder != (struct ucbmail_folder*)NULL) {
    free(ucbmail_folder);
    ucbmail_folder = (struct ucbmail_folder*)NULL;
  }
#ifdef	HAVE_FILES
  sprintf(buff1, "%s%c*.*", ucbmail_mbox, SLASH_CHAR);
  if (!dos_files(buff1, &dp, FILE_ATTR)) {
    do {
#ifdef	__TURBOC__
      strcpy(buff1, dp.ff_name);
#else	/* !__TURBOC__ */
      strcpy(buff1, dp.name);
#endif	/* !__TURBOC__ */
      sprintf(buff2, "%s%c%s", ucbmail_mbox, SLASH_CHAR, buff1);
      if (buff1[strlen(buff1) - 1] != '~') {
        {
	  fp = fopen(buff2, "r");
	  if (fp != (FILE*)NULL) {
	    status = 0;
#ifdef	__TURBOC__
	    if (!dp.ff_fsize) {
#else	/* !__TURBOC__ */
	    if (!dp.filelen) {
#endif	/* !__TURBOC__ */
	      status = 1;
#else	/* !HAVE_FILES */
  if ((dir_ptr = opendir(ucbmail_mbox)) != (DIR*)NULL) {
    while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
      strcpy(buff1, dp->d_name);
      sprintf(buff2, "%s%c%s", ucbmail_mbox, SLASH_CHAR, buff1);
      if ((!stat(buff2, &stat_buff))
	  && (*(buff1 + strlen(dp->d_name) - 1) != '~')) {
	if ((stat_buff.st_mode & S_IFMT) == S_IFREG) {
	  fp = fopen(buff2, "r");
	  if (fp != (FILE*)NULL) {
	    status = 0;
	    if (!stat_buff.st_size) {
	      status = 1;
#endif	/* !HAVE_FILES */
	    } else if (fgets(buff3, sizeof(buff3), fp)) {
	      if (!strncmp(buff3, MSPL_SEPARATER,
			   sizeof(MSPL_SEPARATER) - 1)) {
		status = 1;
	      } else if (!strcmp(buff3, "\n")) {
		if (fgets(buff3, sizeof(buff3), fp) && 
		    (!strncmp(buff3, MSPL_SEPARATER,
			      sizeof(MSPL_SEPARATER) - 1))) {
		  status = 1;
		}
	      }
	    }
	    if (status) {
	      if (ucbmail_alloc_number <= ucbmail_folder_number) {
		if (ucbmail_folder) {
		  ucbmail_alloc_ptr = (struct ucbmail_folder*)
		    realloc(ucbmail_folder,
			    (ucbmail_alloc_number + UCBMAIL_ALLOC_COUNT)
			    * sizeof(struct ucbmail_folder));
		} else {
		  ucbmail_alloc_ptr = (struct ucbmail_folder*)
		    malloc((ucbmail_alloc_number + UCBMAIL_ALLOC_COUNT)
			   * sizeof(struct ucbmail_folder));
		}
		if (ucbmail_alloc_ptr == (struct ucbmail_folder*)NULL) {
		  print_fatal("Can't allocate memory for UCB-mail folder.");
		  fclose(fp);
		  break;
		} else {
		  ucbmail_folder = ucbmail_alloc_ptr;
		  ucbmail_alloc_number += UCBMAIL_ALLOC_COUNT;
		}
	      }
	      strcpy(ucbmail_folder[ucbmail_folder_number].folder_name,
		     buff1);
	      ucbmail_folder_number++;
	      ucbmail_count(ucbmail_folder_number - 1);
	    }
	    fclose(fp);
	  }
	}
      }
#ifdef	HAVE_FILES
    } while (!dos_nfiles(&dp));
#else	/* !HAVE_FILES */
    }
    closedir(dir_ptr);
#endif	/* !HAVE_FILES */
  }
  if (ucbmail_folder_number > 0) {
    qsort(ucbmail_folder, ucbmail_folder_number, sizeof(struct ucbmail_folder),
	  (int (*)())strcmp);
  }
}

/*
 * 記事数カウント
 */

static void	ucbmail_count(folder_number)
     int	folder_number;
{
  FILE		*fp;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  int		max_number;
  int		unread_number;

  if ((folder_number < 0) || (folder_number >= ucbmail_folder_number)) {
    print_fatal("Illegal UCB-mail folder number.");
    return;
  }
  max_number = unread_number = 0;
  sprintf(buff1, "%s%c%s", ucbmail_mbox, SLASH_CHAR,
	  ucbmail_folder[folder_number].folder_name);
  fp = fopen(buff1, "r");
  if (fp != (FILE*)NULL) {
    if (fgets(buff1, sizeof(buff1), fp)) {
      while (1) {
	if (strncmp(buff1, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
	  if (strcmp(buff1, "\n")) {
	    break;
	  }
	  if ((!fgets(buff1, sizeof(buff1), fp)) ||
	      strncmp(buff1, MSPL_SEPARATER,
		      sizeof(MSPL_SEPARATER) - 1)) {
	    break;
	  }
	}
	max_number++;
	buff2[0] = '\0';
	while (fgets(buff1, sizeof(buff1), fp)) {
	  if ((!buff1[0]) || (buff1[0] == '\n')) {
	    break;
	  } else {
	    copy_field(buff1, buff2, UCBMAIL_STATUS_FIELD);
	  }
	}
	if (!strindex(buff2, UCBMAIL_READ_STATUS)) {
	  unread_number++;
	}
	while (fgets(buff1, sizeof(buff1), fp)) {
	  if (!strncmp(buff1, MSPL_SEPARATER,
		       sizeof(MSPL_SEPARATER) - 1)) {
	    break;
	  }
	}
      }
    }
    fclose(fp);
  }
  ucbmail_folder[folder_number].max_article = max_number;
  ucbmail_folder[folder_number].unread_article = unread_number;
}

/*
 * 記事参照
 */

static int	ucbmail_read(real_number, mode)
     int	real_number;
     int	mode;
{
  char	tmp_file1[PATH_BUFF];
  char	tmp_file2[PATH_BUFF];
  char	buff[SMALL_BUFF];
  int	status;

  create_temp_name(tmp_file1, "UC");
  create_temp_name(tmp_file2, "UV");
  if (ucbmail_extract(real_number, tmp_file1)) {
    return(1);
  }
  sprintf(buff, "%d", real_number);
  status = exec_pager(tmp_file1, tmp_file2, mode, buff);
  funlink2(tmp_file1);
  if (status) {
    return(1);
  }
  return(0);
}

/*
 * 記事抽出
 */

static int	ucbmail_extract(real_number, tmp_file)
     int	real_number;
     char	*tmp_file;
{
  FILE	*fp1, *fp2;
  char	buff[BUFF_SIZE];

  fp1 = fopen(ucbmail_file, "r");
  if (fp1 == (FILE*)NULL) {
    return(1);
  }
  while (1) {
    if (!fgets(buff, sizeof(buff), fp1)) {
      fclose(fp1);
      return(1);
    }
    if (!strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
      if (--real_number <= 0) {
	break;
      }
    }
  }
  fp2 = fopen2(tmp_file, "a");
  if (fp2 == (FILE*)NULL) {
    fclose(fp1);
    return(1);
  }
  while (1) {
    fputs(buff, fp2);
    if (!fgets(buff, sizeof(buff), fp1)) {
      break;
    }
    if (!strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
      break;
    }
  }
  fclose(fp1);
  fclose(fp2);
  return(0);
}

/*
 * 記事マーク追加
 */

static void	ucbmail_mark(current_article, mark)
     int	current_article;
     int	mark;
{
  if (!(article_list[current_article].mark & mark)) {
    article_list[current_article].mark |= mark;
    if (mark & ~DELETE_MARK) {
      ucbmail_modify = 1;
    }
  }
}

/*
 * 記事マーク取消
 */

static void	ucbmail_unmark(current_article, mark)
     int	current_article;
     int	mark;
{
  if (article_list[current_article].mark & mark) {
    article_list[current_article].mark &= ~mark;
    if (mark & ~DELETE_MARK) {
      ucbmail_modify = 1;
    }
  }
}

/*
 * 記事リスト取得
 */

static int	ucbmail_get_list()
{
  FILE			*fp;
  char			buff[BUFF_SIZE];
  static char		from_buff[BUFF_SIZE];
  static char		to_buff[BUFF_SIZE];
  static char		date_buff[MAX_FIELD_LEN];
  static char		subject_buff[BUFF_SIZE];
  static char		status_buff[MAX_FIELD_LEN];
  static char		x_nsubj_buff[MAX_FIELD_LEN];
#ifdef	REF_SORT
  static char		message_buff[MAX_FIELD_LEN];
  static char		reference_buff[BUFF_SIZE];
  static char		in_reply_buff[BUFF_SIZE];
#endif	/* REF_SORT */
  static struct cpy_hdr	ucbmail_fields[] = {
    {FROM_FIELD,		from_buff,	sizeof(from_buff)},
#ifdef	LARGE
    {TO_FIELD,		to_buff,	sizeof(to_buff)},
#endif	/* LARGE */
    {DATE_FIELD,		date_buff,	sizeof(date_buff)},
    {SUBJECT_FIELD,		subject_buff,	sizeof(subject_buff)},
    {UCBMAIL_STATUS_FIELD,	status_buff,	sizeof(status_buff)},
    {X_NSUBJ_FIELD,		x_nsubj_buff,	sizeof(x_nsubj_buff)},
#ifdef	REF_SORT
    {MESSAGE_FIELD,	message_buff,	sizeof(message_buff)},
    {REFERENCE_FIELD,	reference_buff,	sizeof(reference_buff)},
    {IN_REPLY_FIELD,	in_reply_buff,	sizeof(in_reply_buff)},
#endif	/* REF_SORT */
  };
  char			*alloc_ptr;
#ifdef	REF_SORT
  char			*ptr;
#else	/* !REF_SORT */
  short			year;
#endif	/* !REF_SORT */
  short			day, hour, minute, second;
  int			alloc_number;
  register int		lines;

  ucbmail_article_number = ucbmail_modify = multi_number = 0;
  if (article_list != (ARTICLE_LIST*)NULL) {
    free(article_list);
  }
  article_list = (ARTICLE_LIST*)NULL;
  alloc_number = 0;
  fp = fopen(ucbmail_file, "r");
  if (fp == (FILE*)NULL) {
    return(1);
  }
  if ((!fgets(buff, sizeof(buff), fp)) ||
      strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
    if (strcmp(buff, "\n")) {
      fclose(fp);
      return(1);
    }
    if ((!fgets(buff, sizeof(buff), fp)) ||
	strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
      fclose(fp);
      return(1);
    }
  }
  while (!feof(fp)) {
    lines = copy_fields(fp, ucbmail_fields,
			 sizeof(ucbmail_fields) / sizeof(struct cpy_hdr),
			 CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
    while (fgets(buff, sizeof(buff), fp)) {
      if (!strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
	break;
      }
      lines++;
    }
    if (alloc_number <= ucbmail_article_number) {
      if (article_list != (ARTICLE_LIST*)NULL) {
	alloc_ptr = (char*)realloc(article_list,
				   sizeof(ARTICLE_LIST) *
				   (alloc_number + UCBMAIL_ALLOC_COUNT));
      } else {
	alloc_ptr = (char*)malloc(sizeof(ARTICLE_LIST) *
				  (alloc_number + UCBMAIL_ALLOC_COUNT));
      }
      if (alloc_ptr == (char*)NULL) {
	print_fatal("Can't allocate memory for UCB-mail article struct.");
	break;
      }
      article_list = (ARTICLE_LIST*)alloc_ptr;
#ifdef	REF_SORT
      if (sort_rule == 1) {
	if (message_list != (MESSAGE_LIST*)NULL) {
	  alloc_ptr = (char*)realloc(message_list,
				     sizeof(MESSAGE_LIST) *
				     (alloc_number + UCBMAIL_ALLOC_COUNT));
	} else {
	  alloc_ptr = (char*)malloc(sizeof(MESSAGE_LIST) *
				    (alloc_number + UCBMAIL_ALLOC_COUNT));
	}
	if (alloc_ptr == (char*)NULL) {
	  print_fatal("Can't allocate memory for message struct.");
	  break;
	}
	message_list = (MESSAGE_LIST*)alloc_ptr;
      }
#endif	/* REF_SORT */
      alloc_number += UCBMAIL_ALLOC_COUNT;
    }
    article_list[ucbmail_article_number].real_number =
      ucbmail_article_number + 1;
    article_list[ucbmail_article_number].lines = lines;
    if (strindex(status_buff, UCBMAIL_READ_STATUS)) {
      article_list[ucbmail_article_number].mark = READ_MARK;
    } else {
      article_list[ucbmail_article_number].mark = 0;
    }
    get_real_adrs(from_buff, article_list[ucbmail_article_number].from);
    if (my_adrs_mode) {
      if (check_my_adrs(article_list[ucbmail_article_number].from)) {
	article_list[ucbmail_article_number].from[0] = MY_ADRS_CHAR;
	get_real_adrs(to_buff, &article_list[ucbmail_article_number].from[1]);
      }
    }
    if (x_nsubj_mode && x_nsubj_buff[0]) {
      recover_jis(subject_buff, x_nsubj_buff);
    }
    mime_decode_func(from_buff, subject_buff, default_code);
    euc_tab_strncpy(article_list[ucbmail_article_number].subject, from_buff,
		    MAX_SUBJECT_LEN - 1);
#ifdef	REF_SORT
    convert_article_date(date_buff, &article_list[ucbmail_article_number].year,
			 &article_list[ucbmail_article_number].month,
			 &article_list[ucbmail_article_number].date,
			 &day, &hour, &minute, &second, from_buff);
    if (message_list != (MESSAGE_LIST*)NULL) {
      strncpy(message_list[ucbmail_article_number].msg_id, message_buff,
	      MAX_FIELD_LEN - 1);
      message_list[ucbmail_article_number].msg_id[MAX_FIELD_LEN - 1] = '\0';
      ptr = strrchr(reference_buff, '<');
      if (ptr != (char*)NULL) {
	strncpy(message_list[ucbmail_article_number].ref_id, ptr,
		MAX_FIELD_LEN - 1);
	message_list[ucbmail_article_number].ref_id[MAX_FIELD_LEN - 1] = '\0';
      } else {
	ptr = strrchr(in_reply_buff, '<');
	if (ptr != (char*)NULL) {
	  strcpy(message_list[ucbmail_article_number].ref_id, ptr);
	} else {
	  message_list[ucbmail_article_number].ref_id[0] = '\0';
	}
      }
      ptr = strchr(message_list[ucbmail_article_number].ref_id, '>');
      if (ptr != (char*)NULL) {
	*(ptr + 1) = '\0';
      }
    }
#else	/* !REF_SORT */
    convert_article_date(date_buff, &year,
			 &article_list[ucbmail_article_number].month,
			 &article_list[ucbmail_article_number].date,
			 &day, &hour, &minute, &second, from_buff);
#endif	/* !REF_SORT */
    ucbmail_article_number++;
  }
  fclose(fp);

  /*	ソート処理	*/

  if (sort_articles(-1, &ucbmail_article_number,
		    mail_thread_mode, 0,
		    (int (*)())NULL, (void (*)())NULL) < 0) {
    return(-1);
  }

  /*	パック処理	*/

#ifdef	notdef
  if (mail_article_mask) {
    pack_articles(-1, &ucbmail_article_number, (int (*)())NULL,
		  (void (*)())NULL);
  }
#endif	/* notdef */
  return(0);
}

/*
 * 新着記事取り込み
 */

static void	ucbmail_include(spool_file, rm_flag)
     char	*spool_file;
     int	rm_flag;
{
  FILE		*fp1, *fp2;
  struct stat	stat_buff;
  POP3_ERR_CODE	status;
  POP3_RD_STAT	rd_stat;
  int		mode;
  int		b_flag;
  int		p_flag;
  char		backup_file[PATH_BUFF];
  char		reserve_file[PATH_BUFF];
  char		buff[BUFF_SIZE];

  /*	バックアップファイルへリネーム	*/

  create_backup_name(backup_file, ucbmail_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  b_flag = 0;
  if (rename(ucbmail_file, backup_file)) {
    if (errno != ENOENT) {
      print_fatal("Can't backup UCB-mail folder file.");
      return;
    }
  } else {
    b_flag = 1;
  }
  if (stat(backup_file, &stat_buff)) {
    stat_buff.st_mode = 0;
  }
  status = POP3_OK;
  fp2 = fopen(ucbmail_file, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't save new UCB-mail folder file.");
    status = POP3_ERR_PROG;
  } else {
    chmod(ucbmail_file, ((stat_buff.st_mode & 01777) | S_IREAD | S_IWRITE));
    print_mode_line(japanese ? "メールボックスを更新中です。" :
		    "Updating UCB-mail folder file.");
    if (b_flag) {
      fp1 = fopen(backup_file, "r");
      if (fp1 != (FILE*)NULL) {
	while (fgets(buff, sizeof(buff), fp1)) {
	  fputs(buff, fp2);
	}
	if (fflush(fp2)) {
	  status = POP3_ERR_PROG;
	}
	fclose(fp1);
      }
    }
  }

  /*	メール取り込み	*/

  mode = pop3_connect_mode;
  if (spool_file != (char*)NULL) {
    pop3_connect_mode = 0;
    strcpy(reserve_file, mail_spool);
    strcpy(mail_spool, spool_file);
  }
  p_flag = 0;
  if (status == POP3_OK) {
    pop3_from_mode = 1;
    status = pop3_open();
    while (status == POP3_OK) {
      p_flag = 1;
      status = pop3_read(buff, sizeof(buff), &rd_stat);
      switch (rd_stat) {
      case POP3_RD_FROM:
      case POP3_RD_HEAD:
      case POP3_RD_SEP:
      case POP3_RD_BODY:
	fputs(buff, fp2);
	break;
      case POP3_RD_END:
      case POP3_RD_EOF:
	break;
      default:
	status = POP3_ERR_PROG;
	break;
      }
      if (rd_stat == POP3_RD_EOF) {
	break;
      }
    }
  }
  if (fp2 != (FILE*)NULL) {
    if (fflush(fp2)) {
      status = POP3_ERR_PROG;
      print_fatal("Can't save new UCB-mail folder file.");
    }
    fclose(fp2);
  }
  if (status != POP3_OK) {
    if (b_flag) {
      rename(backup_file, ucbmail_file);
    }
    rm_flag = 0;
  }
  if (p_flag) {
    if (rm_flag && (pop3_del_mode || (!pop3_connect_mode))) {
      pop3_delete();
    }
    pop3_close();
  }
  if (spool_file != (char*)NULL) {
    pop3_connect_mode = mode;
    strcpy(mail_spool, reserve_file);
  }
}

/*
 * MBOX ファイル更新(マーク更新、削除 or 移動)
 */

static void	ucbmail_update(mode, refile_folder)
     int	mode;		/* 0:マーク更新,0以外:削除 or 移動 */
     char	*refile_folder;	/* 移動先フォルダ */
{
  FILE		*fp1, *fp2, *fp3;
  struct stat	stat_buff;
  int		article_number;
  int		b_flag;
  int		status;
  char		backup_file[PATH_BUFF];
  char		buff[BUFF_SIZE];
  register int	i;

  /*	更新必要性チェック	*/

  if (!(mode || ucbmail_modify)) {
    return;
  }

  /*	バックアップファイルへリネーム	*/

  create_backup_name(backup_file, ucbmail_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  b_flag = 0;
  if (rename(ucbmail_file, backup_file)) {
    if (errno != ENOENT) {
      print_fatal("Can't backup UCB-mail folder file.");
      return;
    }
  } else {
    b_flag = 1;
  }
  if (stat(backup_file, &stat_buff)) {
    stat_buff.st_mode = 0;
  }
  status = 0;
  fp1 = fopen(backup_file, "r");
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't read backup UCB-mail folder file.");
    status = 1;
  } else if ((!fgets(buff, sizeof(buff), fp1)) ||
	     strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
    if (strcmp(buff, "\n") || (!fgets(buff, sizeof(buff), fp1)) ||
	strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
      print_fatal("Unexpected UCB-mail folder file format.");
      status = 1;
    }
  }
  fp2 = fopen(ucbmail_file, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't save new UCB-mail folder file.");
    status = 1;
  } else {
    chmod(ucbmail_file, ((stat_buff.st_mode & 01777) | S_IREAD | S_IWRITE));
  }
  fp3 = (FILE*)NULL;
  if (refile_folder != (char*)NULL) {
    fp3 = fopen(refile_folder, "a");
    if (fp3 == (FILE*)NULL) {
      print_fatal("Can't append UCB-mail folder file.");
      status = 1;
    } else {
      print_mode_line(japanese ? "リファイル中です。" : "Refiling.");
    }
  } else {
    if (mode) {
      print_mode_line(japanese ? "削除中です。" : "Removing.");
    } else {
      print_mode_line(japanese ? "更新中です。" : "Updating.");
    }
  }
  article_number = 0;
  while ((!status) && (!feof(fp1))) {
    for (i = 0; i < ucbmail_article_number; i++) {
      if (article_list[i].real_number == (article_number + 1)) {
	break;
      }
    }
    if (i < ucbmail_article_number) {
      if (mode && (article_list[i].mark & DELETE_MARK)) {
	if (fp3 != (FILE*)NULL) {
	  fputs(buff, fp3);
	}
	while (fgets(buff, sizeof(buff), fp1)) {
	  if (!strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
	    break;
	  }
	  if (fp3 != (FILE*)NULL) {
	    fputs(buff, fp3);
	  }
	}
	article_number++;
	continue;
      }
    }
    fputs(buff, fp2);
    while (fgets(buff, sizeof(buff), fp1)) {
      if (!strncasecmp(buff, UCBMAIL_STATUS_FIELD,
		       sizeof(UCBMAIL_STATUS_FIELD) - 1)) {
	continue;
      }
      if ((buff[0] == '\n') || (buff[0] == '\0')) {
	if (i < ucbmail_article_number) {
	  if (article_list[i].mark & READ_MARK) {
	    fprintf(fp2, "%s %s\n\n", UCBMAIL_STATUS_FIELD,
		    UCBMAIL_READ_STATUS);
	  } else {
	    fprintf(fp2, "%s %s\n\n", UCBMAIL_STATUS_FIELD,
		    UCBMAIL_OLD_STATUS);
	  }
	} else {
	  fputc('\n', fp2);
	}
	while (fgets(buff, sizeof(buff), fp1)) {
	  if (!strncmp(buff, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
	    break;
	  }
	  fputs(buff, fp2);
	}
	break;
      }
      fputs(buff, fp2);
    }
    article_number++;
  }
  if (fp1 != (FILE*)NULL) {
    fclose(fp1);
  }
  if (fp2 != (FILE*)NULL) {
    if (fflush(fp2)) {
      status = 1;
    }
    fclose(fp2);
  }
  if (fp3 != (FILE*)NULL) {
    if (fflush(fp3)) {
      status = 1;
    }
    fclose(fp3);
  }
  if (status) {
    print_fatal("Can't update UCB-mail folder file.");
    if (b_flag) {
      rename(backup_file, ucbmail_file);
    }
  }
}

/*
 * フォルダ作成
 */

static int	ucbmail_create_folder(folder_name)
     char	*folder_name;
{
  struct stat	stat_buff;
  FILE		*fp;
  char		buff[BUFF_SIZE];

  sprintf(buff, "%s%c%s", ucbmail_mbox, SLASH_CHAR, folder_name);
  if (stat(buff, &stat_buff)) {
    fp = fopen(buff, "a");
    if (fp != (FILE*)NULL) {
      chmod(buff, S_IREAD | S_IWRITE);
      fclose(fp);
      return(0);
    }
  }
  term_bell();
  print_mode_line(japanese ? "フォルダが作成できませんでした。" :
		  "Can't create folder.");
  sleep(ERROR_SLEEP);
  return(1);
}

/*
 * フォルダ削除
 */

static int	ucbmail_delete_folder(folder_name)
     char	*folder_name;
{
  struct stat	stat_buff;
  char		buff[BUFF_SIZE];

  sprintf(buff, "%s%c%s", ucbmail_mbox, SLASH_CHAR, folder_name);
  if (!stat(buff, &stat_buff)) {
    if (!stat_buff.st_size) {
      unlink(buff);
      return(0);
    }
  }
  term_bell();
  print_mode_line(japanese ? "フォルダが削除できませんでした。" :
		  "Can't delete folder.");
  sleep(ERROR_SLEEP);
  return(1);
}

/*
 * フォルダ位置補正
 */

static int	ucbmail_adjust_folder(folder_name)
     char	*folder_name;
{
  int	current_folder;
  int	i;

  current_folder = 0;
  for (i = 0; i < ucbmail_folder_number; i++) {
    if (strnamecmp(ucbmail_folder[i].folder_name, folder_name) > 0) {
      break;
    }
    current_folder = i;
  }
  return(current_folder);
}

#ifndef	SMALL
/*
 * フォルダ名検索(複数)
 */

static int	ucbmail_search_name(mode, max_folder, folder_ptr)
     int	mode;
     int	max_folder;
     int	*folder_ptr;
{
  register int	current_folder;
  char		*str;

  str = input_search_string(mode);
  if (str != (char*)NULL) {
    if (mode) {
      for (current_folder = *folder_ptr - 1; current_folder >= 0;
	   current_folder--) {
	if (strindex(ucbmail_folder[current_folder].folder_name, str)) {
	  print_mode_line(japanese ? "見つけました。" : "Search succeed.");
	  *folder_ptr = current_folder;
	  return(0);
	}
      }
    } else {
      for (current_folder = *folder_ptr + 1; current_folder < max_folder;
	   current_folder++) {
	if (strindex(ucbmail_folder[current_folder].folder_name, str)) {
	  print_mode_line(japanese ? "見つけました。" : "Search succeed.");
	  *folder_ptr = current_folder;
	  return(0);
	}
      }
    }
    print_mode_line(japanese ? "見つかりませんでした。" : "Search failed.");
    return(1);
  }
  return(0);
}
#endif	/* !SMALL */
#endif	/* UCBMAIL */
