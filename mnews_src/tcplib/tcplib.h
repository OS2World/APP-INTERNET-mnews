/*
 *
 *  TCP/IP ライブラリ
 *
 *  Copyright 1995-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : NNTP/SMTP/POP3 library
 *  Sub system  : TCP/IP define
 *  File        : tcplib.h
 *  Version     : 0.82
 *  First Edit  : 1995-12/25
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	__TCPLIB_H__
#define	__TCPLIB_H__

#ifdef	MSDOS
#if	defined(NNTP) || defined(SMTP) || defined(POP3)
#ifdef	X68K
#include	<socket.h>
#include	<network.h>
#define		closesocket(sock)	close_s(sock)
#define		recv(a, b, c, d)	read_s(a, b, c)
#define		send(a, b, c, d)	write_s(a, b, c)
#else	/* !X68K */
#ifdef	INETBIOS
#include	"inetbios.h"
#define		closesocket(sock)	soclose(sock)
#endif	/* INETBIOS */
#ifdef	WINSOCK
#include	<winsock.h>
#endif	/* WINSOCK */
#endif	/* !X68K */
#endif	/* (NNTP || SMTP || POP3) */
#include	<sys/stat.h>
#else	/* !MSDOS */
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<sys/param.h>
#endif	/* !MSDOS */

#ifndef	MAXHOSTNAMELEN
#define	MAXHOSTNAMELEN 64
#endif	/* MAXHOSTNAMELEN */

#ifndef	IPPROTO_TCP
#define	IPPROTO_TCP	6
#endif	/* !IPPROTO_TCP */
#ifndef	IPPORT_RESERVED
#define	IPPORT_RESERVED	1024
#endif	/* !IPPORT_RESERVED */

#define	TCP_END_MARK		'.'

/*
 * 認証ファイル
 */

#ifdef	USE_NEWSAUTH
#ifdef	MSDOS
#ifdef	X68K
#define	AUTHORITY_FILE	"_newsauth"
#else	/* !X68K */
#define	AUTHORITY_FILE	"_newsaut"
#endif	/* !X68K */
#else	/* !MSDOS */
#define	AUTHORITY_FILE	".newsauth"
#endif	/* !MSDOS */
#else	/* !USE_NEWSAUTH */
#ifdef	MSDOS
#define	AUTHORITY_FILE	"_netrc"
#else	/* !MSDOS */
#define	AUTHORITY_FILE	".netrc"
#endif	/* !MSDOS */
#endif	/* !USE_NEWSAUTH */

/*
 * フィールド処理マスク
 */

#define	CF_CLR_MASK	0x01		/* バッファクリア	*/
#define	CF_CNT_MASK	0x02		/* 行数カウント		*/
#define	CF_GET_MASK	0x04		/* フィールド取得	*/
#define	CF_ADD_MASK	0x08		/* フィールド追加	*/
#define	CF_SPC_MASK	0x10		/* 空白除去		*/
#define	CF_HED_MASK	0x20		/* HEAD コマンド	*/

struct cpy_hdr {
  char	*field_name;
  char	*field_buff;
  int	buff_size;
};

extern char	auth_file[];		/* ユーザ認証ファイル		*/
extern int	tcp_connect_number;	/* TCP/IP 接続数		*/
#if	!(defined(MNEWS) || defined(MAKEKIN))
extern char	prog_name[];		/* プログラム名			*/
extern short	quiet_mode;		/* エラー/警告抑制モード	*/
#endif	/* !(MNEWS || MANEKIN) */

#ifdef	__STDC__
extern int	tcp_init(void);		/* TCP/IP 初期化処理		*/
extern int	tcp_exit(void);		/* TCP/IP 終了処理		*/
#else	/* !__STDC__ */
extern int	tcp_init();		/* TCP/IP 初期化処理		*/
extern int	tcp_exit();		/* TCP/IP 終了処理		*/
#endif	/* !__STDC__ */

#ifndef	MSDOS
/* for tcp read buffering */
typedef struct {
  int	fd;
  char	*ptr;
  int	len;
  char	buf[8192];
} TCPRD_FILE;
#endif	/* !MSDOS */

#if	defined(NNTP) || defined(SMTP) || defined(POP3)
#ifdef	__STDC__
#ifdef	MSDOS
extern int	tcp_open(char*, char*, char*, unsigned int, int, int*, int*);
					/* TCP/IP ソケットオープン	*/
extern int	tcp_close(int*, int*);
					/* TCP/IP ソケットクローズ	*/
extern int	tcp_send(char*, int*, int*, char*);
					/* TCP/IP 送信			*/
extern int	tcp_receive(char*, int*, int*, char*, int);
					/* TCP/IP 受信			*/
#else	/* !MSDOS */
extern int	tcp_open(char*, char*, char*, unsigned int, int, TCPRD_FILE**,
			 int*);
					/* TCP/IP ソケットオープン	*/
extern int	tcp_close(TCPRD_FILE**, int*);
					/* TCP/IP ソケットクローズ	*/
extern int	tcp_send(char*, TCPRD_FILE**, int*, char*);
					/* TCP/IP 送信			*/
extern int	tcp_receive(char*, TCPRD_FILE**, int*, char*, int);
					/* TCP/IP 受信			*/
#endif	/* !MSDOS */
extern int	tcp_get_default_server(char*, char*, char*, unsigned int*);
					/* デフォルトサーバ名取得	*/
extern int	tcp_get_user_pass(char*, char*, char*);
					/* ユーザ/パスワード取得	*/
#else	/* !__STDC__ */
extern int	tcp_open();		/* TCP/IP ソケットオープン	*/
extern int	tcp_close();		/* TCP/IP ソケットクローズ	*/
extern int	tcp_send();		/* TCP/IP 送信			*/
extern int	tcp_receive();		/* TCP/IP 受信			*/
extern int	tcp_get_default_server();
					/* デフォルトサーバ名取得	*/
extern int	tcp_get_user_pass();	/* ユーザ/パスワード取得	*/
#endif	/* !__STDC__ */
#endif	/* (NNTP || SMTP || POP3) */

#if	defined(__STDC__) && !defined(DONT_HAVE_DOPRNT)
extern int	copy_fields(FILE*, struct cpy_hdr*, int, int);
					/* 記事ヘッダコピー(複数)	*/
extern char	*copy_field(char*, char*, char*);
					/* 記事ヘッダコピー		*/
extern void	print_warning(char*, ...);
					/* 警告エラー表示		*/
extern void	print_fatal(char*, ...);/* 致命的エラー表示		*/
#else	/* (!__STDC__ || DONT_HAVE_DOPRNT) */
extern int	copy_fields();		/* 記事ヘッダコピー(複数)	*/
extern char	*copy_field();		/* 記事ヘッダコピー		*/
extern void	print_warning();	/* 警告エラー表示		*/
extern void	print_fatal();		/* 致命的エラー表示		*/
#endif	/* (!__STDC__ || DONT_HAVE_DOPRNT) */

#endif	/* !__TCPLIB_H__ */
