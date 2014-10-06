/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : RMAIL support routine
 *  File        : rmail.c
 *  Version     : 1.21
 *  First Edit  : 1992-07/23
 *  Last  Edit  : 1997-11/11
 *  Author      : MSR24 ��� ��
 *
 */

#ifdef	RMAIL
#include	"compat.h"
#include	"nntplib.h"
#include	"smtplib.h"
#include	"pop3lib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"termlib.h"
#include	"mnews.h"
#include	"group.h"
#include	"rmail.h"
#include	"article.h"
#include	"pager.h"
#include	"mailsend.h"

static int	rmail_select_folder(); 	/* �ե��������			*/
static void	rmail_redraw_folder();	/* �ե����������̺�����	*/
static int	rmail_select_article(); /* ��������			*/
static void	rmail_redraw_article();	/* ����������̺�����		*/
static int	rmail_prev_folder();	/* ���ե�����ֹ����		*/
static int	rmail_next_folder();	/* ���ե�����ֹ����		*/
static int	rmail_prev_unread_folder();
					/* ��̤�ɥե�����ֹ����	*/
static int	rmail_next_unread_folder();
					/* ��̤�ɥե�����ֹ����	*/
static int	rmail_prev_article();	/* �������ֹ����		*/
static int	rmail_next_article();	/* �������ֹ����		*/
static int	rmail_prev_unread_article();
					/* �������ֹ����		*/
static int	rmail_next_unread_article();
					/* �������ֹ����		*/
#ifdef	LARGE
static int	rmail_prev_mark_article();/* ���ޡ��������ֹ����	*/
static int	rmail_next_mark_article();/* ���ޡ��������ֹ����	*/
#endif	/* LARGE */
static void	rmail_search_folder();	/* ���ե��������		*/
static int	rmail_read();		/* ��������			*/
static int	rmail_extract();	/* �������			*/
static void	rmail_mark();		/* �����ޡ����ɲ�		*/
static void	rmail_unmark();		/* �����ޡ������		*/
static void	rmail_count();		/* �������������		*/
static int	rmail_get_list();	/* �����ꥹ�ȼ���		*/
static void	rmail_include();	/* ���嵭��������		*/
static void	rmail_update();		/* RMAIL �ե����빹��		*/
static int	rmail_create_folder();	/* �ե��������			*/
static int	rmail_delete_folder();	/* �ե�������			*/
static int	rmail_adjust_folder();	/* �ե������������		*/
static int	rmail_search_name();	/* �ե����̾����		*/

/*
 * RMAIL �ե�����ꥹ��
 */

static int	rmail_folder_number;	/* RMAIL �ե������		*/
static int	rmail_alloc_number;	/* RMAIL �ե����������		*/
static char	rmail_file[PATH_BUFF];	/* RMAIL �ե�����ե�����̾	*/
static struct rmail_folder	*rmail_folder = (struct rmail_folder*)NULL;
static int	top_position;		/* ɽ�����ϰ���		*/
static int	rmail_article_number;	/* RMAIL ������		*/
static int	rmail_modify;		/* RMAIL �����ե饰	*/

static char	*rmail_ignore_field[] = {
  "Via:",
  "Mail-From:",
  "Origin:",
  "Status:",
  "Received:",
  "Message-ID:",
  "Summary-Line:"
  };

/*
 * RMAIL �إ�ץ�å�����
 */

static char	*rmail_folder_jmessage[] = {
  "RMAIL �ե��������⡼��",
  "\n  [����]",
  "\ti �ޤ��� SPACE  �ե���������򤷤ޤ���",
  "\n  [��ư]",
  "\tk �ޤ��� ^P     ���Υե�����˰�ư���ޤ���",
  "\tj �ޤ��� ^N     ���Υե�����˰�ư���ޤ���",
  "\tp               ����̤�ɥե�����˰�ư���ޤ���",
  "\tn               ����̤�ɥե�����˰�ư���ޤ���",
  "\t^U �ޤ��� ^B    �����̤Υե�����˰�ư���ޤ���",
  "\t^D, ^F �ޤ��� ^V",
  "\t                �����̤Υե�����˰�ư���ޤ���",
  "\t<               ��Ƭ�Υե�����˰�ư���ޤ���",
  "\t>               �Ǹ�Υե�����˰�ư���ޤ���",
  "\tTAB             ����Υե�����˥����פ��ޤ���",
  "\t                (�ե�������ƥ���ޤ��ϥե����̾�����Ϥ��Ʋ�����)",
#ifdef	LARGE
  "\tI               �����ѥե�����˰�ư���ޤ���",
#endif	/* LARGE */
  "\to, q �ޤ��� RETURN",
  "\t                �ե��������⡼�ɤ���ȴ���ޤ���",
#ifdef	MAILSEND
  "\n  [����]",
  "\tm               �����˥᡼���Ф��ޤ���",
#endif	/* MAILSEND */
#ifndef	SMALL
  "\n  [����]",
  "\t/               �ե����̾�������������ޤ���",
  "\t                (����������ʸ��������Ϥ��Ʋ�����)",
  "\t\\               �ե����̾������������ޤ���",
  "\t                (����������ʸ��������Ϥ��Ʋ�����)",
#endif	/* !SMALL */
  "",
  "\tC               �����˥ե������������ޤ���",
  "\t                (�ե����̾�����Ϥ��Ʋ�����)",
  "\tDEL �ޤ��� BS   �ե�����������ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
  (char*)NULL,
  };

static char	*rmail_article_jmessage[] = {
  "RMAIL ��������⡼��",
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
#ifdef	LARGE
  "\t{               ���κ���ޡ��������˰�ư���ޤ���",
  "\t}               ���κ���ޡ��������˰�ư���ޤ���",
#endif	/* LARGE */
  "\to, q �ޤ��� RETURN",
  "\t                ��������⡼�ɤ���ȴ���ޤ���",
  "\n  [�ޡ���]",
  "\tD               ����ޡ��������ε����ذ�ư���ޤ���",
  "\td               ����ޡ��������ε����ذ�ư���ޤ���",
  "\tU               ����ޡ�����������ε����ذ�ư���ޤ���",
  "\tu               ����ޡ�����������ε����ذ�ư���ޤ���",
  "\tY               ���ɥޡ��������ε����ذ�ư���ޤ���",
  "\ty               ���ɥޡ��������ε����ذ�ư���ޤ���",
  "\tZ               ���ɥޡ�����������ε����ذ�ư���ޤ���",
  "\tz               ���ɥޡ�����������ε����ذ�ư���ޤ���",
  "\tM               �ޥ���ޡ��������/��������ε����ذ�ư���ޤ���",
  "\tC               ��¸��������ޡ������������ε����ذ�ư���ޤ���",
  "\tc               ��������ޡ������ޤ���",
  "\t                (y:���ɥޡ��� n:��� d:����ޡ��� �����򤷤Ʋ�����)",
#ifdef	MAILSEND
  "\n  [����]",
  "\tm               �����˥᡼���Ф��ޤ���",
  "\tr               �������ֿ����ޤ���",
  "\tR               ��������Ѥ����ֿ����ޤ���",
  "\t= �ޤ��� f      ������ž�����ޤ���",
#ifdef	LARGE
  "\t~               ������������ޤ���",
#endif	/* LARGE */
#endif	/* MAILSEND */
  "\n  [������]",
  "\ts               �����򥻡��֤��ޤ���",
  "\t                (�ե�����̾�����Ϥ��Ʋ��������ե����뤬¸�ߤ������",
  "\t                 y:���ڥ�� n:��� o:��񤭤�����ǲ�����)",
  "\n  [������]",
  "\tI               �������᡼�������ߤޤ���",
#ifndef	SMALL
  "\tL               UCB-mail �����ε����ե����������ߤޤ���",
  "\t                (�ե�����̾�����Ϥ��Ʋ�����)",
#endif	/* !SMALL */
  "\n  [���/����]",
  "\tDEL �ޤ��� BS   ����ޡ������줿�����������ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
  "\t^               ����ޡ������줿�������̤Υե�����˰�ư���ޤ���",
  "\t                (��ư��ե����̾����Ƥ��ޤ�)",
  "\n  [ɽ��]",
  "\tl               ����/������줿������ɽ���⡼�ɤ����ؤ��ޤ���",
  "\tt               ����åɥ⡼�ɤ����ؤ��ޤ���",
#ifdef	REF_SORT
  "\tT               ��������ˡ�����ؤ��ޤ���",
#endif	/* REF_SORT */
  (char*)NULL,
};

