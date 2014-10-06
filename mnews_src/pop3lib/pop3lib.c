/*
 *
 *  POP3 ライブラリ
 *
 *  Copyright 1995-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : POP3 protocol library
 *  Sub system  : POP3 library
 *  File        : pop3lib.c
 *  Version     : 0.86
 *  First Edit  : 1995-12/25
 *  Last  Edit  : 1997-12/21
 *  Author      : MSR24 宅間 顯
 *
 */

#include	"compat.h"
#include	"pop3lib.h"
#include	"field.h"

extern void	convert_article_date();	/* 記事日付フォーマット変換	*/

#ifdef	POP3
extern char		tmp_dir[];	/* 作業ディレクトリ		*/
#ifdef	__STDC__
extern char		*get_real_adrs(char*, char*);
					/* 実アドレス取得		*/
#else	/* !__STDC__ */
extern char		*get_real_adrs();
					/* 実アドレス取得		*/
#endif	/* !__STDC__ */
#endif	/* POP3 */

char			pop3_server[MAXHOSTNAMELEN];
					/* POP3 サーバホスト名(FQDN)	*/
char			pop3_server_file[PATH_BUFF] = POP3_SERVER_FILE;
					/* POP3 サーバ指定ファイル	*/
char			mail_spool[PATH_BUFF] = MAIL_SPOOL;
					/* メールスプール		*/
short			pop3_connect_mode = 2;
					/* POP3 接続モード		*/
					/* (0:MSPL,1:POP3,2:DEAD)	*/
short			pop3_rpop_mode = 0;
					/* POP3 RPOP 使用モード		*/
short			pop3_from_mode = 0;
					/* POP3 From 復元モード		*/
char			*pop3_connect_string[] = {
  "MSPL",
  "POP3",
  "DEAD",
};					/* POP3 接続モード文字列	*/

#ifdef	MSPL
static FILE		*spool_fp = (FILE*)NULL;
					/* スプールファイルポインタ	*/
#endif	/* MSPL */
#ifdef	POP3
static FILE		*head_fp = (FILE*)NULL;
					/* ヘッダ用ファイルポインタ	*/
static int		spool_num = 0;	/* スプール記事数		*/
static int		read_num = -1;	/* スプール参照数		*/
#ifdef	MSDOS
static int		read_fd = -1;	/* POP3 リードファイル記述子	*/
#else	/* !MSDOS */
static TCPRD_FILE	*read_fp = (TCPRD_FILE*)NULL;
					/* POP3 リードファイルポインタ	*/
#endif	/* !MSDOS */
static int		write_fd = -1;	/* POP3 ライトファイル記述子	*/
static unsigned int	port_number = 0;/* POP3 ポート番号		*/
#endif	/* POP3 */

#ifdef	POP3
#ifdef	__STDC__
static int	pop3_cmd(char*);/* POP3 コマンド送信/ステータス受信	*/
static int	pop3_auth(void);/* POP3 サーバ認証			*/
#else	/* !__STDC__ */
static int	pop3_cmd();	/* POP3 コマンド送信/ステータス受信	*/
static int	pop3_auth();	/* POP3 サーバ認証			*/
#endif	/* !__STDC__ */
#endif	/* POP3 */

