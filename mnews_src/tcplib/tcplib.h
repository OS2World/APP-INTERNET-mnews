/*
 *
 *  TCP/IP �饤�֥��
 *
 *  Copyright 1995-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : NNTP/SMTP/POP3 library
 *  Sub system  : TCP/IP define
 *  File        : tcplib.h
 *  Version     : 0.82
 *  First Edit  : 1995-12/25
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 ��� ��
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
 * ǧ�ڥե�����
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
 * �ե�����ɽ����ޥ���
 */

#define	CF_CLR_MASK	0x01		/* �Хåե����ꥢ	*/
#define	CF_CNT_MASK	0x02		/* �Կ��������		*/
#define	CF_GET_MASK	0x04		/* �ե�����ɼ���	*/
#define	CF_ADD_MASK	0x08		/* �ե�������ɲ�	*/
#define	CF_SPC_MASK	0x10		/* �������		*/
#define	CF_HED_MASK	0x20		/* HEAD ���ޥ��	*/

struct cpy_hdr {
  char	*field_name;
  char	*field_buff;
  int	buff_size;
};

extern char	auth_file[];		/* �桼��ǧ�ڥե�����		*/
extern int	tcp_connect_number;	/* TCP/IP ��³��		*/
#if	!(defined(MNEWS) || defined(MAKEKIN))
extern char	prog_name[];		/* �ץ����̾			*/
extern short	quiet_mode;		/* ���顼/�ٹ������⡼��	*/
#endif	/* !(MNEWS || MANEKIN) */

#ifdef	__STDC__
extern int	tcp_init(void);		/* TCP/IP ���������		*/
extern int	tcp_exit(void);		/* TCP/IP ��λ����		*/
#else	/* !__STDC__ */
extern int	tcp_init();		/* TCP/IP ���������		*/
extern int	tcp_exit();		/* TCP/IP ��λ����		*/
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
					/* TCP/IP �����åȥ����ץ�	*/
extern int	tcp_close(int*, int*);
					/* TCP/IP �����åȥ�����	*/
extern int	tcp_send(char*, int*, int*, char*);
					/* TCP/IP ����			*/
extern int	tcp_receive(char*, int*, int*, char*, int);
					/* TCP/IP ����			*/
#else	/* !MSDOS */
extern int	tcp_open(char*, char*, char*, unsigned int, int, TCPRD_FILE**,
			 int*);
					/* TCP/IP �����åȥ����ץ�	*/
extern int	tcp_close(TCPRD_FILE**, int*);
					/* TCP/IP �����åȥ�����	*/
extern int	tcp_send(char*, TCPRD_FILE**, int*, char*);
					/* TCP/IP ����			*/
extern int	tcp_receive(char*, TCPRD_FILE**, int*, char*, int);
					/* TCP/IP ����			*/
#endif	/* !MSDOS */
extern int	tcp_get_default_server(char*, char*, char*, unsigned int*);
					/* �ǥե���ȥ�����̾����	*/
extern int	tcp_get_user_pass(char*, char*, char*);
					/* �桼��/�ѥ���ɼ���	*/
#else	/* !__STDC__ */
extern int	tcp_open();		/* TCP/IP �����åȥ����ץ�	*/
extern int	tcp_close();		/* TCP/IP �����åȥ�����	*/
extern int	tcp_send();		/* TCP/IP ����			*/
extern int	tcp_receive();		/* TCP/IP ����			*/
extern int	tcp_get_default_server();
					/* �ǥե���ȥ�����̾����	*/
extern int	tcp_get_user_pass();	/* �桼��/�ѥ���ɼ���	*/
#endif	/* !__STDC__ */
#endif	/* (NNTP || SMTP || POP3) */

#if	defined(__STDC__) && !defined(DONT_HAVE_DOPRNT)
extern int	copy_fields(FILE*, struct cpy_hdr*, int, int);
					/* �����إå����ԡ�(ʣ��)	*/
extern char	*copy_field(char*, char*, char*);
					/* �����إå����ԡ�		*/
extern void	print_warning(char*, ...);
					/* �ٹ𥨥顼ɽ��		*/
extern void	print_fatal(char*, ...);/* ��̿Ū���顼ɽ��		*/
#else	/* (!__STDC__ || DONT_HAVE_DOPRNT) */
extern int	copy_fields();		/* �����إå����ԡ�(ʣ��)	*/
extern char	*copy_field();		/* �����إå����ԡ�		*/
extern void	print_warning();	/* �ٹ𥨥顼ɽ��		*/
extern void	print_fatal();		/* ��̿Ū���顼ɽ��		*/
#endif	/* (!__STDC__ || DONT_HAVE_DOPRNT) */

#endif	/* !__TCPLIB_H__ */
