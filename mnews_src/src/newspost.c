/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : News post routine
 *  File        : newspost.c
 *  Version     : 1.21
 *  First Edit  : 1992-08/03
 *  Last  Edit  : 1997-12/23
 *  Author      : MSR24 宅間 顯
 *
 */

#ifdef	NEWSPOST
#include	"compat.h"
#include	"nntplib.h"
#include	"smtplib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"termlib.h"
#include	"mnews.h"
#include	"group.h"
#include	"article.h"
#include	"newspost.h"
#include	"pager.h"
#include	"fcc.h"
#include	"mailsend.h"
#include	"mimelib.h"
#ifdef	MIME_XXX
#include	"mimepart.h"
#endif	/* MIME_XXX */

static FILE	*get_news_signature();	/* signature ファイル取得	*/

static char	from_buff[MAX_FIELD_LEN];
static char	date_buff[MAX_FIELD_LEN];
static char	message_buff[MAX_FIELD_LEN];
static char	subject_buff[MAX_FIELD_LEN];
static char	group_buff[MAX_FIELD_LEN];
static char	dist_buff[MAX_FIELD_LEN];
static char	reference_buff[MAX_FIELD_LEN * 10];
static char	followup_buff[MAX_FIELD_LEN];
static char	x_nsubj_buff[MAX_FIELD_LEN];
#ifdef	FCC
static char	fcc_buff[MAX_FIELD_LEN];
#endif	/* FCC */
#ifdef	MIME_XXX
static char	bin_file_buff[PATH_BUFF];
#endif	/* MIME_XXX */
#ifdef	MAILSEND
static int	mail_flag;
#endif	/* MAILSEND */

/*
 * ニュース投稿
 */

int	news_post(group_name)
     char	*group_name;
{
  char	tmp_file[PATH_BUFF];
  char	backup_file[PATH_BUFF];
  char	group_buff[MAX_FIELD_LEN];
  char	dist_buff[MAX_FIELD_LEN];
  char	subject_buff[MAX_FIELD_LEN];
  FILE	*fp;
  int	first;
  int	status;

#ifdef	SEND_CONFIRM
  if (!yes_or_no(NORMAL_YN_MODE, "新しい記事を投稿しますか?",
		 "Post new article.Are you sure?")) {
    return(1);
  }
#endif	/* SEND_CONFIRM */
  strcpy(group_buff, group_name);
  if (!group_buff[0]) {
    input_line(0, "ニュースグループを入力して下さい:",
	       "Input news group:", group_buff);
    if (!group_buff[0]) {
      return(1);
    }
  }
  subject_buff[0] = '\0';
  input_line(0, "題名(subject)を入力して下さい:",
	     "Input subject:", subject_buff);
  if (!subject_buff[0]) {
    return(1);
  }
  dist_buff[0] = '\0';
  if (ask_distrib_mode) {
    input_line(0, "配布範囲(distribution)を入力して下さい:",
	       "Input distribution:", dist_buff);
  }
  sprintf(tmp_file, "%s%c%s", dot_dir, SLASH_CHAR, MESSAGE_FILE);
  create_backup_name(backup_file, tmp_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  rename(tmp_file, backup_file);
  fp = fopen(tmp_file, "w");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't open news write file.");
    return(1);
  }
  chmod(tmp_file, S_IREAD | S_IWRITE);
  if (news_field[0]) {
    edit_fprintf(fp, "%s\n", news_field);
  }
  fprintf(fp, "%s %s\n", GROUP_FIELD, group_buff);
  fprintf(fp, "%s %s\n", DIST_FIELD, dist_buff);
  fprintf(fp, "%s \n", FOLLOWUP_FIELD);
  edit_fprintf(fp, "%s %s\n", SUBJECT_FIELD, subject_buff);
  if (x_nsubj_mode) {
    if (discriminate_code(subject_buff) != ASCII_CODE) {
      edit_fprintf(fp, "%s %s\n", X_NSUBJ_FIELD, subject_buff);
    } else {
      edit_fprintf(fp, "%s \n", X_NSUBJ_FIELD);
    }
  }
#ifdef	FCC
  if (news_fcc_field[0]) {
    fprintf(fp, "%s %s\n", FCC_FIELD, news_fcc_field);
  }
#endif	/* FCC */
#ifdef	MIME_XXX
  if (mime_format == 2) {
    fprintf(fp, "%s \n", X_BIN_FILE_FIELD);
  }
#endif	/* MIME_XXX */
  fprintf(fp, "%s\n", TEXT_SEPARATER);
  fclose(fp);
  first = 1;
 EDIT:
  if (!exec_editor(tmp_file, first)) {
    first = 0;
    while (1) {
      status = exec_post(tmp_file);
      if (status != -3) {
#ifdef	MAILSEND
	if (mail_flag) {
	  status = exec_send(tmp_file, "");
	  if (status != -3) {
	    break;
	  }
	} else {
	  break;
	}
#else	/* !MAILSEND */
	break;
#endif	/* !MAILSEND */
      }
    }
    if (status) {
      if (status > 0) {
	print_mode_line(japanese ? "ニュースの投稿に失敗しました。" :
			"Post article failed.");
	term_bell();
	sleep(ERROR_SLEEP);
      } else if (status == -2) {
	goto EDIT;
      }
      return(1);
    }
    return(0);
  }
  return(1);
}

