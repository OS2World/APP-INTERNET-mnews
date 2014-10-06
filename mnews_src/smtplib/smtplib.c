/*
 *
 *  SMTP ライブラリ
 *
 *  Copyright 1995-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : SMTP protocol library
 *  Sub system  : SMTP library
 *  File        : smtplib.c
 *  Version     : 0.84
 *  First Edit  : 1995-12/25
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 宅間 顯
 *
 */

#include	"compat.h"
#include	"nntplib.h"
#include	"smtplib.h"
#include	"field.h"

char			smtp_server[MAXHOSTNAMELEN];
					/* SMTP サーバホスト名(FQDN)	*/
char			smtp_server_file[PATH_BUFF] = SMTP_SERVER_FILE;
					/* SMTP サーバ指定ファイル	*/
char			send_program[PATH_BUFF] = SEND_PROGRAM;
					/* 送信プログラム		*/
short			smtp_connect_mode = 2;
					/* SMTP 接続モード		*/
					/* (0:MTAP,1:SMTP,2:DEAD)	*/
char			*smtp_connect_string[] = {
  "MTAP",
  "SMTP",
  "DEAD",
};					/* SMTP 接続モード文字列	*/

#ifdef	SMTP
#ifdef	MSDOS
static int		read_fd = -1;	/* SMTP リードファイル記述子	*/
#else	/* !MSDOS */
static TCPRD_FILE	*read_fp = (TCPRD_FILE*)NULL;
					/* SMTP リードファイルポインタ	*/
#endif	/* !MSDOS */
static int		write_fd = -1;	/* SMTP ライトファイル記述子	*/
static unsigned int	port_number = 0;/* SMTP ポート番号		*/
static char		to_buff[MAX_FIELD_LEN * 5];
					/* To: バッファ			*/
static char		cc_buff[MAX_FIELD_LEN * 5];
					/* Cc: バッファ			*/
static char		bcc_buff[MAX_FIELD_LEN * 5];
					/* Bcc: バッファ		*/
#endif	/* SMTP */

#ifdef	__STDC__
#ifdef	MTAP
static void		send_pipe_error(void);	/* パイプエラーハンドラ	*/
#endif	/* MTAP */
#ifdef	SMTP
static SMTP_ERR_CODE	smtp_cmd(char*, int);	/* SMTP コマンド送信,	*/
						/* ステータス受信	*/
#endif	/* SMTP */
#else	/* !__STDC__ */
#ifdef	MTAP
static void		send_pipe_error();	/* パイプエラーハンドラ	*/
#endif	/* MTAP */
#ifdef	SMTP
static SMTP_ERR_CODE	smtp_cmd();		/* SMTP コマンド送信,	*/
						/* ステータス受信	*/
#endif	/* SMTP */
#endif	/* !__STDC__ */

/*
 * SMTP サーバ選択
 */

#ifdef	__STDC__
SMTP_ERR_CODE	smtp_select_server(char *server_name)
#else	/* !__STDC__ */
SMTP_ERR_CODE	smtp_select_server(server_name)
     char	*server_name;
