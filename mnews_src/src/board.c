/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : BOARD support routine
 *  File        : board.c
 *  Version     : 1.21
 *  First Edit  : 1992-10/05
 *  Last  Edit  : 1997-11/11
 *  Author      : MSR24 ��� ��
 *
 */

#ifdef	BOARD
#include	"compat.h"
#include	"nntplib.h"
#include	"smtplib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"termlib.h"
#include	"mnews.h"
#include	"group.h"
#include	"article.h"
#include	"mark.h"
#include	"board.h"
#include	"pager.h"
#include	"mh.h"
#include	"mailsend.h"

static int	board_get_level();	/* ���롼�ץ�٥����		*/
static char	*board_get_name();	/* ���롼�ץ�٥�̾����		*/
static int	board_select_group();	/* ���롼������			*/
static void	board_redraw_group();	/* ���롼��������̺�����	*/
static int	board_select_article();	/* ��������			*/
static void	board_redraw_article();	/* ����������̺�����		*/
static int	board_prev_unread_sgroup();
					/* �����򥰥롼���ֹ����	*/
static int	board_next_unread_sgroup();
					/* �����򥰥롼���ֹ����	*/
static int	board_prev_sgroup();	/* �����򥰥롼���ֹ����	*/
static int	board_next_sgroup();	/* �����򥰥롼���ֹ����	*/
static int	board_prev_article();	/* �������ֹ����		*/
static int	board_next_article();	/* �������ֹ����		*/
static int	board_prev_unread_article();
					/* �������ֹ����		*/
static int	board_next_unread_article();
					/* �������ֹ����		*/
static int	board_prev_unread_article2();
					/* �������ֹ����(�ޡ���)	*/
static int	board_next_unread_article2();
					/* �������ֹ����(�ޡ���)	*/
static int	board_read();		/* ��������			*/
static int	board_extract();	/* �������			*/
static void	board_count();		/* �������������		*/
static void	board_search_all_group();
					/* �����롼�׸���		*/
static void	board_search_group();	/* ���롼�׸���			*/
static void	board_match_group();	/* ���롼�׾ȹ�			*/
static int	board_get_list();	/* �����ꥹ�ȼ���		*/
static int	board_get_field();	/* �����إå�����		*/
static void	board_read_initfile();	/* ������ե�����꡼��		*/
static void	board_save_initfile();	/* ������ե����륻����		*/
static void	board_add_mark();	/* �ޡ����ꥹ�������ɲ�		*/
static void	board_delete_mark();	/* �ޡ����ꥹ�����Ǻ��		*/
static void	board_fill_mark();	/* �ޡ����ꥹ���������ɲ�	*/
static void	board_clean_mark();	/* �ޡ����ꥹ�������Ǻ��	*/
static void	board_mark();		/* �����ޡ����ɲ�		*/
static void	board_unmark();		/* �����ޡ������		*/
static void	board_count_unread();	/* ̤�ɵ���������		*/
static void	board_get_adrs();	/* �ܡ��ɥ��ɥ쥹����		*/
static int	board_search_name();	/* ���롼��̾����(ʣ��)		*/
static int	board_cancel();		/* �ܡ��ɥ���󥻥�		*/

/*
 * BOARD ���롼�ץꥹ��
 */

static int	board_group_number;	/* BOARD ���롼�׿�		*/
static int	board_select_number;	/* ���򤵤�Ƥ��륰�롼�׿�	*/
static int	board_alloc_number;	/* BOARD ���롼�׳�����		*/
static struct board_group	*board_group = (struct board_group*)NULL;
static struct board_group	*board_selected_group =
  (struct board_group*)NULL;
static int	top_position;		/* ɽ�����ϰ���			*/
static int	board_article_number;	/* BOARD ������			*/
static short	board_count_flag = 1;	/* BOARD ����������ȥե饰	*/

static char	board_subscribe_char[] = {	/* ���ɥ⡼��		*/
  ' ', 'U'
  };

/*
 * BOARD �إ�ץ�å�����
 */

static char	*board_group_jmessage[] = {
  "BOARD ���롼������⡼��",
  "\n  [����]",
  "\ti �ޤ��� SPACE  ���롼�פ����򤷤ޤ���",
  "\n  [��ư]",
  "\tk �ޤ��� ^P     ���Υ��롼�פ˰�ư���ޤ���",
  "\tj �ޤ��� ^N     ���Υ��롼�פ˰�ư���ޤ���",
  "\tp               ����̤�ɥ��롼�פ˰�ư���ޤ���",
  "\tn               ����̤�ɥ��롼�פ˰�ư���ޤ���",
  "\t^U �ޤ��� ^B    �����̤Υ��롼�פ˰�ư���ޤ���",
  "\t^D, ^F �ޤ��� ^V",
  "\t                �����̤Υ��롼�פ˰�ư���ޤ���",
  "\t<               ��Ƭ�Υ��롼�פ˰�ư���ޤ���",
  "\t>               �Ǹ�Υ��롼�פ˰�ư���ޤ���",
  "\tTAB             ����Υܡ��ɥ��ƥ���ޤ��ϥ��롼�פ˥����פ��ޤ���",
  "\t                (�ܡ��ɥ��ƥ���ޤ��ϥ��롼��̾�����Ϥ��Ʋ�����)",
  "\to, q �ޤ��� RETURN",
  "\t                BOARD �⡼�ɤ���ȴ���ޤ���",
  "\n  [�ޡ���]",
  "\tU               ���ƥ���/���롼�פι��ɥޡ��������ؤ�������ư���ޤ���",
  "\tu               ���ƥ���/���롼�פι��ɥޡ��������ؤ�������ư���ޤ���",
  "\tc               ���ƥ���/���롼�פε��������ƴ��ɤˤ��ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
#ifdef	MAILSEND
  "\n  [���]",
  "\ta               �����˵�����Ф��ޤ���",
  "\n  [����]",
  "\tm               �����˥᡼���Ф��ޤ���",
#endif	/* MAILSEND */
#ifndef	SMALL
  "\n  [����]",
  "\t/               ���롼��̾�������������ޤ���",
  "\t                (����������ʸ��������Ϥ��Ʋ�����)",
  "\t\\               ���롼��̾������������ޤ���",
  "\t                (����������ʸ��������Ϥ��Ʋ�����)",
#endif	/* !SMALL */
  "\n  [ɽ��]",
  "\tL �ޤ��� l      ���ɤΥ��롼�פ�ɽ���⡼�ɤ����ؤ��ޤ���",
  "\tG               �˥塼��/�ܡ��ɥ��롼�פ�ɽ�����������ؤ��ޤ���",
  "\t                (�ȥåץ��롼�ץ��ƥ�����Τ�ͭ��)",
  (char*)NULL,
};

static char	*board_article_jmessage[] = {
  "BOARD ��������⡼��",
  "\n  [����]",
  "\tSPACE, i �ޤ��� .",
  "\t                �����򻲾Ȥ��ޤ���",
  "\tv �ޤ��� V      ���ƤΥإå��ȤȤ�˵����򻲾Ȥ��ޤ���",
#ifdef	MIME
  "\t                ('V' �� MIME �ǥ������б�)",
#endif	/* MIME */
  "\tp               ����̤�ɵ����򻲾Ȥ��ޤ���",
  "\tn               ����̤�ɵ����򻲾Ȥ��ޤ���",
  "\tP               ���ε����򻲾Ȥ��ޤ���",
  "\tN               ���ε����򻲾Ȥ��ޤ���",
  "\n  [��ư]",
  "\tk �ޤ��� ^P     ���ε����˰�ư���ޤ���",
  "\tj �ޤ��� ^N     ���ε����˰�ư���ޤ���",
  "\t^U �ޤ��� ^B    �����̤ε����˰�ư���ޤ���",
  "\t^D, ^F �ޤ��� ^V",
  "\t                �����̤ε����˰�ư���ޤ���",
  "\t<               ��Ƭ�ε����˰�ư���ޤ���",
  "\t>               �Ǹ�ε����˰�ư���ޤ���",
  "\tTAB             ����ε����˥����פ��ޤ���",
  "\t                (�����ֹ�����Ϥ��Ʋ�����)",
  "\to, q �ޤ��� RETURN",
  "\t                ���롼�פ���ȴ���ޤ���",
  "\n  [�ޡ���]",
  "\tD �ޤ��� Y      ���ɥޡ��������ε����ذ�ư���ޤ���",
  "\td �ޤ��� y      ���ɥޡ��������ε����ذ�ư���ޤ���",
  "\tU �ޤ��� Z      ���ɥޡ�����������ε����ذ�ư���ޤ���",
  "\tu �ޤ��� z      ���ɥޡ�����������ε����ذ�ư���ޤ���",
  "\tM               �ޥ���ޡ��������/��������ε����ذ�ư���ޤ���",
#ifndef	SMALL
  "\tK               Ʊ�쥵�֥������Ȥε�������ɥޡ������ޤ���",
#endif	/* !SMALL */
  "\tc               ����������ɥޡ������ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
#ifdef	LARGE
  "\t+               ����������������ɥޡ������ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
  "\t-               ����������������ɥޡ������ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
#endif	/* LARGE */
#ifdef	MAILSEND
  "\n  [���]",
  "\ta               �����˵�����Ф��ޤ���",
  "\tf               �����˥ե������ޤ���",
  "\tF               ��������Ѥ��ƥե������ޤ���",
  "\tC               �����򥭥�󥻥뤷�ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
  "\n  [����]",
  "\tm               �����˥᡼���Ф��ޤ���",
  "\tr               �������ֿ����ޤ���",
  "\tR               ��������Ѥ����ֿ����ޤ���",
  "\t=               ������ž�����ޤ���",
#ifdef	LARGE
  "\t~               ������������ޤ���",
#endif	/* LARGE */
#endif	/* MAILSEND */
  "\n  [������]",
  "\ts               �����򥻡��֤��ޤ���",
  "\t                (�ե�����̾�����Ϥ��Ʋ��������ե����뤬¸�ߤ������",
  "\t                 y:���ڥ�� n:��� o:��񤭤�����ǲ�����)",
#ifdef	MH
  "\tO               ������ MH �ե�����˥����֤��ޤ���",
  "\t                (MH �ե����̾�����Ϥ��Ʋ�����)",
#endif	/* MH */
  "\n  [ɽ��]",
  "\tl               ����/����󥻥뤵�줿������ɽ���⡼�ɤ����ؤ��ޤ���",
  "\tt               ����åɥ⡼�ɤ����ؤ��ޤ���",
#ifdef	REF_SORT
  "\tT               ��������ˡ�����ؤ��ޤ���",
#endif	/* REF_SORT */
  (char*)NULL,
};

