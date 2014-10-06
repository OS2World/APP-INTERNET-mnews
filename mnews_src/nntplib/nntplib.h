/*
 *
 *  NNTP �饤�֥��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : NNTP protocol library
 *  Sub system  : NNTP define
 *  File        : nntplib.h
 *  Version     : 2.46
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 ��� ��
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
 * �˥塼���Ķ�
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
 * NNTP ���ޥ��
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
 * NNTP �쥹�ݥ�
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
 * �˥塼�����롼�ץ��ѥ졼��
 */

#define	NEWS_GROUP_SEPARATER	'.'

enum group_mode {
  POST_ENABLE,
  POST_DISABLE,
  POST_MODERATED,
  POST_UNKNOWN
};

/*
 * NNTP �ؿ����顼������
 */

typedef enum nntp_err_code {
  NNTP_OK = 0,				/* ���ｪλ		*/
  NNTP_ERR_RSV,				/* ̤���ݡ���		*/
  NNTP_ERR_SERV,			/* �����ӥ���ߥ��顼	*/
  NNTP_ERR_COMM,			/* ���٥��̿����顼	*/
  NNTP_ERR_PROT,			/* �ץ�ȥ��륨�顼	*/
  NNTP_ERR_AUTH,			/* ǧ�ڥ��顼		*/
  NNTP_ERR_PROG,			/* �ץ���२�顼	*/
  NNTP_ERR_MEM,				/* ������­���顼	*/
  NNTP_ERR_FILE,			/* �ե����륨�顼	*/
  NNTP_ERR_NONE				/* ̤¸�ߥ��顼		*/
} NNTP_ERR_CODE;

/*
 * ���롼��̾�ϥ����Ȥǻ��Ѥ���ΤǺǽ�Υ��ФǤ��뤳��
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

extern short		ignore_mode;	/* �˥塼�����롼��̵��⡼��	*/
extern short		nntp_connect_mode;
					/* NNTP ��³�⡼��		*/
					/* (0:NSPL,1:NNTP,2:DEAD)	*/
extern short		xover_mode;	/* XOVER ͭ�������å�		*/
extern short		post_enable;	/* ��Ƶ��ĥե饰		*/
extern short		force_use_inews;/* ���� inews ���ѥե饰	*/
extern int		group_number;	/* ���롼�׿�			*/
#ifdef	NG_ALLOC
extern long		name_used_size;	/* �˥塼�����롼��̾���ѥ�����	*/
extern struct news_group *news_group;	/* �˥塼�����롼�׾���		*/
extern char		*name_pool;	/* �˥塼�����롼��̾��¸����	*/
#else	/* !NG_ALLOC */
extern struct news_group news_group[];	/* �˥塼�����롼�׾���		*/
#endif	/* !NG_ALLOC */
extern char		nntp_server[];	/* NNTP �����Хۥ���̾(FQDN)	*/
extern char		nntp_server_file[];
					/* NNTP �����л���ե�����	*/
extern char		post_program[];	/* ��ƥץ����		*/
extern char		news_spool[];	/* �˥塼�����ס���		*/
extern char		news_lib[];	/* �˥塼���饤�֥��		*/
extern char		ignore_groups[];/* ̵�뤹��˥塼�����롼�׷�	*/
extern char		*nntp_connect_string[];
					/* NNTP ��³�⡼��ʸ����	*/
#ifdef	NNTP_AUTH
extern char		nntp_user[];    /* NNTP ǧ��̾			*/
extern short		nntp_auth_mode;	/* NNTP ǧ�ڥ⡼��		*/
extern short		nntp_get_pass;	/* NNTP �ѥ�������ϥե饰	*/
#endif	/* NNTP_AUTH */

#ifdef	__STDC__
extern NNTP_ERR_CODE	nntp_select_server(char*);
					/* NNTP ����������		*/
extern NNTP_ERR_CODE	nntp_open(void);/* NNTP �����Х����ץ�		*/
extern NNTP_ERR_CODE	nntp_close(void);
					/* NNTP �����Х�����		*/
extern NNTP_ERR_CODE	nntp_send(char*);
					/* NNTP ����			*/
extern NNTP_ERR_CODE	nntp_receive(char*, int);
					/* NNTP ����			*/