/*
 * ニュースフォロー
 */

int	news_follow(base_file, mask)
     char	*base_file;
     int	mask;
{
  FILE			*fp1, *fp2;
  char			tmp_file[PATH_BUFF];
  char			backup_file[PATH_BUFF];
  char			follow_file[PATH_BUFF];
  char			buff1[BUFF_SIZE];
  char			buff2[BUFF_SIZE];
#ifdef	RECOVER_SUBJECT
  static char		recover_buff[BUFF_SIZE];
#endif	/* RECOVER_SUBJECT */
  static struct cpy_hdr	follow_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
    {DATE_FIELD,	date_buff,	sizeof(date_buff)},
    {MESSAGE_FIELD,	message_buff,	sizeof(message_buff)},
#ifdef	RECOVER_SUBJECT
    {SUBJECT_FIELD,	recover_buff,	sizeof(recover_buff)},
#else	/* !RECOVER_SUBJECT */
    {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
#endif	/* !RECOVER_SUBJECT */
    {GROUP_FIELD,	group_buff,	sizeof(group_buff)},
    {DIST_FIELD,	dist_buff,	sizeof(dist_buff)},
    {REFERENCE_FIELD,	reference_buff,	sizeof(reference_buff)},
    {FOLLOWUP_FIELD,	followup_buff,	sizeof(followup_buff)},
    {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
  };
  char			*ptr1, *ptr2;
  int			first;
  int			status;
  int			i;

#ifdef	SEND_CONFIRM
  if (!yes_or_no(NORMAL_YN_MODE, "記事をフォローしますか?",
		 "Follow article.Are you sure?")) {
    return(1);
  }
#endif	/* SEND_CONFIRM */
  print_mode_line(japanese ? "変換中です。" : "Converting.");
  create_temp_name(follow_file, "FD");
  convert_code(base_file, follow_file, EUC_CODE, 4);
#if	defined(MSDOS) || defined(OS2)
  fp1 = fopen(follow_file, "rb");			/* バイナリモード */
#else	/* !(MSDOS || OS2) */
  fp1 = fopen(follow_file, "r");
#endif	/* !(MSDOS || OS2) */
  funlink2(follow_file);
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't open post read file.");
    return(1);
  }
  sprintf(tmp_file, "%s%c%s", dot_dir, SLASH_CHAR, MESSAGE_FILE);
  create_backup_name(backup_file, tmp_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  rename(tmp_file, backup_file);
  fp2 = fopen(tmp_file, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't open post write file.");
    fclose(fp1);
    return(1);
  }
  chmod(tmp_file, S_IREAD | S_IWRITE);
  if (news_field[0]) {
    edit_fprintf(fp2, "%s\n", news_field);
  }
  copy_fields(fp1, follow_fields,
	      sizeof(follow_fields) / sizeof(struct cpy_hdr),
	      CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
  if (followup_buff[0]) {
    if (!strcmp(followup_buff, POSTER)) {
      fclose(fp1);
      fclose(fp2);
      unlink(tmp_file);
      return(mail_reply(base_file, mask, ""));
    }
    fprintf(fp2, "%s %s\n", GROUP_FIELD, followup_buff);
  } else {
    fprintf(fp2, "%s %s\n", GROUP_FIELD, group_buff);
  }
  fprintf(fp2, "%s %s\n", DIST_FIELD, dist_buff);
#ifdef	RECOVER_SUBJECT
  recover_jis(subject_buff, recover_buff);
#endif	/* RECOVER_SUBJECT */
  ptr1 = subject_buff;
  if (!strncasecmp(subject_buff, REPLY_SUBJECT, sizeof(REPLY_SUBJECT) - 1)) {
    ptr1 += (sizeof(REPLY_SUBJECT) - 1);
    if (*ptr1 == ' ') {
      ptr1++;
    }
  }
  edit_fprintf(fp2, "%s %s %s\n", SUBJECT_FIELD, REPLY_SUBJECT, ptr1);
  if (x_nsubj_mode) {
    if (x_nsubj_buff[0]) {
      recover_jis(subject_buff, x_nsubj_buff);
    } else {
      subject_buff[0] = '\0';
    }
    if (strncasecmp(subject_buff, REPLY_SUBJECT, sizeof(REPLY_SUBJECT) - 1)
	&& subject_buff[0]) {
      edit_fprintf(fp2, "%s %s %s\n", X_NSUBJ_FIELD, REPLY_SUBJECT,
		   subject_buff);
    } else {
      edit_fprintf(fp2, "%s %s\n", X_NSUBJ_FIELD, subject_buff);
    }
  }

  /*
   * Reference: フィールド作成
   */

  ptr1 = reference_buff;
  if (!(mask & FOLLOW_QUOTE_MASK)) {
    *ptr1 = '\0';
  }
  first = status = 1;
  while (1) {
    if (!(*ptr1)) {
      if (status) {
	ptr1 = message_buff;
	status = 0;
      } else {
	break;
      }
    }
    while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
      ptr1++;
    }
    ptr2 = buff1;
    while (*ptr1) {
      if (*ptr1 == '>') {
	*ptr2++ = *ptr1++;
	break;
      } else if (*ptr1 == '\n') {
	break;
      }
      *ptr2++ = *ptr1++;
    }
    *ptr2 = '\0';
    if (first) {
      fprintf(fp2, "%s %s\n", REFERENCE_FIELD, buff1);
      first = 0;
    } else {
      fprintf(fp2, "\t%s\n", buff1);
    }
  }
#ifdef	FCC
  if (news_fcc_field[0]) {
    fprintf(fp2, "%s %s\n", FCC_FIELD, news_fcc_field);
  }
#endif	/* FCC */
#ifdef	MIME_XXX
  if (mime_format == 2) {
    fprintf(fp2, "%s \n", X_BIN_FILE_FIELD);
  }
#endif	/* MIME_XXX */
  fprintf(fp2, "%s\n", TEXT_SEPARATER);
  if (mask & FOLLOW_QUOTE_MASK) {
    fseek(fp1, 0L, 0);
    while (1) {
      copy_fields(fp1, follow_fields,
		  sizeof(follow_fields) / sizeof(struct cpy_hdr),
		  CF_CLR_MASK | CF_GET_MASK | CF_SPC_MASK);
#ifdef	RECOVER_SUBJECT
      recover_jis(subject_buff, recover_buff);
#endif	/* RECOVER_SUBJECT */

      /*
       * フィールド検出チェック
       */

      for (i = 0; i < (sizeof(follow_fields) / sizeof(struct cpy_hdr)); i++) {
	if (follow_fields[i].field_buff[0]) {
	  i = -1;
	  break;
	}
      }
      if (i >= 0) {
	break;
      }
      if (follow_message[0]) {
	ptr1 = follow_message;
      } else {
	ptr1 = (japanese ? "%Iの記事において\n%F{さん,私}は書きました。\n" :
		"In article %I\n%F writes:\n");
#ifdef	SJIS_SRC
	sjis_to_euc(buff1, ptr1);
	ptr1 = buff1;
#endif	/* SJIS_SRC */
      }
      field_fprintf(fp2, ptr1,
		    from_buff, subject_buff, message_buff,
		    group_buff, date_buff);
      fputc('\n', fp2);
      while (fgets2(buff1, sizeof(buff1), fp1)) {
	if (!strcmp(buff1, MULTI_END_MARK)) {
	  fputc('\n', fp2);
	  break;
	}
	field_fprintf(fp2, reply_indicator,
		      from_buff, subject_buff, message_buff,
		      group_buff, date_buff);
	to_euc(buff2, buff1, EUC_CODE);
	edit_fprintf(fp2, "%s", buff2);
      }
      if (feof(fp1)) {
	break;
      }
    }
  }
  fclose(fp1);
  fclose(fp2);
  first = 1;
 EDIT:
  if (!exec_editor(tmp_file, first)) {
    first = 0;
    while (1) {
      status = exec_post(tmp_file);
      if (status != -3) {
#ifdef	MAILSEND
	if (mail_flag) {
	  status = exec_send(tmp_file, "");
	  if (status != -3) {
	    break;
	  }
	} else {
	  break;
	}
#else	/* !MAILSEND */
	break;
#endif	/* !MAILSEND */
      }
    }
    if (status) {
      if (status > 0) {
	print_mode_line(japanese ? "ニュースのフォローに失敗しました。" :
			"Post follow article failed.");
	term_bell();
	sleep(ERROR_SLEEP);
      } else if (status == -2) {
	goto EDIT;
      }
      return(1);
    }
    return(0);
  }
  return(1);
}

/*
 * ニュースキャンセル
 */

int	news_cancel(article_number)
     int	article_number;
{
  FILE			*fp;
  char			buff1[SMALL_BUFF];
  char			buff2[SMALL_BUFF];
  char			tmp_file[PATH_BUFF];
  char			*ptr1, *ptr2;
  static struct cpy_hdr	cancel_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
    {MESSAGE_FIELD,	message_buff,	sizeof(message_buff)},
    {GROUP_FIELD,	group_buff,	sizeof(group_buff)},
    {DIST_FIELD,	dist_buff,	sizeof(dist_buff)},
  };
  NNTP_ERR_CODE		status;
#ifdef	ADD_DATE_FIELD
  struct tm		*tm;
  time_t		now_time;
#endif	/* ADD_DATE_FIELD */

  if (nntp_copy_fields(article_number, cancel_fields,
		       sizeof(cancel_fields) / sizeof(struct cpy_hdr),
		       CF_CLR_MASK | CF_GET_MASK) != NNTP_OK) {
    return(1);
  }
  get_real_adrs(from_buff, buff1);

  /*
   * 自分の記事か調べる
   */

  ptr1 = from_buff;
  if (!strncmp(news_field, FROM_FIELD, sizeof(FROM_FIELD) - 1)) {
    ptr2 = &news_field[sizeof(FROM_FIELD) - 1];
    while ((*ptr2 == ' ') || (*ptr2 == '\t')) {
      ptr2++;
    }
    while (*ptr2) {
      if (!strncmp(ptr2, "\\n", 2)) {
	break;
      }
      *ptr1++ = *ptr2++;
    }
    *ptr1 = '\0';
  } else {
    if (user_gecos[0]) {
      if (from_mode) {
	sprintf(ptr1, "%s <%s@%s>", user_gecos, user_name, domain_name);
      } else {
	sprintf(ptr1, "%s@%s (%s)", user_name, domain_name, user_gecos);
      }
    } else {
      if (from_mode) {
	sprintf(ptr1, "<%s@%s>", user_name, domain_name);
      } else {
	sprintf(ptr1, "%s@%s", user_name, domain_name);
      }
    }
  }
  get_real_adrs(from_buff, buff2);
  if (strcmp(buff1, buff2)) {
    return(1);
  }
  if ((group_buff[0] == '0') || (message_buff[0] == '\0')) {
    return(1);
  }
  create_temp_name(tmp_file, "PS");
  fp = fopen2(tmp_file, "w");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't open news write file.");
    return(1);
  }
  fprintf(fp, "%s %s\n", FROM_FIELD, from_buff);
  fprintf(fp, "%s %s\n", GROUP_FIELD, group_buff);