#endif	/* !__STDC__ */
{
#ifdef	SMTP
  struct hostent	*server_entry;
#endif	/* SMTP */
#ifdef	MTAP
  char			buff[PATH_BUFF];
  struct stat		stat_buff;
#endif	/* MTAP */
  char			*ptr;
  SMTP_ERR_CODE		status;

  switch (smtp_connect_mode) {
  case 0:					/* MTAP→SMTP 接続	*/
#ifndef	MTAP
    smtp_connect_mode = 1;
#endif	/* MTAP */
    break;
  case 1:					/* SMTP→MTAP 接続	*/
#ifndef	SMTP
    smtp_connect_mode = 0;
#endif	/* SMTP */
    break;
  default:
    break;
  }
  status = SMTP_ERR_COMM;
  switch (smtp_connect_mode) {
  case 0:						/* MTAP 接続	*/
#ifdef	MTAP
    strcpy(buff, send_program);
    ptr = strchr(buff, ' ');
    if (ptr != (char*)NULL) {
      *ptr = '\0';
    }
    if (strchr(buff, SLASH_CHAR) && stat(buff, &stat_buff)) {
      print_fatal("Can't stat MTA program file.");
      break;
    }
#ifdef	SMTP
    if (gethostname(smtp_server, sizeof(smtp_server))) {
      print_fatal("Can't get hostname.");
      break;
    }
#else	/* !SMTP */
    strcpy(smtp_server, "LOCAL");
#endif	/* !SMTP */
    status = SMTP_OK;
#else	/* !MTAP */
    print_fatal("MTAP mode unsupported.");
    status = SMTP_ERR_RSV;
#endif	/* !MTAP */
    break;
  case 1:						/* SMTP 接続	*/
#ifdef	SMTP
    if (server_name != (char*)NULL) {
      strcpy(smtp_server, server_name);
      ptr = strchr(server_name, ':');
      if (ptr != (char*)NULL) {
	*ptr++ = '\0';
	if (atoi(ptr) > 0) {
	  port_number = atoi(ptr) & 0xFFFF;
	}
      }
    }
    if ((server_name == (char*)NULL) || (!server_name[0])) {
      if (tcp_get_default_server(smtp_server_file, smtp_server, "SMTPSERVER",
				 &port_number)) {
	if (gethostname(smtp_server, sizeof(smtp_server))) {
	  print_fatal("Can't get hostname.");
	  break;
	}
      }
    }
#ifndef	DONT_USE_REALNAME
    server_entry = gethostbyname(smtp_server);
    if (server_entry != (struct hostent*)NULL) {
      strcpy(smtp_server, server_entry->h_name);
    } else {
      print_warning("Can't get SMTP server host entry.");
    }
#endif	/* !DONT_USE_REALNAME */
    status = SMTP_OK;
#else	/* !SMTP */
    print_fatal("SMTP mode unsupported.");
    status = SMTP_ERR_RSV;
#endif	/* !SMTP */
    break;
  default:
    break;
  }
  if (status != SMTP_OK) {
    smtp_connect_mode = 2;
  }
  return(status);
}

/*
 * SMTP オープン
 */