extern NNTP_ERR_CODE	nntp_list(void);/* NNTP ���롼�װ�������	*/
#ifdef	NG_ALLOC
extern NNTP_ERR_CODE	nntp_free(void);
					/* NNTP ���롼�׾������	*/
#endif	/* NG_ALLOC */
extern NNTP_ERR_CODE	nntp_group(char*);
					/* NNTP ���롼������		*/
extern NNTP_ERR_CODE	nntp_article(int, FILE*);
					/* NNTP ��������		*/
extern NNTP_ERR_CODE	nntp_article2(CASTPTR, FILE*);
					/* NNTP ��������		*/
extern NNTP_ERR_CODE	nntp_head(int, FILE*);
					/* NNTP �إå�����		*/
extern NNTP_ERR_CODE	nntp_head2(char*, FILE*);
					/* NNTP �إå�����		*/
extern NNTP_ERR_CODE	nntp_body(int, FILE*);
					/* NNTP ��ʸ����		*/
extern NNTP_ERR_CODE	nntp_body2(char*, FILE*);
					/* NNTP ��ʸ����		*/
extern NNTP_ERR_CODE	nntp_copy_fields(int, struct cpy_hdr*, int, int);
					/* NNTP �ե�����ɼ���		*/
extern NNTP_ERR_CODE	nntp_copy_fields2(char*, struct cpy_hdr*, int, int);
					/* NNTP �ե�����ɼ���		*/
extern NNTP_ERR_CODE	nntp_post(char*, char*);
					/* NNTP �������		*/
extern NNTP_ERR_CODE	nntp_noop(void);/* NNTP ��³��ǧ		*/
extern NNTP_ERR_CODE	nntp_description(char*, char*);
					/* NNTP ���ϼ���		*/
#ifdef	XOVER
extern NNTP_ERR_CODE	nntp_xover(int, int, int);
					/* NNTP �إå��������(XOVER)	*/
#endif	/* XOVER */
extern void		confirm_article(char*, int*, int*);
					/* ��������ǧ			*/
#else	/* !__STDC__ */
extern NNTP_ERR_CODE	nntp_select_server();
					/* NNTP ����������		*/
extern NNTP_ERR_CODE	nntp_open();	/* NNTP �����Х����ץ�		*/
extern NNTP_ERR_CODE	nntp_close();	/* NNTP �����Х�����		*/
extern NNTP_ERR_CODE	nntp_send();	/* NNTP ����			*/
extern NNTP_ERR_CODE	nntp_receive();	/* NNTP ����			*/
extern NNTP_ERR_CODE	nntp_list();	/* NNTP ���롼�װ�������	*/
#ifdef	NG_ALLOC
extern NNTP_ERR_CODE	nntp_free();	/* NNTP ���롼�׾������	*/
#endif	/* NG_ALLOC */
extern NNTP_ERR_CODE	nntp_group();	/* NNTP ���롼������		*/
extern NNTP_ERR_CODE	nntp_article();	/* NNTP ��������		*/
extern NNTP_ERR_CODE	nntp_article2();/* NNTP ��������		*/
extern NNTP_ERR_CODE	nntp_head();	/* NNTP �إå�����		*/
extern NNTP_ERR_CODE	nntp_head2();	/* NNTP �إå�����		*/
extern NNTP_ERR_CODE	nntp_body();	/* NNTP ��ʸ����		*/
extern NNTP_ERR_CODE	nntp_body2();	/* NNTP ��ʸ����		*/
extern NNTP_ERR_CODE	nntp_copy_fields();
					/* NNTP �ե�����ɼ���		*/
extern NNTP_ERR_CODE	nntp_copy_fields2();
					/* NNTP �ե�����ɼ���		*/
extern NNTP_ERR_CODE	nntp_post();	/* NNTP �������		*/
extern NNTP_ERR_CODE	nntp_noop();	/* NNTP ��³��ǧ		*/
extern NNTP_ERR_CODE	nntp_description();
					/* NNTP ���ϼ���		*/
#ifdef	XOVER
extern NNTP_ERR_CODE	nntp_xover();	/* NNTP �إå��������(XOVER)	*/
#endif	/* XOVER */
extern void		confirm_article();
					/* ��������ǧ			*/
#endif	/* !__STDC__ */
#endif	/* !__NNTPLIB_H__ */