#ifdef	ADD_MESSAGE_ID
  create_msg_id(buff1);
  fputs(buff1, fp);
#endif	/* ADD_MESSAGE_ID */
#ifdef	ADD_DATE_FIELD
  now_time = time((time_t*)NULL);
  tm = localtime(&now_time);
  create_date_field(buff1, tm);
  fputs(buff1, fp);
#endif	/* ADD_DATE_FIELD */
  if (dist_buff[0]) {
    fprintf(fp, "%s %s\n", DIST_FIELD, dist_buff);
  }
  fprintf(fp, "%s %s %s\n", SUBJECT_FIELD, CANCEL_CONTROL, message_buff);
  fprintf(fp, "%s %s %s\n\n", CONTROL_FIELD, CANCEL_CONTROL, message_buff);
  fclose(fp);
  if (!yes_or_no(CARE_YN_MODE, "記事をキャンセルしてよろしいですか?",
		 "Cancel article.Are you sure?")) {
    funlink2(tmp_file);
    return(1);
  }
  print_mode_line(japanese ? "キャンセル中です。" : "Cancelling.");
#ifdef	DEBUG
  sleep(5);
  status = NNTP_OK;
#else	/* !DEBUG */
  status = nntp_post(tmp_file, domain_name);
#endif	/* !DEBUG */
  funlink2(tmp_file);
  if (status != NNTP_OK) {
    print_mode_line(japanese ? "ニュースのキャンセルに失敗しました。" :
		    "Cancel article failed.");
    term_bell();
    sleep(ERROR_SLEEP);
    return(1);
  }
  return(0);
}

