/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Mail send routine
 *  File        : mailsend.c
 *  Version     : 1.21
 *  First Edit  : 1992-07/29
 *  Last  Edit  : 1997-12/23
 *  Author      : MSR24 宅間 顯
 *
 */

#ifdef	MAILSEND
#include	"compat.h"
#include	"nntplib.h"
#include	"smtplib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"termlib.h"
#include	"mnews.h"
#include	"article.h"
#include	"mailsend.h"
#include	"mh.h"
#include	"pager.h"
#include	"fcc.h"
#include	"mimelib.h"
#ifdef	MIME_XXX
#include	"mimepart.h"
#endif	/* MIME_XXX */

static FILE	*get_mail_signature();	/* signature ファイル取得	*/
static int	expand_field();		/* エイリアス展開(上位関数)	*/
static int	ucbmail_expand_alias();	/* エイリアス展開(下位関数)	*/
#ifdef	notdef
static int	mh_expand_alias();	/* エイリアス展開(下位関数)	*/
#endif	/* notdef */

static char	from_buff[MAX_FIELD_LEN];
static char	date_buff[MAX_FIELD_LEN];
static char	message_buff[MAX_FIELD_LEN];
static char	subject_buff[MAX_FIELD_LEN];
static char	group_buff[MAX_FIELD_LEN];
static char	reply_buff[MAX_FIELD_LEN];
static char	x_nsubj_buff[MAX_FIELD_LEN];
#ifdef	FCC
static char	fcc_buff[MAX_FIELD_LEN];
#endif	/* FCC */
static char	to_buff[ALIAS_BUFF];
static char	cc_buff[ALIAS_BUFF];
static char	app_buff[ALIAS_BUFF];
static char	resent_from_buff[MAX_FIELD_LEN];
static char	resent_to_buff[ALIAS_BUFF];
static char	resent_cc_buff[ALIAS_BUFF];
static char	resent_reply_buff[MAX_FIELD_LEN];
#ifdef	RECOVER_SUBJECT
static char	recover_buff[BUFF_SIZE];
#endif	/* RECOVER_SUBJECT */
#ifdef	LARGE
static char	sender_buff[MAX_FIELD_LEN];
static char	in_reply_buff[BUFF_SIZE];
#endif	/* LARGE */
#ifdef	MIME_XXX
static char	bin_file_buff[PATH_BUFF];
#endif	/* MIME_XXX */

/*
 * メール送信
 */

