/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : MH support routine
 *  File        : mh.c
 *  Version     : 1.21
 *  First Edit  : 1992-07/16
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 ��� ��
 *
 */

#ifdef	MH
#include	"compat.h"
#include	"nntplib.h"
#include	"smtplib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"termlib.h"
#include	"mnews.h"
#include	"group.h"
#include	"article.h"
#include	"mh.h"
#include	"pager.h"
#include	"mailsend.h"

static int	mh_select_folder();	/* �ե��������			*/
static void	mh_redraw_folder();	/* �ե����������̺�����	*/
static int	mh_select_article();	/* ��������			*/
static void	mh_redraw_article();	/* ����������̺�����		*/
static int	mh_prev_sfolder();	/* ������ե�����ֹ����	*/
static int	mh_next_sfolder();	/* ������ե�����ֹ����	*/
static int	mh_prev_article();	/* �������ֹ����		*/
static int	mh_next_article();	/* �������ֹ����		*/
static int	mh_prev_unread_article();
					/* �������ֹ����		*/
static int	mh_next_unread_article();
					/* �������ֹ����		*/
#ifdef	LARGE
static int	mh_prev_mark_article();	/* ���ޡ��������ֹ����		*/
static int	mh_next_mark_article();	/* ���ޡ��������ֹ����		*/
#endif	/* LARGE */
static int	mh_read();		/* ��������			*/
static int	mh_extract();		/* �������			*/
static void	mh_mark();		/* �����ޡ����ɲ�		*/
static void	mh_unmark();		/* �����ޡ������		*/
static void	mh_save_current();	/* �����Ȱ�����¸		*/
static void	mh_pick();		/* �����ԥå�			*/
static void	mh_count();		/* �������������		*/
static int	mh_trans_current();	/* �����ȵ����ֹ��Ѵ�		*/
static void	mh_search_all_folder();	/* ���ե��������		*/
static void	mh_search_folder();	/* �ե��������			*/
static void	mh_match_folder();	/* �ե�����ȹ�			*/
static int	mh_get_list();		/* �����ꥹ�ȼ���		*/
static int	mh_get_field();		/* �����إå�����		*/
static int	mh_delete_folder();	/* �ե�������			*/
static int	mh_adjust_folder();	/* �ե������������		*/
static int	mh_read_fields();	/* �ե�����ɼ���(ʣ��)		*/
static int	mh_read_field();	/* �ե�����ɼ���(ñ��)		*/
static int	mh_exec_command();	/* MH ���ޥ�ɼ¹�		*/
static int	mh_search_name();	/* �ե����̾����		*/
#ifndef	USE_MH
static int	create_folder();	/* �ե��������			*/
#endif	/* !USE_MH */

/*
 * MH �ե�����ꥹ��
 */

static int	mh_folder_number;	/* MH �ե������		*/
static int	mh_select_number;	/* ���򤵤�Ƥ���ե������	*/
static int	mh_alloc_number;	/* MH �ե����������		*/
static char	mh_path[PATH_BUFF];	/* MH �ѥ�			*/
static char	mh_draft_folder[PATH_BUFF];
					/* MH �ɥ�եȥե����		*/
static char	mh_unseen_seq[SMALL_BUFF];
					/* MH ̤�ɴ����ե������	*/
static int	top_position;		/* ɽ�����ϰ���			*/
static int	mh_article_number;	/* MH ������			*/
static struct mh_folder	*mh_folder = (struct mh_folder*)NULL;
					/* MH �ե����			*/
static struct mh_folder	*mh_selected_folder = (struct mh_folder*)NULL;
					/* MH ����ե����		*/
static unsigned int	mh_f_protect = MH_DEFAULT_F_PROTECT;
					/* MH �ե�����ץ�ƥ���	*/
static unsigned int	mh_m_protect = MH_DEFAULT_M_PROTECT;
					/* MH ��å������ץ�ƥ���	*/

/*
 * MH �إ�ץ�å�����
 */

static char	*mh_folder_jmessage[] = {
  "MH ���ƥ���/�ե��������⡼��",
  "\n  [����]",
  "\ti �ޤ��� SPACE  �ե���������򤷤ޤ���",
  "\n  [��ư]",
  "\tk, ^P �ޤ��� p  ���Υե�����˰�ư���ޤ���",
  "\tj, ^N �ޤ��� n  ���Υե�����˰�ư���ޤ���",
  "\t^U �ޤ��� ^B    �����̤Υե�����˰�ư���ޤ���",
  "\t^D, ^F �ޤ��� ^V",
  "\t                �����̤Υե�����˰�ư���ޤ���",
  "\t<               ��Ƭ�Υե�����˰�ư���ޤ���",
  "\t>               �Ǹ�Υե�����˰�ư���ޤ���",
  "\tTAB             ����Υե�������ƥ���ޤ��ϥե�����˥����פ��ޤ���",
  "\t                (���ƥ���ޤ��ϥե����̾�����Ϥ��Ʋ�����)",
#ifdef	LARGE
  "\tI               �����ѥե�����˰�ư���ޤ���",
#endif	/* LARGE */
  "\to, q �ޤ��� RETURN",
  "\t                ���ƥ���/�ե��������ȴ���ޤ���",
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
  "\n  [ɽ��]",
  "\tG               �˥塼�����롼��/�ե������ɽ�����������ؤ��ޤ���",
  "\t                (�ȥåץե�������ƥ�����Τ�ͭ��)",
  "\001",
  "\tC               �����˥ե������������ޤ���",
  "\t                (�ե����̾�����Ϥ��Ʋ�����)",
  "\tDEL �ޤ��� BS   �ե�����������ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
  (char*)NULL,
};

static char	*mh_article_jmessage[] = {
  "MH ��������⡼��",
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
  "\t$               ������ԥå����ƺ���ޡ������ޤ���",
  "\t                (�ԥå�����ѥ���������Ϥ��Ʋ�����)",
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
  "\tL               UCB-mail �ޤ��ϥѥå������ε����ե����������ߤޤ���",
  "\t                (�ե�����̾�����Ϥ��Ʋ�����)",
#endif	/* !SMALL */
  "\n  [���/����]",
  "\tDEL �ޤ��� BS   ����ޡ������줿�����������ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
  "\t^               ����ޡ������줿�������̤Υե�����˰�ư���ޤ���",
  "\t                (��ư��ե����̾����Ƥ��ޤ�)",
  "\t#               �����η��֤�Ĥ�ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
  "\t%               ������������˥����Ȥ��ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
#ifdef	LARGE
  "\tb               �����������ȵ�����С����Ȥ��ޤ���",
  "\t                (y/n �ǳ�ǧ����Ƥ��ޤ�)",
#endif	/* LARGE */
  "\n  [ɽ��]",
  "\tl               ����/������줿������ɽ���⡼�ɤ����ؤ��ޤ���",
  "\tt               ����åɥ⡼�ɤ����ؤ��ޤ���",
#ifdef	REF_SORT
  "\tT               ��������ˡ�����ؤ��ޤ���",
#endif	/* REF_SORT */
  (char*)NULL,
};

static char	*mh_folder_message[] = {
#ifndef	SMALL
  "MH CATEGORY/FOLDER SELECT MODE",
  "\n  [SELECT]",
  "\ti or SPACE   select folder.",
  "\n  [MOVE]",
  "\tk, ^P or p   previous folder.",
  "\tj, ^N or n   next folder.",
  "\t^U or ^B     previous page folder.",
  "\t^D, ^F or ^V next page folder.",
  "\t<            top folder.",
  "\t>            last folder.",
  "\tTAB          jump to specified category/folder.",
  "\t             (Please input category/folder name.)",
#ifdef	LARGE
  "\tI            jump to new article folder.",
#endif	/* LARGE */
  "\to, q or RETURN",
  "\t             exit from category/folder.",
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
  "\tG            toggle news(board) group/folder print mode.",
  "\t             (When top folder only)",
  "\001",
  "\tC            create new folder.",
  "\t             (Please input folder name.)",
  "\tDEL or BS    delete folder.",
  "\t             (Please make sure y/n.)",
#endif	/* !SMALL */
  (char*)NULL,
};

