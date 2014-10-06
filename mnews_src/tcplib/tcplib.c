/*
 *
 *  TCP/IP ライブラリ
 *
 *  Copyright 1995-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : NNTP/SMTP/POP3 library
 *  Sub system  : TCP/IP library
 *  File        : tcplib.c
 *  Version     : 0.82
 *  First Edit  : 1995-12/25
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 宅間 顯
 *
 */

#include	"compat.h"
#include	"tcplib.h"
#include	"field.h"
#if	defined(NNTP) || defined(NSPL)
#include	"nntplib.h"
#endif	/* (NNTP || NSPL) */
#ifdef	MNEWS
#include	"kanjilib.h"
#include	"mnews.h"
#endif	/* MNEWS */
#ifdef	MMH
#include	"termlib.h"
#endif	/* MMH */

#ifdef	MMH
extern char	user_name[];
#endif	/* MMH */

char		auth_file[PATH_BUFF] = "";	/* ユーザ認証ファイル	*/
int		tcp_connect_number = 0;		/* TCP/IP 接続数	*/

#ifdef	WINSOCK
static WSADATA		wsa_data;
#endif	/* WINSOCK */

/*
 * TCP/IP 初期化処理
 */

#ifdef	__STDC__
int	tcp_init(void)
#else	/* !__STDC__ */
int	tcp_init()
#endif	/* !__STDC__ */
{
#ifdef	WINSOCK
  if (WSAStartup(MAKEWORD(1,1), &wsa_data)) {
    print_fatal("Can't initialize winsock.");
    return(-1);
  }
#endif	/* WINSOCK */
  return(0);
}

/*
 * TCP/IP 終了処理
 */

#ifdef	__STDC__
int	tcp_exit(void)
#else	/* !__STDC__ */
int	tcp_exit()
#endif	/* !__STDC__ */
{
#ifdef	WINSOCK
  WSACleanup();
#endif	/* WINSOCK */
  return(0);
}

#if	defined(NNTP) || defined(SMTP) || defined(POP3)
/*
 * TCP/IP ソケットオープン
 */

#ifdef	MSDOS
#ifdef	__STDC__
int	tcp_open(char *server_name, char *service_name, char *protocol_name,
		 unsigned int port_number, int default_port,
		 int *read_fdd, int *write_fdd)
#else	/* !__STDC__ */
int	tcp_open(server_name, service_name, protocol_name, port_number,
		 default_port, read_fdd, write_fdd)
     char		*server_name;
     char		*service_name;
     char		*protocol_name;
     unsigned int	port_number;
     int		default_port;
     int		*read_fdd;
     int		*write_fdd;
#endif	/* !__STDC__ */
#else	/* !MSDOS */
#ifdef	__STDC__
int	tcp_open(char *server_name, char *service_name, char *protocol_name,
		 unsigned int port_number, int default_port,
		 TCPRD_FILE **read_fpp, int *write_fdd)
#else	/* !__STDC__ */
int	tcp_open(server_name, service_name, protocol_name, port_number,
		 default_port, read_fpp, write_fdd)
     char		*server_name;
     char		*service_name;
     char		*protocol_name;
     unsigned int	port_number;
     int		default_port;
     TCPRD_FILE		**read_fpp;
     int		*write_fdd;
