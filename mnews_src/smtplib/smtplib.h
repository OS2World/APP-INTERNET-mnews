/*
 *
 *  SMTP �饤�֥��
 *
 *  Copyright 1995-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : SMTP protocol library
 *  Sub system  : SMTP define
 *  File        : smtplib.h
 *  Version     : 0.84
 *  First Edit  : 1995-12/25
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 ��� ��
 *
 */

#ifndef	__SMTPLIB_H__
#define	__SMTPLIB_H__

#include	"tcplib.h"

#ifndef	SMTP_SERVER_FILE
#define	SMTP_SERVER_FILE	"/usr/local/lib/default-mailsv"
#endif	/* !SMTP_SERVER_FILE */
#ifndef	SEND_PROGRAM
#define	SEND_PROGRAM		"/usr/lib/sendmail -t -om -oi"
#endif	/* !SEND_PROGRAM */
#define	SMTP_LOCAL_HOST_NAME	"localhost"

/*
 * �᡼�������Ķ�
 */

#ifndef	SMTP_PORT
#define	SMTP_PORT		25
#endif	/* !SMTP_PORT */

/*
 * SMTP ���ޥ��
 */

#define	SMTP_QUIT_CMD		"QUIT"	/* QUIT			*/
#define	SMTP_HELO_CMD		"HELO"	/* HELO domain		*/
#define	SMTP_MAIL_CMD		"MAIL"
				/* MAIL FROM:reverse-path	*/
#define	SMTP_RCPT_CMD		"RCPT"
				/* RCPT TO:forward-path		*/
#define	SMTP_DATA_CMD		"DATA"	/* DATA			*/
#define	SMTP_SEND_CMD		"SEND"
				/* SEND FROM:reverse-path	*/
#define	SMTP_SOML_CMD		"SOML"	/*			*/
				/* SOML FROM:reverse-path	*/
#define	SMTP_SAML_CMD		"SAML"	/*			*/
				/* SAML FROM:reverse-path	*/
#define	SMTP_RSET_CMD		"RSET"	/* RSET			*/
#define	SMTP_VRFY_CMD		"VRFY"	/* VRFY string		*/
#define	SMTP_EXPN_CMD		"EXPN"	/* EXPN string		*/
#define	SMTP_HELP_CMD		"HELP"	/* HELP [string]	*/
#define	SMTP_NOOP_CMD		"NOOP"	/* NOOP			*/
#define	SMTP_TURN_CMD		"TURN"	/* TURN			*/

/*
 * SMTP �쥹�ݥ�
 */

#define	SMTP_OPEN_RES		220	/* 220 H SMTP version		*/
#define	SMTP_QUIT_RES		221	/* 220 H closing connection	*/
#define	SMTP_OKAY_RES		250	/* 250 requested mail action	*/
#define	SMTP_FORWARD_RES	251	/* 251 user not local; forward	*/
#define	SMTP_INPUT_RES		354	/* 354 start mail input		*/
#define	SMTP_NO_SRV_RES		421	/* 421 H service not available	*/
#define	SMTP_BUSY_RES		450	/* 450 mailbox unavailable	*/
#define	SMTP_ABORT_RES		451	/* 451 requested action aborted	*/
#define	SMTP_STORAGE_RES	452	/* 452 insufficient sys storage	*/
#define	SMTP_ACCESS_RES		550	/* 550 mailbox unavailable	*/
#define	SMTP_TRY_RES		551	/* 551 user not local; try	*/
#define	SMTP_ALLOC_RES		552	/* 552 exceeded storage alloc	*/
#define	SMTP_SYNTAX_RES		553	/* 552 mailbox name not allowed	*/
#define	SMTP_FAIL_RES		554	/* 554 transaction failed	*/

/*
 * SMTP �ؿ����顼������
 */

typedef enum smtp_err_code {
  SMTP_OK = 0,				/* ���ｪλ		*/
  SMTP_ERR_RSV,				/* ̤���ݡ���		*/
  SMTP_ERR_BUSY,			/* ����پ���		*/
  SMTP_ERR_COMM,			/* ���٥��̿����顼	*/
  SMTP_ERR_PROT,			/* �ץ�ȥ��륨�顼	*/
  SMTP_ERR_AUTH,			/* ǧ�ڥ��顼		*/
  SMTP_ERR_PROG,			/* �ץ���२�顼	*/
  SMTP_ERR_MEM,				/* ������­���顼	*/
  SMTP_ERR_FILE,			/* �ե����륨�顼	*/
  SMTP_ERR_NONE				/* ̤¸�ߥ��顼		*/
} SMTP_ERR_CODE;

extern char		smtp_server[];	/* SMTP �����Хۥ���̾(FQDN)	*/
extern char		smtp_server_file[];
					/* SMTP �����л���ե�����	*/
extern char		send_program[];	/* �����ץ����		*/
extern short		smtp_connect_mode;
					/* SMTP ��³�⡼��		*/
					/* (0:MTAP,1:SMTP,2:DEAD)	*/
extern char		*smtp_connect_string[];
					/* SMTP ��³�⡼��ʸ����	*/

#ifdef	__STDC__
extern SMTP_ERR_CODE	smtp_select_server(char*);
					/* SMTP ����������		*/
extern SMTP_ERR_CODE	smtp_open(void);/* SMTP �����Х����ץ�		*/
extern SMTP_ERR_CODE	smtp_close(void);
					/* SMTP �����Х�����		*/
extern SMTP_ERR_CODE	smtp_send(char*);
					/* SMTP ����			*/
extern SMTP_ERR_CODE	smtp_receive(char*, int);
					/* SMTP ����			*/
extern SMTP_ERR_CODE	smtp_noop(void);/* SMTP ��³��ǧ		*/
extern SMTP_ERR_CODE	smtp_mail(char*, char*, char*);
					/* SMTP ��������		*/
extern char		*get_real_adrs(char*, char*);
					/* �¥��ɥ쥹����		*/
#else	/* !__STDC__ */
extern SMTP_ERR_CODE	smtp_select_server();
					/* SMTP ����������		*/
extern SMTP_ERR_CODE	smtp_open();	/* SMTP �����Х����ץ�		*/
extern SMTP_ERR_CODE	smtp_close();	/* SMTP �����Х�����		*/
extern SMTP_ERR_CODE	smtp_send();	/* SMTP ����			*/
extern SMTP_ERR_CODE	smtp_receive();	/* SMTP ����			*/
extern SMTP_ERR_CODE	smtp_noop();	/* SMTP ��³��ǧ		*/
extern SMTP_ERR_CODE	smtp_mail();	/* SMTP ��������		*/
extern char		*get_real_adrs();
					/* �¥��ɥ쥹����		*/
#endif	/* !__STDC__ */
#endif	/* !__SMTPLIB_H__ */