static char	*mh_article_message[] = {
#ifndef	SMALL
  "MH ARTICLE SELECT MODE",
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
  "\t$            pick articles and mark as delete.",
  "\t             (Please input pick pattern.)",
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
  "\tL            include UCB-mail or pack format article file.",
  "\t             (Please input file name.)",
  "\n  [DELETE/REORDER]",
  "\tDEL or BS    remove delete marked article.",
  "\t             (Please make sure y/n.)",
  "\t^            refile delete marked article.",
  "\t             (Please input folder name.)",
  "\t#            pack articles.",
  "\t             (Please make sure y/n.)",
  "\t%            sort article by date.",
  "\t             (Please make sure y/n.)",
#ifdef	LARGE
  "\tb            burst digest article.",
  "\t             (Please make sure y/n.)",
#endif	/* LARGE */
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
 * MH �⡼�ɽ����
 */

int	mh_init()
{
  static struct mh_field	init_field[] = {
    {MH_F_PROTECT_FIELD,NULL,			NULL},
    {MH_M_PROTECT_FIELD,NULL,			NULL},
    {MH_UNSEEN_FIELD,	mh_unseen_seq,		NULL},
    {MH_DRAFT_FIELD,	mh_draft_folder,	MH_DEFAULT_FOLDER},
    {MH_PATH_FIELD,	mh_path,		MH_DEFAULT_PATH},
    {NULL,		NULL,			NULL}};
  struct stat			stat_buff;
  char				init_file[PATH_BUFF];
  char				buff1[SMALL_BUFF];
  char				buff2[SMALL_BUFF];

  if (!mh_mode) {
    return(0);
  }
  sprintf(init_file, "%s%c%s", home_dir, SLASH_CHAR, MH_INIT_FILE);
  buff1[0] = buff2[0] = '0';
  init_field[0].field_buff = &buff1[1];
  init_field[1].field_buff = &buff2[1];
  mh_read_fields(init_file, init_field);
  if (buff1[1]) {
    sscanf(buff1, "%o", &mh_f_protect);
  }
  if (buff2[1]) {
    sscanf(buff2, "%o", &mh_m_protect);
  }
  if (mh_path[0]) {
#if	defined(MSDOS) || defined(OS2)
    if ((mh_path[0] != SLASH_CHAR) &&
	((!isalpha(mh_path[0])) || (mh_path[1] != ':'))) {
#else	/* !(MSDOS || OS2) */
    if (mh_path[0] != SLASH_CHAR) {
#endif	/* !(MSDOS || OS2) */
      strcpy(init_file, mh_path);
      sprintf(mh_path, "%s%c%s", home_dir, SLASH_CHAR, init_file);
    }
  } else {
    sprintf(mh_path, "%s%c%s", home_dir, SLASH_CHAR, MH_DEFAULT_PATH);
  }
  if ((!stat(mh_path, &stat_buff)) && (stat_buff.st_mode & S_IFDIR)) {
    return(1);
  }
  return(0);
}

/*
 * MH ��˥塼
 */

int	mh_menu()
{
  char		buff[BUFF_SIZE];
  int		status;

  while (1) {
    print_mode_line(japanese ? "������Ǥ���" : "Searching.");
    if ((!select_name[0]) && (!jump_name[0])) {
      mh_search_all_folder();
      if (mh_select_mode) {
	sprintf(buff, "%s%c%s", mh_path, SLASH_CHAR, MH_CONTEXT_FILE);
	mh_read_field(buff, MH_CONTEXT_FIELD, jump_name);
      }
      if (auto_inc_mode && auto_inc_folder[0] && (check_new_mail() > 0)) {
	strcpy(jump_name, auto_inc_folder);
      }
    }
    mh_match_folder(select_name);
    status = mh_select_folder();
    if (select_name[0] || (!jump_name[0])) {
      break;
    }
  }
  return(status);
}

/*
 * �ե����������̺�����
 */

static void	mh_redraw_folder(current_folder)
     int	current_folder;
{
  register int	i, j;
  char		buff[SMALL_BUFF];
  char		jname[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_folder,
				  mh_select_number, mail_thread_mode,
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
  get_jname(MH_JN_DOMAIN, jname, 40);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "�ե�������ƥ���:%-40.40s       ����:%s" :
		  "Folder category:%-40.40s    Position:%s",
		  jname, buff);
  if (!wide_mode) {
    j = (check_new_mail() > 0);
    term_locate(0, 2);
    term_attrib(RESET_ATTRIB);
    term_attrib(color_code[HEADER_COLOR]);
    print_full_line(japanese ?
		    "  ����    �Ǿ�    �ե����̾                       [%-8.8s]" :
		    "  Max     Min     Folder name                      [%-8.8s]",
		    new_mail_string[j]);
  }
  term_attrib(RESET_ATTRIB);
  for (i = 0, j = top_position; i < term_lines - mode_lines; i++, j++) {
    if (j >= mh_select_number) {
      break;
    }
    term_locate(0, head_lines + i);
#ifdef	COLOR
    term_attrib(color_code[NUMBER_COLOR]);
#endif	/* COLOR */
    if (mh_selected_folder[j].max_article >= 0) {
      cprintf(" %6d  %6d", mh_selected_folder[j].max_article,
	      mh_selected_folder[j].min_article);
    } else {
      cprintf("               ");
    }
    strcpy(buff, mh_selected_folder[j].folder_name);
#ifdef	JNAMES
    get_jname(MH_JN_DOMAIN, buff, term_columns - 24);
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

static int	mh_select_folder()
{
  /* �ѿ��ϺƵ�����Τ� static �ˤ��ʤ����� */

  int		current_folder;		/* ������Υե�����ֹ�		*/
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
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
      for (i = 0; i < mh_select_number; i++) {
	if (!strnnamecmp(mh_selected_folder[i].folder_name, jump_name,
			 strlen(mh_selected_folder[i].folder_name))) {
	  current_folder = i;
	  key = ' ';
	}
      }
      if (current_folder < 0) {
	for (i = 0; i < mh_select_number; i++) {
	  if (!strnnamecmp(mh_selected_folder[i].folder_name, jump_name,
			   strlen(jump_name))) {
	    if (current_folder < 0) {
	      current_folder = i;
	      key = ' ';
	    } else {
	      key = 0;
	    }
	  }
	}
      }
      if (current_folder < 0) {
	current_folder = 0;
      }
      if (!key) {
	jump_name[0] = '\0';
	continue;
      }
    } else {
      mh_redraw_folder(current_folder);
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
	  if (line < mh_folder_number) {
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
    case 'p':
      current_folder = mh_prev_sfolder(current_folder);
      break;
    case 0x0e:		/* ^N ���ե������ư	*/
    case 'j':
    case 'n':
      current_folder = mh_next_sfolder(current_folder);
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
	  >= mh_select_number) {
	if (mh_select_number > 0) {
	  current_folder = mh_select_number - 1;
	} else {
	  current_folder = 0;
	}
      }
      break;
    case '<':		/* �ǽ�Υե����	*/
      current_folder = 0;
      break;
    case '>':		/* �Ǹ�Υե����	*/
      if (mh_select_number > 0) {
	current_folder = mh_select_number - 1;
      } else {
	current_folder = 0;
      }
      break;
    case '\t':		/* TAB ���ꥫ�ƥ���/�ե�����إ�����	*/
      strcpy(buff1, mh_path);
      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
		 "�ե�������ƥ���ޤ��ϥե����̾�����Ϥ��Ʋ�����:",
		 "Input folder category or folder group name:", buff1);
      if (buff1[0]) {
	strcpy(jump_name, buff1);
	return(1);
      } else {
	top_position = -1;
      }
      break;
    case '?':		/* �إ��		*/
      help(mh_folder_jmessage, mh_folder_message,
	   GLOBAL_MODE_MASK | GROUP_MODE_MASK);
      top_position = -1;
      break;
    case 'Q':		/* ��λ			*/
      return(1);
      /* break; */
    case 'G':		/* ����ɽ���⡼��/���	*/
      if (!select_name[0]) {
	gnus_mode = !gnus_mode;
	mh_match_folder(select_name);
	current_folder = 0;
	top_position = -1;
      }
      break;
    case 'C':		/* �ե��������		*/
      strcpy(buff1, mh_path);
      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
		 "������������ե����̾�����Ϥ��Ʋ�����:",
		 "Input new folder name:", buff1);
      if (buff1[0]) {
	if (!mh_create_folder(buff1)) {
	  if (mh_select_number > 0) {
	    strcpy(buff1, mh_selected_folder[current_folder].folder_name);
	    mh_search_all_folder();
	    mh_match_folder(select_name);
	    current_folder = mh_adjust_folder(buff1);
	  } else {
	    mh_search_all_folder();
	    mh_match_folder(select_name);
	    current_folder = 0;
	  }
	}
      }
      top_position = -1;
      break;
    default:
      if (mh_select_number > 0) {
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
	  strcpy(buff1, mh_selected_folder[current_folder].folder_name);
	  strcpy(buff2, select_name);
	  strcpy(select_name,
		 mh_selected_folder[current_folder].folder_name);
	  if (mh_selected_folder[current_folder].max_article < 0) {
	    status = mh_menu();
	  } else {
	    status = mh_select_article(auto_inc_mode &&
				       ((!auto_inc_folder[0]) ||
					(!strnamecmp(auto_inc_folder,
		mh_selected_folder[current_folder].folder_name))));
	  }
	  strcpy(select_name, buff2);
	  if (status) {
	    return(1);
	  }
	  mh_match_folder(select_name);
	  current_folder = mh_adjust_folder(buff1);
	  top_position = -1;
	  break;
#ifndef	SMALL
	case '/':		/* ��������		*/
	  mh_search_name(0, mh_folder_number, &current_folder);
	  break;
	case '\\':		/* ��������		*/
	  mh_search_name(1, mh_folder_number, &current_folder);
	  break;
#endif	/* !SMALL */
	case 0x7f:		/* �ե�������		*/
	case 0x08:
	  if (mh_selected_folder[current_folder].max_article) {
	    print_mode_line(japanese ?
			    "�ե����(%s)�϶��ǤϤ���ޤ���" :
			    "Folder(%s) does not empty.",
			    mh_selected_folder[current_folder].folder_name);
	    term_bell();
	    sleep(ERROR_SLEEP);
	  } else if (!strnamecmp(mh_selected_folder[current_folder].folder_name,
				 select_name)) {
	    print_mode_line(japanese ?
			    "�ƥե�������ƥ���˰�ư���ƺ�����Ʋ�������" :
			    "Please move to parent folder category and delete.",
			    mh_selected_folder[current_folder].folder_name);
	    term_bell();
	    sleep(ERROR_SLEEP);
	  } else {
	    if (yes_or_no(CARE_YN_MODE, "�ե����(%s)�������ޤ���?",
			  "Delete folder(%s)?",
			  mh_selected_folder[current_folder].folder_name)) {
	      if (!mh_delete_folder(mh_selected_folder[current_folder].folder_name)) {
		strcpy(buff1, mh_selected_folder[current_folder].folder_name);
		mh_search_all_folder();
		mh_match_folder(select_name);
		current_folder = mh_adjust_folder(buff1);
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

static void	mh_redraw_article(current_folder, current_article)
     int	current_folder;
     int	current_article;
{
  register int	i;
  char		buff[SMALL_BUFF];
  char		jname[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_article,
				  mh_article_number, mail_thread_mode,
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
  get_jname(MH_JN_DOMAIN, jname, 48);
#endif	/* JNAMES */
  print_full_line(japanese ?
		  "�ե����:%-48.48s       ����:%s" :
		  "Folder:%-48.48s     Position:%s",
		  jname, buff);
  print_articles(top_position, current_folder, mh_article_number,
		 mh_get_field, "REMOVED");
  print_mode_line(japanese ?
		  "?:�إ�� j,^N:���� k,^P:���� SPACE,i,.:���� o,q:���� Q:��λ" :
		  "?:help j,^N:next k,^P:previous SPACE,i,.:read o,q:return Q:exit");
}

/*
 * ��������
 */

static int	mh_select_article(auto_flag)
     int	auto_flag;
{
  register int	current_folder;		/* ������Υե�����ֹ�		*/
  int		current_article;	/* ������ε����ֹ�		*/
  char		buff1[CMD_BUFF];
  char		buff2[CMD_BUFF];
  char		*ptr;
  int		loop;
  int		status;
  int		key;
  register int	i, j;

  current_folder = -1;
  jump_name[0] = '\0';
  for (i = 0; i < mh_folder_number; i++) {
    if (!strnamecmp(select_name, mh_folder[i].folder_name)) {
      current_folder = i;
      break;
    }
  }
  if (current_folder < 0) {
    print_fatal("Unexpected folder selected.");
    return(0);
  }
  print_mode_line(japanese ? "������Ǥ���" : "Searching.");
  if (auto_flag) {
    if (check_new_mail() > 0) {	/* ��ư�����ǽ		*/
      mh_exec_command(MH_INC_COMMAND, mh_folder[current_folder].folder_name,
		      (char*)NULL);
      mh_count(current_folder, 0);
    }
  }
  if (mh_get_list(current_folder) < 0) {
    return(0);
  }
  current_article =
    mh_trans_current(mh_folder[current_folder].current_article);
  top_position = -1;
  loop = 1;
  status = 0;
  while (loop) {
    mh_redraw_article(current_folder, current_article);
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
	if (line < mh_article_number) {
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
      current_article = mh_prev_article(current_article);
      break;
    case 0x0e:		/* ^N ��������ư	*/
    case 'j':
      current_article = mh_next_article(current_article);
      break;
#ifdef	LARGE
    case '{':		/* { ���ޡ���������ư	*/
      current_article = mh_prev_mark_article(current_article);
      break;
    case '}':		/* } ���ޡ���������ư	*/
      current_article = mh_next_mark_article(current_article);
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
	  >= mh_article_number) {
	if (mh_article_number > 0) {
	  current_article = mh_article_number - 1;
	} else {
	  current_article = 0;
	}
      }
      break;
    case '<':		/* �ǽ�ε���		*/
      current_article = 0;
      break;
    case '>':		/* �Ǹ�ε���		*/
      if (mh_article_number > 0) {
	current_article = mh_article_number - 1;
      } else {
	current_article = 0;
      }
      break;
#ifndef	SMALL
    case 'L':		/* �����ե����������	*/
      buff1[0] = '\0';
      input_line(INPUT_EXPAND_MASK | INPUT_COMP_MASK,
		 "�ե�����̾�����Ϥ��Ʋ�����:",
		 "Input file name:", buff1);
      if (buff1[0]) {
#if	defined(MSDOS) || defined(OS2)
	sprintf(buff2, "%s -file %s +%s > NUL",
		MH_INC_COMMAND, buff1,
		mh_folder[current_folder].folder_name);
#else	/* !(MSDOS || OS2) */
	sprintf(buff2, "exec %s%c%s -file %s +%s > /dev/null 2>&1",
		mh_command_path, SLASH_CHAR, MH_INC_COMMAND,
		buff1, mh_folder[current_folder].folder_name);
#endif	/* !(MSDOS || OS2) */
	if (system(buff2)) {
	  print_fatal("Can't execute inc command.");
	}
      }
      /* break ����	*/
#endif	/* !SMALL */
    case 'I':		/* ���嵭��������	*/
      mh_save_current(current_folder);
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
	mh_exec_command(MH_INC_COMMAND,
			mh_folder[current_folder].folder_name, (char*)NULL);
      }
      mh_count(current_folder, 0);
      mh_get_list(current_folder);
      current_article =
	mh_trans_current(mh_folder[current_folder].current_article);
      top_position = -1;
      break;
    case '?':		/* �إ��		*/
      help(mh_article_jmessage, mh_article_message,
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
      if (!change_sort_rule(current_folder, &mh_article_number,
			    &current_article,
			    &mail_article_mask, &mail_thread_mode,
			    mh_get_list, key)) {
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
	  if (mh_article_number > 0) {
	    if (key == 'p') {
	      current_article = mh_prev_unread_article(current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(!current_article);
	    } else {
	      current_article = mh_prev_article(current_article);
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    i = current_folder - 1;
	    if (i >= 0) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"���Υե����(%s)�򻲾Ȥ��ޤ���?",
				"Read previous folder(%s)?",
				mh_folder[i].folder_name)) {
	      case 1:
		mh_save_current(current_folder);
		strcpy(jump_name, mh_folder[i].folder_name);
		return(1);
		/* break; */
	      case 2:
		mh_save_current(current_folder);
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
	case 'n':		/* ����̤�ɵ�������	*/
	case 'N':		/* ���ε�������		*/
	  j = current_article;
	  if (mh_article_number > 0) {
	    if (key == 'n') {
	      current_article = mh_next_unread_article(current_article);
	      status = (article_list[current_article].mark & READ_MARK) &&
		(current_article == (mh_article_number - 1));
	    } else {
	      current_article = mh_next_article(current_article);
	      status = (current_article == j);
	    }
	  } else {
	    status = 1;
	  }
	  if (status) {
	    i = current_folder + 1;
	    if (i < mh_folder_number) {
	      switch (yes_or_no(JUMP_YN_MODE,
				"���Υե����(%s)�򻲾Ȥ��ޤ���?",
				"Read next folder(%s)?",
				mh_folder[i].folder_name)) {
	      case 1:
		mh_save_current(current_folder);
		strcpy(jump_name, mh_folder[i].folder_name);
		return(1);
		/* break; */
	      case 2:
		mh_save_current(current_folder);
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
	if (mh_article_number > 0) {
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
	    if (!mh_read(article_list[current_article].real_number, i)) {
	      mh_folder[current_folder].current_article = current_article;
	      switch (last_key) {
	      case 'D':
		mh_mark(current_folder, current_article,
			READ_MARK | DELETE_MARK);
		last_key = 'p';
		break;
	      case 'd':
		mh_mark(current_folder, current_article,
			READ_MARK | DELETE_MARK);
		last_key = 'n';
		break;
	      case 'Y':
		mh_mark(current_folder, current_article, READ_MARK);
		last_key = 'p';
		break;
	      case 'y':
		mh_mark(current_folder, current_article, READ_MARK);
		last_key = 'n';
		break;
	      case 'U':
		mh_mark(current_folder, current_article, READ_MARK);
		mh_unmark(current_folder, current_article, DELETE_MARK);
		last_key = 'p';
		break;
	      case 'u':
		mh_mark(current_folder, current_article, READ_MARK);
		mh_unmark(current_folder, current_article, DELETE_MARK);
		last_key = 'n';
		break;
	      case 'Z':
		mh_unmark(current_folder, current_article, READ_MARK);
		last_key = 'p';
		break;
	      case 'z':
		mh_unmark(current_folder, current_article, READ_MARK);
		last_key = 'n';
		break;
	      default:
		mh_mark(current_folder, current_article, READ_MARK);
		break;
	      }
	    }
	    if (pager_mode) {
	      switch (key) {
	      case 'p':
		current_article = mh_prev_unread_article(current_article);
		break;
	      case 'n':
	      case ' ':
		current_article = mh_next_unread_article(current_article);
		break;
	      case 'P':
		current_article = mh_prev_article(current_article);
		break;
	      case 'N':
		current_article = mh_next_article(current_article);
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
	    mh_mark(current_folder, current_article, DELETE_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'D') {
	      current_article = mh_prev_article(current_article);
	    } else {
	      current_article = mh_next_article(current_article);
	    }
	    break;
	  case 'U':		/* ����ޡ������,��������ư	*/
	  case 'u':		/* ����ޡ������,��������ư	*/
	    mh_unmark(current_folder, current_article, DELETE_MARK);
	    toggle_mark(top_position, current_article, 0);
	    if (key == 'U') {
	      current_article = mh_prev_article(current_article);
	    } else {
	      current_article = mh_next_article(current_article);
	    }
	    break;
          case 'Y':             /* ���ɥޡ���,��������ư	*/
          case 'y':             /* ���ɥޡ���,��������ư	*/
            mh_mark(current_folder, current_article, READ_MARK);
            toggle_mark(top_position, current_article, 0);
            if (key == 'Y') {
              current_article = mh_prev_article(current_article);
            } else {
              current_article = mh_next_article(current_article);
            }
            break;
          case 'Z':             /* ���ɥޡ������,��������ư	*/
          case 'z':             /* ���ɥޡ������,��������ư	*/
            mh_unmark(current_folder, current_article, READ_MARK);
            toggle_mark(top_position, current_article, 0);
            if (key == 'Z') {
              current_article = mh_prev_article(current_article);
            } else {
              current_article = mh_next_article(current_article);
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
	      for (i = 0; i < mh_article_number; i++) {
		mh_mark(current_folder, i, j);
	      }
	    }
	    top_position = -1;
	    break;
	  case '^':		/* �ޡ���������ե�����	*/
	    j = 0;
	    for (i = 0; i < mh_article_number; i++) {
	      if ((article_list[i].mark & DELETE_MARK)
		  && !(article_list[i].mark & CANCEL_MARK)) {
		j = 1;
	      }
	    }
	    if (j) {
	      strcpy(buff1, mh_path);
	      input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
			 "��ư��ե����̾�����Ϥ��Ʋ�����:",
			 "Input destination folder name:", buff1);
	      if (buff1[0]) {
		j = 0;
		for (i = 0; i < mh_folder_number; i++) {
		  if (!strnamecmp(mh_folder[i].folder_name, buff1)) {
		    status = i;
		    j = 1;
		    break;
		  }
		}
		if (!j) {
		  if (yes_or_no(NORMAL_YN_MODE,
				"�ե����(%s)�򿷵��������ޤ���?",
				"Create folder(%s).Are you sure?",
				buff1)) {
		    if (!mh_create_folder(buff1)) {
		      j = 1;
		      status = -1;
		    }
		  }
		}
		if (j) {
		  if (current_folder == status) {
		    print_mode_line(japanese ?
				    "��ư���Ȱ�ư�褬Ʊ���ե�����Ǥ���" :
				    "Source and destination folders are identical.");
		    term_bell();
		    sleep(ERROR_SLEEP);
		  } else {
		    print_mode_line(japanese ? "��ե�������Ǥ���" :
				    "Refiling.");
		    mh_save_current(current_folder);
		    i = 0;
		    while (1) {
		      j = 0;
#if	defined(MSDOS) || defined(OS2)
		      sprintf(buff2, "%s -src %s +%s ",
			      MH_REFILE_COMMAND,
			      mh_folder[current_folder].folder_name,
			      buff1);
#else	/* !(MSDOS || OS2) */
		      sprintf(buff2, "exec %s%c%s -src %s +%s ",
			      mh_command_path, SLASH_CHAR, MH_REFILE_COMMAND,
			      mh_folder[current_folder].folder_name,
			      buff1);
#endif	/* !(MSDOS || OS2) */
		      for (; i < mh_article_number; i++) {
			if ((article_list[i].mark & DELETE_MARK)
			    && !(article_list[i].mark & CANCEL_MARK)) {
			  ptr = buff2;
			  while (*ptr) {
			    ptr++;
			  }
			  j = 1;
			  sprintf(ptr, "%d ", article_list[i].real_number);
			  if (strlen(buff2) >= sizeof(buff2) - 32) {
			    i++;
			    break;
			  }
			}
		      }
		      if (j) {
#if	defined(MSDOS) || defined(OS2)
			strcat(buff2, " > NUL");
#else	/* !(MSDOS || OS2) */
			strcat(buff2, " > /dev/null 2>&1");
#endif	/* !(MSDOS || OS2) */
#ifdef	DEBUG
			fprintf(stderr, "EXEC \"%s\"", buff2);
			sleep(10);
#else	/* !DEBUG */
			if (system(buff2)) {
			  print_fatal("Can't execute refile command.");
			}
#endif	/* !DEBUG */
		      } else {
			break;
		      }
		    }
		    strcpy(buff1, mh_folder[current_folder].folder_name);
		    mh_search_all_folder();
		    current_folder = 0;
		    for (i = 0; i < mh_folder_number; i++) {
		      if (!strnamecmp(mh_folder[i].folder_name, buff1)) {
			current_folder = i;
			break;
		      }
		    }
		    mh_get_list(current_folder);
		    current_article =
		      mh_trans_current(mh_folder[current_folder].current_article);
		  }
		}
	      }
	      status = 0;
	      top_position = -1;
	    }
	    break;
	  case '%':		/* ������		*/
	    if (yes_or_no(NORMAL_YN_MODE, "�����򥽡��Ȥ��Ƥ�����Ǥ���?",
			  "Sort articles.Are you sure?")) {
	      print_mode_line(japanese ? "��������Ǥ���" :
			      "Sorting.");
	      mh_save_current(current_folder);
	      mh_exec_command(MH_SORTM_COMMAND,
			      mh_folder[current_folder].folder_name,
			      (char*)NULL);
	      mh_count(current_folder, 0);
	      mh_get_list(current_folder);
	      current_article =
		mh_trans_current(mh_folder[current_folder].current_article);
	    }
	    status = 0;
	    top_position = -1;
	    break;
	  case 0x7f:		/* �ޡ����������	*/
	  case 0x08:
	    j = 0;
	    for (i = 0; i < mh_article_number; i++) {
	      if ((article_list[i].mark & DELETE_MARK)
		  && !(article_list[i].mark & CANCEL_MARK)) {
		j = 1;
	      }
	    }
	    if (j) {
	      if (yes_or_no(CARE_YN_MODE,
			    "�ޡ������줿���������ƺ�����Ƥ�����Ǥ���?",
			    "Delete all marked articles.Are you sure?")) {
		print_mode_line(japanese ? "�����Ǥ���" :
				"Removing.");
		mh_save_current(current_folder);
		i = 0;
		while (1) {
		  j = 0;
#if	defined(MSDOS) || defined(OS2)
		  sprintf(buff1, "%s +%s ", MH_RMM_COMMAND,
			  mh_folder[current_folder].folder_name);
#else	/* !(MSDOS || OS2) */
		  sprintf(buff1, "exec %s%c%s +%s ", mh_command_path,
			  SLASH_CHAR, MH_RMM_COMMAND,
			  mh_folder[current_folder].folder_name);
#endif	/* !(MSDOS || OS2) */
		  for (; i < mh_article_number; i++) {
		    if ((article_list[i].mark & DELETE_MARK)
			&& !(article_list[i].mark & CANCEL_MARK)) {
		      ptr = buff1;
		      while (*ptr) {
			ptr++;
		      }
		      j = 1;
		      sprintf(ptr, "%d ", article_list[i].real_number);
		      if (strlen(buff1) >= sizeof(buff1) - 32) {
			i++;
			break;
		      }
		    }
		  }
		  if (j) {
#if	defined(MSDOS) || defined(OS2)
		    strcat(buff1, " > NUL");
#else	/* !(MSDOS || OS2) */
		    strcat(buff1, " > /dev/null 2>&1");
#endif	/* !(MSDOS || OS2) */
#ifdef	DEBUG
		    fprintf(stderr, "EXEC \"%s\"", buff1);
		    sleep(10);
#else	/* !DEBUG */
		    if (system(buff1)) {
		      print_fatal("Can't execute rmm command.");
		    }
#endif	/* !DEBUG */
		  } else {
		    break;
		  }
		}
		mh_count(current_folder, 0);
		mh_get_list(current_folder);
		current_article =
		  mh_trans_current(mh_folder[current_folder].current_article);
	      }
	      status = 0;
	      top_position = -1;
	    }
	    break;
	  case '#':		/* �����ѥå�		*/
	    if (yes_or_no(NORMAL_YN_MODE, "������ѥå����Ƥ�����Ǥ���?",
			  "Pack articles.Are you sure?")) {
	      print_mode_line(japanese ? "�ѥå���Ǥ���" :
			      "Packing.");
	      mh_exec_command(MH_FOLDER_COMMAND,
			      mh_folder[current_folder].folder_name, "-pack");
	      mh_count(current_folder, 0);
	      mh_get_list(current_folder);
	      current_article = mh_folder[current_folder].current_article;
	      if (current_article >= mh_article_number) {
		if ((current_article = mh_article_number - 1) < 0) {
		  current_article = 0;
		}
	      }
	    }
	    status = 0;
	    top_position = -1;
	    break;
	  case '$':		/* �����ԥå�		*/
	    buff1[0] = '\0';
	    input_line(0, "�ԥå�����ѥ���������Ϥ��Ʋ�����:",
		       "Input pick pattern:", buff1);
	    if (buff1[0]) {
	      mh_pick(current_folder, buff1);
	    }
	    status = 0;
	    top_position = -1;
	    break;
	  case 's':		/* ����������		*/
	    if (!multi_save(current_article, mh_extract)) {
	      multi_add_mark(mh_article_number, current_folder,
			     current_article, READ_MARK, mh_mark);
	    }
	    top_position = -1;
	    break;
#ifndef	SMALL
	  case '/':		/* ��������		*/
	    search_subjects(0, mh_article_number, &current_article,
			    mh_get_field);
	    break;
	  case '\\':		/* ��������		*/
	    search_subjects(1, mh_article_number, &current_article,
			    mh_get_field);
	    break;
	  case '|':		/* �����ѥ��׼¹�	*/
	    if (!multi_pipe(current_article, mh_extract)) {
	      multi_add_mark(mh_article_number, current_folder,
			     current_article, READ_MARK, mh_mark);
	    }
	    top_position = -1;
	    break;
#endif	/* !SMALL */
	  case '\t':		/* TAB ���국���إ�����	*/
	    buff1[0] = '\0';
	    input_line(INPUT_SPCCUT_MASK, "�����ֹ�����Ϥ��Ʋ�����:",
		       "Input article number:", buff1);
	    if ((j = atoi(buff1)) > 0) {
	      for (i = 0; i < mh_article_number; i++) {
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
	    sprintf(buff1, "%s%c%s%c%d", mh_path, SLASH_CHAR, select_name,
		    SLASH_CHAR, article_list[current_article].real_number);
	    if (!mail_reply(buff1, 0, select_name)) {
	      mh_mark(current_folder, current_article, ANSWER_MARK);
	    }
	    top_position = -1;
	    break;
	  case 'R':		/* �᡼���ֿ�		*/
	    create_temp_name(buff1, "MR");
	    if (!multi_extract(current_article, buff1, mh_extract)) {
	      if (!mail_reply(buff1, REPLY_QUOTE_MASK, select_name)) {
		multi_add_mark(mh_article_number, current_folder,
			       current_article, ANSWER_MARK, mh_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -1;
	    break;
	  case '=':		/* �᡼��ž��		*/
	  case 'f':
	    create_temp_name(buff1, "MT");
	    if (!multi_extract(current_article, buff1, mh_extract)) {
	      if (!mail_forward(buff1, select_name)) {
		multi_add_mark(mh_article_number, current_folder,
			       current_article, FORWARD_MARK, mh_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -1;
	    break;
#ifdef	LARGE
	  case '~':		/* �᡼�����		*/
	    create_temp_name(buff1, "ML");
	    if (!multi_extract(current_article, buff1, mh_extract)) {
	      if (!mail_relay(buff1, select_name)) {
		multi_add_mark(mh_article_number, current_folder,
			       current_article, FORWARD_MARK, mh_mark);
	      }
	      funlink2(buff1);
	    }
	    top_position = -1;
	    break;
#endif	/* LARGE */
#endif	/* MAILSEND */
	  case 'C':		/* ���ޡ������		*/
	    mh_unmark(current_folder, current_article,
		      READ_MARK | FORWARD_MARK | ANSWER_MARK);
	    toggle_mark(top_position, current_article, 0);
	    current_article = mh_next_article(current_article);
	    break;
	  case 'M':		/* �ޥ���ޡ�������/���	*/
	    multi_mark(top_position, current_article);
	    current_article = mh_next_article(current_article);
	    break;
#ifdef	LARGE
	  case '@':		/* ����			*/
	    if (!multi_print(current_article, mh_extract)) {
	      multi_add_mark(mh_article_number, current_folder,
			     current_article, READ_MARK, mh_mark);
	    }
	    top_position = -1;
	    break;
	  case '(':		/* ������������		*/
	    search_articles(0, mh_article_number, &current_article, 
			    mh_extract);
	    break;
	  case ')':		/* ������������		*/
	    search_articles(1, mh_article_number, &current_article, 
			    mh_extract);
	    break;
	  case 'b':		/* �С�����		*/
	    if (yes_or_no(NORMAL_YN_MODE, "������С����Ȥ��Ƥ�����Ǥ���?",
			  "Burst article.Are you sure?")) {
	      print_mode_line(japanese ? "�С�������Ǥ���" :
			      "Bursting.");
#if	defined(MSDOS) || defined(OS2)
	      sprintf(buff1, "%s -quiet +%s %d > NUL",
		      MH_BURST_COMMAND,
		      mh_folder[current_folder].folder_name,
		      article_list[current_article].real_number);
#else	/* !(MSDOS || OS2) */
	      sprintf(buff1, "exec %s%c%s -quiet +%s %d", mh_command_path,
		      SLASH_CHAR, MH_BURST_COMMAND,
		      mh_folder[current_folder].folder_name,
		      article_list[current_article].real_number);
#endif	/* !(MSDOS || OS2) */
#ifdef DEBUG
	      fprintf(stderr, "%s\n", buff1);
#endif /* DEBUG */
	      if (system(buff1)) {
		print_fatal("Can't execute burst command.");
	      }
	      mh_count(current_folder, 0);
	      mh_get_list(current_folder);
	      current_article =
		mh_trans_current(mh_folder[current_folder].current_article);
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
  mh_save_current(current_folder);
  return(status);
}

/*
 * ������ե�����ֹ����
 */

static int	mh_prev_sfolder(current_folder)
     int	current_folder;
{
  if (--current_folder < 0) {
    current_folder = 0;
  }
  return(current_folder);
}

/*
 * ������ե�����ֹ����
 */

static int	mh_next_sfolder(current_folder)
     int	current_folder;
{
  if (++current_folder >= mh_select_number) {
    if (mh_select_number > 0) {
      current_folder = mh_select_number - 1;
    } else {
      current_folder = 0;
    }
  }
  return(current_folder);
}

/*
 * �������ֹ����
 */

static int	mh_prev_article(current_article)
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

static int	mh_next_article(current_article)
     int	current_article;
{
  if (++current_article >= mh_article_number) {
    if (mh_article_number > 0) {
      current_article = mh_article_number - 1;
    } else {
      current_article = 0;
    }
  }
  return(current_article);
}

/*
 * ��̤�ɵ����ֹ����
 */

static int	mh_prev_unread_article(current_article)
     int	current_article;
{
  term_enable_abort();
  while (current_article > 0) {
    current_article--;
    if (article_list[current_article].mark & UNFETCH_MARK) {
      mh_get_field(-1, current_article);
    }
    if (!(article_list[current_article].mark & (READ_MARK | CANCEL_MARK))) {
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

static int	mh_next_unread_article(current_article)
     int	current_article;
{
  term_enable_abort();
  while (current_article < (mh_article_number - 1)) {
    current_article++;
    if (article_list[current_article].mark & UNFETCH_MARK) {
      mh_get_field(-1, current_article);
    }
    if (!(article_list[current_article].mark & (READ_MARK | CANCEL_MARK))) {
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

static int	mh_prev_mark_article(current_article)
     int	current_article;
{
  while (current_article > 0) {
    current_article--;
    if (article_list[current_article].mark & UNFETCH_MARK) {
      mh_get_field(-1, current_article);
    }
    if ((article_list[current_article].mark & DELETE_MARK) &&
        (!(article_list[current_article].mark & CANCEL_MARK))) {
      break;
    }
  }
  return(current_article);
}

/*
 * ���ޡ��������ֹ����
 */

static int	mh_next_mark_article(current_article)
     int	current_article;
{
  while (current_article < (mh_article_number - 1)) {
    current_article++;
    if (article_list[current_article].mark & UNFETCH_MARK) {
      mh_get_field(-1, current_article);
    }
    if ((article_list[current_article].mark & DELETE_MARK) &&
        (!(article_list[current_article].mark & CANCEL_MARK))) {
      break;
    }
  }
  return(current_article);
}
#endif	/* LARGE */

/*
 * ��������
 */

static int	mh_read(real_number, mode)
     int	real_number;
     int	mode;
{
  char	mh_file[PATH_BUFF];
  char	tmp_file[PATH_BUFF];
  char	buff[SMALL_BUFF];

  sprintf(mh_file, "%s%c%s%c%d", mh_path, SLASH_CHAR, select_name,
	  SLASH_CHAR, real_number);
  create_temp_name(tmp_file, "MV");
  sprintf(buff, "%d", real_number);
  if (exec_pager(mh_file, tmp_file, mode, buff)) {
    return(1);
  }
  return(0);
}

/*
 * �������
 */

static int	mh_extract(real_number, tmp_file)
     int	real_number;
     char	*tmp_file;
{
  FILE	*fp1, *fp2;
  char	mh_file[PATH_BUFF];
  char	buff[BUFF_SIZE];
  int	status;

  sprintf(mh_file, "%s%c%s%c%d", mh_path, SLASH_CHAR, select_name,
	  SLASH_CHAR, real_number);
  fp1 = fopen(mh_file, "r");
  if (fp1 == (FILE*)NULL) {
    return(1);
  }
  fp2 = fopen2(tmp_file, "a");
  if (fp2 == (FILE*)NULL) {
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
 * �����ޡ����ɲ�
 */

static void	mh_mark(current_folder, current_article, mark)
     int	current_folder;
     int	current_article;
     int	mark;
{
  FILE		*fp1, *fp2;
  struct stat	stat_buff;
  char		mh_file[PATH_BUFF];
  char		tmp_file[PATH_BUFF];
  char		buff[BUFF_SIZE];
  int		status;

  if (article_list[current_article].mark & CANCEL_MARK) {
    return;
  }
  mark &= (READ_MARK | FORWARD_MARK | ANSWER_MARK | DELETE_MARK);
  if (mark & READ_MARK) {
    if (article_list[current_article].mark & READ_MARK) {
      mark &= ~READ_MARK;
    } else if (mh_unseen_seq[0]) {
#if	defined(MSDOS) || defined(OS2)
      sprintf(buff, "%s +%s -sequence %s -delete %d > NUL",
	      MH_MARK_COMMAND, mh_folder[current_folder].folder_name,
	      mh_unseen_seq, article_list[current_article].real_number);
#else	/* !(MSDOS || OS2) */
      sprintf(buff, "%s/%s +%s -sequence %s -delete %d > /dev/null 2>&1",
	      mh_command_path, MH_MARK_COMMAND,
	      mh_folder[current_folder].folder_name,
	      mh_unseen_seq, article_list[current_article].real_number);
#endif	/* !(MSDOS || OS2) */
      if (system(buff)) {
	print_fatal("Can't execute mark command.");
      }
      article_list[current_article].mark |= READ_MARK;
      mark &= ~READ_MARK;
    }
  }
  if (!mh_mark_mode) {
    article_list[current_article].mark |= mark;
    return;
  }
  if (mark & DELETE_MARK) {
    article_list[current_article].mark |= DELETE_MARK;
    mark &= ~DELETE_MARK;
  }
  if (mark & FORWARD_MARK) {
    if (article_list[current_article].mark & FORWARD_MARK) {
      mark &= ~FORWARD_MARK;
    }
  }
  if (mark & ANSWER_MARK) {
    if (article_list[current_article].mark & ANSWER_MARK) {
      mark &= ~ANSWER_MARK;
    }
  }
  if (!mark) {
    return;
  }
  sprintf(mh_file, "%s%c%s%c%d", mh_path, SLASH_CHAR, select_name, SLASH_CHAR,
	  article_list[current_article].real_number);
  create_backup_name(tmp_file, mh_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(tmp_file);
#endif	/* (MSDOS || OS2) */
  if (rename(mh_file, tmp_file)) {
    if (errno != ENOENT) {
      print_fatal("Can't backup MH article.");
    }
    return;
  }
  fp1 = fopen(tmp_file, "r");
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't open MH read article.");
    return;
  }
  fp2 = fopen(mh_file, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't open MH write article.");
    fclose(fp1);
#if	defined(MSDOS) || defined(OS2)
    unlink(mh_file);
#endif	/* (MSDOS || OS2) */
    rename(tmp_file, mh_file);
    return;
  }
  if (stat(tmp_file, &stat_buff)) {
    stat_buff.st_mode = 0;
  }
  chmod(mh_file, ((stat_buff.st_mode & 01777) | mh_m_protect));
  article_list[current_article].mark |= mark;
  if (mark & READ_MARK) {
    fprintf(fp2, "%s \n", MH_READ_FIELD);
  }
  if (mark & FORWARD_MARK) {
    fprintf(fp2, "%s \n", MH_FORWARD_FIELD);
  }
  if (mark & ANSWER_MARK) {
    fprintf(fp2, "%s \n", MH_REPLY_FIELD);
  }
  while (fgets(buff, sizeof(buff), fp1)) {
    fputs(buff, fp2);
  }
  fclose(fp1);
  status = fflush(fp2);
  fclose(fp2);
  if (status) {
    print_fatal("Can't update MH article.");
    rename(tmp_file, mh_file);
  } else {
    unlink(tmp_file);
  }
}

/*
 * �����ޡ������
 */

static void	mh_unmark(current_folder, current_article, mark)
     int	current_folder;
     int	current_article;
     int	mark;
{
  FILE		*fp1, *fp2;
  struct stat	stat_buff;
  char		mh_file[PATH_BUFF];
  char		tmp_file[PATH_BUFF];
  char		dummy_buff[MAX_FIELD_LEN];
  char		buff[BUFF_SIZE];
  int		status;

  if (article_list[current_article].mark & CANCEL_MARK) {
    return;
  }
  mark &= (READ_MARK | FORWARD_MARK | ANSWER_MARK | DELETE_MARK);
  if (mark & READ_MARK) {
    if (!(article_list[current_article].mark & READ_MARK)) {
      mark &= ~READ_MARK;
    } else if (mh_unseen_seq[0]) {
#if	defined(MSDOS) || defined(OS2)
      sprintf(buff, "%s +%s -sequence %s -add %d > NUL",
	      MH_MARK_COMMAND, mh_folder[current_folder].folder_name,
	      mh_unseen_seq, article_list[current_article].real_number);
#else	/* !(MSDOS || OS2) */
      sprintf(buff, "%s/%s +%s -sequence %s -add %d > /dev/null 2>&1",
	      mh_command_path, MH_MARK_COMMAND,
	      mh_folder[current_folder].folder_name,
	      mh_unseen_seq, article_list[current_article].real_number);
#endif	/* !(MSDOS || OS2) */
      if (system(buff)) {
	print_fatal("Can't execute mark command.");
      }
    }
  }
  if (!mh_mark_mode) {
    article_list[current_article].mark &= ~mark;
    return;
  }
  if (mark & DELETE_MARK) {
    article_list[current_article].mark &= ~DELETE_MARK;
    mark &= ~DELETE_MARK;
  }
  if (mark & FORWARD_MARK) {
    if (!(article_list[current_article].mark & FORWARD_MARK)) {
      mark &= ~FORWARD_MARK;
    }
  }
  if (mark & ANSWER_MARK) {
    if (!(article_list[current_article].mark & ANSWER_MARK)) {
      mark &= ~ANSWER_MARK;
    }
  }
  if (!mark) {
    return;
  }
  sprintf(mh_file, "%s%c%s%c%d", mh_path, SLASH_CHAR, select_name, SLASH_CHAR,
	  article_list[current_article].real_number);
  create_backup_name(tmp_file, mh_file);
  if (rename(mh_file, tmp_file)) {
    if (errno != ENOENT) {
      print_fatal("Can't rename MH article.");
    }
    return;
  }
  fp1 = fopen(tmp_file, "r");
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't open MH read article.");
    return;
  }
  fp2 = fopen(mh_file, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't open MH write article.");
    fclose(fp1);
    rename(tmp_file, mh_file);
    return;
  }
  if (stat(tmp_file, &stat_buff)) {
    stat_buff.st_mode = 0;
  }
  chmod(mh_file, ((stat_buff.st_mode & 01777) | mh_m_protect));
  article_list[current_article].mark &= ~mark;
  while (fgets(buff, sizeof(buff), fp1)) {
    if ((!buff[0]) || (buff[0] == 'n')) {
      mark = 0;
    }
    if (copy_field(buff, dummy_buff, MH_READ_FIELD)) {
      if (mark & READ_MARK) {
	continue;
      }
    } else if (copy_field(buff, dummy_buff, MH_FORWARD_FIELD)) {
      if (mark & FORWARD_MARK) {
	continue;
      }
    } else if (copy_field(buff, dummy_buff, MH_REPLY_FIELD)) {
      if (mark & ANSWER_MARK) {
	continue;
      }
    }
    fputs(buff, fp2);
  }
  fclose(fp1);
  status = fclose(fp2);
  if (status) {
    print_fatal("Can't update MH article.");
    rename(tmp_file, mh_file);
  } else {
    unlink(tmp_file);
  }
}

/*
 * �����ȵ���������¸
 */

static void	mh_save_current(current_folder)
     int	current_folder;
{
  char	buff[BUFF_SIZE];

  if ((current_folder < 0) || (current_folder >= mh_folder_number)) {
    return;
  }
  if (mh_article_number <= mh_folder[current_folder].current_article) {
    return;
  }
  mh_folder[current_folder].current_article =
    article_list[mh_folder[current_folder].current_article].real_number;
  sprintf(buff, "%d", mh_folder[current_folder].current_article);
  mh_exec_command(MH_FOLDER_COMMAND, mh_folder[current_folder].folder_name,
		  buff);
}

/*
 * �����ԥå�
 */

static void	mh_pick(current_folder, pattern)
     int	current_folder;
     char	*pattern;
{
  FILE	*fp;
  char	buff[BUFF_SIZE];
  int	i, j;

  print_mode_line(japanese ? "�ԥå���Ǥ���" : "Picking.");
#if	defined(MSDOS) || defined(OS2)
  sprintf(buff, "%s +%s %s", MH_PICK_COMMAND,
	  mh_folder[current_folder].folder_name, pattern);
#else	/* !(MSDOS || OS2) */
  sprintf(buff, "exec %s%c%s +%s %s 2> /dev/null", mh_command_path,
	  SLASH_CHAR, MH_PICK_COMMAND,
	  mh_folder[current_folder].folder_name, pattern);
#endif	/* !(MSDOS || OS2) */
  fp = popen(buff, "r");
  if (fp != (FILE*)NULL) {
    while (fgets(buff, sizeof(buff), fp)) {
      if ((j = atoi(buff)) > 0) {
	for (i = 0; i < mh_article_number; i++) {
	  if (article_list[i].real_number == j) {
	    article_list[i].mark |= DELETE_MARK;
	    break;
	  }
	}
      }
    }
    pclose(fp);
  } else {
    print_fatal("Can't execute pick command.");
  }
}

/*
 * �������������
 */

static void	mh_count(current_folder, mode)
     int	current_folder;
     int	mode;	/* 0:�Ƶ����ʤ�,0�ʳ�:�Ƶ� */
{
  /* �ѿ��ϺƵ�����Τ� static �ˤ��ʤ����� */

  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		buff3[BUFF_SIZE];
  char		*ptr, *ptr2;
  struct stat	stat_buff;
#ifdef	HAVE_FILES
  FILES_STRUCT	dp;
#else	/* !HAVE_FILES */
  DIR_PTR	*dp;
  DIR		*dir_ptr;
#endif	/* !HAVE_FILES */
  int		current_article;
  int		top_flag;
  register int	max_number;
  register int	min_number;
  register int	i, j;

  current_article = max_number = min_number = 0;
  if (mh_folder[current_folder].folder_name[0]) {
    top_flag = 0;
    sprintf(buff1, "%s%c%s", mh_path, SLASH_CHAR,
	    mh_folder[current_folder].folder_name);
  } else {
    top_flag = 1;
    strcpy(buff1, mh_path);
    current_article = -1;
  }
  sprintf(buff2, "%s%c%s", buff1, SLASH_CHAR, MH_SEQ_FILE);
  if (mh_folder[current_folder].folder_name[0] &&
      mh_read_field(buff2, MH_CURRENT_FIELD, buff3)) {
    i = atoi(buff3);
  } else {
    i = current_article;
  }
  strcpy(buff2, buff1);
  ptr = buff2;
  while (*ptr) {
    ptr++;
  }
  *ptr++ = SLASH_CHAR;
#ifdef	HAVE_FILES
  sprintf(buff3, "%s%c*.*", buff1, SLASH_CHAR);
  if (!dos_files(buff3, &dp, FILE_ATTR | DIR_ATTR)) {
    current_article = i;
    do {
#ifdef	__TURBOC__
      strcpy(ptr, dp.ff_name);
#else	/* !__TURBOC__ */
      strcpy(ptr, dp.name);
#endif	/* !__TURBOC__ */
#else	/* !HAVE_FILES */
  if ((dir_ptr = opendir(buff1)) != (DIR*)NULL) {
    current_article = i;
    while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
      strcpy(ptr, dp->d_name);
#endif	/* !HAVE_FILES */
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
      if (i && mode) {
	if (!stat(buff2, &stat_buff)) {
	  if (stat_buff.st_mode & S_IFDIR) {
	    if (strcmp(ptr, ".") && strcmp(ptr, "..")) {
	      if (current_article >= 0) {
		sprintf(buff3, "%s%c%s", mh_folder[current_folder].folder_name,
			SLASH_CHAR, ptr);
	      } else {
		strcpy(buff3, ptr);
	      }
#ifdef	USE_NLINK
	      mh_search_folder(buff3, stat_buff.st_nlink > 2);
#else	/* !USE_NLINK */
	      mh_search_folder(buff3, 1);
#endif	/* !USE_NLINK */
	    }
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
  if (current_article >= 0) {
    mh_folder[current_folder].min_article = min_number;
    mh_folder[current_folder].max_article = max_number;
    mh_folder[current_folder].current_article = current_article;
  }
}

/*
 * �����ȵ����ֹ��Ѵ�
 */

static int	mh_trans_current(real_number)
     int	real_number;
{
  register int	current_article;
  register int	i;

  current_article = 0;
  for (i = 0; i < mh_article_number; i++) {
    current_article = i;
    if (article_list[current_article].real_number >= real_number) {
      break;
    }
  }
  return(current_article);
}

/*
 * ���ե��������
 */

static void	mh_search_all_folder()
{
  char		*ptr;
  register int	i;

  mh_folder_number = mh_alloc_number = 0;
  if (mh_folder != (struct mh_folder*)NULL) {
    free(mh_folder);
    mh_folder = (struct mh_folder*)NULL;
  }
  mh_search_folder("", 1);
  if (mh_folder_number > 0) {
    for (i = 0; i < mh_folder_number; i++) {	/* ��������������	*/
      ptr = mh_folder[i].folder_name;
      while (*ptr) {
	if (*ptr == FOLDER_SEPARATER) {
	  *ptr++ = ' ';
	} else {
	  ptr++;
	}
      }
    }
    qsort(mh_folder, mh_folder_number, sizeof(struct mh_folder),
	  (int (*)())strcmp);
    for (i = 0; i < mh_folder_number; i++) {	/* �����������θ����	*/
      ptr = mh_folder[i].folder_name;
      while (*ptr) {
	if (*ptr == ' ') {
	  *ptr++ = FOLDER_SEPARATER;
	} else {
	  ptr++;
	}
      }
    }
  }
}

/*
 * �ե��������
 */

static void	mh_search_folder(folder_name, mode)
     char	*folder_name;
     int	mode;	/* 0:�Ƶ����ʤ�,0�ʳ�:�Ƶ� */
{
  struct mh_folder	*mh_alloc_ptr;

  if (mh_alloc_number <= mh_folder_number) {
    if (mh_folder) {
      mh_alloc_ptr = (struct mh_folder*)
	realloc(mh_folder, (mh_alloc_number + MH_ALLOC_COUNT)
		* sizeof(struct mh_folder));
    } else {
      mh_alloc_ptr = (struct mh_folder*)
	malloc((mh_alloc_number + MH_ALLOC_COUNT)
	       * sizeof(struct mh_folder));
    }
    if (mh_alloc_ptr == (struct mh_folder*)NULL) {
      print_fatal("Can't allocate memory for MH folder.");
      return;
    }
    mh_folder = mh_alloc_ptr;
    mh_alloc_number += MH_ALLOC_COUNT;
  }
  if (*folder_name) {
    strcpy(mh_folder[mh_folder_number].folder_name, folder_name);
    mh_count(mh_folder_number++, mode);
  } else {
    mh_folder[0].folder_name[0] = '\0';
    mh_count(0, mode);
  }
}

/*
 * �ޥå����륰�롼�׸���
 */

static void	mh_match_folder(folder_name)
     char	*folder_name;
{
  int		length1, length2;
  register int	i, j, k, l;
  int		max, min;
  int		match;
  char		*ptr;

  mh_select_number = 0;
  if (mh_selected_folder) {
    free(mh_selected_folder);
    mh_selected_folder = (struct mh_folder*)NULL;
  }
  if (!mh_select_number) {
    mh_selected_folder = (struct mh_folder*)
      malloc(mh_folder_number * sizeof(struct mh_folder));
    if (mh_selected_folder == (struct mh_folder*)NULL) {
      print_fatal("Can't allocate memory for MH select folder.");
      return;
    }
  }
  if (gnus_mode) {
    for (i = 0; i < mh_folder_number; i++) {
      strcpy(mh_selected_folder[mh_select_number].folder_name,
	     mh_folder[i].folder_name);
      mh_selected_folder[mh_select_number].max_article = mh_folder[i].max_article;
      mh_selected_folder[mh_select_number].min_article = mh_folder[i].min_article;
      mh_select_number++;
    }
  } else {

    /*
     * ����(vin like)�⡼�ɤν���
     * �������鲼�ν����� mnews �������ܤ����򤬺������ʬ�Ǥ���
     * �����ν�������ñ������Ǥ���ͤ���º�ɤ��ޤ���
     */

    j = l = 0;
    length1 = strlen(folder_name);
    for (i = 0; i < mh_folder_number; i++) {
      if ((!strncmp(mh_folder[i].folder_name, folder_name, length1)) &&
	  ((!folder_name[0]) ||
	   (!mh_folder[i].folder_name[length1]) ||
	   (mh_folder[i].folder_name[length1] == FOLDER_SEPARATER))) {
	match = 1;
	ptr = &mh_folder[i].folder_name[length1];
	if (*ptr) {
	  k = 0;
	  if (*ptr == FOLDER_SEPARATER) {
	    ptr++;
	    k++;
	  }
	  while (*ptr) {
	    if (*ptr == FOLDER_SEPARATER) {
	      break;
	    }
	    ptr++;
	    k++;
	  }
	  k += length1;
	  for (; j < i; j++) {
	    if (!strncmp(mh_folder[i].folder_name, mh_folder[j].folder_name,
			 k)) {
	      match = 0;
	      break;
	    }
	  }
	} else {
	  k = length1;
	}
	if (match) {
	  max = min = -1;
	  length2 = k;
	  k = 0;
	  for (; l < mh_folder_number; l++) {
	    if ((!strncmp(mh_folder[i].folder_name, mh_folder[l].folder_name,
			  length2)) &&
		((!mh_folder[l].folder_name[length2]) ||
		 (mh_folder[l].folder_name[length2] == FOLDER_SEPARATER))) {
	      if (length2 == strlen(mh_folder[l].folder_name)) {
		if (l + 1 >= mh_folder_number) {
		  k++;
		} else if ((strncmp(mh_folder[l].folder_name,
				    mh_folder[l + 1].folder_name,
				    strlen(mh_folder[l].folder_name))) ||
			   (mh_folder[l + 1].folder_name[strlen(mh_folder[l].folder_name)]
			    != FOLDER_SEPARATER)) {
		  k++;
		} else if (strlen(folder_name)
			   == strlen(mh_folder[l].folder_name)) {
		  k++;
		}
		if (k == 1) {
		  max = mh_folder[l].max_article;
		  min = mh_folder[l].min_article;
		  break;
		}
	      }
	      k = 1;
	    } else if (k) {
	      break;
	    }
	  }
#ifdef	DEBUG
	  printf("MATCH %04d:%s\n", i, mh_folder[i].folder_name);
#endif	/* DEBUG */
	  strcpy(mh_selected_folder[mh_select_number].folder_name,
		 mh_folder[i].folder_name);
	  mh_selected_folder[mh_select_number].folder_name[length2] = '\0';
	  mh_selected_folder[mh_select_number].max_article = max;
	  mh_selected_folder[mh_select_number].min_article = min;
	  mh_select_number++;
	}
      }
    }
  }
}

/*
 * �����ꥹ�ȼ���
 */

static int	mh_get_list(current_folder)
     int	current_folder;
{
  char		buff1[PATH_BUFF];
  char		buff2[BUFF_SIZE];
  int		min_number, max_number;
  char		*ptr;
  register int	i;

  min_number = mh_folder[current_folder].min_article;
  mh_article_number = mh_folder[current_folder].max_article - min_number + 1;
  multi_number = 0;
  if ((mh_article_number <= 0) || (min_number <= 0)) {
    mh_article_number = 0;
    return(0);
#ifdef	MH_MAX_ARTICLE
  } else if (mh_article_number > MH_MAX_ARTICLE) {
    mh_article_number = MH_MAX_ARTICLE;
    min_number = mh_folder[current_folder].max_article - mh_article_number + 1;
#endif	/* MH_MAX_ARTICLE */
  }
  if (article_list != (ARTICLE_LIST*)NULL) {
    free(article_list);
  }
  article_list = (ARTICLE_LIST*)
    malloc(sizeof(ARTICLE_LIST) * mh_article_number);
  if (article_list == (ARTICLE_LIST*)NULL) {
    print_fatal("Can't allocate memory for MH article struct.");
    mh_article_number = 0;
    return(1);
  }
  for (i = 0; i < mh_article_number; i++) {
    article_list[i].real_number = min_number + i;
    article_list[i].mark = UNFETCH_MARK;	/* �إå�̤����	*/
  }

  /*	���ɥޡ�������		*/

  if (mh_unseen_seq[0]) {
    for (i = 0; i < mh_article_number; i++) {
      article_list[i].mark |= READ_MARK;	/* ����		*/
    }
    sprintf(buff1, "%s%c%s%c%s", mh_path, SLASH_CHAR,
	    mh_folder[current_folder].folder_name, SLASH_CHAR, MH_SEQ_FILE);
    if (mh_read_field(buff1, mh_unseen_seq, buff2)) {
      ptr = buff2;
      if (*ptr == ':') {
	ptr++;
	while (*ptr) {
	  if ((*ptr == ' ') || (*ptr == '\t')) {
	    ptr++;
	  } else {
	    if (!(min_number = atoi(ptr))) {
	      break;
	    }
	    while (isdigit(*ptr)) {
	      ptr++;
	    }
	    if (*ptr == '-') {
	      ptr++;
	      if (!(max_number = atoi(ptr))) {
		break;
	      }
	      while (isdigit(*ptr)) {
		ptr++;
	      }
	    } else {
	      max_number = min_number;
	    }
	    for (i = 0; i < mh_article_number; i++) {
	      if ((article_list[i].real_number >= min_number) &&
		  (article_list[i].real_number <= max_number)) {
		article_list[i].mark &= ~READ_MARK;
	      }
	    }
	  }
	}
      }
    }
  }

  /*	�����Ƚ���	*/

#ifdef	REF_SORT
  if (message_list != (MESSAGE_LIST*)NULL) {
    print_fatal("Unexpected message list allocation found.");
    free(message_list);
    message_list = (MESSAGE_LIST*)NULL;
  }
  if (mh_article_number && (sort_rule == 1)) {
    message_list = (MESSAGE_LIST*)malloc(sizeof(MESSAGE_LIST)
					 * mh_article_number);
  }
#endif	/* REF_SORT */
  if (sort_articles(current_folder, &mh_article_number,
		    mail_thread_mode, mail_article_mask,
		    mh_get_field, (void (*)())NULL) < 0) {
    return(-1);
  }

  /*	�ѥå�����	*/

  if (mail_article_mask) {
    pack_articles(current_folder, &mh_article_number, mh_get_field,
		  (void (*)())NULL);
  }
  return(0);
}

/*
 * �����إå�����
 */

static int	mh_get_field(current_folder, current_article)
     int	current_folder;
     int	current_article;		/*	�����̤��ֹ�	*/
{
  FILE			*fp;
  char			tmp_file[PATH_BUFF];
  static char		from_buff[BUFF_SIZE];
  static char		to_buff[BUFF_SIZE];
  static char		date_buff[MAX_FIELD_LEN];
  static char		subject_buff[BUFF_SIZE];
  static char		x_nsubj_buff[MAX_FIELD_LEN];
  static char		mh_read_buff[2];
  static char		mh_reply_buff[2];
  static char		mh_forward_buff[2];
#ifdef	REF_SORT
  static char		message_buff[MAX_FIELD_LEN];
  static char		reference_buff[BUFF_SIZE];
  static char		in_reply_buff[BUFF_SIZE];
#endif	/* REF_SORT */
  static struct cpy_hdr	mh_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
#ifdef	LARGE
    {TO_FIELD,		to_buff,	sizeof(to_buff)},
#endif	/* LARGE */
    {DATE_FIELD,	date_buff,	sizeof(date_buff)},
    {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
    {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
    {MH_READ_FIELD,	mh_read_buff,	sizeof(mh_read_buff)},
    {MH_REPLY_FIELD,	mh_reply_buff,	sizeof(mh_reply_buff)},
    {MH_FORWARD_FIELD,	mh_forward_buff,sizeof(mh_forward_buff)},
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

  if ((current_article < 0) || (current_article >= mh_article_number)) {
    return(1);
  }
  article_list[current_article].mark &= ~UNFETCH_MARK;
  sprintf(tmp_file, "%s%c%s%c%d", mh_path, SLASH_CHAR, select_name, SLASH_CHAR,
	  article_list[current_article].real_number);
  fp = fopen(tmp_file, "r");
  if (fp != (FILE*)NULL) {
    article_list[current_article].lines =
      copy_fields(fp, mh_fields, sizeof(mh_fields) / sizeof(struct cpy_hdr),
		   CF_CLR_MASK | CF_GET_MASK | CF_CNT_MASK | CF_SPC_MASK);
    fclose(fp);
  } else {
    article_list[current_article].mark |= CANCEL_MARK;
    article_list[current_article].mark &= ~READ_MARK;
    return(1);
  }
  if (mh_read_buff[0]) {
    article_list[current_article].mark |= READ_MARK;
  }
  if (mh_reply_buff[0]) {
    article_list[current_article].mark |= ANSWER_MARK;
  }
  if (mh_forward_buff[0]) {
    article_list[current_article].mark |= FORWARD_MARK;
  }
  get_real_adrs(from_buff, article_list[current_article].from);
  if (my_adrs_mode) {
    if (check_my_adrs(article_list[current_article].from)) {
      article_list[current_article].from[0] = MY_ADRS_CHAR;
      get_real_adrs(to_buff, &article_list[current_article].from[1]);
    }
  }
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
      strcpy(message_list[current_article].ref_id, ptr);
    } else {
      ptr = strrchr(in_reply_buff, '<');
      if (ptr != (char*)NULL) {
	strcpy(message_list[current_article].ref_id, ptr);
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
 * �ե��������
 */

int	mh_create_folder(folder_name)
     char	*folder_name;
{
#ifdef	USE_MH
  if (mh_exec_command(MH_FOLDER_COMMAND,
		      folder_name, "-create")) {
    term_bell();
    print_mode_line(japanese ? "�ե�����������Ǥ��ޤ���Ǥ�����" :
		    "Can't create folder.");
    sleep(ERROR_SLEEP);
    return(1);
  }
#else	/* !USE_MH */
  if (create_folder(folder_name, mh_f_protect)) {
    term_bell();
    print_mode_line(japanese ? "�ե�����������Ǥ��ޤ���Ǥ�����" :
		    "Can't create folder.");
    sleep(ERROR_SLEEP);
    return(1);
  }
#endif	/* !USE_MH */
  return(0);
}

/*
 * �ե�������
 */

static int	mh_delete_folder(folder_name)
     char	*folder_name;
{
  char	buff[BUFF_SIZE];

  sprintf(buff, "%s%c%s%c%s", mh_path, SLASH_CHAR, folder_name,
	  SLASH_CHAR, MH_SEQ_FILE);
  unlink(buff);
  sprintf(buff, "%s%c%s", mh_path, SLASH_CHAR, folder_name);
  if (!rmdir(buff)) {
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

static int	mh_adjust_folder(folder_name)
     char	*folder_name;
{
  int	current_folder;
  int	i;

  current_folder = 0;
  for (i = 0; i < mh_select_number; i++) {
    if (strnamecmp(mh_selected_folder[i].folder_name, folder_name) > 0) {
      break;
    }
    current_folder = i;
  }
  return(current_folder);
}

/*
 * �ե�����ɼ���(ʣ��)
 */

static int	mh_read_fields(file_name, field_ptr)
     char		*file_name;
     struct mh_field	*field_ptr;
{
  FILE			*fp;
  char			buff[BUFF_SIZE];
  char			*ptr1, *ptr2, *ptr3;
  struct mh_field	*field_ptr2;
  int			status;
  register int		i;

  /*
   * �ե�����ɥХåե������
   */

  status = 0;
  field_ptr2 = field_ptr;
  while (field_ptr2->field_name != (char*)NULL) {
    if (field_ptr2->field_default != (char*)NULL) {
      strcpy(field_ptr2->field_buff, field_ptr2->field_default);
    } else {
      field_ptr2->field_buff[0] = '\0';
    }
    field_ptr2++;
  }

  /*
   * �ե�����ɥ꡼��
   */

  fp = fopen(file_name, "r");
  if (fp != (FILE*)NULL) {
    while (fgets2(buff, sizeof(buff), fp)) {
      field_ptr2 = field_ptr;
      i = 0;
      while (field_ptr2->field_name != (char*)NULL) {
	if (!strncasecmp(buff, field_ptr2->field_name,
			 strlen(field_ptr2->field_name))) {
	  ptr1 = field_ptr2->field_buff;
	  ptr2 = &buff[strlen(field_ptr2->field_name)];
	  while ((*ptr2 == ' ') || (*ptr2 == '\t')) {
	    ptr2++;
	  }
	  while (*ptr2) {
	    if ((*ptr2 == ' ') || (*ptr2 == '\t')) {

	      /* ������;ʬ�ʶ��򤬤�����ϼ����� */

	      ptr3 = ptr2;
	      while ((*ptr3 == ' ') || (*ptr3 == '\t')) {
		ptr3++;
	      }
	      if ((!*ptr3) || (*ptr3 == '\n')) {
		break;
	      }
	    }
	    if (*ptr2 == '\n') {
	      break;
	    }
	    *ptr1++ = *ptr2++;
	  }
	  status |= (1 << i);
	  *ptr1 = '\0';
	  break;
	}
	field_ptr2++;
	i++;
      }
    }
    fclose(fp);
  }
  return(status);
}

/*
 * �ե�����ɼ���(ñ��)
 */

static int	mh_read_field(file_name, field, ptr)
     char	*file_name;
     char	*field;
     char	*ptr;
{
  static struct mh_field	read_field[] = {{NULL, NULL, NULL},
						  {NULL, NULL, NULL}};

  read_field[0].field_name    = field;
  read_field[0].field_buff    = ptr;
  read_field[0].field_default = (char*)NULL;
  return(mh_read_fields(file_name, read_field));
}

/*
 * MH �ե�����ص���������
 */

int	save_mh_folder(current_article, extract)
     int	current_article;
     int	(*extract)();
{
  char		buff1[SMALL_BUFF];
  char		buff2[SMALL_BUFF];
  char		buff3[SMALL_BUFF];
  char		*ptr, *ptr2;
  KANJICODE	tmp_code;
  struct stat	stat_buff;
#ifdef	HAVE_FILES
  FILES_STRUCT	dp;
#else	/* !HAVE_FILES */
  DIR_PTR	*dp;
  DIR		*dir_ptr;
#endif	/* !HAVE_FILES */
  int		line, total;
  int		max_number;
  register int	i, j;
  int		status;

  max_number = 0;
  strcpy(buff1, mh_path);
  input_line(INPUT_SPCCUT_MASK | INPUT_FOLDER_MASK,
	     "�����֤���ե����̾�����Ϥ��Ʋ�����:",
	     "Input save folder name:", buff1);
  if (!buff1[0]) {
    return(1);
  }
  sprintf(buff2, "%s%c%s", mh_path, SLASH_CHAR, buff1);
  if (stat(buff2, &stat_buff)) {
    if (!yes_or_no(NORMAL_YN_MODE,
		   "�ե����(%s)�򿷵��������ޤ���?",
		   "Create folder(%s).Are you sure?",
		   buff1)) {
      return(1);
    }
    if (mh_create_folder(buff1)) {
      return(1);
    }
  }
#ifdef	HAVE_FILES
  sprintf(buff3, "%s%c*.*", buff2, SLASH_CHAR);
  if (!dos_files(buff3, &dp, FILE_ATTR)) {
    ptr = buff2;
    while (*ptr) {
      ptr++;
    }
    *ptr++ = SLASH_CHAR;
    do {
#ifdef	__TURBOC__
      strcpy(ptr, dp.ff_name);
#else	/* !__TURBOC__ */
      strcpy(ptr, dp.name);
#endif	/* !__TURBOC__ */
#else	/* !HAVE_FILES */
  if ((dir_ptr = opendir(buff2)) != (DIR*)NULL) {
    strcpy(buff3, buff2);
    ptr = buff3;
    while (*ptr) {
      ptr++;
    }
    *ptr++ = SLASH_CHAR;
    while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
      strcpy(ptr, dp->d_name);
#endif	/* !HAVE_FILES */
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
	if (max_number < i) {
	  max_number = i;
	}
      }
#ifdef	HAVE_FILES
    } while (!dos_nfiles(&dp));
    *--ptr = '\0';
#else	/* !HAVE_FILES */
    }
    closedir(dir_ptr);
#endif	/* !HAVE_FILES */
  } else {
    if (yes_or_no(NORMAL_YN_MODE, "�ե����(%s)�򿷵��������ޤ���?",
		  "Create folder(%s).Are you sure?",
		  buff1)) {
      if (mh_create_folder(buff1)) {
	return(1);
      }
    } else {
      return(1);
    }
  }
  j = unixfrom_mode;
  unixfrom_mode = 0;		/* ����˴ؤ�餺 MH �ե����ޥåȤ���� */
  tmp_code = save_code;
  save_code = ASCII_CODE;	/* ����˴ؤ�餺̵�Ѵ������ */
  status = 0;
  if (multi_number) {
    total = line = 0;
    for (i = 0; i < multi_number; i++) {
      total += (int)article_list[multi_list[i]].lines;
    }
    for (i = 0; i < multi_number; i++) {
      print_mode_line(japanese ?
		      "[%d/%d]-(%d/%d) ��������Ǥ���" :
		      "[%d/%d]-(%d/%d) Saving.",
		      i + 1, multi_number, line, total);
      sprintf(buff1, "%s%c%d", buff2, SLASH_CHAR, ++max_number);
      if (save_article(multi_list[i], extract, buff1)) {
	status = 1;
      } else {
	chmod(buff1, mh_m_protect);
      }
      line += (int)article_list[multi_list[i]].lines;
    }
  } else {
    sprintf(buff1, "%s%c%d", buff2, SLASH_CHAR, max_number + 1);
    status = save_article(current_article, extract, buff1);
    if (!status) {
      chmod(buff1, mh_m_protect);
    }
  }
  unixfrom_mode = j;
  save_code = tmp_code;
  if (status) {
    print_mode_line(japanese ? "�����������֤Ǥ��ޤ���Ǥ�����" :
		    "Save article failed.");
    term_bell();
    sleep(ERROR_SLEEP);
  }
  return(status);
}

/*
 * MH ���ޥ�ɼ¹�
 */

static int	mh_exec_command(command, folder_name, args)
     char	*command;
     char	*folder_name;
     char	*args;
{
  char		buff[BUFF_SIZE];

  if (!args) {
    args = "";
  }
#if	defined(MSDOS) || defined(OS2)
  sprintf(buff, "%s +%s %s > NUL", command, folder_name, args);
#else	/* !(MSDOS || OS2) */
  sprintf(buff, "exec %s%c%s +%s %s > /dev/null 2>&1", mh_command_path,
	  SLASH_CHAR, command, folder_name, args);
#endif	/* !(MSDOS || OS2) */
#ifdef	DEBUG
  printf("EXEC \"%s\"", buff);
  sleep(10);
#else	/* !DEBUG */
 if (system(buff)) {
    print_fatal("Can't execute %s command.", command);
    return(1);
  }
#endif	/* !DEBUG */
  return(0);
}

#ifndef	SMALL
/*
 * �ե����̾����(ʣ��)
 */

static int	mh_search_name(mode, max_number, folder_ptr)
     int	mode;
     int	max_number;
     int	*folder_ptr;
{
  register int	current_folder;
  char		*str;

  str = input_search_string(mode);
  if (str != (char*)NULL) {
    if (mode) {
      for (current_folder = *folder_ptr - 1; current_folder >= 0;
	   current_folder--) {
	if (strindex(mh_selected_folder[current_folder].folder_name, str)) {
	  print_mode_line(japanese ? "���Ĥ��ޤ�����" : "Search succeed.");
	  *folder_ptr = current_folder;
	  return(0);
	}
      }
    } else {
      for (current_folder = *folder_ptr + 1; current_folder < max_number;
	   current_folder++) {
	if (strindex(mh_selected_folder[current_folder].folder_name, str)) {
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

#ifdef	MAILSEND
/*
 * MH �����ꥢ��Ÿ��
 */

int	mh_expand_alias(recipient, alias, original, number)
     char	*recipient;
     char	*alias;
     char	*original;
     int	number;
{
  FILE	*fp;
  char	buff[ALIAS_BUFF];
  char	*ptr1, *ptr2;
  int	status;

  strcpy(recipient, alias);
#if	defined(MSDOS) || defined(OS2)
  sprintf(buff, "%s %s", MH_ALI_COMMAND, alias);
#else	/* !(MSDOS || OS2) */
  sprintf(buff, "exec %s%c%s %s", mh_command_path, SLASH_CHAR,
	  MH_ALI_COMMAND, alias);
#endif	/* !(MSDOS || OS2) */
  status = 1;
#ifdef	SIGPIPE
  signal(SIGPIPE, pipe_error);
#endif	/* SIGPIPE */
  fp = popen(buff, "r");
  if (fp != (FILE*)NULL) {
    ptr1 = recipient;
    while (fgets(buff, sizeof(buff), fp)) {
      ptr2 = buff;
      status = 0;
      while (*ptr2 >= ' ') {
	if (--number <= 0) {
	  status = 1;
	  break;
	}
	*ptr1++ = *ptr2++;
      }
      *ptr1 = '\0';
    }
    if (pclose(fp)) {
      status = 1;
    }
  } else {
    status = 0;
  }
#ifdef	SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif	/* SIGPIPE */
  if (!strcmp(recipient, alias)) {
    strcpy(recipient, original);
  }
  return(status);
}
#endif	/* MAILSEND */

/*
 * MH ����᡼������å�
 */

int	mh_chkmsg()
{
  char		buff[SMALL_BUFF];

#if	defined(MSDOS) || defined(OS2)
  sprintf(buff, "%s > NUL", MH_MSGCHK_COMMAND);
#else	/* !(MSDOS || OS2) */
  sprintf(buff, "exec %s%c%s > /dev/null 2>&1",
	  mh_command_path, SLASH_CHAR, MH_MSGCHK_COMMAND);
#endif	/* !(MSDOS || OS2) */
  return(!system(buff));
}

#ifndef	USE_MH
/*
 * �ե��������
 * ����:�ܴؿ��ϺƵ��������롣
 */

static int	create_folder(folder_name, perm)
     char	*folder_name;
     int	perm;
{
  struct stat	stat_buff;
  char		buff1[PATH_BUFF];
  char		buff2[PATH_BUFF];
  char		*ptr;
  int		status;

  sprintf(buff1, "%s/%s", mh_path, folder_name);
  if (stat(buff1, &stat_buff)) {
    status = 0;
    strcpy(buff2, folder_name);
    ptr = separater(buff2);
    if (ptr != (char*)NULL) {
      *ptr = '\0';
      status = create_folder(buff2, perm);
    }
    if (!status) {
      status = mkdir(buff1, perm);
    }
    return(status);
  }
  return(0);
}
#endif	/* !USE_MH */
#endif	/* MH */
