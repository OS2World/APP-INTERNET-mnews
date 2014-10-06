/*
 *
 *  NNTP ライブラリ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : NNTP protocol library
 *  Sub system  : NNTP define
 *  File        : nntplib.h
 *  Version     : 2.46
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	__NNTPLIB_H__
#define	__NNTPLIB_H__

#include	"tcplib.h"

#ifdef	MNEWS
#define	NG_ALLOC
#define	NG_SORT
#endif	/* MNEWS */

#ifdef	NG_ALLOC
#define	NG_ALLOC_COUNT	256
#define	NAME_ALLOC_SIZE	1024
#else	/* !NG_ALLOC */
#define	MAX_NEWS_GROUP	2048
#endif	/* !NG_ALLOC */

#define	XOVER_BUFF	4096

#define	MAX_GROUP_NAME	128
#define	NUMBER_BUFF	12
#ifndef	NNTP_SERVER_FILE
#define	NNTP_SERVER_FILE	"/usr/local/lib/default-server"
#endif	/* !NNTP_SERVER_FILE */
#ifndef	POST_PROGRAM
#define	POST_PROGRAM		"/usr/lib/news/inews -h"
#endif	/* !POST_PROGRAM */

#define	INN_STRING		" INN "

/*
 * ニュース環境
 */

#define	NNTP_PORT		119
#ifndef	NEWS_SPOOL
#define	NEWS_SPOOL		"/usr/spool/news"
#endif	/* !NEWS_SPOOL */
#ifndef	NEWS_LIB
#define	NEWS_LIB		"/usr/lib/news"
#endif	/* !NEWS_LIB */
#ifndef	NEWS_ACTIVE_FILE
#define	NEWS_ACTIVE_FILE	"active"
#endif	/* !NEWS_ACTIVE_FILE */
#ifndef	NEWS_DESCRIPTION_FILE
#define	NEWS_DESCRIPTION_FILE	"newsgroups"
#endif	/* !NEWS_DESCRIPTION_FILE */

/*
 * NNTP コマンド
 */

#define	NNTP_QUIT_CMD		"QUIT"		/* QUIT		*/
#define	NNTP_HELP_CMD		"HELP"		/* HELP		*/
#define	NNTP_IHAVE_CMD		"IHAVE"		/* IHAVE <msg>	*/
#define	NNTP_NEWNG_CMD		"NEWGROUPS"
			/* NEWGROUPS date time [GMT] [<dist>]	*/
#define	NNTP_NEWNEWS_CMD	"NEWNEWS"
		/* NEWNEWS group date time [GMT] [<dist>]	*/
#define	NNTP_SLAVE_CMD		"SLAVE"		/* SLAVE	*/
#define	NNTP_LIST_CMD		"LIST"		/* LIST		*/
#define	NNTP_GROUP_CMD		"GROUP"		/* GROUP group	*/
#define	NNTP_ARTICLE_CMD	"ARTICLE"	/* ARTICLE [n]	*/
#define	NNTP_STAT_CMD		"STAT"		/* STAT [n]	*/
#define	NNTP_HEAD_CMD		"HEAD"		/* HEAD [n]	*/
#define	NNTP_BODY_CMD		"BODY"		/* BODY [n]	*/
#define	NNTP_NEXT_CMD		"NEXT"		/* NEXT		*/
#define	NNTP_LAST_CMD		"LAST"		/* LAST		*/
#define	NNTP_POST_CMD		"POST"		/* POST		*/
#define	NNTP_USER_CMD		"AUTHINFO USER"
				/* AUTHINFO USER user (INN)	*/
#define	NNTP_PASS_CMD		"AUTHINFO PASS"	/* INN only	*/
				/* AUTHINFO PASS password (INN)	*/
#define	NNTP_DESCRIPTION_CMD	"LIST newsgroups"
					/* LIST newsgroups	*/
#define	NNTP_READER_CMD		"MODE reader"
					/* MODE reader (INN) 	*/