static char	*board_group_message[] = {
#ifndef	SMALL
  "BOARD GROUP SELECT MODE",
  "\n  [SELECT]",
  "\ti or SPACE   select group.",
  "\n  [MOVE]",
  "\tk or ^P      previous group.",
  "\tj or ^N      next group.",
  "\tp            previous unread group.",
  "\tn            next unread group.",
  "\t^U or ^B     previous page group.",
  "\t^D, ^F or ^V next page group.",
  "\t<            top group.",
  "\t>            last group.",
  "\tTAB          jump to specified board category or group.",
  "\t             (Please input board category or group name.)",
  "\to, q or RETURN",
  "\t             exit group.",
  "\n  [MARK]",
  "\tU            toggle subscribe mark and move to previous.",
  "\tu            toggle subscribe mark and move to next.",
  "\tc            mark as read all articles in category/group.",
  "\t             (Please make sure y/n.)",
#ifdef	MAILSEND
  "\n  [POST]",
  "\ta            send new article.",
  "\n  [SEND]",
  "\tm            mail.",
#endif	/* MAILSEND */
  "\n  [SEARCH]",
  "\t/            forward search pattern.",
  "\t             (Please input search pattern.)",
  "\t\\            backward search pattern.",
  "\t             (Please input search pattern.)",
  "\n  [DISPLAY]",
  "\tL or l       toggle print read group.",
  "\tG            toggle news/board group print mode.",
  "\t             (When top category only)",
#endif	/* !SMALL */
  (char*)NULL,
};

static char	*board_article_message[] = {
#ifndef	SMALL
  "BOARD ARTICLE SELECT MODE",
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
  "\t^U or ^B     previous page article.",
  "\t^D, ^F or ^V next page article.",
  "\t<            top article.",
  "\t>            last article.",
  "\tTAB          jump to specified article.",
  "\t             (Please input article number.)",
  "\to, q or return",
  "\t             exit group.",
  "\n  [MARK]",
  "\tD or Y       mark as read and move to previous.",
  "\td or y       mark as read and move to next.",
  "\tU or Z       clear read mark and move to previous.",
  "\tu or z       clear read mark and move to next.",
  "\tM            toggle multi mark and move to next.",
  "\tK            mark as read same subject article.",
  "\tc            mark as read all articles.",
  "\t             (Please make sure y/n.)",
#ifdef	LARGE
  "\t+            mark as read all forward articles.",
  "\t             (Please make sure y/n.)",
  "\t-            mark as read all backward articles.",
  "\t             (Please make sure y/n.)",
#endif	/* LARGE */
#ifdef	MAILSEND
  "\n  [POST]",
  "\ta            send new article.",
  "\tf            follow.",
  "\tF            follow with original article.",
  "\tC            cancel article.",
  "\t             (Please make sure y/n.)",
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
#endif	/* !SMALL */
  (char*)NULL,
};

/*
 * BOARD �⡼�ɽ����
 */

int	board_init(unread_ptr)
     long	*unread_ptr;
{
  struct stat	stat_buff;
  static long	unread = 0;		/* static �ˤ��뤳��	*/
  register int	i;

  *unread_ptr = -1;
  if (!board_mode) {
    return(0);
  }
  if ((!stat(board_spool, &stat_buff)) && (stat_buff.st_mode & S_IFDIR)) {
    if (board_count_mode) {
      if (board_count_flag) {
	print_mode_line(japanese ? "������Ǥ���" : "Searching.");
	board_count_flag = 0;
	board_search_all_group();
	board_read_initfile(board_mark_file);
	unread = 0;
	for (i = 0; i < board_group_number; i++) {
	  unread += board_group[i].unread_article;
	}
      }
      *unread_ptr = unread;
    }
    return(1);
  }
  return(0);
}

/*
 * BOARD ��˥塼
 */

int	board_menu()
{
  int	status;

  while (1) {
    print_mode_line(japanese ? "������Ǥ���" : "Searching.");
    if ((!select_name[0]) && board_count_flag) {
      board_count_flag = 0;
      board_search_all_group();
      board_read_initfile(board_mark_file);
    }
    board_match_group(select_name);
    status = board_select_group();
    if (select_name[0] || !jump_name[0]) {
      break;
    }
  }
  if (!select_name[0]) {
    board_save_initfile(board_mark_file);
    board_count_flag = 1;
  }
  return(status);
}

/*
 * ���롼��������̺�����
 */

static void	board_redraw_group(current_group)
     int	current_group;
{
  register int	i, j;
  char		buff[BOARD_MAX_GROUP_NAME];
  char		jname[BOARD_MAX_GROUP_NAME];

  i = top_position;
  top_position = get_top_position(top_position, current_group,
				  board_select_number, news_thread_mode,
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
  get_jname(BOARD_JN_DOMAIN, jname, 40);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "���롼�ץ��ƥ���:%-40.40s       ����:%s" :
		  "Group category:%-40.40s     Position:%s",
		  jname, buff);
  if (!wide_mode) {
    term_locate(0, 2);
    term_attrib(RESET_ATTRIB);
    term_attrib(color_code[HEADER_COLOR]);
    print_full_line(japanese ?
		    "  ����    ̤��  �⡼��  �ܡ��ɥ��ƥ���/�ܡ��ɥ��롼��̾" :
		    "  Max    Unread Mode    Board category/Board group name");
  }
  term_attrib(RESET_ATTRIB);
  for (i = 0, j = top_position; i < term_lines - mode_lines; i++, j++) {
    if (j >= board_select_number) {
      break;
    }
    term_locate(0, head_lines + i);
#ifdef	COLOR
    term_attrib(color_code[NUMBER_COLOR]);
#endif	/* COLOR */
    if (board_selected_group[j].max_article >= 0) {
      cprintf("%7d", board_selected_group[j].max_article);
    } else {
      cprintf("       ");
    }
    cprintf(" %7d", board_selected_group[j].unread_article);
    strcpy(buff, board_selected_group[j].group_name);
#ifdef	JNAMES
    get_jname(BOARD_JN_DOMAIN, buff, term_columns - 24);
#else	/* !JNAMES */
    buff[term_columns - 24] = '\0';
#endif	/* !JNAMES */
#ifdef	COLOR
    term_attrib(color_code[CATEGORY_COLOR]);
#endif	/* COLOR */
#ifdef	SJIS_SRC
    kanji_printf(SJIS_CODE, "  [B] %c  %s",
		 board_subscribe_char[board_selected_group[j].unsubscribe],
		 buff);
#else	/* !SJIS_SRC */
    kanji_printf(EUC_CODE, "  [B] %c  %s",
		 board_subscribe_char[board_selected_group[j].unsubscribe],
		 buff);
#endif	/* !SJIS_SRC */
  }
  term_attrib(RESET_ATTRIB);
  print_mode_line(japanese ?
		  "?:�إ�� j,n,^N:���� k,p,^P:���� SPACE,i:���� o,q,RETURN:���� Q:��λ" :
		  "?:help j,n,^N:next k,p,^P:previous SPACE,i:select o,q,RETURN:return Q:exit");
}

/*
 * ���롼������
 */

