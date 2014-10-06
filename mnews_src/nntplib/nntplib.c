/*
 *
 *  NNTP ライブラリ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : NNTP protocol library
 *  Sub system  : NNTP library
 *  File        : nntplib.c
 *  Version     : 2.46
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-12/12
 *  Author      : MSR24 宅間 顯
 *
 */

#include	"compat.h"
#include	"nntplib.h"
#include	"field.h"

char			nntp_server[MAXHOSTNAMELEN];
					/* NNTP サーバホスト名(FQDN)	*/
char			nntp_server_file[PATH_BUFF] = NNTP_SERVER_FILE;
					/* NNTP サーバ指定ファイル	*/
char			post_program[PATH_BUFF] = POST_PROGRAM;
					/* 投稿プログラム		*/
char			news_spool[PATH_BUFF] = NEWS_SPOOL;
					/* ニューススプール		*/
char			news_lib[PATH_BUFF] = NEWS_LIB;
					/* ニュースライブラリ		*/
char			ignore_groups[BUFF_SIZE] = {0, 0};
					/* 無視するニュースグループ群	*/
short			ignore_mode = 0;
					/* ニュースグループ無視モード	*/
short			nntp_connect_mode = 2;
					/* NNTP 接続モード		*/
					/* (0:NSPL,1:NNTP,2:DEAD)	*/
char			*nntp_connect_string[] = {
  "NSPL",
  "NNTP",
  "DEAD",
};					/* NNTP 接続モード文字列	*/
#ifdef	XOVER
short			xover_mode = 1;	/* XOVER 有効スイッチ		*/
#endif	/* XOVER */
short			post_enable = 1;/* 投稿許可フラグ		*/
short			force_use_inews = 0;
					/* 強制 inews 使用フラグ	*/
int			group_number;	/* グループ数			*/
#ifdef	NNTP
#ifdef	MSDOS
static int		read_fd = -1;	/* NNTP リードファイル記述子	*/
#else	/* !MSDOS */
static TCPRD_FILE	*read_fp = (TCPRD_FILE*)NULL;
					/* NNTP リードファイルポインタ	*/
#endif	/* !MSDOS */
static int		write_fd = -1;	/* NNTP ライトファイル記述子	*/
static unsigned int	port_number = 0;/* NNTP ポート番号		*/
#ifdef	NNTP_AUTH
char			nntp_user[SMALL_BUFF] = "";
					/* NNTP 認証名			*/
short			nntp_auth_mode = 0;
					/* NNTP 認証モード		*/
short			nntp_get_pass = 0;
					/* NNTP のパスワード入力フラグ	*/
#endif	/* NNTP_AUTH */
#endif	/* NNTP */
#ifdef	NSPL
static char		current_group[SMALL_BUFF];
					/* カレントニュースグループ名	*/
#endif	/* NSPL */
static int		too_many;	/* too many 警告表示フラグ	*/
#ifdef	NG_ALLOC
struct news_group	*news_group = (struct news_group*)NULL;
					/* ニュースグループ情報		*/
char			*name_pool = (char*)NULL;
					/* ニュースグループ名保存空間	*/
long			name_used_size = 0;
					/* ニュースグループ名使用サイズ	*/
static long		name_alloc_size = 0;
					/* ニュースグループ名確保サイズ	*/
static int		news_alloc_number = 0;
					/* ニュースグループ名確保回数	*/
#else	/* !NG_ALLOC */
struct news_group	news_group[MAX_NEWS_GROUP];
					/* ニュースグループ情報		*/
#endif	/* !NG_ALLOC */

#ifdef	__STDC__
static int	add_group(char*);	/* ニュースグループ登録		*/
#ifdef	NG_ALLOC
static int	compare_group(struct news_group*, struct news_group*);
					/* ニュースグループ名比較	*/
#endif	/* NG_ALLOC */
#ifdef	NSPL
static int	nspl_search_article(char*);
					/* 記事検索			*/
static void	post_pipe_error(void);	/* パイプエラーハンドラ		*/
#endif	/* NSPL */
#ifdef	NNTP
static NNTP_ERR_CODE	nntp_auth(void);/* NNTP サーバ認証		*/
static int	inn_check(char*);	/* INN チェック			*/
#ifdef	XOVER
extern int	xover_copy_field(int, char*);/* XOVER コピーフィールド	*/
#endif	/* XOVER */
#endif	/* NNTP */
static int	atoi2(char**);		/* 文字列数値化			*/
#else	/* !__STDC__ */
static int	add_group();		/* ニュースグループ登録		*/
#ifdef	NG_ALLOC
static int	compare_group();	/* ニュースグループ名比較	*/
#endif	/* NG_ALLOC */
#ifdef	NSPL
static int	nspl_search_article();	/* 記事検索			*/
static void	post_pipe_error();	/* パイプエラーハンドラ		*/
#endif	/* NSPL */
#ifdef	NNTP
static NNTP_ERR_CODE	nntp_auth();	/* NNTP サーバ認証		*/
static int	inn_check();		/* INN チェック			*/
#ifdef	XOVER
extern int	xover_copy_field();	/* XOVER コピーフィールド	*/
#endif	/* XOVER */
#endif	/* NNTP */
static int	atoi2();		/* 文字列数値化			*/
#endif	/* !__STDC__ */

/*
 * NNTP サーバ選択
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_select_server(char *server_name)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_select_server(server_name)
     char	*server_name;
#endif	/* !__STDC__ */
{
#ifdef	NNTP
  struct hostent	*server_entry;
#endif	/* NNTP */
#ifdef	NSPL
  struct stat 		stat_buff;
#endif	/* NSPL */
  char			buff[PATH_BUFF];
  char			*ptr;
  NNTP_ERR_CODE		status;

  switch (nntp_connect_mode) {
  case 0:					/* NSPL→NNTP 接続	*/
#ifndef	NSPL
    nntp_connect_mode = 1;
#endif	/* NSPL */
    break;
  case 1:					/* NNTP→NSPL 接続	*/
#ifndef	NNTP
    nntp_connect_mode = 0;
#endif	/* NNTP */
    break;
  default:
    break;
  }
  status = NNTP_ERR_COMM;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    if (stat(news_lib, &stat_buff)) {
      print_fatal("Can't stat news library directory.");
      break;
    } else if (stat(news_spool, &stat_buff)) {
      print_fatal("Can't stat news spool directory.");
      break;
    }
    sprintf(buff, "%s%c%s", news_lib, SLASH_CHAR, NEWS_ACTIVE_FILE);
    if (stat(buff, &stat_buff)) {
      print_fatal("Can't stat news active file.");
      break;
    }
#ifdef	NNTP
    if (gethostname(nntp_server, sizeof(nntp_server))) {
      print_fatal("Can't get hostname.");
      break;
    }
#else	/* !NNTP */
    strcpy(nntp_server, "LOCAL");
#endif	/* !NNTP */
    status = NNTP_OK;
#else	/* !MSPL */
    print_fatal("NSPL mode unsupported.");
    status = NNTP_ERR_RSV;
#endif	/* !NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
    if (server_name != (char*)NULL) {
      strcpy(nntp_server, server_name);
      ptr = strchr(server_name, ':');
      if (ptr != (char*)NULL) {
	*ptr++ = '\0';
	if (atoi(ptr) > 0) {
	  port_number = atoi(ptr) & 0xFFFF;
	}
      }
    }
    if ((server_name == (char*)NULL) || (!server_name[0])) {
      if (tcp_get_default_server(nntp_server_file, nntp_server, "NNTPSERVER",
				 &port_number)) {
	if (gethostname(nntp_server, sizeof(nntp_server))) {
	  print_fatal("Can't get hostname.");
	  break;
	}
      }
    }
#ifndef	DONT_USE_REALNAME
    server_entry = gethostbyname(nntp_server);
    if (server_entry != (struct hostent*)NULL) {
      strcpy(nntp_server, server_entry->h_name);
    } else {
      print_warning("Can't get NNTP server host entry.");
    }
#endif	/* !DONT_USE_REALNAME */
    status = NNTP_OK;
#else	/* !NNTP */
    print_fatal("NNTP mode unsupported.");
    status = NNTP_ERR_RSV;
#endif	/* !NNTP */
    break;
  default:
    break;
  }
  if (status != NNTP_OK) {
    nntp_connect_mode = 2;
  }
  return(status);
}

