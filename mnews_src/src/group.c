/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Group select routine
 *  File        : group.c
 *  Version     : 1.21
 *  First Edit  : 1991-07/20
 *  Last  Edit  : 1997-11/06
 *  Author      : MSR24 ��� ��
 *
 */

#include	"compat.h"
#include	"nntplib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"termlib.h"
#include	"mnews.h"
#include	"group.h"
#include	"article.h"
#ifdef	MH
#include	"mh.h"
#endif	/* MH */
#ifdef	UCBMAIL
#include	"ucbmail.h"
#endif	/* UCBMAIL */
#ifdef	RMAIL
#include	"rmail.h"
#endif	/* RMAIL */
#ifdef	BOARD
#include	"board.h"
#endif	/* BOARD */
#include	"mark.h"
#include	"newspost.h"

static int	news_get_level();	/* ���롼�ץ�٥����		*/
static char	*news_get_name();	/* ���롼�ץ�٥�̾����		*/
static int	news_select_group();	/* ���롼������			*/
static void	news_redraw_group();	/* ���롼��������̺�����	*/
static int	news_prev_unread_sgroup();
					/* ������̤�ɥ��롼���ֹ����	*/
static int	news_next_unread_sgroup();
					/* ������̤�ɥ��롼���ֹ����	*/
static int	news_prev_sgroup();	/* �����򥰥롼���ֹ����	*/
static int	news_next_sgroup();	/* �����򥰥롼���ֹ����	*/
static int	news_prev_level();	/* �����إ��롼��̾����		*/
static int	news_find_group();	/* ���롼�׸���			*/
static void	news_match_group();	/* ���롼�׾ȹ�			*/
static void	news_add_group();	/* GNUS �⡼�ɥ��롼���ɲ�	*/
static int	news_check_new();	/* ���嵭�������å�		*/
static int	news_search_name();	/* ���롼��̾����(ʣ��)		*/
static int	news_transpose_group();	/* ���롼������			*/
static int	news_find_next_order();	/* ���ޡ����ֹ渡��		*/

/*
 * ���򥰥롼�ץꥹ��
 */

char	select_name[MAX_GROUP_NAME];	/* ���򤵤�Ƥ���̾��		*/
char	jump_name[MAX_GROUP_NAME];	/* �����פ���̾��		*/
int	skip_direction = 0;		/* ���롼�׸�������		*/

static int	news_select_number;	/* ���򤵤�Ƥ��륰�롼�׿�	*/
static SELECT_LIST	*news_selected_group = (SELECT_LIST*)NULL;

static char	group_mode_char[] = {	/* ���롼�ץ⡼��	*/
  'Y', 'N', 'M', '-', '+', '+', '+', 'B'
  };
static char	*mark_mode_string[] = {	/* ���ɥ⡼��		*/
  "  ",
  "U ",
  " T",
  "UT",
  };
static int	top_position;		/* ɽ�����ϰ���		*/

/*
 * ���롼������إ�ץ�å�����
 */

static char	*news_group_jmessage[] = {
  "�˥塼�����ƥ���/���롼������⡼��",
  "\n  [����]",
  "\ti �ޤ��� SPACE  ���ƥ���/���롼�פ����򤷤ޤ���",
  "\n  [��ư]",
  "\tk �ޤ��� ^P     ���Υ��ƥ���/���롼�פ˰�ư���ޤ���",
  "\tj �ޤ��� ^N     ���Υ��ƥ���/���롼�פ˰�ư���ޤ���",
  "\tp               ����̤�ɥ��ƥ���/���롼�פ˰�ư���ޤ���",
  "\tn               ����̤�ɥ��ƥ���/���롼�פ˰�ư���ޤ���",
  "\t^U �ޤ��� ^B    �����̤Υ��ƥ���/���롼�פ˰�ư���ޤ���",
  "\t^D, ^F �ޤ��� ^V",
  "\t                �����̤Υ��ƥ���/���롼�פ˰�ư���ޤ���",
  "\t<               ��Ƭ�Υ��ƥ���/���롼�פ˰�ư���ޤ���",
  "\t>               �Ǹ�Υ��ƥ���/���롼�פ˰�ư���ޤ���",
  "\tTAB             ����Υ˥塼�����ƥ���ޤ��ϥ��롼�פ˥����פ��ޤ���",
  "\t                (�˥塼�����ƥ���ޤ��ϥ��롼��̾�����Ϥ��Ʋ�����)",
  "\tq               ���ƥ���/���롼�פ���ȴ���ޤ���",
  "\t                (�Ǿ�̤Υ��ƥ���ʤ齪λ���ޤ�)",
  "\to �ޤ��� RETURN ���ƥ���/���롼�פ���ȴ���ޤ���",
  "\t                (�Ǿ�̤Υ��ƥ��꤫���ȴ���ޤ���)",
  "\n  [�ޡ���]",
  "\tU               ���ƥ���/���롼�פι��ɥޡ��������ؤ�������ư���ޤ���",
  "\tu               ���ƥ���/���롼�פι��ɥޡ��������ؤ�������ư���ޤ���",
  "\tc               ���ƥ���/���롼�פε��������ƴ��ɤˤ��ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
#ifndef	SMALL
  "\tT               ���ƥ���/���롼�פ��ִ��ޡ��������ؤ�������ư���ޤ���",
  "\tt               ���ƥ���/���롼�פ��ִ��ޡ��������ؤ�������ư���ޤ���",
  "\t#               �ִ��ޡ������줿���ƥ���/���롼�פ��ư���ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
#endif	/* !SMALL */
#ifdef	NEWSPOST
  "\n  [���]",
  "\ta               �����˵�����ݥ��Ȥ��ޤ���",
#endif	/* NEWSPOST */
#ifdef	MAILSEND
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
  "\t                (�ȥåץ��ƥ�����Τ�ͭ��)",
  "\001",
#ifndef	SMALL
  "\tg               ���嵭�����ʤ����ƥ����å����ޤ���",
#endif	/* !SMALL */
  "\tB               ̵���ʥ˥塼�����롼�פ������ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
  "\tW               ̤�ɾ���򥻡��֤��ޤ���",
  "\tX               ̤�ɾ���򥻡��֤��ʤ��ǥ����ƥ������λ���ޤ���",
  "\t                (�ȥåץ��ƥ�����Τ�ͭ��)",
#ifdef	notdef
  "\t*               �˥塼�������Ф˥桼��̾�ȥѥ���ɤ����롣",
#endif	/* notdef */
  (char*)NULL,
};

