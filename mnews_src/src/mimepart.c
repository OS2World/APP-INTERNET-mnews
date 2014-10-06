/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1996-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : MIME multipart select routine
 *  File        : mimepart.c
 *  Version     : 1.21
 *  First Edit  : 1996-12/08
 *  Last  Edit  : 1997-11/30
 *  Author      : MSR24 ��� ��
 *
 */

#ifdef	MIME
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
#include	"newspost.h"
#include	"mailsend.h"
#include	"mimepart.h"

#define	MIME_MULTIMEDIA

static int	mime_save_file();	/* MIME �ե����륻����		*/
static void	mime_redraw_part();	/* �ѡ���������̺�����		*/
static int	mime_select_article();	/* �ѡ�������			*/
static int	mime_read();		/* �ѡ��Ȼ���			*/
static int	mime_extract();		/* �ѡ������			*/
static void	mime_mark();		/* �ѡ��ȥޡ����ɲ�		*/
static void	mime_unmark();		/* �ѡ��ȥޡ������		*/
static int	mime_prev_part();	/* ���ѡ����ֹ����		*/
static int	mime_next_part();	/* ���ѡ����ֹ����		*/
static int	mime_prev_unread_part();/* ��̤�ɥѡ����ֹ����		*/
static int	mime_next_unread_part();/* ��̤�ɥѡ����ֹ����		*/
static int	mime_get_list();	/* �ѡ��ȥꥹ�ȼ���		*/

static char	*mime_file = (char*)NULL;
					/* MIME �����ե�����		*/
static char	*mime_boundary = (char*)NULL;
					/* MIME ����ʸ����		*/
static char	mime_subject[MIME_SUBJECT_LEN];
					/* MIME ����ʸ����		*/
static int	mime_part_number;	/* ���򤵤�Ƥ���ѡ��ȿ�	*/

static int	top_position;		/* ɽ�����ϰ���			*/