#define	NNTP_DATE_CMD		"DATE"		/* DATE		*/
#ifdef	XOVER
#define	NNTP_XOVER_CMD		"XOVER"		/* XOVER	*/
#endif	/* XOVER */

/*
 * NNTP レスポンス
 */

#define	NNTP_HELP_RES		100	/* 100 help text		*/
#define	NNTP_DATE_RES		111	/* 111 YYYYMMDDhhmmss		*/
#define	NNTP_THROUGH_RES	190	/* 190 through			*/
#define	NNTP_DEBUG_RES		199	/* 199 debug output		*/
#define	NNTP_OPEN_RES1		200	/* 200 H NNTP[auth] version	*/
#define	NNTP_OPEN_RES2		201	/* 201 H NNTP[auth] version	*/
#define	NNTP_NOTE_RES		202	/* 202 slave status noted	*/
#define	NNTP_QUIT_RES		205	/* 205 H closing connection	*/
#define	NNTP_GROUP_RES		211	/* 211 N F L S group selected	*/
#define	NNTP_LIST_RES		215	/* 215 list of newsgroup follows */
#define	NNTP_ARTICLE_RES	220	/* 220 N A M article retrieved	*/
#define	NNTP_HEAD_RES		221	/* 221 N A M article retrieved	*/
#define	NNTP_BODY_RES		222	/* 222 N A M article retrieved	*/
#define	NNTP_NEXT_RES		223	/* 223 N A article retrieved	*/
#define	NNTP_LAST_RES		223	/* 223 N A article retrieved	*/
#ifdef	XOVER
#define	NNTP_XOVER_RES		224	/* 224 data follows		*/
#endif	/* XOVER */
#define	NNTP_NEW_ARTICLE_RES	230	/* 230 list of new articles	*/
#define	NNTP_TRANS_RES1		235	/* 235 artcile transferrred ok	*/
#define	NNTP_POST_RES2		240	/* 240 article post ok		*/
#define	NNTP_AUTHDONE_RES	281	/* 281 Authentication accepted	*/
#define	NNTP_TRANS_RES2		335	/* 335 send article to be	*/
					/*     transferred		*/
#define	NNTP_POST_RES1		340	/* 340 send article to be posted */
#define	NNTP_PASS_RES		381	/* 381 PASS required		*/
#define	NNTP_NO_SERVICE_RES	400	/* 400 service discontinued	*/
#define	NNTP_NO_GROUP_RES	411	/* 411 no such news group	*/
#define	NNTP_NO_NEWNG_RES	412	/* 412 no newsgroup		*/
#define	NNTP_NO_CUR_RES		420	/* 420 no current article	*/
#define	NNTP_NO_NEXT_RES	421	/* 421 no next article		*/
#define	NNTP_NO_PREV_RES	422	/* 422 no previous article	*/
#define	NNTP_NO_NUM_RES		423	/* 423 no such article number	*/
#define	NNTP_NO_FOUND_RES	430	/* 430 no such article found	*/
#define	NNTP_NO_WANT_RES	435	/* 435 article not wanted	*/
#define	NNTP_TRANS_FAIL_RES	436	/* 436 transfer failed		*/
#define	NNTP_REJECT_RES		437	/* 437 article rejected		*/
#define	NNTP_NO_ALLOW_RES	440	/* 440 posting not allowed	*/
#define	NNTP_POST_FAIL_RES	441	/* 440 posting failed		*/
#define	NNTP_AUTHCOMP_RES	482	/* 482 Authorization already	*/
					/*     completed		*/
#define	NNTP_NO_CMD_RES		500	/* 500 bad command		*/
#define	NNTP_SYNTAX_RES		501	/* 501 command syntax error	*/
#define	NNTP_AUTHERR_RES	502	/* 502 Authentication error	*/
#define	NNTP_FAULT_RES		503	/* 503 program fault		*/

#define	CANCEL_CONTROL		"cancel"

/*
 * ニュースグループセパレータ
 */

#define	NEWS_GROUP_SEPARATER	'.'