#endif	/* !__STDC__ */
#endif	/* !MSDOS */
{
#ifndef	MSDOS
  TCPRD_FILE		*rfp;
#ifdef	notdef
  struct hostent	host_data;
#endif	/* notdef */
#endif	/* !MSDOS */
  struct hostent	*host_ent;
  struct sockaddr_in	sock_in;
  struct servent	*serv_ent;
  int			port;
  int			s;
  int			x;
#ifdef	h_addr
  register char		**pptr;
  unsigned char		*inet_adrs_ptr;
#ifdef	notdef
  char			*alias_list[1];
#endif	/* notdef */
#endif	/* h_addr */

#ifdef	MSDOS
  *read_fdd = -1;
#else	/* !MSDOS */
  *read_fpp = (TCPRD_FILE*)NULL;
#endif	/* !MSDOS */
  *write_fdd = -1;

  if (default_port < 0) {
    port = IPPORT_RESERVED - 1;
  } else {
    if (port_number > 0) {
      port = port_number;
    } else {
      serv_ent = getservbyname(protocol_name, "tcp");
      if (serv_ent != (struct servent*)NULL) {
	port = ntohs(serv_ent->s_port);
      } else {
	if (default_port >= 0) {
	  print_warning("Can't get \"%s\" port,use default.", protocol_name);
	  port = default_port;
	} else {
	  print_fatal("Can't get \"%s\" port.", protocol_name);
	  return(-1);
	}
      }
    }
  }
  memset(&sock_in, 0, sizeof(sock_in));
  sock_in.sin_family = AF_INET;
  if (!isdigit(*server_name)) {			/* サーバ名	*/
    host_ent = gethostbyname(server_name);
  } else {					/* IP アドレス	*/
#ifdef	MSDOS
#ifdef	WINSOCK
    if ((sock_in.sin_addr.s_addr = inet_addr(server_name)) == -1) {
      print_fatal("Illegal IP address \"%s\".", server_name);
      return(-1);
    }
    host_ent = gethostbyaddr((char*)&sock_in.sin_addr,
			     sizeof(sock_in.sin_addr), AF_INET);
#else	/* !WINSOCK */
    print_fatal("%s server can't use IP-address.", service_name);
    return(-1);
#endif	/* !WINSOCK */
#else	/* !MSDOS */
    if ((sock_in.sin_addr.s_addr = inet_addr(server_name)) == -1) {
      print_fatal("Illegal IP address \"%s\".", server_name);
      return(-1);
    }
#ifdef	notdef
    host_data.h_name      = server_name;
#ifdef	h_addr
    host_data.h_addr_list = alias_list;
#endif	/* h_addr */
    host_data.h_addr      = (char*)&sock_in.sin_addr;
    host_data.h_length    = sizeof(struct in_addr);
    host_data.h_addrtype  = AF_INET;
    host_data.h_aliases   = 0;
    host_ent              = &host_data;
#else	/* !notdef */
    host_ent = gethostbyaddr((char*)&sock_in.sin_addr,
			     sizeof(sock_in.sin_addr), AF_INET);
#endif	/* !notdef */
#endif	/* !MSDOS */  
  }
  if (host_ent) {
    sock_in.sin_family = host_ent->h_addrtype;
  } else {
    print_fatal("Unknown %s server \"%s\".", service_name, server_name);
    return(-1);
  }

  print_warning("Trying to connect %s.", server_name);
  s = x = -1;
  while (1) {
    sock_in.sin_port = htons(port);
#ifdef	h_addr
    for (pptr = host_ent->h_addr_list; pptr && *pptr; pptr++) {
      s = socket(host_ent->h_addrtype, SOCK_STREAM, 0);
      if (s < 0) {
	print_fatal("Can't open socket.");
	return(-1);
      }
      memcpy((char*)&sock_in.sin_addr, *pptr, host_ent->h_length);
      inet_adrs_ptr = (unsigned char*)&sock_in.sin_addr;
#ifdef	notdef
      if (x < 0) {
	print_warning("Trying to connect %d.%d.%d.%d",
		      inet_adrs_ptr[0], inet_adrs_ptr[1],
		      inet_adrs_ptr[2], inet_adrs_ptr[3]);
      }
#endif	/* notdef */
      x = connect(s, (struct sockaddr*)&sock_in, sizeof(sock_in));
      if (!x) {
	break;
      }
#ifdef	notdef
      print_warning("Connect to %d.%d.%d.%d",
		    inet_adrs_ptr[0], inet_adrs_ptr[1],
		    inet_adrs_ptr[2], inet_adrs_ptr[3]);
#endif	/* notdef */
      (void)close(s);
    }
#else	/* !h_addr */
#ifdef	EXCELAN
    if ((s = socket(SOCK_STREAM,(struct sockproto*)NULL, &sock_in,
		    SO_KEEPALIVE)) < 0) {
      print_fatal("Can't open socket.");
      return(-1);
    }
    memset(&sock_in, 0, sizeof(sock_in));
    sock_in.sin_family = AF_INET;
    sock_in.sin_port   = htons(port);
    
    if ((sock_in.sin_addr.s_addr = rhost(&server_name)) == -1) {
      print_fatal("Unknown %s server \"%s\".", service_name, server_name);
      return(-1);
    }
    x = connect(s, (struct sockaddr*)&sock_in);
#else	/* !EXCELAN */
#ifdef	MSDOS
    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      print_fatal("Can't open socket.");
      return(-1);
    }
#else	/* !MSDOS */
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      print_fatal("Can't open socket.");
      return(-1);
    }
#endif	/* !MSDOS */
#ifdef	INETBIOS
    sock_in.sin_addr.s_addr = host_ent->h_addr;
    x = connect(s, (struct sockaddr_in*)&sock_in, sizeof(sock_in));
#else	/* !INETBIOS */
    memcpy((char*)&sock_in.sin_addr, host_ent->h_addr, host_ent->h_length);
    x = connect(s, (struct sockaddr*)&sock_in, sizeof(sock_in));
#endif	/* !INETBIOS */
#endif	/* !EXCELAN */
#endif	/* !h_addr */
    if (default_port < 0) {
      if (port > (IPPORT_RESERVED / 2)) {
	port--;
      } else {
	break;
      }
    } else {
      break;
    }
  }
  if (x < 0) {
    print_fatal("Can't connect %s server \"%s\".", service_name, server_name);
    if (s >= 0) {
      (void)close(s);
    }
    return(-1);
  }
#ifdef	MSDOS
  *read_fdd = s;
#else	/* !MSDOS */
  rfp = (TCPRD_FILE*)malloc(sizeof(TCPRD_FILE));
  if (rfp == (TCPRD_FILE*)NULL) {
    close(s);
    return(1);
  }
  rfp->fd = s;
  rfp->len = 0;
  *read_fpp = rfp;