static int	board_select_group()
{
  /* �ѿ��ϺƵ�����Τ� static �ˤ��ʤ����� */

  int		current_group;		/* ������Υ��롼���ֹ�		*/
  char		buff1[BOARD_MAX_GROUP_NAME];
  char		buff2[BOARD_MAX_GROUP_NAME];
  int		level;
  int		loop;
  int		status;
  int		key;
  register int	i, j;

  level = board_get_level(select_name);
  top_position = -1;
  current_group = 0;

  /*
   * �ǽ��̤�ɥ��롼�פޤǥ����Ȥ�ʤ��
   */

  current_group = 0;
  if (board_select_number > 0) {
    if (!board_selected_group[current_group].unread_article) {
      current_group = board_next_unread_sgroup(current_group);
    }
  }

  loop = 1;
  while (loop) {
    if (jump_name[0]) {
      key = 0;
      current_group = -1;
      for (i = 0; i < board_select_number; i++) {
	strcpy(buff1, board_get_name(i, level));
	if (gnus_mode) {
	  if (!strcmp(buff1, jump_name)) {
	    current_group = i;
	    key = ' ';
	    break;
	  }
	} else {
	  if (!strncmp(buff1, jump_name, strlen(buff1))) {
	    current_group = i;
	    key = ' ';
	  }
	}
      }
      if (current_group < 0) {
	for (i = 0; i < board_select_number; i++) {
	  strcpy(buff1, board_get_name(i, level));
	  if (!strncmp(buff1, jump_name, strlen(jump_name))) {
	    if (current_group < 0) {
	      current_group = i;
	      key = ' ';
	    } else {
	      key = 0;
	    }
	  }
	}
      }
      if (current_group < 0) {
	current_group = 0;
      }
      if (!key) {
	jump_name[0] = '\0';
	continue;
      }
    } else {
      board_redraw_group(current_group);
      term_locate(21, head_lines + current_group - top_position);
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
	  if (line < board_select_number) {
	    current_group = line;
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
    case 0x0c:		/* ^L ������		*/
      term_init(2);
      top_position = -1;
      break;
    case 0x10:		/* ^P �����롼�װ�ư	*/
    case 'k':
      current_group = board_prev_sgroup(current_group);
      break;
    case 0x0e:		/* ^N �����롼�װ�ư	*/
    case 'j':
      current_group = board_next_sgroup(current_group);
      break;
    case 'o':
    case 'q':
    case '\015':	/* RETURN		*/
    case '\n':
      loop = 0;
      break;
    case 'p':		/* ��̤�ɥ��롼��	*/
      i = current_group;
      current_group = board_prev_unread_sgroup(current_group);
      if ((current_group == i) && board_select_number) {
	for (i = 0; i < board_group_number; i++) {
	  if (!strcmp(board_group[i].group_name,
		      board_selected_group[0].group_name)) {
	    i = board_prev_unread_group(i);
	    if (i >= 0) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"����̤��(%d)���롼��(%s)�򻲾Ȥ��ޤ���?",
				"Read previous unread(%d) group(%s)?",
				board_group[i].unread_article,
				board_group[i].group_name)) {
	      case 1:
		strcpy(jump_name, board_group[i].group_name);
		return(1);
		break;
	      case 2:
		loop = 0;
		break;
	      default:
		term_init(2);
		top_position = -1;
		break;
	      }
	    }
	    break;
	  }
	}
      }
      break;
    case 'n':		/* ��̤�ɥ��롼��	*/
      i = current_group;
      current_group = board_next_unread_sgroup(current_group);
      if ((current_group == i) && board_select_number) {
	for (i = 0; i < board_group_number; i++) {
	  if (!strcmp(board_group[i].group_name,
		      board_selected_group[board_select_number - 1].group_name)) {
	    i = board_next_unread_group(i);
	    if (i >= 0) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"����̤��(%d)���롼��(%s)�򻲾Ȥ��ޤ���?",
				"Read next unread(%d) group(%s)?",
				board_group[i].unread_article,
				board_group[i].group_name)) {
	      case 1:
		strcpy(jump_name, board_group[i].group_name);
		return(1);
		break;
	      case 2:
		loop = 0;
		break;
	      default:
		term_init(2);
		top_position = -1;
		break;
	      }
	    }
	    break;
	  }
	}
      }
      break;
    case 0x02:		/* ^B			*/
    case 0x15:		/* ^U ���ڡ���		*/
      if ((current_group -= (term_lines - mode_lines)) < 0) {
	current_group = 0;
      }
      break;
    case 0x04:		/* ^D			*/
    case 0x06:		/* ^F			*/
    case 0x16:		/* ^V ���ڡ���		*/
      if ((current_group += (term_lines - mode_lines))
	  >= board_select_number) {
	if (board_select_number > 0) {
	  current_group = board_select_number - 1;
	} else {
	  current_group = 0;
	}
      }
      break;
    case '<':		/* �ǽ�Υ��롼��	*/
      current_group = 0;
      break;
    case '>':		/* �Ǹ�Υ��롼��	*/
      if (board_select_number > 0) {
	current_group = board_select_number - 1;
      } else {
	current_group = 0;
      }
      break;
    case '\t':		/* TAB ���ꥫ�ƥ���/���롼�פإ�����	*/
      strcpy(buff1, board_spool);
      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
		 "�ܡ��ɥ��ƥ���ޤ��ϥ��롼��̾�����Ϥ��Ʋ�����:",
		 "Input board category or board group name:", buff1);
      if (buff1[0]) {
	strcpy(jump_name, buff1);
	return(1);
      } else {
	top_position = -1;
      }
      break;
    case '?':		/* �إ��		*/
      help(board_group_jmessage, board_group_message,
	   GLOBAL_MODE_MASK | GROUP_MODE_MASK);
      top_position = -1;
      break;
    case 'l':
    case 'L':		/* ���ɥ��롼�ץѥå�/���	*/
      group_mask = !group_mask;
      board_match_group(select_name);
      current_group = 0;
      top_position = -1;
      break;
    case 'Q':		/* ��λ			*/
      return(1);
      /* break; */
    case 'G':		/* ����ɽ���⡼��/���	*/
      if (!select_name[0]) {
	gnus_mode = !gnus_mode;
	board_match_group(select_name);
	current_group = 0;
	top_position = -1;
      }
      break;
    case 'a':		/* �ܡ������		*/
      if (board_select_number > 0) {
	board_get_adrs(board_selected_group[current_group].group_name, buff1);
      } else {
	buff1[0] = '\0';
      }
      mail_send(buff1, "");
      top_position = -1;
      break;
    default:
      if (board_select_number > 0) {
	switch (key) {
	case ' ':
	case 'i':		/* ���롼������		*/
	  strcpy(buff1, board_selected_group[current_group].group_name);
	  strcpy(buff2, select_name);
	  strcpy(select_name,
		 board_selected_group[current_group].group_name);
	  if (board_selected_group[current_group].max_article < 0) {
	    status = board_menu();
	  } else {
	    status = board_select_article();
	  }
	  strcpy(select_name, buff2);
	  if (status) {
	    return(1);
	  }
	  board_match_group(select_name);
	  current_group = 0;
	  for (i = 0; i < board_select_number; i++) {
	    current_group = i;
	    if (strncmp(board_selected_group[i].group_name, buff1,
			strlen(buff1)) >= 0) {
	      break;
	    }
	  }
	  top_position = -1;
	  break;
#ifndef	SMALL
	case '/':		/* ��������		*/
	  board_search_name(0, board_select_number, &current_group);
	  break;
	case '\\':		/* ��������		*/
	  board_search_name(1, board_select_number, &current_group);
	  break;
#endif	/* !SMALL */
	case 'u':
	case 'U':		/* ���ɥޡ�������	*/
	  status = board_selected_group[current_group].unsubscribe =
	    !board_selected_group[current_group].unsubscribe;
	  if (board_selected_group[current_group].max_article < 0) {
	    j = strlen(board_selected_group[current_group].group_name);
	    for (i = 0; i < board_group_number; i++) {
	      if ((!strncmp(board_group[i].group_name,
			    board_selected_group[current_group].group_name,
			    j)) &&
		  ((!board_group[i].group_name[j]) ||
		   (board_group[i].group_name[j] ==
		    BOARD_GROUP_SEPARATER))) {
		board_group[i].unsubscribe = status;
	      }
	    }
	  } else {
	    for (i = 0; i < board_group_number; i++) {
	      if (!strcmp(board_group[i].group_name,
			  board_selected_group[current_group].group_name)) {
		board_group[i].unsubscribe = status;
	      }
	    }
	  }
	  putchar(board_subscribe_char[status]);
	  if (key == 'U') {
	    current_group = board_prev_sgroup(current_group);
	  } else {
	    current_group = board_next_sgroup(current_group);
	  }
	  status = 0;
	  break;
	case 'c':		/* ���롼�����ޡ���	*/
	  if (yes_or_no(NORMAL_YN_MODE,
			"�ܥ��ƥ���/���롼�פ����ƥޡ������Ƥ�����Ǥ���?",
			"Mark all articles in this news category/group.Are you sure?")) {
	    board_selected_group[current_group].unread_article = 0;
	    if (board_selected_group[current_group].max_article < 0) {
	      j = strlen(board_selected_group[current_group].group_name);
	      for (i = 0; i < board_group_number; i++) {
		if ((!strncmp(board_group[i].group_name,
			      board_selected_group[current_group].group_name,
			      j)) &&
		    ((!board_group[i].group_name[j]) ||
		     (board_group[i].group_name[j] ==
		      BOARD_GROUP_SEPARATER))) {
		  board_fill_mark(i);
		  board_group[i].unread_article = 0;
		}
	      }
	    } else {
	      for (i = 0; i < board_group_number; i++) {
		if (!strcmp(board_group[i].group_name,
			    board_selected_group[current_group].group_name)) {
		  board_fill_mark(i);
		  board_group[i].unread_article = 0;
		}
	      }
	    }
	  }
	  current_group = board_next_unread_sgroup(current_group);
	  top_position = -1;
	  break;
	}
	status = 0;
      }
      break;
    }
  }
  return(0);
}

/*
 * ����������̺�����
 */

static void	board_redraw_article(current_group, current_article)
     int	current_group;
     int	current_article;
{
  register int	i;
  char		buff[SMALL_BUFF];
  char		jname[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_article,
				  board_article_number, news_thread_mode,
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
  get_jname(BOARD_JN_DOMAIN, jname, 48);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "���롼��:%-48.48s       ����:%s" :
		  "Group:%-48.48s      Position:%s",
		  jname, buff);
  print_articles(top_position, current_group, board_article_number,
		 board_get_field, "CANCEL/LOST");
  print_mode_line(japanese ?
		  "?:�إ�� j,^N:���� k,^P:���� SPACE,i,.:���� o,q:���� Q:��λ" :
		  "?:help j,^N:next k,^P:previous SPACE,i,.:read o,q:return Q:exit");
}

/*
 * ��������
 */

