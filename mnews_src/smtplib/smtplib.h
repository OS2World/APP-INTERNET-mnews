/*
 *
 *  SMTP ライブラリ
 *
 *  Copyright 1995-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : SMTP protocol library
 *  Sub system  : SMTP define
 *  File        : smtplib.h
 *  Version     : 0.84
 *  First Edit  : 1995-12/25
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 宅間 顯
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
 * メール送信環境
 */

#ifndef	SMTP_PORT
#define	SMTP_PORT		25
#endif	/* !SMTP_PORT */

/*
 * SMTP コマンド
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
 * SMTP レスポンス
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
 * SMTP 関数エラーコード
 */

typedef enum smtp_err_code {
  SMTP_OK = 0,				/* 正常終了		*/
  SMTP_ERR_RSV,				/* 未サポート		*/
  SMTP_ERR_BUSY,			/* 過負荷状態		*/
  SMTP_ERR_COMM,			/* 低レベル通信エラー	*/
  SMTP_ERR_PROT,			/* プロトコルエラー	*/
  SMTP_ERR_AUTH,			/* 認証エラー		*/
  SMTP_ERR_PROG,			/* プログラムエラー	*/
  SMTP_ERR_MEM,				/* メモリ不足エラー	*/
  SMTP_ERR_FILE,			/* ファイルエラー	*/
  SMTP_ERR_NONE				/* 未存在エラー		*/
} SMTP_ERR_CODE;

extern char		smtp_server[];	/* SMTP サーバホスト名(FQDN)	*/
extern char		smtp_server_file[];
					/* SMTP サーバ指定ファイル	*/
extern char		send_program[];	/* 送信プログラム		*/
extern short		smtp_connect_mode;
					/* SMTP 接続モード		*/
					/* (0:MTAP,1:SMTP,2:DEAD)	*/
extern char		*smtp_connect_string[];
					/* SMTP 接続モード文字列	*/

#ifdef	__STDC__
extern SMTP_ERR_CODE	smtp_select_server(char*);
					/* SMTP サーバ選択		*/
extern SMTP_ERR_CODE	smtp_open(void);/* SMTP サーバオープン		*/
extern SMTP_ERR_CODE	smtp_close(void);
					/* SMTP サーバクローズ		*/
extern SMTP_ERR_CODE	smtp_send(char*);
					/* SMTP 送信			*/
extern SMTP_ERR_CODE	smtp_receive(char*, int);
					/* SMTP 受信			*/
extern SMTP_ERR_CODE	smtp_noop(void);/* SMTP 接続確認		*/
extern SMTP_ERR_CODE	smtp_mail(char*, char*, char*);
					/* SMTP 記事送信		*/
extern char		*get_real_adrs(char*, char*);
					/* 実アドレス取得		*/
#else	/* !__STDC__ */
extern SMTP_ERR_CODE	smtp_select_server();
					/* SMTP サーバ選択		*/
extern SMTP_ERR_CODE	smtp_open();	/* SMTP サーバオープン		*/
extern SMTP_ERR_CODE	smtp_close();	/* SMTP サーバクローズ		*/
extern SMTP_ERR_CODE	smtp_send();	/* SMTP 送信			*/
extern SMTP_ERR_CODE	smtp_receive();	/* SMTP 受信			*/
extern SMTP_ERR_CODE	smtp_noop();	/* SMTP 接続確認		*/
extern SMTP_ERR_CODE	smtp_mail();	/* SMTP 記事送信		*/
extern char		*get_real_adrs();
					/* 実アドレス取得		*/
#endif	/* !__STDC__ */
#endif	/* !__SMTPLIB_H__ */