#ifdef	__STDC__
SMTP_ERR_CODE	smtp_open(void)
#else	/* !__STDC__ */
SMTP_ERR_CODE	smtp_open()
#endif	/* !__STDC__ */
{
  char		buff[SMALL_BUFF];
#ifdef	SMTP
  char		*ptr;
#endif	/* SMTP */
  SMTP_ERR_CODE	status;

#ifdef	SMTP
#ifdef	MSDOS
  if ((write_fd != -1) || (read_fd != -1)) {
#else	/* !MSDOS */
  if ((write_fd != -1) || (read_fp != (TCPRD_FILE*)NULL)) {
#endif	/* !MSDOS */
    print_fatal("SMTP server already open.");
    return(SMTP_ERR_PROG);
  }
#endif	/* SMTP */
  status = SMTP_ERR_COMM;
  switch (smtp_connect_mode) {
  case 0:						/* MTAP 接続	*/
#ifdef	MTAP
#ifdef	SMTP_DEBUG
    print_warning("Connecting to MTAP server %s.", smtp_server);
#endif	/* SMTP_DEBUG */
    status = SMTP_OK;
#else	/* !MTAP */
    print_fatal("MTAP mode unsupported.");
    status = SMTP_ERR_RSV;
#endif	/* !MTAP */
    break;
  case 1:						/* SMTP 接続	*/
#ifdef	SMTP
#ifdef	SMTP_DEBUG
    print_warning("Connecting to SMTP server %s.", smtp_server);
#endif	/* SMTP_DEBUG */
#ifdef	MSDOS
    if (tcp_open(smtp_server, "SMTP", "smtp", port_number, SMTP_PORT,
		 &read_fd, &write_fd)) {
#else	/* !MSDOS */
    if (tcp_open(smtp_server, "SMTP", "smtp", port_number, SMTP_PORT,
		 &read_fp, &write_fd)) {
#endif	/* !MSDOS */
      print_fatal("Can't connect SMTP server %s.", smtp_server);
      break;
    }
    
    /*
     * SMTP サーバの接続メッセージ取得
     */

    do {
      ptr = (char*)NULL;
      if (smtp_receive(buff, sizeof(buff))) {
	break;
      }
      ptr = buff;
      while (isdigit(*ptr)) {
	ptr++;
      }
    } while (*ptr == '-');
    if (ptr != (char*)NULL) {
      switch (atoi(buff)) {
      case SMTP_OPEN_RES:
	status = SMTP_OK;
	break;
      default:
	print_fatal("Unexpected SMTP reply in connect sequence.");
	print_fatal("%s", ptr);
	break;
      }
    }
#else	/* !SMTP */
    print_fatal("SMTP mode unsupported.");
    status = SMTP_ERR_RSV;
#endif	/* !SMTP */
    break;
  case 2:
    status = SMTP_ERR_RSV;
    /* break 不要 */
  default:
    break;
  }
  if (status != SMTP_OK) {
    smtp_close();
    smtp_connect_mode = 2;
  }
  return(status);
}

/*
 * SMTP クローズ
 */

#ifdef	__STDC__
SMTP_ERR_CODE	smtp_close(void)
#else	/* !__STDC__ */
SMTP_ERR_CODE	smtp_close()
#endif	/* !__STDC__ */
{
  SMTP_ERR_CODE	status;

  status = SMTP_ERR_COMM;
  switch (smtp_connect_mode) {
  case 0:						/* MTAP 接続	*/
#ifdef	MTAP
    status = SMTP_OK;
#endif	/* MTAP */
    break;
  case 1:						/* SMTP 接続	*/
#ifdef	SMTP
#ifdef	MSDOS
    if ((write_fd == -1) || (read_fd == -1)) {
      break;
    }
#else	/* !MSDOS */
    if ((write_fd == -1) || (read_fp == (TCPRD_FILE*)NULL)) {
      break;
    }
#endif	/* !MSDOS */
    smtp_cmd(SMTP_QUIT_CMD, SMTP_QUIT_RES);
#ifdef	MSDOS
    tcp_close(&read_fd, &write_fd);
#else	/* !MSDOS */
    tcp_close(&read_fp, &write_fd);
#endif	/* !MSDOS */
    status = SMTP_OK;
#endif	/* SMTP */
    break;
  default:
    break;
  }
  return(status);
}

/*
 * SMTP 送信
 */

#ifdef	__STDC__
SMTP_ERR_CODE	smtp_send(char *string)
#else	/* !__STDC__ */
SMTP_ERR_CODE	smtp_send(string)
     char	*string;
#endif	/* !__STDC__ */
{
  switch (smtp_connect_mode) {
  case 0:						/* MTAP 接続	*/
    print_fatal("MTAP write does not need.");
    break;
  case 1:						/* SMTP 接続	*/
#ifdef	SMTP
#ifdef	MSDOS
    if (tcp_send("SMTP", &read_fd, &write_fd, string)) {
      return(SMTP_ERR_COMM);
    } else {
      return(SMTP_OK);
    }
#else	/* !MSDOS */
    if (tcp_send("SMTP", &read_fp, &write_fd, string)) {
      return(SMTP_ERR_COMM);
    } else {
      return(SMTP_OK);
    }
#endif	/* !MSDOS */
#endif	/* SMTP */
    break;
  default:
    break;
  }
  return(SMTP_ERR_PROG);
}

/*
 * SMTP 受信
 */

#ifdef	__STDC__
SMTP_ERR_CODE	smtp_receive(char *ptr, int size)
#else	/* !__STDC__ */
SMTP_ERR_CODE	smtp_receive(ptr, size)
     char	*ptr;
     int	size;
#endif	/* !__STDC__ */
{
  switch (smtp_connect_mode) {
  case 0:						/* MTAP 接続	*/
    print_fatal("MTAP read does not need.");
    break;
  case 1:						/* SMTP 接続	*/
#ifdef	SMTP
#ifdef	MSDOS
    if (tcp_receive("SMTP", &read_fd, &write_fd, ptr, size)) {
      return(SMTP_ERR_COMM);
    } else {
      return(SMTP_OK);
    }
#else	/* !MSDOS */
    if (tcp_receive("SMTP", &read_fp, &write_fd, ptr, size)) {
      return(SMTP_ERR_COMM);
    } else {
      return(SMTP_OK);
    }
#endif	/* !MSDOS */
#endif	/* SMTP */
    break;
  default:
    break;
  }
  return(SMTP_ERR_PROG);
}

/*
 * SMTP 接続確認
 */

#ifdef	__STDC__
SMTP_ERR_CODE	smtp_noop(void)
#else	/* !__STDC__ */
SMTP_ERR_CODE	smtp_noop()
#endif	/* !__STDC__ */
{
  switch (smtp_connect_mode) {
  case 0:						/* MTAP 接続	*/
    break;
  case 1:						/* SMTP 接続	*/
#ifdef	SMTP
    return(smtp_cmd(SMTP_NOOP_CMD, SMTP_OKAY_RES));
#endif	/* SMTP */
    break;
  default:
    break;
  }
  return(SMTP_OK);
}

/*
 * SMTP 記事送信
 */

#ifdef	__STDC__
SMTP_ERR_CODE	smtp_mail(char *file_name, char *domain_name, char *user_name)
#else	/* !__STDC__ */
SMTP_ERR_CODE	smtp_mail(file_name, domain_name, user_name)
     char	*file_name;
     char	*domain_name;
     char	*user_name;
#endif	/* !__STDC__ */
{
  FILE		*fp1;
#ifdef	MTAP
  FILE		*fp2;
#endif	/* MTAP */
  char		buff1[BUFF_SIZE];
#ifdef	SMTP
  char		buff2[BUFF_SIZE];
  char		*ptr;
  register int	i, j;
  static struct cpy_hdr	smtp_fields[] = {
    {TO_FIELD,		to_buff,	sizeof(to_buff)},
    {CC_FIELD,		cc_buff,	sizeof(cc_buff)},
    {BCC_FIELD,		bcc_buff,	sizeof(bcc_buff)},
  };
#endif	/* SMTP */
  SMTP_ERR_CODE	status;

  status = SMTP_ERR_COMM;
  switch (smtp_connect_mode) {
  case 0:						/* MTAP 接続	*/
#ifdef	MTAP
#ifdef	SIGPIPE
    signal(SIGPIPE, send_pipe_error);
#endif	/* SIGPIPE */
    fp1 = fopen(file_name, "r");
    if (fp1 != (FILE*)NULL) {
#ifdef	MSDOS
      strcpy(buff1, send_program);
#else	/* !MSDOS */
#ifdef	OS2
      sprintf(buff1, "%s > NUL", send_program);
#else	/* !OS2 */
      sprintf(buff1, "%s > /dev/null 2>&1", send_program);
#endif	/* !OS2 */
#endif	/* !MSDOS */
      fp2 = popen(buff1, "w");
      if (fp2 != (FILE*)NULL) {
	status = SMTP_OK;
	while (fgets(buff1, sizeof(buff1), fp1)) {
	  if (fputs(buff1, fp2) == EOF) {
	    status = SMTP_ERR_COMM;
	    break;
	  }
	}
	if (pclose(fp2)) {
	  status = SMTP_ERR_COMM;
	}
      } else {
	print_fatal("Can't execute send command.");
      }
      fclose(fp1);
    }
#ifdef	SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif	/* SIGPIPE */
#endif	/* MTAP */
    break;
  case 1:						/* SMTP 接続	*/
#ifdef	SMTP
    fp1 = fopen(file_name, "r");
    if (fp1 != (FILE*)NULL) {
      copy_fields(fp1, smtp_fields,
		  sizeof(smtp_fields) / sizeof(struct cpy_hdr),
		  CF_CLR_MASK | CF_GET_MASK | CF_ADD_MASK | CF_SPC_MASK);
      fseek(fp1, 0L, 0);
      for (i = 0; i < 4; i++) {
	switch (i) {
	case 0:
	  if (gethostname(buff2, sizeof(buff2))) {
	    strcpy(buff2, domain_name);
	  } else {
	    if (strchr(buff2, '.') == (char*)NULL) {
	      strcat(buff2, ".");
	      strcat(buff2, domain_name);
	    }
	  }
	  if (!strcmp(buff2, smtp_server)) {
	    strcpy(buff2, SMTP_LOCAL_HOST_NAME);
	  }
	  sprintf(buff1, "%s %s", SMTP_HELO_CMD, buff2);
	  break;
	case 1:
#ifdef	USE_QMAIL
	  sprintf(buff1, "%s %s <%s@%s>", SMTP_MAIL_CMD, FROM_FIELD,
		  user_name, domain_name);
#else	/* !USE_QMAIL */
	  sprintf(buff1, "%s %s %s@%s", SMTP_MAIL_CMD, FROM_FIELD,
		  user_name, domain_name);
#endif	/* !USE_QMAIL */
	  break;
	case 2:
	  for (j = 0; j < (sizeof(smtp_fields) / sizeof(struct cpy_hdr));
	       j++) {
	    ptr = smtp_fields[j].field_buff;
	    while (*ptr) {
	      ptr = get_real_adrs(ptr, buff2);
#ifdef	USE_QMAIL
	      sprintf(buff1, "%s %s <%s>", SMTP_RCPT_CMD, TO_FIELD, buff2);
#else	/* !USE_QMAIL */
	      sprintf(buff1, "%s %s %s", SMTP_RCPT_CMD, TO_FIELD, buff2);
#endif	/* !USE_QMAIL */
	      if (smtp_cmd(buff1, SMTP_OKAY_RES)) {
		break;
	      }
	    }
	    if (*ptr) {
	      break;
	    }
	  }
	  if (*ptr) {
	    buff1[0] = '\0';
	  } else {
	    strcpy(buff1, SMTP_DATA_CMD);
	  }
	  break;
	default:
	  status = SMTP_OK;
	  while (fgets(buff1, sizeof(buff1), fp1)) {
	    ptr = buff1;
	    while (*ptr) {
	      if (*ptr == '\n') {
		*ptr = '\0';
		break;
	      }
	      ptr++;
	    }
	    if (smtp_send(buff1)) {
	      status = SMTP_ERR_COMM;
	      break;
	    }
	  }
	  if (status != SMTP_OK) {
	    buff1[0] = '\0';
	  } else {
	    buff1[0] = TCP_END_MARK;
	    buff1[1] = '\0';
	  }
	  break;
	}
	if (buff1[0]) {
	  if (smtp_cmd(buff1, (i == 2) ? SMTP_INPUT_RES : SMTP_OKAY_RES)) {
	    status = SMTP_ERR_COMM;
	    break;
	  }
	} else {
	  break;
	}
      }
      fclose(fp1);
    }
#endif	/* SMTP */
    break;
  default:
    break;
  }
  return(status);
}

/*
 * 実アドレス取得
 */

#ifdef	__STDC__
char	*get_real_adrs(char *str1, char *str2)
#else	/* !__STDC__ */
char	*get_real_adrs(str1, str2)
     char	*str1;
     char	*str2;
#endif	/* !__STDC__ */
{
  int		status;
  char		*ptr1, *ptr2;
  char		quote;
  register int	i;

  /*
   * ここの処理は怪しかったので思いきり書き換えました。
   */

  ptr1 = ptr2 = str1;
  quote = '\0';
  status = 1;
  while (status) {
    if (*ptr1 == 0x1b) {	/* エスケープシーケンスを空白に置換 */
      *ptr1++ = ' ';
      while (*ptr1) {
	if ((*ptr1 < 0x30) || (*ptr1 > 0x7e)) {
	  *ptr1++ = ' ';
	} else {
	  *ptr1++ = ' ';
	  break;
	}
      }
    } else if (quote) {
      if (*ptr1) {
	if (*ptr1++ == quote) {
	  quote = '\0';
	}
      } else {
	quote = '\0';
	print_warning("Unmatched quote found in mail address.");
      }
    } else {
      switch (*ptr1) {
      case '(':
	ptr1++;
	status = 1;
	while (status) {
	  if (!(*ptr1)) {
	    break;
	  }
	  switch (*ptr1++) {
	  case '(':
	    status++;
	    break;
	  case ')':
	    status--;
	    break;
	  default:
	    break;
	  }
	}
	if (status) {
	  print_warning("Unexpected bracket found in mail address.");
	}
	status = 1;
	break;
      case '<':
	ptr2 = ++ptr1;
	while ((*ptr2 == ' ') || (*ptr2 == '\t')) {
	  ptr2++;
	}
	status = 1;
	while (*ptr1) {
	  if (*ptr1++ == '>') {
	    status = 0;
	    break;
	  }
	}
	break;
      case '\"':
	quote = *ptr1++;
	break;
      case ',':
      case '\n':
      case '\0':
	ptr2 = str1;
	status = 0;
	break;
      default:
	ptr1++;
	break;
      }
    }
  }
  i = 0;
  while (*ptr2 && (*ptr2 != ',') && (*ptr2 != ' ') && (*ptr2 != '\t')
	 && (*ptr2 != '\n') && (*ptr2 != '>') && (*ptr2 != '(')) {
    if (++i < MAX_FROM_LEN) {
      *str2++ = *ptr2++;
    } else {
      ptr2++;
    }
  }
  *str2 = '\0';
  while ((*ptr1 == ',') || (*ptr1 == ' ') || (*ptr1 == '\t')
	 || (*ptr1 == '\n')) {
    *ptr1++ = '\0';
  }
  return(ptr1);
}

/*
 * SMTP コマンド送信/ステータス受信
 */

#ifdef	SMTP
#ifdef	__STDC__
static SMTP_ERR_CODE	smtp_cmd(char *cmd, int code)
#else	/* !__STDC__ */
static SMTP_ERR_CODE	smtp_cmd(cmd, code)
     char	*cmd;
     int	code;
#endif	/* !__STDC__ */
{
  char	*ptr1;
  char	*ptr2;
  char	buff[BUFF_SIZE];

  if (smtp_send(cmd) == SMTP_OK) {
    do {
      ptr1 = (char*)NULL;
      if (smtp_receive(buff, sizeof(buff))) {
	break;
      }
      ptr1 = buff;
      while (isdigit(*ptr1)) {
	ptr1++;
      }
    } while (*ptr1 == '-');
    if (ptr1 != (char*)NULL) {
      if (atoi(buff) == code) {
	return(SMTP_OK);
      }
    }
  }
  ptr1 = buff;
  ptr2 = cmd;
  while (*ptr2) {
    if (*ptr2 <= ' ') {
      break;
    }
    *ptr1++ = *ptr2++;
  }
  *ptr1 = '\0';
  print_fatal("SMTP error at %s command.", buff);
  print_fatal("%s", buff);
  return(SMTP_ERR_COMM);
}
#endif	/* SMTP */

#ifdef	MTAP
/*
 * パイプエラーハンドラ
 * (Broken Pipe)
 */

#ifdef	__STDC__
static void	send_pipe_error(void)
#else	/* !__STDC__ */
static void	send_pipe_error()
#endif	/* !__STDC__ */
{
  print_fatal("MTA broken pipe.");
}
#endif	/* MTAP */