/*
 * ニュース投稿(下位関数)
 *
 * 戻り値:
 *	-3:MIME切替
 *	-2:再編集
 *	-1:中断
 *	 0:正常終了
 *	 1:エラー
 */

int	exec_post(base_file)
     char	*base_file;
{
  char		tmp_file1[PATH_BUFF];
  char		tmp_file2[PATH_BUFF];
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		buff3[BUFF_SIZE];
  char		buff4[BUFF_SIZE];
  char		field[SMALL_BUFF];
  FILE		*fp1, *fp2, *fp3;
  static struct cpy_hdr post_fields[] = {
    {FROM_FIELD,	from_buff,	PF_FROM_MASK},
    {DATE_FIELD,	date_buff,	PF_ETC_MASK},
    {SUBJECT_FIELD,	(char*)NULL,	PF_SUBJ_MASK},
    {GROUP_FIELD,	group_buff,	PF_NG_MASK},
    {DIST_FIELD,	dist_buff,	PF_ETC_MASK},
    {FOLLOWUP_FIELD,	(char*)NULL,	PF_ETC_MASK},
    {PATH_FIELD,	(char*)NULL,	PF_PATH_MASK},
    {ORGAN_FIELD,	(char*)NULL,	PF_ORGAN_MASK},
    {LINE_FIELD,	(char*)NULL,	PF_LINE_MASK},
    {X_NSUBJ_FIELD,	(char*)NULL,	PF_THROUGH_MASK},
#ifdef	FCC
    {FCC_FIELD,		fcc_buff,	PF_DELETE_MASK},
#endif	/* FCC */
#ifdef	MIME
    {MIME_VER_FIELD,	(char*)NULL,	PF_MIME_MASK},
    {CONTENT_TYPE_FIELD,(char*)NULL,	PF_CONTENT_MASK},
#endif	/* MIME */
#ifdef	MAILSEND
#ifdef	DELETE_FIELD
    {TO_FIELD,		(char*)NULL,	PF_MAIL_MASK | PF_DELETE_MASK},
    {CC_FIELD,		(char*)NULL,	PF_MAIL_MASK | PF_DELETE_MASK},
#else	/* !DELETE_FIELD */
    {TO_FIELD,		(char*)NULL,	PF_MAIL_MASK},
    {CC_FIELD,		(char*)NULL,	PF_MAIL_MASK},
#endif	/* !DELETE_FIELD */
    {BCC_FIELD,		(char*)NULL,	PF_MAIL_MASK | PF_DELETE_MASK},
#endif	/* MAILSEND */
#ifdef	MIME_XXX
    {X_BIN_FILE_FIELD,	bin_file_buff,	PF_FILE_MASK},
#endif	/* MIME_XXX */
  };
  char		*ptr1, *ptr2, *ptr3, *last;
  NNTP_ERR_CODE	status;
  int		status1, status2, status3, status4, status5;
  int		line;
  long		length, position;
#ifdef	ADD_DATE_FIELD
  struct tm		*tm;
  time_t		now_time;
#endif	/* ADD_DATE_FIELD */
  register int	i;

#ifdef	MAILSEND
  mail_flag = 0;
#endif	/* MAILSEND */
#if	defined(MSDOS) || defined(OS2)
  fp1 = fopen(base_file, "rb");				/* バイナリモード */
#else	/* !(MSDOS || OS2) */
  fp1 = fopen(base_file, "r");
#endif	/* !(MSDOS || OS2) */
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't open news read file.");
    return(1);
  }
  create_temp_name(tmp_file1, "PS");
  fp2 = fopen2(tmp_file1, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't open news write file.");
    fclose(fp1);
    return(1);
  }
  sprintf(tmp_file2, "%s%c%s", dot_dir, SLASH_CHAR, SIGNATURE_FILE);
  ptr3 = tmp_file2;
  while (*ptr3) {
    ptr3++;
  }

  /*
   * ヘッダ準備
   */

  status1 = status2 = status3 = status4 = 0;
