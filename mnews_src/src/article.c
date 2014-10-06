/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Article select routine
 *  File        : article.c
 *  Version     : 1.21
 *  First Edit  : 1991-07/14
 *  Last  Edit  : 1997-11/11
 *  Author      : MSR24 宅間 顯
 *
 */

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
#include	"pager.h"
#include	"mark.h"
#include	"mailsend.h"
#include	"newspost.h"
#include	"mh.h"

static void	news_redraw_article();	/* 記事選択画面再描画		*/
static int	news_read();		/* 記事参照			*/
static int	news_extract();		/* 記事抽出			*/
static void	news_mark();		/* 記事マーク追加(上位関数)	*/
static void	news_unmark();		/* 記事マーク取消		*/
static void	news_xref_mark();	/* 記事マーク追加(下位関数)	*/
static int	news_refer();		/* リファレンス記事参照		*/
static int	news_find();		/* Message-ID 検索		*/
static int	news_prev_article();	/* 前記事番号取得		*/
static int	news_next_article();	/* 次記事番号取得		*/
static int	news_prev_unread_article();
					/* 前未読記事番号取得		*/
static int	news_next_unread_article();
					/* 次未読記事番号取得		*/
static int	news_prev_unread_article2();
					/* 前未読記事番号取得(マーク)	*/
static int	news_next_unread_article2();
					/* 次未読記事番号取得(マーク)	*/
static int	news_get_list();	/* 未読記事リスト取得		*/
static int	news_get_field();	/* 記事ヘッダ取得		*/
#ifdef	LARGE
static void	kill_articles();	/* 記事全マーク			*/
#endif	/* LARGE */
static int	pipe_article();		/* 記事パイプ実行(下位関数)	*/
static int	sort_subject();		/* Subject ソート		*/
static int	sort_reference();	/* Reference ソート		*/
static int	sort_date();		/* Date ソート			*/
static int	make_thread();		/* スレッド作成			*/
static int	compare_date();		/* 日付比較			*/

static int	(*sort_article_func[])() = {
  sort_subject,
#ifdef	REF_SORT
  sort_reference,
  sort_date,
#endif	/* REF_SORT */
  (int (*)())NULL,
};
static int	news_article_number;		/* 選択されている記事数	*/

ARTICLE_LIST	*article_list = (ARTICLE_LIST*)NULL;
						/* 記事リスト		*/
ARTICLE_LIST	*sort_list = (ARTICLE_LIST*)NULL;
						/* 記事リスト(ソート用)	*/
#ifdef	REF_SORT
MESSAGE_LIST	*message_list = (MESSAGE_LIST*)NULL;
						/* メッセージリスト	*/
#endif	/* REF_SORT */
#if	defined(LARGE) && (!defined(JUST_KILL))
KILL_LIST	*kill_list = (KILL_LIST*)NULL;	/* 自動マークリスト	*/
#endif	/* (LARGE || (!JUST_KILL)) */

static int	top_position;			/* 表示開始位置		*/
static char	base_message[SMALL_BUFF];	/* ベースメッセージ ID	*/
static char	refer_message[SMALL_BUFF];	/* 参照メッセージ ID	*/

/*
 * 記事選択ヘルプメッセージ
 */

static char	*news_article_jmessage[] = {
  "ニュース記事選択モード",
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
#ifndef	SMALL
  "\t*               Message-ID の記事を参照します。",
#endif	/* !SMALL */
  "\t^               リファレンスを参照します。",
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
  "\to, q または RETURN",
  "\t                記事選択モードから抜けます。",
  "\n  [マーク]",
  "\tD または Y      既読マークし前の未読記事へ移動します。",
  "\td または y      既読マークし次の未読記事へ移動します。",
  "\tU または Z      既読マーク解除し前の記事へ移動します。",
  "\tu または z      既読マーク解除し次の記事へ移動します。",
  "\tM               マルチマークを指定/解除し次の記事へ移動します。",
#ifndef	SMALL
  "\tK               同一サブジェクトの記事を既読マークします。",
#endif	/* !SMALL */
  "\tc               全記事を既読マークします。",
  "\t                (y/n で確認を求めてきます)",
#ifdef	LARGE
  "\t+               前方の全記事を既読マークします。",
  "\t                (y/n で確認を求めてきます)",
  "\t-               後方の全記事を既読マークします。",
  "\t                (y/n で確認を求めてきます)",
  "\t$               同一サブジェクトの記事を全グループマークします。",
  "\t%               同一差出人の記事を全グループマークします。",
#endif	/* LARGE */
#ifdef	NEWSPOST
  "\n  [投稿]",
  "\ta               新たに記事をポストします。",
  "\tf               記事にフォローします。",
  "\tF               記事を引用してフォローします。",
  "\tC               記事をキャンセルします。",
  "\t                (y/n で確認を求めてきます)",
#endif	/* NEWSPOST */
#ifdef	MAILSEND
  "\n  [送信]",
  "\tm               新たにメールを出します。",
  "\tr               記事に返信します。",
  "\tR               記事を引用して返信します。",
  "\t=               記事を転送します。",
#ifdef	LARGE
  "\t~               記事を回送します。",
#endif	/* LARGE */
#endif	/* MAILSEND */
  "\n  [セーブ]",
  "\ts               記事をセーブします。",
  "\t                (ファイル名を入力して下さい。ファイルが存在する時は",
  "\t                 y:アペンド n:中止 o:上書きを選んで下さい)",
#ifdef	MH
  "\tO               記事を MH フォルダにセーブします。",
  "\t                (MH フォルダ名を入力して下さい)",
#endif	/* MH */
  "\n  [表示]",
  "\tl               既読/キャンセルされた記事の表示モードを切替えます。",
  "\tt               スレッドモードを切替えます。",
#ifdef	REF_SORT
  "\tT               ソート方法を切替えます。",
#endif	/* REF_SORT */
  "\001",
#ifndef	SMALL
  "\tG               ニュースグループの憲章を表示します。",
  "\tg               ニュースサーバと再接続します。",
#endif	/* !SMALL */
  (char*)NULL,
};

static char	*news_article_message[] = {
#ifndef	SMALL
  "NEWS ARTICLE SELECT MODE",
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
  "\t*            read Message-ID article.",
  "\t^            read referece article.",
  "\n  [MOVE]",
  "\tk or ^P      previous article.",
  "\tj or ^N      next article.",
  "\t^U or ^B     previous page article.",
  "\t^D, ^F or ^V next page article.",
  "\t<            top article.",
  "\t>            last article.",
  "\tTAB          jump to specified article.",
  "\t             (Please input article number.)",
  "\to, q or return",
  "\t             exit from article select mode.",
  "\n  [MARK]",
  "\tD or Y       mark as read and move to previous unread.",
  "\td or y       mark as read and move to next unread.",
  "\tU or Z       clear read mark and move to previous.",
  "\tu or z       clear read mark and move to next.",
  "\tM            toggle multi mark and move to next.",
  "\tK            mark as read same subject articles.",
  "\tc            mark as read all articles.",
  "\t             (Please make sure y/n.)",
#ifdef	LARGE
  "\t+            mark as read all forward articles.",
  "\t             (Please make sure y/n.)",
  "\t-            mark as read all backward articles.",
  "\t             (Please make sure y/n.)",
  "\t$            mark same subject as read for all groups.",
  "\t%            mark same author as read for all groups.",
#endif	/* LARGE */
#ifdef	NEWSPOST
  "\n  [POST]",
  "\ta            post new article.",
  "\tf            follow.",
  "\tF            follow with original article.",
  "\tC            cancel article.",
  "\t             (Please make sure y/n.)",
#endif	/* NEWSPOST */
#ifdef	MAILSEND
  "\n  [SEND]",
  "\tm            mail.",
  "\tr            reply.",
  "\tR            reply with original article.",
  "\t=            forward.",
#ifdef	LARGE
  "\t~            relay.",
#endif	/* LARGE */
#endif	/* MAILSEND */
  "\n  [SAVE]",
  "\ts            save article.",
  "\t             (Please input file name.If file exists, please select",
  "\t              y:append n:abort or o:overwrite.)",
#ifdef	MH
  "\tO            save article in MH folder.",
  "\t             (Please Input MH folder name.)",
#endif	/* MH */
  "\n  [DISPLAY]",
  "\tl            toggle print read/cancel article.",
  "\tt            toggle thread mode.",
#ifdef	REF_SORT
  "\tT            change sort rule.",
#endif	/* REF_SORT */
  "\001",
  "\tG            print news group description.",
  "\tg            reconnect to news server.",
#endif	/* !SMALL */
  (char*)NULL,
};

/*
 * 記事選択画面再描画
 */

static void	news_redraw_article(current_group, current_article)
     int	current_group;
     int	current_article;
{
  register int	i;
  char		buff[SMALL_BUFF];
  char		jname[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_article,
				  news_article_number, news_thread_mode,
				  buff);
  if (i == top_position) {
    return;
  }
  print_title();
#ifdef	COLOR
  if (wide_mode) {
    term_attrib(color_code[HEADER_COLOR]);
  } else {
    term_attrib(color_code[CATEGORY_COLOR]);
  }
#else	/* !COLOR */
  if (wide_mode) {
    term_attrib(REVERSE_ATTRIB);
  }
#endif	/* !COLOR */
  strcpy(jname, select_name);
#ifdef	JNAMES
  get_jname(NEWS_JN_DOMAIN, jname, 40);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "ニュースグループ:%-40.40s       位置:%s" :
		  "News group:%-40.40s         Position:%s",
		  jname, buff);
  print_articles(top_position, current_group, news_article_number,
		 news_get_field, "CANCEL/LOST");
  print_mode_line(japanese ?
		  "?:ヘルプ j,^N:次行 k,^P:前行 SPACE,i,.:参照 l:表示モード切替 o,q:復帰 Q:終了" :
		  "?:help j,^N:next k,^P:previous SPACE,i,.:read l:print mode o,q:return Q:exit");
}

/*
 * 記事選択
 */