static char	*news_group_message[] = {
#ifndef	SMALL
  "NEWS CATEGORY/GROUP SELECT MODE",
  "\n  [SELECT]",
  "\ti or SPACE   select category/group.",
  "\n  [MOVE]",
  "\tk or ^P      previous category/group.",
  "\tj or ^N      next category/group.",
  "\tp            previous unread category/group.",
  "\tn            next unread category/group.",
  "\t^U or ^B     previous page category/group.",
  "\t^D, ^F or ^V next page category/group.",
  "\t<            top category/group.",
  "\t>            last category/group.",
  "\tTAB          jump to specified news category/group.",
  "\t             (Please input news category/group name.)",
  "\tq            exit category/group.",
  "\t             (When top category then exit.)",
  "\to or RETURN  exit category/group.",
  "\t             (When top category then ignore.)",
  "\n  [MARK]",
  "\tU            toggle subscribe mark and move to previous.",
  "\tu            toggle subscribe mark and move to next.",
  "\tc            mark as read all articles in category/group.",
  "\t             (Please make sure y/n.)",
  "\tT            toggle transpose mark and move to previous.",
  "\tt            toggle transpose mark and move to next.",
  "\t#            move transpose marked category/group.",
  "\t             (Please make sure y/n.)",
#ifdef	NEWSPOST
  "\n  [POST]",
  "\ta            post new article.",
#endif	/* NEWSPOST */
#ifdef	MAILSEND
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
  "\001",
  "\tg            check new news again.",
  "\tB            delete bogus newsgroup.",
  "\t             (Please make sure y/n.)",
  "\tW            save mark-file.",
  "\tX            force quit system without save mark-file.",
  "\t             (When top category only)",
#ifdef	notdef
  "\t*            send user name and password to news-server.",
#endif	/* notdef */
#endif	/* !SMALL */
  (char*)NULL,
};

/*
 * ���롼�������˥塼
 */

int	news_group_menu()
{
  news_match_group(select_name);
  if (news_select_number < 0) {
    print_fatal("No such news group \"%s\".", select_name);
    return(0);
  }
  return(news_select_group());
}

/*
 * ���롼��������̺�����
 */

static void	news_redraw_group(current_group)
     int	current_group;
{
  register int	i, j;
  int		level;
  char		buff[MAX_GROUP_NAME];
  char		jname[MAX_GROUP_NAME];

  i = top_position;
  top_position = get_top_position(top_position, current_group,
				  news_select_number, news_thread_mode,
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
		  "�˥塼�����ƥ���:%-40.40s       ����:%s" :
		  "News category:%-40.40s      Position:%s",
		  jname, buff);
  if (!wide_mode) {
    term_locate(0, 2);
    term_attrib(RESET_ATTRIB);
#ifdef	COLOR
    term_attrib(color_code[HEADER_COLOR]);
#else	/* !COLOR */
    term_attrib(REVERSE_ATTRIB);
#endif	/* !COLOR */
    print_full_line(japanese ?
		    "  ����    ̤��  �⡼��  �˥塼�����ƥ���/�˥塼�����롼��̾" :
		    "  Max    Unread Mode    News category/News group name");
  }
  level = news_get_level(select_name);
  term_attrib(RESET_ATTRIB);
  for (i = 0, j = top_position; i < term_lines - mode_lines; i++, j++) {
    if (j >= news_select_number) {
      break;
    }
    term_locate(0, head_lines + i);
#ifdef	COLOR
    term_attrib(color_code[NUMBER_COLOR]);
#endif	/* COLOR */
    if (news_selected_group[j].max_article >= 0) {
      cprintf("%7d", news_selected_group[j].max_article);
    } else {
      cprintf("       ");
    }
    if (((news_selected_group[j].group_mode <= (short)POST_UNKNOWN) ||
	 (news_selected_group[j].group_mode >= (short)POST_BOARD)) &&
	(news_selected_group[j].unread_article >= 0)) {
#ifdef	MSDOS
      cprintf(" %7ld", (long)news_selected_group[j].unread_article);
#else	/* !MSDOS */
      cprintf(" %7d", (int)news_selected_group[j].unread_article);
#endif	/* !MSDOS */
    } else {
      cprintf("        ");
    }
    strcpy(buff, news_get_name(j, level));
#ifdef	JNAMES
    get_jname(NEWS_JN_DOMAIN, buff, term_columns - 24);
#else	/* !JNAMES */
    buff[term_columns - 24] = '\0';
#endif	/* !JNAMES */
#ifdef	COLOR
    term_attrib(color_code[CATEGORY_COLOR]);
#endif	/* COLOR */
#ifdef	SJIS_SRC
    kanji_printf(SJIS_CODE, "  [%c] %s %s",
		 group_mode_char[(int)news_selected_group[j].group_mode],
		 mark_mode_string[news_selected_group[j].mark_mode], buff);
#else	/* !SJIS_SRC */
    kanji_printf(EUC_CODE, "  [%c] %s %s",
		 group_mode_char[(int)news_selected_group[j].group_mode],
		 mark_mode_string[news_selected_group[j].mark_mode], buff);
#endif	/* !SJIS_SRC */
  }
  term_attrib(RESET_ATTRIB);
  print_mode_line(japanese ?
		  "?:�إ�� j,^N:���� k,^P:���� SPACE,i:���� l:ɽ���⡼������ q:���� Q:��λ" :
		  "?:help j,^N:next k,^P:previous SPACE,i:select l:print mode q:return Q:exit");
}

/*
 * ���롼������
 */