#ifdef	SUPPORT_X0201
  reset_char_mask();
#endif	/* SUPPORT_X0201 */
#ifdef	MIME_XXX
  mime_init_content();
#endif	/* MIME_XXX */
  length = 0L;
  for (i = 0; i < sizeof(post_fields) / sizeof(struct cpy_hdr); i++) {
    if (post_fields[i].field_buff != (char*)NULL) {
      post_fields[i].field_buff[0] = '\0';
    }
  }
  last = (char*)NULL;
  while (fgets2(buff1, sizeof(buff1), fp1)) {
    if ((!*buff1) || (*buff1 == '\n')) {
      break;
    }
    if ((*buff1 == ' ') || (*buff1 == '\t')) {	/* 継続行処理	*/
      if (last && status4) {
	if ((strlen(buff2) + strlen(buff1)) < sizeof(buff2)) {
	  *last++ = '\n';
	  strcpy(last, buff1);
	  while (*last) {
	    if (*last == '\n') {
	      *last = '\0';
	      break;
	    }
	    last++;
	  }
	} else {
	  print_mode_line(japanese ?
			  "警告 : %s フィールドがオーバーフローしました。" :
			  "Warning : %s field overflow.", field);
	  term_bell();
	  sleep(ERROR_SLEEP);
	  status4 = 0;
	}
	continue;
      } else {
	continue;
      }
    } else {					/* 先頭行処理	*/
      if (last) {
	if (status3) {
	  if (status3 & PF_DELETE_MASK) {
	    status2 |= status3;
	  } else {
	    status3 |= PF_DELETE_MASK;
	    ptr1 = buff2;
	    while (*ptr1) {
	      if ((*ptr1 != ' ') && (*ptr1 != '\t') && (*ptr1 != '\n')) {
		status3 &= ~PF_DELETE_MASK;
		break;
	      }
	      ptr1++;
	    }
	  }
	}
#ifdef	MIME_XXX
	if ((!(status3 & PF_DELETE_MASK)) && (status3 & PF_FILE_MASK)) {
	  mime_add_content(M_MTYPE_APP, M_STYPE_OCT, M_TTYPE_B64, buff2);
	  status3 |= PF_DELETE_MASK;
	}
#endif	/* MIME_XXX */
	if (!(status3 & PF_DELETE_MASK)) {
	  status2 |= status3;
	  ptr1 = buff2;
	  while (1) {
	    ptr2 = buff3;
	    if (*field) {
	      sprintf(ptr2, "%s ", field);
	      while (*ptr2) {
		ptr2++;
	      }
	      *field = '\0';
	    }
	    while (*ptr1) {
	      if (*ptr1 == '\n') {
		ptr1++;
		break;
	      }
	      *ptr2++ = *ptr1++;
	    }
	    *ptr2 = '\0';
	    if (status3 & PF_THROUGH_MASK) {
	      to_jis(buff4, buff3, EUC_CODE);
	    } else {
	      status5 = 0;
	      if (mime_encode_func(buff4, buff3, EUC_CODE) < 0) {
		status5 = 1;
	      } else if ((!(status3 & PF_SUBJ_MASK)) &&
			 (discriminate_code(buff4) != ASCII_CODE)) {
		status5 = 1;
	      }
	      if (status5) {
		fclose(fp1);
		fclose(fp2);
		funlink2(tmp_file1);
		switch (yes_or_no(MIME_YN_MODE,
				  "不正な文字が含まれています。再編集しますか?",
				  "Illegal character set included. Edit again?")) {
		case 1:
		  return(-2);
		case 3:
		  return(-3);
		case 0:
		default:
		  return(-1);
		}
	      }
	    }

	    /*
	     * Newsgroups: フィールドの余分な空白を取り除く
	     * (イマイチなコードです)
	     */

	    if (status3 & PF_NG_MASK) {
	      ptr1 = strchr(buff4, ':');
	      if (ptr1 != (char*)NULL) {
		ptr1++;
		if ((*ptr1 == ' ') || (*ptr1 == '\t')) {
		  ptr1++;
		}
		while (*ptr1) {
		  if ((*ptr1 == ' ') || (*ptr1 == '\t')) {
		    memmove(ptr1, ptr1 + 1, strlen(ptr1));
		  } else {
		    ptr1++;
		  }
		}
	      }
	    }
	    fputs(buff4, fp2);
	    fputc('\n', fp2);
	    if (!*ptr1) {
	      break;
	    }
	  }
	}
	status3 = 0;
      }
    }
    if (!strncmp(buff1, TEXT_SEPARATER, sizeof(TEXT_SEPARATER) - 1)) {
      status1 = 1;
      break;
    } else {
      last = (char*)NULL;
      ptr1 = buff1;
      ptr2 = field;
      status4 = 1;
      while (*ptr1) {
	if (*ptr1 == ':') {
	  *ptr2++ = *ptr1++;
	  last = buff2;
	  break;
	}
	*ptr2++ = *ptr1++;
      }
      *ptr2 = '\0';
      if (last) {
	while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
	  ptr1++;
	}
	while (*ptr1) {
	  if (*ptr1 == '\n') {
	    break;
	  }
	  *last++ = *ptr1++;
	}
	*last = '\0';
	for (i = 0; i < sizeof(post_fields) / sizeof(struct cpy_hdr); i++) {
	  if (!strncmp(post_fields[i].field_name, field,
		       strlen(post_fields[i].field_name))) {
	    if (post_fields[i].field_buff != (char*)NULL) {
	      strcpy(post_fields[i].field_buff, buff2);
	    }
	    status3 = post_fields[i].buff_size;
#ifdef	MAILSEND
	    if (status3 & PF_MAIL_MASK) {
	      mail_flag = 1;
	    }
#endif	/* MAILSEND */
	    break;
	  }
	}
      }
    }
  }

  if ((!status1) || ((status2 & (PF_NG_MASK | PF_SUBJ_MASK)) !=
		     (PF_NG_MASK | PF_SUBJ_MASK))) {
    fclose(fp1);
    fclose(fp2);
    funlink2(tmp_file1);
    return(1);
  }
  fp3 = get_news_signature(group_buff, dist_buff);

  /*
   * .signature の漢字コードを調べ、行数を数える。
   */

  status3 = 0;
  line = 0;
  if (fp3 != (FILE*)NULL) {
    while (fgets2(buff1, sizeof(buff1), fp3)) {
#ifdef	MIME
      if (discriminate_code(buff1) != ASCII_CODE) {
	status3 = 1;
      }
#endif	/* MIME */
      line++;
    }
    if ((!nntp_connect_mode) && inews_sig_mode) {
      fclose(fp3);
      fp3 = (FILE*)NULL;
    } else {
      fseek(fp3, 0L, 0);
    }
  }
  if (!(status2 & PF_FROM_MASK)) {	/* From: フィールドを作成する */

    /*
     * 後で Fcc: 用に使用するために一旦 from_buff に展開する
     */

    sprintf(from_buff, "%s@%s", user_name, domain_name);
    if (user_gecos[0]) {
      if (from_mode) {
	length += (long)fprintf2(fp2, "%s %s <%s>\n", FROM_FIELD,
				 user_gecos, from_buff);
      } else {
	length += (long)fprintf2(fp2, "%s %s (%s)\n", FROM_FIELD,
				 from_buff, user_gecos);
      }
    } else {
      if (from_mode) {
	length += (long)fprintf2(fp2, "%s <%s>\n", FROM_FIELD, from_buff);
      } else {
	length += (long)fprintf2(fp2, "%s %s\n", FROM_FIELD, from_buff);
      }
    }
  }
  if (!(status2 & PF_PATH_MASK)) {	/* Path: フィールドを作成する */
#ifdef	ADD_HOSTPATH
    gethostname(buff1, sizeof(buff1));
    length += (long)fprintf2(fp2, "%s %s!%s\n", PATH_FIELD, buff1,
			     user_name);
#else	/* !ADD_HOSTPATH */
    length += (long)fprintf2(fp2, "%s %s\n", PATH_FIELD, user_name);
#endif	/* !ADD_HOSTPATH */
  }
  if ((!(status2 & PF_ORGAN_MASK)) && organization[0]) {
				/* Organization: フィールドを作成する */
    length += (long)fprintf2(fp2, "%s %s\n", ORGAN_FIELD, organization);
  }
  if ((!(status2 & PF_LINE_MASK)) || (mime_format == 2)) {
    position = ftell(fp1);
    while (fgets(buff1, sizeof(buff1), fp1)) {
#ifdef	MIME
      if (discriminate_code(buff1) != ASCII_CODE) {
	status3 = 1;
      }
#endif	/* MIME */
      line++;
    }
    fseek(fp1, position, 0);
  }
  if ((!(status2 & PF_LINE_MASK)) && add_lines_mode) {
					/* Lines: フィールドを作成する */
    length += (long)fprintf2(fp2, "%s %d\n", LINE_FIELD, line);
  }