int	news_select_article()
{
  register int	current_group;		/* 選択中のグループ番号		*/
  int		current_article;	/* 選択中の記事番号		*/
  char		buff1[BUFF_SIZE];
#ifndef	SMALL
  char		buff2[BUFF_SIZE];
#endif	/* !SMALL */
  int		loop;
  int		status;
  int		key;
  register int	i, j;

  current_group = -1;
  jump_name[0] = '\0';
  top_position = -1;
  for (i = 0; i < group_number; i++) {
    if (!strcmp(select_name, news_group[i].group_name)) {
      current_group = i;
      break;
    }
  }
  if (current_group < 0) {
    print_fatal("Unexpected group selected.");
    return(0);
  }
  if (news_group[current_group].max_article == 0) {
    return(0);
  }
  if (nntp_group(select_name) != NNTP_OK) {
    return(0);
  }
  if ((news_group[current_group].max_article
       - news_group[current_group].min_article) > CONFIRM_NUMBER) {
    confirm_article(news_group[current_group].group_name,
		    &news_group[current_group].min_article,
		    &news_group[current_group].max_article);
  }
  status = news_get_list(current_group);
  if (news_group[current_group].min_article > 1) {
    news_add_mark(current_group, 1,
		  news_group[current_group].min_article - 1);
  }
  if (status < 0) {
    return(0);
  } else if (!status) {
    /*
     * 最初の未読記事までカレントを進める
     */

    current_article = news_next_unread_article2(current_group, 0);
    if ((current_article < news_article_number) &&
	(!(article_list[current_article].mark & READ_MARK))) {
      skip_direction = 0;
    } else {
      j = news_next_article(current_article);
      if (current_article != j) {
	skip_direction = 0;
      }
    }
  } else {
    current_article = 0;
  }
  if (skip_direction) {
    if (skip_direction < 0) {
      i = news_prev_unread_group(current_group);
    } else {
      i = news_next_unread_group(current_group);
    }
    if (i >= 0) {
      print_mode_line(japanese ?
		      "未読記事がないため、未読グループ(%s)を参照します。" :
		      "Because unread article not found,Read unread group(%s).",
		      news_group[i].group_name);
      news_count_unread(current_group);
      strcpy(jump_name, news_group[i].group_name);
      return(1);
    } else {
      skip_direction = 0;
    }
  }

  loop = 1;
  status = 0;
  while (loop) {
    if (top_position == -2) {
      nntp_group(select_name);
    }
    news_redraw_article(current_group, current_article);
    term_locate(article_format ? 6 : 11,
		head_lines + current_article - top_position);
    switch (skip_direction) {
    case -1:
      key = 'p';
      break;
    case 1:
      key = 'n';
      break;
    default:
      key = get_key(NEWS_MODE_MASK | GLOBAL_MODE_MASK | SUBJECT_MODE_MASK |
		    MOUSE_MODE_MASK);
      break;
    }
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
	if (line < news_article_number) {
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
      current_article = news_prev_article(current_article);
      break;
    case 0x0e:		/* ^N 次記事移動	*/
    case 'j':
      current_article = news_next_article(current_article);
      break;
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
	  >= news_article_number) {
	if (news_article_number > 0) {
	  current_article = news_article_number - 1;
	} else {
	  current_article = 0;
	}
      }
      break;
    case '<':		/* 最初の記事		*/
      current_article = 0;
      break;
    case '>':		/* 最後の記事		*/
      if (news_article_number > 0) {
	current_article = news_article_number - 1;
      } else {
	current_article = 0;
      }
      break;
    case '?':		/* ヘルプ		*/
      help(news_article_jmessage, news_article_message,
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
      if (!change_sort_rule(current_group, &news_article_number,
			    &current_article,
			    &news_article_mask, &news_thread_mode,
			    news_get_list, key)) {
	top_position = -1;
      }
      break;
#ifdef	NEWSPOST
    case 'a':		/* ニュース投稿		*/
      if ((news_group[current_group].group_mode == (short)POST_ENABLE) ||
	  (news_group[current_group].group_mode == (short)POST_MODERATED)) {
	news_post(select_name);
	top_position = -2;
      }
      break;
#endif	/* NEWSPOST */
    default:

      /*
       * ここから下の処理は見たら目が腐る危険性があります。
       */

      last_key = key;
      while (last_key) {
	key = last_key;
	last_key = 0;
	switch (key) {
	case 'p':		/* 前未読記事参照	*/
	case 'P':		/* 前記事参照		*/
	  j = current_article;
	  if (news_article_number > 0) {
	    status = current_article;
	    current_article = news_prev_article(current_article);
	    if (key == 'p') {
	      current_article = news_prev_unread_article2(current_group,
							  current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(!current_article);
	    } else {
	      status = (status == current_article);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    i = news_prev_unread_group(current_group);
	    if (i >= 0) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"前の未読(%d)グループ(%s)を参照しますか?",
				"Read previous unread(%d) group(%s)?",
				group_list[i].unread_article,
				news_group[i].group_name)) {
	      case 1:
		news_count_unread(current_group);
		last_key = 0;
		skip_direction = -1;
		strcpy(jump_name, news_group[i].group_name);
		return(1);
		break;
	      case 2:
		news_count_unread(current_group);
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
	case 'n':		/* 次の未読記事参照	*/
	case 'N':		/* 次の記事参照		*/
	  j = current_article;
	  if (news_article_number > 0) {
	    current_article = news_next_article(current_article);
	    if (key == 'n') {
	      current_article = news_next_unread_article2(current_group,
							  current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(current_article == (news_article_number - 1));
	    } else {
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (skip_direction) {
	    skip_direction = 0;
	    key = 0;
	  }
	  if (status) {
	    i = news_next_unread_group(current_group);
	    if (i >= 0) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"次の未読(%d)グループ(%s)を参照しますか?",
				"Read next unread(%d) group(%s)?",
				group_list[i].unread_article,
				news_group[i].group_name)) {
	      case 1:
		skip_direction = 1;
		news_count_unread(current_group);
		last_key = 0;
		strcpy(jump_name, news_group[i].group_name);
		return(1);
		break;
	      case 2:
		news_count_unread(current_group);
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
#ifndef	SMALL
	case 'G':		/* 憲章表示		*/
	  if (nntp_description(news_group[current_group].group_name, buff1)
	      == NNTP_OK) {
	    jis_to_euc(buff2, buff1);
	    print_mode_line(buff2);
	  } else {
	    print_mode_line(japanese ? "憲章が見つかりませんでした。" :
			    "Description not found.");
	  }
	  break;
	case 'g':		/* 再接続		*/
	  nntp_close();
	  print_mode_line(japanese ? "再接続中です。" : "Reconnecting.");
	  open_nntp_server(1);
	  top_position = -2;
	  break;
#endif	/* !SMALL */
	default:
	  break;
	}
	if (news_article_number > 0) {
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
	    if (!news_read(article_list[current_article].real_number, i)) {
	      switch (last_key) {
	      case 'D':
	      case 'Y':
		news_mark(current_group, current_article, READ_MARK);
		last_key = 'p';
		break;
	      case 'd':
	      case 'y':
		news_mark(current_group, current_article, READ_MARK);
		last_key = 'n';
		break;
	      case 'U':
	      case 'Z':
		news_unmark(current_group, current_article);
		last_key = 'P';
		break;
	      case 'u':
	      case 'z':
		news_unmark(current_group, current_article);
		last_key = 'N';
		break;
	      default:
		news_mark(current_group, current_article, READ_MARK);
		break;
	      }
	      if (article_list[current_article].mark & CANCEL_MARK) {
		switch (key) {
		case 'p':
		case 'P':
		case 'n':
		case 'N':
		  last_key = key;
		  break;
		default:
		  last_key = 'n';
		}
	      }
	    }
	    if (pager_mode) {
	      switch (key) {
	      case 'p':
		current_article = news_prev_unread_article2(current_group,
							    current_article);
		break;
	      case 'n':
	      case ' ':
		current_article = news_next_unread_article2(current_group,
							    current_article);
		break;
	      case 'P':
		current_article = news_prev_article(current_article);
		break;
	      case 'N':
		current_article = news_next_article(current_article);
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
	  case 'D':		/* マーク,前記事移動	*/
	  case 'Y':
	  case 'd':		/* マーク,次記事移動	*/
	  case 'y':
	    news_mark(current_group, current_article, READ_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (isupper(key)) {
	      current_article = news_prev_unread_article(current_article);
	    } else {
	      current_article = news_next_unread_article(current_article);
	    }
	    break;
	  case 'U':		/* マーク解除,前記事移動	*/
	  case 'Z':
	  case 'u':		/* マーク解除,次記事移動	*/
	  case 'z':
	    news_unmark(current_group, current_article);
	    toggle_mark(top_position, current_article, 0);
	    if (isupper(key)) {
	      current_article = news_prev_article(current_article);
	    } else {
	      current_article = news_next_article(current_article);
	    }
	    break;
	  case 'c':		/* 全記事マーク		*/
	    if (yes_or_no(NORMAL_YN_MODE,
			  "全記事をマークしてよろしいですか?",
			  "Mark all articles.Are you sure?")) {
	      for (i = 0; i < news_article_number; i++) {
		article_list[i].mark |= READ_MARK;
	      }
	      news_fill_mark(current_group);
	    }
	    top_position = -1;
	    break;
	  case 's':		/* 記事セーブ		*/
	    if (!multi_save(current_article, news_extract)) {
	      multi_add_mark(news_article_number, current_group,
			     current_article, READ_MARK, news_mark);
	    }
	    top_position = -1;
	    break;
#ifdef	MH
	  case 'O':		/* フォルダへ記事セーブ */
	    if (!save_mh_folder(current_article, news_extract)) {
	      multi_add_mark(news_article_number, current_group,
			     current_article, READ_MARK, news_mark);
	    }
	    top_position = -1;
	    break;
#endif	/* MH */
#ifndef	SMALL
	  case '/':		/* 前方検索		*/
	    search_subjects(0, news_article_number, &current_article, 
			    news_get_field);
	    break;
	  case '\\':		/* 後方検索		*/
	    search_subjects(1, news_article_number, &current_article, 
			    news_get_field);
	    break;
	  case '|':		/* 記事パイプ実行	*/
	    if (!multi_pipe(current_article, news_extract)) {
	      multi_add_mark(news_article_number, current_group,
			     current_article, READ_MARK, news_mark);
	    }
	    top_position = -1;
	    break;
	  case '*':		/* Message-ID 参照	*/
	    buff1[0] = '\0';
	    input_line(INPUT_SPCCUT_MASK, "Message-IDを入力して下さい:",
		       "Input Message-ID:", buff1);
	    news_find(buff1);
	    top_position = -1;
	    break;
#endif	/* !SMALL */
	  case '\t':		/* TAB 指定記事へジャンプ	*/
	    buff1[0] = '\0';
	    input_line(INPUT_SPCCUT_MASK, "記事番号を入力して下さい:",
		       "Input article number:", buff1);
	    if ((j = atoi(buff1)) > 0) {
	      for (i = 0; i < news_article_number; i++) {
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
	    create_temp_name(buff1, "NR");
	    if (!news_extract(article_list[current_article].real_number,
			      buff1)) {
	      mail_reply(buff1, 0, "");
	      funlink2(buff1);
	    }
	    top_position = -2;
	    break;
	  case 'R':		/* メール返信		*/
	    create_temp_name(buff1, "NR");
	    if (!multi_extract(current_article, buff1, news_extract)) {
	      if (!mail_reply(buff1, REPLY_QUOTE_MASK, "")) {
		multi_add_mark(news_article_number, current_group,
			       current_article, READ_MARK, news_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -2;
	    break;
	  case '=':		/* メール転送		*/
	    create_temp_name(buff1, "NT");
	    if (!multi_extract(current_article, buff1, news_extract)) {
	      if (!mail_forward(buff1, "")) {
		multi_add_mark(news_article_number, current_group,
			       current_article, READ_MARK, news_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -2;
	    break;
#ifdef	LARGE
	  case '~':		/* メール回送		*/
	    create_temp_name(buff1, "NL");
	    if (!multi_extract(current_article, buff1, news_extract)) {
	      if (!mail_relay(buff1, "")) {
		multi_add_mark(news_article_number, current_group,
			       current_article, READ_MARK, news_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -2;
	    break;
#endif	/* LARGE */
#endif	/* MAILSEND */
#ifdef	NEWSPOST
	  case 'f':		/* ニュースフォロー	*/
	    if ((news_group[current_group].group_mode == (short)POST_ENABLE) ||
		(news_group[current_group].group_mode ==
		 (short)POST_MODERATED)) {
	      create_temp_name(buff1, "NF");
	      if (!news_extract(article_list[current_article].real_number,
				buff1)) {
		news_follow(buff1, 0);
		funlink2(buff1);
	      }
	      top_position = -2;
	    }
	    break;
	  case 'F':		/* ニュースフォロー	*/
	    if ((news_group[current_group].group_mode == (short)POST_ENABLE) ||
		(news_group[current_group].group_mode ==
		 (short)POST_MODERATED)) {
	      create_temp_name(buff1, "NF");
	      if (!multi_extract(current_article, buff1, news_extract)) {
		if (!news_follow(buff1, FOLLOW_QUOTE_MASK)) {
		  multi_add_mark(news_article_number, current_group,
				 current_article, READ_MARK, news_mark);
		}
		funlink2(buff1);
	      }
	      top_position = -2;
	    }
	    break;
	  case 'C':		/* キャンセル		*/
	    if ((news_group[current_group].group_mode == (short)POST_ENABLE) ||
		(news_group[current_group].group_mode ==
		 (short)POST_MODERATED)) {
	      if (!news_cancel(article_list[current_article].real_number)) {
		article_list[current_article].mark |= CANCEL_MARK;
	      }
	      top_position = -1;
	    }
	    break;
#endif	/* NEWSPOST */
	  case '^':		/* リファレンス記事参照	*/
	    news_refer(article_list[current_article].real_number);
	    top_position = -1;
	    if (last_key != '^') {
	      last_key = 0;
	    }
	    break;
	  case 'M':		/* マルチマーク設定/解除	*/
	    multi_mark(top_position, current_article);
	    current_article = news_next_article(current_article);
	    break;
#ifndef	SMALL
	  case 'K':		/* 同一Subjectマーク,次記事移動	*/
	    kill_subjects(current_group, current_article, news_article_number,
			  news_get_field, news_add_mark);
	    current_article = news_next_unread_article2(current_group,
							current_article);
	    top_position = -1;
	    break;
#endif	/* !SMALL */
#ifdef	LARGE
	  case '@':		/* 印刷			*/
	    if (!multi_print(current_article, news_extract)) {
	      multi_add_mark(news_article_number, current_group,
			     current_article, READ_MARK, news_mark);
	    }
	    top_position = -1;
	    break;
	  case '(':		/* 前方記事検索		*/
	    search_articles(0, news_article_number, &current_article, 
			    news_extract);
	    break;
	  case ')':		/* 後方記事検索		*/
	    search_articles(1, news_article_number, &current_article, 
			    news_extract);
	    break;
	  case '$':		/* 同一Subject全マーク	*/
	    kill_articles(0, current_group, current_article);
	    top_position = -1;
	    break;
	  case '%':		/* 同一From全マーク	*/
	    kill_articles(1, current_group, current_article);
	    top_position = -1;
	    break;
	  case '+':		/* 前方記事全マーク	*/
	    if (yes_or_no(NORMAL_YN_MODE,
			  "前方の全記事をマークしてよろしいですか?",
			  "Mark all forward articles.Are you sure?")) {
	      for (i = current_article; i < news_article_number; i++) {
		article_list[i].mark |= READ_MARK;
		news_add_mark(current_group,
			      article_list[i].real_number,
			      article_list[i].real_number);
	      }
	    }
	    top_position = -1;
	    break;
	  case '-':		/* 後方記事全マーク	*/
	    if (yes_or_no(NORMAL_YN_MODE,
			  "後方の全記事をマークしてよろしいですか?",
			  "Mark all backward articles.Are you sure?")) {
	      for (i = 0; i <= current_article; i++) {
		article_list[i].mark |= READ_MARK;
		news_add_mark(current_group,
			      article_list[i].real_number,
			      article_list[i].real_number);
	      }
	    }
	    top_position = -1;
	    break;
#endif	/* LARGE */
	  default:
	    break;
	  }
	} else if (key == ' ') {
	  last_key = 'n';
	}
	switch (last_key) {
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
	case 'f':
	case 'F':
	case '^':
	case 'K':
	case 'O':
	case 's':
	case '*':
	  break;
	default:
	  last_key = 0;
	}
      }
      break;
    }
  }
  news_count_unread(current_group);
  return(status);
}

/*
 * 記事参照
 */

static int	news_read(real_number, mode)
     int	real_number;
     int	mode;
{
  FILE		*fp;
  char		tmp_file1[PATH_BUFF];
  char		tmp_file2[PATH_BUFF];
  char		buff[BUFF_SIZE];
  static struct cpy_hdr	xref_fields[] = {
    {XREF_FIELD,	(char*)NULL,	sizeof(buff)},
  };
  int		status;

  create_temp_name(tmp_file1, "NC");
  create_temp_name(tmp_file2, "NV");
  status = news_extract(real_number, tmp_file1);
  if (status) {
    if (status < 0) {
      return(0);
    }
    return(1);
  }
  sprintf(buff, "%d", real_number);
  status = exec_pager(tmp_file1, tmp_file2, mode, buff);

  /*
   * クロスポスト時の未読処理
   * ここの処理も見たら目が腐る危険があります。
   */

  if (!status) {
    fp = fopen(tmp_file1, "r");
    if (fp != (FILE*)NULL) {
      xref_fields[0].field_buff = buff;
      copy_fields(fp, xref_fields,
		  sizeof(xref_fields) / sizeof(struct cpy_hdr),
		  CF_CLR_MASK | CF_GET_MASK);
      fclose(fp);
      if (buff[0]) {
	news_xref_mark(buff);
      }
    }
  }
  funlink2(tmp_file1);
  return(status);
}

/*
 * 記事抽出
 */

static int	news_extract(real_number, tmp_file)
     int	real_number;
     char	*tmp_file;
{
  FILE	*fp;
  int	status;

  fp = fopen2(tmp_file, "a");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't open extract file.");
    return(1);
  }
  status = nntp_article(real_number, fp);
  fclose(fp);
  if (status != NNTP_OK) {
    funlink2(tmp_file);
    return(1);
  }
  return(0);
}

/*
 * 記事マーク追加(上位関数)
 */

static void	news_mark(current_group, current_article, mark)
     int	current_group;
     int	current_article;
     int	mark;		/* 未使用だが I/F の統一のため必要	*/
{
  char			buff[BUFF_SIZE];
  static struct cpy_hdr	xref_fields[] = {
    {XREF_FIELD,	(char*)NULL,	sizeof(buff)},
  };

  if (mark & READ_MARK) {
    if (cross_mark_mode) {
      xref_fields[0].field_buff = buff;
      if (nntp_copy_fields(article_list[current_article].real_number,
			   xref_fields,
			   sizeof(xref_fields) / sizeof(struct cpy_hdr),
			   CF_CLR_MASK | CF_GET_MASK) == NNTP_OK) {
	news_xref_mark(buff);
      }
    }
    news_add_mark(current_group,
		  article_list[current_article].real_number,
		  article_list[current_article].real_number);
    article_list[current_article].mark |= READ_MARK;
  }
}

/*
 * 記事マーク取消
 */

static void	news_unmark(current_group, current_article)
     int	current_group;
     int	current_article;
{
  article_list[current_article].mark &= ~READ_MARK;
  news_delete_mark(current_group,
		   article_list[current_article].real_number,
		   article_list[current_article].real_number);
}

/*
 * 記事マーク追加(下位関数)
 */

static void	news_xref_mark(ptr1)
     char	*ptr1;
{
  char		*ptr2;
  register int	i, j;

  /*
   * マシン名をスキップ
   */
  
  while ((*ptr1 != ' ') && (*ptr1 != '\t') && (*ptr1 != '\0')) {
    ptr1++;
  }
  while (*ptr1) {
    while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
      ptr1++;
    }
    ptr2 = ptr1;
    while (*ptr1) {
      if (*ptr1 == ':') {
	*ptr1++ = '\0';
	j = atoi(ptr1);
	while (isdigit(*ptr1)) {
	  ptr1++;
	}
	if (j > 0) {
	  for (i = 0; i < group_number; i++) {
	    if (!strcmp(news_group[i].group_name, ptr2)) {
	      news_add_mark(i, j, j);
	      news_count_unread(i);
	      break;
	    }
	  }
	}
	break;
      } else {
	ptr1++;
      }
    }
  }
}

/*
 * リファレンス記事参照
 */

static int	news_refer(real_number)
     int	real_number;
{
  FILE			*fp;
  static char		buff1[SMALL_BUFF];
  static char		buff2[BUFF_SIZE];
  char			tmp_file1[PATH_BUFF];
  char			tmp_file2[PATH_BUFF];
  char			*ptr1, *ptr2;
  static struct cpy_hdr	refer_fields[] = {
    {MESSAGE_FIELD,	buff1,	sizeof(buff1)},
    {REFERENCE_FIELD,	buff2,	sizeof(buff2)},
  };
  int			status;

  if (nntp_copy_fields(real_number, refer_fields,
		       sizeof(refer_fields) / sizeof(struct cpy_hdr),
		       CF_CLR_MASK | CF_GET_MASK) != NNTP_OK) {
    return(1);
  }
  if (strcmp(buff1, base_message) || (!refer_message[0])) {
    strcpy(base_message, buff1);
    refer_message[0] = '\0';
  }

  ptr1 = buff2;
  if (refer_message[0]) {
    ptr2 = strindex(ptr1, refer_message);
    if (ptr2 != (char*)NULL) {
      *ptr2 = '\0';	/* 既に参照した Reference 情報を捨てる */
      if (ptr1 == ptr2) {
	base_message[0] = '\0';
	return(1);
      }
    } else {
      base_message[0] = '\0';
      return(1);
    }
  }
  ptr2 = ptr1;

  /*
   * 一つ前の Reference を探す
   */

  while (*ptr1) {
    while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
      ptr1++;
    }
    if (*ptr1) {
      ptr2 = ptr1;
      while (*ptr1) {
	if (*ptr1++ == '>') {
	  break;
	}
      }
    }
  }
  ptr1 = refer_message;
  while (*ptr2) {
    if (*ptr2 == '>') {
      *ptr1++ = *ptr2;
      break;
    }
    *ptr1++ = *ptr2++;
  }
  *ptr1 = '\0';

  if (!refer_message[0]) {
    return(1);
  }
  create_temp_name(tmp_file1, "NC");
  create_temp_name(tmp_file2, "NV");
  fp = fopen2(tmp_file1, "w");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't open reference file.");
    return(1);
  }
  status = nntp_article2((CASTPTR)refer_message, fp);
  fclose(fp);
  if (status != NNTP_OK) {
    funlink2(tmp_file1);
    return(1);
  }
  status = exec_pager(tmp_file1, tmp_file2, 2, refer_message);
  funlink2(tmp_file1);
  return(status);
}

#ifndef	SMALL
/*
 * Message-ID 検索
 */

static int	news_find(message_id)
     char	*message_id;
{
  FILE	*fp;
  char	tmp_file1[PATH_BUFF];
  char	tmp_file2[PATH_BUFF];
  int	status;

  create_temp_name(tmp_file1, "NC");
  create_temp_name(tmp_file2, "NV");
  fp = fopen2(tmp_file1, "w");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't open reference file.");
    return(1);
  }
  status = nntp_article2((CASTPTR)message_id, fp);
  fclose(fp);
  if (status != NNTP_OK) {
    funlink2(tmp_file1);
    return(1);
  }
  status = exec_pager(tmp_file1, tmp_file2, 2, message_id);
  funlink2(tmp_file1);
  return(status);
}
#endif	/* !SMALL */

/*
 * 前記事番号取得
 */

static int	news_prev_article(current_article)
     int	current_article;
{
  if (--current_article < 0) {
    current_article = 0;
  }
  return(current_article);
}

/*
 * 次記事番号取得
 */

static int	news_next_article(current_article)
     int	current_article;
{
  if (++current_article >= news_article_number) {
    if (news_article_number > 0) {
      current_article = news_article_number - 1;
    } else {
      current_article = 0;
    }
  }
  return(current_article);
}

/*
 * 前未読記事番号取得
 */

static int	news_prev_unread_article(current_article)
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

static int	news_next_unread_article(current_article)
     int	current_article;
{
  term_enable_abort();
  while (current_article < (news_article_number - 1)) {
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

/*
 * 前未読記事番号取得(キャンセル記事は自動マーク)
 */

static int	news_prev_unread_article2(current_group, current_article)
     int	current_group;
     int	current_article;
{
  register int	current_article2, current_article3;
  int		current_article4;

  current_article2 = current_article;
  if (news_article_number > 0) {
    while (1) {
      if (article_list[current_article].mark & READ_MARK) {
	if (!current_article) {
	  break;
	}
	current_article = news_prev_unread_article(current_article);
      } else {
	if (article_list[current_article].mark & UNFETCH_MARK) {
	  news_get_field(current_group, current_article);
	}
	if (!(article_list[current_article].mark & CANCEL_MARK)) {
	  break;
	}
	article_list[current_article].mark |= READ_MARK;
      }
    }
    if (article_list[current_article].mark & READ_MARK) {
      current_article3 = current_article;
    } else {
      current_article3 = current_article + 1;
    }
    if (current_article3 <= current_article2) {
      if (news_thread_mode) {
	while (current_article3 <= current_article2) {
	  current_article4 = current_article3;
	  while (current_article3 < current_article2) {
	    if (article_list[current_article3].real_number + 1
		!= article_list[current_article3 + 1].real_number) {
	      break;
	    }
	    current_article3++;
	  }
	  news_add_mark(current_group,
			article_list[current_article4].real_number,
			article_list[current_article3++].real_number);
	}
      } else {
	news_add_mark(current_group,
		      article_list[current_article3].real_number,
		      article_list[current_article2].real_number);
      }
    }
  } else {
    news_add_mark(current_group,
		  news_group[current_group].min_article,
		  news_group[current_group].max_article);
  }
  return(current_article);
}

/*
 * 次未読記事番号取得(キャンセル記事は自動マーク)
 */

static int	news_next_unread_article2(current_group, current_article)
     int	current_group;
     int	current_article;
{
  register int	current_article2, current_article3;
  int		current_article4;

  current_article2 = current_article;
  if (news_article_number > 0) {
    while (1) {
      if (article_list[current_article].mark & READ_MARK) {
	if (current_article >= news_article_number - 1) {
	  break;
	}
	current_article = news_next_unread_article(current_article);
      } else {
	if (article_list[current_article].mark & UNFETCH_MARK) {
	  news_get_field(current_group, current_article);
	}
	if (!(article_list[current_article].mark & CANCEL_MARK)) {
	  break;
	}
	article_list[current_article].mark |= READ_MARK;
      }
    }
    if (article_list[current_article].mark & READ_MARK) {
      current_article3 = current_article;
    } else {
      current_article3 = current_article - 1;
    }
    if (current_article3 >= current_article2) {
      if (news_thread_mode) {
	while (current_article2 <= current_article3) {
	  current_article4 = current_article2;
	  while (current_article2 < current_article3) {
	    if (article_list[current_article2].real_number + 1
		!= article_list[current_article2 + 1].real_number) {
	      break;
	    }
	    current_article2++;
	  }
	  news_add_mark(current_group,
			article_list[current_article4].real_number,
			article_list[current_article2++].real_number);
	}
      } else {
	news_add_mark(current_group,
		      article_list[current_article2].real_number,
		      article_list[current_article3].real_number);
      }
    }
  } else {
    news_add_mark(current_group,
		  news_group[current_group].min_article,
		  news_group[current_group].max_article);
  }
  return(current_article);
}

/*
 * 未読記事リスト取得
 */

static int	news_get_list(current_group)
     int	current_group;
{
  MARK_LIST	*mark_ptr;
  int		min_number;
  register int	i, j;

  min_number = news_group[current_group].min_article;
  news_article_number = news_group[current_group].max_article
    - min_number + 1;
  multi_number = 0;
  base_message[0] = refer_message[0] = '\0';
  if (news_article_number <= 0) {
    news_article_number = 0;
    return(1);
#ifdef	NEWS_MAX_ARTICLE
  } else if (news_article_number > NEWS_MAX_ARTICLE) {
    news_article_number = NEWS_MAX_ARTICLE;
    min_number = news_group[current_group].max_article
      - news_article_number + 1;
#endif	/* NEWS_MAX_ARTICLE */
  }
  if (article_list != (ARTICLE_LIST*)NULL) {
    free(article_list);
  }
  article_list = (ARTICLE_LIST*)malloc(sizeof(ARTICLE_LIST)
				       * news_article_number);
  if (article_list == (ARTICLE_LIST*)NULL) {
    print_fatal("Can't allocate memory for article struct.");
    news_article_number = 0;
    return(1);
  }
  for (i = 0; i < news_article_number; i++) {
    article_list[i].real_number = min_number + i;
    article_list[i].mark = UNFETCH_MARK;	/* ヘッダ未取得	*/
  }

  /*	既読マーク処理		*/

  mark_ptr = group_list[current_group].mark_ptr;
  while (mark_ptr != (MARK_LIST*)NULL) {
    if ((mark_ptr->end_number >= news_group[current_group].min_article) &&
	(mark_ptr->start_number <= news_group[current_group].max_article)) {
      for (i = mark_ptr->start_number; i <= mark_ptr->end_number; i++) {
	j = i - min_number;
	if ((j >= 0) && (j < news_article_number)) {
	  article_list[j].mark |= READ_MARK;
	}
      }
    }
    mark_ptr = mark_ptr->next_ptr;
  }

  /*	ソート処理	*/

#ifdef	REF_SORT
  if (message_list != (MESSAGE_LIST*)NULL) {
    print_fatal("Unexpected message list allocation found.");
    free(message_list);
    message_list = (MESSAGE_LIST*)NULL;
  }
  if (news_article_number && (sort_rule == 1)) {
    message_list = (MESSAGE_LIST*)malloc(sizeof(MESSAGE_LIST)
					 * news_article_number);
  }
#endif	/* REF_SORT */
  if (sort_articles(current_group, &news_article_number,
		    news_thread_mode, news_article_mask,
		    news_get_field, news_add_mark) < 0) {
    return(-1);
  }

  /*	パック処理	*/

  if (news_article_mask) {
    pack_articles(current_group, &news_article_number,
		  news_get_field, news_add_mark);
  }
  return(0);
}

/*
 * 記事ヘッダ取得
 */

static int	news_get_field(current_group, current_article)
     int	current_group;
     int	current_article;		/*	記事通し番号	*/
{
  static char		from_buff[BUFF_SIZE];
  static char		date_buff[MAX_FIELD_LEN];
  static char		subject_buff[BUFF_SIZE];
#ifdef	RECOVER_SUBJECT
  static char		recover_buff[BUFF_SIZE];
#endif	/* RECOVER_SUBJECT */
  static char		x_nsubj_buff[MAX_FIELD_LEN];
  static char		line_buff[MAX_FIELD_LEN];
#ifdef	REF_SORT
  static char		message_buff[MAX_FIELD_LEN];
  static char		reference_buff[BUFF_SIZE * 4];
#endif	/* REF_SORT */
  static struct cpy_hdr	news_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
    {DATE_FIELD,	date_buff,	sizeof(date_buff)},
#ifdef	RECOVER_SUBJECT
    {SUBJECT_FIELD,	recover_buff,	sizeof(recover_buff)},
#else	/* !RECOVER_SUBJECT */
    {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
#endif	/* !RECOVER_SUBJECT */
    {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
    {LINE_FIELD,	line_buff,	sizeof(line_buff)},
#ifdef	REF_SORT
    {MESSAGE_FIELD,	message_buff,	sizeof(message_buff)},
    {REFERENCE_FIELD,	reference_buff,	sizeof(reference_buff)},
#endif	/* REF_SORT */
  };
#ifdef	REF_SORT
  char			*ptr;
#else	/* !REF_SORT */
  short			year;
#endif	/* !REF_SORT */
#if	defined(LARGE) && (!defined(JUST_KILL))
  KILL_LIST		*kill_ptr;
#endif	/* (LARGE || (!JUST_KILL)) */
  short			day, hour, minute, second;

  if ((current_article < 0) || (current_article >= news_article_number)) {
    return(1);
  }
  article_list[current_article].mark &= ~UNFETCH_MARK;
  if (nntp_copy_fields(article_list[current_article].real_number, news_fields,
		       sizeof(news_fields) / sizeof(struct cpy_hdr),
		       CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK) != NNTP_OK) {
    article_list[current_article].mark |= CANCEL_MARK;
    return(-1);
  }

#if	defined(LARGE) && (!defined(JUST_KILL))
  /*
   * 自動マーク処理
   */

  kill_ptr = kill_list;
  while (kill_ptr != (KILL_LIST*)NULL) {
    if (!strcmp(kill_ptr->field, kill_ptr->mode ? from_buff : subject_buff)) {
      article_list[current_article].mark |= READ_MARK;
      if (current_group >= 0) {
	news_add_mark(current_group, article_list[current_article].real_number,
		      article_list[current_article].real_number);
      }
      break;
    }
    kill_ptr = kill_ptr->next_ptr;
  }
#endif	/* (LARGE || (!JUST_KILL)) */

  get_real_adrs(from_buff, article_list[current_article].from);
#ifdef	RECOVER_SUBJECT
  recover_jis(subject_buff, recover_buff);
#endif	/* RECOVER_SUBJECT */
  if (x_nsubj_mode && x_nsubj_buff[0]) {
    recover_jis(subject_buff, x_nsubj_buff);
  }
  mime_decode_func(from_buff, subject_buff, default_code);
  euc_tab_strncpy(article_list[current_article].subject, from_buff,
		  MAX_SUBJECT_LEN - 1);
  article_list[current_article].lines = atoi(line_buff);
#ifdef	REF_SORT
  convert_article_date(date_buff, &article_list[current_article].year,
		       &article_list[current_article].month,
		       &article_list[current_article].date, &day, &hour,
		       &minute, &second, from_buff);
  if (message_list != (MESSAGE_LIST*)NULL) {
    strncpy(message_list[current_article].msg_id, message_buff,
	    MAX_FIELD_LEN - 1);
    message_list[current_article].msg_id[MAX_FIELD_LEN - 1] = '\0';
    ptr = strrchr(reference_buff, '<');
    if (ptr != (char*)NULL) {
      strncpy(message_list[current_article].ref_id, ptr,
	      MAX_FIELD_LEN - 1);
      message_list[current_article].ref_id[MAX_FIELD_LEN - 1] = '\0';
      ptr = strchr(message_list[current_article].ref_id, '>');
      if (ptr != (char*)NULL) {
	*(ptr + 1) = '\0';
      }
    } else {
      message_list[current_article].ref_id[0] = '\0';
    }
  }
#else	/* !REF_SORT */
  convert_article_date(date_buff, &year, &article_list[current_article].month,
		       &article_list[current_article].date, &day, &hour,
		       &minute, &second, from_buff);
#endif	/* REF_SORT */
  return(0);
}

#ifndef	SMALL
/*
 * 記事サブジェクト検索(複数)
 */

int	search_subjects(mode, max_number, current_ptr, retrieve)
     int	mode;
     int	max_number;
     int	*current_ptr;
     int	(*retrieve)();
{
  register int	current_article;
  short		mark;
  int		found;
  char		*str;

  str = input_search_string(mode);
  if (str != (char*)NULL) {
    term_enable_abort();
    if (mode) {
      for (current_article = *current_ptr - 1; current_article >= 0;
	   current_article--) {
	mark = article_list[current_article].mark;
	found = search_subject(current_article, str, retrieve);
	if (check_abort()) {
	  print_mode_line(japanese ? "中断しました。" : "Aborted.");
	  *current_ptr = current_article;
	  article_list[current_article].mark = mark;
	  return(1);
	}
	if (found) {
	  print_mode_line(japanese ? "見つけました。" : "Search succeed.");
	  *current_ptr = current_article;
	  term_disable_abort();
	  return(0);
	}
      }
    } else {
      for (current_article = *current_ptr + 1; current_article < max_number;
	   current_article++) {
	mark = article_list[current_article].mark;
	found = search_subject(current_article, str, retrieve);
	if (check_abort()) {
	  print_mode_line(japanese ? "中断しました。" : "Aborted.");
	  *current_ptr = current_article;
	  article_list[current_article].mark = mark;
	  return(1);
	}
	if (found) {
	  print_mode_line(japanese ? "見つけました。" : "Search succeed.");
	  *current_ptr = current_article;
	  term_disable_abort();
	  return(0);
	}
      }
    }
    print_mode_line(japanese ? "見つかりませんでした。" : "Search failed.");
    term_disable_abort();
    return(1);
  }
  return(0);
}

/*
 * 記事サブジェクト検索(単一)
 */

int	search_subject(article_number, str, retrieve)
     int	article_number;
     char	*str;
     int	(*retrieve)();
{
  if (article_list[article_number].mark & UNFETCH_MARK) {
    if (retrieve != (int (*)())NULL) {
      retrieve(-1, article_number);
    }
  }
  if (article_list[article_number].mark & CANCEL_MARK) {
    return(0);
  }
  if (strindex(article_list[article_number].from, str)) {
    return(1);
  }
  if (strindex(article_list[article_number].subject, str)) {
    return(1);
  }
  return(0);
}

/*
 * 同一サブジェクトマーク
 */

void	kill_subjects(current_group, article_number, max_article,
		      retrieve, mark)
     int	current_group;
     int	article_number;
     int	max_article;
     int	(*retrieve)();
     void	(*mark)();
{
  char		*ptr1, *ptr2;
  register int	i;

  ptr1 = article_list[article_number].subject;
  if (!*ptr1) {
    return;
  }
  if ((*ptr1 == ' ') && 
      ((*(ptr1 + 1) == ' ') || (*(ptr1 + 1) == THREAD_CHAR))) {
    ptr1 += 2;
    while (*ptr1 == THREAD_CHAR) {
      ptr1++;
    }
    if (*ptr1 == ' ') {
      ptr1++;
    }
    if (*ptr1 == ' ') {
      ptr1++;
    }
  } else if (!strncasecmp(ptr1, REPLY_SUBJECT, sizeof(REPLY_SUBJECT) - 1)) {
    ptr1 += (sizeof(REPLY_SUBJECT) - 1);
    if (*ptr1 == ' ') {
      ptr1++;
    }
  }
  for (i = article_number; i < max_article; i++) {
    if (article_list[i].mark & UNFETCH_MARK) {
      if (retrieve != (int (*)())NULL) {
	retrieve(-1, i);
      }
    }
    if (!(article_list[i].mark & CANCEL_MARK)) {
      ptr2 = article_list[i].subject;
      if ((*ptr2 == ' ') && 
	  ((*(ptr2 + 1) == ' ') || (*(ptr2 + 1) == THREAD_CHAR))) {
	ptr2 += 2;
	while (*ptr2 == THREAD_CHAR) {
	  ptr2++;
	}
	if (*ptr2 == ' ') {
	  ptr2++;
	}
	if (*ptr2 == ' ') {
	  ptr2++;
	}
      } else if (!strncasecmp(ptr2, REPLY_SUBJECT,
			      sizeof(REPLY_SUBJECT) - 1)) {
	ptr2 += (sizeof(REPLY_SUBJECT) - 1);
	if (*ptr2 == ' ') {
	  ptr2++;
	}
      }
      if (!strcmp(ptr1, ptr2)) {
	article_list[i].mark |= READ_MARK;
	if (mark) {
	  mark(current_group, article_list[i].real_number,
	       article_list[i].real_number);
	}
      }
    }
  }
}
#endif	/* !SMALL */

#ifdef	LARGE
/*
 * 記事全体検索(複数)
 */

int	search_articles(mode, max_number, current_ptr, extract)
     int	mode;
     int	max_number;
     int	*current_ptr;
     int	(*extract)();
{
  register int	current_article;
  short		mark;
  int		found;
  char		*str;

  str = input_search_string(mode);
  if (str != (char*)NULL) {
    term_enable_abort();
    if (mode) {
      for (current_article = *current_ptr - 1; current_article >= 0;
	   current_article--) {
	mark = article_list[current_article].mark;
	found = search_article(current_article, str, extract);
	if (check_abort()) {
	  print_mode_line(japanese ? "中断しました。" : "Aborted.");
	  *current_ptr = current_article;
	  article_list[current_article].mark = mark;
	  return(1);
	}
	if (found) {
	  print_mode_line(japanese ? "見つけました。" : "Search succeed.");
	  *current_ptr = current_article;
	  term_disable_abort();
	  return(0);
	}
      }
    } else {
      for (current_article = *current_ptr + 1; current_article < max_number;
	   current_article++) {
	mark = article_list[current_article].mark;
	found = search_article(current_article, str, extract);
	if (check_abort()) {
	  print_mode_line(japanese ? "中断しました。" : "Aborted.");
	  *current_ptr = current_article;
	  article_list[current_article].mark = mark;
	  return(1);
	}
	if (found) {
	  print_mode_line(japanese ? "見つけました。" : "Search succeed.");
	  *current_ptr = current_article;
	  term_disable_abort();
	  return(0);
	}
      }
    }
    print_mode_line(japanese ? "見つかりませんでした。" : "Search failed.");
    term_disable_abort();
    return(1);
  }
  return(0);
}

/*
 * 記事全体検索(単一)
 */

int	search_article(article_number, str, extract)
     int	article_number;
     char	*str;
     int	(*extract)();
{
  FILE	*fp;
  char	tmp_file[PATH_BUFF];
  char	buff1[BUFF_SIZE];
  char	buff2[BUFF_SIZE];
  int	status;

  create_temp_name(tmp_file, "AS");
  if (extract(article_list[article_number].real_number, tmp_file)) {
    return(0);
  }
  status = 0;
  fp = fopen(tmp_file, "r");
  if (fp != (FILE*)NULL) {
    while (fgets(buff1, sizeof(buff1), fp)) {
      to_euc(buff2, buff1, default_code);
      if (strindex(buff2, str)) {
	status = 1;
	break;
      }
    }
    fclose(fp);
  }
  funlink2(tmp_file);
  return(status);
}
#endif	/* LARGE */

/*
 * マルチマーク設定/解除
 */

void	multi_mark(top_position, article_number)
     int	top_position;
     int	article_number;
{
  int	i, j;

  if (!(article_list[article_number].mark & CANCEL_MARK)) {
    if ((article_list[article_number].mark & MULTI_MARK) ||
	(multi_number < MAX_MULTI_NUMBER)) {
      toggle_mark(top_position, article_number, MULTI_MARK);
      if (article_list[article_number].mark & MULTI_MARK) {
	multi_list[multi_number++] = article_number;
      } else {
	for (i = 0; i < multi_number; i++) {
	  if (multi_list[i] == article_number) {
	    multi_number--;
	    for (j = i; j < multi_number; j++) {
	      multi_list[j] = multi_list[j + 1];
	    }
	    break;
	  }
	}
      }
    }
  }
}

/*
 * マルチマーク解除
 */

void	multi_clear(max_article)
     int	max_article;
{
  int	i;

  multi_number = 0;
  if (article_list != (ARTICLE_LIST*)NULL) {
    for (i = 0; i < max_article; i++) {
      article_list[i].mark &= ~MULTI_MARK;
    }
  }
}

/*
 * 複数記事マーク,マルチマーク消去
 */

void	multi_add_mark(max_article, current_group, current_article,
		       mark, add_mark)
     int	max_article;
     int	current_group;
     int	current_article;
     int	mark;
     int	(*add_mark)();
{
  register int	i;

  if (article_list != (ARTICLE_LIST*)NULL) {
    if (multi_number) {
      for (i = 0; i < max_article; i++) {
	if (article_list[i].mark & MULTI_MARK) {
	  if (current_group >= 0) {
	    add_mark(current_group, i, mark);
	  } else {
	    add_mark(i, mark);
	  }
	}
      }
    } else {
      if (current_group >= 0) {
	add_mark(current_group, current_article, mark);
      } else {
	add_mark(current_article, mark);
      }
    }
  }
  multi_clear(max_article);
}

#if	defined(NEWSPOST) || defined(MAILSEND)
/*
 * 複数記事抽出
 */

int	multi_extract(current_article, tmp_file, extract)
     int	current_article;
     char	*tmp_file;
     int	(*extract)();
{
  FILE		*fp;
  register int	i, j;

  j = 0;
  for (i = 0; i < multi_number; i++) {
    if (j) {
      fclose2(tmp_file);
    }
    if (extract(article_list[multi_list[i]].real_number, tmp_file)) {
      return(1);
    }
    fp = fopen(tmp_file, "a");
    if (fp == (FILE*)NULL) {
      funlink2(tmp_file);
      return(1);
    }
    fprintf(fp, "%s", MULTI_END_MARK);
    fclose(fp);
    j = 1;
  }
  if (!j) {
    if (extract(article_list[current_article].real_number, tmp_file)) {
      return(1);
    }
  }
  return(0);
}
#endif	/* (NEWSPOST || MAILSEND) */

/*
 * 記事一覧表示
 */

void	print_articles(top_position, group, max_article, retrieve,
		       lost_message)
     int	top_position;
     int	group;
     int	max_article;
     int	(*retrieve)();
     char	*lost_message;
{
  register int	i, j, k, l;
  char		buff[BUFF_SIZE];
#ifdef	JNAMES
  char		jfrom[BUFF_SIZE];
  char		*ptr;
#endif	/* JNAMES */
#ifdef	COLOR
  int		color_type;
#endif	/* COLOR */


  if (!wide_mode) {
    l = (check_new_mail() > 0);
    term_locate(0, 2);
    term_attrib(RESET_ATTRIB);
#ifdef	COLOR
    term_attrib(color_code[HEADER_COLOR]);
#else	/* !COLOR */
    term_attrib(REVERSE_ATTRIB);
#endif	/* !COLOR */
    if (max_article > 0) {
      i = j = article_list[0].real_number;
      for (k = 0; k < max_article; k++) {
	if (i > article_list[k].real_number) {
	  i = article_list[k].real_number;
	}
	if (j < article_list[k].real_number) {
	  j = article_list[k].real_number;
	}
      }
    } else {
      i = j = 0;
    }
    switch (article_format) {
    case 0:
      print_full_line(japanese ?
		      "マーク 番号 日付  行数 差出人       サブジェクト名 [%-8.8s] (%d-%d)" :
		      "Mark   No.  Date  Line From         Subject        [%-8.8s] (%d-%d)",
		      new_mail_string[l], i, j);
      break;
    case 1:
      print_full_line(japanese ?
		      "マーク 日付  差出人       サブジェクト名 [%-8.8s] (%d-%d)" :
		      "Mark   Date  From         Subject        [%-8.8s] (%d-%d)",
		      new_mail_string[l], i, j);
      break;
    case 2:
      print_full_line(japanese ?
		      "マーク サブジェクト名 [%-8.8s] (%d-%d)" :
		      "Mark   Subject        [%-8.8s] (%d-%d)",
		      new_mail_string[l], i, j);
      break;
    default:
      break;
    }
  }
  term_attrib(RESET_ATTRIB);
  switch (article_format) {
  case 0:
    l = term_columns - 36;
    break;
  case 1:
    l = term_columns - 26;
    break;
  case 2:
  default:
    l = term_columns - 7;
    break;
  }
  for (i = 0, j = top_position; i < term_lines - mode_lines; i++, j++) {
    if (j >= max_article) {
      break;
    }
    if (article_list[j].mark & UNFETCH_MARK) {
      if (retrieve != (int (*)())NULL) {
	retrieve(group, j);
      }
    }
    toggle_mark(top_position, j, 0);
    if (!(article_list[j].mark & UNFETCH_MARK)) {
      if (article_format) {
	cprintf("  ");
      } else {
#ifdef	COLOR
	term_attrib(color_code[NUMBER_COLOR]);
#endif	/* COLOR */
	cprintf("%6d ", article_list[j].real_number);
      }
      if (article_list[j].mark & CANCEL_MARK) {
#ifdef	COLOR
	term_attrib(color_code[LOST_COLOR]);
#endif	/* COLOR */
	cprintf("           %s\n", lost_message);
      } else {
#ifdef	COLOR
	color_type = FROM_COLOR;
#endif	/* COLOR */
#ifdef	JNAMES
	strcpy(buff, article_list[j].from);
	ptr = buff;
	k = 12;
	if (*ptr == MY_ADRS_CHAR) {
#ifdef	COLOR
	  color_type = TO_COLOR;
#endif	/* COLOR */
	  ptr++;
	  k--;
	}
#ifdef	COLOR
	if (get_jadrs(jfrom, ptr)) {
	  color_type = JNAMES_COLOR;
	}
#else	/* !COLOR */
	get_jadrs(jfrom, ptr);
#endif	/* !COLOR */
	euc_strncpy2(ptr, jfrom, k);
#else	/* !JNAMES */
	sprintf(buff, "%-12.12s", article_list[j].from);
#ifdef	COLOR
	if (buff[0] == MY_ADRS_CHAR) {
	  color_type = TO_COLOR;
	}
#endif	/* COLOR */
#endif	/* JNAMES */
	switch (article_format) {
	case 0:
#ifdef	COLOR
	  term_attrib(color_code[DATE_COLOR]);
#endif	/* COLOR */
	  cprintf("%02d/%02d", (int)article_list[j].month,
		  (int)article_list[j].date);
#ifdef	COLOR
	  term_attrib(color_code[LINE_COLOR]);
#endif	/* COLOR */
	  cprintf("%5d ", (int)article_list[j].lines);
#ifdef	COLOR
	  term_attrib(color_code[color_type]);
#endif	/* COLOR */
	  euc_printf("%s ", buff);
	  break;
	case 1:
#ifdef	COLOR
	  term_attrib(color_code[DATE_COLOR]);
#endif	/* COLOR */
	  cprintf("%02d/%02d ", (int)article_list[j].month,
		  (int)article_list[j].date);
#ifdef	COLOR
	  term_attrib(color_code[color_type]);
#endif	/* COLOR */
	  euc_printf("%s ", buff);
	  break;
	case 2:
	default:
	  break;
	}
#ifdef	COLOR
	if (article_list[j].mark & THREAD_MARK) {
	  term_attrib(color_code[THREAD_COLOR]);
	} else {
	  term_attrib(color_code[SUBJECT_COLOR]);
	}
#endif	/* COLOR */
	euc_strncpy(buff, article_list[j].subject, l);
	euc_printf("%s", buff);
      }
    }
  }
  term_attrib(RESET_ATTRIB);
}

/*
 * マーク反転/表示
 */

void	toggle_mark(top_position, current_article, mark)
     int	top_position;
     int	current_article;
     int	mark;
{
  static char	mark_buff[8];
  char		*ptr;
  register int	m;

  m = (article_list[current_article].mark ^= mark);
  term_locate(0, head_lines + current_article - top_position);
  ptr = mark_buff;
#ifdef	COLOR
  term_attrib(color_code[MARK_COLOR]);
#endif	/* COLOR */
  if (m & MULTI_MARK) {
    *ptr++ = 'M';
  } else {
    *ptr++ = ' ';
  }
  if (m & READ_MARK) {
    *ptr++ = 'R';
  } else {
    *ptr++ = ' ';
  }
  if (m & DELETE_MARK) {
    *ptr++ = 'D';
  } else {
    *ptr++ = ' ';
  }
  if (m & FORWARD_MARK) {
    *ptr++ = 'F';
  } else {
    *ptr++ = ' ';
  }
  if (m & ANSWER_MARK) {
    *ptr++ = 'A';
  } else {
    *ptr++ = ' ';
  }
  *ptr = '\0';
  cprintf(mark_buff);
#ifdef	COLOR
  term_attrib(RESET_ATTRIB);
#endif	/* COLOR */
}

/*
 * 複数記事セーブ
 */

int	multi_save(current_article, extract)
     int	current_article;
     int	(*extract)();
{
  char	file_name[PATH_BUFF];
  int	i, j;
  int	line, total;
  int	status;
#ifdef	MSDOS
  char	*ptr;
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
  input_line(INPUT_EXPAND_MASK | INPUT_COMP_MASK,
	     "ファイル名を入力して下さい:", "Input file name:", file_name);
  if (!file_name[0]) {
    return(1);
  }
  status = 0;
  if (multi_number) {
    j = strlen(file_name);
    total = line = 0;
    for (i = 0; i < multi_number; i++) {
      total += (int)article_list[multi_list[i]].lines;
    }
    for (i = 0; i < multi_number; i++) {
      print_mode_line(japanese ?
		      "[%d/%d]-(%d/%d) セーブ中です。" :
		      "[%d/%d]-(%d/%d) Saving.",
		      i + 1, multi_number, line, total);
      sprintf(&file_name[j], "%02d", i + 1);
      if (save_article(multi_list[i], extract, file_name)) {
	status = 1;
      }
      line += (int)article_list[multi_list[i]].lines;
    }
  } else {
    print_mode_line(japanese ? "セーブ中です。" : "Saving.");
    status = save_article(current_article, extract, file_name);
  }
  if (status) {
    print_mode_line(japanese ? "記事がセーブできませんでした。" :
		    "Save article failed.");
    term_bell();
    sleep(ERROR_SLEEP);
  }
  return(status);
}

/*
 * 記事セーブ(下位関数)
 *
 * 備考:article_number が負数なら extract には例外的に抽出関数ではなく
 *      ファイル名が渡される。
 */

int	save_article(article_number, extract, file_name)
     int	article_number;
     int	(*extract)();
     char	*file_name;
{
  struct stat	stat_buff;
  struct tm	*tm;
  FILE		*fp1;
  FILE		*fp2;
  char		tmp_file1[PATH_BUFF];
  char		tmp_file2[PATH_BUFF];
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  int		status;
  time_t	now_time;
  int		replace;

  if (!stat(file_name, &stat_buff)) {
    switch (yes_or_no(FILE_YN_MODE,
		      "ファイルが存在します。アペンドしますか?",
		      "File exists.Append?")) {
    case 2:
      unlink(file_name);
      break;
    case 1:
      break;
    case 0:
    default:
      return(0);
      /* break; */
    }
  }
  status = 1;
  if (article_number >= 0) {
    create_temp_name(tmp_file1, "AS");
    if (extract(article_list[article_number].real_number, tmp_file1)) {
      return(status);
    }
  } else {
    strcpy(tmp_file1, (char*)extract);	/* 汚い処理です */
  }
  create_temp_name(tmp_file2, "SD");
  if (save_code == ASCII_CODE) {
    convert_code(tmp_file1, tmp_file2, save_code, 0);
  } else {
    convert_code(tmp_file1, tmp_file2, save_code, 4);
  }
  fp1 = fopen(tmp_file2, "r");
  funlink2(tmp_file2);
  if (fp1 != (FILE*)NULL) {
    fp2 = fopen(file_name, "a");
    if (fp2 != (FILE*)NULL) {
      if (unixfrom_mode) {
	buff2[0] = '\0';
	while (fgets(buff1, sizeof(buff1), fp1)) {
	  if (status == 1) {
	    status = 2;
	    if (!strncmp(buff1, MSPL_SEPARATER,
			 sizeof(MSPL_SEPARATER) - 1)) {
	      break;
	    }
	  }
	  copy_field(buff1, buff2, FROM_FIELD);
	  if ((!buff1[0]) || (buff1[0] == '\n')) {
	    break;
	  }
	}
	fseek(fp1, 0L, 0);
	if (buff2[0]) {
	  get_real_adrs(buff2, buff1);
	  now_time = time((time_t*)NULL);
	  tm = localtime(&now_time);
	  fprintf(fp2, "%s%s %s %s %2d %02d:%02d:%02d 19%d\n",
		  MSPL_SEPARATER, buff1, day_string[tm->tm_wday + 1],
		  month_string[tm->tm_mon + 1], tm->tm_mday, tm->tm_hour,
		  tm->tm_min, tm->tm_sec, tm->tm_year);
	}
      }
      replace = 0;
      while (fgets(buff1, sizeof(buff1), fp1)) {
	if (replace && (!strncmp(buff1, MSPL_SEPARATER,
				 sizeof(MSPL_SEPARATER) - 1))) {
	  fputc('>', fp2);
	}
	fputs(buff1, fp2);
	replace = unixfrom_mode;
      }
      if (unixfrom_mode) {
	fputc('\n', fp2);
      }
      status = fclose(fp2);
    }
    fclose(fp1);
  }
  if (article_number >= 0) {
    funlink2(tmp_file1);
  }
  return(status);
}

#ifndef	SMALL
/*
 * 複数記事パイプ実行
 */

int	multi_pipe(current_article, extract)
     int	current_article;
     int	(*extract)();
{
  FILE	*fp;
  char	tmp_file[PATH_BUFF];
  char	buff[BUFF_SIZE];
  char	*ptr;
  int	i;
  int	line, total;
  int	status;

  buff[0] = '\0';
  input_line(0, "パイプコマンドを入力して下さい:|",
	     "Input pipe command:|", buff);
  ptr = buff;
  while ((*ptr == ' ') || (*ptr == '\t')) {
    ptr++;
  }
  if (!(*ptr)) {
    return(1);
  }
  create_temp_name(tmp_file, "AP");
  status = 0;
#ifdef	SIGPIPE
  signal(SIGPIPE, pipe_error);
#endif	/* SIGPIPE */
  term_init(1);
  fp = popen(ptr, "w");
  if (fp != (FILE*)NULL) {
    if (multi_number) {
      total = line = 0;
      for (i = 0; i < multi_number; i++) {
	total += (int)article_list[multi_list[i]].lines;
      }
      for (i = 0; i < multi_number; i++) {
	print_mode_line(japanese ?
			"[%d/%d]-(%d/%d) パイプ処理実行中です。" :
			"[%d/%d]-(%d/%d) Executing pipe command.",
			i + 1, multi_number, line, total);
	if (pipe_article(fp, multi_list[i], extract, tmp_file)) {
	  status = 1;
	}
	line += (int)article_list[multi_list[i]].lines;
      }
    } else {
      print_mode_line(japanese ?
		      "パイプ処理実行中です。" : "Executing pipe command.");
      status = pipe_article(fp, current_article, extract, tmp_file);
    }
    if (pclose(fp)) {
      status = 1;
    }
    term_init(2);
  } else {
    term_init(2);
    print_fatal("Can't execute pipe command.");
  }
#ifdef	SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif	/* SIGPIPE */
  if (status) {
    print_mode_line(japanese ? "パイプ処理に失敗しました。" :
		    "Pipe command error.");
    term_bell();
    sleep(ERROR_SLEEP);
    return(1);
  }
  return(0);
}

#ifdef	LARGE
#ifdef	JUST_KILL
/*
 * 記事全マーク
 *
 * 備考:XOVER を使用するとフィールドが正確に取得できないため本関数は敢えて
 *	XOVER 対応していません。
 */

static void	kill_articles(mode, current_group, current_article)
     int	mode;	/* マークモード(0:Subject,1:From)	*/
     int	current_group;
     int	current_article;
{
  char			buff1[BUFF_SIZE];
  char			buff2[BUFF_SIZE];
  static struct cpy_hdr	subject_fields[] = {
    {SUBJECT_FIELD,	(char*)NULL,	sizeof(buff1)},
  };
  static struct cpy_hdr	from_fields[] = {
    {FROM_FIELD,	(char*)NULL,	sizeof(buff1)},
  };
  struct cpy_hdr	*kill_fields;
  MARK_LIST		*mark_ptr;
  int			number;
  register int		i, j, k;

  if (mode) {
    kill_fields = from_fields;
  } else {
    kill_fields = subject_fields;
  }
  kill_fields->field_buff = buff1;
  if (nntp_copy_fields(article_list[current_article].real_number,
		       kill_fields, 1,
		       CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK) != NNTP_OK) {
    return;
  }
  kill_fields->field_buff = buff2;
  for (i = 0; i < group_number; i++) {
    if ((!group_list[i].unsubscribe) || (!group_mask)) {
      print_mode_line(japanese ? "記事をマーク中です。(グループ:%s)" :
		      "Marking articles.(group:%s)",
		      news_group[i].group_name);
      if (nntp_group(news_group[i].group_name) == NNTP_OK) {
	number = news_group[i].max_article - news_group[i].min_article + 1;
	if (number > max_thread_number) {
	  number = max_thread_number;
	}
	mark_ptr = group_list[i].mark_ptr;
	for (j = news_group[i].max_article - number + 1;
	     j <= news_group[i].max_article; j++) {

	  /*
	   * 既読記事はチェックを省略する
	   */

	  while (mark_ptr != (MARK_LIST*)NULL) {
	    if (mark_ptr->end_number < j) {
	      mark_ptr = mark_ptr->next_ptr;
	    } else {
	      break;
	    }
	  }
	  if (mark_ptr && (j >= mark_ptr->start_number) &&
	      (j <= mark_ptr->end_number)) {
	    continue;
	  }
	  if (nntp_copy_fields(j, kill_fields, 1,
			       CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK)
	      == NNTP_OK) {
	    if (!strcmp(buff1, buff2)) {
	      if (i == current_group) {
		for (k = 0; k < news_article_number; k++) {
		  if (article_list[k].real_number == j) {
		    article_list[k].mark |= READ_MARK;
		  }
		}
	      }
	      news_add_mark(i, j, j);
	      mark_ptr = group_list[i].mark_ptr;
	    }
	  }
	}
      }
    }
  }
  nntp_group(select_name);
}
#else	/* !JUST_KILL */
/*
 * 記事全マーク
 */

static void	kill_articles(mode, current_group, current_article)
     int	mode;	/* マークモード(0:Subject,1:From)	*/
     int	current_group;
     int	current_article;
{
  char			buff1[BUFF_SIZE];
  char			buff2[BUFF_SIZE];
  static struct cpy_hdr	subject_fields[] = {
    {SUBJECT_FIELD,	(char*)NULL,	sizeof(buff1)},
  };
  static struct cpy_hdr	from_fields[] = {
    {FROM_FIELD,	(char*)NULL,	sizeof(buff1)},
  };
  struct cpy_hdr	*kill_fields;
  register int		i;

  if (mode) {
    kill_fields = from_fields;
  } else {
    kill_fields = subject_fields;
  }
  kill_fields->field_buff = buff1;
  if (nntp_copy_fields(article_list[current_article].real_number,
		       kill_fields, 1,
		       CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK) != NNTP_OK) {
    return;
  }
  add_kill_list(mode, buff1);

  /*
   * カレントグループのマーク
   */

  kill_fields->field_buff = buff2;
  for (i = 0; i < news_article_number; i++) {
    if (!(article_list[i].mark & READ_MARK)) {
      if (nntp_copy_fields(article_list[i].real_number, kill_fields, 1,
			   CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK)
	  == NNTP_OK) {
	if (!strcmp(buff1, buff2)) {
	  article_list[i].mark |= READ_MARK;
	  news_add_mark(current_group, article_list[i].real_number,
			article_list[i].real_number);
	}
      }
    }
  }
}

/*
 * 自動マーク情報追加
 */

void	add_kill_list(mode, ptr)
     int	mode;
     char	*ptr;
{
  KILL_LIST	*kill_ptr;

  if (strlen(ptr) >= MAX_FIELD_LEN) {
    ptr[MAX_FIELD_LEN - 1] = '\0';
  }

  /*
   * 同じ情報が既に登録されていないかチェック
   */

  kill_ptr = kill_list;
  while (kill_ptr != (KILL_LIST*)NULL) {
    if ((kill_ptr->mode == mode) && (!strcmp(kill_ptr->field, ptr))) {
      break;
    }
    kill_ptr = kill_ptr->next_ptr;
  }

  if (kill_ptr == (KILL_LIST*)NULL) {
    kill_ptr = (KILL_LIST*)malloc(sizeof(KILL_LIST));
    if (kill_ptr == (KILL_LIST*)NULL) {
      print_fatal("Can't allocate memory for kill struct.");
      return;
    }
    kill_ptr->mode = mode;
    strcpy(kill_ptr->field, ptr);
    if (kill_list == (KILL_LIST*)NULL) {
      kill_list = kill_ptr;
      kill_ptr->next_ptr = (KILL_LIST*)NULL;
    } else {
      kill_ptr->next_ptr = kill_list;
      kill_list = kill_ptr;
    }
  }
}
#endif	/* !JUST_KILL */
#endif	/* LARGE */

/*
 * 記事パイプ実行(下位関数)
 */

static int	pipe_article(fp1, article_number, extract, tmp_file)
     FILE	*fp1;
     int	article_number;
     int	(*extract)();
     char	*tmp_file;
{
#ifdef	notdef
  FILE	*fp2;
  char	buff1[BUFF_SIZE];
  char	buff2[BUFF_SIZE];
  int	status;

  if (extract(article_list[article_number].real_number, tmp_file)) {
    funlink2(tmp_file);
    return(1);
  }
  status = 1;
  fp2 = fopen(tmp_file, "r");
  if (fp2 != (FILE*)NULL) {
    status = 0;
    while (fgets(buff1, sizeof(buff1), fp2)) {
      switch (pipe_code) {
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
      if (fputs(buff2, fp1) == EOF) {
	status = 1;
      }
    }
    fclose(fp2);
  }
#else	/* !notdef */
  FILE	*fp2;
  char	pipe_file[PATH_BUFF];
  char	buff[BUFF_SIZE];
  int	status;

  if (extract(article_list[article_number].real_number, tmp_file)) {
    funlink2(tmp_file);
    return(1);
  }
  create_temp_name(pipe_file, "PD");
  convert_code(tmp_file, pipe_file, pipe_code, 4);
  status = 1;
  fp2 = fopen(pipe_file, "r");
  if (fp2 != (FILE*)NULL) {
    status = 0;
    while (fgets(buff, sizeof(buff), fp2)) {
      if (fputs(buff, fp1) == EOF) {
	status = 1;
      }
    }
    fclose(fp2);
  }
  funlink2(pipe_file);
#endif	/* !notdef */
  funlink2(tmp_file);
  return(status);
}
#endif	/* !SMALL */

#ifdef	LARGE
/*
 * 複数記事印刷
 */

int	multi_print(current_article, extract)
     int	current_article;
     int	(*extract)();
{
  FILE		*fp;
  char		tmp_file[PATH_BUFF];
  KANJICODE	tmp_code;
  int		i;
  int		line, total;
  int		status;

  if (!yes_or_no(CARE_YN_MODE, "印刷してよろしいですか?",
		 "Print article?")) {
    return(1);
  }
  create_temp_name(tmp_file, "AP");
  status = 0;
#ifdef	SIGPIPE
  signal(SIGPIPE, pipe_error);
#endif	/* SIGPIPE */
  fp = popen(lpr_command, "w");
  if (fp != (FILE*)NULL) {
    tmp_code = pipe_code;
    pipe_code = lpr_code;
    if (multi_number) {
      total = line = 0;
      for (i = 0; i < multi_number; i++) {
	total += (int)article_list[multi_list[i]].lines;
      }
      for (i = 0; i < multi_number; i++) {
	print_mode_line(japanese ?
			"[%d/%d]-(%d/%d) 印刷中です。" :
			"[%d/%d]-(%d/%d) Printing.",
			i + 1, multi_number, line, total);
	if (pipe_article(fp, multi_list[i], extract, tmp_file)) {
	  status = 1;
	}
	line += (int)article_list[multi_list[i]].lines;
      }
    } else {
      print_mode_line(japanese ? "印刷中です。" : "Printing.");
      status = pipe_article(fp, current_article, extract, tmp_file);
    }
    pipe_code = tmp_code;
    if (pclose(fp)) {
      status = 1;
    }
  } else {
    print_fatal("Can't execute print out command.");
  }
#ifdef	SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif	/* SIGPIPE */
  if (status) {
    print_mode_line(japanese ? "印刷に失敗しました。" :
		    "Print out failed.");
    term_bell();
    sleep(ERROR_SLEEP);
    return(1);
  }
  return(0);
}
#endif	/* LARGE */

#ifdef	XOVER
int	xover_copy_field(current_group, ptr1)
     int	current_group;
     char	*ptr1;
{
  char		buff[XOVER_BUFF];
  char		*ptr2;
  register int	current_article;
#ifndef	REF_SORT
  short		year;
#endif	/* !REF_SORT */
  short		day, hour, minute, second;
#if	defined(LARGE) && (!defined(JUST_KILL))
  KILL_LIST		*kill_ptr;
#endif	/* (LARGE || (!JUST_KILL)) */

  current_article = atoi(ptr1) - article_list[0].real_number;
  if ((current_article < 0) || (current_article >= news_article_number)) {
    return(1);
  }
  while (isdigit(*ptr1)) {
    ptr1++;
  }
  if (*ptr1 == '\t') {
    ptr1++;
  } else {
    return(1);
  }
  ptr2 = ptr1;		/* ptr1 = Subject */
  while (*ptr2) {
    if (*ptr2 == '\t') {
      *ptr2++ = '\0';
      break;
    }
    ptr2++;
  }
#if	defined(LARGE) && (!defined(JUST_KILL))
  /*
   * 自動マーク処理
   */

  kill_ptr = kill_list;
  while (kill_ptr != (KILL_LIST*)NULL) {
    if ((!kill_ptr->mode) && (!strcmp(kill_ptr->field, ptr1))) {
      article_list[current_article].mark |= READ_MARK;
      if (current_group >= 0) {
	news_add_mark(current_group, article_list[current_article].real_number,
		      article_list[current_article].real_number);
      }
      break;
    }
    kill_ptr = kill_ptr->next_ptr;
  }
#endif	/* !JUST_KILL */
  mime_decode_func(buff, ptr1, default_code);
  euc_tab_strncpy(article_list[current_article].subject, buff,
		  MAX_SUBJECT_LEN - 1);
  ptr1 = ptr2;		/* ptr1 = From */
  while (*ptr2) {
    if (*ptr2 == '\t') {
      *ptr2++ = '\0';
      break;
    }
    ptr2++;
  }
#if	defined(LARGE) && (!defined(JUST_KILL))
  /*
   * 自動マーク処理
   */

  kill_ptr = kill_list;
  while (kill_ptr != (KILL_LIST*)NULL) {
    if (kill_ptr->mode && (!strcmp(kill_ptr->field, ptr1))) {
      article_list[current_article].mark |= READ_MARK;
      if (current_group >= 0) {
	news_add_mark(current_group, article_list[current_article].real_number,
		      article_list[current_article].real_number);
      }
      break;
    }
    kill_ptr = kill_ptr->next_ptr;
  }
#endif	/* !JUST_KILL */
  get_real_adrs(ptr1, article_list[current_article].from);
  ptr1 = ptr2;
  while (*ptr2) {
    if (*ptr2 == '\t') {
      *ptr2++ = '\0';
      break;
    }
    ptr2++;
  }
#ifdef	REF_SORT
  convert_article_date(ptr1,
		       &article_list[current_article].year, 
		       &article_list[current_article].month,
		       &article_list[current_article].date,
		       &day, &hour, &minute, &second, buff);
#else	/* !REF_SORT */
  convert_article_date(ptr1,
		       &year,
		       &article_list[current_article].month,
		       &article_list[current_article].date,
		       &day, &hour, &minute, &second, buff);
#endif	/* !REF_SORT */
  ptr1 = ptr2;
  while (*ptr2) {
    if (*ptr2 == '\t') {
      *ptr2++ = '\0';
      break;
    }
    ptr2++;
  }
#ifdef	REF_SORT
  if (message_list != (MESSAGE_LIST*)NULL) {
    strncpy(message_list[current_article].msg_id, ptr1, MAX_FIELD_LEN - 1);
    message_list[current_article].msg_id[MAX_FIELD_LEN - 1] = '\0';
  }
#endif	/* REF_SORT */
  ptr1 = ptr2;
  while (*ptr2) {
    if (*ptr2 == '\t') {
      *ptr2++ = '\0';
      break;
    }
    ptr2++;
  }
#ifdef	REF_SORT
  if (message_list != (MESSAGE_LIST*)NULL) {
    ptr1 = strrchr(ptr1, '<');
    if (ptr1 != (char*)NULL) {
      strncpy(message_list[current_article].ref_id, ptr1,
	      MAX_FIELD_LEN - 1);
      message_list[current_article].ref_id[MAX_FIELD_LEN - 1] = '\0';
      ptr1 = strchr(message_list[current_article].ref_id, '>');
      if (ptr1 != (char*)NULL) {
	*(ptr1 + 1) = '\0';
      }
    } else {
      message_list[current_article].ref_id[0] = '\0';
    }
  }
#endif	/* REF_SORT */
  ptr1 = ptr2;
  while (isdigit(*ptr1)) {
    ptr1++;
  }
  if (*ptr1 == '\t') {
    ptr1++;
  } else {
    return(1);
  }
  article_list[current_article].lines = atoi(ptr1);
  article_list[current_article].mark &= ~UNFETCH_MARK;
  return(0);
}
#endif	/* XOVER */

/*
 * 記事ソート
 */

int	sort_articles(current_group, article_ptr, thread_mode, article_mask,
		      retrieve, mark)
     int	current_group;
     int	*article_ptr;
     short	thread_mode;
     short	article_mask;
     int	(*retrieve)();
     void	(*mark)();
{
  char		buff[BUFF_SIZE];
  int		article_number;
  int		min_number;
  int		status;
  register int	i, j;

  article_number = *article_ptr;
  if ((!thread_mode) || (!article_number)) {
    status = 1;
    goto sort_end;
  }

  min_number = article_number - 1;
  for (i = 0; i < article_number; i++) {
    if (!(article_list[i].mark & READ_MARK)) {
      min_number = i;
      break;
    }
  }
  if (min_number > (article_number - term_lines)) {
    if ((min_number = article_number - term_lines) < 0) {
      min_number = 0;
    }
  }
  if ((article_number - min_number) > max_thread_number) {
#ifdef	notdef
    sprintf(buff, "%d", article_number - min_number);
#else	/* !notdef */
    sprintf(buff, "%d", max_thread_number);
#endif	/* !notdef */
    print_title();
    input_line(INPUT_SPCCUT_MASK, "ソートする記事数を入力して下さい:",
	       "Input how many articles sort:", buff);
    if (buff[0]) {
      i = atoi(buff);
      if (i <= 0) {
	status = 1;
	goto sort_end;
      } else if (i >= article_number) {
	min_number = 0;
      } else {
	min_number = article_number - i;
      }
    } else {
      status = -1;
      goto sort_end;
    }
  }

  /*
   * ソートする記事を準備する
   */
  
  print_mode_line(japanese ? "ソート中です。" : "Sorting.");
  sort_list = (ARTICLE_LIST*)malloc(sizeof(ARTICLE_LIST) * article_number);
  if (sort_list != (ARTICLE_LIST*)NULL) {
    if (min_number > 0) {
      memcpy(sort_list, article_list, sizeof(ARTICLE_LIST) * min_number);
    }
#ifdef	XOVER
    if (xover_mode && (mark == news_add_mark)) {
      if (nntp_xover(current_group, article_list[min_number].real_number,
		     article_list[article_number - 1].real_number)
	  == NNTP_OK) {
	for (i = min_number; i < article_number; i++) {
	  if (article_list[i].mark & UNFETCH_MARK) {
	    article_list[i].mark &= ~UNFETCH_MARK;
	    article_list[i].mark |= CANCEL_MARK;
	    if (mark) {
	      mark(current_group, article_list[i].real_number,
		   article_list[i].real_number);
	    }
	  }
	}
      }
    } else {
#else	/* !XOVER */
    {
#endif	/* !XOVER */
      if (article_mask) {
	for (i = min_number; i < article_number; i++) {
	  if (!(article_list[i].mark & READ_MARK)) {
	    if (retrieve != (int (*)())NULL) {
	      if (retrieve(current_group, i) < 0) {
		if (mark) {
		  mark(current_group, article_list[i].real_number,
		       article_list[i].real_number);
		}
	      }
	    }
	  }
	}
      } else {
	for (i = min_number; i < article_number; i++) {
	  if (retrieve != (int (*)())NULL) {
	    if (retrieve(current_group, i) < 0) {
	      if (mark) {
		mark(current_group, article_list[i].real_number,
		     article_list[i].real_number);
	      }
	    }
	  }
	}
      }
    }

    /*
     * ソート処理メイン
     */

    sort_article_func[sort_rule](&article_number, min_number);
  
    /*
     * ソート後処理
     */

    if (article_number - min_number) {
      memcpy(&article_list[min_number], &sort_list[min_number],
	     (article_number - min_number) * sizeof(ARTICLE_LIST));
    }
    free(sort_list);
    for (j = min_number; j < article_number; j++) {
      article_list[j].mark &=~ SORT_MARK;
    }
  }
  *article_ptr = article_number;
  status = 0;
 sort_end:
#ifdef	REF_SORT
  if (message_list != (MESSAGE_LIST*)NULL) {
    free(message_list);
    message_list = (MESSAGE_LIST*)NULL;
  }
#endif	/* REF_SORT */
  return(status);
}

/*
 * 記事パック
 */

int	pack_articles(current_group, article_ptr, retrieve, mark)
     int	current_group;
     int	*article_ptr;
     int	(*retrieve)();
     void	(*mark)();
{
  int		article_number;
  register int	i, j, k;

  article_number = *article_ptr;
  for (i = 0; i < article_number; i++) {
    if ((article_list[i].mark & UNFETCH_MARK) &&
	(!(article_list[i].mark & (READ_MARK | CANCEL_MARK)))) {
      if (retrieve != (int (*)())NULL) {
	if (retrieve(current_group, i) < 0) {
	  if (mark) {
	    mark(current_group, article_list[i].real_number,
		 article_list[i].real_number);
	  }
	}
      } else {
	print_fatal("Unexpected unfetched article-%d.", i);
      }
    }
  }
  i = j = 0;
  while (j < article_number) {
    k = j;
    if (!(article_list[j].mark & (READ_MARK | CANCEL_MARK))) {
      while (!(article_list[j].mark & (READ_MARK | CANCEL_MARK))) {
	if (++j >= article_number) {
	  break;
	}
      }
#ifdef	DEBUG
      print_fatal("ARTICLE=%d(%d)->%d(%d)", j, article_list[j].real_number,
		  i, article_list[i].real_number);
#endif	/* DEBUG */
      if (i != k) {
	memmove(&article_list[i], &article_list[k],
		(j - k) * sizeof(struct article_struct));
      }
      i += (j - k);
    } else {
      j++;
    }
  }
  *article_ptr = i;
  return(0);
}

/*
 * Subject ソート
 */

static int	sort_subject(article_ptr, min_number)
     int	*article_ptr;
     int	min_number;
{
  int		article_number;
  char		*ptr1, *ptr2;
  register int	i, j, k, l;

  /*
   * 新たな Subject の記事をまとめる
   */
  
  article_number = *article_ptr;
  i = min_number;
  
  for (j = min_number; j < article_number; j++) {
    if ((!(article_list[j].mark & CANCEL_MARK)) &&
	(strncasecmp(article_list[j].subject, REPLY_SUBJECT,
		     sizeof(REPLY_SUBJECT) - 1))) {
      article_list[j].mark |= SORT_MARK;
      memcpy(&sort_list[i++], &article_list[j], sizeof(ARTICLE_LIST));
      article_list[j].mark |= CANCEL_MARK;
      ptr1 = article_list[j].subject;
      for (k = min_number; k < article_number; k++) {
	if ((!(article_list[k].mark & CANCEL_MARK)) &&
	    (!strncasecmp(article_list[k].subject, REPLY_SUBJECT,
			  sizeof(REPLY_SUBJECT) - 1))) {
	  ptr2 = &article_list[k].subject[sizeof(REPLY_SUBJECT) - 1];
	  if (*ptr2 == ' ') {
	    ptr2++;
	  }
	  if (!strcmp(ptr1, ptr2)) {
	    strncpy(article_list[k].subject, THREAD_SUBJECT1, 3);
	    article_list[k].mark |= THREAD_MARK;
	    memcpy(&sort_list[i++], &article_list[k], sizeof(ARTICLE_LIST));
	    article_list[k].mark |= CANCEL_MARK;
	  }
	}
      }
    }
  }
  
  /*
   * 新たな Subject ではないフォローの記事をまとめる
   */
  
  for (j = min_number; j < article_number; j++) {
    ptr1 = article_list[j].subject;
    if ((!(article_list[j].mark & CANCEL_MARK)) &&
	(!strncasecmp(ptr1, REPLY_SUBJECT,
		      sizeof(REPLY_SUBJECT) - 1))) {
      ptr1 += (sizeof(REPLY_SUBJECT) - 1);
      if (*ptr1 == ' ') {
	ptr1++;
      }
      k = article_list[j].real_number;
      for (l = min_number; l < i; l++) {
	if ((sort_list[l].mark & SORT_MARK) &&
	    (sort_list[l].real_number > k)) {
	  memmove(&sort_list[l + 1], &sort_list[l],
		  sizeof(ARTICLE_LIST) * (i - l));
	  break;
	}
      }
      article_list[j].mark |= SORT_MARK;
      memcpy(&sort_list[l++], &article_list[j], sizeof(ARTICLE_LIST));
      i++;
      article_list[j].mark |= CANCEL_MARK;
      for (k = min_number; k < article_number; k++) {
	ptr2 = article_list[k].subject;
	if ((!(article_list[k].mark & CANCEL_MARK)) &&
	    (!strncasecmp(ptr2, REPLY_SUBJECT, sizeof(REPLY_SUBJECT) - 1))) {
	  ptr2 += (sizeof(REPLY_SUBJECT) - 1);
	  if (*ptr2 == ' ') {
	    ptr2++;
	  }
	  if (!strcmp(ptr1, ptr2)) {
	    strncpy(article_list[k].subject, THREAD_SUBJECT2, 3);
	    if (l < i) {
	      memmove(&sort_list[l + 1], &sort_list[l],
		      sizeof(ARTICLE_LIST) * (i - l));
	    }
	    article_list[k].mark |= THREAD_MARK;
	    memcpy(&sort_list[l++], &article_list[k], sizeof(ARTICLE_LIST));
	    i++;
	    article_list[k].mark |= CANCEL_MARK;
	  }
	}
      }
    }
  }
  
  /*
   * 上記以外の記事を並べる
   */
  
  for (j = min_number; j < article_number; j++) {
    if (!(article_list[j].mark & CANCEL_MARK)) {
      k = article_list[j].real_number;
      for (l = min_number; l < i; l++) {
	if ((sort_list[l].mark & SORT_MARK) &&
	    (sort_list[l].real_number > k)) {
	  memmove(&sort_list[l + 1], &sort_list[l],
		  sizeof(ARTICLE_LIST) * (i - l));
	  break;
	}
      }
      memcpy(&sort_list[l++], &article_list[j], sizeof(ARTICLE_LIST));
      i++;
    }
  }
  *article_ptr = i;
  return(0);
}

#ifdef	REF_SORT
/*
 * Reference ソート
 */

static int	sort_reference(article_ptr, min_number)
     int	*article_ptr;
     int	min_number;
{
  int		article_number;
  int		index;
  register int	i;

  article_number = *article_ptr;
  if (message_list == (MESSAGE_LIST*)NULL) {
    memcpy(sort_list, article_list, sizeof(ARTICLE_LIST) * article_number);
    return(1);
  }
  index = min_number;
  for (i = min_number; i < article_number; i++) {
    make_thread(0, &index, i, (char*)NULL, min_number, article_number);
  }
  *article_ptr = index;
  return(0);
}

/*
 * Date ソート
 */

static int	sort_date(article_ptr, min_number)
     int	*article_ptr;
     int	min_number;
{
  int		article_number;
  register int	i, j;

  article_number = *article_ptr;
  memcpy(sort_list, article_list, sizeof(ARTICLE_LIST) * article_number);
  if ((article_number - min_number) > 0) {
    qsort(&sort_list[min_number], article_number - min_number,
	  sizeof(ARTICLE_LIST), compare_date);
  }
  i = min_number;
  for (j = min_number; j < article_number; j++) {
    if (!(article_list[j].mark & CANCEL_MARK)) {
      i++;
    }
  }
  *article_ptr = i;
  return(0);
}

/*
 * スレッド作成
 */

static int	make_thread(level, index_ptr, current_article, message_id,
			    min_number, max_article)
     int	level;
     int	*index_ptr;
     int	current_article;
     char	*message_id;
     int	min_number;
     int	max_article;
{
  int		index;
  register int	i;

  if (article_list[current_article].mark & CANCEL_MARK) {
    return(1);
  }
  index = *index_ptr;
  if (message_id) {
    if (!strcmp(message_id, message_list[current_article].ref_id)) {
      memcpy(&sort_list[index], &article_list[current_article],
	     sizeof(ARTICLE_LIST));
      article_list[current_article].mark |= CANCEL_MARK;
      for (i = 0; i <= level; i++) {
	sort_list[index].subject[i] = (i < 2 ? ' ' : THREAD_CHAR);
      }
      strcpy(&sort_list[index].subject[i], THREAD_SUBJECT3);
      if (strncasecmp(article_list[current_article].subject, REPLY_SUBJECT,
		      sizeof(REPLY_SUBJECT) - 1)) {
	euc_strncpy(&sort_list[index].subject[i + sizeof(THREAD_SUBJECT3) - 1],
		    article_list[current_article].subject,
		    MAX_SUBJECT_LEN - (i + sizeof(THREAD_SUBJECT3) - 1) - 1);
      } else {
	sort_list[index].mark |= THREAD_MARK;
	euc_strncpy(&sort_list[index].subject[i + sizeof(THREAD_SUBJECT3) - 1],
		    &article_list[current_article].subject[sizeof(REPLY_SUBJECT) - 1],
		    MAX_SUBJECT_LEN - (i + sizeof(THREAD_SUBJECT3) - 1) - 1);
      }
      index++;
      message_id = message_list[current_article].msg_id;
      if (message_id[0]) {
	for (i = min_number; i < max_article; i++) {
	  if (!(article_list[i].mark & CANCEL_MARK)) {
	    if ((i != current_article) &&
		(!strcmp(message_id, message_list[i].ref_id))) {
	      i = -1;
	      break;
	    }
	  }
	}
      } else {
	i = 0;
      }
      if (i < 0) {
	for (i = min_number; i < max_article; i++) {
	  make_thread(level + 1, &index, i, message_id,
		      min_number, max_article);
	}
      }
    }
  } else if (message_list[current_article].msg_id[0]) {
    if (message_list[current_article].ref_id[0]) {
      for (i = min_number; i < max_article; i++) {
	if (!(article_list[i].mark & CANCEL_MARK)) {
	  if ((i != current_article) &&
	      (!strcmp(message_list[current_article].ref_id,
		       message_list[i].msg_id))) {
	    i = -1;
	    break;
	  }
	}
      }
    } else {
      i = 0;
    }
    if (i >= 0) {
      memcpy(&sort_list[index++], &article_list[current_article],
	     sizeof(ARTICLE_LIST));
      article_list[current_article].mark |= CANCEL_MARK;
      for (i = min_number; i < max_article; i++) {
	make_thread(level + 1, &index, i,
		    message_list[current_article].msg_id,
		    min_number, max_article);
      }
    }
  } else {
    memcpy(&sort_list[index++], &article_list[current_article],
	   sizeof(ARTICLE_LIST));
    article_list[current_article].mark |= CANCEL_MARK;
  }
  *index_ptr = index;
  return(0);
}

/*
 * 日付比較
 */

static int	compare_date(article_ptr1, article_ptr2)
     ARTICLE_LIST	*article_ptr1;
     ARTICLE_LIST	*article_ptr2;
{
  if (article_ptr1 == article_ptr2) {
    return(0);
  }
  if (article_ptr1->mark & CANCEL_MARK) {
    if (article_ptr2->mark & CANCEL_MARK) {
      return(0);
    } else {
      return(1);
    }
  } else if (article_ptr2->mark & CANCEL_MARK) {
    return(-1);
  }
#ifdef	REF_SORT
  if (article_ptr1->year != article_ptr2->year) {
    return(article_ptr1->year < article_ptr2->year ? -1 : 1);
  }
#endif	/* REF_SORT */
  if (article_ptr1->month != article_ptr2->month) {
    return(article_ptr1->month < article_ptr2->month ? -1 : 1);
  }
  if (article_ptr1->date != article_ptr2->date) {
    return(article_ptr1->date < article_ptr2->date ? -1 : 1);
  }
  return(article_ptr1->real_number < article_ptr2->real_number ? -1 : 1);
}
#endif	/* REF_SORT */

/*
 * パック/ソート方法切替
 */

int	change_sort_rule(current_group, article_ptr, current_ptr,
			 mask_ptr, thread_ptr, get_list, key)
     int	current_group;
     int	*article_ptr;
     int	*current_ptr;
     short	*mask_ptr;
     short	*thread_ptr;
     int	(*get_list)();
     int	key;
{
  int		current_article;
  register int	i, j;

  current_article = *current_ptr;
  if (key == 'l') {
    *mask_ptr = !*mask_ptr;
  } else {
    if (key == 't') {
      *thread_ptr = !*thread_ptr;
    } else {
#ifdef	REF_SORT
      if (++sort_rule > 2) {
	sort_rule = 0;
      }
      *thread_ptr = 1;
#else	/* !REF_SORT */
      return(1);
#endif	/* !REF_SORT */
    }
  }
  if (*article_ptr > 0) {
    j = article_list[current_article].real_number;
  } else {
    j = 0;
  }

  /*
   * get_list すると *article_ptr が変化するので注意
   */

  if (current_group >= 0) {
    get_list(current_group);
  } else {
    get_list();
  }
  current_article = 0;
  for (i = 0; i < *article_ptr; i++) {
    if (!(article_list[i].mark & CANCEL_MARK)) {
      current_article = i;
      if (article_list[i].real_number >= j) {
	break;
      }
    }
  }
  *current_ptr = current_article;
  return(0);
}

/*
 * 作業ファイル名作成
 */

void	create_temp_name(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
{
  sprintf(ptr1, "%s%cmnews_%s.%d", tmp_dir, SLASH_CHAR, ptr2, (int)getpid());
}