static int	news_select_group()
{
  /* �ѿ��ϺƵ�����Τ� static �ˤ��ʤ����� */

  int		current_group;		/* ������Υ��롼���ֹ�		*/
  char		buff1[MAX_GROUP_NAME];
  char		buff2[MAX_GROUP_NAME];
  char		buff3[MAX_GROUP_NAME];
  int		level;
  int		loop;
  int		status;
  int		key;
  register int	i, j;

  level = news_get_level(select_name);
  top_position = -1;
  if (!jump_name[0]) {
    print_mode_line(japanese ? "������Ǥ���" : "Searching.");
  }

  /*
   * �ǽ��̤�ɥ��롼�פޤǥ����Ȥ�ʤ��
   */

  current_group = 0;
  if (news_select_number > 0) {
    if (!news_selected_group[current_group].unread_article) {
      current_group = news_next_unread_sgroup(current_group);
    }
  }

  loop = 1;
  while (loop) {
    if (jump_name[0]) {
      key = 0;
      current_group = -1;
      for (i = 0; i < news_select_number; i++) {
	strcpy(buff1, news_get_name(i, level));
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
	for (i = 0; i < news_select_number; i++) {
	  strcpy(buff1, news_get_name(i, level));
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
      news_redraw_group(current_group);
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
	  if (line < news_select_number) {
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
    case 0x0e:		/* ^N �����롼�װ�ư	*/
    case 'j':
      current_group = news_next_sgroup(current_group);
      break;
    case 0x10:		/* ^P �����롼�װ�ư	*/
    case 'k':
      current_group = news_prev_sgroup(current_group);
      break;
    case 'q':
      loop = 0;
      break;
    case '\015':	/* RETURN		*/
    case '\n':
    case 'o':
      if (select_name[0]) {
	loop = 0;
      }
      break;
    case 'p':		/* ��̤�ɥ��롼��	*/
      i = current_group;
      current_group = news_prev_unread_sgroup(current_group);
      if ((current_group == i) && news_select_number) {
	i = news_prev_unread_group(news_selected_group[0].group_index);
	if (i >= 0) {
	  switch (yes_or_no(JUMP_YN_MODE,
			    "����̤��(%d)���롼��(%s)�򻲾Ȥ��ޤ���?",
			    "Read previous unread(%d) group(%s)?",
			    group_list[i].unread_article,
			    news_group[i].group_name)) {
	  case 1:
	    skip_direction = -1;
	    strcpy(jump_name, news_group[i].group_name);
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
      }
      break;
    case 'n':		/* ��̤�ɥ��롼��	*/
      i = current_group;
      current_group = news_next_unread_sgroup(current_group);
      if ((current_group == i) && news_select_number &&
	  (news_selected_group[current_group].group_mode <=
	   (short)POST_UNKNOWN)) {
	i = news_next_unread_group(news_selected_group[news_select_number - 1].group_index);
	if (i >= 0) {
	  switch (yes_or_no(JUMP_YN_MODE,
			    "����̤��(%d)���롼��(%s)�򻲾Ȥ��ޤ���?",
			    "Read next unread(%d) group(%s)?",
			    group_list[i].unread_article,
			    news_group[i].group_name)) {
	  case 1:
	    skip_direction = 1;
	    strcpy(jump_name, news_group[i].group_name);
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
	  >= news_select_number) {
	if (news_select_number > 0) {
	  current_group = news_select_number - 1;
	} else {
	  current_group = 0;
	}
      }
      break;
    case '<':		/* �ǽ�Υ��롼��	*/
      current_group = 0;
      break;
    case '>':		/* �Ǹ�Υ��롼��	*/
      if (news_select_number > 0) {
	current_group = news_select_number - 1;
      } else {
	current_group = 0;
      }
      break;
    case '?':		/* �إ��		*/
      help(news_group_jmessage, news_group_message,
	   GLOBAL_MODE_MASK | GROUP_MODE_MASK);
      top_position = -1;
      break;
    case 'Q':		/* ��λ			*/
      return(1);
      /* break; */
    case 'W':		/* �ޡ����ե����륻����	*/
      news_save_initfile(mark_file);
      top_position = -1;
      break;
    case 'X':		/* ������λ		*/
      if (!select_name[0]) {
	if (yes_or_no(CARE_YN_MODE, "������λ���Ƥ�����Ǥ���?",
		      "Force exit system.Are you sure?")) {
	  return(-1);
	}
	top_position = -1;
      }
      break;
    case 'l':
    case 'L':		/* ���ɥ��롼�ץѥå�/���	*/
      group_mask = !group_mask;
      strcpy(buff1, news_get_name(current_group, level));
      news_match_group(select_name);
      current_group = news_find_group(buff1, level);
      top_position = -1;
      break;
    case 'B':		/* ̵���˥塼�����롼�׺��	*/
      news_delete_bogus();
      top_position = -1;
      break;
    case 'G':		/* ����ɽ���⡼��/���	*/
      if (!select_name[0]) {
	gnus_mode = !gnus_mode;
	strcpy(buff1, news_get_name(current_group, level));
	news_match_group(select_name);
	current_group = news_find_group(buff1, level);
	top_position = -1;
      }
      break;
#ifndef	SMALL
    case 'g':		/* ���嵭�������å�	*/
      strcpy(buff1, news_get_name(current_group, level));
      news_check_new();
      news_match_group(select_name);
      current_group = news_find_group(buff1, level);
      top_position = -1;
      break;
#endif	/* !SMALL */
    case '\t':		/* TAB ���ꥫ�ƥ���/���롼�פإ�����	*/
      buff1[0] = '\0';
      input_line(INPUT_SPCCUT_MASK,
		 "�˥塼�����ƥ���ޤ��ϥ˥塼�����롼��̾�����Ϥ��Ʋ�����:",
		 "Input news category or news group name:", buff1);
      if (buff1[0]) {
	strcpy(jump_name, buff1);
	return(1);
      } else {
	top_position = -1;
      }
      break;
#ifdef	NEWSPOST
    case 'a':		/* �˥塼�����		*/
      if (nntp_connect_mode < 2) {
	strcpy(buff1, news_get_name(current_group, level));
	input_line(INPUT_SPCCUT_MASK, "�˥塼�����롼�פ����Ϥ��Ʋ�����:",
		   "Input news group:", buff1);
	if (buff1[0]) {
	  news_post(buff1);
	}
	top_position = -1;
      }
      break;
#endif	/* NEWSPOST */
    default:
      if (news_select_number > 0) {
	switch (key) {
#ifndef	SMALL
	case '/':		/* ��������		*/
	  news_search_name(0, news_select_number, &current_group, level);
	  break;
	case '\\':		/* ��������		*/
	  news_search_name(1, news_select_number, &current_group, level);
	  break;
	case '#':		/* �ִ��¹�		*/
	  if (gnus_mode) {
	    if (yes_or_no(NORMAL_YN_MODE,
			  "�ִ��ޡ������줿���ƥ���/���롼�פ��ư���Ƥ�����Ǥ���?",
			  "Move transpose marked category/group.Are you sure?")) {
	      if (!news_transpose_group(current_group, level)) {
		strcpy(buff1, news_get_name(current_group, level));
		news_match_group(select_name);
		current_group = news_find_group(buff1, level);
	      }
	    }
	    top_position = -1;
	  }
	  break;
#endif	/* !SMALL */
	default:
	  if (news_selected_group[current_group].group_mode
	      > (short)POST_UNKNOWN) {
	    
	    /*	�᡼�륰�롼��	*/
	    
	    switch (key) {
	    case ' ':
	    case 'i':		/* ���롼������		*/
	      strcpy(buff1, select_name);
	      jump_name[0] = '\0';
	      switch (news_selected_group[current_group].group_mode) {
#ifdef	MH
	      case POST_MH:
		status = mh_menu();
		break;
#endif	/* MH */
#ifdef	UCBMAIL
	      case POST_UCBMAIL:
		status = ucbmail_menu();
		break;
#endif	/* UCBMAIL */
#ifdef	RMAIL
	      case POST_RMAIL:
		status = rmail_menu();
		break;
#endif	/* RMAIL */
#ifdef	BOARD
	      case POST_BOARD:
		status = board_menu();
		break;
#endif	/* BOARD */
	      default:
		status = 0;
		break;
	      }
	      if (status) {
		return(status);
	      }
	      strcpy(select_name, buff1);
	      strcpy(buff1, news_get_name(current_group, level));
	      news_match_group(select_name);
	      current_group = news_find_group(buff1, level);
	      top_position = -1;
	      break;
	    default:
	      break;
	    }
	  } else {
	    
	    /* �˥塼�����롼��	*/
	    
	    switch (key) {
	    case ' ':
	    case 'i':		/* ���롼������		*/
	      strcpy(buff1, news_get_name(current_group, level));
	      strcpy(buff2, select_name);
	      strcpy(select_name, buff1);
	      if (news_selected_group[current_group].max_article < 0) {
		status = news_group_menu();
		news_prev_level(select_name, buff3);
		strcpy(select_name, buff3);
		if (strcmp(select_name, buff2)) {
		  print_fatal("Previous group mismatch.\"%s\",\"%s\"",
			      select_name, buff2);
		  strcpy(select_name, buff2);
		}
	      } else {
		status = news_select_article();
		strcpy(select_name, buff2);
	      }
	      if (status) {
		return(1);
	      }
	      news_match_group(select_name);
	      current_group = news_find_group(buff1, level);
	      top_position = -1;
	      break;
#ifndef	SMALL
	    case 't':
	    case 'T':		/* �ִ�			*/
	      news_selected_group[current_group].mark_mode ^= TRANS_MASK;
#ifdef	COLOR
	      term_attrib(color_code[CATEGORY_COLOR]);
#endif	/* COLOR */
	      printf(mark_mode_string[news_selected_group[current_group].mark_mode]);
#ifdef	COLOR
	      term_attrib(RESET_ATTRIB);
#endif	/* COLOR */
	      if (key == 'T') {
		current_group = news_prev_sgroup(current_group);
	      } else {
		current_group = news_next_sgroup(current_group);
	      }
	      break;
#endif	/* !SMALL */
	    case 'u':
	    case 'U':		/* ���ɥޡ�������	*/
	      if (news_selected_group[current_group].max_article < 0) {
		top_position = -1;
		if (!yes_or_no(NORMAL_YN_MODE,
			       "�ܥ��ƥ���ι��ɥ⡼�ɤ����ؤƤ�����Ǥ���?",
			       "Toggle subscribe mode in this news category.Are you sure?")) {
		  status = 0;
		  break;
		}
	      }
	      news_selected_group[current_group].mark_mode ^= UNSUB_MASK;
	      status =
		news_selected_group[current_group].mark_mode & UNSUB_MASK;
	      strcpy(buff1, news_get_name(current_group, level));
	      if (news_selected_group[current_group].max_article < 0) {
		j = strlen(buff1);
		for (i = 0; i < group_number; i++) {
		  if ((!strncmp(news_group[i].group_name, buff1, j)) &&
		      ((!news_group[i].group_name[j]) ||
		       (news_group[i].group_name[j] ==
			NEWS_GROUP_SEPARATER))) {
		    group_list[i].unsubscribe = status;
		  }
		}
	      } else {
		for (i = 0; i < group_number; i++) {
		  if (!strcmp(news_group[i].group_name, buff1)) {
		    group_list[i].unsubscribe = status;
		  }
		}
	      }
#ifdef	COLOR
	      term_attrib(color_code[CATEGORY_COLOR]);
#endif	/* COLOR */
	      printf(mark_mode_string[news_selected_group[current_group].mark_mode]);
#ifdef	COLOR
	      term_attrib(RESET_ATTRIB);
#endif	/* COLOR */
	      if (key == 'U') {
		current_group = news_prev_sgroup(current_group);
	      } else {
		current_group = news_next_sgroup(current_group);
	      }
	      status = 0;
	      break;
	    case 'c':		/* ���롼�����ޡ���	*/
	      if (yes_or_no(NORMAL_YN_MODE,
			    "�ܥ��ƥ���/���롼�פ����ƥޡ������Ƥ�����Ǥ���?",
			    "Mark all articles in this news category/group.Are you sure?")) {
		news_selected_group[current_group].unread_article = 0;
		strcpy(buff1, news_get_name(current_group, level));
		if (news_selected_group[current_group].max_article < 0) {
		  j = strlen(buff1);
		  for (i = 0; i < group_number; i++) {
		    if ((!strncmp(news_group[i].group_name, buff1, j)) &&
			((!news_group[i].group_name[j]) ||
			 (news_group[i].group_name[j] ==
			  NEWS_GROUP_SEPARATER))) {
		      if ((!group_list[i].unsubscribe) || (!group_mask)) {
			news_fill_mark(i);
			group_list[i].unread_article = 0;
		      }
		    }
		  }
		} else {
		  for (i = 0; i < group_number; i++) {
		    if (!strcmp(news_group[i].group_name, buff1)) {
		      news_fill_mark(i);
		      group_list[i].unread_article = 0;
		    }
		  }
		}
	      }
	      current_group = news_next_unread_sgroup(current_group);
	      top_position = -1;
	      break;
	    }
	    status = 0;
	  }
	  break;
	}
      }
      break;
    }
  }
  return(0);
}

/*
 * ��̤�ɥ��롼���ֹ����
 */

int	news_prev_unread_group(current_group)
     int	current_group;
{
  register int	i;

  if (gnus_mode) {
    for (i = 0; i < news_select_number; i++) {
      if (news_selected_group[i].group_index == current_group) {
	for (--i; i >= 0; i--) {
	  if ((news_selected_group[i].max_article >= 0) &&
	      (group_list[news_selected_group[i].group_index].unread_article
	       > 0)) {
	    return(news_selected_group[i].group_index);
	  }
	}
	break;
      }
    }
    return(-1);
  } else {
    while (1) {
      if (--current_group < 0) {
	current_group = -1;
	break;
      }
      if (news_group[current_group].group_mode > (short)POST_UNKNOWN) {
	current_group = -1;
	break;
      }
      if ((group_list[current_group].unread_article > 0) &&
	  ((!group_list[current_group].unsubscribe) || (!group_mask))) {
	break;
      }
    }
  }
  return(current_group);
}

/*
 * ��̤�ɥ��롼���ֹ����
 */

int	news_next_unread_group(current_group)
     int	current_group;
{
  register int	i;

  if (gnus_mode) {
    for (i = 0; i < news_select_number; i++) {
      if (news_selected_group[i].group_index == current_group) {
	for (++i; i < news_select_number; i++) {
	  if ((news_selected_group[i].max_article >= 0) &&
	      (group_list[news_selected_group[i].group_index].unread_article
	       > 0)) {
	    return(news_selected_group[i].group_index);
	  }
	}
	break;
      }
    }
    return(-1);
  } else {
    while (1) {
      if (++current_group >= group_number) {
	current_group = -1;
	break;
      }
      if (news_group[current_group].group_mode > (short)POST_UNKNOWN) {
	current_group = -1;
	break;
      }
      if ((group_list[current_group].unread_article > 0) &&
	  ((!group_list[current_group].unsubscribe) || (!group_mask))) {
	break;
      }
    }
  }
  return(current_group);
}

/*
 * ������̤�ɥ��롼���ֹ����
 */

static int	news_prev_unread_sgroup(current_group)
     int	current_group;
{
  register int	i;

  i = current_group;
  while (i > 0) {
    if (news_selected_group[--i].unread_article) {
      current_group = i;
      break;
    }
  }
  return(current_group);
}

/*
 * ������̤�ɥ��롼���ֹ����
 */

static int	news_next_unread_sgroup(current_group)
     int	current_group;
{
  register int	i;

  i = current_group;
  while (i < (news_select_number - 1)) {
    if (news_selected_group[++i].unread_article) {
      current_group = i;
      break;
    }
  }
  return(current_group);
}

/*
 * �����򥰥롼���ֹ����
 */

static int	news_prev_sgroup(current_group)
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

static int	news_next_sgroup(current_group)
     int	current_group;
{
  if (++current_group >= news_select_number) {
    if (news_select_number > 0) {
      current_group = news_select_number - 1;
    } else {
      current_group = 0;
    }
  }
  return(current_group);
}

/*
 * ���롼�׸���
 */

static int	news_find_group(group_name, level)
     char	*group_name;
     int	level;
{
  char		buff[BUFF_SIZE];
  register int	i, j;
  register int	status;

  for (i = 0; i < news_select_number; i++) {
    strcpy(buff, news_get_name(i, level));
    if (!strcmp(buff, group_name)) {
      return(i);
    }
  }
  status = 0;
  if (gnus_mode) {
    for (i = 0; i < group_number; i++) {
      if (!strcmp(news_group[i].group_name, group_name)) {
	i = group_list[i].mark_order;
	for (j = 0; j < news_select_number; j++) {
	  status = j;
	  if (group_list[news_selected_group[j].group_index].mark_order >= i) {
	    break;
	  }
	}
	break;
      }
    }
  } else {
    for (i = 0; i < news_select_number; i++) {
      status = i;
      strcpy(buff, news_get_name(i, level));
      if (strncmp(buff, group_name, strlen(group_name)) >= 0) {
	break;
      }
    }
  }
  return(status);
}

/*
 * �ޥå����륰�롼�׸���
 */

static void	news_match_group(group_name)
     char	*group_name;
{
  int		length1, length2;
  register int	i, j, k, l;
  long		unread;
  int		max, mode, unsubscribe;
  int		match;
  char		*ptr;

  news_select_number = 0;
  if (news_selected_group == (SELECT_LIST*)NULL) {
    news_selected_group = (SELECT_LIST*)
      large_malloc((long)(group_number + MAX_MAIL_GROUP)
		   * (long)sizeof(SELECT_LIST));
    if (news_selected_group == (SELECT_LIST*)NULL) {
      print_fatal("Can't allocate memory for select struct.");
      return;
    }
  }

  /*
   * ����(GNUS like)�⡼�ɤν���
   */

  if (gnus_mode) {
#ifdef	GNUS_NOSORT
    for (i = 0; i < group_number; i++) {
      news_add_group(i);
    }
#else	/* !GNUS_NOSORT */
    j = group_number;
    for (i = 0; j > 0; i++) {
      k = 0;
      if (i) {
	l = 0;
	for (; k < group_number; k++) {
	  if (group_list[k].mark_order == i) {
	    news_add_group(k++);
	    j--;
	    l = 1;
	    break;
	  }
	}
	if (!l) {
	  continue;
	}
      }
      for (; k < group_number; k++) {
	if (group_list[k].mark_order < 0) {
	  news_add_group(k);
	  j--;
	} else {
	  break;
	}
      }
    }
#endif	/* !GNUS_NOSORT */
  } else {

    /*
     * ����(vin like)�⡼�ɤν���
     * �������鲼�ν����� mnews �ǰ������򤬺������ʬ�Ǥ���
     * �����ν�������ñ������Ǥ���ͤ���º�ɤ��ޤ���
     */

    j = l = 0;
    length1 = strlen(group_name);
    for (i = 0; i < group_number; i++) {
      if ((!strncmp(news_group[i].group_name, group_name, length1)) &&
	  ((!group_name[0]) ||
	   (!news_group[i].group_name[length1]) ||
	   (news_group[i].group_name[length1] == NEWS_GROUP_SEPARATER))) {
	match = 1;
	ptr = &news_group[i].group_name[length1];
	if (*ptr) {
	  k = 0;
	  if (*ptr == NEWS_GROUP_SEPARATER) {
	    ptr++;
	    k++;
	  }
	  while (*ptr) {
	    if (*ptr == NEWS_GROUP_SEPARATER) {
	      break;
	    }
	    ptr++;
	    k++;
	  }
	  k += length1;
	  for (; j < i; j++) {
	    if (!strncmp(news_group[i].group_name, news_group[j].group_name,
			 k)) {
	      match = 0;
	      break;
	    }
	  }
	} else {
	  k = length1;
	}
	group_list[i].match_flag = match;
	if (match) {
	  unread = 0L;
	  max = -1;
	  mode = (int)POST_UNKNOWN;
	  unsubscribe = group_list[i].unsubscribe;
	  length2 = k;
	  k = 0;
	  for (; l < group_number; l++) {
	    if ((!strncmp(news_group[i].group_name, news_group[l].group_name,
			  length2)) &&
		((!news_group[l].group_name[length2]) ||
		 (news_group[l].group_name[length2] ==
		  NEWS_GROUP_SEPARATER))) {
	      unsubscribe &= group_list[l].unsubscribe;
	      if ((!group_list[l].unsubscribe) || (!group_mask)) {
		unread += (long)group_list[l].unread_article;
	      }
	      if (length2 == strlen(news_group[l].group_name)) {
		if (l + 1 >= group_number) {
		  k++;
		} else if ((strncmp(news_group[l].group_name,
				    news_group[l + 1].group_name,
				    strlen(news_group[l].group_name))) ||
			   (news_group[l + 1].group_name[strlen(news_group[l].group_name)]
			    != NEWS_GROUP_SEPARATER)) {
		  k++;
		} else if (strlen(group_name)
			   == strlen(news_group[l].group_name)) {
		  k++;
		}
		if (k == 1) {
		  max = news_group[l].max_article;
		  if (post_enable) {
		    mode = news_group[l].group_mode;
		  } else {
		    mode = (int)POST_DISABLE;
		  }
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
	    fprintf(stderr, "MATCH %04d:%s\n", i, news_group[i].group_name);
#endif	/* DEBUG */
	    news_selected_group[news_select_number].group_index = i;
	    news_selected_group[news_select_number].unread_article = unread;
	    news_selected_group[news_select_number].max_article = max;
	    news_selected_group[news_select_number].group_mode = mode;
	    news_selected_group[news_select_number].mark_mode = unsubscribe;
	    news_select_number++;
	  }
	}
      }
    }
  }
  if (*group_name) {
    return;
  }

  /*
   * �᡼�륰�롼�פ��ɲ�
   * ��©�ʽ����Ǥ��ͤ���
   */

#ifdef	BOARD
  if (board_init(&unread)) {
    news_selected_group[news_select_number].group_index = 0;
    news_selected_group[news_select_number].unread_article = unread;
    news_selected_group[news_select_number].max_article = -1;
    news_selected_group[news_select_number].group_mode = (short)POST_BOARD;
    news_selected_group[news_select_number].mark_mode = 0;
    news_select_number++;
  }
#endif	/* BOARD */
#ifdef	MH
  if (mh_init()) {
    news_selected_group[news_select_number].group_index = 0;
    news_selected_group[news_select_number].unread_article = -1L;
    news_selected_group[news_select_number].max_article = -1;
    news_selected_group[news_select_number].group_mode = (short)POST_MH;
    news_selected_group[news_select_number].mark_mode = 0;
    news_select_number++;
  }
#endif	/* MH */
#ifdef	UCBMAIL
  if (ucbmail_init()) {
    news_selected_group[news_select_number].group_index = 0;
    news_selected_group[news_select_number].unread_article = -1L;
    news_selected_group[news_select_number].max_article = -1;
    news_selected_group[news_select_number].group_mode = (short)POST_UCBMAIL;
    news_selected_group[news_select_number].mark_mode = 0;
    news_select_number++;
  }
#endif	/* UCBMAIL */
#ifdef	RMAIL
  if (rmail_init()) {
    news_selected_group[news_select_number].group_index = 0;
    news_selected_group[news_select_number].unread_article = -1L;
    news_selected_group[news_select_number].max_article = -1;
    news_selected_group[news_select_number].group_mode = (short)POST_RMAIL;
    news_selected_group[news_select_number].mark_mode = 0;
    news_select_number++;
  }
#endif	/* RMAIL */
}

/*
 * GNUS �⡼�ɥ��롼���ɲ�
 */

static void	news_add_group(current_group)
     int	current_group;
{
  int		mode;

  if ((!group_mask) || ((group_list[current_group].unread_article > 0) &&
			(!group_list[current_group].unsubscribe))) {
    if (post_enable) {
      mode = news_group[current_group].group_mode;
    } else {
      mode = (int)POST_DISABLE;
    }
    news_selected_group[news_select_number].group_index = current_group;
    news_selected_group[news_select_number].unread_article =
      (long)group_list[current_group].unread_article;
    news_selected_group[news_select_number].max_article =
      news_group[current_group].max_article;
    news_selected_group[news_select_number].group_mode = mode;
    news_selected_group[news_select_number].mark_mode =
      group_list[current_group].unsubscribe;
    news_select_number++;
  }
}

#ifndef	SMALL
/*
 * ���嵭�������å�
 */

static int	news_check_new()
{
  struct news_group	*old_news_ptr;
  GROUP_LIST		*old_group_ptr;
  char			*old_name_ptr;
  int			old_group_number;
  long			old_name_used_size;
  int			status;
  register int		i, j;

  status = 1;
  if (group_number <= 0) {
    return(status);
  }

  /*
   * ���顼���������Ƹ��ξ��֤���¸���Ƥ���¹�
   */

  old_news_ptr = (struct news_group*)
    large_malloc((long)sizeof(struct news_group) * (long)group_number);
  if (old_news_ptr == (struct news_group*)NULL) {
    print_fatal("Can't allocate memory for news struct.");
    return(status);
  }
  old_name_ptr = (char*)large_malloc(name_used_size);
  if (old_name_ptr == (char*)NULL) {
    print_fatal("Can't allocate memory for name pool.");
    large_free(old_news_ptr);
    return(status);
  }
  old_group_ptr = group_list;
  print_mode_line(japanese ? "�����å���Ǥ���" : "Checking.");
  memcpy(old_news_ptr, news_group, sizeof(struct news_group) * group_number);
  memcpy(old_name_ptr, name_pool, name_used_size);
#if	defined(MSDOS) && !defined(X68K) && !defined(__GO32__)
  for (i = 0; i < group_number; i++) {
    old_news_ptr[i].group_name = old_name_ptr +
      ((unsigned)old_news_ptr[i].group_name - (unsigned)name_pool);
  }
#else	/* (!MSDOS || X68K || __GO32__) */
  j = (int)(old_name_ptr - name_pool);
  for (i = 0; i < group_number; i++) {
    old_news_ptr[i].group_name += j;
  }
#endif	/* (!MSDOS || X68K || __GO32__) */
  old_group_number = group_number;
  old_name_used_size = name_used_size;
  nntp_free();
  group_number = 0;
  status = nntp_list();
  if (status != NNTP_OK) {
    print_mode_line(japanese ? "����³��Ǥ���" : "Reconnecting.");
    if (open_nntp_server(0) == NNTP_OK) {
      status = nntp_list();
    }
  }
  if (status == NNTP_OK) {
    group_list = (GROUP_LIST*)
      large_malloc((long)group_number * (long)sizeof(GROUP_LIST));
    if (group_list == (GROUP_LIST*)NULL) {
      status = 1;
    } else {
      if (news_selected_group != (SELECT_LIST*)NULL) {
	large_free(news_selected_group);
      }
      news_selected_group = (SELECT_LIST*)NULL;
      for (i = 0; i < group_number; i++) {
	group_list[i].unsubscribe = 0;
	group_list[i].mark_ptr    = (MARK_LIST*)NULL;
	group_list[i].mark_order  = -1;
	for (j = 0; j < old_group_number; j++) {
	  if (!strcmp(news_group[i].group_name,
		      old_news_ptr[j].group_name)) {
	    group_list[i].unsubscribe = old_group_ptr[j].unsubscribe;
	    group_list[i].mark_ptr    = old_group_ptr[j].mark_ptr;
	    group_list[i].mark_order  = old_group_ptr[j].mark_order;
	    break;
	  }
	}
	news_count_unread(i);
      }
    }
  }
  if (status) {		/*	���Ԥ����鸵�ξ��֤��᤹	*/
    group_number = old_group_number;
    name_used_size = old_name_used_size;
    news_group = old_news_ptr;
    group_list = old_group_ptr;
    name_pool = old_name_ptr;
  } else {		/*	���������鸵�ξ��֤�ΤƤ�	*/
    large_free(old_news_ptr);
    large_free(old_group_ptr);
    large_free(old_name_ptr);
  }
  return(status);
}
#endif	/* !SMALL */

/*
 * ���롼�ץ�٥����
 */

static int	news_get_level(group_name)
     char	*group_name;
{
  register int	level;

  level = 0;

  if (*group_name) {
    level++;
  }
  while (*group_name) {
    if (*group_name++ == NEWS_GROUP_SEPARATER) {
      level++;
    }
  }
  return(level);
}

/*
 * ���롼�ץ�٥�̾����
 */

static char	*news_get_name(current_group, level)
     int	current_group;
     int	level;
{
  static char	buff[MAX_GROUP_NAME];	/*	static �Ǥ��뤳��	*/
  char		*ptr;

  if (current_group >= news_select_number) {
    buff[0] = '\0';
    return(buff);
  }
  switch (news_selected_group[current_group].group_mode) {
  case POST_MH:
    strcpy(buff, "MH");
    break;
  case POST_UCBMAIL:
    strcpy(buff, "MAIL");
    break;
  case POST_RMAIL:
    strcpy(buff, "RMAIL");
    break;
  case POST_BOARD:
    strcpy(buff, "BOARD");
    break;
  default:

    /*
     * �˥塼�����롼��̾�ΰ������ڤ��롣
     * ̵������©�Ǥ��ͤ����Ǥ⤳������ȥ�������Ǥ����Ǥ���
     */

    strcpy(buff,
	   news_group[news_selected_group[current_group].group_index].group_name);
    if (!gnus_mode) {
      ptr = buff;
      while (level >= 0) {
	switch (*ptr) {
	case '\0':
	  level = -1;
	  break;
	case NEWS_GROUP_SEPARATER:
	  if (ptr == buff) {
	    ptr++;
	  } else if (--level >= 0) {
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
    break;
  }
  return(buff);
}

/*
 * �����إ��롼��̾����
 */

static int	news_prev_level(group_name1, group_name2)
     char	*group_name1;
     char	*group_name2;
{
  char	*ptr;

  strcpy(group_name2, group_name1);
  ptr = strrchr(group_name2, NEWS_GROUP_SEPARATER);
  if (ptr) {
    *ptr = '\0';
  } else {
    *group_name2 = '\0';
  }
  return(0);
}

#ifdef	notdef
/*
 * GNUS �⡼���� NG ����
 */

int	gnus_prev_group(current_group)
     int	current_group;
{
  register int	i;

  for (i = 0; i < news_select_number; i++) {
    if (news_selected_group[i].group_index == current_group) {
      if (--i > 0) {
	if (news_selected_group[i].max_article >= 0) {
	  return(news_selected_group[i].group_index);
	}
      }
      break;
    }
  }
  return(-1);
}

/*
 * GNUS �⡼�ɼ� NG ����
 */

int	gnus_next_group(current_group)
     int	current_group;
{
  register int	i;

  for (i = 0; i < news_select_number; i++) {
    if (news_selected_group[i].group_index == current_group) {
      if (++i < news_select_number) {
	if (news_selected_group[i].max_article >= 0) {
	  return(news_selected_group[i].group_index);
	}
      }
      break;
    }
  }
  return(-1);
}
#endif	/* notdef */

#ifndef	SMALL
/*
 * ���롼��̾����(ʣ��)
 */

static int	news_search_name(mode, max_number, group_ptr, level)
     int	mode;
     int	max_number;
     int	*group_ptr;
     int	level;
{
  register int	current_group;
  char		*str;
  char		buff[MAX_GROUP_NAME];

  str = input_search_string(mode);
  if (str != (char*)NULL) {
    if (mode) {
      for (current_group = *group_ptr - 1; current_group >= 0;
	   current_group--) {
	strcpy(buff, news_get_name(current_group, level));
	if (strindex(buff, str)) {
	  print_mode_line(japanese ? "���Ĥ��ޤ�����" : "Search succeed.");
	  *group_ptr = current_group;
	  return(0);
	}
      }
    } else {
      for (current_group = *group_ptr + 1; current_group < max_number;
	   current_group++) {
	strcpy(buff, news_get_name(current_group, level));
	if (strindex(buff, str)) {
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

/*
 * ���롼������
 */

static int	news_transpose_group(current_group, level)
     int	current_group;
     int	level;
{
  BOGUS_LIST		*bogus_ptr;
  short			*order_ptr;
  char			buff[MAX_GROUP_NAME];
  register int		i, j, k;
  int			max_order;
  int			current_order;

  /*
   * �Ť�����������¸
   */

  order_ptr = (short*)malloc(group_number * sizeof(short));
  if (order_ptr == (short*)NULL) {
    return(1);
  }
  for (i = 0; i < group_number; i++) {
    order_ptr[i] = group_list[i].mark_order;
  }

  /*
   * ��ư���륰�롼�פ�ԥå����å�(���Ū�� -2 ������)
   */

  for (k = 0; k < news_select_number; k++) {
    if (news_selected_group[k].mark_mode & TRANS_MASK) {
      strcpy(buff, news_get_name(k, level));
      j = strlen(buff);
      for (i = 0; i < group_number; i++) {
	if ((!strncmp(news_group[i].group_name, buff, j)) &&
	    ((!news_group[i].group_name[j]) ||
	     (news_group[i].group_name[j] == NEWS_GROUP_SEPARATER))) {
	  if (group_list[i].mark_order != -2) {
	    group_list[i].mark_order = -2;
	  }
	}
      }
    }
  }

  /*
   * ��ư�襰�롼�פ����
   */

  current_order = max_order = news_find_next_order(order_ptr, 0, 0);
  while (1) {
    i = news_find_next_order(order_ptr, max_order, 0);
    if (i < 0) {
      max_order++;
      break;
    }
    max_order = i;
  }
  if (news_selected_group[current_group].group_mode > (short)POST_UNKNOWN) {
    current_order = max_order;
  } else {
    while (current_group >= 0) {
      strcpy(buff, news_get_name(current_group, level));
      j = strlen(buff);
      for (i = 0; i < group_number; i++) {
	if ((!strncmp(news_group[i].group_name, buff, j)) &&
	    ((!news_group[i].group_name[j]) ||
	     (news_group[i].group_name[j] == NEWS_GROUP_SEPARATER))) {
	  if (group_list[i].mark_order >= 0) {
	    current_order = group_list[i].mark_order;
	    current_group = -1;
	  } else {
	    current_group--;
	  }
	  break;
	}
      }
    }
  }
#ifdef	DEBUG
  print_fatal("CURRENT:%d", current_order);
#endif	/* DEBUG */

  /*
   * ��ư�������Υ��롼�פ�ͤ��
   */

  k = news_find_next_order(order_ptr, 0, 1);
  while (1) {
    for (j = k; j < current_order; j++) {
      for (i = 0; i < group_number; i++) {
	if (group_list[i].mark_order == j) {
	  group_list[i].mark_order = k;
	  j = -1;
	  break;
	}
      }
      if (j < 0) {
	break;
      }
    }
    if (j >= 0) {
      break;
    }
    k = news_find_next_order(order_ptr, k, 1);
  }
#ifdef	DEBUG
  for (i = 0; i < group_number; i++) {
    print_fatal("PACK1:%s:%d", news_group[i].group_name,
		group_list[i].mark_order);
  }
#endif	/* DEBUG */

  /*
   * ��ư�������롼�פ�ԥå����å�(���Ū�� -3 ������)
   */

  current_order = k;
  for (i = 0; i < group_number; i++) {
    if (group_list[i].mark_order >= current_order) {
      group_list[i].mark_order = -3;
    }
  }
#ifdef	DEBUG
  for (i = 0; i < group_number; i++) {
    print_fatal("NEXT1:%s:%d", news_group[i].group_name,
		group_list[i].mark_order);
  }
#endif	/* DEBUG */

  /*
   * ��ư���륰�롼�פ�ͤ��
   */

  while (1) {
    j = -1;
    k = max_order + 1;
    for (i = 0; i < group_number; i++) {
      if ((group_list[i].mark_order == -2) && (order_ptr[i] < k)) {
	j = i;
	k = order_ptr[i];
      }
    }
    if (j < 0) {
      break;
    }
    group_list[j].mark_order = current_order;
    if (order_ptr[j] == -1) {
      for (i = 0; i < group_number; i++) {
	if (order_ptr[i] >= current_order) {
#ifdef	DEBUG
	  print_fatal("FIX GROUP:%s:%d+1", news_group[i].group_name,
		      order_ptr[i]);
#endif	/* DEBUG */
	  order_ptr[i]++;
	}
      }
      bogus_ptr = bogus_list;
      while (bogus_ptr) {
	if (bogus_ptr->mark_order >= current_order) {
#ifdef	DEBUG
	  print_fatal("FIX BOGUS:%s:%d+1", bogus_ptr->group_name,
		      bogus_ptr->mark_order);
#endif	/* DEBUG */
	  bogus_ptr->mark_order++;
	}
	bogus_ptr = bogus_ptr->next_ptr;
      }
    }
    current_order++;
#ifdef	DEBUG
    for (i = 0; i < group_number; i++) {
      print_fatal("PACK2:%s:%d", news_group[i].group_name,
		  group_list[i].mark_order);
    }
#endif	/* DEBUG */
  }

  /*
   * ��ư������Υ��롼�פ�ͤ��
   */

  while (1) {
    j = -1;
    k = max_order + 1;
    for (i = 0; i < group_number; i++) {
      if ((group_list[i].mark_order == -3) && (order_ptr[i] < k)) {
	j = i;
	k = order_ptr[i];
      }
    }
    if (j < 0) {
      break;
    }
    group_list[j].mark_order = current_order;
    current_order = news_find_next_order(order_ptr, current_order, 1);
  }
#ifdef	DEBUG
  for (i = 0; i < group_number; i++) {
    print_fatal("NEXT2:%s:%d", news_group[i].group_name,
		group_list[i].mark_order);
  }
#endif	/* DEBUG */
  free(order_ptr);
  return(0);
}

/*
 * ���Υޡ����ֹ�򸡺����롣
 */

static int	news_find_next_order(order_ptr, number, mode)
     short	*order_ptr;
     int	number;
     int	mode;	/*
			 * 0:�Ǹ�ʤ� -1 ���֤�
			 * 1:�Ǹ�ʤ� number + 1 ���֤�
			 */
{
  register int	i, j;

  j = -1;
  for (i = 0; i < group_number; i++) {
    if ((order_ptr[i] > number) &&
	((j == -1) || (j > order_ptr[i]))) {
      j = order_ptr[i];
    }
  }
  if ((j < 0) && mode) {
    return(number + 1);
  }
  return(j);
}
#endif	/* !SMALL */