#ifdef	MIME
  if (mime_format == 2) {		/* MIME 関連フィールドを作成する */
    if (!(status2 & PF_MIME_MASK)) {
      length += (long)fprintf2(fp2, "%s %s\n", MIME_VER_FIELD,
			       MIME_VERSION);
    }
    if (!(status2 & PF_CONTENT_MASK)) {
      length += (long)fprintf2(fp2, "%s %s/%s; %s%s\n",
			       CONTENT_TYPE_FIELD, MIME_MTYPE_TEXT,
			       MIME_STYPE_PLAIN, MIME_CHARSET,
			       status3 ? MIME_CHARSET_ISO2022_1 :
			       MIME_CHARSET_ASCII);
    }
  }
#endif	/* MIME */
#ifdef	ADD_MESSAGE_ID
  create_msg_id(buff1);
  fputs(buff1, fp2);
#endif	/* ADD_MESSAGE_ID */
#ifdef	ADD_DATE_FIELD
  now_time = time((time_t*)NULL);
  tm = localtime(&now_time);
  create_date_field(buff1, tm);
  fputs(buff1, fp2);
#endif	/* ADD_DATE_FIELD */
#ifndef	DONT_ADD_XFIELD
  length += (long)fprintf2(fp2, "%s %s [version %s] %s\n",
			   X_READER_FIELD, READER_NAME,
			   MNEWS_VERSION, MNEWS_DATE);