/*
 * POP3 サーバ選択
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_select_server(char *server_name)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_select_server(server_name)
     char	*server_name;
#endif	/* !__STDC__ */
{
#ifdef	POP3
  struct hostent	*server_entry;
#endif	/* POP3 */
#ifdef	MSPL
  static int		first = 1;
  char			spool_dir[PATH_BUFF];
  struct stat		stat_buff;
#endif	/* MSPL */
  char			*ptr;
  POP3_ERR_CODE		status;

  switch (pop3_connect_mode) {
  case 0:					/* MSPL→POP3 接続	*/
#ifndef	MSPL
    pop3_connect_mode = 1;
#endif	/* MSPL */
    break;
  case 1:					/* POP3→MSPL 接続	*/
#ifndef	POP3
    pop3_connect_mode = 0;
#endif	/* POP3 */
    break;
  default:
    break;
  }
  status = POP3_ERR_COMM;
  switch (pop3_connect_mode) {
  case 0:						/* MSPL 接続	*/
#ifdef	MSPL
    if (first) {
      first = 0;
      strcpy(spool_dir, mail_spool);
      ptr = strrchr(spool_dir, SLASH_CHAR);
      if (ptr != (char*)NULL) {
	*ptr = '\0';
      }
      if (stat(spool_dir, &stat_buff)) {
	print_fatal("Can't stat mail spool directory.");
	break;
      }
#ifdef	POP3
      if (gethostname(pop3_server, sizeof(pop3_server))) {
	print_fatal("Can't get hostname.");
	break;
      }
#else	/* !POP3 */
      strcpy(pop3_server, "LOCAL");
#endif	/* !POP3 */
    }
    status = POP3_OK;
#else	/* !MSPL */
    print_fatal("MSPL mode unsupported.");
    status = POP3_ERR_RSV;
#endif	/* !MSPL */
    break;
  case 1:						/* POP3 接続	*/
#ifdef	POP3
    if (server_name != (char*)NULL) {
      strcpy(pop3_server, server_name);
      ptr = strchr(server_name, ':');
      if (ptr != (char*)NULL) {
	*ptr++ = '\0';
	if (atoi(ptr) > 0) {
	  port_number = atoi(ptr) & 0xFFFF;
	}
      }
    }
    if ((server_name == (char*)NULL) || (!server_name[0])) {
      if (tcp_get_default_server(pop3_server_file, pop3_server, "POP3SERVER",
				 &port_number)) {
	if (gethostname(pop3_server, sizeof(pop3_server))) {
	  print_fatal("Can't get hostname.");
	  break;
	}
      }
    }
#ifndef	DONT_USE_REALNAME
    server_entry = gethostbyname(pop3_server);
    if (server_entry != (struct hostent*)NULL) {
      strcpy(pop3_server, server_entry->h_name);
    } else {
      print_warning("Can't get POP3 server host entry.");
    }
#endif	/* !DONT_USE_REALNAME */
    status = POP3_OK;
#else	/* !POP3 */
    print_fatal("POP3 mode unsupported.");
    status = POP3_ERR_RSV;
#endif	/* !POP3 */
    break;
  default:
    break;
  }
  if (status != POP3_OK) {
    pop3_connect_mode = 2;
  }
  return(status);
}