/*
 * NNTP オープン
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_open(void)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_open()
#endif	/* !__STDC__ */
{
  char		buff[SMALL_BUFF];
#ifdef	NSPL
  struct stat	stat_buff;
  char		*ptr;
#endif	/* NSPL */
  NNTP_ERR_CODE	status;

  post_enable = 0;
#ifdef	SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif	/* SIGPIPE */
#ifdef	NNTP
#ifdef	MSDOS
  if ((write_fd != -1) || (read_fd != -1)) {
#else	/* !MSDOS */
  if ((write_fd != -1) || (read_fp != (TCPRD_FILE*)NULL)) {
#endif	/* !MSDOS */
    print_fatal("NNTP server already open.");
    return(NNTP_ERR_PROG);
  }
#endif	/* NNTP */
  status = NNTP_ERR_COMM;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NNTP_DEBUG
    print_warning("Connecting to NSPL server %s.", nntp_server);
#endif	/* NNTP_DEBUG */
#ifdef	NSPL
    status = NNTP_OK;
#else	/* !NSPL */
    print_fatal("NSPL mode unsupported.");
#endif	/* !NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP_DEBUG
    print_warning("Connecting to NNTP server %s.", nntp_server);
#endif	/* NNTP_DEBUG */
#ifdef	NNTP
#ifdef	MSDOS
    if (tcp_open(nntp_server, "NNTP", "nntp", port_number, NNTP_PORT,
		 &read_fd, &write_fd)) {
#else	/* !MSDOS */
    if (tcp_open(nntp_server, "NNTP", "nntp", port_number, NNTP_PORT,
		 &read_fp, &write_fd)) {
#endif	/* !MSDOS */
      print_fatal("Can't connect NNTP server %s.", nntp_server);
      break;
    }
    group_number = 0;
    
    /*
     * NNTP サーバの接続メッセージ取得
     */
    
    if (nntp_receive(buff, sizeof(buff))) {
      break;
    }
    switch (atoi(buff)) {
    case NNTP_OPEN_RES1:
      post_enable = 1;
      /* break 不要 */
    case NNTP_OPEN_RES2:
      status = NNTP_OK;
      break;
    case NNTP_NO_SERVICE_RES:
      print_warning("NNTP server service discontinued.");
      print_warning("%s", buff);
#ifdef	MSDOS
      tcp_close(&read_fd, &write_fd);
#else	/* !MSDOS */
      tcp_close(&read_fp, &write_fd);
#endif	/* !MSDOS */
      status = NNTP_ERR_SERV;
      break;
    default:
      print_fatal("Unexpected NNTP reply in connect sequence.");
      print_fatal("%s", buff);
      break;
    }
    if (status == NNTP_OK) {
      
      /* Server check innd or not */
      
      if (inn_check(buff)) {
	status = NNTP_ERR_COMM;
	if (nntp_send(NNTP_READER_CMD)) {
	  break;
	}
	if (nntp_receive(buff, sizeof(buff))) {
	  break;
	}
	switch (atoi(buff)) {
	case NNTP_OPEN_RES1:
	  post_enable = 1;
	  /* break 不要 */
	case NNTP_OPEN_RES2:
	  status = NNTP_OK;
	  break;
	case NNTP_NO_SERVICE_RES:
	  print_warning("NNTP server service discontinued.");
	  print_warning("%s", buff);
	  status = NNTP_ERR_SERV;
	  break;
	default:
	  print_fatal("Unexpected NNTP reply in connect sequence.");
	  print_fatal("%s", buff);
	  status = NNTP_ERR_PROT;
	  break;
	}
      }
    }
    if (status == NNTP_OK) {
      status = nntp_auth();
    }
#else	/* !NNTP */
    print_fatal("NNTP mode unsupported.");
#endif	/* !NNTP */
    break;
  case 2:
    status = NNTP_ERR_RSV;
    /* break 不要 */
  default:
    break;
  }
  if (status != NNTP_OK) {
    nntp_close();
    nntp_connect_mode = 2;
  } else {
    if ((!nntp_connect_mode) || force_use_inews) {
      post_enable = 0;
#ifdef	NSPL
      strcpy(buff, post_program);
      ptr = strchr(buff, ' ');
      if (ptr != (char*)NULL) {
	*ptr = '\0';
      }
      if ((strchr(buff, SLASH_CHAR) == (char*)NULL) ||
	  (!stat(buff, &stat_buff))) {
	post_enable = 1;
      }
#endif	/* NSPL */
    }
  }
  return(status);
}

/*
 * NNTP クローズ
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_close(void)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_close()
#endif	/* !__STDC__ */
{
#ifdef	NNTP
  char		buff[BUFF_SIZE];
#endif	/* NNTP */
  NNTP_ERR_CODE	status;

  status = NNTP_ERR_COMM;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    status = NNTP_OK;
#endif	/* NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
#ifdef	MSDOS
    if ((write_fd == -1) || (read_fd == -1)) {
      break;
    }
#else	/* !MSDOS */
    if ((write_fd == -1) || (read_fp == (TCPRD_FILE*)NULL)) {
      break;
    }
#endif	/* !MSDOS */
    if (nntp_send(NNTP_QUIT_CMD)) {
      break;
    } else if (nntp_receive(buff, sizeof(buff))) {
      break;
    } else {
      if (atoi(buff) != NNTP_QUIT_RES) {
	print_warning("Unexpected NNTP reply in QUIT command.");
	print_warning("%s", buff);
	break;
      }
    }
#ifdef	MSDOS
    tcp_close(&read_fd, &write_fd);
#else	/* !MSDOS */
    tcp_close(&read_fp, &write_fd);
#endif	/* !MSDOS */
    status = NNTP_OK;
#endif	/* NNTP */
    break;
  default:
    break;
  }
  return(status);
}

/*
 * NNTP 送信
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_send(char *string)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_send(string)
     char	*string;
#endif	/* !__STDC__ */
{
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
    print_fatal("NSPL write does not need.");
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
#ifdef	MSDOS
    if (tcp_send("NNTP", &read_fd, &write_fd, string)) {
      return(NNTP_ERR_COMM);
    } else {
      return(NNTP_OK);
    }
#else	/* !MSDOS */
    if (tcp_send("NNTP", &read_fp, &write_fd, string)) {
      return(NNTP_ERR_COMM);
    } else {
      return(NNTP_OK);
    }
#endif	/* !MSDOS */
#endif	/* NNTP */
    break;
  default:
    break;
  }
  return(NNTP_ERR_PROG);
}

/*
 * NNTP 受信
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_receive(char *ptr, int size)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_receive(ptr, size)
     char	*ptr;
     int	size;
#endif	/* !__STDC__ */
{
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
    print_fatal("NSPL read does not need.");
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
#ifdef	MSDOS
    if (tcp_receive("NNTP", &read_fd, &write_fd, ptr, size)) {
      return(NNTP_ERR_COMM);
    } else {
      return(NNTP_OK);
    }
#else	/* !MSDOS */
    if (tcp_receive("NNTP", &read_fp, &write_fd, ptr, size)) {
      return(NNTP_ERR_COMM);
    } else {
      return(NNTP_OK);
    }
#endif	/* !MSDOS */
#endif	/* NNTP */
    break;
  default:
    break;
  }
  return(NNTP_ERR_PROG);
}

/*
 * NNTP サーバ認証
 */