#endif	/* !MSDOS */
  *write_fdd = s;
  tcp_connect_number++;
  return(0);
}

/*
 * ソケットクローズ
 */

#ifndef	MSDOS
#ifdef	__STDC__
static void	socket_fclose(TCPRD_FILE *fp)
#else	/* !__STDC__ */
static void	socket_fclose(fp)
     TCPRD_FILE	*fp;
#endif	/* !__STDC__ */
{
  close(fp->fd);
  free(fp);
}

/*
 * ソケットライト
 * 戻り値
 *	正数:正常終了
 *	   0:接続切断
 *	負数:エラー
 */

#ifdef	__STDC__
static int	socket_write(int fd, char *buf, int len)
#else	/* !__STDC__ */
static int	socket_write(fd, buf, len)
     int	fd;
     char	*buf;
     int	len;
#endif	/* !__STDC__ */
{
  int	nw = 0;

  for ( ; len > 0; len -= nw, buf += nw) {
  again:
    errno = 0;
    if ((nw = write(fd, buf, len)) <= 0) {
      if (errno == EINTR) {
	goto again;
      }
    }
    if (nw <= 0) {
      break;
    }
  }
  return(nw);
}

/*
 * 1 行入力。シグナル処理が入った場合は再処理する。
 * (BSD システムでは不必要)
 */

#ifdef	__STDC__
static int	socket_gets(TCPRD_FILE *fp, char *buf, int buflen)
#else	/* !__STDC__ */
static int	socket_gets(fp, buf, buflen)
     TCPRD_FILE	*fp;
     char	*buf;
     int	buflen;
#endif	/* !__STDC__ */
{
  int	len = fp->len;
  char	*ptr = fp->ptr;
  int	cnt = 0;

  while (--buflen > 0) {
    if (!len) {
    again:
      errno = 0;
      if ((len = read(fp->fd, fp->buf, sizeof fp->buf)) < 0) {
	if (errno == EINTR) {
	  goto again;
	}
	fp->len = 0;
	if (cnt > 0) {
	  buf[cnt] = '\0';
	  return(cnt);
	} else {
	  return(-1);	/* error */
	}
      }
      if (len == 0) {
	fp->len = 0;
	if (cnt > 0) {
	  buf[cnt] = '\0';
	  return(cnt);
	} else {
	  return(0);	/* end of file */
	}
      }
      ptr = fp->buf;
    }
    len--;
    if ((buf[cnt++] = *ptr++) == '\n') {
      break;
    }
  }
  buf[cnt] = '\0';
  fp->ptr = ptr;
  fp->len = len;
  return(cnt);
}
#endif	/* !MSDOS */

/*
 * TCP/IP ソケットクローズ
 */

#ifdef	MSDOS
#ifdef	__STDC__
int	tcp_close(int *read_fdd, int *write_fdd)
#else	/* !__STDC__ */
int	tcp_close(read_fdd, write_fdd)
     int	*read_fdd;
     int	*write_fdd;
#endif	/* !__STDC__ */
{
  if (*read_fdd >= 0) {
    closesocket(*read_fdd);
    tcp_connect_number--;
  }
  *read_fdd = -1;
  *write_fdd = -1;
  return(0);
}
#else	/* !MSDOS */
#ifdef	__STDC__
int	tcp_close(TCPRD_FILE **read_fpp, int *write_fdd)
#else	/* !__STDC__ */
int	tcp_close(read_fpp, write_fdd)
     TCPRD_FILE	**read_fpp;
     int	*write_fdd;
#endif	/* !__STDC__ */
{
  if (*read_fpp != (TCPRD_FILE*)NULL) {
    tcp_connect_number--;
    socket_fclose(*read_fpp);
  }
  if (*write_fdd != -1) {
    close(*write_fdd);
  }
  *read_fpp = (TCPRD_FILE*)NULL;
  *write_fdd = -1;
  return(0);
}
#endif	/* !MSDOS */

/*
 * TCP/IP 送信
 */

#ifdef	MSDOS
#ifdef	__STDC__
int	tcp_send(char *service_name, int *read_pfd, int *write_pfd,
		 char *string)
#else	/* !__STDC__ */
int	tcp_send(service_name, read_pfd, write_pfd, string)
     char	*service_name;
     int	*read_pfd;
     int	*write_pfd;
     char	*string;