int	mail_send(adrs, folder)
     char	*adrs;
     char	*folder;
{
  FILE	*fp;
  char	tmp_file[PATH_BUFF];
  char	backup_file[PATH_BUFF];
  char	to_buff[MAX_FIELD_LEN];
  char	cc_buff[MAX_FIELD_LEN];
  char	subject_buff[MAX_FIELD_LEN];
  int	first;
  int	status;

#ifdef	SEND_CONFIRM
  if (!yes_or_no(NORMAL_YN_MODE, "新しい記事を発信しますか?",
		 "Send new article.Are you sure?")) {
    return(1);
  }
#endif	/* SEND_CONFIRM */
  strcpy(to_buff, adrs);
  input_line(0, "主宛先(to)を入力して下さい:",
	     "Input recipient(to) address:", to_buff);
  if (!to_buff[0]) {
    return(1);
  }
  if (add_cc_mode) {
    sprintf(cc_buff, "%s@%s", user_name, domain_name);
  } else {
    cc_buff[0] = '\0';
  }
  input_line(0, "コピー宛先(cc)を入力して下さい:",
	     "Input carbon copy(cc) address:", cc_buff);
  subject_buff[0] = '\0';
  input_line(0, "題名(subject)を入力して下さい:",
	     "Input subject:", subject_buff);
  sprintf(tmp_file, "%s%c%s", dot_dir, SLASH_CHAR, MESSAGE_FILE);
  create_backup_name(backup_file, tmp_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  rename(tmp_file, backup_file);
  fp = fopen(tmp_file, "w");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't open mail write file.");
    return(1);
  }
  chmod(tmp_file, S_IREAD | S_IWRITE);
  if (mail_field[0]) {
    edit_fprintf(fp, "%s\n", mail_field);
  }
  fprintf(fp, "%s %s\n", TO_FIELD, to_buff);
  fprintf(fp, "%s %s\n", CC_FIELD, cc_buff);
  fprintf(fp, "%s \n", BCC_FIELD);
  edit_fprintf(fp, "%s %s\n", SUBJECT_FIELD, subject_buff);
  if (x_nsubj_mode) {
    if (discriminate_code(subject_buff) != ASCII_CODE) {
      edit_fprintf(fp, "%s %s\n", X_NSUBJ_FIELD, subject_buff);
    } else {
      edit_fprintf(fp, "%s \n", X_NSUBJ_FIELD);
    }
  }
#ifdef	FCC
  if (mail_fcc_field[0]) {
    edit_fprintf(fp, "%s %s\n", FCC_FIELD, mail_fcc_field);
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
      status = exec_send(tmp_file, folder);
      if (status != -3) {
	break;
      }
    }
    if (status) {
      if (status > 0) {
	print_mode_line(japanese ? "メールの送信に失敗しました。" :
			"Send mail failed.");
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
 * メール返信
 */

int	mail_reply(base_file, mask, folder, adrs)
     char	*base_file;
     int	mask;
     char	*folder;
     char	*adrs;
{
  FILE			*fp1, *fp2;
  char			tmp_file[PATH_BUFF];
  char			backup_file[PATH_BUFF];
  char			reply_file[PATH_BUFF];
  char			buff1[BUFF_SIZE];
  char			buff2[BUFF_SIZE];
  static struct cpy_hdr	reply_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
    {DATE_FIELD,	date_buff,	sizeof(date_buff)},
    {MESSAGE_FIELD,	message_buff,	sizeof(message_buff)},
#ifdef	RECOVER_SUBJECT
    {SUBJECT_FIELD,	recover_buff,	sizeof(recover_buff)},
#else	/* !RECOVER_SUBJECT */
    {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
#endif	/* !RECOVER_SUBJECT */
    {TO_FIELD,		to_buff,	sizeof(to_buff)},
    {CC_FIELD,		cc_buff,	sizeof(cc_buff)},
    {GROUP_FIELD,	group_buff,	sizeof(group_buff)},
    {APP_TO_FIELD,	app_buff,	sizeof(app_buff)},
    {REPLY_FIELD,	reply_buff,	sizeof(reply_buff)},
    {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
    {RESENT_FROM_FIELD,	resent_from_buff,	sizeof(resent_from_buff)},
    {RESENT_TO_FIELD,	resent_to_buff,		sizeof(resent_to_buff)},
    {RESENT_CC_FIELD,	resent_cc_buff,		sizeof(resent_cc_buff)},
    {RESENT_REPLY_FIELD,resent_reply_buff,	sizeof(resent_reply_buff)},
  };
  char			*ptr1, *ptr2;
  int			first;
  int			status;
  int			length;
  int			i;

#ifdef	SEND_CONFIRM
  if (!yes_or_no(NORMAL_YN_MODE, "記事を返信しますか?",
		 "Send reply article.Are you sure?")) {
    return(1);
  }
#endif	/* SEND_CONFIRM */
  print_mode_line(japanese ? "変換中です。" : "Converting.");
  create_temp_name(reply_file, "RD");
  convert_code(base_file, reply_file, EUC_CODE, 4);
#if	defined(MSDOS) || defined(OS2)
  fp1 = fopen(reply_file, "rb");			/* バイナリモード */
#else	/* !(MSDOS || OS2) */
  fp1 = fopen(reply_file, "r");
#endif	/* !(MSDOS || OS2) */
  funlink2(reply_file);
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't open mail read file.");
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
    print_fatal("Can't open mail write file.");
    fclose(fp1);
    return(1);
  }
  chmod(tmp_file, S_IREAD | S_IWRITE);
  if (mail_field[0]) {
    edit_fprintf(fp2, "%s\n", mail_field);
  }
  copy_fields(fp1, reply_fields,
	      sizeof(reply_fields) / sizeof(struct cpy_hdr),
	      CF_CLR_MASK | CF_GET_MASK | CF_ADD_MASK | CF_SPC_MASK);
  if (resent_from_buff[0] && (!from_buff[0])) {
    strcpy(from_buff, resent_from_buff);
  }
  if (resent_to_buff[0] && (!to_buff[0])) {
    strcpy(to_buff, resent_to_buff);
  }
  if (resent_cc_buff[0] && (!cc_buff[0])) {
    strcpy(cc_buff, resent_cc_buff);
  }
  if (resent_reply_buff[0] && (!reply_buff[0])) {
    strcpy(reply_buff, resent_reply_buff);
  }
  if (!to_buff[0]) {
    strcpy(to_buff, app_buff);
  }
  if (!reply_buff[0]) {
    strcpy(reply_buff, from_buff);
  }
  ptr1 = reply_buff;
  ptr2 = buff1;
  while (*ptr1) {
    if (ptr2 != buff1) {
      *ptr2++ = ',';
      *ptr2++ = ' ';
    }
    ptr1 = get_real_adrs(ptr1, ptr2);
    while (*ptr2) {
      ptr2++;
    }
  }
  strcpy(reply_buff, buff1);
  sprintf(buff2, "%s@%s", user_name, domain_name);
  ptr1 = to_buff;
  if (mask & FOLLOW_BOARD_MASK) {	/* ボードフォローモード */
    fprintf(fp2, "%s %s\n", TO_FIELD, adrs);
    fprintf(fp2, "%s ", CC_FIELD);
  } else if (mask & REPLY_BOARD_MASK) {	/* ボードリプライモード	*/
    fprintf(fp2, "%s %s\n", TO_FIELD, reply_buff);
    fprintf(fp2, "%s ", CC_FIELD);
    if (add_cc_mode) {
      fprintf(fp2, "%s", buff2);
    }
  } else {				/* 通常のリプライモード	*/
    fprintf(fp2, "%s %s\n", TO_FIELD, reply_buff);
    length = fprintf2(fp2, "%s ", CC_FIELD);
    status = 0;
    first = -1;
    while (status < 3) {
      ptr2 = get_real_adrs(ptr1, buff1);
      ptr1 = buff1;

      /*
       * Cc: が自分か To: に等しいなら Cc: に加えるのは飛ばす
       */

      if (*ptr1) {
	if ((strcmp(ptr1, buff2) && strcmp(ptr1, user_name) &&
	     strcmp(ptr1, reply_buff)) || (status > 1)) {
	  if (first == -1) {
	    first++;
	  } else {
	    if ((length + (int)strlen(ptr1)) >= MAX_FIELD_COLUMN) {
	      if (++first % 4) {
		fprintf(fp2, ",\n\t");
		length = 8;		/* TAB column */
	      } else {
		fputc('\n', fp2);
		length = fprintf2(fp2, "%s ", CC_FIELD);
	      }
	    } else {
	      length += fprintf2(fp2, ", ");
	    }
	  }
	  length += fprintf2(fp2, "%s", ptr1);
	}
      }
      ptr1 = ptr2;
      if (!(*ptr1)) {
	switch (status++) {
	case 0:		/* Cc: アドレスを Cc: フィールドへ足す */
	  if (reply_cc_mode) {
	    ptr1 = cc_buff;
	  } else {
	    ptr1 = "";
	  }
	  break;
	case 1:		/* 自分のアドレスを Cc: フィールドの最後へ足す */
	  if (add_cc_mode) {
	    ptr1 = buff2;
	  }
	  break;
	default:
	  break;
	}
      }
    }
  }
  fprintf(fp2, "\n%s \n", BCC_FIELD);
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
  if (message_buff[0]) {
    fprintf(fp2, "%s %s\"%s\".\n\t%s\n", IN_REPLY_FIELD, REPLY_MESSAGE,
	    date_buff, message_buff);
  }
#ifdef	FCC
  if (mail_fcc_field[0]) {
    fprintf(fp2, "%s %s\n", FCC_FIELD, mail_fcc_field);
  }
#endif	/* FCC */
#ifdef	MIME_XXX
  if (mime_format == 2) {
    fprintf(fp2, "%s \n", X_BIN_FILE_FIELD);
  }
#endif	/* MIME_XXX */
  fprintf(fp2, "%s\n", TEXT_SEPARATER);
  if (mask & REPLY_QUOTE_MASK) {	/* 引用	*/
    fseek(fp1, 0L, 0);
    while (1) {
      copy_fields(fp1, reply_fields,
		  sizeof(reply_fields) / sizeof(struct cpy_hdr),
		  CF_CLR_MASK | CF_GET_MASK | CF_ADD_MASK | CF_SPC_MASK);
#ifdef	RECOVER_SUBJECT
      recover_jis(subject_buff, recover_buff);
#endif	/* RECOVER_SUBJECT */

      /*
       * フィールド検出チェック
       */

      for (i = 0; i < (sizeof(reply_fields) / sizeof(struct cpy_hdr)); i++) {
	if (reply_fields[i].field_buff[0]) {
	  i = -1;
	  break;
	}
      }
      if (i >= 0) {
	break;
      }

      if (mask & (FOLLOW_BOARD_MASK | REPLY_BOARD_MASK)) {
	strcpy(group_buff, adrs);	/* BOARD 用 %N 展開	*/
	ptr1 = group_buff;
	while (*ptr1) {
	  if (*ptr1 == '_') {
	    *ptr1 = '.';
	  }
	  ptr1++;
	}
      }
      ptr1 = (japanese ? "%Iの記事において\n%F{さん,私}は書きました。\n" :
	      "In article %I\n%F writes:\n");
#ifdef	SJIS_SRC
      sjis_to_euc(buff1, ptr1);
      ptr1 = buff1;
#endif	/* SJIS_SRC */
      if (group_buff[0]) {
	if (follow_message[0]) {
	  ptr1 = follow_message;
	}
      } else {
	if (reply_message[0]) {
	  ptr1 = reply_message;
	}
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
      status = exec_send(tmp_file, folder);
      if (status != -3) {
	break;
      }
    }
    if (status) {
      if (status > 0) {
	print_mode_line(japanese ? "メールの返信に失敗しました。" :
			"Send reply mail failed.");
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
 * メール転送
 */

int	mail_forward(base_file, folder)
     char	*base_file;
     char	*folder;
{
  FILE			*fp1, *fp2;
  char			tmp_file[PATH_BUFF];
  char			backup_file[PATH_BUFF];
  char			forward_file[PATH_BUFF];
  char			buff1[BUFF_SIZE];
  char			buff2[BUFF_SIZE];
  static struct cpy_hdr	forward_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
    {DATE_FIELD,	date_buff,	sizeof(date_buff)},
    {MESSAGE_FIELD,	message_buff,	sizeof(message_buff)},
    {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
    {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
    {GROUP_FIELD,	group_buff,	sizeof(group_buff)},
  };
#ifdef	LARGE
  char			*ptr;
#endif	/* LARGE */
  int			status;
  int			first;
  int			i;
  unsigned long		position;

#ifdef	SEND_CONFIRM
  if (!yes_or_no(NORMAL_YN_MODE, "記事を転送しますか?",
		 "Forward article.Are you sure?")) {
    return(1);
  }
#endif	/* SEND_CONFIRM */
  to_buff[0] = cc_buff[0] = '\0';
  input_line(0, "主宛先(to)を入力して下さい:",
	     "Input recipient(to) address:", to_buff);
  if (!to_buff[0]) {
    return(1);
  }
  input_line(0, "コピー宛先(cc)を入力して下さい:",
	     "Input carbon copy(cc) address:", cc_buff);
  print_mode_line(japanese ? "変換中です。" : "Converting.");
  create_temp_name(forward_file, "TD");
  convert_code(base_file, forward_file, EUC_CODE, 4);
#if	defined(MSDOS) || defined(OS2)
  fp1 = fopen(forward_file, "rb");			/* バイナリモード */
#else	/* !(MSDOS || OS2) */
  fp1 = fopen(forward_file, "r");
#endif	/* !(MSDOS || OS2) */
  funlink2(forward_file);
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't open mail read file.");
    return(1);
  }
  sprintf(tmp_file, "%s%c%s", dot_dir, SLASH_CHAR, MESSAGE_FILE);
  create_backup_name(backup_file, tmp_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  rename(tmp_file, backup_file);
  first = 1;
  while (1) {
    fp2 = fopen(tmp_file, "w");
    if (fp2 == (FILE*)NULL) {
      print_fatal("Can't open mail write file.");
      fclose(fp1);
      return(1);
    }
    chmod(tmp_file, S_IREAD | S_IWRITE);
    if (mail_field[0]) {
      fprintf(fp2, "%s\n", mail_field);
    }
    position = ftell(fp1);
    copy_fields(fp1, forward_fields,
		sizeof(forward_fields) / sizeof(struct cpy_hdr),
		CF_CLR_MASK | CF_GET_MASK);

    /*
     * フィールド検出チェック
     */

    for (i = 0; i < (sizeof(forward_fields) / sizeof(struct cpy_hdr)); i++) {
      if (forward_fields[i].field_buff[0]) {
	i = -1;
	break;
      }
    }
    if (i >= 0) {
      fclose(fp2);
      fclose(fp1);
      return(0);
    }
    fprintf(fp2, "%s %s\n", TO_FIELD, to_buff);
    fprintf(fp2, "%s %s\n", CC_FIELD, cc_buff);
    fprintf(fp2, "%s \n", BCC_FIELD);
    get_real_adrs(from_buff, buff1);
    edit_fprintf(fp2, "%s [%s: %s]\n", SUBJECT_FIELD, buff1, subject_buff);
    if (x_nsubj_mode && x_nsubj_buff[0]) {
      recover_jis(subject_buff, x_nsubj_buff);
      to_euc(buff2, subject_buff, EUC_CODE);
      edit_fprintf(fp2, "%s [%s: %s]\n", X_NSUBJ_FIELD, buff1, buff2);
    }
#ifdef	FCC
    if (mail_fcc_field[0]) {
      fprintf(fp2, "%s %s\n", FCC_FIELD, mail_fcc_field);
    }
#endif	/* FCC */
#ifdef	MIME_XXX
    if (mime_format == 2) {
      fprintf(fp2, "%s \n", X_BIN_FILE_FIELD);
    }
#endif	/* MIME_XXX */
    fprintf(fp2, "%s\n", TEXT_SEPARATER);
#ifdef	LARGE
    ptr = FORWARD_START_MESSAGE;
    if (forward_start[0]) {
      ptr = forward_start;
    }
    field_fprintf(fp2, ptr,
		  from_buff, subject_buff, message_buff,
		  group_buff, date_buff);
    fputc('\n', fp2);
#else	/* LARGE */
    fprintf(fp2, "%s\n", FORWARD_START_MESSAGE);
#endif	/* LARGE */
    if (fseek(fp1, position, 0)) {
      fclose(fp2);
      break;
    }
    while (fgets2(buff1, sizeof(buff1), fp1)) {
      if (!strcmp(buff1, MULTI_END_MARK)) {
	break;
      }
      to_euc(buff2, buff1, EUC_CODE);
      if (buff2[0] == '-') {
	fprintf(fp2, "- ");
      }
      edit_fprintf(fp2, "%s", buff2);
    }
#ifdef	LARGE
    ptr = FORWARD_END_MESSAGE;
    if (forward_end[0]) {
      ptr = forward_end;
    }
    field_fprintf(fp2, ptr,
		  from_buff, subject_buff, message_buff,
		  group_buff, date_buff);
    fputc('\n', fp2);
#else	/* !LARGE */
    fprintf(fp2, "%s\n", FORWARD_END_MESSAGE);
#endif	/* !LARGE */
    fclose(fp2);
  EDIT:
    if ((!first) || (!exec_editor(tmp_file, 0))) {
      first = 0;
      while (1) {
	status = exec_send(tmp_file, folder);
	if (status != -3) {
	  break;
	}
      }
      if (status) {
	if (status > 0) {
	  print_mode_line(japanese ? "メールの転送に失敗しました。" :
			  "Forward mail failed.");
	  term_bell();
	  sleep(ERROR_SLEEP);
	} else if (status == -2) {
	  first = 1;
	  goto EDIT;
	}
	break;
      }
    } else {
      break;
    }
    if (feof(fp1)) {
      fclose(fp1);
      return(0);
    }
    term_clear_all();
  }
  fclose(fp1);
  return(1);
}

#ifdef	LARGE
/*
 * メール回送
 */

int	mail_relay(base_file, folder)
     char	*base_file;
     char	*folder;
{
  FILE			*fp1, *fp2;
  char			tmp_file[PATH_BUFF];
  char			backup_file[PATH_BUFF];
  char			relay_file[PATH_BUFF];
  char			buff1[BUFF_SIZE];
  char			buff2[BUFF_SIZE];
  char			to_buff2[SMALL_BUFF];
  char			cc_buff2[SMALL_BUFF];
  static struct cpy_hdr	relay_fields[] = {
    {FROM_FIELD,	from_buff,	sizeof(from_buff)},
    {DATE_FIELD,	date_buff,	sizeof(date_buff)},
    {MESSAGE_FIELD,	message_buff,	sizeof(message_buff)},
    {SENDER_FIELD,	sender_buff,	sizeof(sender_buff)},
    {SUBJECT_FIELD,	subject_buff,	sizeof(subject_buff)},
    {TO_FIELD,		to_buff,	sizeof(to_buff)},
    {CC_FIELD,		cc_buff,	sizeof(cc_buff)},
    {GROUP_FIELD,	group_buff,	sizeof(group_buff)},
    {APP_TO_FIELD,	app_buff,	sizeof(app_buff)},
    {REPLY_FIELD,	reply_buff,	sizeof(reply_buff)},
    {X_NSUBJ_FIELD,	x_nsubj_buff,	sizeof(x_nsubj_buff)},
    {RESENT_FROM_FIELD,	resent_from_buff,	sizeof(resent_from_buff)},
    {RESENT_TO_FIELD,	resent_to_buff,		sizeof(resent_to_buff)},
    {RESENT_CC_FIELD,	resent_cc_buff,		sizeof(resent_cc_buff)},
    {RESENT_REPLY_FIELD,resent_reply_buff,	sizeof(resent_reply_buff)},
    {IN_REPLY_FIELD,	in_reply_buff,		sizeof(in_reply_buff)},
  };
  int			status;
  int			i;

#ifdef	SEND_CONFIRM
  if (!yes_or_no(NORMAL_YN_MODE, "記事を回送しますか?",
		 "Relay article.Are you sure?")) {
    return(1);
  }
#endif	/* SEND_CONFIRM */
  to_buff2[0] = cc_buff2[0] = '\0';
  input_line(0, "主宛先(to)を入力して下さい:",
	     "Input recipient(to) address:", to_buff2);
  if (!to_buff2[0]) {
    return(1);
  }
  input_line(0, "コピー宛先(cc)を入力して下さい:",
	     "Input carbon copy(cc) address:", cc_buff2);
  print_mode_line(japanese ? "変換中です。" : "Converting.");
  create_temp_name(relay_file, "LD");
  convert_code(base_file, relay_file, EUC_CODE, 4);
#if	defined(MSDOS) || defined(OS2)
  fp1 = fopen(relay_file, "rb");			/* バイナリモード */
#else	/* !(MSDOS || OS2) */
  fp1 = fopen(relay_file, "r");
#endif	/* !(MSDOS || OS2) */
  funlink2(relay_file);
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't open mail read file.");
    return(1);
  }
  sprintf(tmp_file, "%s%c%s", dot_dir, SLASH_CHAR, MESSAGE_FILE);
  create_backup_name(backup_file, tmp_file);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);
#endif	/* (MSDOS || OS2) */
  rename(tmp_file, backup_file);
  while (1) {
    fp2 = fopen(tmp_file, "w");
    if (fp2 == (FILE*)NULL) {
      print_fatal("Can't open mail write file.");
      fclose(fp1);
      return(1);
    }
    chmod(tmp_file, S_IREAD | S_IWRITE);
    if (mail_field[0]) {
      fprintf(fp2, "%s\n", mail_field);
    }
    copy_fields(fp1, relay_fields,
		sizeof(relay_fields) / sizeof(struct cpy_hdr),
		CF_CLR_MASK | CF_GET_MASK);

    /*
     * フィールド検出チェック
     */

    for (i = 0; i < (sizeof(relay_fields) / sizeof(struct cpy_hdr)); i++) {
      if (relay_fields[i].field_buff[0]) {
	i = -1;
	break;
      }
    }
    if (i >= 0) {
      fclose(fp2);
      fclose(fp1);
      return(0);
    }
    if (user_gecos[0]) {
      fprintf(fp2, "%s %s@%s (%s)\n", SENDER_FIELD, user_name, domain_name,
	      user_gecos);
    } else {
      fprintf(fp2, "%s %s@%s\n", SENDER_FIELD, user_name, domain_name);
    }
    fprintf(fp2, "%s %s\n", FROM_FIELD, from_buff);
    fprintf(fp2, "%s %s\n", TO_FIELD, to_buff2);
    fprintf(fp2, "%s %s\n", CC_FIELD, cc_buff2);
    if (subject_buff[0]) {
      edit_fprintf(fp2, "%s %s\n", SUBJECT_FIELD, subject_buff);
    }
    if (x_nsubj_buff[0]) {
      recover_jis(subject_buff, x_nsubj_buff);
      to_euc(buff1, subject_buff, EUC_CODE);
      edit_fprintf(fp2, "%s %s\n", X_NSUBJ_FIELD, buff1);
    }
    if (group_buff[0]) {
      fprintf(fp2, "%s %s\n", GROUP_FIELD, group_buff);
    }
    if (in_reply_buff[0]) {
      fprintf(fp2, "%s %s\n", IN_REPLY_FIELD, in_reply_buff);
    }
    if (sender_buff[0]) {
      fprintf(fp2, "X-ORG-%s %s\n", SENDER_FIELD, sender_buff);
    }
    if (message_buff[0]) {
      fprintf(fp2, "X-ORG-%s %s\n", MESSAGE_FIELD, message_buff);
    }
    if (app_buff[0]) {
      fprintf(fp2, "X-ORG-%s %s\n", APP_TO_FIELD, app_buff);
    }
    if (to_buff[0]) {
      fprintf(fp2, "X-ORG-%s %s\n", TO_FIELD, to_buff);
    }
    if (cc_buff[0]) {
      fprintf(fp2, "X-ORG-%s %s\n", CC_FIELD, cc_buff);
    }
    if (date_buff[0]) {
      fprintf(fp2, "X-ORG-%s %s\n", DATE_FIELD, date_buff);
    }
    if (reply_buff[0]) {
      fprintf(fp2, "X-ORG-%s %s\n", REPLY_FIELD, reply_buff);
    }
#ifdef	FCC
    if (mail_fcc_field[0]) {
      fprintf(fp2, "%s %s\n", FCC_FIELD, mail_fcc_field);
    }
#endif	/* FCC */
    fprintf(fp2, "%s\n", TEXT_SEPARATER);
    while (fgets2(buff1, sizeof(buff1), fp1)) {
      if (!strcmp(buff1, MULTI_END_MARK)) {
	break;
      }
      to_euc(buff2, buff1, EUC_CODE);
      edit_fprintf(fp2, "%s", buff2);
    }
    fclose(fp2);
    while (1) {
      status = exec_send(tmp_file, folder);
      if (status == -2) {
	exec_editor(tmp_file, 0);
      } else if (status != -3) {
	break;
      }
    }
    if (status) {
      if (status > 0) {
	print_mode_line(japanese ? "メールの回送に失敗しました。" :
			"Relay mail failed.");
	term_bell();
	sleep(ERROR_SLEEP);
      }
      break;
    }
    if (feof(fp1)) {
      fclose(fp1);
      return(0);
    }
    term_clear_all();
  }
  fclose(fp1);
  return(1);
}
#endif	/* LARGE */

/*
 * メール送信(下位関数)
 *
 * 戻り値:
 *	-3:MIME切替
 *	-2:再編集
 *	-1:中断
 *	 0:正常終了
 *	 1:エラー
 */

int	exec_send(base_file, folder)
     char	*base_file;
     char	*folder;
{
  char		tmp_file1[PATH_BUFF];
  char		tmp_file2[PATH_BUFF];
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		buff3[BUFF_SIZE];
  char		buff4[BUFF_SIZE];
  char		field[MAX_FIELD_LEN];
  FILE		*fp1, *fp2, *fp3;
  static struct cpy_hdr send_fields[] = {
    {FROM_FIELD,	from_buff,	SF_FROM_MASK},
    {DATE_FIELD,	date_buff,	SF_ETC_MASK},
    {SUBJECT_FIELD,	(char*)NULL,	SF_SUBJ_MASK},
    {TO_FIELD,		(char*)NULL,	SF_TO_MASK | SF_EXPAND_MASK},
    {CC_FIELD,		(char*)NULL,	SF_CC_MASK | SF_EXPAND_MASK},
    {BCC_FIELD,		(char*)NULL,	SF_BCC_MASK | SF_EXPAND_MASK},
    {X_NSUBJ_FIELD,	(char*)NULL,	SF_THROUGH_MASK},
#ifdef	FCC
    {FCC_FIELD,		fcc_buff,	SF_DELETE_MASK},
#endif	/* FCC */
#ifdef	MIME
    {MIME_VER_FIELD,	(char*)NULL,	SF_MIME_MASK},
    {CONTENT_TYPE_FIELD,(char*)NULL,	SF_CONTENT_MASK},
#endif	/* MIME */
#ifdef	NEWSPOST
#ifdef	DELETE_FIELD
    {GROUP_FIELD,	(char*)NULL,	SF_DELETE_MASK},
    {DIST_FIELD,	(char*)NULL,	SF_DELETE_MASK},
    {FOLLOWUP_FIELD,	(char*)NULL,	SF_DELETE_MASK},
#endif	/* DELETE_FIELD */
#endif	/* NEWSPOST */
#ifdef	MIME_XXX
    {X_BIN_FILE_FIELD,	bin_file_buff,	SF_FILE_MASK},
#endif	/* MIME_XXX */
  };
  char		*ptr1, *ptr2, *last;
  int		status1, status2, status3, status4, status5;
  long		length;
#ifdef	MIME
  long		position;
#endif	/* MIME */
#ifdef	ADD_DATE_FIELD
  struct tm		*tm;
  time_t		now_time;
#endif	/* ADD_DATE_FIELD */
  register int	i;

#if	defined(MSDOS) || defined(OS2)
  fp1 = fopen(base_file, "rb");				/* バイナリモード */
#else	/* !(MSDOS || OS2) */
  fp1 = fopen(base_file, "r");
#endif	/* !(MSDOS || OS2) */
  if (fp1 == (FILE*)NULL) {
    print_fatal("Can't open mail read file.");
    return(1);
  }
  create_temp_name(tmp_file1, "SS");
  fp2 = fopen2(tmp_file1, "w");
  if (fp2 == (FILE*)NULL) {
    print_fatal("Can't open mail write file.");
    fclose(fp1);
    return(1);
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
  for (i = 0; i < sizeof(send_fields) / sizeof(struct cpy_hdr); i++) {
    if (send_fields[i].field_buff != (char*)NULL) {
      send_fields[i].field_buff[0] = '\0';
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
	  if (status3 & SF_DELETE_MASK) {
	    status2 |= status3;
	  } else {
	    status3 |= SF_DELETE_MASK;
	    ptr1 = buff2;
	    while (*ptr1) {
	      if ((*ptr1 != ' ') && (*ptr1 != '\t') && (*ptr1 != '\n')) {
		status3 &= ~SF_DELETE_MASK;
		break;
	      }
	      ptr1++;
	    }
	  }
	}
#ifdef	MIME_XXX
	if ((!(status3 & SF_DELETE_MASK)) && (status3 & SF_FILE_MASK)) {
	  mime_add_content(M_MTYPE_APP, M_STYPE_OCT, M_TTYPE_B64, buff2);
	  status3 |= SF_DELETE_MASK;
	}
#endif	/* MIME_XXX */
	if (!(status3 & SF_DELETE_MASK)) {
	  status2 |= status3;
	  if (status3 & SF_EXPAND_MASK) {
	    if (expand_field(fp2, field, buff2)) {	/* エイリアス展開 */
	      print_mode_line(japanese ?
			      "警告 : エイリアス展開に失敗しました。" :
			      "Warning : alias expand failed.");
	      term_bell();
	      sleep(ERROR_SLEEP);
	    }
	  } else {
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
	      if (status3 & SF_THROUGH_MASK) {
		to_jis(buff4, buff3, EUC_CODE);
	      } else {
		status5 = 0;
		if (mime_encode_func(buff4, buff3, EUC_CODE) < 0) {
		  status5 = 1;
		} else if ((!(status3 & SF_SUBJ_MASK)) &&
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
	      fputs(buff4, fp2);
	      fputc('\n', fp2);
	      if (!*ptr1) {
		break;
	      }
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
	for (i = 0; i < sizeof(send_fields) / sizeof(struct cpy_hdr); i++) {
	  if (!strncmp(send_fields[i].field_name, field,
		       strlen(send_fields[i].field_name))) {
	    if (send_fields[i].field_buff != (char*)NULL) {
	      strcpy(send_fields[i].field_buff, buff2);
	    }
	    status3 = send_fields[i].buff_size;
	    break;
	  }
	}
      }
    }
  }

  if ((!status1) || (!(status2 & (SF_TO_MASK | SF_CC_MASK | SF_BCC_MASK)))) {
    fclose(fp1);
    fclose(fp2);
    funlink2(tmp_file1);
    return(1);
  }
  fp3 = get_mail_signature(folder);
#ifdef	MIME

  /*
   * .signature の漢字コードを調べる。
   */

  status3 = 0;
  if (fp3 != (FILE*)NULL) {
    while (fgets2(buff1, sizeof(buff1), fp3)) {
      if (discriminate_code(buff1) != ASCII_CODE) {
	status3 = 1;
      }
    }
    fseek(fp3, 0L, 0);
  }
#endif	/* MIME */
  if (!(status2 & SF_FROM_MASK)) {	/* From: フィールドを作成する */

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
#ifdef	MIME
  if (mime_format == 2) {		/* MIME 関連フィールドを作成する */
    position = ftell(fp1);
    while (fgets(buff1, sizeof(buff1), fp1)) {
      if (discriminate_code(buff1) != ASCII_CODE) {
	status3 = 1;
	break;
      }
    }
    fseek(fp1, position, 0);
    if (!(status2 & SF_MIME_MASK)) {
      length += (long)fprintf2(fp2, "%s %s\n", MIME_VER_FIELD,
			      MIME_VERSION);
    }
    if (!(status2 & SF_CONTENT_MASK)) {
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
			   X_MAILER_FIELD, READER_NAME,
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
  if (*send_filter) {
    create_temp_name(tmp_file2, "SU");
    sprintf(buff1, "%s < %s > %s", send_filter, tmp_file1, tmp_file2);
    if (system(buff1)) {
      print_fatal("Send filter error.");
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
   * 送信直前の記事を表示
   */

#ifdef	MAX_SEND_SIZE
  if (length >= (long)MAX_SEND_SIZE) {
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
#endif	/* MAX_SEND_SIZE */
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
  create_temp_name(tmp_file2, "SV");
  exec_pager(tmp_file1, tmp_file2, 3, japanese ? "送信確認" : "CONFIRM");
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
   * メーラーを実行
   */

  switch (yes_or_no(SEND_YN_MODE, "記事を送信してよろしいですか?",
		    "Send article.Are you sure?")) {
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
  print_mode_line(japanese ? "送信中です。" : "Sending.");
  status1 = SMTP_OK;
#ifdef	DEBUG
  sleep(5);
#else	/* !DEBUG */
  if (smtp_open() == SMTP_OK) {
    status1 = smtp_mail(tmp_file1, domain_name, user_name);
    smtp_close();
  }
#endif	/* !DEBUG */
  if (status1 != SMTP_OK) {
    print_fatal("Send article failed.");
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
 * signature ファイル取得(メール用)
 */

static FILE	*get_mail_signature(folder)
    char	*folder;
{
  FILE	*fp;
  char	signature_file[PATH_BUFF];
  char	*ptr1, *ptr2, *ptr3;

  sprintf(signature_file, "%s%c%s", dot_dir, SLASH_CHAR, SIGNATURE_FILE);
  ptr2 = signature_file;
  while (*ptr2) {
    ptr2++;
  }
  *ptr2 = '=';
  ptr1 = ptr3 = ptr2 + 1;
  strcpy(ptr3, folder);
  while (*ptr3) {
    if (*ptr3 == FOLDER_SEPARATER) {
      *ptr3 = NEWS_GROUP_SEPARATER;
    }
    ptr3++;
  }
  fp = (FILE*)NULL;
  while (*ptr1) {
    fp = fopen(signature_file, "r");
    if (fp != (FILE*)NULL) {
      break;
    } else {
      ptr3 = strrchr(ptr1, NEWS_GROUP_SEPARATER);
      if (ptr3 != (char*)NULL) {
	*ptr3 = '\0';
      } else {
	*ptr1 = '\0';
      }
    }
  }
  if (fp == (FILE*)NULL) {
    strcpy(ptr2, "=mail");
    fp = fopen(signature_file, "r");
    if (fp == (FILE*)NULL) {
      *ptr2 = '\0';
      fp = fopen(signature_file, "r");
    }
  }
  if (fp != (FILE*)NULL) {
    if (!auto_sig_mode) {
      if (!yes_or_no(NORMAL_YN_MODE, "signature%s ファイルをつけますか?",
		     "Add signature%s file.Are you sure?", ptr2)) {
	fclose(fp);
	fp = (FILE*)NULL;
      }
    }
  }
  return(fp);
}

/*
 * エイリアス展開(上位関数)
 */

static int	expand_field(fp1, field, ptr1)
     FILE	*fp1;
     char	*field;
     char	*ptr1;
{
  FILE	*fp2;
  char	recipient[ALIAS_BUFF];
  char	alias[ALIAS_BUFF];
  char	buff1[ALIAS_BUFF];
  char	buff2[SMALL_BUFF];
  char	*ptr2, *ptr3, *ptr4;
  int	status;
  int	length;
  int	first;

  fp2 = (FILE*)NULL;
  if (!mh_alias_mode) {
    sprintf(buff1, "%s%c%s", home_dir, SLASH_CHAR, MAILRC_FILE);
    fp2 = fopen(buff1, "r");
  }
  length = fprintf2(fp1, "%s ", field);
  status = 0;
  first = 1;
  while (*ptr1) {
    while ((*ptr1 == ',') || (*ptr1 == '\t') || (*ptr1 == ' ')) {
      ptr1++;
    }
#ifdef	notdef
    ptr2 = alias;
    while (*ptr1) {
      if ((*ptr1 != ',') && (*ptr1 != ' ') && (*ptr1 != '\t')) {
	*ptr2++ = *ptr1++;
      } else {
	ptr1++;
	break;
      }
    }
    *ptr2 = '\0';
#else	/* !notdef */
    ptr2 = ptr1;
    ptr1 = get_real_adrs(ptr1, alias);
#endif	/* !notdef */
    if (mh_alias_mode) {
#ifdef	MH
      if (mh_expand_alias(recipient, alias, ptr2, sizeof(recipient))) {
	status = 1;
	break;
      }
#else	/* !MH */
      strcpy(recipient, alias);
#endif	/* !MH */
    } else {
      if (ucbmail_expand_alias(fp2, recipient, alias, ptr2,
			       sizeof(recipient))) {
	status = 1;
	break;
      }
    }
    ptr2 = recipient;
    while (*ptr2) {
      ptr3 = ptr2;
#ifdef	notdef
      while (*ptr2 && (*ptr2 != ',') && (*ptr2 != ' ') && (*ptr2 != '\t')) {
	ptr2++;
      }
#else	/* !notdef */
      ptr2 = get_real_adrs(ptr2, buff1);
      ptr4 = ptr3;
      while (ptr4 != ptr2) {
	if (*ptr4 == ',') {
	  *ptr4 = '\0';
	  break;
	}
	ptr4++;
      }
#endif	/* !notdef */
      while ((*ptr2 == ',') || (*ptr2 == ' ') || (*ptr2 == '\t')) {
	*ptr2++ = '\0';
      }
      if (first) {
	first = 0;
      } else {
	if ((length + (int)strlen(ptr3)) >= MAX_FIELD_COLUMN) {
	  fprintf(fp1, ",\n\t");
	  length = 8;		/* TAB column */
	} else {
	  length += fprintf2(fp1, ", ");
	}
      }
      if (mime_encode_func(buff2, ptr3, EUC_CODE) < 0) {
	status = 1;
      } else if (discriminate_code(buff2) != ASCII_CODE) {
	status = 1;
      }
      length += fprintf2(fp1, "%s", buff2);
    }
  }
  fputc('\n', fp1);
  if (fp2 != (FILE*)NULL) {
    fclose(fp2);
  }
  return(status);
}

/*
 * UCB-mail .mailrc 用エイリアス展開(下位関数)
 * alias の多重展開時は再帰呼びだしされる。
 */

static int	ucbmail_expand_alias(fp, recipient, alias, original, number)
     FILE	*fp;
     char	*recipient;
     char	*alias;
     char	*original;
     int	number;
{
  static int	level = 0;		/* alias level */
  char		buff1[ALIAS_BUFF];
  char		buff2[ALIAS_BUFF];
  char		buff3[ALIAS_BUFF];
  char		*ptr1, *ptr2, *ptr3;
  int		next_flag;
  int		quote;

  if ((number <= 0) || (level >= 15)) {
    return(1);		/* alias loop or too long alias list */
  }
  strcpy(recipient, original);
  if (!fp) {
    return(0);
  }
  level++;
  next_flag = 0;
  fseek(fp, 0L, 0);
  while (fgets(buff1, sizeof(buff1), fp)) {
    ptr1 = buff1;
    while (*ptr1) {
      if (*ptr1 == '\n') {
	*ptr1 = '\0';
	break;
      }
      ptr1++;
    }
    ptr1 = buff1;
    while ((*ptr1 == '\t') || (*ptr1 == ' ')) {
      ptr1++;
    }
    ptr2 = ptr1;
    if (!strncasecmp(ptr1, ALIAS_PARAM1, sizeof(ALIAS_PARAM1) - 1)) {
      ptr1 += (sizeof(ALIAS_PARAM1) - 1);
    } else if (!strncasecmp(ptr1, ALIAS_PARAM2, sizeof(ALIAS_PARAM2) - 1)) {
      ptr1 += (sizeof(ALIAS_PARAM2) - 1);
    }
    if (ptr1 != ptr2) {
      while ((*ptr1 == '\t') || (*ptr1 == ' ')) {
	ptr1++;
      }
      if (!strncasecmp(ptr1, alias, strlen(alias))) {
	ptr1 += strlen(alias);
	if ((*ptr1 == '\t') || (*ptr1 == ' ')) {
	  while ((*ptr1 == '\t') || (*ptr1 == ' ')) {
	    ptr1++;
	  }
	  ptr2 = buff3;
	  while (*ptr1) {
	    switch (quote = *ptr1) {
	    case '\t':
	    case ' ':
	    case ',':
	    case '\n':
	    case '\\':
	      ptr1++;
	      next_flag = 1;
	      if (quote == '\\') {
		if (fgets(buff1, sizeof(buff1), fp)) {
		  ptr1 = buff1;
		} else {
		  while (*ptr1) {
		    ptr1++;
		  }
		}
	      }
	      break;
	    default:
	      if (next_flag) {
		if (ptr2 != buff3) {
		  *ptr2++ = ',';
		}
		next_flag = 0;
	      }
	      if ((quote != '\'') && (quote != '\"')) {
		*ptr2++ = *ptr1++;
	      } else {
		ptr1++;
		ptr3 = buff2;
		while (*ptr1) {
		  if ((*ptr1 == quote) || (*ptr1 == '\n')) {
		    ptr1++;
		    break;
		  }
		  *ptr3++ = *ptr1++;
		}
		*ptr3 = '\0';
#ifdef	ONLY_REAL_ALIAS
		get_real_adrs(buff2, ptr2);
#else	/* !ONLY_REAL_ALIAS */
		strcpy(ptr2, buff2);
#endif	/* !ONLY_REAL_ALIAS */
		while (*ptr2) {
		  ptr2++;
		}
	      }
	      break;
	    }
	  }
	  *ptr2 = '\0';

	  /*
	   * alias の多重展開チェック
	   */

	  ptr2 = buff3;
	  while (*ptr2) {
	    while ((*ptr2 == ',') || (*ptr2 == ' ') || (*ptr2 == '\t')) {
	      ptr2++;
	    }
	    ptr3 = ptr2;
	    ptr2 = get_real_adrs(ptr2, buff2);
	    if (ucbmail_expand_alias(fp, recipient, buff2, ptr3, number)) {
	      level--;
	      return(1);
	    } else {
	      while (*recipient) {
		recipient++;
		number--;
	      }
	    }
	    if (*ptr2) {
	      *recipient++ = ',';
	      *recipient++ = ' ';
	      number -= 2;
	    }
	    *recipient = '\0';
	  }
	  break;
	}
      }
    }
  }
  level--;
  return(0);
}

#ifdef	notdef
/*
 * MH -alias mh_alias 用エイリアス展開(下位関数)
 * alias の多重展開時は再帰呼びだしされる。
 */

static int	mh_expand_alias(fp, recipient, alias, original, number)
     FILE	*fp;
     char	*recipient;
     char	*alias;
     char	*original;
     int	number;
{
  static int	level = 0;		/* alias level */
  char		buff1[ALIAS_BUFF];
  char		buff2[ALIAS_BUFF];
  char		buff3[ALIAS_BUFF];
  char		*ptr1, *ptr2, *ptr3;
  int		next, match, first;
  int		status;
  unsigned long	position;

  if ((number <= 0) || (level >= 15)) {
    return(1);		/* alias loop or too long alias list */
  }
  if (alias) {
    strcpy(recipient, original);
  } else {
    *recipient = '\0';
  }
  if (!fp) {
    return(0);
  }
  level++;
  status = 0;
  while (fgets(buff1, sizeof(buff1), fp)) {
    next = 0;
    ptr1 = buff1;
    while (*ptr1) {
      if ((*ptr1 == '\\') && (*(ptr1 + 1) == '\n')) {
	*ptr1 = '\0';
	next = 1;
	break;
      }
      if (*ptr1 == '\n') {
	*ptr1 = '\0';
	break;
      }
      ptr1++;
    }
    ptr1 = buff1;
    if ((!*ptr1) || (*ptr1 == '#')) {
      continue;
    }
    if ((*ptr1 == '\t') || (*ptr1 == ' ')) {
      continue;
    }
    match = 0;
    if (!alias) {
      ptr3 = ptr1;
      while (*ptr1) {
	if ((*ptr1 == '\t') || (*ptr1 == ' ')) {
	  continue;
	} else if (*ptr1 == ':') {
	  *ptr1++ = '\0';
	  alias = ptr3;
	  printf("%s: ", ptr3);
	  match = 1;
	  break;
	}
	ptr1++;
      }
    } if (!strncasecmp(ptr1, alias, strlen(alias))) {
      ptr1 += strlen(alias);
      if (*ptr1 == ':') {
	ptr1++;
	match = 2;
      }
    }
    if (match) {
      status = 0;
      first = 1;
      while (*ptr1) {
	if ((*ptr1 == '\t') || (*ptr1 == ' ')) {
	  ptr1++;
	} else {
	  ptr2 = ptr1;
	  ptr1 = get_real_adrs(ptr1, buff2);
	  if (first) {
	    first = 0;
	  } else {
	    *recipient++ = ',';
	    *recipient++ = ' ';
	    number -= 2;
	  }
	  *recipient = '\0';

	  /*
	   * alias の多重展開チェック
	   */

	  position = ftell(fp);
	  if (match == 1) {
	    strcpy(recipient, ptr2);
	  } else if (mh_expand_alias(fp, recipient, buff2, buff2, number)) {
	    status = 1;
	    strcpy(recipient, ptr2);
	  }
	  fseek(fp, position, 0);
	  while (*recipient) {
	    *recipient++;
	    number--;
	  }
	}
	if ((!*ptr1) && next) {
	  next = 0;
	  if (fgets(buff1, sizeof(buff1), fp)) {
	    ptr1 = buff1;
	    while (*ptr1) {
	      if ((*ptr1 == '\\') && (*(ptr1 + 1) == '\n')) {
		*ptr1 = '\0';
		next = 1;
		break;
	      }
	      if (*ptr1 == '\n') {
		*ptr1 = '\0';
		break;
	      }
	      ptr1++;
	    }
	    ptr1 = buff1;
	  } else {
	    break;
	  }
	}
      }
      break;
    }
  }
  level--;
  return(status);
}
#endif	/* notdef */
#endif	/* MAILSEND */