#ifdef	__STDC__
static NNTP_ERR_CODE	nntp_auth(void)
#else	/* !__STDC__ */
static NNTP_ERR_CODE	nntp_auth()
#endif	/* !__STDC__ */
{
#ifdef	NNTP_AUTH
  static char	user[SMALL_BUFF];
  static char	pass[SMALL_BUFF];
  static int	first = 1;
  char		buff[BUFF_SIZE];

  if (!nntp_auth_mode) {
    return(NNTP_OK);
  }
  if (first) {
    nntp_get_pass = 1;
    tcp_get_user_pass(nntp_server, user, pass);
    nntp_get_pass = 0;
  }
  if (!*user) {
    return(NNTP_OK);
  }
  sprintf(buff, "%s %s", NNTP_USER_CMD, user);
  if (nntp_send(buff)) {
    return(NNTP_ERR_COMM);
  }
  if (nntp_receive(buff, sizeof(buff))) {
    return(NNTP_ERR_COMM);
  }
  switch (atoi(buff)) {
  case NNTP_AUTHCOMP_RES:
  case NNTP_AUTHDONE_RES:
    first = 0;
    return(NNTP_OK);
    break;
  case NNTP_AUTHERR_RES:
    print_warning("%s", buff);
    return(NNTP_ERR_AUTH);
  case NNTP_PASS_RES:
    break;
  default:
    print_fatal("Unexpected NNTP reply in USER command.");
    print_warning("%s", buff);
    return(NNTP_ERR_PROT);
  }
  sprintf(buff, "%s %s", NNTP_PASS_CMD, pass);
  if (nntp_send(buff)) {
    return(NNTP_ERR_COMM);
  }
  if (nntp_receive(buff, sizeof(buff))) {
    return(NNTP_ERR_COMM);
  }
  switch (atoi(buff)) {
  case NNTP_AUTHCOMP_RES:
  case NNTP_AUTHDONE_RES:
    break;
  case NNTP_AUTHERR_RES:
    print_warning("%s", buff);
    return(NNTP_ERR_AUTH);
  default:
    print_fatal("Unexpected NNTP reply in PASS command.");
    print_warning("%s", buff);
    return(NNTP_ERR_PROT);
  }
  first = 0;
  return(NNTP_OK);			/* Authentication accepted */
#else	/* !NNTP_AUTH */
  return(NNTP_OK);
#endif	/* !NNTP_AUTH */
}

/*
 * ニュースグループリスト取得
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_list(void)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_list()
#endif	/* !__STDC__ */
{
  char		buff[BUFF_SIZE];
  char		*ptr;
  int		first_line;
  NNTP_ERR_CODE	status;
#ifdef	NSPL
  FILE		*fp;
#endif	/* NSPL */
  register int	i;

#ifdef	NG_ALLOC
  nntp_free();
#endif	/* NG_ALLOC */
  first_line = 1;
  too_many = 0;
  group_number = 0;
  status = NNTP_ERR_COMM;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    sprintf(buff, "%s%c%s", news_lib, SLASH_CHAR, NEWS_ACTIVE_FILE);
    fp = fopen(buff, "r");
    if (fp == (FILE*)NULL) {
      print_fatal("Can't open news active list.");
      break;
    }
    status = NNTP_OK;
    while (fgets(buff, sizeof(buff), fp)) {
      ptr = buff;		/* グループ情報取得	*/
      if ((status = add_group(ptr)) != NNTP_OK) {
	break;
      }
    }
    fclose(fp);
#endif	/* NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
    if (nntp_send(NNTP_LIST_CMD)) {
      break;
    }
    status = NNTP_OK;
    while (1) {
      if (nntp_receive(buff, sizeof(buff))) {
	status = NNTP_ERR_COMM;
	break;
      }
      ptr = buff;
      if (first_line) {		/* ステータス取得	*/
	if (atoi(ptr) != NNTP_LIST_RES) {
	  print_fatal("Unexpected NNTP reply in LIST command.");
	  print_fatal("%s", ptr);
	  status = NNTP_ERR_PROT;
	  break;
	}
	first_line = 0;
      } else {			/* グループ情報取得	*/
	if ((buff[0] == TCP_END_MARK) && (!buff[1])) {
	  break;
	}
	if (add_group(ptr)) {
	  status = NNTP_ERR_PROG;
	}
      }
    }
#endif	/* NNTP */
    break;
  default:
    break;
  }
  if (status != NNTP_OK) {
    group_number = 0;
#ifdef	NG_ALLOC
    nntp_free();
#endif	/* NG_ALLOC */
  } else {
#ifdef	NG_SORT
    for (i = 0; i < group_number; i++) {	/* せこい処理開始	*/
      ptr = news_group[i].group_name;
      while (*ptr) {
	if (*ptr == NEWS_GROUP_SEPARATER) {
	  *ptr++ = ' ';
	} else {
	  ptr++;
	}
      }
    }
#ifdef	NG_ALLOC
#ifdef	__STDC__
    qsort(news_group, group_number, sizeof(struct news_group),
	  (int (*)(const void*, const void*))compare_group);
#else	/* !__STDC__ */
    qsort(news_group, group_number, sizeof(struct news_group), compare_group);
#endif	/* !__STDC__ */
#else	/* !NG_ALLOC */
#ifdef	__STDC__
    qsort(news_group, group_number, sizeof(struct news_group),
	  (int (*)(const void*, const void*))strcmp);
#else	/* !__STDC__ */
    qsort(news_group, group_number, sizeof(struct news_group), strcmp);
#endif	/* !__STDC__ */
#endif	/* !NG_ALLOC */
    for (i = 0; i < group_number; i++) {	/* せこい処理の後始末	*/
      ptr = news_group[i].group_name;
      while (*ptr) {
	if (*ptr == ' ') {
	  *ptr++ = NEWS_GROUP_SEPARATER;
	} else {
	  ptr++;
	}
      }

      /*
       * ニュースグループが重複していた場合のエラー処理
       */

      if (i > 0) {
	if (!strcmp(news_group[i - 1].group_name, news_group[i].group_name)) {
	  print_warning("Duplicate news group %s.", news_group[i].group_name);
	  if ((group_number - i) > 0) {
	    memcpy(&news_group[i - 1], &news_group[i],
		   sizeof(struct news_group) * (group_number - i));
	  }
	  group_number--;
	  i--;
	}
      }
    }
#else	/* !NG_SORT */

    /*
     * ニュースグループが重複していた場合のエラー処理
     */

    for (i = 0; i < group_number - 1; i++) {
      if (!strcmp(news_group[i].group_name, news_group[i + 1].group_name)) {
	print_warning("Duplicate news group %s.", news_group[i].group_name);
	group_number--;
	if ((group_number - i) > 0) {
	  memcpy(&news_group[i], &news_group[i + 1],
		 sizeof(struct news_group) * (group_number - i));
	}
	i--;
      }
    }
#endif	/* NG_SORT */
  }
  return(status);
}

/*
 * ニュースグループリスト取得
 */

#ifdef	NG_ALLOC
#ifdef	__STDC__
NNTP_ERR_CODE	nntp_free(void)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_free()
#endif	/* !__STDC__ */
{
  if (news_group != (struct news_group*)NULL) {
    large_free(news_group);
  }
  if (name_pool != (char*)NULL) {
    large_free(name_pool);
  }
  news_group = (struct news_group*)NULL;
  name_pool = (char*)NULL;
  news_alloc_number = 0;
  name_alloc_size = name_used_size = 0;
  return(NNTP_OK);
}
#endif	/* NG_ALLOC */

/*
 * ニュースグループ登録
 */

#ifdef	__STDC__
static int	add_group(char *ptr)
#else	/* !__STDC__ */
static int	add_group(ptr)
     char	*ptr;