#endif	/* !__STDC__ */
{
#ifdef	MSDOS
  char		buff[BUFF_SIZE];
#endif	/* MSDOS */

#ifdef	MSDOS
  if (*write_pfd == -1) {
#else	/* !MSDOS */
  if (write_fp == (FILE*)NULL) {
#endif	/* !MSDOS */
    print_fatal("%s write stream not ready.", service_name);
    return(1);
  }
#ifdef	TCP_DEBUG
  fprintf(stderr, "SEND:%s\n", string);
#endif	/* TCP_DEBUG */

  /*
   * DECstation では setenv PROG_ENV POSIX して compile しないと
   * printf の戻り値が正常時に 0 なのでうまく動きません。
   */

#ifdef	MSDOS
  sprintf(buff, "%s\r\n", string);
  if (send(*write_pfd, buff, strlen(buff), 0) > 0) {
    return(0);
  }
#else	/* !MSDOS */
  if (fprintf2(write_fp, "%s\r\n", string) == (strlen(string) + 2)) {
    if (!fflush(write_fp)) {
      return(0);
    }
  }
#endif	/* !MSDOS */
  print_fatal("%s connection closed.", service_name);
#ifdef	MSDOS
  closesocket(*write_pfd);
  *read_pfd = *write_pfd = -1;
#else	/* !MSDOS */
  if (write_fp != (FILE*)NULL) {
    fclose(write_fp);
    write_fp = (FILE*)NULL;
  }
  if (read_fp != (FILE*)NULL) {
    fclose(read_fp);
    read_fp = (FILE*)NULL;
  }
#endif	/* !MSDOS */
  return(1);
}
#else	/* !MSDOS */
#ifdef	__STDC__
int	tcp_send(char *service_name, TCPRD_FILE **read_pfp, int *write_pfd,
		 char *string)
#else	/* !__STDC__ */
int	tcp_send(service_name, read_pfp, write_pfd, string)
  char		*service_name;
  TCPRD_FILE	**read_pfp;
  int		*write_pfd;
  char		*string;
#endif	/* !__STDC__ */
{
  char		buff[BUFF_SIZE];
  int		len;

#ifdef	MSDOS
  if (write_fd == -1) {
#else	/* !MSDOS */
  if (*write_pfd == -1) {
#endif	/* !MSDOS */
    print_fatal("%s write stream not ready.", service_name);
    return(1);
  }
#ifdef	TCP_DEBUG
  fprintf(stderr, "SEND:%s\n", string);
#endif	/* TCP_DEBUG */

  /*
   * DECstation では setenv PROG_ENV POSIX して compile しないと
   * printf の戻り値が正常時に 0 なのでうまく動きません。
   */

  len = strlen(string);
  strcpy(buff, string);
  buff[len++] = '\r';
  buff[len++] = '\n';
  buff[len] = '\0';
#ifdef	MSDOS
  if (send(write_fd, buff, len, 0) > 0) {
    return(0);
  }
#else	/* !MSDOS */
  if (socket_write(*write_pfd, buff, len) > 0) {
    return(0);
  }
#endif	/* !MSDOS */
  print_fatal("%s connection closed.", service_name);
#ifdef	MSDOS
  closesocket(write_fd);
  read_fd = write_fd = -1;
#else	/* !MSDOS */
  if (*write_pfd != -1) {
    close(*write_pfd);
    *write_pfd = -1;
  }
  if (*read_pfp != (TCPRD_FILE*)NULL) {
    socket_fclose(*read_pfp);
    *read_pfp = (TCPRD_FILE*)NULL;
  }
#endif	/* !MSDOS */
  return(1);
}
#endif	/* !MSDOS */

/*
 * TCP/IP 受信
 */

#ifdef	MSDOS
#ifdef	__STDC__
int	tcp_receive(char *service_name, int *read_pfd, int *write_pfd,
		    char *ptr, int size)
#else	/* !__STDC__ */
int	tcp_receive(service_name, read_pfd, write_pfd, ptr, size)
  char	*service_name;
  int	*read_pfd;
  int	*write_pfd;
  char	*ptr;
  int	size;
#endif	/* !__STDC__ */
{
  static char		*ptr2;
  static char		read_buff[BUFF_SIZE];
  static int		count = 0;
  char			c;
  
  if (*read_pfd == -1) {
    print_fatal("%s read stream not ready.", service_name);
    return(1);
  }
  while (1) {
    if (count <= 0) {
#ifdef	INETBIOS
      count = _recv(*read_pfd, read_buff, sizeof(read_buff), 0);
#else	/* !INETBIOS */
      count = recv(*read_pfd, read_buff, sizeof(read_buff), 0);
#endif	/* !INETBIOS */
      ptr2 = read_buff;
      if (count <= 0) {
	print_fatal("%s connection closed.", service_name);
	closesocket(*write_pfd);
	*read_pfd = *write_pfd = -1;
	return(1);
      }
    }
    while (count--) {
      c = *ptr2++;
      if (c == '\n') {
        goto done;
      }
      if (c != '\r') {
	*ptr++ = c;
	if (--size == 0) {
	  goto done;
	}
      }
    }
  }
done:
  *ptr = '\0';
#ifdef	TCP_DEBUG
  fprintf(stderr, "RECEIVE:%s\n", ptr);
#endif	/* TCP_DEBUG */
  return(0);
}
#else	/* !MSDOS */
#ifdef	__STDC__
int	tcp_receive(char *service_name, TCPRD_FILE **read_pfp, int *write_pfd,
		    char *ptr, int size)
#else	/* !__STDC__ */
int	tcp_receive(service_name, read_pfp, write_pfd, ptr, size)
     char	*service_name;
     TCPRD_FILE	**read_pfp;
     int	*write_pfd;
     char	*ptr;
     int	size;
#endif	/* !__STDC__ */
{
  register char		*ptr2;
  
  if (*read_pfp == (TCPRD_FILE*)NULL) {
    print_fatal("%s read stream not ready.", service_name);
    return(1);
  }
  if (socket_gets(*read_pfp, ptr, size) <= 0) {
    print_fatal("%s connection closed.", service_name);
    if (*write_pfd != -1) {
      close(*write_pfd);
      *write_pfd = -1;
    }
    if (*read_pfp != (TCPRD_FILE*)NULL) {
      socket_fclose(*read_pfp);
      *read_pfp = (TCPRD_FILE*)NULL;
    }
    return(1);
  }
  ptr2 = strchr(ptr, '\r');
  if (ptr2 != (char*)NULL) {
    *ptr2 = '\0';
  } else {
    ptr2 = strchr(ptr, '\n');
    if (ptr2 != (char*)NULL) {
      *ptr2 = '\0';
    }
  }
#ifdef	TCP_DEBUG
  fprintf(stderr, "RECEIVE:%s\n", ptr);
#endif	/* TCP_DEBUG */
  return(0);
}
#endif	/* !MSDOS */

/*
 * デフォルトサーバ名取得
 */

#ifdef	__STDC__
int	tcp_get_default_server(char *filename, char *server_name,
			       char *env_name, unsigned int *port_ptr)
#else	/* !__STDC__ */
int	tcp_get_default_server(filename, server_name, env_name, port_ptr)
  char		*filename;
  char		*server_name;
  char		*env_name;
  unsigned int	*port_ptr;
#endif	/* !__STDC__ */
{
  FILE		*fp;
  register char	*ptr1, *ptr2;
  char		buff[BUFF_SIZE];
  
  ptr1 = (char*)getenv(env_name);
  if (ptr1 != (char*)NULL) {
    strcpy(server_name, ptr1);
    ptr2 = strchr(server_name, ':');
    if (ptr2 != (char*)NULL) {
      *ptr2++ = '\0';
      if (atoi(ptr2) > 0) {
	*port_ptr = atoi(ptr2) & 0xFFFF;
      }
    }
    return(0);
  }
  if (!filename) {
    return(1);
  }
  if (!*filename) {
    return(1);
  }
  fp = fopen(filename, "r");
  if (fp == (FILE*)NULL) {
    return(1);
  }
  while (fgets(buff, sizeof(buff), fp)) {
    ptr1 = buff;
    while ((*ptr1 == '\t') || (*ptr1 == ' ')) {
      ptr1++;
    }
    ptr2 = ptr1;
    if ((*ptr1 == '\n') || (*ptr1 == '#')) {
      continue;
    }
    ptr1 = strchr(ptr1, '\n');
    if (ptr1) {
      *ptr1 = '\0';
    }
    fclose(fp);
    strcpy(server_name, ptr2);
    return(0);
  }
  fclose(fp);
  return(1);			 /* No entry */
}

/*
 * ユーザ/パスワード取得
 */

#ifdef	__STDC__
int	tcp_get_user_pass(char *server_name, char *user_ptr, char *pass_ptr)
#else	/* !__STDC__ */
int	tcp_get_user_pass(server_name, user_ptr, pass_ptr)
     char	*server_name;
     char	*user_ptr;
     char	*pass_ptr;
#endif	/* !__STDC__ */
{
  FILE			*fp;
  register char		*ptr1, *ptr2;
  char			buff[BUFF_SIZE];
  struct hostent	*server_entry;
#ifndef	USE_NEWSAUTH
  static char		*param_name[] = {
    "machine", "default", "login", "password", "account", "macdef"
    };
  int			match;
  int			line;
  register char		*ptr3;
  register int		i;
#endif	/* !USE_NEWSAUTH */
#if	!(defined(MNEWS) || defined(MMH))
#ifdef	MSDOS
  char			*env_ptr;
#else	/* !MSDOS */
  struct passwd		*passwd;
#endif	/* !MSDOS */
#endif	/* !(MNEWS || MMH) */

  *user_ptr = '\0';
  if (pass_ptr != (char*)NULL) {
    *pass_ptr = '\0';
  }
#if	defined(MNEWS) || defined(MMH)
  strcpy(user_ptr, user_name);
#else	/* !(MNEWS || MMH) */
#ifdef	MSDOS
  if (env_ptr = (char*)getenv("USER")) {
    strcpy(user_ptr, env_ptr);
  } else {
    return(1);
  }
#else	/* !MSDOS */
  passwd = (struct passwd*)getpwuid(geteuid());
  if (passwd != (struct passwd*)NULL) {
    strcpy(user_ptr, passwd->pw_name);
  } else {
    return(1);
  }
#endif	/* !MSDOS */
#endif	/* !(MNEWS || MMH) */
#ifdef	NNTP_AUTH
  if (nntp_get_pass) {
    if (nntp_user[0]) {
      strcpy(user_ptr, nntp_user);
    }
  }
#endif	/* NNTP_AUTH */
#ifdef	USE_NEWSAUTH
  if (pass_ptr == (char*)NULL) {
    return(0);
  }
#else	/* !USE_NEWSAUTH */
  match = line = 0;
#endif	/* !USE_NEWSAUTH */
  if (auth_file[0]) {
    fp = fopen(auth_file, "r");
    if (fp != (FILE*)NULL) {
      while (fgets(buff, sizeof(buff), fp)) {
#ifndef	USE_NEWSAUTH
	line++;
#endif	/* !USE_NEWSAUTH */
	ptr1 = buff;
	while ((*ptr1 == '\t') || (*ptr1 == ' ')) {
	  ptr1++;
	}
	if ((*ptr1 == '\n') || (*ptr1 == '#')) {
	  continue;
	}
	ptr2 = strchr(ptr1, '\n');
	if (ptr2 != (char*)NULL) {
	  *ptr2 = '\0';
	}
#ifdef	USE_NEWSAUTH
	ptr2 = ptr1;
	while (*ptr2 && (*ptr2 != '\t') && (*ptr2 != ' ')) {
	  ptr2++;
	}
	if (!*ptr2) {
	  continue;
	}
	*ptr2++ = '\0';
	server_entry = gethostbyname(ptr1);
	if (server_entry == (struct hostent*)NULL) {
	  continue;
	}
	if (!strcmp(server_entry->h_name, server_name)) {
	  while ((*ptr2 == '\t') || (*ptr2 == ' ')) {
	    ptr2++;
	  }
	  if ((!*ptr2) || (*ptr2 == '#')) {
	    continue;
	  }
	  strcpy(pass_ptr, ptr2);
	  break;
	}
#else	/* !USE_NEWSAUTH */
	while (*ptr1) {
	  ptr2 = ptr1;
	  while (*ptr2 && (*ptr2 != '\t') && (*ptr2 != ' ')) {
	    ptr2++;
	  }
	  while ((*ptr2 == '\t') || (*ptr2 == ' ')) {
	    *ptr2++ = '\0';
	  }
	  for (i = 0; i < (sizeof(param_name) / sizeof(char*)); i++) {
	    if (!strcmp(ptr1, param_name[i])) {
	      switch (i) {
	      case 0:			/* machine	*/
		if (!*ptr2) {
		  print_fatal("Line %4d : Host name not specified in \"%s\".",
			      line, auth_file);
		  break;
		}
		ptr3 = ptr2;
		while (*ptr2 && (*ptr2 != '\t') && (*ptr2 != ' ')) {
		  ptr2++;
		}
		if (*ptr2) {
		  *ptr2++ = '\0';
		}
		server_entry = gethostbyname(ptr3);
		if (server_entry != (struct hostent*)NULL) {
		  if (!strcmp(server_entry->h_name, server_name)) {
		    match = 1;
		  } else {
		    if (match) {
		      match = -1;
		    }
		  }
		}
		break;
	      case 1:			/* default	*/
		if (!match) {
		  match = 1;
		}
		break;
	      case 2:			/* login	*/
		if (!*ptr2) {
		  print_fatal("Line %4d : User name not specified in \"%s\".",
			      line, auth_file);
		  break;
		}
		ptr3 = ptr2;
		while (*ptr2 && (*ptr2 != '\t') && (*ptr2 != ' ')) {
		  ptr2++;
		}
		if (*ptr2) {
		  *ptr2++ = '\0';
		}
		if (match > 0) {
		  strcpy(user_ptr, ptr3);
		}
		break;
	      case 3:			/* password	*/
		if (!*ptr2) {
		  print_fatal("Line %4d : Password not specified in \"%s\".",
			      line, auth_file);
		  break;
		}
		ptr3 = ptr2;
		while (*ptr2 && (*ptr2 != '\t') && (*ptr2 != ' ')) {
		  ptr2++;
		}
		if (*ptr2) {
		  *ptr2++ = '\0';
		}
		if (match > 0) {
		  if (pass_ptr != (char*)NULL) {
		    strcpy(pass_ptr, ptr3);
		  }
		}
		break;
	      case 4:			/* account	*/
	      case 5:			/* macdef	*/
		while (*ptr2 && (*ptr2 != '\t') && (*ptr2 != ' ')) {
		  ptr2++;
		}
		break;
	      default:
		print_fatal("Line %4d : Illegal keyword in \"%s\".",
			    line, auth_file);
		break;
	      }
	      break;
	    }
	  }
	  while ((*ptr2 == '\t') || (*ptr2 == ' ')) {
	    *ptr2++ = '\0';
	  }
	  ptr1 = ptr2;
	}
#endif	/* !USE_NEWSAUTH */
      }
      fclose(fp);
    }
  }
  if (*user_ptr && (pass_ptr ? *pass_ptr : 1)) {
    return(0);
  }
#ifdef	MNEWS
  strcpy(buff, user_ptr);
  input_line(INPUT_SPCCUT_MASK ,"ユーザ名を入力して下さい:",
	     "Input user name :" , buff);
  if (!buff[0]) {
    *user_ptr = '\0';
    return(1);
  }
  strcpy(user_ptr, buff);
  buff[0] = '\0';
  input_line(INPUT_SHADOW_MASK,
	     "パスワードを入力して下さい:",
	     "Input password :", buff);
  strcpy(pass_ptr, buff);
#else	/* !MNEWS */
#ifdef	MMH
  term_init(0);
#endif	/* MMH */
  fprintf(stderr, "Password:");
  {
    char	*ptr;
    int		i;
    int		key;

    i = 0;
    ptr = pass_ptr;
    *ptr = '\0';
    while (i >= 0) {
#ifdef	MSDOS
      key = getch();
#else	/* !MSDOS */
      key = getchar();
#endif	/* !MSDOS */
      switch (key) {
      case 0x08:	/* ^H		*/
	if (i > 0) {
	  ptr--;
	  i--;
	  *ptr = '\0';
	}
	break;
      case 0x0D:	/* RETURN	*/
	i = -1;
	break;
      case 0x15:	/* ^U		*/
	i = 0;
	ptr = pass_ptr;
	*ptr = '\0';
	break;
      default:
	if (i < (SMALL_BUFF - 1)) {
	  *ptr++ = key;
	  *ptr = '\0';
	  i++;
	}
	break;
      }
    }
  }
#ifdef	MMH
  term_init(3);
#endif	/* MMH */
  fprintf(stderr, "\n");
#endif	/* !MNEWS */
  return(0);
}
#endif	/* (NNTP || SMTP || POP3) */

/*
 * 記事フィールドコピー(NNTP 接続/ファイル兼用)
 */

#ifdef	__STDC__
int	copy_fields(FILE *fp, struct cpy_hdr *hdr_ptr1, int number,
		    int mask)
#else	/* !__STDC__ */
int	copy_fields(fp, hdr_ptr1, number, mask)
     FILE		*fp;
     struct cpy_hdr	*hdr_ptr1;
     int		number;
     int		mask;
#endif	/* !__STDC__ */
{
  struct cpy_hdr	*hdr_ptr2;
  char			buff[BUFF_SIZE];
  char			*ptr1, *ptr2;
  register int		lines = 0;
  register int		i;

  hdr_ptr2 = hdr_ptr1;
  if (mask & CF_CLR_MASK) {
    for (i = 0; i < number; i++) {
      ptr1 = hdr_ptr2->field_buff;
      if (ptr1 != (char*)NULL) {
	*ptr1 = '\0';
      }
      hdr_ptr2++;
    }
  }
  if (!(mask & (CF_CNT_MASK | CF_GET_MASK))) {
    return(0);
  }
  ptr1 = (char*)NULL;
  while (1) {
    if (fp != (FILE*)NULL) {
      if (!fgets(buff, sizeof(buff), fp)) {
	break;
      }
#if	defined(MSDOS) || defined(OS2)
      /*
       * バイナリモードで fopen された場合、改行コードを削除する。
       */

      ptr2 = buff;
      while (*ptr2) {
	if ((*ptr2 == '\n') || (*ptr2 == '\r')) {
	  *ptr2 = '\0';
	  break;
	}
	ptr2++;
      }
#endif	/* (MSDOS || OS2) */
    } else {
#if	defined(NNTP) || defined(NSPL)
      if (nntp_receive(buff, sizeof(buff))) {
	break;
      }
      if ((buff[0] == TCP_END_MARK) && (!buff[1])) {
	break;
      }
#else	/* !(NNTP || NSPL) */
      break;
#endif	/* !(NNTP || NSPL) */
    }
    if ((buff[0] == ' ') || (buff[0] == '\t')) {
      if (ptr1 != (char*)NULL) {
	ptr2 = buff;
	if (mask & CF_SPC_MASK) {
	  while ((*ptr2 == ' ') || (*ptr2 == '\t')) {
	    ptr2++;
	  }
	}
	if ((int)(strlen(ptr1) + strlen(ptr2)) < (hdr_ptr2->buff_size)) {
	  strcat(ptr1, ptr2);
	  ptr2 = ptr1;
	  while (*ptr2) {
	    if ((*ptr2 == '\n') && (!*(ptr2 + 1))) {
	      *ptr2 = '\0';
	      break;
	    }
	    ptr2++;
	  }
	}
      }
    } else {
      ptr1 = (char*)NULL;
      hdr_ptr2 = hdr_ptr1;
      for (i = 0; i < number; i++) {
	if (!strncasecmp(buff, hdr_ptr2->field_name,
			 strlen(hdr_ptr2->field_name))) {
	  ptr1 = hdr_ptr2->field_buff;
	  if (ptr1 != (char*)NULL) {
	    ptr2 = &buff[strlen(hdr_ptr2->field_name)];
	    while ((*ptr2 == ' ') || (*ptr2 == '\t')) {
	      ptr2++;
	    }
	    if ((mask & CF_ADD_MASK) && *ptr1) {
	      if ((int)(strlen(ptr1) + strlen(ptr2) + 1) <
		  (hdr_ptr2->buff_size)) {
		strcat(ptr1, "\n");
		strcat(ptr1, ptr2);
	      }
	    } else {
	      if ((int)strlen(ptr2) < (hdr_ptr2->buff_size)) {
		strcpy(ptr1, ptr2);
	      }
	    }
	    ptr2 = ptr1;
	    while (*ptr2) {
	      if ((*ptr2 == '\n') && (!*(ptr2 + 1))) {
		*ptr2 = '\0';
		break;
	      }
	      ptr2++;
	    }
	    if (!*ptr1) {		/* 空フィールド検出 */
	      *ptr1++ = ' ';
	      *ptr1 = '\0';
	    }
	  }
	  i = -1;
	  break;
	}
	hdr_ptr2++;
      }
      if (i >= 0) {
	if (mask & CF_CNT_MASK) {
	  if (!strncasecmp(buff, LINE_FIELD, strlen(LINE_FIELD))) {
	    ptr2 = &buff[strlen(LINE_FIELD)];
	    while ((*ptr2 == ' ') || (*ptr2 == '\t')) {
	      ptr2++;
	    }
	    lines = atoi(ptr2);
	  }
	}
	if ((!buff[0]) || (buff[0] == '\n')) {
	  if (mask & CF_CNT_MASK) {
	    if (fp != (FILE*)NULL) {
	      if (!lines) {
		while (fgets(buff, sizeof(buff), fp)) {
		  lines++;
		}
	      }
	    } else {
	      lines = 0;
	      while (1) {
#if	defined(NNTP) || defined(NSPL)
		if (nntp_receive(buff, sizeof(buff))) {
		  break;
		}
		if ((buff[0] == TCP_END_MARK) && (!buff[1])) {
		  break;
		}
#else	/* !(NNTP || NSPL) */
		break;
#endif	/* !(NNTP || NSPL) */
		lines++;
	      }
	    }
	  }
	  if (!(mask & CF_HED_MASK)) {
	    break;
	  }
	}
      }
    }
  }
  return(lines);
}

/*
 * 記事フィールドコピー
 */

#ifdef	__STDC__
char	*copy_field(char *ptr1, char *ptr2, char *field)
#else	/* !__STDC__ */
char	*copy_field(ptr1, ptr2, field)
     char	*ptr1;
     char	*ptr2;
     char	*field;
#endif	/* !__STDC__ */
{
  register int	i;

  if ((!ptr1) || (!ptr2)) {
    return((char*)NULL);
  }
  i = strlen((char*)field);
  while (i-- > 0) {
    if ((*ptr1++ & 0xdf) != (*field++ & 0xdf)) {
      return((char*)NULL);
    }
  }
  while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
    ptr1++;
  }
  strcpy((char*)ptr2, (char*)ptr1);
  while (*ptr2) {
    if (*ptr2 == '\n') {
      *ptr2 = '\0';
      break;
    }
    ptr2++;
  }
  return(ptr2);
}

#if	!(defined(MNEWS) || defined(MAKEKIN) || defined(MMH))
/*
 * 警告エラー表示
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	print_warning(char *fmt, int arg1, int arg2, int arg3, int arg4,
		      int arg5, int arg6, int arg7, int arg8)
#else	/* !__STDC__ */
void	print_warning(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
#endif	/* !__STDC__ */
{
  char		buff[SMALL_BUFF];

  if (!quiet_mode) {
    sprintf(buff, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    fprintf(stderr, "%s : Warning : %s\n", prog_name, buff);
    fflush(stderr);
  }
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	print_warning(char *fmt, ...)
#else	/* !__STDC__ */
void	print_warning(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff[SMALL_BUFF];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  vsprintf(buff, fmt, ap);
  if (!quiet_mode) {
    fprintf(stderr, "%s : Warning : %s\n", prog_name, buff);
    fflush(stderr);
  }
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */

/*
 * 致命的エラー表示
 */

#ifdef	DONT_HAVE_DOPRNT
#ifdef	__STDC__
void	print_fatal(char *fmt, int arg1, int arg2, int arg3, int arg4,
		    int arg5, int arg6, int arg7, int arg8)
#else	/* !__STDC__ */
void	print_fatal(fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
     char	*fmt;
     int	arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
#endif	/* !__STDC__ */
{
  char		buff[SMALL_BUFF];

  if (quiet_mode < 2) {
    sprintf(buff, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    fprintf(stderr, "%s : %s\n", prog_name, buff);
    fflush(stderr);
  }
}
#else	/* !DONT_HAVE_DOPRNT */
#ifdef	__STDC__
void	print_fatal(char *fmt, ...)
#else	/* !__STDC__ */
void	print_fatal(fmt, va_alist)
     char	*fmt;
#endif	/* !__STDC__ */
{
  va_list	ap;
  char		buff[SMALL_BUFF];

#ifdef	__STDC__
  va_start(ap, fmt);
#else	/* !__STDC__ */
  va_start(ap);
#endif	/* !__STDC__ */
  vsprintf(buff, fmt, ap);
  if (quiet_mode < 2) {
    fprintf(stderr, "%s : %s\n", prog_name, buff);
    fflush(stderr);
  }
  va_end(ap);
}
#endif	/* !DONT_HAVE_DOPRNT */
#endif	/* !(MNEWS || MANEKIN || MMH) */