#endif	/* !DONT_ADD_XFIELD */
  fputc('\n', fp2);

  /*
   * 本文の漢字コード変換
   */

  while (fgets2(buff1, sizeof(buff1), fp1)) {
#ifdef	EUC_SPOOL
    to_euc(buff2, buff1, EUC_CODE);
#else	/* !EUC_SPOOL */
    to_jis(buff2, buff1, EUC_CODE);
#endif	/* !EUC_SPOOL */
    fputs(buff2, fp2);
    length += (long)strlen(buff2);
  }
  fclose(fp1);

  /*
   * .signature の付加
   */

  if (fp3 != (FILE*)NULL) {
    while (fgets2(buff1, sizeof(buff1), fp3)) {
#ifdef	EUC_SPOOL
      to_euc(buff2, buff1, default_code);
#else	/* !EUC_SPOOL */
      to_jis(buff2, buff1, default_code);
#endif	/* !EUC_SPOOL */
      fputs(buff2, fp2);
      length += (long)strlen(buff2);
    }
    fclose(fp3);
  }
  fclose(fp2);

  /*
   * ユーザ定義フィルタ実行
   */

#ifdef	LARGE
  if (*post_filter) {
    create_temp_name(tmp_file2, "PU");
    sprintf(buff1, "%s < %s > %s", post_filter, tmp_file1, tmp_file2);
    if (system(buff1)) {
      print_fatal("Post filter error.");
      unlink(tmp_file2);
    } else {
      chmod(tmp_file2, S_IREAD | S_IWRITE);
#if	defined(MSDOS) || defined(OS2)
      unlink(tmp_file1);
#endif	/* (MSDOS || OS2) */
      rename(tmp_file2, tmp_file1);
    }
  }
#endif	/* LARGE */

  /*
   * 投稿直前の記事を表示
   */

#ifdef	MAX_POST_SIZE
  if (length >= (long)MAX_POST_SIZE) {
    funlink2(tmp_file1);
    term_bell();
    if (yes_or_no(NORMAL_YN_MODE,
		  "記事が大き過ぎます。再編集しますか?",
		  "Too large article. Edit again?")) {
      return(-2);
    } else {
      return(-1);
    }
  }
#endif	/* MAX_POST_SIZE */
#ifdef	CONFIRM_FIELD
  term_clear_all();
  fp1 = fopen(tmp_file1, "r");
  if (fp1 != (FILE*)NULL) {
    while (fgets(buff1, sizeof(buff1), fp1)) {
      if ((!buff1[0]) || (buff1[0] == '\n')) {
	break;
      }
#ifdef	EUC_SPOOL
      euc_printf("%s\r", buff1);
#else	/* !EUC_SPOOL */
      jis_printf("%s\r", buff1);
#endif	/* !EUC_SPOOL */
    }
    fclose(fp1);
  }