#endif	/* !__STDC__ */
{
  struct news_group	*alloc_ptr;
  register unsigned int	length;
  register int		i;
  char			name_buff[MAX_GROUP_NAME];
  char			*ptr2;

  /*	ニュースグループ名	*/

  length = 0;
  ptr2 = name_buff;
  while (*ptr > ' ') {
    if (length < (sizeof(name_buff) - 1)) {
      *ptr2++ = *ptr++;
      length++;
    } else {
      *ptr2 = '\0';
      print_fatal("Too long news group name %s.", name_buff);
      return(NNTP_OK);
    }
  }
  *ptr2 = '\0';
  length++;

  ptr2 = ignore_groups;
  if (ignore_mode) {
    i = 0;
    while (*ptr2) {
      i = strlen(ptr2);
      if (!strncmp(name_buff, ptr2, i)) {
	i = 0;
	break;
      }
      ptr2 += (i + 1);
    }
    if (i) {
      return(NNTP_OK);
    }
  } else {
    while (*ptr2) {
      i = strlen(ptr2);
      if (!strncmp(name_buff, ptr2, i)) {
	return(NNTP_OK);
      }
      ptr2 += (i + 1);
    }
  }

#ifdef	NG_ALLOC
  /*
   * この部分は NNTP Library の中で一番汚い部分です。
   * 真剣に解読しない方が身のためでしょう。
   */

  if (group_number >= news_alloc_number) {
    if (news_group != (struct news_group*)NULL) {
      alloc_ptr = (struct news_group*)
	large_realloc(news_group, (long)sizeof(struct news_group) *
		      (long)(news_alloc_number + NG_ALLOC_COUNT));
      if (alloc_ptr == (struct news_group*)NULL) {
	print_fatal("Can't allocate memory for news struct.");
	return(NNTP_ERR_MEM);
      }
      news_group = alloc_ptr;
    } else {
      news_group = (struct news_group*)
	large_malloc((long)sizeof(struct news_group) *
		     (long)(news_alloc_number + NG_ALLOC_COUNT));
      if (news_group == (struct news_group*)NULL) {
	print_fatal("Can't allocate memory for news struct.");
	return(NNTP_ERR_MEM);
      }
    }
    news_alloc_number += NG_ALLOC_COUNT;
  }
  alloc_ptr = &news_group[group_number];
  if (length >= (name_alloc_size - name_used_size)) {
    if (name_pool != (char*)NULL) {
      ptr2 = (char*)large_realloc(name_pool,
				  (long)name_alloc_size +
				  (long)NAME_ALLOC_SIZE);
      if (ptr2 == (char*)NULL) {
	print_fatal("Can't allocate memory for name pool.");
	return(NNTP_ERR_MEM);
      } else {
	if (name_pool != ptr2) {
	  for (i = 0; i < group_number; i++) {
#if	defined(MSDOS) && !defined(X68K) && !defined(__GO32__)
	    news_group[i].group_name = ptr2 +
	      ((unsigned)news_group[i].group_name - (unsigned)name_pool);
#else	/* (!MSDOS || X68K || __GO32__) */
	    news_group[i].group_name += (ptr2 - name_pool);
#endif	/* (!MSDOS || X68K || __GO32__) */
	  }
	}
	name_pool = ptr2;
      }
    } else {
      name_pool = (char*)large_malloc((long)name_alloc_size +
				      (long)NAME_ALLOC_SIZE);
      if (name_pool == (char*)NULL) {
	print_fatal("Can't allocate memory for name pool.");
	return(NNTP_ERR_MEM);
      }
    }
    name_alloc_size += NAME_ALLOC_SIZE;
  }
  alloc_ptr->group_name = &name_pool[name_used_size];
  strcpy(alloc_ptr->group_name, name_buff);
  name_used_size += length;
#else	/* !NG_ALLOC */
  if (group_number >= MAX_NEWS_GROUP) {
    if (!too_many) {
      too_many = 1;
      print_fatal("Too many news group.");
    }
    return(NNTP_ERR_MEM);
  }
  alloc_ptr = &news_group[group_number];
  strcpy(alloc_ptr->group_name, name_buff);
#endif	/* !NG_ALLOC */
  
  /*	最大保管記事番号	*/
  
  if ((alloc_ptr->max_article = atoi2(&ptr)) <= 0) {
    alloc_ptr->max_article = 0;
    alloc_ptr->min_article = 0;
    atoi2(&ptr);		/* 最小保管記事番号を捨てる */
  } else {
    
    /*	最小保管記事番号	*/
    
    if ((alloc_ptr->min_article = atoi2(&ptr)) < 0) {
      alloc_ptr->max_article = 0;
      alloc_ptr->min_article = 0;
    } else if (alloc_ptr->min_article == 0) {
      alloc_ptr->min_article = 1;
    }
  }
  
  /*	モード (y/n/m)		*/
  
  while ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\n')) {
    ptr++;
  }
  switch (*ptr) {
  case 'y':	/* ポスト可能	*/
  case 'Y':
    alloc_ptr->group_mode = (short)POST_ENABLE;
    break;
  case 'n':	/* ポスト不可	*/
  case 'N':
  case 'x':	/* ポスト不可	*/
  case 'X':
  case 'j':	/* Junk		*/
  case 'J':
  case '=':	/* エイリアス	*/
    alloc_ptr->group_mode = (short)POST_DISABLE;
    break;
  case 'm':	/* モデレート	*/
  case 'M':
    alloc_ptr->group_mode = (short)POST_MODERATED;
    break;
  default:
    print_warning("Unexpected news group mode %s.", alloc_ptr->group_name);
    alloc_ptr->group_mode = (short)POST_UNKNOWN;
    break;
  }
  group_number++;
  return(NNTP_OK);
}

#ifdef	NG_ALLOC
/*
 * ニュースグループ名比較
 */

#ifdef	__STDC__
static int	compare_group(struct news_group *ptr1, struct news_group *ptr2)
#else	/* !__STDC__ */
static int	compare_group(ptr1, ptr2)
     struct news_group	*ptr1;
     struct news_group	*ptr2;
#endif	/* !__STDC__ */
{
  return(strcmp(ptr1->group_name, ptr2->group_name));
}
#endif	/* NG_ALLOC */

/*
 * ニュースグループ選択
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_group(char *group_name)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_group(group_name)
     char	*group_name;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
#ifdef	NNTP
  char		buff2[BUFF_SIZE];
  int		status;
  int		estimate_number;
  int		first_number;
  int		last_number;
#endif	/* NNTP */
#ifdef	NSPL
  struct stat 	stat_buff;
  char		*ptr;
#endif	/* NSPL */

  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    strcpy(buff1, group_name);
    ptr = buff1;
    while (*ptr) {
      if (*ptr == NEWS_GROUP_SEPARATER) {
	*ptr = SLASH_CHAR;
      }
      ptr++;
    }
    if (buff1[0] == NEWS_GROUP_SEPARATER) {
      sprintf(current_group, "%s%s", news_spool, buff1);
    } else {
      sprintf(current_group, "%s%c%s", news_spool, SLASH_CHAR, buff1);
    }
#ifdef	XOVER
    xover_mode = 0;
#endif	/* XOVER */
    if (stat(current_group, &stat_buff)) {
      print_fatal("Invalid news group \"%s\".", group_name);
      current_group[0] = '\0';
      return(NNTP_ERR_NONE);
    }
#else	/* !NSPL */
    return(NNTP_ERR_RSV);
#endif	/* !NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
    sprintf(buff1, "%s %s", NNTP_GROUP_CMD, group_name);
    if (nntp_send(buff1)) {
      return(NNTP_ERR_COMM);
    }
    if (nntp_receive(buff1, sizeof(buff1))) {	/* ステータス取得	*/
      return(NNTP_ERR_COMM);
    }
    if (sscanf(buff1, "%d %d %d %d %s", &status, &estimate_number,
	       &first_number, &last_number, buff2) != 5) {
      print_fatal("Invalid news group \"%s\".", group_name);
      return(NNTP_ERR_PROT);
    }
    if (status != NNTP_GROUP_RES) {
      print_fatal("Unexpected NNTP reply in GROUP command.");
      print_fatal("%s", buff1);
      return(NNTP_ERR_PROT);
    }
#ifdef	XOVER
    if (xover_mode) {
      xover_mode = 0;
      if (!nntp_send(NNTP_XOVER_CMD)) {
	if (!nntp_receive(buff1, sizeof(buff1))) {
	  switch (atoi(buff1)) {
	  case NNTP_XOVER_RES:
	    xover_mode = 1;
	    while (!nntp_receive(buff1, sizeof(buff1))) {
	      if ((buff1[0] == TCP_END_MARK) && (!buff1[1])) {
		break;
	      }
	    }
	    break;
	  case NNTP_NO_CMD_RES:	/* XOVER コマンド未サポート */
	    break;
	  default:
	    print_fatal("Unexpected NNTP reply in XOVER command.");
	    print_fatal("%s", buff1);
	    break;
	  }
	}
      }
    }