enum group_mode {
  POST_ENABLE,
  POST_DISABLE,
  POST_MODERATED,
  POST_UNKNOWN
};

/*
 * NNTP 関数エラーコード
 */

typedef enum nntp_err_code {
  NNTP_OK = 0,				/* 正常終了		*/
  NNTP_ERR_RSV,				/* 未サポート		*/
  NNTP_ERR_SERV,			/* サービス中止エラー	*/
  NNTP_ERR_COMM,			/* 低レベル通信エラー	*/
  NNTP_ERR_PROT,			/* プロトコルエラー	*/
  NNTP_ERR_AUTH,			/* 認証エラー		*/
  NNTP_ERR_PROG,			/* プログラムエラー	*/
  NNTP_ERR_MEM,				/* メモリ不足エラー	*/
  NNTP_ERR_FILE,			/* ファイルエラー	*/
  NNTP_ERR_NONE				/* 未存在エラー		*/
} NNTP_ERR_CODE;

/*
 * グループ名はソートで使用するので最初のメンバであること
 */

#ifdef	NG_ALLOC
struct news_group {
  char		*group_name;
  int		min_article;
  int		max_article;
  short		group_mode;
};
#else	/* !NG_ALLOC */
struct news_group {
  char		group_name[MAX_GROUP_NAME];
  int		min_article;
  int		max_article;
  short		group_mode;
};
#endif	/* !NG_ALLOC */

extern short		ignore_mode;	/* ニュースグループ無視モード	*/
extern short		nntp_connect_mode;
					/* NNTP 接続モード		*/
					/* (0:NSPL,1:NNTP,2:DEAD)	*/
extern short		xover_mode;	/* XOVER 有効スイッチ		*/
extern short		post_enable;	/* 投稿許可フラグ		*/
extern short		force_use_inews;/* 強制 inews 使用フラグ	*/
extern int		group_number;	/* グループ数			*/
#ifdef	NG_ALLOC
extern long		name_used_size;	/* ニュースグループ名使用サイズ	*/
extern struct news_group *news_group;	/* ニュースグループ情報		*/
extern char		*name_pool;	/* ニュースグループ名保存空間	*/
#else	/* !NG_ALLOC */
extern struct news_group news_group[];	/* ニュースグループ情報		*/
#endif	/* !NG_ALLOC */
extern char		nntp_server[];	/* NNTP サーバホスト名(FQDN)	*/
extern char		nntp_server_file[];
					/* NNTP サーバ指定ファイル	*/
extern char		post_program[];	/* 投稿プログラム		*/
extern char		news_spool[];	/* ニューススプール		*/
extern char		news_lib[];	/* ニュースライブラリ		*/
extern char		ignore_groups[];/* 無視するニュースグループ群	*/
extern char		*nntp_connect_string[];
					/* NNTP 接続モード文字列	*/
#ifdef	NNTP_AUTH
extern char		nntp_user[];    /* NNTP 認証名			*/
extern short		nntp_auth_mode;	/* NNTP 認証モード		*/
extern short		nntp_get_pass;	/* NNTP パスワード入力フラグ	*/
#endif	/* NNTP_AUTH */

#ifdef	__STDC__
extern NNTP_ERR_CODE	nntp_select_server(char*);
					/* NNTP サーバ選択		*/
extern NNTP_ERR_CODE	nntp_open(void);/* NNTP サーバオープン		*/
extern NNTP_ERR_CODE	nntp_close(void);
					/* NNTP サーバクローズ		*/
extern NNTP_ERR_CODE	nntp_send(char*);
					/* NNTP 送信			*/
extern NNTP_ERR_CODE	nntp_receive(char*, int);
					/* NNTP 受信			*/
extern NNTP_ERR_CODE	nntp_list(void);/* NNTP グループ一覧取得	*/
#ifdef	NG_ALLOC
extern NNTP_ERR_CODE	nntp_free(void);
					/* NNTP グループ情報解放	*/
#endif	/* NG_ALLOC */
extern NNTP_ERR_CODE	nntp_group(char*);
					/* NNTP グループ選択		*/