/*
 * POP3 オープン
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_open(void)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_open()
#endif	/* !__STDC__ */
{
#ifdef	POP3
  char		buff[SMALL_BUFF];
#endif	/* POP3 */
  POP3_ERR_CODE	status;

#ifdef	MSPL
  if (spool_fp != (FILE*)NULL) {
    print_fatal("MSPL server already open.");
    return(POP3_ERR_PROG);
  }
#endif	/* MSPL */
#ifdef	POP3
  if (head_fp != (FILE*)NULL) {
    fclose(head_fp);
    head_fp = (FILE*)NULL;
  }
  read_num = -1;
#ifdef	MSDOS
  if ((write_fd != -1) || (read_fd != -1)) {
#else	/* !MSDOS */
  if ((write_fd != -1) || (read_fp != (TCPRD_FILE*)NULL)) {
#endif	/* !MSDOS */
    print_fatal("POP3 server already open.");
    return(POP3_ERR_PROG);
  }
#endif	/* POP3 */
  status = POP3_ERR_COMM;
  switch (pop3_connect_mode) {
  case 0:						/* MSPL 接続	*/
#ifdef	MSPL
#ifdef	POP3_DEBUG
    print_warning("Connecting to MSPL server %s.", pop3_server);
#endif	/* POP3_DEBUG */
    status = POP3_OK;
#else	/* !MSPL */
    print_fatal("MSPL mode unsupported.");
    status = POP3_ERR_RSV;
#endif	/* !MSPL */
    break;
  case 1:						/* POP3 接続	*/
#ifdef	POP3_DEBUG
    print_warning("Connecting to POP3 server %s.", pop3_server);
#endif	/* POP3_DEBUG */
#ifdef	POP3
    if (pop3_rpop_mode) {
#ifdef	MSDOS
      if (tcp_open(pop3_server, "RPOP", "rpop", port_number, -1,
		   &read_fd, &write_fd)) {
#else	/* !MSDOS */
      if (tcp_open(pop3_server, "RPOP", "rpop", port_number, -1,
		   &read_fp, &write_fd)) {
#endif	/* !MSDOS */
	print_fatal("Can't connect RPOP server %s.", pop3_server);
	break;
      }
    } else {
#ifdef	MSDOS
      if (tcp_open(pop3_server, "POP3", "pop3", port_number, POP3_PORT,
		   &read_fd, &write_fd)) {
#else	/* !MSDOS */
      if (tcp_open(pop3_server, "POP3", "pop3", port_number, POP3_PORT,
		   &read_fp, &write_fd)) {
#endif	/* !MSDOS */
	print_fatal("Can't connect POP3 server %s.", pop3_server);
	break;
      }
    }
    
    /*
     * POP3 サーバの接続メッセージ取得
     */
    
    if (pop3_receive(buff, sizeof(buff))) {
      break;
    }
    if (strncmp(buff, POP3_OKAY_RES, sizeof(POP3_OKAY_RES) - 1)) {
      print_fatal("POP3 error at connect sequence.");
      print_fatal("%s", buff);
      break;
    } else {
      status = POP3_OK;
    }
#else	/* !POP3 */
    print_fatal("POP3 mode unsupported.");
    status = POP3_ERR_RSV;
#endif	/* !POP3 */
    break;
  case 2:
    status = POP3_ERR_RSV;
    /* break 不要 */
  default:
    break;
  }
  if (status != POP3_OK) {
    pop3_close();
    pop3_connect_mode = 2;
  }
  return(status);
}

/*
 * POP3 クローズ
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_close(void)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_close()
#endif	/* !__STDC__ */
{
  POP3_ERR_CODE	status;

  status = POP3_ERR_COMM;
  switch (pop3_connect_mode) {
  case 0:						/* MSPL 接続	*/
#ifdef	MSPL
    if (spool_fp != (FILE*)NULL) {
#ifdef	USE_FLOCK
      flock(fileno(fp), LOCK_UN);
#endif	/* USE_FLOCK */
      fclose(spool_fp);
      spool_fp = (FILE*)NULL;
    }
    status = POP3_OK;
#endif	/* MSPL */
    break;
  case 1:						/* POP3 接続	*/
#ifdef	POP3
    if (head_fp != (FILE*)NULL) {
      fclose(head_fp);
      head_fp = (FILE*)NULL;
    }
#ifdef	MSDOS
    if ((write_fd == -1) || (read_fd == -1)) {
      break;
    }
#else	/* !MSDOS */
    if ((write_fd == -1) || (read_fp == (TCPRD_FILE*)NULL)) {
      break;
    }
#endif	/* !MSDOS */
    pop3_cmd(POP3_QUIT_CMD);
#ifdef	MSDOS
    tcp_close(&read_fd, &write_fd);
#else	/* !MSDOS */
    tcp_close(&read_fp, &write_fd);
#endif	/* !MSDOS */
    status = POP3_OK;
#endif	/* POP3 */
    break;
  default:
    break;
  }
  return(status);
}

/*
 * POP3 送信
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_send(char *string)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_send(string)
     char	*string;
#endif	/* !__STDC__ */
{
  switch (pop3_connect_mode) {
  case 0:						/* MSPL 接続	*/
    print_fatal("MSPL write does not need.");
    break;
  case 1:						/* POP3 接続	*/
#ifdef	POP3
#ifdef	MSDOS
    if (tcp_send("POP3", &read_fd, &write_fd, string)) {
      return(POP3_ERR_COMM);
    } else {
      return(POP3_OK);
    }
#else	/* !MSDOS */
    if (tcp_send("POP3", &read_fp, &write_fd, string)) {
      return(POP3_ERR_COMM);
    } else {
      return(POP3_OK);
    }
#endif	/* !MSDOS */
#endif	/* POP3 */
    break;
  default:
    break;
  }
  return(POP3_ERR_PROG);
}

/*
 * POP3 受信
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_receive(char *ptr, int size)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_receive(ptr, size)
     char	*ptr;
     int	size;
#endif	/* !__STDC__ */
{
  switch (pop3_connect_mode) {
  case 0:						/* MSPL 接続	*/
    print_fatal("MSPL read does not need.");
    break;
  case 1:						/* POP3 接続	*/
#ifdef	POP3
#ifdef	MSDOS
    if (tcp_receive("POP3", &read_fd, &write_fd, ptr, size)) {
      return(POP3_ERR_COMM);
    } else {
      return(POP3_OK);
    }
#else	/* !MSDOS */
    if (tcp_receive("POP3", &read_fp, &write_fd, ptr, size)) {
      return(POP3_ERR_COMM);
    } else {
      return(POP3_OK);
    }
#endif	/* !MSDOS */
#endif	/* POP3 */
    break;
  default:
    break;
  }
  return(POP3_ERR_PROG);
}

/*
 * POP3 スプールリード
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_read(char *ptr, int size, POP3_RD_STAT *stat_ptr)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_read(ptr, size, stat_ptr)
     char		*ptr;
     int		size;
     POP3_RD_STAT	*stat_ptr;
#endif	/* !__STDC__ */
{
  static long		length1, length2;
#ifdef	POP3
  static char		*month_string[] = {	/* 月名			*/
  "???", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  static char		*day_string[] = {	/* 曜日名		*/
  "???", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  char			tmp_file[PATH_BUFF];
  short			year, month, date, day, hour, minute, second;
  char			from_buff[BUFF_SIZE];
  char			date_buff[MAX_FIELD_LEN];
  static struct cpy_hdr	unixfrom_fields[] = {
    {FROM_FIELD,	(char*)NULL,	sizeof(from_buff)},
    {DATE_FIELD,	(char*)NULL,	sizeof(date_buff)},
  };
#endif	/* POP3 */
  static POP3_RD_STAT	rd_stat;
  static int		sp_flag;
  char			buff[BUFF_SIZE];
  POP3_ERR_CODE		status;

  status = POP3_ERR_COMM;
  *ptr = '\0';
  if (size <= 0) {
    return(POP3_ERR_PROG);
  }
  switch (pop3_connect_mode) {
  case 0:						/* MSPL 接続	*/
#ifdef	MSPL
    if (spool_fp == (FILE*)NULL) {
      status = POP3_OK;
      spool_fp = fopen(mail_spool, "r");
      if (spool_fp == (FILE*)NULL) {
	rd_stat = POP3_RD_EOF;
	break;
      }
#ifdef	USE_FLOCK
      flock(fileno(spool_fp), LOCK_EX);
#endif	/* USE_FLOCK */
      if (!fgets(ptr, size, spool_fp)) {
	*ptr = '\0';
	rd_stat = POP3_RD_EOF;
	break;
      }
      if (*ptr == '\n') {
	if (!fgets(ptr, size, spool_fp)) {
	  *ptr = '\0';
	  rd_stat = POP3_RD_EOF;
	  break;
	}
      }
      if (strncmp(ptr, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
	*ptr = '\0';
	status = POP3_ERR_FILE;
	rd_stat = POP3_RD_EOF;
	break;
      }
      rd_stat = POP3_RD_FROM;
      length1 = length2 = 0;
      status = POP3_OK;
      if (pop3_from_mode) {
	break;
      }
    }
    status = POP3_OK;
    if (!fgets(ptr, size, spool_fp)) {
      *ptr = '\0';
      rd_stat = POP3_RD_EOF;;
      break;
    }
    switch (rd_stat) {
    case POP3_RD_FROM:
    case POP3_RD_END:
      rd_stat = POP3_RD_HEAD;
      length1 = length2 = 0;
      break;
    case POP3_RD_HEAD:
      if (!strncasecmp(ptr, LENGTH_FIELD, sizeof(LENGTH_FIELD) - 1)) {
	length1 = atoi(ptr + sizeof(LENGTH_FIELD) - 1);
      }
      if (*ptr == '\n') {
	rd_stat = POP3_RD_SEP;
      }
      break;
    case POP3_RD_SEP:
      rd_stat = POP3_RD_BODY;
      sp_flag = 1;
      length2 += strlen(ptr);
      break;
    case POP3_RD_BODY:
      if (!strncmp(ptr, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
	if (sp_flag && (length1 ? length2 >= length1 : 1)) {
	  if (pop3_from_mode) {
	    rd_stat = POP3_RD_FROM;
	  } else {
	    rd_stat = POP3_RD_END;
	  }
	  break;
	} else {
	  length2 += strlen(ptr);
	  strcpy(buff, ptr);
	  *ptr = '>';
	  strcpy(ptr + 1, buff);
	}
	sp_flag = 0;
      } else {
	sp_flag = (*ptr == '\n');
	length2 += strlen(ptr);
      }
      break;
    default:
      rd_stat = POP3_RD_EOF;		/* エラー		*/
      break;
    }
#else	/* !MSPL */
    status = POP3_ERR_RSV;
    rd_stat = POP3_RD_EOF;
#endif	/* !MSPL */
    break;
  case 1:						/* POP3 接続	*/
#ifdef	POP3
    if (read_num < 0) {
      if (pop3_auth()) {
	break;
      }
      if (pop3_send(POP3_STAT_CMD)) {
	break;
      }
      if (pop3_receive(buff, sizeof(buff))) {
	break;
      }
      if (strncmp(buff, POP3_OKAY_RES, sizeof(POP3_OKAY_RES) - 1)) {
	break;
      }
      spool_num = atoi(&buff[4]);
      read_num = 0;
      rd_stat = POP3_RD_END;
      status = POP3_OK;
    }
    if ((rd_stat == POP3_RD_FROM) || (rd_stat == POP3_RD_END)) {
      if (read_num < spool_num) {
	sprintf(buff, "%s %d", POP3_RETR_CMD, read_num + 1);
	if (pop3_cmd(buff)) {
	  rd_stat = POP3_RD_EOF;
	  break;
	}
      } else {
	rd_stat = POP3_RD_EOF;
	break;
      }
    }
    if (head_fp != (FILE*)NULL) {
      if (!fgets(buff, sizeof(buff), head_fp)) {
	fclose(head_fp);
	head_fp = (FILE*)NULL;
	rd_stat = POP3_RD_EOF;
	break;
      }
    } else {
      if (pop3_receive(buff, sizeof(buff))) {
	rd_stat = POP3_RD_EOF;
	break;
      }
      strcat(buff, "\n");
    }
    status = POP3_OK;
    strncpy(ptr, buff, size);
    *(ptr + size - 1) = '\0';
    switch (rd_stat) {
    case POP3_RD_END:
    case POP3_RD_FROM:
      length1 = length2 = 0;
      rd_stat = POP3_RD_HEAD;
      if (pop3_from_mode) {
	sprintf(tmp_file, "%s%cpop3_%03d.%d", tmp_dir, SLASH_CHAR, read_num,
		(int)getpid());
	unixfrom_fields[0].field_buff = from_buff;
	unixfrom_fields[1].field_buff = date_buff;
	if (head_fp != (FILE*)NULL) {
	  fclose(head_fp);
	}
	head_fp = fopen(tmp_file, "w");
	if (head_fp != (FILE*)NULL) {
	  do {
	    if (pop3_receive(buff, sizeof(buff))) {
	      rd_stat = POP3_RD_EOF;
	      break;
	    }
	    strcat(buff, "\n");
	    fputs(buff, head_fp);
	  } while (buff[0] != '\n');
	  head_fp = freopen(tmp_file, "r", head_fp);
	  unlink(tmp_file);
	  if (head_fp == (FILE*)NULL) {
	    rd_stat = POP3_RD_EOF;
	    status = POP3_ERR_FILE;
	    break;
	  }
	  copy_fields(head_fp, unixfrom_fields,
		      sizeof(unixfrom_fields) / sizeof(struct cpy_hdr),
		      CF_CLR_MASK | CF_GET_MASK);
	  convert_article_date(date_buff, &year, &month, &date, &day,
			       &hour, &minute, &second, buff);
	  get_real_adrs(from_buff, buff);
	  sprintf(ptr, "%s%s %s %s %2d %02d:%02d:%02d %4d\n", MSPL_SEPARATER,
		  buff, day_string[day], month_string[month],
		  date, hour, minute, second, year);
	  fseek(head_fp, 0L, 0);
	} else {
	  sprintf(ptr, "%s%s %s %s %2d %02d:%02d:%02d %4d\n", MSPL_SEPARATER,
		  "?", day_string[0], month_string[0], 0, 0, 0, 0, 1970);
	  status = POP3_ERR_FILE;
	}
	break;
      }
      /* break 不要 */
    case POP3_RD_HEAD:
      if (!strncasecmp(ptr, LENGTH_FIELD, sizeof(LENGTH_FIELD) - 1)) {
	length1 = atoi(ptr + sizeof(LENGTH_FIELD) - 1);
      }
      if (*ptr == '\n') {
	if (head_fp != (FILE*)NULL) {
	  fclose(head_fp);
	  head_fp = (FILE*)NULL;
	}
	rd_stat = POP3_RD_SEP;
      }
      break;
    case POP3_RD_SEP:
      rd_stat = POP3_RD_BODY;
      sp_flag = 1;
      break;
    case POP3_RD_BODY:
      if (!strncmp(ptr, MSPL_SEPARATER, sizeof(MSPL_SEPARATER) - 1)) {
	strcpy(buff, ptr);
	*ptr = '>';
	strcpy(ptr + 1, buff);
	sp_flag = 0;
      } else if (*ptr == TCP_END_MARK) {
	strncpy(ptr, ptr + 1, size);
	if (*ptr == '\n') {
	  read_num++;
	  if (read_num < spool_num) {
	    rd_stat = POP3_RD_END;
	  } else {
	    rd_stat = POP3_RD_EOF;
	  }
	}
      } else {
	sp_flag = (*ptr == '\n');
      }
      break;
    default:
      rd_stat = POP3_RD_EOF;		/* エラー		*/
      break;
    }
#else	/* !POP3 */
    status = POP3_ERR_RSV;
    rd_stat = POP3_RD_EOF;
#endif	/* !POP3 */
    break;
  default:
    rd_stat = POP3_RD_EOF;
    break;
  }
  *stat_ptr = rd_stat;
  return(status);
}

/*
 * POP3 スプールチェック
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_stat(int mode)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_stat(mode)
     int	mode;
#endif	/* !__STDC__ */
{
#ifdef	MSPL
  struct stat	stat_buff;
#endif	/* MSPL */
#ifdef	POP3
  char		buff[BUFF_SIZE];
#endif	/* POP3 */
  POP3_ERR_CODE	status;

  if (mode) {
    status = pop3_open();
  } else {
    status = POP3_OK;
  }
  if (status == POP3_OK) {
    switch (pop3_connect_mode) {
    case 0:						/* MSPL 接続	*/
#ifdef	MSPL
      if ((!stat(mail_spool, &stat_buff)) && (stat_buff.st_size > 0)) {
	status = POP3_OK;
      } else {
	status = POP3_ERR_NONE;
      }
#endif	/* MSPL */
      break;
    case 1:						/* POP3 接続	*/
#ifdef	POP3
      status = pop3_auth();
      if (status != POP3_OK) {
	break;
      }
      status = POP3_ERR_COMM;
      if (pop3_send(POP3_STAT_CMD)) {
	break;
      }
      if (pop3_receive(buff, sizeof(buff))) {
	break;
      }
      if (strncmp(buff, POP3_OKAY_RES, sizeof(POP3_OKAY_RES) - 1)) {
	break;
      }
      if (atoi(&buff[4]) > 0) {
	status = POP3_OK;
      } else {
	status = POP3_ERR_NONE;
      }
#endif	/* POP3 */
      break;
    default:
      break;
    }
    pop3_close();
  }
  return(status);
}

/*
 * POP3 スプール削除
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_delete(void)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_delete()
#endif	/* !__STDC__ */
{
#ifdef	MSPL
#ifndef	SVR4
  FILE		*fp;
#endif	/* !SVR4 */
#endif	/* MSPL */
#ifdef	POP3
  char		buff[BUFF_SIZE];
  register int	i;
#endif	/* POP3 */

  switch (pop3_connect_mode) {
  case 0:						/* MSPL 接続	*/
#ifdef	MSPL
#ifdef	SVR4
    if (!truncate(mail_spool, 0)) {
      return(POP3_OK);
    }
#else	/* !SVR4 */
    fp = fopen(mail_spool, "w");
    if (fp != (FILE*)NULL) {
      if (!fclose(fp)) {
	return(POP3_OK);
      }
    }
#endif	/* !SVR4 */
    print_fatal("Can't truncate spool file.");
#endif	/* MSPL */
    break;
  case 1:						/* POP3 接続	*/
#ifdef	POP3
    for (i = 0; i < read_num; i++) {
      sprintf(buff, "%s %d", POP3_DELE_CMD, i + 1);
      if (pop3_cmd(buff)) {
	i = 0;
	break;
      }
    }
    if (i) {
      return(POP3_OK);
    }
#endif	/* POP3 */
    break;
  default:
    break;
  }
  return(POP3_ERR_COMM);
}

/*
 * POP3 接続確認
 */

#ifdef	__STDC__
POP3_ERR_CODE	pop3_noop(void)
#else	/* !__STDC__ */
POP3_ERR_CODE	pop3_noop()
#endif	/* !__STDC__ */
{
  switch (pop3_connect_mode) {
  case 0:						/* MSPL 接続	*/
    break;
  case 1:						/* POP3 接続	*/
#ifdef	POP3
    return(pop3_cmd(POP3_NOOP_CMD));
#endif	/* POP3 */
    break;
  default:
    break;
  }
  return(POP3_OK);
}

#ifdef	POP3
/*
 * POP3 サーバ認証
 */

#ifdef	__STDC__
static int	pop3_auth(void)
#else	/* !__STDC__ */
static int	pop3_auth()
#endif	/* !__STDC__ */
{
  static char	user[SMALL_BUFF];
  static char	pass[SMALL_BUFF];
  static int	first = 1;
  char		buff[BUFF_SIZE];
  POP3_ERR_CODE	status;

  if (first) {
    if (pop3_rpop_mode) {
      tcp_get_user_pass(pop3_server, user, (char*)NULL);
      strcpy(pass, user);
    } else {
      tcp_get_user_pass(pop3_server, user, pass);
    }
  }
  sprintf(buff, "%s %s", POP3_USER_CMD, user);
  status = pop3_cmd(buff);
  if (status != POP3_OK) {
    return(status);
  }
  sprintf(buff, "%s %s", pop3_rpop_mode ? POP3_RPOP_CMD : POP3_PASS_CMD, pass);
  status = pop3_cmd(buff);
  if (status != POP3_OK) {
    return(status);
  }
  first = 0;
  return(POP3_OK);			/* Authentication accepted */
}

/*
 * POP3 コマンド送信/ステータス受信
 */

#ifdef	__STDC__
static int	pop3_cmd(char *cmd)
#else	/* !__STDC__ */
static int	pop3_cmd(cmd)
     char	*cmd;
#endif	/* !__STDC__ */
{
  char		*ptr1;
  char		*ptr2;
  char		buff1[BUFF_SIZE];
  char		buff2[SMALL_BUFF];
  POP3_ERR_CODE	status;

  status = pop3_send(cmd);
  if (status != POP3_OK) {
    return(status);
  }
  status = pop3_receive(buff1, sizeof(buff1));
  if (status != POP3_OK) {
    return(status);
  }
  if (!strncmp(buff1, POP3_OKAY_RES, sizeof(POP3_OKAY_RES) - 1)) {
    return(POP3_OK);
  }
  ptr1 = buff2;
  ptr2 = cmd;
  while (*ptr2) {
    if (*ptr2 <= ' ') {
      break;
    }
    *ptr1++ = *ptr2++;
  }
  *ptr1 = '\0';
  print_fatal("POP3 error at %s command.", buff2);
  print_fatal("%s", buff1);
  return(POP3_ERR_PROT);
}
#endif	/* POP3 */