#endif	/* XOVER */
#else	/* !NNTP */
    return(NNTP_ERR_RSV);
#endif	/* !NNTP */
    break;
  default:
    return(NNTP_ERR_PROG);
    break;
  }
  return(NNTP_OK);
}

/*
 * ニュース記事取得(番号指定)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_article(int article_number, FILE *fp1)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_article(article_number, fp1)
     int	article_number;
     FILE	*fp1;
#endif	/* !__STDC__ */
{
  char	buff[NUMBER_BUFF];

  sprintf(buff, "%d", article_number);
  return(nntp_article2((CASTPTR)buff, fp1));
}

/*
 * ニュース記事取得(Message-ID 指定)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_article2(CASTPTR message_id, FILE *fp1)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_article2(message_id, fp1)
     CASTPTR	message_id;
     FILE	*fp1;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  int		first_line;
  NNTP_ERR_CODE	status;
#ifdef	NSPL
  int		article_number;
  FILE		*fp2;
#endif	/* NSPL */

  first_line = 1;
  status = NNTP_ERR_COMM;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    if (!current_group[0]) {
      print_fatal("News group not selected.");
      status = NNTP_ERR_PROG;
      break;
    }
    if (atoi((char*)message_id) > 0) {
      sprintf(buff1, "%s%c%s", current_group, SLASH_CHAR,
	      (char*)message_id);
    } else {
      if ((article_number = nspl_search_article((char*)message_id)) <= 0) {
	break;
      }
      sprintf(buff1, "%s%c%d", current_group, SLASH_CHAR,
	      (int)article_number);
    }
    fp2 = fopen(buff1, "r");
    if (fp2 == (FILE*)NULL) {
      status = NNTP_ERR_NONE;
      break;
    }
    while (fgets(buff2, sizeof(buff2), fp2)) {
      fputs(buff2, fp1);
    }
    fclose(fp2);
    status = NNTP_OK;
#else	/* !NSPL */
    status = NNTP_ERR_RSV;
#endif	/* !NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
    sprintf(buff1, "%s %s", NNTP_ARTICLE_CMD, (char*)message_id);
    if (nntp_send(buff1)) {
      break;
    }
    status = NNTP_OK;
    while (1) {
      if (nntp_receive(buff2, sizeof(buff2))) {
	return(NNTP_ERR_COMM);
      }
      if (first_line) {		/* ステータス取得	*/
#ifdef	notdef
	if (atoi(buff2) != NNTP_ARTICLE_RES) {
	  
	  /*
	   * キャンセルされてないのかチェック
	   */
	  
	  if (atoi(buff2) != NNTP_NO_NUM_RES) {
	    print_fatal("Unexpected NNTP reply in ARTICLE command.");
	    print_fatal("%s", buff2);
	    status = NNTP_ERR_PROT;
	    break;
	  } else {
	    status = NNTP_ERR_NONE;
	    break;
	  }
	}
#else	/* !notdef */
	switch (atoi(buff2)) {
	case NNTP_ARTICLE_RES:
	  break;
	case NNTP_NO_CUR_RES:
	case NNTP_NO_NUM_RES:
	case NNTP_NO_FOUND_RES:
	  status = NNTP_ERR_NONE;
	  break;
	default:
	  print_fatal("Unexpected NNTP reply in ARTICLE command.");
	  print_fatal("%s", buff2);
	  status = NNTP_ERR_PROT;
	  break;
	}
	if (status != NNTP_OK) {
	  break;
	}
#endif	/* !notdef */
	first_line = 0;
      } else {
	if (buff2[0] == TCP_END_MARK) {
	  if (buff2[1]) {
	    fprintf(fp1, "%s\n", &buff2[1]);
	  } else {
	    break;
	  }
	} else {
	  fprintf(fp1, "%s\n", buff2);
	}
      }
    }
#else	/* !NNTP */
    status = NNTP_ERR_RSV;
#endif	/* !NNTP */
    break;
  default:
    break;
  }
  return(status);
}

/*
 * ニュースヘッダ取得(番号指定)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_head(int article_number, FILE *fp1)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_head(article_number, fp1)
     int	article_number;
     FILE	*fp1;
#endif	/* !__STDC__ */
{
  char	buff[NUMBER_BUFF];

  sprintf(buff, "%d", article_number);
  return(nntp_head2(buff, fp1));
}

/*
 * ニュースヘッダ取得(Message-ID 指定)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_head2(char *message_id, FILE *fp1)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_head2(message_id, fp1)
     char	*message_id;
     FILE	*fp1;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  int		first_line;
  NNTP_ERR_CODE	status;
#ifdef	NSPL
  int		article_number;
  FILE		*fp2;
#endif	/* NSPL */

  status = NNTP_ERR_COMM;
  first_line = 1;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    if (!current_group[0]) {
      print_fatal("News group not selected.");
      break;
    }
    if (atoi(message_id) > 0) {
      sprintf(buff1, "%s%c%s", current_group, SLASH_CHAR, message_id);
    } else {
      if ((article_number = nspl_search_article(message_id)) <= 0) {
	break;
      }
      sprintf(buff1, "%s%c%d", current_group, SLASH_CHAR, article_number);
    }
    fp2 = fopen(buff1, "r");
    if (fp2 == (FILE*)NULL) {
      break;
    }
    status = NNTP_OK;
    while (fgets(buff2, sizeof(buff2), fp2)) {
      if ((!buff2[0]) || (buff2[0] == '\n')) {
	break;
      }
      fputs(buff2, fp1);
    }
    fclose(fp2);
#else	/* !NSPL */
    status = NNTP_ERR_RSV;
#endif	/* !NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
    sprintf(buff1, "%s %s", NNTP_HEAD_CMD, message_id);
    if (nntp_send(buff1)) {
      break;
    }
    status = NNTP_OK;
    while (1) {
      if (nntp_receive(buff2, sizeof(buff2))) {
	status = NNTP_ERR_COMM;
	break;
      }
      if (first_line) {		/* ステータス取得	*/
#ifdef	notdef
	if (atoi(buff2) != NNTP_HEAD_RES) {
	  
	  /*
	   * キャンセルされてないのかチェック
	   */
	  
	  if (atoi(buff2) != NNTP_NO_NUM_RES) {
	    print_fatal("Unexpected NNTP reply in HEAD command.");
	    print_fatal("%s", buff2);
	    status = NNTP_ERR_COMM;
	    break;
	  } else {
	    status = NNTP_ERR_NONE;
	    break;
	  }
	}
#else	/* !notdef */
	switch (atoi(buff2)) {
	case NNTP_HEAD_RES:
	  break;
	case NNTP_NO_CUR_RES:
	case NNTP_NO_NUM_RES:
	case NNTP_NO_FOUND_RES:
	  status = NNTP_ERR_NONE;
	  break;
	default:
	  print_fatal("Unexpected NNTP reply in HEAD command.");
	  print_fatal("%s", buff2);
	  status = NNTP_ERR_PROT;
	  break;
	}
	if (status != NNTP_OK) {
	  break;
	}
#endif	/* !notdef */
	first_line = 0;
      } else {
	if (buff2[0] == TCP_END_MARK) {
	  if (buff2[1]) {
	    fprintf(fp1, "%s\n", &buff2[1]);
	  } else {
	    break;
	  }
	} else {
	  fprintf(fp1, "%s\n", buff2);
	}
      }
    }
#else	/* !NNTP */
    status = NNTP_ERR_RSV;
#endif	/* !NNTP */
    break;
  default:
    break;
  }
  return(status);
}

/*
 * ニュースフィールド取得(記事番号指定)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_copy_fields(int article_number, struct cpy_hdr *hdr_ptr,
				 int number, int mask)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_copy_fields(article_number, hdr_ptr, number, mask)
     int		article_number;
     struct cpy_hdr	*hdr_ptr;
     int		number;
     int		mask;
#endif	/* !__STDC__ */
{
  char	buff[NUMBER_BUFF];

  sprintf(buff, "%d", article_number);
  return(nntp_copy_fields2(buff, hdr_ptr, number, mask));
}

