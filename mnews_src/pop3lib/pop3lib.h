/*
 *
 *  POP3 ライブラリ
 *
 *  Copyright 1995-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : POP3 protocol library
 *  Sub system  : POP3 define
 *  File        : pop3lib.h
 *  Version     : 0.86
 *  First Edit  : 1995-12/25
 *  Last  Edit  : 1997-12/21
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	__POP3LIB_H__
#define	__POP3LIB_H__

#include	"tcplib.h"

#ifndef	POP3_SERVER_FILE
#define	POP3_SERVER_FILE	"/usr/local/lib/default-pop3sv"
#endif	/* !POP3_SERVER_FILE */

#define	MSPL_SEPARATER		"From "

/*
 * メール着信環境
 */

#ifndef	POP2_PORT
#define	POP2_PORT		109
#endif	/* !POP2_PORT */
#ifndef	POP3_PORT
#define	POP3_PORT		110
#endif	/* !POP3_PORT */
#ifndef	MAIL_SPOOL
#define	MAIL_SPOOL		"/usr/spool/mail"
#endif	/* !MAIL_SPOOL */

/*
 * POP3 コマンド
 */

#define	POP1_USER_CMD		"USER"	/* USER user		*/
#define	POP1_PASS_CMD		"PASS"	/* PASS password	*/
#define	POP1_RETR_CMD		"RETR"	/* RETR mail-box	*/
#define	POP1_RDEL_CMD		"RDEL"	/* RDEL mail-box	*/
#define	POP1_RCEV_CMD		"RCEV"	/* RCEV			*/
#define	POP1_RCVD_CMD		"RCVD"	/* RCVD			*/
#define	POP1_QUIT_CMD		"QUIT"	/* QUIT			*/
#define	POP1_NOOP_CMD		"NOOP"	/* NOOP			*/
#define	POP1_RSET_CMD		"RSET"	/* RSET			*/

#define	POP2_HELO_CMD		"HELO"	/* HELO user password	*/
#define	POP2_FOLD_CMD		"FOLD"	/* FOLD mail-box	*/
#define	POP2_READ_CMD		"READ"	/* READ [n]		*/
#define	POP2_RETR_CMD		"RETR"	/* RETR			*/
#define	POP2_ACKS_CMD		"ACKS"	/* ACKS			*/
#define	POP2_ACKD_CMD		"ACKD"	/* ACKD			*/
#define	POP2_NACK_CMD		"NACK"	/* NACK			*/
#define	POP2_QUIT_CMD		"QUIT"	/* QUIT			*/

#define	POP3_USER_CMD		"USER"	/* USER user		*/
#define	POP3_PASS_CMD		"PASS"	/* PASS password	*/
#define	POP3_QUIT_CMD		"QUIT"	/* QUIT			*/
#define	POP3_STAT_CMD		"STAT"	/* STAT			*/
#define	POP3_LIST_CMD		"LIST"	/* LIST [msg]		*/
#define	POP3_RETR_CMD		"RETR"	/* RETR msg		*/
#define	POP3_DELE_CMD		"DELE"	/* DELE msg		*/
#define	POP3_NOOP_CMD		"NOOP"	/* NOOP			*/
#define	POP3_LAST_CMD		"LAST"	/* LAST			*/
#define	POP3_RSET_CMD		"RSET"	/* RSET			*/
#define	POP3_RPOP_CMD		"RPOP"	/* RPOP user	   (opt)*/
#define	POP3_APOP_CMD		"APOP"	/* APOP user digest(opt)*/
#define	POP3_UIDL_CMD		"UIDL"	/* UIDL [msg]	   (opt)*/
#define	POP3_TOP_CMD		"TOP"	/* TOP msg n	   (opt)*/

/*
 * POP3 レスポンス
 */

#define	POP1_OKAY_RES		"+OK"
#define	POP1_ERROR_RES		"-ERR"
#define	POP2_OKAY_RES		"+OK"
#define	POP2_ERROR_RES		"-ERR"
#define	POP3_OKAY_RES		"+OK"
#define	POP3_ERROR_RES		"-ERR"