static char	from_buff[BUFF_SIZE];
static char	date_buff[MAX_FIELD_LEN];
static char	subject_buff[BUFF_SIZE];
static char	x_nsubj_buff[MAX_FIELD_LEN];
static char	type_buff[MAX_FIELD_LEN];
static char	trans_buff[MAX_FIELD_LEN];
static char	dpos_buff[MAX_FIELD_LEN];
static struct cpy_hdr	mime_fields[] = {
  {FROM_FIELD,		from_buff,	sizeof(from_buff)},
  {DATE_FIELD,		date_buff,	sizeof(date_buff)},
  {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
  {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
};
static struct cpy_hdr	part_fields[] = {
  {CONTENT_TYPE_FIELD,	type_buff,	sizeof(type_buff)},
  {CONTENT_TRANS_FIELD,	trans_buff,	sizeof(trans_buff)},
  {CONTENT_DPOS_FIELD,	dpos_buff,	sizeof(dpos_buff)},
};
#if	defined(NEWSPOST) || defined(MAILSEND)
static CONTENT_LIST	*mime_content_ptr = (CONTENT_LIST*)NULL;
#endif	/* (NEWSPOST || MAILSEND) */

/*
 * �ѡ�������إ�ץ�å�����
 */

static char	*mime_part_jmessage[] = {
  "�ѡ�������⡼��",
  "\n  [����]",
  "\tSPACE, i �ޤ��� .",
  "\t                �ѡ��Ȥ򻲾Ȥ��ޤ���",
  "\tv �ޤ��� V      ���ƤΥإå��ȤȤ�˥ѡ��Ȥ򻲾Ȥ��ޤ���",
#ifdef	MIME
  "\t                ('V' �� MIME �ǥ������б�)",
#endif	/* MIME */
  "\tp               ����̤�ɥѡ��Ȥ򻲾Ȥ��ޤ���",
  "\tn               ����̤�ɥѡ��Ȥ򻲾Ȥ��ޤ���",
  "\tP               ���Υѡ��Ȥ򻲾Ȥ��ޤ���",
  "\tN               ���Υѡ��Ȥ򻲾Ȥ��ޤ���",
  "\n  [��ư]",
  "\tk �ޤ��� ^P     ���Υѡ��Ȥ˰�ư���ޤ���",
  "\tj �ޤ��� ^N     ���Υѡ��Ȥ˰�ư���ޤ���",
  "\t^U �ޤ��� ^B    �����̤Υѡ��Ȥ˰�ư���ޤ���",
  "\t^D, ^F �ޤ��� ^V",
  "\t                �����̤Υѡ��Ȥ˰�ư���ޤ���",
  "\t<               ��Ƭ�Υѡ��Ȥ˰�ư���ޤ���",
  "\t>               �Ǹ�Υѡ��Ȥ˰�ư���ޤ���",
  "\tTAB             ����Υѡ��Ȥ˥����פ��ޤ���",
  "\t                (�ѡ����ֹ�����Ϥ��Ʋ�����)",
  "\to, q �ޤ��� RETURN",
  "\t                �ѡ�������⡼�ɤ���ȴ���ޤ���",
  "\n  [�ޡ���]",
  "\tD �ޤ��� Y      ���ɥޡ���������̤�ɥѡ��Ȥذ�ư���ޤ���",
  "\td �ޤ��� y      ���ɥޡ���������̤�ɥѡ��Ȥذ�ư���ޤ���",
  "\tU �ޤ��� Z      ���ɥޡ������������̤�ɥѡ��Ȥذ�ư���ޤ���",
  "\tu �ޤ��� z      ���ɥޡ������������̤�ɥѡ��Ȥذ�ư���ޤ���",
  "\tM               �ޥ���ޡ��������/��������Υѡ��Ȥذ�ư���ޤ���",
#ifdef	NEWSPOST
  "\n  [���]",
  "\ta               �����˵�����ݥ��Ȥ��ޤ���",
  "\tf               �ѡ��Ȥ˥ե������ޤ���",
  "\tF               �ѡ��Ȥ���Ѥ��ƥե������ޤ���",
#endif	/* NEWSPOST */
#ifdef	MAILSEND
  "\n  [����]",
  "\tm               �����˥᡼���Ф��ޤ���",
  "\tr               �ѡ��Ȥ��ֿ����ޤ���",
  "\tR               �ѡ��Ȥ���Ѥ����ֿ����ޤ���",
  "\t=               �ѡ��Ȥ�ž�����ޤ���",
#ifdef	LARGE
  "\t~               �ѡ��Ȥ�������ޤ���",
#endif	/* LARGE */
#endif	/* MAILSEND */
  "\n  [������]",
  "\ts               �ѡ��Ȥ򥻡��֤��ޤ���",
  "\t                (�ե�����̾�����Ϥ��Ʋ��������ե����뤬¸�ߤ������",
  "\t                 y:���ڥ�� n:��� o:��񤭤�����ǲ�����)",
#ifdef	MH
  "\tO               �ѡ��Ȥ� MH �ե�����˥����֤��ޤ���",
  "\t                (MH �ե����̾�����Ϥ��Ʋ�����)",
#endif	/* MH */
  (char*)NULL,
};

static char	*mime_part_message[] = {
#ifndef	SMALL
  "PART SELECT MODE",
  "\n  [READ]",
  "\tSPACE, i or .",
  "\t             read part.",
  "\tv or V       read part with all header.",
#ifdef	MIME
  "\t             ('V' decode MIME header.)",
#endif	/* MIME */
  "\tp            read previous unread part.",
  "\tn            read next unread part.",
  "\tP            read previous part.",
  "\tN            read next part.",
  "\n  [MOVE]",
  "\tk or ^P      previous part.",
  "\tj or ^N      next part.",
  "\t^U or ^B     previous page part.",
  "\t^D, ^F or ^V next page part.",
  "\t<            top part.",
  "\t>            last part.",
  "\tTAB          jump to specified part.",
  "\t             (Please input part number.)",
  "\to, q or return",
  "\t             exit from part select mode.",
  "\n  [MARK]",
  "\tD or Y       mark as read and move to previous unread.",
  "\td or y       mark as read and move to next unread.",
  "\tU or Z       clear read mark and move to previous.",
  "\tu or z       clear read mark and move to next.",
  "\tM            toggle multi mark and move to next.",
#ifdef	NEWSPOST
  "\n  [POST]",
  "\ta            post new article.",
  "\tf            follow.",
  "\tF            follow with original part.",
#endif	/* NEWSPOST */
#ifdef	MAILSEND
  "\n  [SEND]",
  "\tm            mail.",
  "\tr            reply.",
  "\tR            reply with original part.",
  "\t=            forward.",
#ifdef	LARGE
  "\t~            relay.",
#endif	/* LARGE */
#endif	/* MAILSEND */
  "\n  [SAVE]",
  "\ts            save part.",
  "\t             (Please input file name.If file exists, please select",
  "\t              y:append n:abort or o:overwrite.)",
#ifdef	MH
  "\tO            save part in MH folder.",
  "\t             (Please Input MH folder name.)",
#endif	/* MH */
#endif	/* !SMALL */
  (char*)NULL,
};

/*
 * MIME ����
 * �����(���:����,0:���顼,����:���ｪλ)
 */

int	mime_manage(fp, mime_mtype, mime_stype, mime_ttype, name)
     FILE	*fp;			/* ���ϥե�����ݥ���	*/
     MIME_MTYPE	mime_mtype;		/* �ᥤ�󥿥���		*/
     MIME_STYPE	mime_stype;		/* ���֥�����		*/
     MIME_TTYPE	mime_ttype;		/* ž�����󥳡��ɥ�����	*/
     char	*name;			/* �ե�����̾		*/
{
  struct stat	stat_buff;
  char		file_name[PATH_BUFF];
#ifdef	MIME_MULTIMEDIA
  char		buff[SMALL_BUFF];
  char		*ptr = (char*)NULL;
#endif	/* MIME_MULTIMEDIA */

  switch (mime_mtype) {
  case M_MTYPE_UNKNOWN:
  case M_MTYPE_TEXT:
    return(0);
    break;
  case M_MTYPE_APP:
#ifdef	MIME_MULTIMEDIA
    if (mime_stype == M_STYPE_PS) {
      ptr = ps_command;
    }
#endif	/* MIME_MULTIMEDIA */
    break;
#ifdef	MIME_MULTIMEDIA
  case M_MTYPE_AUDIO:
    if (mime_stype == M_STYPE_BASIC) {
      ptr = audio_command;
    }
    break;
  case M_MTYPE_IMAGE:
    if (mime_stype == M_STYPE_JPEG) {
      ptr = jpeg_command;
    } else if (mime_stype == M_STYPE_GIF) {
      ptr = gif_command;
    }
    break;
  case M_MTYPE_MSG:
    return(0);
    break;
  case M_MTYPE_VIDEO:
    if (mime_stype == M_STYPE_MPEG) {
      ptr = mpeg_command;
    }
    break;
  case M_MTYPE_EXT:
    return(0);
    break;
  case M_MTYPE_MULTI:
    return(0);
    break;
#endif	/* MIME_MULTIMEDIA */
  default:
    return(0);
    break;
  }
#ifdef	MIME_MULTIMEDIA
  if (ptr != (char*)NULL) {
    if (*ptr) {
      if (yes_or_no(NORMAL_YN_MODE,
		    "\"%s\"��\"%s\"�ν����Τ���¹Ԥ��ޤ���?",
		    "Execute \"%s\" for \"%s\"?", ptr, name)) {
	file_name[0] = '\0';
	if (!mime_save_file(fp, mime_ttype, file_name, name)) {
	  sprintf(buff, "%s %s", ptr, file_name);
	  print_mode_line(japanese ?
			  "\"%s\"�ν�λ���Ե����Ƥ��ޤ���" :
			  "Waiting for \"%s\" exit.", ptr);
	  term_system(buff);
#ifdef	X68K
	  _iocs_crtmod(0x10);
#endif	/* X68K */
	  unlink(file_name);
	  return(1);
	}
	return(-1);
      }
    }
  }
#endif	/* MIME_MULTIMEDIA */
  sprintf(file_name, "%s%c%s", save_dir, SLASH_CHAR, name);
  print_title();
  input_line(INPUT_EXPAND_MASK | INPUT_COMP_MASK,
	     "�ե�����̾�����Ϥ��Ʋ�����:", "Input file name:",
	     file_name);
  if (file_name[0]) {
    if (!stat(file_name, &stat_buff)) {
      if (!yes_or_no(NORMAL_YN_MODE,
		     "�ե�����¸�ߤ��ޤ�����񤭤��ޤ���?",
		     "File exists.Overwrite?")) {
	return(-1);
      }
    }
    if (!mime_save_file(fp, mime_ttype, file_name, file_name)) {
      return(1);
    }
  }
  return(-1);
}

/*
 * MIME �ե����륻����
 */

static int	mime_save_file(fp1, mime_ttype, file_name, hint_name)
     FILE	*fp1;
     MIME_TTYPE	mime_ttype;
     char	*file_name;
     char	*hint_name;
{
  FILE	*fp2;
  int	status;

  status = 1;
  if (!file_name[0]) {
    if (hint_name[0]) {
      sprintf(file_name, "%s/%s", tmp_dir, hint_name);
    } else {
      create_temp_name(file_name, "DM");
    }
  }
#if	defined(MSDOS) || defined(OS2)
  fp2 = fopen(file_name, "wb");
#else	/* !(MSDOS || OS2) */
  fp2 = fopen(file_name, "w");
#endif	/* !(MSDOS || OS2) */
  if (fp2 != (FILE*)NULL) {
    chmod(file_name, S_IREAD | S_IWRITE);
    while (!mime_decode_file(mime_ttype, fp1, fp2));
    if (!fflush(fp2)) {
      status = 0;
    }
    fclose(fp2);
  }
  if (status) {
    print_mode_line(japanese ? "�ե�����\"%s\"�Υ����֤˼��Ԥ��ޤ�����" :
		    "Can't save file \"%s\".", file_name);
    term_bell();
    sleep(ERROR_SLEEP);
  }
  return(status);
}

/*
 * MIME �ѡ��ȥ�˥塼
 */

int	mime_part_menu(file_name, boundary, current_group)
     char	*file_name;
     char	*boundary;
     int	current_group;
{
  ARTICLE_LIST	*article_ptr;
  char		*file_ptr;
  char		*boundary_ptr;
  int		part_number;
  int		status;

  article_ptr  = article_list;
  article_list = (ARTICLE_LIST*)NULL;
  file_ptr     = mime_file;
  mime_file    = file_name;
  boundary_ptr = mime_boundary;
  mime_boundary = boundary;
  part_number = mime_part_number;
  status = mime_select_article(current_group);
  if (article_list != (ARTICLE_LIST*)NULL) {
    free(article_list);
  }
  article_list     = article_ptr;
  mime_file        = file_ptr;
  mime_boundary    = boundary_ptr;
  mime_part_number = part_number;
  return(status);
}

/*
 * �ѡ���������̺�����
 */

static void	mime_redraw_part(current_part)
     int	current_part;
{
  register int	i;
  char		buff[SMALL_BUFF];

  i = top_position;
  top_position = get_top_position(top_position, current_part,
				  mime_part_number, 0, buff);
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
  print_full_line(japanese ?
		  "���֥�������:%-46.46s     ����:%s" :
		  "Subject:%-46.46s      Position:%s",
		  mime_subject, buff);
  print_articles(top_position, current_part, mime_part_number,
		 (int (*)())NULL, "PANIC");
  print_mode_line(japanese ?
		  "?:�إ�� j,^N:���� k,^P:���� SPACE,i,.:���� o,q:���� Q:��λ" :
		  "?:help j,^N:next k,^P:previous SPACE,i,.:read o,q:return Q:exit");
}

/*
 * �ѡ�������
 */

static int	mime_select_article(current_group)
     int	current_group;
{
  int		current_part;		/* ������Υѡ����ֹ�		*/
  char		buff[BUFF_SIZE];
  int		loop;
  int		status;
  int		key;
  register int	i, j;

  current_part = 0;
  jump_name[0] = '\0';
  top_position = -1;
  status = mime_get_list();
  if (status < 0) {
    return(0);
  }

  loop = 1;
  status = 0;
  while (loop) {
    mime_redraw_part(current_part);
    term_locate(article_format ? 6 : 11,
		head_lines + current_part - top_position);
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
	if (line < mime_part_number) {
	  current_part = line;
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
      current_part = mime_prev_part(current_part);
      break;
    case 0x0e:		/* ^N ��������ư	*/
    case 'j':
      current_part = mime_next_part(current_part);
      break;
    case '\015':	/* RETURN		*/
    case '\n':
    case 'o':
    case 'q':
      loop = 0;
      break;
    case 0x02:		/* ^B 			*/
    case 0x15:		/* ^U ���ڡ���		*/
      if ((current_part -= (term_lines - mode_lines)) < 0) {
	current_part = 0;
      }
      break;
    case 0x04:		/* ^D			*/
    case 0x06:		/* ^F			*/
    case 0x16:		/* ^V ���ڡ���		*/
      if ((current_part += (term_lines - mode_lines))
	  >= mime_part_number) {
	if (mime_part_number > 0) {
	  current_part = mime_part_number - 1;
	} else {
	  current_part = 0;
	}
      }
      break;
    case '<':		/* �ǽ�ε���		*/
      current_part = 0;
      break;
    case '>':		/* �Ǹ�ε���		*/
      if (mime_part_number > 0) {
	current_part = mime_part_number - 1;
      } else {
	current_part = 0;
      }
      break;
    case '?':		/* �إ��		*/
      help(mime_part_jmessage, mime_part_message,
	   GLOBAL_MODE_MASK | SUBJECT_MODE_MASK);
      top_position = -1;
      break;
    case 'Q':		/* ��λ			*/
      loop = 0;
      status = 1;
      break;
#ifdef	NEWSPOST
    case 'a':		/* �˥塼�����		*/
      if ((current_group >= 0) &&
	  ((news_group[current_group].group_mode == (short)POST_ENABLE) ||
	   (news_group[current_group].group_mode == (short)POST_MODERATED))) {
	news_post(select_name);
	top_position = -2;
      }
      break;
#endif	/* NEWSPOST */
    default:

      /*
       * �������鲼�ν����ϸ������ܤ�������������ޤ���
       */

      last_key = key;
      while (last_key) {
	key = last_key;
	last_key = 0;
	j = current_part;
	switch (key) {
	case 'p':		/* ���ѡ��Ȼ���		*/
	case 'P':
	  if (key == 'p') {
	    current_part = mime_prev_unread_part(current_part);
	  } else {
	    current_part = mime_prev_part(current_part);
	  }
	  if (current_part == j) {
	    last_key = key;
	    return(0);
	  }
	  break;
	case 'n':		/* ���ѡ��Ȼ���		*/
	case 'N':
	  if (key == 'n') {
	    current_part = mime_next_unread_part(current_part);
	  } else {
	    current_part = mime_next_part(current_part);
	  }
	  if (current_part == j) {
	    last_key = key;
	    return(0);
	  }
	  break;
	default:
	  break;
	}
	if (mime_part_number > 0) {
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
	    if (!mime_read(article_list[current_part].real_number, i)) {
	      switch (last_key) {
	      case 'D':
	      case 'Y':
		mime_mark(current_part, READ_MARK);
		last_key = 'p';
		break;
	      case 'y':
	      case 'd':
		mime_mark(current_part, READ_MARK);
		last_key = 'n';
		break;
	      case 'U':
	      case 'Z':
		mime_unmark(current_part, READ_MARK);
		last_key = 'P';
		break;
	      case 'u':
	      case 'z':
		mime_unmark(current_part, READ_MARK);
		last_key = 'N';
		break;
	      default:
		mime_mark(current_part, READ_MARK);
		break;
	      }
	      if (article_list[current_part].mark & CANCEL_MARK) {
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
	      case 'P':
		current_part = mime_prev_part(current_part);
		break;
	      case 'n':
	      case 'N':
		current_part = mime_next_part(current_part);
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
	    mime_mark(current_part, READ_MARK);
	    toggle_mark(top_position, current_part, 0);
	    if (isupper(key)) {
	      current_part = mime_prev_unread_part(current_part);
	    } else {
	      current_part = mime_next_unread_part(current_part);
	    }
	    break;
	  case 'U':		/* �ޡ������,��������ư	*/
	  case 'Z':
	  case 'u':		/* �ޡ������,��������ư	*/
	  case 'z':
	    mime_unmark(current_part, READ_MARK);
	    toggle_mark(top_position, current_part, 0);
	    if (isupper(key)) {
	      current_part = mime_prev_part(current_part);
	    } else {
	      current_part = mime_next_part(current_part);
	    }
	    break;
	  case 's':		/* ����������		*/
	    if (!multi_save(current_part, mime_extract)) {
	      multi_add_mark(mime_part_number, -1,
			     current_part, READ_MARK, mime_mark);
	    }
	    top_position = -1;
	    break;
#ifdef	MH
	  case 'O':		/* �ե�����ص��������� */
	    if (!save_mh_folder(current_part, mime_extract)) {
	      multi_add_mark(mime_part_number, -1,
			     current_part, READ_MARK, mime_mark);
	    }
	    top_position = -1;
	    break;
#endif	/* MH */
#ifndef	SMALL
	  case '/':		/* ��������		*/
	    search_subjects(0, mime_part_number, &current_part, 
			    (int (*)())NULL);
	    break;
	  case '\\':		/* ��������		*/
	    search_subjects(1, mime_part_number, &current_part, 
			    (int (*)())NULL);
	    break;
	  case '|':		/* �����ѥ��׼¹�	*/
	    if (!multi_pipe(current_part, mime_extract)) {
	      multi_add_mark(mime_part_number, -1,
			     current_part, READ_MARK, mime_mark);
	    }
	    top_position = -1;
	    break;
#endif	/* !SMALL */
	  case '\t':		/* TAB ���국���إ�����	*/
	    buff[0] = '\0';
	    input_line(INPUT_SPCCUT_MASK, "�����ֹ�����Ϥ��Ʋ�����:",
		       "Input article number:", buff);
	    if ((j = atoi(buff)) > 0) {
	      for (i = 0; i < mime_part_number; i++) {
		if (article_list[i].real_number == j) {
		  current_part = i;
		  break;
		}
	      }
	    }
	    top_position = -1;
	    break;
#ifdef	MAILSEND
	  case 'r':		/* �᡼���ֿ�		*/
	    create_temp_name(buff, "NR");
	    if (!mime_extract(article_list[current_part].real_number,
			      buff)) {
	      mail_reply(buff, 0, "");
	      funlink2(buff);
	    }
	    top_position = -2;
	    break;
	  case 'R':		/* �᡼���ֿ�		*/
	    create_temp_name(buff, "NR");
	    if (!multi_extract(current_part, buff, mime_extract)) {
	      if (!mail_reply(buff, REPLY_QUOTE_MASK, "")) {
		multi_add_mark(mime_part_number, -1,
			       current_part, READ_MARK, mime_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -2;
	    break;
	  case '=':		/* �᡼��ž��		*/
	    create_temp_name(buff, "NT");
	    if (!multi_extract(current_part, buff, mime_extract)) {
	      if (!mail_forward(buff, "")) {
		multi_add_mark(mime_part_number, -1,
			       current_part, READ_MARK, mime_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -2;
	    break;
#ifdef	LARGE
	  case '~':		/* �᡼�����		*/
	    create_temp_name(buff, "NL");
	    if (!multi_extract(current_part, buff, mime_extract)) {
	      if (!mail_relay(buff, "")) {
		multi_add_mark(mime_part_number, -1,
			       current_part, READ_MARK, mime_mark);
	      }
	      funlink2(buff);
	    }
	    top_position = -2;
	    break;
#endif	/* LARGE */
#endif	/* MAILSEND */
#ifdef	NEWSPOST
	  case 'f':		/* �˥塼���ե���	*/
	    if ((current_group >= 0) &&
		((news_group[current_group].group_mode ==
		  (short)POST_ENABLE) ||
		 (news_group[current_group].group_mode ==
		  (short)POST_MODERATED))) {
	      create_temp_name(buff, "NF");
	      if (!mime_extract(article_list[current_part].real_number,
				buff)) {
		news_follow(buff, 0);
		funlink2(buff);
	      }
	      top_position = -2;
	    }
	    break;
	  case 'F':		/* �˥塼���ե���	*/
	    if ((current_group >= 0) &&
		((news_group[current_group].group_mode ==
		  (short)POST_ENABLE) ||
		 (news_group[current_group].group_mode ==
		  (short)POST_MODERATED))) {
	      create_temp_name(buff, "NF");
	      if (!multi_extract(current_part, buff, mime_extract)) {
		if (!news_follow(buff, FOLLOW_QUOTE_MASK)) {
		  multi_add_mark(mime_part_number, -1,
				 current_part, READ_MARK, mime_mark);
		}
		funlink2(buff);
	      }
	      top_position = -2;
	    }
	    break;
#endif	/* NEWSPOST */
	  case 'M':		/* �ޥ���ޡ�������/���	*/
	    multi_mark(top_position, current_part);
	    current_part = mime_next_part(current_part);
	    break;
#ifdef	LARGE
	  case '@':		/* ����			*/
	    if (!multi_print(current_part, mime_extract)) {
	      multi_add_mark(mime_part_number, -1,
			     current_part, READ_MARK, mime_mark);
	    }
	    top_position = -1;
	    break;
	  case '(':		/* ������������		*/
	    search_articles(0, mime_part_number, &current_part, 
			    mime_extract);
	    break;
	  case ')':		/* ������������		*/
	    search_articles(1, mime_part_number, &current_part, 
			    mime_extract);
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
  return(status);
}

/*
 * �ѡ��Ȼ���
 */

static int	mime_read(real_number, mode)
     int	real_number;
     int	mode;
{
  static int	nest_level = 0;
  char		tmp_file1[PATH_BUFF];
  char		tmp_file2[PATH_BUFF];
  char		buff[SMALL_BUFF];
  int		status;

  nest_level++;
  sprintf(buff, "%1dC", nest_level);
  create_temp_name(tmp_file1, buff);
  sprintf(buff, "%1dV", nest_level);
  create_temp_name(tmp_file2, buff);
  if (mime_extract(real_number, tmp_file1)) {
    nest_level--;
    return(1);
  }
  sprintf(buff, "%d", real_number);
  status = exec_pager(tmp_file1, tmp_file2, mode, buff);
  nest_level--;
  funlink2(tmp_file1);
  if (status) {
    return(1);
  }
  return(0);
}

/*
 * �ѡ������
 */

static int	mime_extract(real_number, tmp_file)
     int	real_number;
     char	*tmp_file;
{
  FILE		*fp1, *fp2;
  char		buff[BUFF_SIZE];
  int		number;
  register int	i, j;

  fp1 = fopen(mime_file, "r");
  if (fp1 == (FILE*)NULL) {
    return(1);
  }
  fp2 = fopen2(tmp_file, "a");
  if (fp2 == (FILE*)NULL) {
    fclose(fp1);
    return(1);
  }
  j = 0;
  while (fgets(buff, sizeof(buff), fp1)) {
    if ((!buff[0]) || (buff[0] == '\n')) {
      break;
    }
    if ((buff[0] == ' ') || (buff[0] == '\t')) {
      if (!j) {
	fputs(buff, fp2);
      }
    } else {
      j = 0;
      for (i = 0; i < (sizeof(part_fields) / sizeof(struct cpy_hdr)); i++) {
	if (!strncasecmp(buff, part_fields[i].field_name,
			 strlen(part_fields[i].field_name))) {
	  j = 1;
	  break;
	}
      }
      if (!j) {
	fputs(buff, fp2);
      }
    }
  }
  while (fgets(buff, sizeof(buff), fp1)) {
    if (strindex(buff, mime_boundary)) {
      break;
    }
  }
  number = 1;
  while (!feof(fp1)) {
    if (number < real_number) {
      while (fgets(buff, sizeof(buff), fp1)) {
	if (strindex(buff, mime_boundary)) {
	  number++;
	  break;
	}
      }
    } else {
      while (fgets(buff, sizeof(buff), fp1)) {
	if (strindex(buff, mime_boundary)) {
	  break;
	}
	fputs(buff, fp2);
      }
      fclose(fp1);
      fclose(fp2);
      return(0);
    }
  }
  fclose(fp1);
  fclose(fp2);
  funlink2(tmp_file);
  return(1);
}

/*
 * ���ѡ����ֹ����
 */

static int	mime_prev_part(current_part)
     int	current_part;
{
  if (--current_part < 0) {
    current_part = 0;
  }
  return(current_part);
}

/*
 * ���ѡ����ֹ����
 */

static int	mime_next_part(current_part)
     int	current_part;
{
  if (++current_part >= mime_part_number) {
    if (mime_part_number > 0) {
      current_part = mime_part_number - 1;
    } else {
      current_part = 0;
    }
  }
  return(current_part);
}

/*
 * ��̤�ɥѡ����ֹ����
 */

static int	mime_prev_unread_part(current_part)
     int	current_part;
{
  while (current_part > 0) {
    if (!(article_list[--current_part].mark & READ_MARK)) {
      break;
    }
  }
  return(current_part);
}

/*
 * ��̤�ɥѡ����ֹ����
 */

static int	mime_next_unread_part(current_part)
     int	current_part;
{
  while (current_part < (mime_part_number - 1)) {
    if (!(article_list[++current_part].mark & READ_MARK)) {
      break;
    }
  }
  return(current_part);
}

/*
 * �ѡ��ȥޡ����ɲ�
 */

static void	mime_mark(current_part, mark)
     int	current_part;
     int	mark;
{
  article_list[current_part].mark |= mark;
}

/*
 * �ѡ��ȥޡ������
 */

static void	mime_unmark(current_part, mark)
     int	current_part;
     int	mark;
{
  article_list[current_part].mark &= ~mark;
}

/*
 * �ѡ��ȥꥹ�ȼ���
 */

static int	mime_get_list()
{
  FILE			*fp;
  char			buff[BUFF_SIZE];
  int			alloc_number;
  ARTICLE_LIST		*alloc_ptr;
  short			year, month, date, day, hour, minute, second;
  register int		lines;

  mime_part_number = 0;
  mime_subject[0] = '\0';
  if (article_list != (ARTICLE_LIST*)NULL) {
    free(article_list);
  }
  article_list = (ARTICLE_LIST*)NULL;
  alloc_number = 0;
  fp = fopen(mime_file, "r");
  if (fp == (FILE*)NULL) {
    return(1);
  }
  copy_fields(fp, mime_fields,
	      sizeof(mime_fields) / sizeof(struct cpy_hdr),
	      CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
  while (fgets(buff, sizeof(buff), fp)) {
    if (strindex(buff, mime_boundary)) {
      break;
    }
  }
  if (!feof(fp)) {
    convert_article_date(date_buff, &year, &month, &date, &day,
			 &hour, &minute, &second, buff);
    get_real_adrs(from_buff, buff);
    strcpy(from_buff, buff);
    if (x_nsubj_mode && x_nsubj_buff[0]) {
      recover_jis(subject_buff, x_nsubj_buff);
    }
    mime_decode_func(buff, subject_buff, default_code);
    euc_tab_strncpy(mime_subject, buff, sizeof(mime_subject) - 1);
#ifdef	SJIS_SRC
    euc_to_sjis(buff, mime_subject);
    strcpy(mime_subject, buff);
#endif	/* SJIS_SRC */
    while (!feof(fp)) {
      copy_fields(fp, part_fields,
		  sizeof(part_fields) / sizeof(struct cpy_hdr),
		  CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
      if (feof(fp)) {
	break;
      }
      if (!type_buff[0]) {
	strcpy(type_buff, "(text/plain; charset=US-ASCII)");
      }
      lines = 0;
      while (1) {
	if (fgets(buff, sizeof(buff), fp)) {
	  if (strindex(buff, mime_boundary)) {
	    break;
	  }
	  lines++;
	} else {
	  lines = 0;
	  break;
	}
      }
      if (!lines) {
	break;
      }
      if (alloc_number <= mime_part_number) {
	if (article_list != (ARTICLE_LIST*)NULL) {
	  alloc_ptr =
	    (ARTICLE_LIST*)realloc(article_list,
				   sizeof(ARTICLE_LIST) *
				   (alloc_number + MIME_ALLOC_COUNT));
	} else {
	  alloc_ptr = (ARTICLE_LIST*)malloc(sizeof(ARTICLE_LIST) *
					    (alloc_number + MIME_ALLOC_COUNT));
	}
	if (alloc_ptr == (ARTICLE_LIST*)NULL) {
	  print_fatal("Can't allocate memory for MIME part struct.");
	  break;
	}
	article_list = alloc_ptr;
	alloc_number += MIME_ALLOC_COUNT;
      }
      article_list[mime_part_number].real_number = mime_part_number + 1;
      strcpy(article_list[mime_part_number].from, from_buff);
      strcpy(article_list[mime_part_number].subject, type_buff);
      article_list[mime_part_number].mark  = 0;
#ifdef	REF_SORT
      article_list[mime_part_number].year  = year;
#endif	/* REF_SORT */
      article_list[mime_part_number].month = month;
      article_list[mime_part_number].date  = date;
      article_list[mime_part_number].lines = lines;
      mime_part_number++;
    }
  }
  fclose(fp);
  return(0);
}

#if	defined(NEWSPOST) || defined(MAILSEND)
/*
 * MIME ����ƥ�Ľ����
 */

void	mime_init_content()
{
  CONTENT_LIST	*content_ptr1, *content_ptr2;

  content_ptr1 = mime_content_ptr;
  while (content_ptr1 != (CONTENT_LIST*)NULL) {
    content_ptr2 = content_ptr1;
    content_ptr1 = content_ptr1->next_ptr;
    free(content_ptr2);
  }
  mime_content_ptr = (CONTENT_LIST*)NULL;
}

/*
 * MIME ����ƥ����Ͽ
 */

int	mime_add_content(mime_mtype, mime_stype, mime_ttype, name)
     MIME_MTYPE	mime_mtype;		/* �ᥤ�󥿥���		*/
     MIME_STYPE	mime_stype;		/* ���֥�����		*/
     MIME_TTYPE	mime_ttype;		/* ž�����󥳡��ɥ�����	*/
     char	*name;			/* �ե�����̾		*/
{
  CONTENT_LIST	*content_ptr1, *content_ptr2;
  struct stat	stat_buff;
  char		file_name[PATH_BUFF];

  if (mime_format != 2) {
    print_mode_line(japanese ?
		    "�ٹ� : MIME ���󥳡��ɥ⡼�ɤǤϤʤ��Τǥ���ƥ�Ļ����̵�뤷�ޤ���" :
		    "Warning : Ignoring contents because not MIME encode mode.");
    term_bell();
    sleep(ERROR_SLEEP);
    return(1);
  }
  strcpy(file_name, name);
  while (stat(file_name, &stat_buff)) {
    print_title();
    print_mode_line(japanese ?
		    "�ե�����\"%s\"�����Ĥ���ޤ���" :
		    "File \"%s\" not found.", file_name);
    term_bell();
    sleep(ERROR_SLEEP);
    input_line(INPUT_EXPAND_MASK | INPUT_COMP_MASK,
	       "�ե�����̾�����Ϥ��Ʋ�����:", "Input file name:", file_name);
    if (!file_name[0]) {
      return(1);
    }
  }
  content_ptr2 = (CONTENT_LIST*)malloc(sizeof(CONTENT_LIST));
  if (content_ptr2 == (CONTENT_LIST*)NULL) {
    print_fatal("Can't allocate memory for MIME contents struct.");
    return(1);
  }
  content_ptr2->c_mtype = mime_mtype;
  content_ptr2->c_stype = mime_stype;
  content_ptr2->c_ttype = mime_ttype;
  content_ptr2->c_size  = (long)stat_buff.st_size;
  strcpy(content_ptr2->c_name, file_name);
  content_ptr2->next_ptr = (CONTENT_LIST*)NULL;
  if (mime_content_ptr == (CONTENT_LIST*)NULL) {
    mime_content_ptr = content_ptr2;
  } else {
    content_ptr1 = mime_content_ptr;
    while (content_ptr1->next_ptr != (CONTENT_LIST*)NULL) {
      content_ptr1 = content_ptr1->next_ptr;
    }
    content_ptr1->next_ptr = content_ptr2;
  }
  return(0);
}

/*
 * MIME ����ƥ�Ŀ�����
 */

int	mime_get_content_num()
{
  CONTENT_LIST	*content_ptr;
  register int	number = 0;

  content_ptr = mime_content_ptr;
  while (content_ptr != (CONTENT_LIST*)NULL) {
    number++;
    content_ptr = content_ptr->next_ptr;
  }
  return(number);
}
#endif	/* (NEWSPOST || MAILSEND) */
#endif	/* MIME */