/*
 * ニュースフィールド取得(Message-ID 指定)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_copy_fields2(char *message_id, struct cpy_hdr *hdr_ptr,
				  int number, int mask)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_copy_fields2(message_id, hdr_ptr, number, mask)
     char		*message_id;
     struct cpy_hdr	*hdr_ptr;
     int		number;
     int		mask;
#endif	/* !__STDC__ */
{
  char		buff[BUFF_SIZE];
  NNTP_ERR_CODE	status;
#ifdef	NSPL
  int		article_number;
  FILE		*fp;
#endif	/* NSPL */

  if (mask & CF_CLR_MASK) {
    copy_fields((FILE*)NULL, hdr_ptr, number, CF_CLR_MASK);
    mask &= !CF_CLR_MASK;
  }
  status = NNTP_ERR_COMM;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    if (!current_group[0]) {
      print_fatal("News group not selected.");
      status = NNTP_ERR_PROG;
      break;
    }
    if (atoi(message_id) > 0) {
      sprintf(buff, "%s%c%s", current_group, SLASH_CHAR, message_id);
    } else {
      if ((article_number = nspl_search_article(message_id)) <= 0) {
	break;
      }
      sprintf(buff, "%s%c%d", current_group, SLASH_CHAR, article_number);
    }
    fp = fopen(buff, "r");
    if (fp != (FILE*)NULL) {
      copy_fields(fp, hdr_ptr, number, CF_GET_MASK | CF_SPC_MASK);
      fclose(fp);
      status = NNTP_OK;
    } else {
      status = NNTP_ERR_NONE;
    }
#else	/* !NSPL */
    status = NNTP_ERR_RSV;
#endif	/* !NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
    sprintf(buff, "%s %s", NNTP_HEAD_CMD, message_id);
    if (nntp_send(buff)) {
      break;
    }
    if (nntp_receive(buff, sizeof(buff))) {
      break;
    }
    switch (atoi(buff)) {
    case NNTP_HEAD_RES:
      copy_fields((FILE*)NULL, hdr_ptr, number,
		  CF_GET_MASK | CF_SPC_MASK | CF_HED_MASK);
      status = NNTP_OK;
      break;
    case NNTP_NO_NUM_RES:	/* キャンセル	*/
      status = NNTP_ERR_NONE;
      break;
    default:
      print_fatal("Unexpected NNTP reply in HEAD command.");
      print_fatal("%s", buff);
      break;
    }
#else	/* !NNTP */
    status = NNTP_ERR_RSV;
#endif	/* !NNTP */
    break;
  default:
    break;
  }
  return(status);
}

#ifndef	SMALL
/*
 * ニュース本文取得(記事番号指定)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_body(int article_number, FILE *fp)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_body(article_number, fp)
     int	article_number;
     FILE	*fp;
#endif	/* !__STDC__ */
{
  char	buff[NUMBER_BUFF];

  sprintf(buff, "%d", article_number);
  return(nntp_body2(buff, fp));
}

/*
 * ニュース本文取得(Message-ID 指定)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_body2(char *message_id, FILE *fp1)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_body2(message_id, fp1)
     char	*message_id;
     FILE	*fp1;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  int		first_line;
  NNTP_ERR_CODE	status;
#ifdef	NSPL
  int		article_number;
  FILE		*fp2;
#endif	/* NSPL */

  first_line = 1;
  status = NNTP_ERR_COMM;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    if (!current_group[0]) {
      print_fatal("News group not selected.");
      status = NNTP_ERR_PROG;
      break;
    }
    if (atoi(message_id) > 0) {
      sprintf(buff1, "%s%c%s", current_group, SLASH_CHAR, message_id);
    } else {
      if ((article_number = nspl_search_article(message_id)) <= 0) {
	break;
      }
      sprintf(buff1, "%s%c%d", current_group, SLASH_CHAR, article_number);
    }
    fp2 = fopen(buff1, "r");
    if (fp2 == (FILE*)NULL) {
      break;
    }
    status = NNTP_OK;
    while (fgets(buff2, sizeof(buff2), fp2)) {
      if (first_line) {
	if ((!buff2[0]) || (buff2[0] == '\n')) {
	  first_line = 0;
	}
      } else {
	fputs(buff2, fp1);
      }
    }
    fclose(fp2);
#else	/* !NSPL */
    status = NNTP_ERR_RSV;
#endif	/* !NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
    sprintf(buff1, "%s %s", NNTP_BODY_CMD, message_id);
    if (nntp_send(buff1)) {
      break;
    }
    status = NNTP_OK;
    while (1) {
      if (nntp_receive(buff2, sizeof(buff2))) {
	status = NNTP_ERR_COMM;
	break;
      }
      if (first_line) {		/* ステータス取得	*/
#ifdef	notdef
	if (atoi(buff2) != NNTP_BODY_RES) {
	  
	  /*	キャンセルされてないのかチェック	*/
	  
	  if (atoi(buff2) != NNTP_NO_NUM_RES) {
	    print_fatal("Unexpected NNTP reply in BODY command.");
	    print_fatal("%s", buff2);
	    status = NNTP_ERR_PROT;
	    break;
	  } else {
	    status = NNTP_ERR_NONE;
	    break;
	  }
	}
#else	/* !notdef */
	switch (atoi(buff2)) {
	case NNTP_BODY_RES:
	  break;
	case NNTP_NO_CUR_RES:
	case NNTP_NO_NUM_RES:
	case NNTP_NO_FOUND_RES:
	  status = NNTP_ERR_NONE;
	  break;
	default:
	  print_fatal("Unexpected NNTP reply in BODY command.");
	  print_fatal("%s", buff2);
	  status = NNTP_ERR_PROT;
	  break;
	}
	if (status != NNTP_OK) {
	  break;
	}
#endif	/* !notdef */
	first_line = 0;
      } else {
	if (buff2[0] == TCP_END_MARK) {
	  if (buff2[1]) {
	    fprintf(fp1, "%s\n", &buff2[1]);
	  } else {
	    break;
	  }
	} else {
	  fprintf(fp1, "%s\n", buff2);
	}
      }
    }
#else	/* !NNTP */
    status = NNTP_ERR_RSV;
#endif	/* !NNTP */
    break;
  default:
    break;
  }
  return(status);
}
#endif	/* !SMALL */

/*
 * ニュース記事投稿
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_post(char *file_name, char *domain_name)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_post(file_name, domain_name)
     char	*file_name;
     char	*domain_name;
#endif	/* !__STDC__ */
{
  FILE		*fp1;
#ifdef	NSPL
  FILE		*fp2;
#endif	/* NSPL */
  char		buff1[BUFF_SIZE];
#ifdef	NNTP
  char		buff2[BUFF_SIZE];
  char		*ptr;
#endif	/* NNTP */
  NNTP_ERR_CODE	status;

  if (!post_enable) {
    return(NNTP_ERR_RSV);
  }
  fp1 = fopen(file_name, "r");
  if (fp1 == (FILE*)NULL) {
    return(NNTP_ERR_PROG);
  }
  status = NNTP_ERR_COMM;
  if ((!nntp_connect_mode) || force_use_inews) {	/* NSPL 接続	*/
#ifdef	NSPL
#ifdef	SIGPIPE
    signal(SIGPIPE, post_pipe_error);
#endif	/* SIGPIPE */
#if	defined(MSDOS) || defined(OS2)
    strcpy(buff1, post_program);
#else	/* !(MSDOS || OS2) */
    sprintf(buff1, "%s > /dev/null 2>&1", post_program);
#endif	/* !(MSDOS || OS2) */
    fp2 = popen(buff1, "w");
    if (fp2 != (FILE*)NULL) {
      status = NNTP_OK;
      while (fgets(buff1, sizeof(buff1), fp1)) {
	if (fputs(buff1, fp2) == EOF) {
	  status = NNTP_ERR_COMM;
	  break;
	}
      }
      if (pclose(fp2)) {
	status = NNTP_ERR_COMM;
      }
    } else {
      print_fatal("Can't execute post command.");
    }
#ifdef	SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif	/* SIGPIPE */
#else	/* !NSPL */
    status = NNTP_ERR_RSV;
#endif	/* !NSPL */
  } else if (nntp_connect_mode == 1) {			/* NNTP 接続	*/
#ifdef	NNTP
    if (!nntp_send(NNTP_POST_CMD)) {
      if (!nntp_receive(buff1, sizeof(buff1))) {
	if (atoi(buff1) == NNTP_POST_RES1) {
	  while (fgets(buff1, sizeof(buff1), fp1)) {
	    if ((!buff1[0]) || (buff1[0] == '\n')) {
	      break;
	    }
	    ptr = buff1;
	    while (*ptr) {
	      if (*ptr == '\n') {
		*ptr = '\0';
		break;
	      }
	      ptr++;
	    }
	    if (buff1[0] == TCP_END_MARK) {
	      sprintf(buff2, "%c%s", TCP_END_MARK, buff1);
	      nntp_send(buff2);
	    } else {
	      nntp_send(buff1);
	    }
	  }
	  nntp_send("");
	  while (fgets(buff1, sizeof(buff1), fp1)) {
	    ptr = buff1;
	    while (*ptr) {
	      if (*ptr == '\n') {
		*ptr = '\0';
		break;
	      }
	      ptr++;
	    }
	    if (buff1[0] == TCP_END_MARK) {
	      sprintf(buff2, "%c%s", TCP_END_MARK, buff1);
	      nntp_send(buff2);
	    } else {
	      nntp_send(buff1);
	    }
	  }
	  buff1[0] = TCP_END_MARK;
	  buff1[1] = '\0';
	  nntp_send(buff1);
	  if (!nntp_receive(buff1, sizeof(buff1))) {
	    if (atoi(buff1) == NNTP_POST_RES2) {
	      status = NNTP_OK;
	    }
	  }
	}
      }
    }
#endif	/* NNTP */
  }
  fclose(fp1);
  return(status);
}