static char	*rmail_folder_message[] = {
#ifndef	SMALL
  "RMAIL FOLDER SELECT MODE",
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

static char	*rmail_article_message[] = {
#ifndef	SMALL
  "RMAIL ARTICLE SELECT MODE",
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
#ifdef	LARGE
  "\t{            previous delete mark article.",
  "\t}            next delete mark article.",
#endif	/* LARGE */
  "\to, q or return",
  "\t             exit from article select mode.",
  "\n  [MARK]",
  "\tD            mark for delete and move to previous.",
  "\td            mark for delete and move to next.",
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
 * RMAIL �⡼�ɽ����
 */

int	rmail_init()
{
  return(rmail_mode);
}

/*
 * RMAIL ��˥塼
 */

int	rmail_menu()
{
  struct stat	stat_buff;
  int		status;

  select_name[0] = '\0';
  print_mode_line(japanese ? "������Ǥ���" : "Searching.");
  if ((!stat(rmail_mbox, &stat_buff)) && (stat_buff.st_mode & S_IFDIR)) {
    rmail_search_folder();
    while (1) {
      if (auto_inc_mode && auto_inc_folder[0] && (check_new_mail() > 0)) {
	strcpy(jump_name, auto_inc_folder);
      }
      status = rmail_select_folder();
      if (!jump_name[0]) {
	break;
      }
    }
    return(status);
  } else {
    rmail_folder_number = 0;
    strcpy(rmail_file, rmail_mbox);
    return(rmail_select_article(auto_inc_mode));
  }
}

/*
 * �ե����������̺�����
 */

static void	rmail_redraw_folder(current_folder)
     int	current_folder;
{
  register int	i, j;
  char		buff[SMALL_BUFF];
  char		jname[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_folder,
				  rmail_folder_number, mail_thread_mode, buff);
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
  get_jname(RMAIL_JN_DOMAIN, jname, 40);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "�ե����:%-48.48s       ����:%s" :
		  "Folder:%-48.48s     Position:%s",
		  jname, buff);
  if (!wide_mode) {
    j = (check_new_mail() > 0);
    term_locate(0, 2);
    term_attrib(RESET_ATTRIB);
    term_attrib(color_code[HEADER_COLOR]);
    print_full_line(japanese ?
		    "  ����    ̤��    �ե����̾                       [%-8.8s]" :
		    "  Max     Unread  Folder name                      [%-8.8s]",
		    new_mail_string[j]);
  }
  term_attrib(RESET_ATTRIB);
  for (i = 0, j = top_position; i < term_lines - mode_lines; i++, j++) {
    if (j >= rmail_folder_number) {
      break;
    }
    term_locate(0, head_lines + i);
#ifdef	COLOR
    term_attrib(color_code[NUMBER_COLOR]);
#endif	/* COLOR */
    cprintf(" %6d  %6d", rmail_folder[j].max_article,
	    rmail_folder[j].unread_article);
    strcpy(buff, rmail_folder[j].folder_name);
#ifdef	JNAMES
    get_jname(RMAIL_JN_DOMAIN, buff, term_columns - 24);
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
		  "?:�إ�� j,n,^N:���� k,p,^P:���� SPACE,i:���� o,q,RETURN:���� Q:��λ" :
		  "?:help j,n,^N:next k,p,^P:previous SPACE,i:select o,q,RETURN:return Q:exit");
}

/*
 * �ե��������
 */

static int	rmail_select_folder()
{
  int		current_folder;		/* ������Υե�����ֹ�		*/
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
      for (i = 0; i < rmail_folder_number; i++) {
	if (!strnamecmp(rmail_folder[i].folder_name, jump_name)) {
	  current_folder = i;
	  key = ' ';
	  break;
	}
	if (!strnnamecmp(rmail_folder[i].folder_name, jump_name,
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
      rmail_redraw_folder(current_folder);
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
	  if (line < rmail_folder_number) {
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
    case 0x0c:		/* ^L ������		*/
      term_init(2);
      top_position = -1;
      break;
    case 0x10:		/* ^P ���ե������ư	*/
    case 'k':
      current_folder = rmail_prev_folder(current_folder);
      break;
    case 0x0e:		/* ^N ���ե������ư	*/
    case 'j':
      current_folder = rmail_next_folder(current_folder);
      break;
    case 'p':		/* p ��̤�ɥե������ư	*/
      current_folder = rmail_prev_unread_folder(current_folder);
      break;
    case 'n':		/* n ��̤�ɥե������ư	*/
      current_folder = rmail_next_unread_folder(current_folder);
      break;
    case 'o':
    case 'q':
    case '\015':	/* RETURN		*/
    case '\n':
      loop = 0;
      break;
    case 0x02:		/* ^B			*/
    case 0x15:		/* ^U ���ڡ���		*/
      if ((current_folder -= (term_lines - mode_lines)) < 0) {
	current_folder = 0;
      }
      break;
    case 0x04:		/* ^D			*/
    case 0x06:		/* ^F			*/
    case 0x16:		/* ^V ���ڡ���		*/
      if ((current_folder += (term_lines - mode_lines))
	  >= rmail_folder_number) {
	if (rmail_folder_number > 0) {
	  current_folder = rmail_folder_number - 1;
	} else {
	  current_folder = 0;
	}
      }
      break;
    case '<':		/* �ǽ�Υե����	*/
      current_folder = 0;
      break;
    case '>':		/* �Ǹ�Υե����	*/
      if (rmail_folder_number > 0) {
	current_folder = rmail_folder_number - 1;
      } else {
	current_folder = 0;
      }
      break;
    case '\t':		/* TAB ����ե�����إ�����	*/
      strcpy(buff1, rmail_mbox);
      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
		 "�ե����̾�����Ϥ��Ʋ�����:", "Input folder name:", buff1);
      if (buff1[0]) {
	strcpy(jump_name, buff1);
      }
      top_position = -1;
      break;
    case '?':		/* �إ��		*/
      help(rmail_folder_jmessage, rmail_folder_message,
	   GLOBAL_MODE_MASK | GROUP_MODE_MASK);
      top_position = -1;
      break;
    case 'Q':		/* ��λ			*/
      return(1);
      /* break; */
    case 'C':		/* �ե��������		*/
      strcpy(buff1, rmail_mbox);
      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
		 "������������ե����̾�����Ϥ��Ʋ�����:",
		 "Input new folder name:", buff1);
      if (buff1[0]) {
	if (!rmail_create_folder(buff1)) {
	  if (rmail_folder_number > 0) {
	    strcpy(buff1, rmail_folder[current_folder].folder_name);
	    rmail_search_folder();
	    current_folder = rmail_adjust_folder(buff1);
	  } else {
	    rmail_search_folder();
	    current_folder = 0;
	  }
	}
      }
      top_position = -1;
      break;
    default:
      if (rmail_folder_number > 0) {
	switch (key) {
#ifdef	LARGE
	case 'I':		/* �����ѥե������ư	*/
	  if (auto_inc_folder[0]) {
	    strcpy(jump_name, auto_inc_folder);
	    return(1);
	  }
	  /* break ���� */
#endif	/* LARGE */
	case ' ':
	case 'i':		/* �ե��������		*/
	  strcpy(buff1, select_name);
	  strcpy(select_name, rmail_folder[current_folder].folder_name);
	  sprintf(rmail_file, "%s%c%s", rmail_mbox, SLASH_CHAR,
		  rmail_folder[current_folder].folder_name);
	  status = rmail_select_article(auto_inc_mode &&
					((!auto_inc_folder[0]) ||
					 (!strnamecmp(auto_inc_folder,
				rmail_folder[current_folder].folder_name))));
	  strcpy(select_name, buff1);
	  strcpy(buff1, rmail_folder[current_folder].folder_name);
	  current_folder = 0;
	  for (i = 0; i < rmail_folder_number; i++) {
	    if (!strnnamecmp(rmail_folder[i].folder_name, buff1,
			     strlen(buff1))) {
	      current_folder = i;
	      break;
	    }
	  }
	  rmail_count(current_folder);
	  if (status) {
	    return(1);
	  }
	  top_position = -1;
	  break;
#ifndef	SMALL
	case '/':		/* ��������		*/
	  rmail_search_name(0, rmail_folder_number, &current_folder);
	  break;
	case '\\':		/* ��������		*/
	  rmail_search_name(1, rmail_folder_number, &current_folder);
	  break;
#endif	/* !SMALL */
	case 0x7f:		/* �ե�������		*/
	case 0x08:
	  if (rmail_folder[current_folder].max_article) {
	    print_mode_line(japanese ?
			    "�ե����(%s)�϶��ǤϤ���ޤ���" :
			    "Folder(%s) does not empty.",
			    rmail_folder[current_folder].folder_name);
	    term_bell();
	    sleep(ERROR_SLEEP);
	  } else {
	    if (yes_or_no(CARE_YN_MODE, "�ե����(%s)�������ޤ���?",
			  "Delete folder(%s)?",
			  rmail_folder[current_folder].folder_name)) {
	      if (!rmail_delete_folder(rmail_folder[current_folder].folder_name)) {
		strcpy(buff1, rmail_folder[current_folder].folder_name);
		rmail_search_folder();
		current_folder = rmail_adjust_folder(buff1);
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
 * ����������̺�����
 */

static void	rmail_redraw_article(current_folder, current_article)
     int	current_folder;
     int	current_article;
{
  register int	i;
  char		buff[SMALL_BUFF];
  char		jname[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_article,
				  rmail_article_number, mail_thread_mode,
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
  if ((int)strlen(rmail_file) > (int)strlen(rmail_mbox)) {
    strcpy(jname, &rmail_file[strlen(rmail_mbox) + 1]);
  } else {
    strcpy(jname, rmail_file);
  }
#ifdef	JNAMES
  get_jname(RMAIL_JN_DOMAIN, jname, 42);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "�᡼��ե�����:%-42.42s       ����:%s" :
		  "Mail file:%-42.42s        Position:%s",
		  jname, buff);
  print_articles(top_position, current_folder, rmail_article_number,
		 (int (*)())NULL, "PANIC");
  print_mode_line(japanese ?
		  "?:�إ�� j,^N:���� k,^P:���� SPACE,i,.:���� o,q:���� Q:��λ" :
		  "?:help j,^N:next k,^P:previous SPACE,i,.:read o,q:return Q:exit");
}

/*
 * ��������
 */

static int	rmail_select_article(auto_flag)
     int	auto_flag;
{
  register int	current_folder;		/* ������Υե�����ֹ�		*/
  int		current_article;	/* ������ε����ֹ�		*/
  char		buff[BUFF_SIZE];
  char		refile_folder[BUFF_SIZE];
  int		loop;
  int		status;
  int		key;
  register int	i, j;

  current_folder = -1;
  jump_name[0] = '\0';
  for (i = 0; i < rmail_folder_number; i++) {
    if (!strnamecmp(select_name, rmail_folder[i].folder_name)) {
      current_folder = i;
      break;
    }
  }
  if ((current_folder < 0) && rmail_folder_number) {
    print_fatal("Unexpected folder selected.");
    return(0);
  }
  print_mode_line(japanese ? "������Ǥ���" : "Searching.");
  if (auto_flag) {	/* ��ư�����ǽ		*/
    if (check_new_mail() > 0) {
      rmail_include((char*)NULL, 1);
    }
  }
  if (rmail_get_list() < 0) {
    return(0);
  }

  /*
   * �ǽ��̤�ɵ����ޤǥ����Ȥ�ʤ��
   */

  current_article = 0;
  if (rmail_article_number > 0) {
    while (1) {
      if (current_article >= rmail_article_number - 1) {
	break;
      } else if (article_list[current_article].mark & READ_MARK) {
	current_article = rmail_next_unread_article(current_article);
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
    rmail_redraw_article(current_folder, current_article);
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
	if (line < rmail_article_number) {
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
      current_article = rmail_prev_article(current_article);
      break;
    case 0x0e:		/* ^N ��������ư	*/
    case 'j':
      current_article = rmail_next_article(current_article);
      break;
#ifdef	LARGE
    case '{':		/* { ���ޡ���������ư	*/
      current_article = rmail_prev_mark_article(current_article);
      break;
    case '}':		/* } ���ޡ���������ư	*/
      current_article = rmail_next_mark_article(current_article);
      break;
#endif	/* LARGE */
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
	  >= rmail_article_number) {
	if (rmail_article_number > 0) {
	  current_article = rmail_article_number - 1;
	} else {
	  current_article = 0;
	}
      }
      break;
    case '<':		/* �ǽ�ε���		*/
      current_article = 0;
      break;
    case '>':		/* �Ǹ�ε���		*/
      if (rmail_article_number > 0) {
	current_article = rmail_article_number - 1;
      } else {
	current_article = 0;
      }
      break;
#ifndef	SMALL
    case 'L':		/* �����ե����������	*/
      buff[0] = '\0';
      input_line(INPUT_EXPAND_MASK | INPUT_COMP_MASK,
		 "�ե�����̾�����Ϥ��Ʋ�����:",
		 "Input file name:", buff);
      if (!buff[0]) {
	top_position = -1;
	break;
      }
      /* break ����	*/
#endif	/* !SMALL */
    case 'I':		/* ���嵭��������	*/
      rmail_update(0, (char*)NULL);
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
	rmail_include((char*)NULL, 1);
      } else {
	rmail_include(buff, 0);
      }
      i = rmail_article_number;
      rmail_get_list();
      current_article = i;
      if (current_article >= rmail_article_number) {
	if (rmail_article_number > 0) {
	  current_article = rmail_article_number - 1;
	} else {
	  current_article = 0;
	}
      }
      top_position = -1;
      break;
    case '?':		/* �إ��		*/
      help(rmail_article_jmessage, rmail_article_message,
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
      rmail_update(0, (char*)NULL);
      if (!change_sort_rule(-1, &rmail_article_number,
			    &current_article,
			    &mail_article_mask, &mail_thread_mode,
			    rmail_get_list, key)) {
	top_position = -1;
      }
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
	  if (rmail_article_number > 0) {
	    if (key == 'p') {
	      current_article = rmail_prev_unread_article(current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(!current_article);
	    } else {
	      current_article = rmail_prev_article(current_article);
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    if (rmail_folder_number) {
	      i = current_folder;
	      while (1) {
		if (--i < 0) {
		  i = current_folder;
		  break;
		}
		if (rmail_folder[i].unread_article > 0) {
		  break;
		}
	      }
	      if (i != current_folder) {
		switch (yes_or_no(JUMP_YN_MODE,
				  "���Υե����(%s)�򻲾Ȥ��ޤ���?",
				  "Read previous folder(%s)?",
				  rmail_folder[i].folder_name)) {
		case 1:
		  rmail_update(0, (char*)NULL);
		  strcpy(jump_name, rmail_folder[i].folder_name);
		  return(1);
		  /* break; */
		case 2:
		  rmail_update(0, (char*)NULL);
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
	case 'n':		/* ����̤�ɵ�������	*/
	case 'N':		/* ���ε�������		*/
	  j = current_article;
	  if (rmail_article_number > 0) {
	    if (key == 'n') {
	      current_article = rmail_next_unread_article(current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(current_article == (rmail_article_number - 1));
	    } else {
	      current_article = rmail_next_article(current_article);
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    i = current_folder;
	    while (1) {
	      if (++i >= rmail_folder_number) {
		i = current_folder;
		break;
	      }
	      if (rmail_folder[i].unread_article > 0) {
		break;
	      }
	    }
	    if (i != current_folder) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"����̤�ɥե����(%s)�򻲾Ȥ��ޤ���?",
				"Read next unread folder(%s)?",
				rmail_folder[i].folder_name)) {
	      case 1:
		rmail_update(0, (char*)NULL);
		strcpy(jump_name, rmail_folder[i].folder_name);
		return(1);
		/* break; */
	      case 2:
		rmail_update(0, (char*)NULL);
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
	if (rmail_article_number > 0) {
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
	    if (!rmail_read(article_list[current_article].real_number, i)) {
	      switch (last_key) {
	      case 'D':
		rmail_mark(current_article, READ_MARK | DELETE_MARK);
		last_key = 'p';
		break;
	      case 'd':
		rmail_mark(current_article, READ_MARK | DELETE_MARK);
		last_key = 'n';
		break;
	      case 'Y':
		rmail_mark(current_article, READ_MARK);
		last_key = 'p';
		break;
	      case 'y':
		rmail_mark(current_article, READ_MARK);
		last_key = 'n';
		break;
	      case 'U':
		rmail_mark(current_article, READ_MARK);
		rmail_unmark(current_article, DELETE_MARK);
		last_key = 'p';
		break;
	      case 'u':
		rmail_mark(current_article, READ_MARK);
		rmail_unmark(current_article, DELETE_MARK);
		last_key = 'n';
		break;
	      case 'Z':
		rmail_unmark(current_article, READ_MARK);
		last_key = 'p';
		break;
	      case 'z':
		rmail_unmark(current_article, READ_MARK);
		last_key = 'n';
		break;
	      default:
		rmail_mark(current_article, READ_MARK);
		break;
	      }
	    }
	    if (pager_mode) {
	      switch (key) {
	      case 'p':
		current_article = rmail_prev_unread_article(current_article);
		break;
	      case 'n':
	      case ' ':
		current_article = rmail_next_unread_article(current_article);
		break;
	      case 'P':
		current_article = rmail_prev_article(current_article);
		break;
	      case 'N':
		current_article = rmail_next_article(current_article);
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
	  case 'D':		/* ����ޡ���,��������ư	*/
	  case 'd':		/* ����ޡ���,��������ư	*/
	    rmail_mark(current_article, DELETE_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'D') {
	      current_article = rmail_prev_article(current_article);
	    } else {
	      current_article = rmail_next_article(current_article);
	    }
	    break;
	  case 'U':		/* ����ޡ������,��������ư	*/
	  case 'u':		/* ����ޡ������,��������ư	*/
	    rmail_unmark(current_article, DELETE_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'U') {
	      current_article = rmail_prev_article(current_article);
	    } else {
	      current_article = rmail_next_article(current_article);
	    }
	    break;
	  case 'Y':		/* ���ɥޡ���,��������ư	*/
	  case 'y':		/* ���ɥޡ���,��������ư	*/
	    rmail_mark(current_article, READ_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'Y') {
	      current_article = rmail_prev_article(current_article);
	    } else {
	      current_article = rmail_next_article(current_article);
	    }
	    break;
	  case 'Z':		/* ���ɥޡ������,��������ư	*/
	  case 'z':		/* ���ɥޡ������,��������ư	*/
	    rmail_unmark(current_article, READ_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'Z') {
	      current_article = rmail_prev_article(current_article);
	    } else {
	      current_article = rmail_next_article(current_article);
	    }
	    break;
	  case 'c':		/* �������ޡ���		*/
	    switch (yes_or_no(MARK_YN_MODE,
			      "��������ޡ������Ƥ�����Ǥ���?",
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
	      for (i = 0; i < rmail_article_number; i++) {
		rmail_mark(i, j);
	      }
	    }
	    top_position = -1;
	    break;
	  case '^':		/* �ޡ���������ե�����	*/
	    j = 0;
	    for (i = 0; i < rmail_article_number; i++) {
	      if (article_list[i].mark & DELETE_MARK) {
		j++;
	      }
	    }
	    if (j) {
	      strcpy(buff, rmail_mbox);
	      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
			 "��ư��ե����̾�����Ϥ��Ʋ�����:",
			 "Input destination folder name:", buff);
	      if (buff[0]) {
		j = 0;
		for (i = 0; i < rmail_folder_number; i++) {
		  if (!strnamecmp(rmail_folder[i].folder_name, buff)) {
		    status = i;
		    j = 1;
		    break;
		  }
		}
		if (!j)  {
		  if (yes_or_no(NORMAL_YN_MODE,
				"�ե����(%s)�򿷵��������ޤ���?",
				"Create folder(%s).Are you sure?",
				buff)) {
		    if (!rmail_create_folder(buff)) {
		      j = 1;
		    }
		  }
		}
		if (j) {
		  sprintf(refile_folder, "%s%c%s", rmail_mbox, SLASH_CHAR,
			  buff);
		  if (!strnamecmp(refile_folder, rmail_file)) {
		    print_mode_line(japanese ?
				    "��ư���Ȱ�ư�褬Ʊ���ե�����Ǥ���" :
				    "Source and destination folders are identical.");
		    term_bell();
		    sleep(ERROR_SLEEP);
		  } else {
		    rmail_update(0, (char*)NULL);
		    rmail_update(1, refile_folder);
		    rmail_search_folder();
		    rmail_get_list();
		    if (current_article >= rmail_article_number) {
		      if (rmail_article_number > 0) {
			current_article = rmail_article_number - 1;
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
	  case 0x7f:		/* �ޡ����������	*/
	  case 0x08:
	    j = 0;
	    for (i = 0; i < rmail_article_number; i++) {
	      if (article_list[i].mark & DELETE_MARK) {
		j++;
	      }
	    }
	    if (j) {
	      if (yes_or_no(CARE_YN_MODE,
			    "�ޡ������줿���������ƺ�����Ƥ�����Ǥ���?",
			    "Delete all marked articles.Are you sure?")) {
		rmail_update(1, (char*)NULL);
		rmail_get_list();
		if (current_article >= rmail_article_number) {
		  if (rmail_article_number > 0) {
		    current_article = rmail_article_number - 1;
		  } else {
		    current_article = 0;
		  }
		}
	      }
	      status = 0;
	    }
	    top_position = -1;
	    break;
	  case 's':		/* ����������		*/
	    if (!multi_save(current_article, rmail_extract)) {
	      multi_add_mark(rmail_article_number, -1, current_article,
			     READ_MARK, rmail_mark);
	    }
	    top_position = -1;
	    break;
#ifndef	SMALL
	  case '/':		/* ��������		*/
	    search_subjects(0, rmail_article_number, &current_article,
			    (int (*)())NULL);
	    break;
	  case '\\':		/* ��������		*/
	    search_subjects(1, rmail_article_number, &current_article,
			    (int (*)())NULL);
	    break;
	  case '|':		/* �����ѥ��׼¹�	*/
	    if (!multi_pipe(current_article, rmail_extract)) {
	      multi_add_mark(rmail_article_number, -1, current_article,
			     READ_MARK, rmail_mark);
	    }
	    top_position = -1;
	    break;
#endif	/* !SMALL */
	  case '\t':		/* TAB ���국���إ�����	*/
	    buff[0] = '\0';
	    input_line(INPUT_SPCCUT_MASK, "�����ֹ�����Ϥ��Ʋ�����:",
		       "Input article number:", buff);
	    j = atoi(buff);
	    if ((j > 0) && buff[0]) {
	      for (i = 0; i < rmail_article_number; i++) {
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
	    create_temp_name(buff, "RR");
	    if (!rmail_extract(article_list[current_article].real_number,
		buff)) {
	      if (!mail_reply(buff, 0, select_name)) {
		rmail_mark(current_article, ANSWER_MARK);
	      }
	      funlink2(buff);
	    }
	    top_position = -1;
	    break;
	  case 'R':		/* �᡼���ֿ�		*/
	    create_temp_name(buff, "RR");
	    if (!multi_extract(current_article, buff, rmail_extract)) {
	      if (!mail_reply(buff, REPLY_QUOTE_MASK, select_name)) {
		multi_add_mark(rmail_article_number, -1, current_article,
			       ANSWER_MARK, rmail_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -1;
	    break;
	  case '=':		/* �᡼��ž��		*/
	  case 'f':
	    create_temp_name(buff, "RT");
	    if (!multi_extract(current_article, buff, rmail_extract)) {
	      if (!mail_forward(buff, select_name)) {
		multi_add_mark(rmail_article_number, -1, current_article,
			       FORWARD_MARK, rmail_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -1;
	    break;
#ifdef	LARGE
	  case '~':		/* �᡼�����		*/
	    create_temp_name(buff, "RL");
	    if (!multi_extract(current_article, buff, rmail_extract)) {
	      if (!mail_relay(buff, select_name)) {
		multi_add_mark(rmail_article_number, -1, current_article,
			       FORWARD_MARK, rmail_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -1;
	    break;
#endif	/* LARGE */
#endif	/* MAILSEND */
	  case 'C':		/* ���ޡ������		*/
	    rmail_unmark(current_article, READ_MARK | FORWARD_MARK
			 | ANSWER_MARK | DELETE_MARK);
	    if (article_list[current_article].mark & MULTI_MARK) {
	      multi_mark(top_position, current_article);
	    }
	    toggle_mark(top_position, current_article, 0);
	    current_article = rmail_next_article(current_article);
	    break;
	  case 'M':		/* �ޥ���ޡ�������/���	*/
	    multi_mark(top_position, current_article);
	    current_article = rmail_next_article(current_article);
	    break;
#ifdef	LARGE
	  case '@':		/* ����			*/
	    if (!multi_print(current_article, rmail_extract)) {
	      multi_add_mark(rmail_article_number, -1, current_article,
			     READ_MARK, rmail_mark);
	    }
	    top_position = -1;
	    break;
	  case '(':		/* ������������		*/
	    search_articles(0, rmail_article_number, &current_article, 
			    rmail_extract);
	    break;
	  case ')':		/* ������������		*/
	    search_articles(1, rmail_article_number, &current_article, 
			    rmail_extract);
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
  rmail_update(0, (char*)NULL);
  return(status);
}

/*
 * ���ե�����ֹ����
 */

static int	rmail_prev_folder(current_folder)
     int	current_folder;
{
  if (--current_folder < 0) {
    current_folder = 0;
  }
  return(current_folder);
}

/*
 * ���ե�����ֹ����
 */

static int	rmail_next_folder(current_folder)
     int	current_folder;
{
  if (++current_folder >= rmail_folder_number) {
    if (rmail_folder_number > 0) {
      current_folder = rmail_folder_number - 1;
    } else {
      current_folder = 0;
    }
  }
  return(current_folder);
}

/*
 * ��̤�ɥե�����ֹ����
 */

static int	rmail_prev_unread_folder(current_folder)
     int	current_folder;
{
  register int	i;

  i = current_folder;
  if (rmail_folder_number > 0) {
    while (--i >= 0) {
      if (rmail_folder[i].unread_article) {
	current_folder = i;
	break;
      }
    }
  }
  return(current_folder);
}

/*
 * ��̤�ɥե�����ֹ����
 */

static int	rmail_next_unread_folder(current_folder)
     int	current_folder;
{
  register int	i;

  i = current_folder;
  if (rmail_folder_number > 0) {
    while (++i < rmail_folder_number) {
      if (rmail_folder[i].unread_article) {
	current_folder = i;
	break;
      }
    }
  }
  return(current_folder);
}

/*
 * �������ֹ����
 */

static int	rmail_prev_article(current_article)
     int	current_article;
{
  if (--current_article < 0) {
    current_article = 0;
  }
  return(current_article);
}

/*
 * ���ե������������
 */

static int	rmail_next_article(current_article)
     int	current_article;
{
  if (++current_article >= rmail_article_number) {
    if (rmail_article_number > 0) {
      current_article = rmail_article_number - 1;
    } else {
      current_article = 0;
    }
  }
  return(current_article);
}

/*
 * ��̤�ɵ����ֹ����
 */

static int	rmail_prev_unread_article(current_article)
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

static int	rmail_next_unread_article(current_article)
     int	current_article;
{
  term_enable_abort();
  while (current_article < (rmail_article_number - 1)) {
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
 * ���ޡ��������ֹ����
 */

static int	rmail_prev_mark_article(current_article)
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
 * ���ޡ��������ֹ����
 */

static int	rmail_next_mark_article(current_article)
     int	current_article;
{
  while (current_article < (rmail_article_number - 1)) {
    if (article_list[++current_article].mark & DELETE_MARK) {
      break;
    }
  }
  return(current_article);
}
#endif	/* LARGE */

/*
 * ���ե��������
 */

static void	rmail_search_folder()
{
  struct rmail_folder	*rmail_alloc_ptr;
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

  rmail_folder_number = rmail_alloc_number = 0;
  if (rmail_folder != (struct rmail_folder*)NULL) {
    free(rmail_folder);
    rmail_folder = (struct rmail_folder*)NULL;
  }
#ifdef	HAVE_FILES
  sprintf(buff1, "%s%c*.*", rmail_mbox, SLASH_CHAR);
  if (!dos_files(buff1, &dp, FILE_ATTR)) {
    do {
#ifdef	__TURBOC__
      strcpy(buff1, dp.ff_name);
#else	/* !__TURBOC__ */
      strcpy(buff1, dp.name);
#endif	/* !__TURBOC__ */
      sprintf(buff2, "%s%c%s", rmail_mbox, SLASH_CHAR, buff1);
      if (buff1[strlen(buff1) - 1] != '~') {
        {
#else	/* !HAVE_FILES */
  if ((dir_ptr = opendir(rmail_mbox)) !=(DIR*)NULL) {
    while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
      strcpy(buff1, dp->d_name);
      sprintf(buff2, "%s%c%s", rmail_mbox, SLASH_CHAR, buff1);
      if ((!stat(buff2, &stat_buff))
	  && (*(buff1 + strlen(dp->d_name) - 1) != '~')) {
	if ((stat_buff.st_mode & S_IFMT) == S_IFREG) {
#endif	/* !HAVE_FILES */
	  fp = fopen(buff2, "r");
	  if (fp != (FILE*)NULL) {
	    status = 0;
	    if (fgets(buff3, sizeof(buff3), fp)) {
	      if ((!strcmp(buff3, RMAIL_EMACS_FIELD2)) ||
		  (!strcmp(buff3, RMAIL_MULE_FIELD2))) {
		status = 1;
	      }
	    }
	    if (status) {
	      if (rmail_alloc_number <= rmail_folder_number) {
		if (rmail_folder) {
		  rmail_alloc_ptr = (struct rmail_folder*)
		    realloc(rmail_folder,
			    (rmail_alloc_number + RMAIL_ALLOC_COUNT)
			    * sizeof(struct rmail_folder));
		} else {
		  rmail_alloc_ptr = (struct rmail_folder*)
		    malloc((rmail_alloc_number + RMAIL_ALLOC_COUNT)
			   * sizeof(struct rmail_folder));
		}
		if (rmail_alloc_ptr == (struct rmail_folder*)NULL) {
		  print_fatal("Can't allocate memory for RMAIL folder.");
		  fclose(fp);
		  break;
		} else {
		  rmail_folder = rmail_alloc_ptr;
		  rmail_alloc_number += RMAIL_ALLOC_COUNT;
		}
	      }
	      strcpy(rmail_folder[rmail_folder_number].folder_name,
		     buff1);
	      rmail_folder_number++;
	      rmail_count(rmail_folder_number - 1);
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
  if (rmail_folder_number > 0) {
    qsort(rmail_folder, rmail_folder_number, sizeof(struct rmail_folder),
	  (int (*)())strcmp);
  }
}

/*
 * �������������
 */

static void	rmail_count(folder_number)
     int	folder_number;
{
  FILE		*fp;
  char		buff[BUFF_SIZE];
  int		max_number;
  int		unread_number;

  if ((folder_number < 0) || (folder_number >= rmail_folder_number)) {
    print_fatal("Illegal RMAIL folder number.");
    return;
  }
  max_number = unread_number = 0;
  sprintf(buff, "%s%c%s", rmail_mbox, SLASH_CHAR,
	  rmail_folder[folder_number].folder_name);
  fp = fopen(buff, "r");
  if (fp != (FILE*)NULL) {
    while (1) {
      while (fgets(buff, sizeof(buff), fp)) {
	if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
	  break;
	}
      }
      if (fgets(buff, sizeof(buff), fp)) {
	max_number++;
	if (strindex(buff, RMAIL_UNSEEN_ATTRIB)) {
	  unread_number++;
	}
      } else {
	break;
      }
    }
    fclose(fp);
  }
  rmail_folder[folder_number].max_article = max_number;
  rmail_folder[folder_number].unread_article = unread_number;
}

/*
 * ��������
 */

static int	rmail_read(real_number, mode)
     int	real_number;
     int	mode;
{
  char	tmp_file1[PATH_BUFF];
  char	tmp_file2[PATH_BUFF];
  char	buff[SMALL_BUFF];
  int	status;

  create_temp_name(tmp_file1, "RC");
  create_temp_name(tmp_file2, "RV");
  if (rmail_extract(real_number, tmp_file1)) {
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
 * �������
 */

static int	rmail_extract(real_number, tmp_file)
     int	real_number;
     char	*tmp_file;
{
  FILE	*fp1, *fp2;
  char	buff[BUFF_SIZE];
  int	status;

  fp1 = fopen(rmail_file, "r");
  if (fp1 == (FILE*)NULL) {
    return(1);
  }
  while (1) {
    if (!fgets(buff, sizeof(buff), fp1)) {
      fclose(fp1);
      return(1);
    }
    if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
      if (--real_number <= 0) {
	break;
      }
    }
  }
  if (!fgets(buff, sizeof(buff), fp1)) {/*	Attribute �Ԥ�ΤƤ�	*/
    fclose(fp1);
    return(1);
  }
  fp2 = fopen2(tmp_file, "a");
  if (fp2 == (FILE*)NULL) {
    fclose(fp1);
    return(1);
  }
  status = 1;
  while (1) {
    if (!fgets(buff, sizeof(buff), fp1)) {
      fclose(fp1);
      return(1);
    }
    if (!strncmp(buff, RMAIL_EOOH, sizeof(RMAIL_EOOH) - 1)) {
      break;
    }
    fputs(buff, fp2);
    status = 0;
  }
  while (fgets(buff, sizeof(buff), fp1)) {
    if ((buff[0] == '\n') || (buff[0] == '\0') || status) {
      if (status) {
	fputs(buff, fp2);
      }
      while (fgets(buff, sizeof(buff), fp1)) {
	if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
	  break;
	}
	fputs(buff, fp2);
      }
      break;
    }
  }
  fclose(fp1);
  fclose(fp2);
  return(0);
}

/*
 * �����ޡ����ɲ�
 */

static void	rmail_mark(current_article, mark)
     int	current_article;
     int	mark;
{
  if (!(article_list[current_article].mark & mark)) {
    article_list[current_article].mark |= mark;
    rmail_modify = 1;
  }
}

/*
 * �����ޡ������
 */

static void	rmail_unmark(current_article, mark)
     int	current_article;
     int	mark;
{
  if (article_list[current_article].mark & mark) {
    article_list[current_article].mark &= ~mark;
    rmail_modify = 1;
  }
}

/*
 * �����ꥹ�ȼ���
 */

static int	rmail_get_list()
{
  FILE			*fp;
  char			buff[BUFF_SIZE];
  static char		from_buff[BUFF_SIZE];
  static char		to_buff[BUFF_SIZE];
  static char		date_buff[MAX_FIELD_LEN];
  static char		subject_buff[BUFF_SIZE];
  static char		x_nsubj_buff[MAX_FIELD_LEN];
#ifdef	REF_SORT
  static char		message_buff[MAX_FIELD_LEN];
  static char		reference_buff[BUFF_SIZE];
  static char		in_reply_buff[BUFF_SIZE];
#endif	/* REF_SORT */
  static struct cpy_hdr	rmail_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
#ifdef	LARGE
    {TO_FIELD,		to_buff,	sizeof(to_buff)},
#endif	/* LARGE */
    {DATE_FIELD,	date_buff,	sizeof(date_buff)},
    {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
    {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
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
  int			mark;
  register int		lines;

  rmail_article_number = rmail_modify = multi_number = 0;
  if (article_list != (ARTICLE_LIST*)NULL) {
    free(article_list);
  }
  article_list = (ARTICLE_LIST*)NULL;
  alloc_number = 0;
  fp = fopen(rmail_file, "r");
  if (fp == (FILE*)NULL) {
    return(1);
  }
  while (fgets(buff, sizeof(buff), fp)) {
    if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
      break;
    }
  }
  while (fgets(buff, sizeof(buff), fp)) {
    mark = 0;
    if (!strindex(buff, RMAIL_UNSEEN_ATTRIB)) {
      mark |= READ_MARK;
    }
    if (strindex(buff, RMAIL_FORWARD_ATTRIB)) {
      mark |= FORWARD_MARK;
    }
    if (strindex(buff, RMAIL_ANSWER_ATTRIB)) {
      mark |= ANSWER_MARK;
    }
    if (strindex(buff, RMAIL_DELETE_ATTRIB)) {
      mark |= DELETE_MARK;
    }
    second = atoi(buff);
    if (second) {
      lines = copy_fields(fp, rmail_fields,
			  sizeof(rmail_fields) / sizeof(struct cpy_hdr),
			  CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
    }
    while (fgets(buff, sizeof(buff), fp)) {
      if (!strncmp(buff, RMAIL_EOOH, sizeof(RMAIL_EOOH) - 1)) {
	break;
      }
    }
    if (second) {
      copy_fields(fp, rmail_fields, 0,	/* �ɤ߼ΤƤʤΤ� 0 ���ɤ�	*/
		  CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
    } else {
      lines = copy_fields(fp, rmail_fields,
			  sizeof(rmail_fields) / sizeof(struct cpy_hdr),
			  CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
    }
    while (fgets(buff, sizeof(buff), fp)) {
      if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
	break;
      }
      lines++;
    }
    if (alloc_number <= rmail_article_number) {
      if (article_list != (ARTICLE_LIST*)NULL) {
	alloc_ptr = (char*)realloc(article_list,
				   sizeof(ARTICLE_LIST) *
				   (alloc_number + RMAIL_ALLOC_COUNT));
      } else {
	alloc_ptr = (char*)malloc(sizeof(ARTICLE_LIST) *
				  (alloc_number + RMAIL_ALLOC_COUNT));
      }
      if (alloc_ptr == (char*)NULL) {
	print_fatal("Can't allocate memory for RMAIL article struct.");
	break;
      }
      article_list = (ARTICLE_LIST*)alloc_ptr;
#ifdef	REF_SORT
      if (sort_rule == 1) {
	if (message_list != (MESSAGE_LIST*)NULL) {
	  alloc_ptr = (char*)realloc(message_list,
				     sizeof(MESSAGE_LIST) *
				     (alloc_number + RMAIL_ALLOC_COUNT));
	} else {
	  alloc_ptr = (char*)malloc(sizeof(MESSAGE_LIST) *
				    (alloc_number + RMAIL_ALLOC_COUNT));
	}
	if (alloc_ptr == (char*)NULL) {
	  print_fatal("Can't allocate memory for message struct.");
	  break;
	}
	message_list = (MESSAGE_LIST*)alloc_ptr;
      }
#endif	/* REF_SORT */
      alloc_number += RMAIL_ALLOC_COUNT;
    }
    article_list[rmail_article_number].real_number = rmail_article_number + 1;
    article_list[rmail_article_number].lines = lines;
    article_list[rmail_article_number].mark = mark;
    get_real_adrs(from_buff, article_list[rmail_article_number].from);
    if (my_adrs_mode) {
      if (check_my_adrs(article_list[rmail_article_number].from)) {
	article_list[rmail_article_number].from[0] = MY_ADRS_CHAR;
	get_real_adrs(to_buff, &article_list[rmail_article_number].from[1]);
      }
    }
    if (x_nsubj_mode && x_nsubj_buff[0]) {
      recover_jis(subject_buff, x_nsubj_buff);
    }
    mime_decode_func(from_buff, subject_buff, default_code);
    euc_tab_strncpy(article_list[rmail_article_number].subject, from_buff,
		    MAX_SUBJECT_LEN - 1);
#ifdef	REF_SORT
    convert_article_date(date_buff, &article_list[rmail_article_number].year,
			 &article_list[rmail_article_number].month,
			 &article_list[rmail_article_number].date,
			 &day, &hour, &minute, &second, from_buff);
    if (message_list != (MESSAGE_LIST*)NULL) {
      strncpy(message_list[rmail_article_number].msg_id, message_buff,
	      MAX_FIELD_LEN - 1);
      message_list[rmail_article_number].msg_id[MAX_FIELD_LEN - 1] = '\0';
      ptr = strrchr(reference_buff, '<');
      if (ptr != (char*)NULL) {
	strncpy(message_list[rmail_article_number].ref_id, ptr,
		MAX_FIELD_LEN - 1);
	message_list[rmail_article_number].ref_id[MAX_FIELD_LEN - 1] = '\0';
      } else {
	ptr = strrchr(in_reply_buff, '<');
	if (ptr != (char*)NULL) {
	  strncpy(message_list[rmail_article_number].ref_id, ptr,
		  MAX_FIELD_LEN - 1);
	  message_list[rmail_article_number].ref_id[MAX_FIELD_LEN - 1] = '\0';
	} else {
	  message_list[rmail_article_number].ref_id[0] = '\0';
	}
      }
      ptr = strchr(message_list[rmail_article_number].ref_id, '>');
      if (ptr != (char*)NULL) {
	*(ptr + 1) = '\0';
      }
    }
#else	/* !REF_SORT */
    convert_article_date(date_buff, &year,
			 &article_list[rmail_article_number].month,
			 &article_list[rmail_article_number].date,
			 &day, &hour, &minute, &second, from_buff);
#endif	/* !REF_SORT */
    rmail_article_number++;
  }
  fclose(fp);

  /*	�����Ƚ���	*/

  if (sort_articles(-1, &rmail_article_number,
		    mail_thread_mode, 0,
		    (int (*)())NULL, (void (*)())NULL) < 0) {
    return(-1);
  }

  /*	�ѥå�����	*/

#ifdef	notdef
  if (mail_article_mask) {
    pack_articles(-1, &rmail_article_number, (int (*)())NULL,
		  (void (*)())NULL);
  }
#endif	/* notdef */
  return(0);
}

/*
 * ���嵭��������
 */

static void	rmail_include(spool_file, rm_flag)
     char	*spool_file;
     int	rm_flag;
{
  FILE		*fp1, *fp2;
  struct stat	stat_buff;
  POP3_ERR_CODE	status;
  POP3_RD_STAT	rd_stat;
  int		mode;
  int		field;
  int		b_flag;
  int		p_flag;
  char		backup_file[PATH_BUFF];
  char		reserve_file[PATH_BUFF];
  char		buff[BUFF_SIZE];

  /*	�Хå����åץե�����إ�͡���	*/

  create_backup_name(backup_file, rmail_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  b_flag = 0;
  if (rename(rmail_file, backup_file)) {
    if (errno != ENOENT) {
      print_fatal("Can't backup RMAIL folder file.");
      return;
    }
  } else {
    b_flag = 1;
  }
  if (stat(backup_file, &stat_buff)) {
    stat_buff.st_mode = 0;
  }
  status = POP3_OK;
  fp1 = (FILE*)NULL;
  fp2 = fopen(rmail_file, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't save new RMAIL folder file.");
    status = POP3_ERR_PROG;
  } else {
    chmod(rmail_file, ((stat_buff.st_mode & 01777) | S_IREAD | S_IWRITE));
    print_mode_line(japanese ? "RMAIL �ե�����򹹿���Ǥ���" :
		    "Updating rmail folder file.");
    fprintf(fp2, "%s", RMAIL_EMACS_FIELD1);
    fprintf(fp2, RMAIL_NOTE_FIELD1, MNEWS_VERSION);
    fprintf(fp2, RMAIL_NOTE_FIELD2);
    fprintf(fp2, RMAIL_NOTE_FIELD3);
    fprintf(fp2, "%s", RMAIL_SEPARATER);
    if (b_flag) {
      fp1 = fopen(backup_file, "r");
      if (fp1 != (FILE*)NULL) {
	while (fgets(buff, sizeof(buff), fp1)) {
	  if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
	    break;
	  }
	}
	status = POP3_ERR_PROG;
	while (1) {
	  if (!fgets(buff, sizeof(buff), fp1)) {
	    if (status == POP3_OK) {
	      fprintf(fp2, "%s", RMAIL_SEPARATER);
	    }
	    status = POP3_OK;
	    break;
	  }
	  if (status != POP3_OK) {
	    fprintf(fp2, "%s\n", RMAIL_SEPARATER2);
	    status = POP3_OK;
	  }
	  if (strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
	    fputs(buff, fp2);
	  } else {
	    status = POP3_ERR_PROG;
	    fprintf(fp2, "%s", RMAIL_SEPARATER);
	  }
	}
	fclose(fp1);
      }
    }
  }

  /*	�᡼�������	*/

  mode = pop3_connect_mode;
  if (spool_file != (char*)NULL) {
    pop3_connect_mode = 0;
    strcpy(reserve_file, mail_spool);
    strcpy(mail_spool, spool_file);
  }
  p_flag = 0;
  if (status == POP3_OK) {
    pop3_from_mode = 0;
    status = pop3_open();
    field = 1;
    while (status == POP3_OK) {
      p_flag = 1;
      status = pop3_read(buff, sizeof(buff), &rd_stat);
      switch (rd_stat) {
      case POP3_RD_HEAD:
      case POP3_RD_SEP:
	if (field) {
	  fprintf(fp2, "%s\n0, %s,,\n", RMAIL_SEPARATER2, RMAIL_UNSEEN_ATTRIB);
	  fprintf(fp2, "%s\n", RMAIL_EOOH);
	  field = 0;
	}
	/* break ���� */
      case POP3_RD_BODY:
	fputs(buff, fp2);
	break;
      case POP3_RD_FROM:
      case POP3_RD_END:
      case POP3_RD_EOF:
	if (field != 1) {
	  fprintf(fp2, "%s", RMAIL_SEPARATER);
	  field = 1;
	}
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
      status = POP3_ERR_COMM;
      print_fatal("Can't save new RMAIL folder file.");
    }
    fclose(fp2);
  }
  if (status != POP3_OK) {
    if (b_flag) {
      rename(backup_file, rmail_file);
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
 * RMAIL �ե����빹��(�ޡ������������)
 */

static void	rmail_update(mode, refile_folder)
     int	mode;		/* 0:�ޡ�������,0�ʳ�:��� or ��ư */
     char	*refile_folder;	/* ��ư��ե���� */
{
  FILE		*fp1, *fp2, *fp3;
  struct stat	stat_buff;
  int		article_number;
  int		b_flag;
  int		status;
  char		backup_file[PATH_BUFF];
  char		buff[BUFF_SIZE];
  char		*ptr1, *ptr2;
  long		position;
  register int	i, j;
  int		ignore;

  /*	����ɬ���������å�	*/

  if (!(mode || rmail_modify)) {
    return;
  }

  /*	�Хå����åץե�����إ�͡���	*/

  create_backup_name(backup_file, rmail_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  b_flag = 0;
  if (rename(rmail_file, backup_file)) {
    if (errno != ENOENT) {
      print_fatal("Can't backup RMAIL folder file.");
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
    print_fatal("Can't read backup RMAIL folder file.");
    status = 1;
  }
  fp2 = fopen(rmail_file, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't save new RMAIL folder file.");
    status = 1;
  } else {
    chmod(rmail_file, ((stat_buff.st_mode & 01777) | S_IREAD | S_IWRITE));
  }
  fp3 = (FILE*)NULL;
  if (refile_folder != (char*)NULL) {
    fp3 = fopen(refile_folder, "a");
    if (fp3 == (FILE*)NULL) {
      print_fatal("Can't append RMAIL folder file.");
      status = 1;
    } else {
      print_mode_line(japanese ? "��ե�������Ǥ���" : "Refiling.");
    }
  } else {
    if (mode) {
      print_mode_line(japanese ? "�����Ǥ���" : "Removing.");
    } else {
      print_mode_line(japanese ? "������Ǥ���" : "Updating.");
    }
  }
  article_number = 0;
  while ((!status) && (!feof(fp1))) {
    if (!fgets(buff, sizeof(buff), fp1)) {
      print_fatal("Unexpected RMAIL folder file format.");
      status = 1;
      break;
    }
    if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
      break;
    }
    fputs(buff, fp2);
  }
  while ((!status) && (!feof(fp1))) {
    for (i = 0; i < rmail_article_number; i++) {
      if (article_list[i].real_number == (article_number + 1)) {
	break;
      }
    }
    if (i < rmail_article_number) {
      if (mode && (article_list[i].mark & DELETE_MARK)) {
	if (fp3 != (FILE*)NULL) {
	  fprintf(fp3, "%s\n", RMAIL_SEPARATER2);
	}
	j = 1;
	while (fgets(buff, sizeof(buff), fp1)) {
	  if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
	    if (fp3 != (FILE*)NULL) {
	      fprintf(fp3, "%s", RMAIL_SEPARATER);
	    }
	    break;
	  }
	  if (fp3 != (FILE*)NULL) {
	    if (j) {
	      j = 0;
	      ptr1 = strindex(buff, RMAIL_DELETE_STRING);
	      if (ptr1 != (char*)NULL) {
		ptr2 = ptr1 + sizeof(RMAIL_DELETE_STRING) - 1;
		while (*ptr2) {
		  *ptr1++ = *ptr2++;
		};
		*ptr1 = '\0';
	      }
	    }
	    fputs(buff, fp3);
	  }
	}
	article_number++;
	continue;
      }
    }
    fprintf(fp2, "%s%s\n", RMAIL_SEPARATER, RMAIL_SEPARATER2);
    if (i < rmail_article_number) {
      if (fgets(buff, sizeof(buff), fp1)) {
	if (buff[0] == '0') {
	  if (article_list[i].mark & READ_MARK) {
	    if (fgets(buff, sizeof(buff), fp1)) {
	      fprintf(fp2, "1,");
	      if (article_list[i].mark & FORWARD_MARK) {
		fprintf(fp2, " %s,", RMAIL_FORWARD_ATTRIB);
	      }
	      if (article_list[i].mark & ANSWER_MARK) {
		fprintf(fp2, " %s,", RMAIL_ANSWER_ATTRIB);
	      }
	      if (article_list[i].mark & DELETE_MARK) {
		fprintf(fp2, " %s,", RMAIL_DELETE_ATTRIB);
	      }
	      fprintf(fp2, ",\n");
	      if (strncmp(buff, RMAIL_EOOH, sizeof(RMAIL_EOOH) - 1)) {
		print_fatal("Unexpected RMAIL folder file format.");
	      }
	      position = ftell(fp1);
	      while (fgets(buff, sizeof(buff), fp1)) {
		fputs(buff, fp2);
		if ((buff[0] == '\n') || (buff[0] == '\0')) {
		  break;
		}
	      }
	      fprintf(fp2, "%s\n", RMAIL_EOOH);
	      fseek(fp1, (long)position, 0);
	      ignore = 0;
	      while (fgets(buff, sizeof(buff), fp1)) {
		if ((buff[0] != ' ') && (buff[0] != '\t')) {
		  ignore = 0;
		  for (j = 0; j < (sizeof(rmail_ignore_field)
				   / sizeof(char*)); j++) {
		    if (!strncasecmp(buff, rmail_ignore_field[j],
				     strlen(rmail_ignore_field[j]))) {
		      ignore = 1;
		      break;
		    }
		  }
		}
		if (!ignore) {
		  fputs(buff, fp2);
		}
		if ((buff[0] == '\n') || (buff[0] == '\0')) {
		  break;
		}
	      }
	    }
	  } else {
	    fprintf(fp2, "0, %s,", RMAIL_UNSEEN_ATTRIB);
	    if (article_list[i].mark & FORWARD_MARK) {
	      fprintf(fp2, " %s,", RMAIL_FORWARD_ATTRIB);
	    }
	    if (article_list[i].mark & ANSWER_MARK) {
	      fprintf(fp2, " %s,", RMAIL_ANSWER_ATTRIB);
	    }
	    if (article_list[i].mark & DELETE_MARK) {
	      fprintf(fp2, " %s,", RMAIL_DELETE_ATTRIB);
	    }
	    fprintf(fp2, ",\n");
	  }
	} else {
	  if (article_list[i].mark & READ_MARK) {
	    fprintf(fp2, "1,");
	    if (article_list[i].mark & FORWARD_MARK) {
	      fprintf(fp2, " %s,", RMAIL_FORWARD_ATTRIB);
	    }
	    if (article_list[i].mark & ANSWER_MARK) {
	      fprintf(fp2, " %s,", RMAIL_ANSWER_ATTRIB);
	    }
	    if (article_list[i].mark & DELETE_MARK) {
	      fprintf(fp2, " %s,", RMAIL_DELETE_ATTRIB);
	    }
	    fprintf(fp2, ",\n");
	  } else {
	    fprintf(fp2, "0, %s,", RMAIL_UNSEEN_ATTRIB);
	    if (article_list[i].mark & FORWARD_MARK) {
	      fprintf(fp2, " %s,", RMAIL_FORWARD_ATTRIB);
	    }
	    if (article_list[i].mark & ANSWER_MARK) {
	      fprintf(fp2, " %s,", RMAIL_ANSWER_ATTRIB);
	    }
	    if (article_list[i].mark & DELETE_MARK) {
	      fprintf(fp2, " %s,", RMAIL_DELETE_ATTRIB);
	    }
	    fprintf(fp2, ",\n");
	    fprintf(fp2, "%s\n", RMAIL_EOOH);
	    while (fgets(buff, sizeof(buff), fp1)) {
	      if (!strncmp(buff, RMAIL_EOOH, sizeof(RMAIL_EOOH) - 1)) {
		while (fgets(buff, sizeof(buff), fp1)) {
		  if ((buff[0] == '\n') || (buff[0] == '\0')) {
		    break;
		  }
		}
		break;
	      }
	      fputs(buff, fp2);
	    }
	  }
	}
      }
    }
    while (fgets(buff, sizeof(buff), fp1)) {
      if (!strncmp(buff, RMAIL_SEPARATER, sizeof(RMAIL_SEPARATER) - 1)) {
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
    fprintf(fp2, "%s", RMAIL_SEPARATER);
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
    print_fatal("Can't update RMAIL folder file.");
    if (b_flag) {
      rename(backup_file, rmail_file);
    }
  }
}

/*
 * �ե��������
 */

static int	rmail_create_folder(folder_name)
     char	*folder_name;
{
  struct stat	stat_buff;
  FILE		*fp;
  char		buff[BUFF_SIZE];

  sprintf(buff, "%s%c%s", rmail_mbox, SLASH_CHAR, folder_name);
  if (stat(buff, &stat_buff)) {
    fp = fopen(buff, "w");
    if (fp != (FILE*)NULL) {
      chmod(buff, S_IREAD | S_IWRITE);
      fprintf(fp, "%s", RMAIL_EMACS_FIELD1);
      fprintf(fp, RMAIL_NOTE_FIELD1, MNEWS_VERSION);
      fprintf(fp, RMAIL_NOTE_FIELD2);
      fprintf(fp, RMAIL_NOTE_FIELD3);
      fprintf(fp, "%s", RMAIL_SEPARATER);
      fclose(fp);
      return(0);
    }
  }
  term_bell();
  print_mode_line(japanese ? "�ե�����������Ǥ��ޤ���Ǥ�����" :
		  "Can't create folder.");
  sleep(ERROR_SLEEP);
  return(1);
}

/*
 * �ե�������
 */

static int	rmail_delete_folder(folder_name)
     char	*folder_name;
{
  char		buff[BUFF_SIZE];

  sprintf(buff, "%s%c%s", rmail_mbox, SLASH_CHAR, folder_name);
  if (!unlink(buff)) {
    return(0);
  }
  term_bell();
  print_mode_line(japanese ? "�ե����������Ǥ��ޤ���Ǥ�����" :
		  "Can't delete folder.");
  sleep(ERROR_SLEEP);
  return(1);
}

/*
 * �ե������������
 */

static int	rmail_adjust_folder(folder_name)
     char	*folder_name;
{
  int	current_folder;
  int	i;

  current_folder = 0;
  for (i = 0; i < rmail_folder_number; i++) {
    if (strnamecmp(rmail_folder[i].folder_name, folder_name) > 0) {
      break;
    }
    current_folder = i;
  }
  return(current_folder);
}

#ifndef	SMALL
/*
 * �ե����̾����(ʣ��)
 */

static int	rmail_search_name(mode, max_folder, folder_ptr)
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
	if (strindex(rmail_folder[current_folder].folder_name, str)) {
	  print_mode_line(japanese ? "���Ĥ��ޤ�����" : "Search succeed.");
	  *folder_ptr = current_folder;
	  return(0);
	}
      }
    } else {
      for (current_folder = *folder_ptr + 1; current_folder < max_folder;
	   current_folder++) {
	if (strindex(rmail_folder[current_folder].folder_name, str)) {
	  print_mode_line(japanese ? "���Ĥ��ޤ�����" : "Search succeed.");
	  *folder_ptr = current_folder;
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
#endif	/* RMAIL */