#else	/* !CONFIRM_FIELD */
  create_temp_name(tmp_file2, "PV");
  exec_pager(tmp_file1, tmp_file2, 3, japanese ? "投稿確認" : "CONFIRM");
  last_key = 0;
#endif	/* !CONFIRM_FIELD */
#ifdef	SUPPORT_X0201
  if (char_mask & (KANA_MASK | HOJO_MASK)) {
    print_mode_line(japanese ?
		    "警告 : 通常許されていない文字が含まれています。" :
		    "Warning : Normally, unallowed character included.");
    term_bell();
    sleep(ERROR_SLEEP);
  }
#endif	/* SUPPORT_X0201 */

  /*
   * ポストを実行
   */

  switch (yes_or_no(SEND_YN_MODE, "記事を投稿してよろしいですか?",
		    "Post article.Are you sure?")) {
  case 1:
    break;
  case 2:
    funlink2(tmp_file1);
    return(-2);
  case 3:
    funlink2(tmp_file1);
    return(-3);
  case 4:
  case 0:
  default:
    funlink2(tmp_file1);
    return(-1);
  }
#ifdef	FCC
  if (fcc_buff[0]) {
    if (fcc_save(tmp_file1, from_buff, date_buff, fcc_buff)) {
      if (yes_or_no(NORMAL_YN_MODE,
		    "FCC ファイルがセーブできませんでした。再編集しますか?",
		    "Failed for FCC file save.Edit Again?")) {
	funlink2(tmp_file1);
	return(-2);
      }
    }
  }
#endif	/* FCC */
  print_mode_line(japanese ? "投稿中です。" : "Posting.");
  status = NNTP_OK;
#ifdef	DEBUG
  sleep(5);
#else	/* !DEBUG */
  if (nntp_connect_mode == 1) {
    nntp_noop();	/* 二度 noop しないと失敗することがあるための対策 */
    if (nntp_noop() != NNTP_OK) {
      print_mode_line(japanese ? "再接続中です。" : "Reconnecting.");
      status = open_nntp_server(1);
    }
  }
  if (status == NNTP_OK) {
    status = nntp_post(tmp_file1, domain_name);
  }
#endif	/* !DEBUG */
  if (status != NNTP_OK) {
    print_fatal("Post article failed.");
    sprintf(tmp_file2, "%s%c%s", dot_dir, SLASH_CHAR, DEAD_FILE);
    fp1 = fopen(tmp_file1, "r");
    if (fp1 != (FILE*)NULL) {
      fp2 = fopen(tmp_file2, "w");
      if (fp2 != (FILE*)NULL) {
	chmod(tmp_file2, S_IREAD | S_IWRITE);
	while (fgets(buff1, sizeof(buff1), fp1)) {
	  fputs(buff1, fp2);
	}
	fclose(fp2);
      }
      fclose(fp1);
    }
    funlink2(tmp_file1);
    return(1);
  }
  funlink2(tmp_file1);
  return(0);
}

/*
 * signature ファイル取得(ニュース用)
 */

static FILE	*get_news_signature(group, distribution)
    char	*group;
    char	*distribution;
{
  FILE	*fp;
  char	signature_file[PATH_BUFF];
  char	*ptr1, *ptr2;

  sprintf(signature_file, "%s%c%s", dot_dir, SLASH_CHAR, SIGNATURE_FILE);
  ptr2 = signature_file;
  while (*ptr2) {
    ptr2++;
  }
  if ((!nntp_connect_mode) && inews_sig_mode) {
			/* ローカル接続では inews により強制的に付加される */
    return(fopen(signature_file, "r"));
  }
  while (1) {
    *ptr2 = '+';
    strcpy(ptr2 + 1, group);
    ptr1 = strchr(ptr2, ',');
    if (ptr1 != (char*)NULL) {
      *ptr1 = '\0';
    }
    fp = fopen(signature_file, "r");
    if (fp != (FILE*)NULL) {
      break;
    }
    ptr1 = strrchr(group, NEWS_GROUP_SEPARATER);
    if (ptr1 != (char*)NULL) {
      *ptr1 = '\0';
    } else {
      break;
    }
  }
  if (fp == (FILE*)NULL) {
    *ptr2 = '-';
    strcpy(ptr2 + 1, distribution);
    fp = fopen(signature_file, "r");
    if (fp == (FILE*)NULL) {
      *ptr2 = '\0';
      fp = fopen(signature_file, "r");
    }
  }
  if (fp != (FILE*)NULL) {
    if ((!auto_sig_mode) &&
	(!yes_or_no(NORMAL_YN_MODE, "signature%s ファイルをつけますか?",
		    "Add signature%s file.Are you sure?", ptr2))) {
      fclose(fp);
      fp = (FILE*)NULL;
    }
  }
  return(fp);
}
#endif	/* NEWSPOST */