/*
 * NNTP 接続確認
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_noop(void)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_noop()
#endif	/* !__STDC__ */
{
#ifdef	USE_NNTP_DATE
  char	buff[BUFF_SIZE];
#endif	/* USE_NNTP_DATE */

  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
#ifdef	USE_NNTP_DATE
    if (nntp_send(NNTP_DATE_CMD)) {
      return(NNTP_ERR_COMM);
    }
    if (nntp_receive(buff, sizeof(buff))) {
      return(NNTP_ERR_COMM);
    }
#ifdef	notdef
    if (atoi(buff) != NNTP_DATE_RES) {
      print_fatal("Unexpected NNTP reply in DATE command.");
      print_fatal("%s", buff);
      return(NNTP_ERR_PROT);
    }
#endif	/* notdef */
    return(NNTP_OK);
#else	/* !USE_NNTP_DATE */
    return(nntp_send(""));
#endif	/* !USE_NNTP_DATE */
#endif	/* NNTP */
    break;
  default:
    break;
  }
  return(NNTP_OK);
}

#ifndef	SMALL
/*
 * NNTP 憲章取得
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_description(char *group_name, char *description)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_description(group_name, description)
     char	*group_name;
     char	*description;
#endif	/* !__STDC__ */
{
  char		buff[BUFF_SIZE];
  char		*ptr;
  int		first_line;
  NNTP_ERR_CODE	status;
#ifdef	NSPL
  FILE		*fp;
#endif	/* NSPL */

  first_line = 1;
  *description = '\0';
  status = NNTP_ERR_COMM;
  switch (nntp_connect_mode) {
  case 0:						/* NSPL 接続	*/
#ifdef	NSPL
    sprintf(buff, "%s%c%s", news_lib, SLASH_CHAR, NEWS_DESCRIPTION_FILE);
    fp = fopen(buff, "r");
    if (fp == (FILE*)NULL) {
      print_fatal("Can't open news description list.");
      break;
    }
    while (fgets(buff, sizeof(buff), fp)) {
      if (!strncmp(buff, group_name, strlen(group_name))) {
	ptr = &buff[strlen(group_name)];
	if ((*ptr == ' ') || (*ptr == '\t')) {
	  while ((*ptr == ' ') || (*ptr == '\t')) {
	    ptr++;
	  }
	  while (*ptr) {
	    if ((*((unsigned char*)ptr) >= ' ') || (*ptr == '\t') ||
		(*ptr == '\033')) {
	      *description++ = *ptr++;
	    } else {
	      break;
	    }
	  }
	  *description = '\0';
	  status = NNTP_OK;
	  break;
	}
      }
    }
    fclose(fp);
#else	/* !NSPL */
    status = NNTP_ERR_RSV;
#endif	/* !NSPL */
    break;
  case 1:						/* NNTP 接続	*/
#ifdef	NNTP
    if (nntp_send(NNTP_DESCRIPTION_CMD)) {
      break;
    }
    while (1) {
      if (nntp_receive(buff, sizeof(buff))) {
	break;
      }
      ptr = buff;
      if (first_line) {		/* ステータス取得	*/
	if (atoi(ptr) != NNTP_LIST_RES) {
	  print_fatal("Unexpected NNTP reply in LIST command.");
	  print_fatal("%s", ptr);
	  break;
	}
	first_line = 0;
      } else {			/* グループ情報取得	*/
	if ((buff[0] == TCP_END_MARK) && (!buff[1])) {
	  break;
	}
	if (!strncmp(buff, group_name, strlen(group_name))) {
	  ptr = &buff[strlen(group_name)];
	  if (((*ptr == ' ') || (*ptr == '\t')) && (status != NNTP_OK)) {
	    while ((*ptr == ' ') || (*ptr == '\t')) {
	      ptr++;
	    }
	    while (*ptr) {
	      if ((*((unsigned char*)ptr) >= ' ') || (*ptr == '\t') ||
		  (*ptr == '\033')) {
		*description++ = *ptr++;
	      } else {
		break;
	      }
	    }
	    *description = '\0';
	    status = NNTP_OK;
	  }
	}
      }
    }
#endif	/* NNTP */
    break;
  default:
    break;
  }
  return(status);
}
#endif	/* !SMALL */

/*
 * 文字列を数値に変換
 */

#ifdef	__STDC__
static int	atoi2(char **pptr)
#else	/* !__STDC__ */
static int	atoi2(pptr)
     char	**pptr;
#endif	/* !__STDC__ */
{
  int	n;
  char	*ptr;
  
  ptr = *pptr;
  
  while ((*ptr == ' ') || (*ptr == '\t')) {
    ptr++;
  }
  if (!isdigit(*ptr)) {
    return(-1);
  }
  n = atoi(ptr);
  while (isdigit(*ptr)) {
    ptr++;
  }
  while ((*ptr == ' ') || (*ptr == '\t')) {
    ptr++;
  }
  *pptr = ptr;
  return(n);
}

#ifdef	NSPL
/*
 * 記事検索
 *
 * 戻り値:負数:エラー,正数:記事番号
 */

#ifdef	__STDC__
static int	nspl_search_article(char *message_id)
#else	/* !__STDC__ */
static int	nspl_search_article(message_id)
     char	*message_id;