extern NNTP_ERR_CODE	nntp_article(int, FILE*);
					/* NNTP 記事取得		*/
extern NNTP_ERR_CODE	nntp_article2(CASTPTR, FILE*);
					/* NNTP 記事取得		*/
extern NNTP_ERR_CODE	nntp_head(int, FILE*);
					/* NNTP ヘッダ取得		*/
extern NNTP_ERR_CODE	nntp_head2(char*, FILE*);
					/* NNTP ヘッダ取得		*/
extern NNTP_ERR_CODE	nntp_body(int, FILE*);
					/* NNTP 本文取得		*/
extern NNTP_ERR_CODE	nntp_body2(char*, FILE*);
					/* NNTP 本文取得		*/
extern NNTP_ERR_CODE	nntp_copy_fields(int, struct cpy_hdr*, int, int);
					/* NNTP フィールド取得		*/
extern NNTP_ERR_CODE	nntp_copy_fields2(char*, struct cpy_hdr*, int, int);
					/* NNTP フィールド取得		*/
extern NNTP_ERR_CODE	nntp_post(char*, char*);
					/* NNTP 記事投稿		*/
extern NNTP_ERR_CODE	nntp_noop(void);/* NNTP 接続確認		*/
extern NNTP_ERR_CODE	nntp_description(char*, char*);
					/* NNTP 憲章取得		*/
#ifdef	XOVER
extern NNTP_ERR_CODE	nntp_xover(int, int, int);
					/* NNTP ヘッダ情報取得(XOVER)	*/
#endif	/* XOVER */
extern void		confirm_article(char*, int*, int*);
					/* 記事数確認			*/
#else	/* !__STDC__ */
extern NNTP_ERR_CODE	nntp_select_server();
					/* NNTP サーバ選択		*/
extern NNTP_ERR_CODE	nntp_open();	/* NNTP サーバオープン		*/
extern NNTP_ERR_CODE	nntp_close();	/* NNTP サーバクローズ		*/
extern NNTP_ERR_CODE	nntp_send();	/* NNTP 送信			*/
extern NNTP_ERR_CODE	nntp_receive();	/* NNTP 受信			*/
extern NNTP_ERR_CODE	nntp_list();	/* NNTP グループ一覧取得	*/
#ifdef	NG_ALLOC
extern NNTP_ERR_CODE	nntp_free();	/* NNTP グループ情報解放	*/
#endif	/* NG_ALLOC */
extern NNTP_ERR_CODE	nntp_group();	/* NNTP グループ選択		*/
extern NNTP_ERR_CODE	nntp_article();	/* NNTP 記事取得		*/
extern NNTP_ERR_CODE	nntp_article2();/* NNTP 記事取得		*/
extern NNTP_ERR_CODE	nntp_head();	/* NNTP ヘッダ取得		*/
extern NNTP_ERR_CODE	nntp_head2();	/* NNTP ヘッダ取得		*/
extern NNTP_ERR_CODE	nntp_body();	/* NNTP 本文取得		*/
extern NNTP_ERR_CODE	nntp_body2();	/* NNTP 本文取得		*/
extern NNTP_ERR_CODE	nntp_copy_fields();
					/* NNTP フィールド取得		*/
extern NNTP_ERR_CODE	nntp_copy_fields2();
					/* NNTP フィールド取得		*/
extern NNTP_ERR_CODE	nntp_post();	/* NNTP 記事投稿		*/
extern NNTP_ERR_CODE	nntp_noop();	/* NNTP 接続確認		*/
extern NNTP_ERR_CODE	nntp_description();
					/* NNTP 憲章取得		*/
#ifdef	XOVER
extern NNTP_ERR_CODE	nntp_xover();	/* NNTP ヘッダ情報取得(XOVER)	*/
#endif	/* XOVER */
extern void		confirm_article();
					/* 記事数確認			*/
#endif	/* !__STDC__ */
#endif	/* !__NNTPLIB_H__ */