static int	board_select_article()
{
  register int	current_group;		/* ������Υ��롼���ֹ�		*/
  int		current_article;	/* ������ε����ֹ�		*/
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  int		loop;
  int		status;
  int		key;
  register int	i, j;

  current_group = -1;
  jump_name[0] = '\0';
  top_position = -1;
  for (i = 0; i < board_group_number; i++) {
    if (!strcmp(select_name, board_group[i].group_name)) {
      current_group = i;
      break;
    }
  }
  if (current_group < 0) {
    print_fatal("Unexpected board group selected.");
    return(0);
  }
  status = board_get_list(current_group);
  if (board_group[current_group].min_article > 1) {
    board_add_mark(current_group, 1,
		   board_group[current_group].min_article - 1);
  }
  if (status < 0) {
    return(0);
  } else if (!status) {
    /*
     * �ǽ��̤�ɵ����ޤǥ����Ȥ�ʤ��
     */

    current_article = board_next_unread_article2(current_group, 0);
  } else {
    current_article = 0;
  }

  loop = 1;
  status = 0;
  while (loop) {
    board_redraw_article(current_group, current_article);
    term_locate(article_format ? 6 : 11,
		head_lines + current_article - top_position);
    key = get_key(NEWS_MODE_MASK | GLOBAL_MODE_MASK | SUBJECT_MODE_MASK |
		  MOUSE_MODE_MASK);
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
	if (line < board_article_number) {
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
    case 0x0c:		/* ^L ������		*/
      term_init(2);
      top_position = -1;
      break;
    case 0x10:		/* ^P ��������ư	*/
    case 'k':
      current_article = board_prev_article(current_article);
      break;
    case 0x0e:		/* ^N ��������ư	*/
    case 'j':
      current_article = board_next_article(current_article);
      break;
    case '\015':	/* RETURN		*/
    case '\n':
    case 'o':
    case 'q':
      loop = 0;
      break;
    case 0x02:		/* ^B 			*/
    case 0x15:		/* ^U ���ڡ���		*/
      if ((current_article -= (term_lines - mode_lines)) < 0) {
	current_article = 0;
      }
      break;
    case 0x04:		/* ^D			*/
    case 0x06:		/* ^F			*/
    case 0x16:		/* ^V ���ڡ���		*/
      if ((current_article += (term_lines - mode_lines))
	  >= board_article_number) {
	if (board_article_number > 0) {
	  current_article = board_article_number - 1;
	} else {
	  current_article = 0;
	}
      }
      break;
    case '<':		/* �ǽ�ε���		*/
      current_article = 0;
      break;
    case '>':		/* �Ǹ�ε���		*/
      if (board_article_number > 0) {
	current_article = board_article_number - 1;
      } else {
	current_article = 0;
      }
      break;
    case '?':		/* �إ��		*/
      help(board_article_jmessage, board_article_message,
	   GLOBAL_MODE_MASK | SUBJECT_MODE_MASK);
      top_position = -1;
      break;
    case 'Q':		/* ��λ			*/
      loop = 0;
      status = 1;
      break;
    case 'l':		/* ���ɵ����ѥå�/���	*/
    case 't':		/* ����åɥ�����	*/
    case 'T':		/* ����åɥ롼��	*/
      if (!change_sort_rule(current_group, &board_article_number,
			    &current_article,
			    &news_article_mask, &news_thread_mode,
			    board_get_list, key)) {
	top_position = -1;
      }
      break;
    case 'a':		/* �ܡ������		*/
      board_get_adrs(select_name, buff1);
      mail_send(buff1, "");
      top_position = -1;
      break;
    default:
      last_key = key;
      while (last_key) {
	key = last_key;
	last_key = 0;
	switch (key) {
	case 'p':		/* ��̤�ɵ�������	*/
	case 'P':		/* ����������		*/
	  j = current_article;
	  if (board_article_number > 0) {
	    current_article = board_prev_article(current_article);
	    if (key == 'p') {
	      current_article = board_prev_unread_article2(current_group,
							 current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(!current_article);
	    } else {
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    i = board_prev_unread_group(current_group);
	    if (i >= 0) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"����̤�ɥ��롼��(%s)�򻲾Ȥ��ޤ���?",
				"Read previous unread group(%s)?",
				board_group[i].group_name)) {
	      case 1:
		board_count_unread(current_group);
		strcpy(jump_name, board_group[i].group_name);
		return(1);
		/* break; */
	      case 2:
		board_count_unread(current_group);
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
	case 'n':		/* ��̤�ɵ�������	*/
	case 'N':		/* ����������		*/
	  j = current_article;
	  if (board_article_number > 0) {
	    current_article = board_next_article(current_article);
	    if (key == 'n') {
	      current_article = board_next_unread_article2(current_group,
							   current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(current_article == (board_article_number - 1));
	    } else {
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    i = board_next_unread_group(current_group);
	    if (i >= 0) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"����̤��(%d)���롼��(%s)�򻲾Ȥ��ޤ���?",
				"Read next unread(%d) group(%s)?",
				board_group[i].unread_article,
				board_group[i].group_name)) {
	      case 1:
		board_count_unread(current_group);
		strcpy(jump_name, board_group[i].group_name);
		return(1);
		/* break; */
	      case 2:
		board_count_unread(current_group);
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
	if (board_article_number > 0) {
	  i = 1;
	  switch (key) {
	  case 'V':		/* ��������		*/
	    i = 3;
	    /* break ���� */
	  case 'v':
	    i--;
	    key = ' ';
	    /* break ���� */
	  case ' ':
	  case 'p':
	  case 'n':
	  case 'P':
	  case 'N':
	  case 'i':
	  case '.':
	    if (!board_read(article_list[current_article].real_number, i)) {
	      switch (last_key) {
	      case 'D':
	      case 'Y':
		board_mark(current_group, current_article, READ_MARK);
		last_key = 'p';
		break;
	      case 'd':
	      case 'y':
		board_mark(current_group, current_article, READ_MARK);
		last_key = 'n';
		break;
	      case 'U':
	      case 'Z':
		board_unmark(current_group, current_article);
		last_key = 'P';
		break;
	      case 'u':
	      case 'z':
		board_unmark(current_group, current_article);
		last_key = 'N';
		break;
	      default:
		board_mark(current_group, current_article, READ_MARK);
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
		current_article = board_prev_unread_article2(current_group,
							    current_article);
		break;
	      case 'n':
	      case ' ':
		current_article = board_next_unread_article2(current_group,
							    current_article);
		break;
	      case 'P':
		current_article = board_prev_article(current_article);
		break;
	      case 'N':
		current_article = board_next_article(current_article);
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
	  case 'D':		/* �ޡ���,��������ư	*/
	  case 'Y':
	  case 'd':		/* �ޡ���,��������ư	*/
	  case 'y':
	    board_mark(current_group, current_article, READ_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (isupper(key)) {
	      current_article = board_prev_unread_article(current_article);
	    } else {
	      current_article = board_next_unread_article(current_article);
	    }
	    break;
	  case 'U':		/* �ޡ������,��������ư	*/
	  case 'Z':
	  case 'u':		/* �ޡ������,��������ư	*/
	  case 'z':
	    board_unmark(current_group, current_article);
	    toggle_mark(top_position, current_article, 0);
	    if (isupper(key)) {
	      current_article = board_prev_article(current_article);
	    } else {
	      current_article = board_next_article(current_article);
	    }
	    break;
	  case 'c':		/* �������ޡ���		*/
	    if (yes_or_no(NORMAL_YN_MODE,
			  "��������ޡ������Ƥ�����Ǥ���?",
			  "Mark all articles.Are you sure?")) {
	      for (i = 0; i < board_article_number; i++) {
		article_list[i].mark |= READ_MARK;
	      }
	      board_fill_mark(current_group);
	    }
	    top_position = -1;
	    break;
	  case 's':		/* ����������		*/
	    if (!multi_save(current_article, board_extract)) {
	      multi_add_mark(board_article_number, current_group,
			     current_article, READ_MARK, board_mark);
	    }
	    top_position = -1;
	    break;
#ifdef	MH
	  case 'O':		/* �ե�����ص��������� */
	    if (!save_mh_folder(current_article, board_extract)) {
	      multi_add_mark(board_article_number, current_group,
			     current_article, READ_MARK, board_mark);
	    }
	    top_position = -1;
	    break;
#endif	/* MH */
#ifndef	SMALL
	  case '/':		/* ��������		*/
	    search_subjects(0, board_article_number, &current_article,
			    board_get_field);
	    break;
	  case '\\':		/* ��������		*/
	    search_subjects(1, board_article_number, &current_article,
			    board_get_field);
	    break;
	  case '|':		/* �����ѥ��׼¹�	*/
	    if (!multi_pipe(current_article, board_extract)) {
	      multi_add_mark(board_article_number, current_group,
			     current_article, READ_MARK, board_mark);
	    }
	    top_position = -1;
	    break;
#endif	/* !SMALL */
	  case '\t':		/* TAB ���국���إ�����	*/
	    buff1[0] = '\0';
	    input_line(INPUT_SPCCUT_MASK, "�����ֹ�����Ϥ��Ʋ�����:",
		       "Input article number:", buff1);
	    if ((j = atoi(buff1)) > 0) {
	      for (i = 0; i < board_article_number; i++) {
		if (article_list[i].real_number == j) {
		  current_article = i;
		  break;
		}
	      }
	    }
	    top_position = -1;
	    break;
#ifdef	MAILSEND
	  case 'r':		/* �᡼���ֿ�		*/
	    sprintf(buff1, "%s%c%s%c%d", board_spool, SLASH_CHAR, select_name,
		    SLASH_CHAR, article_list[current_article].real_number);
	    mail_reply(buff1, REPLY_BOARD_MASK, "");
	    top_position = -1;
	    break;
	  case 'R':		/* �᡼���ֿ�		*/
	    create_temp_name(buff1, "BR");
	    if (!multi_extract(current_article, buff1, board_extract)) {
	      board_get_adrs(select_name, buff2);
	      if (!mail_reply(buff1, REPLY_QUOTE_MASK | REPLY_BOARD_MASK,
			      "", buff2)) {
		multi_add_mark(board_article_number, current_group,
			       current_article, READ_MARK, board_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -1;
	    break;
	  case '=':		/* �᡼��ž��		*/
	    create_temp_name(buff1, "BT");
	    if (!multi_extract(current_article, buff1, board_extract)) {
	      if (!mail_forward(buff1, "")) {
		multi_add_mark(board_article_number, current_group,
			       current_article, READ_MARK, board_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -1;
	    break;
#ifdef	LARGE
	  case '~':		/* �᡼�����		*/
	    create_temp_name(buff1, "BL");
	    if (!multi_extract(current_article, buff1, board_extract)) {
	      if (!mail_relay(buff1, "")) {
		multi_add_mark(board_article_number, current_group,
			       current_article, READ_MARK, board_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -1;
	    break;
#endif	/* LARGE */
	  case 'f':		/* �ܡ��ɥե���	*/
	    sprintf(buff1, "%s%c%s%c%d", board_spool, SLASH_CHAR, select_name,
		    SLASH_CHAR, article_list[current_article].real_number);
	    board_get_adrs(select_name, buff2);
	    mail_reply(buff1, FOLLOW_BOARD_MASK, "", buff2);
	    top_position = -1;
	    break;
	  case 'F':		/* �ܡ��ɥե���	*/
	    create_temp_name(buff1, "BF");
	    if (!multi_extract(current_article, buff1, board_extract)) {
	      board_get_adrs(select_name, buff2);
	      if (!mail_reply(buff1, REPLY_QUOTE_MASK | FOLLOW_BOARD_MASK,
			      "", buff2)) {
		multi_add_mark(board_article_number, current_group,
			       current_article, READ_MARK, board_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -1;
	    break;
	  case 'C':		/* ����󥻥�		*/
	    sprintf(buff1, "%s%c%s%c%d", board_spool, SLASH_CHAR, select_name,
		    SLASH_CHAR, article_list[current_article].real_number);
	    if (!board_cancel(buff1)) {
	      article_list[current_article].mark |= CANCEL_MARK;
	    }
	    top_position = -1;
	    break;
#endif	/* MAILSEND */
	  case 'M':		/* �ޥ���ޡ�������/���	*/
	    multi_mark(top_position, current_article);
	    current_article = board_next_article(current_article);
	    break;
#ifndef	SMALL
	  case 'K':		/* Ʊ��Subject�ޡ���,��������ư	*/
	    kill_subjects(current_group, current_article, board_article_number,
			  board_get_field, board_add_mark);
	    current_article = board_next_unread_article2(current_group,
							 current_article);
	    top_position = -1;
	    break;
#endif	/* !SMALL */
#ifdef	LARGE
	  case '@':		/* ����			*/
	    if (!multi_print(current_article, board_extract)) {
	      multi_add_mark(board_article_number, current_group,
			     current_article, READ_MARK, board_mark);
	    }
	    top_position = -1;
	    break;
	  case '(':		/* ������������		*/
	    search_articles(0, board_article_number, &current_article, 
			    board_extract);
	    break;
	  case ')':		/* ������������		*/
	    search_articles(1, board_article_number, &current_article, 
			    board_extract);
	    break;
	  case '+':		/* �����������ޡ���	*/
	    if (yes_or_no(NORMAL_YN_MODE,
			  "��������������ޡ������Ƥ�����Ǥ���?",
			  "Mark all forward articles.Are you sure?")) {
	      for (i = current_article; i < board_article_number; i++) {
		article_list[i].mark |= READ_MARK;
		board_add_mark(current_group,
			       article_list[i].real_number,
			       article_list[i].real_number);
	      }
	    }
	    top_position = -1;
	    break;
	  case '-':		/* �����������ޡ���	*/
	    if (yes_or_no(NORMAL_YN_MODE,
			  "��������������ޡ������Ƥ�����Ǥ���?",
			  "Mark all backward articles.Are you sure?")) {
	      for (i = 0; i <= current_article; i++) {
		article_list[i].mark |= READ_MARK;
		board_add_mark(current_group,
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
	case 'K':
	case 'O':
	case 's':
	  break;
	default:
	  last_key = 0;
	}
      }
      break;
    }
  }
  board_count_unread(current_group);
  return(status);
}

/*
 * ��̤�ɥ��롼���ֹ����
 */

int	board_prev_unread_group(current_group)
     int	current_group;
{
  while (1) {
    if (--current_group < 0) {
      current_group = -1;
      break;
    }
    if ((board_group[current_group].unread_article > 0) &&
	((!board_group[current_group].unsubscribe) || (!group_mask))) {
      break;
    }
  }
  return(current_group);
}

/*
 * ��̤�ɥ��롼���ֹ����
 */

int	board_next_unread_group(current_group)
     int	current_group;
{
  while (1) {
    if (++current_group >= board_group_number) {
      current_group = -1;
      break;
    }
    if ((board_group[current_group].unread_article > 0) &&
	((!board_group[current_group].unsubscribe) || (!group_mask))) {
      break;
    }
  }
  return(current_group);
}

/*
 * ������̤�ɥ��롼���ֹ����
 */

static int	board_prev_unread_sgroup(current_group)
     int	current_group;
{
  register int	i;

  i = current_group;
  while (i > 0) {
    if (board_selected_group[--i].unread_article) {
      current_group = i;
      break;
    }
  }
  return(current_group);
}

/*
 * �����򥰥롼���ֹ����
 */

static int	board_next_unread_sgroup(current_group)
     int	current_group;
{
  register int	i;

  i = current_group;
  while (i < (board_select_number - 1)) {
    if (board_selected_group[++i].unread_article) {
      current_group = i;
      break;
    }
  }
  return(current_group);
}

/*
 * �����򥰥롼���ֹ����
 */

static int	board_prev_sgroup(current_group)
     int	current_group;
{
  if (--current_group < 0) {
    current_group = 0;
  }
  return(current_group);
}

/*
 * �����򥰥롼���ֹ����
 */

static int	board_next_sgroup(current_group)
     int	current_group;
{
  if (++current_group >= board_select_number) {
    if (board_select_number > 0) {
      current_group = board_select_number - 1;
    } else {
      current_group = 0;
    }
  }
  return(current_group);
}

/*
 * �������ֹ����
 */

static int	board_prev_article(current_article)
     int	current_article;
{
  if (--current_article < 0) {
    current_article = 0;
  }
  return(current_article);
}

/*
 * �������ֹ����
 */

static int	board_next_article(current_article)
     int	current_article;
{
  if (++current_article >= board_article_number) {
    if (board_article_number > 0) {
      current_article = board_article_number - 1;
    } else {
      current_article = 0;
    }
  }
  return(current_article);
}

/*
 * ��̤�ɵ����ֹ����
 */

static int	board_prev_unread_article(current_article)
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
 * ��̤�ɵ����ֹ����
 */

static int	board_next_unread_article(current_article)
     int	current_article;
{
  term_enable_abort();
  while (current_article < (board_article_number - 1)) {
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
 * ��̤�ɵ����ֹ����(����󥻥뵭���ϼ�ư�ޡ���)
 */

static int	board_prev_unread_article2(current_group, current_article)
     int	current_group;
     int	current_article;
{
  register int	current_article2, current_article3;
  int		current_article4;

  current_article2 = current_article;
  if (board_article_number > 0) {
    while (1) {
      if (article_list[current_article].mark & READ_MARK) {
	if (!current_article) {
	  break;
	}
	current_article = board_prev_unread_article(current_article);
      } else {
	if (article_list[current_article].mark & UNFETCH_MARK) {
	  board_get_field(current_group, current_article);
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
	  board_add_mark(current_group,
			 article_list[current_article4].real_number,
			 article_list[current_article3++].real_number);
	}
      } else {
	board_add_mark(current_group,
		       article_list[current_article3].real_number,
		       article_list[current_article2].real_number);
      }
    }
  }
  return(current_article);
}

/*
 * ��̤�ɵ����ֹ����(����󥻥뵭���ϼ�ư�ޡ���)
 */

static int	board_next_unread_article2(current_group, current_article)
     int	current_group;
     int	current_article;
{
  register int	current_article2, current_article3;
  int		current_article4;

  current_article2 = current_article;
  if (board_article_number > 0) {
    while (1) {
      if (article_list[current_article].mark & READ_MARK) {
	if (current_article >= board_article_number - 1) {
	  break;
	}
	current_article = board_next_unread_article(current_article);
      } else {
	if (article_list[current_article].mark & UNFETCH_MARK) {
	  board_get_field(current_group, current_article);
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
	  board_add_mark(current_group,
			 article_list[current_article4].real_number,
			 article_list[current_article2++].real_number);
	}
      } else {
	board_add_mark(current_group,
		       article_list[current_article2].real_number,
		       article_list[current_article3].real_number);
      }
    }
  }
  return(current_article);
}

/*
 * ��������
 */

static int	board_read(real_number, mode)
     int	real_number;
     int	mode;
{
  char	board_file[PATH_BUFF];
  char	tmp_file[PATH_BUFF];
  char	buff[SMALL_BUFF];

  sprintf(board_file, "%s%c%s%c%d", board_spool, SLASH_CHAR, select_name,
	  SLASH_CHAR, real_number);
  create_temp_name(tmp_file, "BV");
  sprintf(buff, "%d", real_number);
  if (exec_pager(board_file, tmp_file, mode, buff)) {
    return(1);
  }
  return(0);
}

/*
 * �������
 */

static int	board_extract(real_number, tmp_file)
     int	real_number;
     char	*tmp_file;
{
  FILE	*fp1, *fp2;
  char	board_file[PATH_BUFF];
  char	buff[BUFF_SIZE];
  int	status;

  sprintf(board_file, "%s%c%s%c%d", board_spool, SLASH_CHAR, select_name,
	  SLASH_CHAR, real_number);
  fp1 = fopen(board_file, "r");
  if (fp1 == (FILE*)NULL) {
    return(1);
  }
  fp2 = fopen2(tmp_file, "a");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't open extract file.");
    fclose(fp1);
    return(1);
  }
  status = 0;
  while (fgets(buff, sizeof(buff), fp1)) {
    if (fputs(buff, fp2) == EOF) {
      status = 1;
    }
  }
  fclose(fp1);
  fclose(fp2);
  if (status) {
    funlink2(tmp_file);
    return(status);
  }
  return(0);
}

/*
 * �������������
 */

static void	board_count(current_group, mode)
     int	current_group;
     int	mode;	/* 0:�Ƶ����ʤ�,0�ʳ�:�Ƶ� */
{
  /* �ѿ��ϺƵ�����Τ� static �ˤ��ʤ����� */

#ifdef	BOARD_COUNT_FILE
  FILE		*fp;
#endif	/* BOARD_COUNT_FILE */
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		buff3[BUFF_SIZE];
  char		*ptr, *ptr2;
  struct stat	stat_buff;
  DIR_PTR	*dp;
  DIR		*dir_ptr;
  int		top_flag;
  int		status;
  register int	max_number;
  register int	min_number;
  register int	i, j;

  max_number = min_number = 0;
  if (board_group[current_group].group_name[0]) {
    top_flag = status = 0;
    sprintf(buff1, "%s%c%s", board_spool, SLASH_CHAR,
	    board_group[current_group].group_name);
  } else {
    top_flag = status = 1;
    strcpy(buff1, board_spool);
  }
#ifdef	BOARD_COUNT_FILE
  sprintf(buff2, "%s%c%s", buff1, SLASH_CHAR, BOARD_COUNT_FILE);
  fp = fopen(buff2, "r");
  if (fp != (FILE*)NULL) {
    if (fgets(buff2, sizeof(buff2), fp)) {
      if (max_number = atoi(buff2)) {
#ifdef	BOARD_NEXT_COUNT
	max_number--;
#endif	/* BOARD_NEXT_COUNT */
	min_number = 1;
      }
    }
    fclose(fp);
  } else {
    status = 1;
  }
#endif	/* BOARD_COUNT_FILE */
  if (!status) {
    board_group_number++;
  }
  strcpy(buff2, buff1);
  ptr = buff2;
  while (*ptr) {
    ptr++;
  }
  *ptr++ = BOARD_GROUP_SEPARATER;
  if ((dir_ptr = opendir(buff1)) != (DIR*)NULL) {
    strcpy(buff1, board_group[current_group].group_name);
    while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
      strcpy(ptr, dp->d_name);
#ifdef	BOARD_COUNT_FILE
      i = ((*ptr < '0') || (*ptr > '9'));
#else	/* !BOARD_COUNT_FILE */
      ptr2 = ptr;
      i = 0;
      while (1) {
	j = *ptr2++ - '0';
	if ((j >= 0) && (j < 10)) {
	  i = i * 10 + j;
	} else {
	  ptr2--;
	  break;
	}
      }
      if (!(*ptr2) && i) {
	if ((min_number > i) || (min_number == 0)) {
	  min_number = i;
	}
	if (max_number < i) {
	  max_number = i;
	}
	i = top_flag;
      } else {
	i = 1;
      }
#endif	/* !BOARD_COUNT_FILE */
      if (i && mode) {
	if (!stat(buff2, &stat_buff)) {
	  if (stat_buff.st_mode & S_IFDIR) {
	    if (strcmp(ptr, ".") && strcmp(ptr, "..")) {
	      if (!top_flag) {
		sprintf(buff3, "%s%c%s", buff1, SLASH_CHAR, ptr);
	      } else {
		strcpy(buff3, ptr);
	      }
#ifdef	USE_NLINK
	      board_search_group(buff3, stat_buff.st_nlink > 2);
#else	/* !USE_NLINK */
	      board_search_group(buff3, 1);
#endif	/* !USE_NLINK */
	    }
	  }
	}
      }
    }
    closedir(dir_ptr);
  }
  if (!status) {
    board_group[current_group].min_article = min_number;
    board_group[current_group].max_article = max_number;
  }
}

/*
 * �����롼�׸���
 */

static void	board_search_all_group()
{
  char		*ptr;
  register int	i;

  board_group_number = board_alloc_number = 0;
  if (board_group != (struct board_group*)NULL) {
    free(board_group);
    board_group = (struct board_group*)NULL;
  }
  if (board_selected_group) {
    free(board_selected_group);
    board_selected_group = (struct board_group*)NULL;
  }
  board_search_group("", 1);
  if (board_group_number > 0) {
    for (i = 0; i < board_group_number; i++) {	/* ��������������	*/
      ptr = board_group[i].group_name;
      while (*ptr) {
	if (*ptr == BOARD_GROUP_SEPARATER) {
	  *ptr++ = ' ';
	} else {
	  ptr++;
	}
      }
    }
    qsort(board_group, board_group_number, sizeof(struct board_group),
	  (int (*)())strcmp);
    for (i = 0; i < board_group_number; i++) {	/* �����������θ����	*/
      ptr = board_group[i].group_name;
      while (*ptr) {
	if (*ptr == ' ') {
	  *ptr++ = BOARD_GROUP_SEPARATER;
	} else {
	  ptr++;
	}
      }
    }
  }
}

/*
 * ���롼�׸���
 */

static void	board_search_group(group_name, mode)
     char	*group_name;
     int	mode;	/* 0:�Ƶ����ʤ�,0�ʳ�:�Ƶ� */
{
  struct board_group	*board_alloc_ptr;

  if (board_alloc_number <= board_group_number) {
    if (board_group) {
      board_alloc_ptr = (struct board_group*)
	realloc(board_group, (board_alloc_number + BOARD_ALLOC_COUNT)
		* sizeof(struct board_group));
    } else {
      board_alloc_ptr = (struct board_group*)
	malloc((board_alloc_number + BOARD_ALLOC_COUNT)
	       * sizeof(struct board_group));
    }
    if (board_alloc_ptr == (struct board_group*)NULL) {
      print_fatal("Can't allocate memory for board group.");
      return;
    }
    board_group = board_alloc_ptr;
    board_alloc_number += BOARD_ALLOC_COUNT;
  }
  if (*group_name) {
    strcpy(board_group[board_group_number].group_name, group_name);
    board_count(board_group_number, mode);
  } else {
    board_group[0].group_name[0] = '\0';
    board_count(0, mode);
  }
}

/*
 * �ޥå����륰�롼�׸���
 */

static void	board_match_group(group_name)
     char	*group_name;
{
  int		length1, length2;
  register int	i, j, k, l;
  int		max, min, unread, unsubscribe;
  int		match;
  char		*ptr;

  board_select_number = 0;
  if (!board_select_number) {
    if (!(board_selected_group = (struct board_group*)
	  malloc(board_group_number * sizeof(struct board_group)))) {
      print_fatal("Can't allocate memory for select group.");
      return;
    }
  }

  /*
   * ����(GNUS like)�⡼�ɤν���
   */

  if (gnus_mode) {
    for (i = 0; i < board_group_number; i++) {
      if ((!group_mask) || ((board_group[i].unread_article > 0) &&
			    (!board_group[i].unsubscribe))) {
	strcpy(board_selected_group[board_select_number].group_name,
	       board_group[i].group_name);
	board_selected_group[board_select_number].max_article =
	  board_group[i].max_article;
	board_selected_group[board_select_number].min_article =
	  board_group[i].min_article;
	board_selected_group[board_select_number].unread_article =
	  board_group[i].unread_article;
	board_selected_group[board_select_number].unsubscribe = 0;
	board_select_number++;
      }
    }
  } else {

    /*
     * ����(vin like)�⡼�ɤν���
     * �������鲼�ν����� mnews �������ܤ����򤬺������ʬ�Ǥ���
     * �����ν�������ñ������Ǥ���ͤ���º�ɤ��ޤ���
     */

    j = l = 0;
    length1 = strlen(group_name);
    for (i = 0; i < board_group_number; i++) {
      if ((!strncmp(board_group[i].group_name, group_name, length1)) &&
	  ((!group_name[0]) ||
	   (!board_group[i].group_name[length1]) ||
	   (board_group[i].group_name[length1] == BOARD_GROUP_SEPARATER))) {
	match = 1;
	ptr = &board_group[i].group_name[length1];
	if (*ptr) {
	  k = 0;
	  if (*ptr == BOARD_GROUP_SEPARATER) {
	    ptr++;
	    k++;
	  }
	  while (*ptr) {
	    if (*ptr == BOARD_GROUP_SEPARATER) {
	      break;
	    }
	    ptr++;
	    k++;
	  }
	  k += length1;
	  for (; j < i; j++) {
	    if (!strncmp(board_group[i].group_name, board_group[j].group_name,
			 k)) {
	      match = 0;
	      break;
	    }
	  }
	} else {
	  k = length1;
	}
	if (match) {
	  unread = 0;
	  max = min = -1;
	  unsubscribe = board_group[i].unsubscribe;
	  length2 = k;
	  k = 0;
	  for (; l < board_group_number; l++) {
	    if ((!strncmp(board_group[i].group_name, board_group[l].group_name,
			  length2)) &&
		((!board_group[l].group_name[length2]) ||
		 (board_group[l].group_name[length2]
		  == BOARD_GROUP_SEPARATER))) {
	      unsubscribe &= board_group[l].unsubscribe;
	      if ((!board_group[l].unsubscribe) || (!group_mask)) {
		unread += board_group[l].unread_article;
	      }
	      if (length2 == strlen(board_group[l].group_name)) {
		if (l + 1 >= board_group_number) {
		  k++;
		} else if ((strncmp(board_group[l].group_name,
				    board_group[l + 1].group_name,
				    strlen(board_group[l].group_name))) ||
			   (board_group[l + 1].group_name[strlen(board_group[l].group_name)]
			    != BOARD_GROUP_SEPARATER)) {
		  k++;
		} else if (strlen(group_name)
			   == strlen(board_group[l].group_name)) {
		  k++;
		}
		if (k == 1) {
		  max = board_group[l].max_article;
		  min = board_group[l].min_article;
		  break;
		}
	      }
	      k = 1;
	    } else if (k) {
	      break;
	    }
	  }
	  if ((!group_mask) || (!unsubscribe)) {
#ifdef	DEBUG
	    fprintf(stderr, "MATCH %04d:%s\n", i, board_group[i].group_name);
#endif	/* DEBUG */
	    strcpy(board_selected_group[board_select_number].group_name,
		   board_group[i].group_name);
	    board_selected_group[board_select_number].group_name[length2] =
	      '\0';
	    board_selected_group[board_select_number].unread_article = unread;
	    board_selected_group[board_select_number].max_article = max;
	    board_selected_group[board_select_number].min_article = min;
	    board_selected_group[board_select_number].unsubscribe =
	      unsubscribe;
	    board_select_number++;
	  }
	}
      }
    }
  }
}

/*
 * �����ꥹ�ȼ���
 */

static int	board_get_list(current_group)
     int	current_group;
{
  MARK_LIST	*mark_ptr;
  int		min_number;
  register int	i, j;

  min_number = board_group[current_group].min_article;
  board_article_number = board_group[current_group].max_article
    - min_number + 1;
  multi_number = 0;
  if ((board_article_number <= 0) || (min_number <= 0)) {
    board_article_number = 0;
    return(1);
#ifdef	BOARD_MAX_ARTICLE
  } else if (board_article_number > BOARD_MAX_ARTICLE) {
    board_article_number = BOARD_MAX_ARTICLE;
    min_number = board_group[current_group].max_article
      - board_article_number + 1;
#endif	/* BOARD_MAX_ARTICLE */
  }
  if (article_list != (ARTICLE_LIST*)NULL) {
    free(article_list);
  }
  if (!(article_list = (ARTICLE_LIST*)
	malloc(sizeof(ARTICLE_LIST) * board_article_number))) {
    print_fatal("Can't allocate memory for article struct.");
    board_article_number = 0;
    return(1);
  }
  for (i = 0; i < board_article_number; i++) {
    article_list[i].real_number = min_number + i;
    article_list[i].mark = UNFETCH_MARK;	/* �إå�̤����	*/
  }

  /*	���ɥޡ�������		*/

  mark_ptr = board_group[current_group].mark_ptr;
  while (mark_ptr != (MARK_LIST*)NULL) {
    if ((mark_ptr->end_number >= board_group[current_group].min_article) &&
	(mark_ptr->start_number <= board_group[current_group].max_article)) {
      for (i = mark_ptr->start_number; i <= mark_ptr->end_number; i++) {
	j = i - min_number;
	if ((j >= 0) && (j < board_article_number)) {
	  article_list[j].mark |= READ_MARK;
	}
      }
    }
    mark_ptr = mark_ptr->next_ptr;
  }

  /*	�����Ƚ���	*/

#ifdef	REF_SORT
  if (message_list != (MESSAGE_LIST*)NULL) {
    print_fatal("Unexpected message list allocation found.");
    free(message_list);
    message_list = (MESSAGE_LIST*)NULL;
  }
  if (board_article_number && (sort_rule == 1)) {
    message_list = (MESSAGE_LIST*)malloc(sizeof(MESSAGE_LIST)
					 * board_article_number);
  }
#endif	/* REF_SORT */
  if (sort_articles(current_group, &board_article_number,
		    news_thread_mode, news_article_mask,
		    board_get_field, board_add_mark) < 0) {
    return(-1);
  }

  /*	�ѥå�����	*/

  if (news_article_mask) {
    pack_articles(current_group, &board_article_number,
		  board_get_field, board_add_mark);
  }
  return(0);
}

/*
 * �����إå�����
 */

static int	board_get_field(current_group, current_article)
     int	current_group;
     int	current_article;		/*	�����̤��ֹ�	*/
{
  FILE			*fp;
  char			buff[BUFF_SIZE];
  static char		from_buff[BUFF_SIZE];
  static char		date_buff[MAX_FIELD_LEN];
  static char		subject_buff[BUFF_SIZE];
  static char		x_nsubj_buff[MAX_FIELD_LEN];
#ifdef	REF_SORT
  static char		message_buff[MAX_FIELD_LEN];
  static char		reference_buff[BUFF_SIZE];
  static char		in_reply_buff[BUFF_SIZE];
#endif	/* REF_SORT */
  static struct cpy_hdr	board_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
    {DATE_FIELD,	date_buff,	sizeof(date_buff)},
    {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
    {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
#ifdef	REF_SORT
    {MESSAGE_FIELD,	message_buff,	sizeof(message_buff)},
    {REFERENCE_FIELD,	reference_buff,	sizeof(reference_buff)},
    {IN_REPLY_FIELD,	in_reply_buff,	sizeof(in_reply_buff)},
#endif	/* REF_SORT */
  };
#ifdef	REF_SORT
  char			*ptr;
#else	/* !REF_SORT */
  short			year;
#endif	/* !REF_SORT */
  short			day, hour, minute, second;

  if ((current_article < 0) || (current_article >= board_article_number)) {
    return(1);
  }
  article_list[current_article].mark &= ~UNFETCH_MARK;
  sprintf(buff, "%s%c%s%c%d", board_spool, SLASH_CHAR, select_name, SLASH_CHAR,
	  article_list[current_article].real_number);
  fp = fopen(buff, "r");
  if (fp != (FILE*)NULL) {
    article_list[current_article].lines =
      copy_fields(fp, board_fields,
		  sizeof(board_fields) / sizeof(struct cpy_hdr),
		   CF_CLR_MASK | CF_GET_MASK | CF_CNT_MASK | CF_SPC_MASK);
    fclose(fp);
  } else {
    article_list[current_article].mark |= CANCEL_MARK;
    return(-1);
  }
  get_real_adrs(from_buff, article_list[current_article].from);
  if (x_nsubj_mode && x_nsubj_buff[0]) {
    recover_jis(subject_buff, x_nsubj_buff);
  }
  mime_decode_func(from_buff, subject_buff, default_code);
  euc_tab_strncpy(article_list[current_article].subject, from_buff,
		  MAX_SUBJECT_LEN - 1);
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
    } else {
      ptr = strrchr(in_reply_buff, '<');
      if (ptr != (char*)NULL) {
	strncpy(message_list[current_article].ref_id, ptr,
		MAX_FIELD_LEN - 1);
	message_list[current_article].ref_id[MAX_FIELD_LEN - 1] = '\0';
      } else {
	message_list[current_article].ref_id[0] = '\0';
      }
    }
    ptr = strchr(message_list[current_article].ref_id, '>');
    if (ptr != (char*)NULL) {
      *(ptr + 1) = '\0';
    }
  }
#else	/* !REF_SORT */
  convert_article_date(date_buff, &year, &article_list[current_article].month,
		       &article_list[current_article].date, &day, &hour,
		       &minute, &second, from_buff);
#endif	/* !REF_SORT */
  return(0);
}

/*
 * �ޡ����ե��������
 */

static void	board_read_initfile(file_name)
     char	*file_name;
{
  FILE		*fp;
  char		buff[BUFF_SIZE];
  char		group_name[BUFF_SIZE];
  char		*ptr;
  int		start_number;
  int		end_number;
  int		current_group;
  int		unsubscribe;
  int		line;
  register int	i;

  if (board_group_number <= 0) {
    return;
  }
  for (i = 0; i < board_group_number; i++) {
    board_group[i].mark_ptr = (MARK_LIST*)NULL;
    if (board_group[i].max_article) {
      board_group[i].unread_article =
	board_group[i].max_article - board_group[i].min_article + 1;
    } else {
      board_group[i].unread_article = 0;
    }
    board_group[i].unsubscribe = 0;
  }
  line = 0;
  fp = fopen(file_name, "r");
  if (fp != (FILE*)NULL) {
    print_mode_line(japanese ? "�ܡ���̤�ɴ����ե�����������Ǥ���" :
		    "Loading board mark-file.");
    while (fgets(buff, sizeof(buff), fp)) {
      line++;
      ptr = buff;
      while ((*ptr == '\t') || (*ptr == ' ')) {
	ptr++;
      }
      if ((*ptr == '\n') || (*ptr == '#') || (*ptr == '\0')) {
	continue;
      }
      i = 0;
      unsubscribe = 0;
      while (*ptr) {
	if (((*ptr == ':') || (*ptr == '!')) &&
	    ((*(ptr + 1) == ' ') || (*(ptr + 1) == '\t')
	     || (*(ptr + 1) == '\n'))) {
	  if (*ptr == '!') {
	    unsubscribe = 1;
	  }
	  ptr++;
	  group_name[i] = '\0';
	  i = -1;
	  while ((*ptr == ' ') || (*ptr == '\t')) {
	    ptr++;
	  }
	  break;
	}
	group_name[i++] = *ptr++;
      }
      if (i >= 0) {
	print_warning("Line %4d : Syntax error-1 in board mark-file.",
		      line);
	continue;
      }
      current_group = -1;
      for (i = 0; i < board_group_number; i++) {
#ifdef	notdef
	if (j = strcmp(group_name, board_group[i].group_name)) {
	  if (j < 0) {
	    break;
	  }
	} else {
	  current_group = i;
	  break;
	}
#else	/* !notdef */
	if (!strcmp(group_name, board_group[i].group_name)) {
	  current_group = i;
	  break;
	}
#endif	/* !notdef */
      }
      if (current_group < 0) {
	print_warning("Line %4d : Bogus board group \"%s\" in board mark-file.",
		      line, group_name);
	continue;
      }
      board_group[current_group].unsubscribe = unsubscribe;
      while (1) {
	if ((*ptr == '\\') && ((*(ptr + 1) == '\0') || (*(ptr + 1) == '\n'))) {
	  if (!fgets(buff, sizeof(buff), fp)) {
	    break;
	  }
	  line++;
	  ptr = buff;
	}
	if ((start_number = read_integer(fp, buff, sizeof(buff),
					 &ptr)) <= 0) {
	  if (start_number) {
	    print_warning("Line %4d : Syntax error-2 in board mark-file.",
			  line);
	  }
	  break;
	}
	end_number = start_number;
	if (*ptr == ',') {
	  ptr++;
	} else if (*ptr == '-') {
	  ptr++;
	  if ((end_number = read_integer(fp, buff, sizeof(buff),
					 &ptr)) < start_number) {
	    if (end_number) {
	      print_warning("Line %4d : Syntax error-3 in board mark-file.",
			    line);
	    }
	    break;
	  }
	  if (*ptr == ',') {
	    ptr++;
	  } else if (*ptr != '\n') {
	    print_warning("Line %4d : Syntax error-4 in board mark-file.",
			  line);
	    break;
	  }
	}
	board_add_mark(current_group, start_number, end_number);
      }
      board_count_unread(current_group);
    }
    fclose(fp);
  }
}

/*
 * �ޡ����ꥹ�������ɲ�
 */

static void	board_add_mark(current_group, start_number, end_number)
     int	current_group;
     int	start_number;
     int	end_number;
{
  if (current_group > board_group_number) {
    print_fatal("Illegal board group number.");
    return;
  }
  if ((start_number < 1) || (start_number > end_number)) {
    print_fatal("Illegal start or end board article list number.");
    return;
  }
  if ((start_number > board_group[current_group].max_article) &&
      (board_group[current_group].max_article > 0)) {
    print_warning("Illegal start board article list number.");
    return;
  }
  if ((end_number > board_group[current_group].max_article) &&
      (board_group[current_group].max_article > 0)) {
    print_warning("Illegal end board article list number.");
    end_number = board_group[current_group].max_article;
  }
#ifndef	DONT_OPTIMIZE_NEWSRC
  if (!board_group[current_group].min_article) {
    return;
  } else if (start_number < board_group[current_group].min_article) {
    start_number = 1;
    if (end_number < board_group[current_group].min_article) {
      end_number = board_group[current_group].min_article - 1;
    }
  }
#endif	/* !DONT_OPTIMIZE_NEWSRC */
#ifdef	DEBUG
  fprintf(stderr, "board group %s add mark %d - %d\n",
	  board_group[current_group].group_name, start_number, end_number);
#endif	/* DEBUG */
  add_mark(&board_group[current_group].mark_ptr, start_number, end_number);
}

/*
 * �ޡ����ꥹ�����Ǻ��
 */

static void	board_delete_mark(current_group, start_number, end_number)
     int	current_group;
     int	start_number;
     int	end_number;
{
  if ((start_number < 1) || (start_number > end_number)) {
    print_fatal("Illegal start or end board article list number.");
    return;
  }
  if ((start_number > board_group[current_group].max_article) &&
      (board_group[current_group].max_article > 0)) {
    print_warning("Illegal start board article list number.");
    return;
  }
  if ((end_number > board_group[current_group].max_article) &&
      (board_group[current_group].max_article > 0)) {
    print_warning("Illegal end board article list number.");
    end_number = board_group[current_group].max_article;
  }

#ifdef	DEBUG
  fprintf(stderr, "board group %s delete mark %d - %d\n",
	  board_group[current_group].group_name, start_number, end_number);
#endif	/* DEBUG */
  delete_mark(&board_group[current_group].mark_ptr, start_number, end_number);
}

/*
 * �ޡ����ꥹ���������ɲ�
 */

static void	board_fill_mark(current_group)
     int	current_group;
{
  MARK_LIST	*mark_ptr;

  board_clean_mark(current_group);
  if ((board_group[current_group].max_article > 0) &&
      (board_group[current_group].min_article <=
       board_group[current_group].max_article)) {
    if (!(mark_ptr = (MARK_LIST*)malloc(sizeof(MARK_LIST)))) {
      print_fatal("Can't allocate memory for board mark list.");
      return;
    }
    board_group[current_group].mark_ptr = mark_ptr;
    mark_ptr->prev_ptr = (MARK_LIST*)NULL;
    mark_ptr->next_ptr = (MARK_LIST*)NULL;
    mark_ptr->start_number = 1;
    mark_ptr->end_number   = board_group[current_group].max_article;
  }
}

/*
 * �ޡ����ꥹ�������Ǻ��
 */

static void	board_clean_mark(current_group)
     int	current_group;
{
  MARK_LIST	*mark_ptr;
  MARK_LIST	*free_ptr;

  mark_ptr = board_group[current_group].mark_ptr;
  while (mark_ptr != (MARK_LIST*)NULL) {
    free_ptr = mark_ptr;
    mark_ptr = mark_ptr->next_ptr;
    free(free_ptr);
  }
  board_group[current_group].mark_ptr = (MARK_LIST*)NULL;
}

/*
 * �����ޡ�����¸
 */

static void	board_mark(current_group, current_article, mark)
     int	current_group;
     int	current_article;
     int	mark;		/* ̤���Ѥ��� I/F ������Τ���ɬ��	*/
{
  if (mark & READ_MARK) {
    article_list[current_article].mark |= READ_MARK;
    board_add_mark(current_group,
		   article_list[current_article].real_number,
		   article_list[current_article].real_number);
  }
}

/*
 * �����ޡ������
 */

static void	board_unmark(current_group, current_article)
     int	current_group;
     int	current_article;
{
  article_list[current_article].mark &= ~READ_MARK;
  board_delete_mark(current_group,
		    article_list[current_article].real_number,
		    article_list[current_article].real_number);
}

/*
 * �ܡ��ɥ���󥻥�
 */

static int	board_cancel(cancel_file)
     char	*cancel_file;
{
  FILE			*fp;
  char			buff[SMALL_BUFF];
  static char		from_buff[BUFF_SIZE];
  static struct cpy_hdr	cancel_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
  };

  fp = fopen(cancel_file, "r");
  if (fp != (FILE*)NULL) {
    copy_fields(fp, cancel_fields,
		sizeof(cancel_fields) / sizeof(struct cpy_hdr),
		CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
    fclose(fp);
    sprintf(buff, "%s@%s", user_name, domain_name);
    if (strindex(from_buff, buff)) {
      if (yes_or_no(CARE_YN_MODE, "�����򥭥�󥻥뤷�Ƥ�����Ǥ���?",
		    "Cancel article.Are you sure?")) {
	return(unlink(cancel_file));
      }
    }
  }
  return(1);
}

/*
 * �ޡ����ե����륻����
 */

static void	board_save_initfile(file_name)
     char	*file_name;
{
  FILE		*fp;
  struct stat	stat_buff;
  char		backup_file[PATH_BUFF];
  int		current_group;

  if (board_group_number <= 0) {
    return;
  }

  /*	�Хå����åץե�����إ�͡���	*/

  create_backup_name(backup_file, file_name);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  if (rename(file_name, backup_file) && (errno != ENOENT)) {
    print_fatal("Can't rename board mark-file.");
    return;
  }
  if (stat(backup_file, &stat_buff)) {
    stat_buff.st_mode = 00644;
    if (errno != ENOENT) {
      print_fatal("Can't get stat of old mark-file.");
    }
  }

  fp = fopen(file_name, "w");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't save board mark-file.");
    return;
  }
  print_mode_line(japanese ? "�ܡ���̤�ɴ����ե�����򥻡�����Ǥ���" :
		  "Saving board mark-file.");
  for (current_group = 0; current_group < board_group_number;
       current_group++) {
    save_group(fp, board_group[current_group].group_name,
	       board_group[current_group].mark_ptr,
	       board_group[current_group].unsubscribe);
  }
  fclose(fp);
  chmod(file_name, ((stat_buff.st_mode & 01777) | S_IREAD | S_IWRITE));
}

/*
 * ̤�ɵ���������
 */

static void	board_count_unread(current_group)
     int	current_group;
{
  MARK_LIST	*mark_ptr;
  register int	start_number;
  register int	end_number;
  register int	max_number;
  register int	min_number;
  register int	unread_number;

  max_number = board_group[current_group].max_article;
  min_number = board_group[current_group].min_article;
  if (max_number < 1) {
    board_group[current_group].min_article = min_number =
      board_group[current_group].max_article = max_number = 0;
    unread_number = 0;
  } else {
    unread_number = max_number - min_number + 1;
  }
  mark_ptr = board_group[current_group].mark_ptr;
  while (mark_ptr != (MARK_LIST*)NULL) {
    start_number = mark_ptr->start_number;
    end_number = mark_ptr->end_number;
    if (start_number <= min_number) {
      if (end_number >= max_number) {
	unread_number = 0;
      } else if (end_number >= min_number) {
	unread_number -= (end_number - min_number + 1);
      }
    } else {
      if (end_number <= max_number) {
	unread_number -= (end_number - start_number + 1);
      } else if (start_number <= max_number) {
	unread_number -= (max_number - start_number + 1);
      }
    }
    if (unread_number < 0) {
      print_warning("Illegal unread board article number.");
#ifdef	DEBUG
      print_mark(board_group[current_group].mark_ptr);
      exit(1);
#endif	/* DEBUG */
      unread_number = 0;
    }
    mark_ptr = mark_ptr->next_ptr;
  }
  board_group[current_group].unread_article = unread_number;
}

/*
 * �ܡ��ɥ��ɥ쥹����
 */

static void	board_get_adrs(ptr1, ptr2)
     char	*ptr1;
     char	*ptr2;
{
  while (*ptr1) {
    if (*ptr1 == BOARD_GROUP_SEPARATER) {
#ifdef	BOARD_DOT_ADRS
      *ptr2++ = '.';
#else	/* !BOARD_DOT_ADRS */
      *ptr2++ = '_';
#endif	/* !BOARD_DOT_ADRS */
      ptr1++;
    } else {
      *ptr2++ = *ptr1++;
    }
  }
#ifdef	BOARD_ADD_LOCAL
  strcpy(ptr2, "@local");
#else	/* BOARD_ADD_LOCAL */
  *ptr2 = '\0';
#endif	/* BOARD_ADD_LOCAL */
}

/*
 * ���롼�ץ�٥����
 */

static int	board_get_level(group_name)
     char	*group_name;
{
  register int	level;

  level = 0;

  if (*group_name) {
    level++;
  }
  while (*group_name) {
    if (*group_name++ == BOARD_GROUP_SEPARATER) {
      level++;
    }
  }
  return(level);
}

/*
 * ���롼�ץ�٥�̾����
 */

static char	*board_get_name(current_group, level)
     int	current_group;
     int	level;
{
  static char	buff[MAX_GROUP_NAME];	/*	static �Ǥ��뤳��	*/
  char		*ptr;

  if (current_group >= board_select_number) {
    buff[0] = '\0';
    return(buff);
  }
  strcpy(buff, board_selected_group[current_group].group_name);
  if (!gnus_mode) {
    ptr = buff;
    while (level >= 0) {
      switch (*ptr) {
      case '\0':
	level = -1;
	break;
      case BOARD_GROUP_SEPARATER:
	if (--level >= 0) {
	  ptr++;
	}
	break;
      default:
	ptr++;
	break;
      }
    }
    *ptr = '\0';
  }
  return(buff);
}

#ifndef	SMALL
/*
 * ���롼��̾����(ʣ��)
 */

static int	board_search_name(mode, max_group, group_ptr)
     int	mode;
     int	max_group;
     int	*group_ptr;
{
  register int	current_group;
  char		*str;

  str = input_search_string(mode);
  if (str != (char*)NULL) {
    if (mode) {
      for (current_group = *group_ptr - 1; current_group >= 0;
	   current_group--) {
	if (strindex(board_selected_group[current_group].group_name, str)) {
	  print_mode_line(japanese ? "���Ĥ��ޤ�����" : "Search succeed.");
	  *group_ptr = current_group;
	  return(0);
	}
      }
    } else {
      for (current_group = *group_ptr + 1; current_group < max_group;
	   current_group++) {
	if (strindex(board_selected_group[current_group].group_name, str)) {
	  print_mode_line(japanese ? "���Ĥ��ޤ�����" : "Search succeed.");
	  *group_ptr = current_group;
	  return(0);
	}
      }
    }
    print_mode_line(japanese ? "���Ĥ���ޤ���Ǥ�����" : "Search failed.");
    return(1);
  }
  return(0);
}
#endif	/* !SMALL */
#endif	/* BOARD */