#endif	/* !__STDC__ */
{
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		*ptr1, *ptr2;
  FILE		*fp;
#ifdef	HAVE_FILES
  FILES_STRUCT	dp;
#else	/* !HAVE_FILES */
  DIR_PTR	*dp;
  DIR		*dir_ptr;
#endif	/* !HAVE_FILES */
  long		article_number;
  register int	match;
  register int	number;

  if (!current_group[0]) {
    print_fatal("News group not selected.");
    return(-1);
  }
  match = 0;
  article_number = -1;
  sprintf(buff2, "%s %s", MESSAGE_FIELD, message_id);
#ifdef	HAVE_FILES
  sprintf(buff1, "%s%c*.*", current_group, SLASH_CHAR);
  if (!dos_files(buff1, &dp, FILE_ATTR)) {
    do {
#ifdef	__TURBOC__
      number = atoi(dp.ff_name);
      sprintf(buff1, "%d", number);
      if (strcmp(buff1, dp.ff_name)) {
	continue;
      }
      sprintf(buff1, "%s%c%s", current_group, SLASH_CHAR, dp.ff_name);
#else	/* !__TURBOC__ */
      number = atoi(dp.name);
      sprintf(buff1, "%d", number);
      if (strcmp(buff1, dp.name)) {
	continue;
      }
      sprintf(buff1, "%s%c%s", current_group, SLASH_CHAR, dp.name);
#endif	/* !__TURBOC__ */
      fp = fopen(buff1, "r");
      if (fp == (FILE*)NULL) {
	continue;
      }
      while (fgets(buff1, sizeof(buff1), fp)) {
	if ((!buff1[0]) || (buff1[0] == '\n')) {
	  break;
	}
	ptr1 = buff1;
	ptr2 = buff2;
	match = 1;
	while (*ptr2) {
	  if ((*ptr1++ & 0xdf) != (*ptr2++ & 0xdf)) {
	    match = 0;
	    break;
	  }
	}
	if (match) {
	  break;
	}
      }
      fclose(fp);
      if (match) {
	article_number = number;
	break;
      }
    } while (!dos_nfiles(&dp));
#else	/* !HAVE_FILES */
  if ((dir_ptr = opendir(current_group)) != (DIR*)NULL) {
    while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
      number = atoi(dp->d_name);
      sprintf(buff1, "%d", number);
      if (strcmp(buff1, dp->d_name)) {
	continue;
      }
      sprintf(buff1, "%s%c%s", current_group, SLASH_CHAR, dp->d_name);
      fp = fopen(buff1, "r");
      if (fp == (FILE*)NULL) {
	continue;
      }
      while (fgets(buff1, sizeof(buff1), fp)) {
	if ((!buff1[0]) || (buff1[0] == '\n')) {
	  break;
	}
	ptr1 = buff1;
	ptr2 = buff2;
	match = 1;
	while (*ptr2) {
	  if ((*ptr1++ & 0xdf) != (*ptr2++ & 0xdf)) {
	    match = 0;
	    break;
	  }
	}
	if (match) {
	  break;
	}
      }
      fclose(fp);
      if (match) {
	article_number = number;
	break;
      }
    }
    closedir(dir_ptr);
#endif	/* !HAVE_FILES */
  }
  return(article_number);
}
#endif	/* NSPL */

/*
 * 記事数確認
 */

#ifdef	__STDC__
void	confirm_article(char *group_name, int *min_ptr, int *max_ptr)
#else	/* !__STDC__ */
void	confirm_article(group_name, min_ptr, max_ptr)
    char	*group_name;
    int		*min_ptr;
    int		*max_ptr;
#endif	/* !__STDC__ */
{
#ifdef	NSPL
  char			buff1[SMALL_BUFF];
  char			buff2[SMALL_BUFF];
  char			*ptr;
#ifdef	HAVE_FILES
  FILES_STRUCT		dp;
#else	/* !HAVE_FILES */
  DIR_PTR		*dp;
  DIR			*dir_ptr;
#endif	/* !HAVE_FILES */
  register int		number;
  register int		min_number;
  register int		max_number;

  if (nntp_connect_mode) {				/* NNTP 接続	*/
    return;
  }
  strcpy(buff2, group_name);
  ptr = buff2;
  while (*ptr) {
    if (*ptr == NEWS_GROUP_SEPARATER) {
      *ptr = SLASH_CHAR;
    }
    ptr++;
  }
  sprintf(buff1, "%s%c%s", news_spool, SLASH_CHAR, buff2);
  min_number = max_number = 0;
#ifdef	HAVE_FILES
  sprintf(ptr, "%c*.*", SLASH_CHAR);
  if (!dos_files(buff1, &dp, FILE_ATTR)) {
    do {
#ifdef	__TURBOC__
      number = atoi(dp.ff_name);
      sprintf(buff2, "%d", number);
      if (!strcmp(buff2, dp.ff_name)) {
#else	/* !__TURBOC__ */
      number = atoi(dp.name);
      sprintf(buff2, "%d", number);
      if (!strcmp(buff2, dp.name)) {
#endif	/* !__TURBOC__ */
	if ((min_number > number) || (!min_number)) {
	  min_number = number;
	}
	if (max_number < number) {
	  max_number = number;
	}
      }
    } while (!dos_nfiles(&dp));
#else	/* !HAVE_FILES */
  if ((dir_ptr = opendir(buff1)) != (DIR*)NULL) {
    while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
      number = atoi(dp->d_name);
      sprintf(buff2, "%d", number);
      if (!strcmp(buff2, dp->d_name)) {
	if ((min_number > number) || (!min_number)) {
	  min_number = number;
	}
	if (max_number < number) {
	  max_number = number;
	}
      }
    }
    closedir(dir_ptr);
#endif	/* !HAVE_FILES */
    if ((*min_ptr < min_number) && min_number) {
      print_warning("Min-number fixed in %s.", group_name);
      *min_ptr = min_number;
    }
#ifdef	notdef
    if ((*max_ptr > max_number) && max_number) {
      print_warning("Max-number fixed in %s.", group_name);
      *max_ptr = max_number;
    }
#endif	/* notdef */
  }
#endif	/* NSPL */
}

#ifdef	NSPL
/*
 * パイプエラーハンドラ
 * (Broken Pipe)
 */

#ifdef	__STDC__
static void	post_pipe_error(void)
#else	/* !__STDC__ */
static void	post_pipe_error()
#endif	/* !__STDC__ */
{
  print_fatal("inews broken pipe.");
}
#endif	/* NSPL */

#ifdef	NNTP
#ifdef	X68K
#ifdef	__STDC__
int	gethostname(char *name, int namelen)
#else	/* !__STDC__ */
int	gethostname(name, namelen)
     char	*name;
     int	namelen;
#endif	/* !__STDC__ */
{
  char	*ptr;

  *name = '\0';
  ptr = getenv("HOSTNAME");
  if (ptr == (char*)NULL) {
    ptr = getenv("HOST");
  }
  if ((ptr == (char*)NULL) || strlen(ptr) >= namelen) {
    return(-1);
  }
  strcpy(name, ptr);
  return(0);
}
#endif	/* X68K */

/*
 * INN チェック
 *
 * 戻り値:(0:INN以外,0以外:INN)
 */

#ifdef	__STDC__
static int	inn_check(char *str1)
#else	/* !__STDC__ */
static int	inn_check(str1)
     char	*str1;
#endif	/* !__STDC__ */
{
  int	length;

  length = strlen(INN_STRING);
  while (*str1) {
    if (!strncmp(str1, INN_STRING, length)) {
      return(1);
    }
    str1++;
  }
  return(0);
}

#ifdef	XOVER
/*
 * ヘッダ情報取得(XOVER)
 */

#ifdef	__STDC__
NNTP_ERR_CODE	nntp_xover(int group, int min, int max)
#else	/* !__STDC__ */
NNTP_ERR_CODE	nntp_xover(group, min, max)
     int	group;
     int	min;
     int	max;
#endif	/* !__STDC__ */
{
  char	buff[XOVER_BUFF];
  int	first;
  int	status;

  first = 1;
  if (max) {
    sprintf(buff, "%s %d-%d", NNTP_XOVER_CMD, min, max);
  } else {
    sprintf(buff, "%s %d", NNTP_XOVER_CMD, min);
  }

  if (nntp_send(buff)) {
    return(NNTP_ERR_COMM);
  }
  status = NNTP_OK;
  while (1) {
    if (nntp_receive(buff, sizeof(buff))) {
      status = NNTP_ERR_COMM;
      break;
    }
    if (first) {		/* ステータス取得	*/
      if (atoi(buff) != NNTP_XOVER_RES) {
	print_fatal("Unexpected NNTP reply in XOVER command.");
	print_fatal("%s", buff);
	status = NNTP_ERR_PROT;
	break;
      }
      first = 0;
    } else {			/* ヘッダ情報取得	*/
      if ((buff[0] == TCP_END_MARK) && (!buff[1])) {
	break;
      }
      if (xover_copy_field(group, buff)) {
	print_fatal("Unexpected xover header format.");
	status = NNTP_ERR_PROT;
      }
    }
  }
  return(status);
}
#endif	/* XOVER */
#endif	/* NNTP */