/*
 * POP3 リードレスポンス
 */

typedef enum pop3_rd_stat {
  POP3_RD_FROM,				/* UNIX-From を検出	*/
  POP3_RD_HEAD,				/* ヘッダを検出		*/
  POP3_RD_SEP,				/* セパレータを検出	*/
  POP3_RD_BODY,				/* 本文を検出		*/
  POP3_RD_END,				/* 本文を最後を検出	*/
  POP3_RD_EOF				/* 全記事の最後を検出	*/
} POP3_RD_STAT;

/*
 * POP3 関数エラーコード
 */

typedef enum pop3_err_code {
  POP3_OK = 0,				/* 正常終了		*/
  POP3_ERR_RSV,				/* 未サポート		*/
  POP3_ERR_BUSY,			/* 過負荷状態		*/
  POP3_ERR_COMM,			/* 低レベル通信エラー	*/
  POP3_ERR_PROT,			/* プロトコルエラー	*/
  POP3_ERR_AUTH,			/* 認証エラー		*/
  POP3_ERR_PROG,			/* プログラムエラー	*/
  POP3_ERR_MEM,				/* メモリ不足エラー	*/
  POP3_ERR_FILE,			/* ファイルエラー	*/
  POP3_ERR_NONE				/* 未存在エラー		*/
} POP3_ERR_CODE;

extern char		pop3_server[];	/* POP3 サーバホスト名(FQDN)	*/
extern char		pop3_server_file[];
					/* POP3 サーバ指定ファイル	*/
extern char		mail_spool[];	/* メールスプール		*/
extern short		pop3_connect_mode;
					/* POP3 接続モード		*/
					/* (0:MSPL,1:POP3,2:DEAD)	*/
extern short		pop3_rpop_mode;	/* POP3 RPOP 使用モード		*/
extern short		pop3_from_mode;	/* POP3 From 復元モード		*/
extern char		*pop3_connect_string[];
					/* POP3 接続モード文字列	*/

#ifdef	__STDC__
extern POP3_ERR_CODE	pop3_select_server(char*);
					/* POP3 サーバ選択		*/
extern POP3_ERR_CODE	pop3_open(void);/* POP3 サーバオープン		*/
extern POP3_ERR_CODE	pop3_close(void);
					/* POP3 サーバクローズ		*/
extern POP3_ERR_CODE	pop3_send(char*);
					/* POP3 送信			*/
extern POP3_ERR_CODE	pop3_receive(char*, int);
					/* POP3 受信			*/
extern POP3_ERR_CODE	pop3_read(char*, int, POP3_RD_STAT*);
					/* POP3 スプールリード		*/
extern POP3_ERR_CODE	pop3_stat(int);	/* POP3 スプールチェック	*/
extern POP3_ERR_CODE	pop3_delete(void);
					/* POP3 スプール削除		*/
extern POP3_ERR_CODE	pop3_noop(void);/* POP3 接続確認		*/
#else	/* !__STDC__ */
extern POP3_ERR_CODE	pop3_select_server();
					/* POP3 サーバ選択		*/
extern POP3_ERR_CODE	pop3_open();	/* POP3 サーバオープン		*/
extern POP3_ERR_CODE	pop3_close();	/* POP3 サーバクローズ		*/
extern POP3_ERR_CODE	pop3_send();	/* POP3 送信			*/
extern POP3_ERR_CODE	pop3_receive();	/* POP3 受信			*/
extern POP3_ERR_CODE	pop3_read();	/* POP3 スプールリード		*/
extern POP3_ERR_CODE	pop3_stat();	/* POP3 スプールチェック	*/
extern POP3_ERR_CODE	pop3_delete();	/* POP3 スプール削除		*/
extern POP3_ERR_CODE	pop3_noop();	/* POP3 接続確認		*/
#endif	/* !__STDC__ */
#endif	/* !__POP3LIB_H__ */
