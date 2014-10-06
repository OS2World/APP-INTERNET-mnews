/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Configuration define
 *  File        : config.h
 *  Version     : 1.21
 *  First Edit  : 1992-08/03
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 ��� ��
 *
 * ����ջ����
 * �����ޥ�����ʣ���ѥ��פǤĤʤ���������Τ� ( ) �ǰϤ�ǲ�������
 * ��MH ����Ѥ��ʤ����� MH ���ޥ�ɥѥ��ϻ��ꤷ�ʤ��Ƥ⹽���ޤ���
 * ���ǥե���ȤΥɥᥤ��̾�ϥХ��ʥ��������������� DOMAIN_FILE ��
 *   �������ե�����˽񤯤褦���ѹ�����ޤ�����
 *
 */

/*
 * �ڡ����㡼���ޥ��
 */

#ifndef	EXT_PAGER
#define	EXT_PAGER	"/usr/local/bin/less"	/* �̾���Ѥ���ʤ� */
#endif	/* !EXT_PAGER */

/*
 * �ǥե����ɽ��������
 */

#ifndef	PRINT_CODE
#define	PRINT_CODE	EUC_CODE	/* EUC,JIS,SJIS or ASCII */
#endif	/* !PRINT_CODE */

/*
 * Ƚ����ǽ���δ���������
 */

#ifndef	DEFAULT_CODE
#define	DEFAULT_CODE	EUC_CODE	/* EUC or SJIS �����̾���ѹ����� */
#endif	/* !DEFAULT_CODE */

/*
 * ���������ִ���������
 */

#ifndef	SAVE_CODE
#define	SAVE_CODE	ASCII_CODE	/* �ǥե���Ȥ�̵�Ѵ� */
#endif	/* !SAVE_CODE */

/*
 * ���ǥ�������������
 */

#ifndef	EDIT_CODE
#define	EDIT_CODE	EUC_CODE	/* EUC,JIS,SJIS or ASCII */
#endif	/* !EDIT_CODE */

/*
 * 1 �����ϴ���������
 */

#ifndef	INPUT_CODE
#define	INPUT_CODE	ASCII_CODE	/* ASCII �ˤ���ȴ���������դ��ʤ� */
#endif	/* !INPUT_CODE */

/*
 * ��������������
 */

#ifndef	LPR_CODE
#define	LPR_CODE	EUC_CODE	/* EUC,JIS,SJIS or ASCII */
#endif	/* !LPR_CODE */

/*
 * ���ǥ������ޥ��
 */

#ifndef	EDITOR_COMMAND
#define	EDITOR_COMMAND	"/usr/local/bin/emacs -nw"
#endif	/* !EDITOR_COMMAND */

/*
 * �������ޥ��
 */

#ifndef	LPR_COMMAND
#define	LPR_COMMAND	"lpr"
#endif	/* !LPR_COMMAND */

/*
 * �᡼���������ޥ��
 */

#ifndef	SEND_COMMAND
#if	defined(_386BSD) || defined(__FreeBSD__) || defined(NET2) || defined(BSD44)
#define	SEND_COMMAND	"/usr/sbin/sendmail -t -om -oi"
#else	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44) */
#define	SEND_COMMAND	"/usr/lib/sendmail -t -om -oi"
#endif	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44) */
#endif	/* !SEND_COMMAND */

/*
 * �˥塼����ƥ��ޥ��
 */

#ifndef	POST_COMMAND
#define	POST_COMMAND	"/usr/lib/news/inews -h"
#endif	/* !POST_COMMAND */

/*
 * MPEG �������ޥ��
 */

#ifndef	MPEG_COMMAND
#define	MPEG_COMMAND	"mpeg_play"
#endif	/* !MPEG_COMMAND */

/*
 * JPEG ɽ�����ޥ��
 */

#ifndef	JPEG_COMMAND
#define	JPEG_COMMAND	"xv"
#endif	/* !JPEG_COMMAND */

/*
 * GIF ɽ�����ޥ��
 */

#ifndef	GIF_COMMAND
#define	GIF_COMMAND	"xv"
#endif	/* !GIF_COMMAND */

/*
 * AUDIO �������ޥ��
 */

#ifndef	AUDIO_COMMAND
#define	AUDIO_COMMAND	"audioplay"
#endif	/* !AUDIO_COMMAND */

/*
 * PostScript ɽ�����ޥ��
 */

#ifndef	PS_COMMAND
#define	PS_COMMAND	"ghostview"
#endif	/* !PS_COMMAND */

/*
 * MH ���ޥ�ɥѥ�
 */

#ifndef	MH_COMMAND_PATH
#define	MH_COMMAND_PATH	"/usr/local/bin/mh"
#endif	/* !MH_COMMAND_PATH */

/*
 * MAIL ���ס���ѥ�
 */

#ifndef	MAIL_SPOOL
#if	defined(_386BSD) || defined(__FreeBSD__) || defined(NET2) || defined(BSD44) || defined(SVR4)
#define	MAIL_SPOOL	"/var/mail"
#else	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44 || SVR4) */
#define	MAIL_SPOOL	"/usr/spool/mail"
#endif	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44 || SVR4) */
#endif	/* !MAIL_SPOOL */

/*
 * BOARD ���ס���ѥ�
 */

#ifndef	BOARD_SPOOL
#if	defined(_386BSD) || defined(__FreeBSD__) || defined(NET2) || defined(BSD44) || defined(SVR4)
#define	BOARD_SPOOL	"/var/board"
#else	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44 || SVR4) */
#define	BOARD_SPOOL	"/usr/spool/board"
#endif	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44 || SVR4) */
#endif	/* !BOARD_SPOOL */

/*
 * ������ե�����
 */

#ifndef	MARK_FILE
#if	defined(MSDOS) && !defined(USE_LONG_FNAME)
#define	MARK_FILE	"_newsrc"
#else	/* !MSDOS || USE_LONG_FNAME */
#define	MARK_FILE	".newsrc"
#endif	/* !MSDOS || USE_LONG_FNAME */
#endif	/* !MARK_FILE */

/*
 * ��ȥǥ��쥯�ȥ�
 */

#ifndef	TMP_DIR
#define	TMP_DIR		"/var/tmp"
#endif	/* !TMP_DIR */

/*
 * ���������֥ǥ��쥯�ȥ�
 */

#ifndef	SAVE_DIR
#define	SAVE_DIR	"~/News"
#endif	/* !SAVE_DIR */

/*
 * �᡼��ܥå����ե�����
 */

#define	MBOX_FILE	"~/mbox"
#define	RMAIL_FILE	"~/RMAIL"

/*
 * ����ե����ե�����
 */

#ifndef	CONFIG_FILE
#define	CONFIG_FILE	".mnews_setup"
#endif	/* !CONFIG_FILE */

/*
 * jnames �ǡ����١���
 */

#ifndef	JNAMES_DB
#define	JNAMES_DB		"/usr/local/lib/jnames"
#endif	/* !JNAMES_DB */

/*
 * Organization �ե����� 1992/10/27, ���@NTT����
 */

#ifndef	ORGAN_FILE
#define	ORGAN_FILE		"/usr/lib/news/organization"
#endif	/* !ORGAN_FILE */

/*
 * �ǥե���Ƚ�����ե�����
 */

#ifndef	DEFAULT_INIT_FILE
#define	DEFAULT_INIT_FILE		"/usr/local/lib/mnews_setup"
#endif	/* !DEFAULT_INIT_FILE */

/*
 * �ɥᥤ��̾���ҥե�����
 */

#ifndef	DOMAIN_FILE
#define	DOMAIN_FILE		"/usr/local/lib/default-domain"
#endif	/* !DOMAIN_FILE */

/*
 * NNTP �����С����ҥե�����
 */

#ifndef	NNTP_SERVER_FILE
#define	NNTP_SERVER_FILE	"/usr/local/lib/default-server"
#endif	/* !NNTP_SERVER_FILE */

/*
 * SMTP �����С����ҥե�����
 */

#ifndef	SMTP_SERVER_FILE
#define	SMTP_SERVER_FILE	"/usr/local/lib/default-smtpsv"
#endif	/* !SMTP_SERVER_FILE */

/*
 * POP3 �����С����ҥե�����
 */

#ifndef	POP3_SERVER_FILE
#define	POP3_SERVER_FILE	"/usr/local/lib/default-pop3sv"
#endif	/* !POP3_SERVER_FILE */

/*
 * ͹�إ����С����ҥե�����
 */

#ifndef	YOUBIN_SERVER_FILE
#define	YOUBIN_SERVER_FILE	"/usr/local/lib/default-mailsv"
#endif	/* !YOUBIN_SERVER_FILE */

/*
 * ǧ�ڥե�����
 */

#ifndef	AUTHORITY_FILE
#ifdef	USE_NEWSAUTH
#define	AUTHORITY_FILE		".newsauth"
#else	/* !USE_NEWSAUTH */
#define	AUTHORITY_FILE		".netrc"
#endif	/* !USE_NEWSAUTH */
#endif	/* !AUTHORITY_FILE */

/*
 * BOARD �ޡ����ե�����
 */

#ifndef	BOARD_MARK_FILE
#define	BOARD_MARK_FILE		".mnews_boardrc"
#endif	/* !BOARD_MARK_FILE */

/*
 * �Ķ��ѿ�
 * (�ü�ʴĶ��ѿ�����Ѥ��������ʳ����ѹ����ʤ��ǲ�����)
 */

#define	HOME_ENV		"HOME"
#define	USER_ENV		"USER"
#define	NAME_ENV		"NAME"
#define	EDITOR_ENV		"EDITOR"
#define	PAGER_ENV		"PAGER"
#define	DOMAIN_ENV		"DOMAINNAME"
#define	OFFICIAL_ENV		"OFFICIALNAME"
#define	ORGANIZATION_ENV	"ORGANIZATION"
#define	NEWSRC_ENV		"NEWSRC"
#define	MNEWSRC_ENV		"MNEWSRC"
#define	DOTDIR_ENV		"DOTDIR"
#ifdef	MSDOS
#ifdef	X68K
#define	TMPDIR_ENV		"temp"
#else	/* !X68K */
#define	TMPDIR_ENV		"TEMP"
#endif	/* !X68K */
#else	/* !MSDOS */
#define	TMPDIR_ENV		"TMPDIR"
#endif	/* !MSDOS */
#define	SAVEDIR_ENV		"SAVEDIR"
#define	MBOX_ENV		"MBOX"
#define	RMAIL_ENV		"RMAIL"
#define	NEWS_SPOOL_ENV		"NEWSSPOOL"
#define	NEWS_LIB_ENV		"NEWSLIB"
#define	MAIL_SPOOL_ENV		"MAILSPOOL"

/*
 * �ǥե��������
 * (�̾�� config.sh (config.jsh) �Ǽ�ư��������Τ��ѹ����ʤ��ǲ�����)
 */

#ifndef	NNTP_MODE
#define	NNTP_MODE		1	/* NNTP ���ѥ⡼��	*/
#endif	/* !NNTP_MODE */

#ifndef	SMTP_MODE
#define	SMTP_MODE		0	/* SMTP ���ѥ⡼��	*/
#endif	/* !SMTP_MODE */

#ifndef	POP3_MODE
#define	POP3_MODE		0	/* POP3 ���ѥ⡼��	*/
#endif	/* !POP3_MODE */

#ifndef	RPOP_MODE
#define	RPOP_MODE		0	/* RPOP ���ѥ⡼��	*/
#endif	/* !RPOP_MODE */

#ifndef	JAPANESE
#define	JAPANESE		1	/* ���ܸ�⡼��		*/
#endif	/* !JAPANESE */

#ifndef	X_NSUBJ_MODE
#define	X_NSUBJ_MODE		0	/* X-Nsubject: �⡼��	*/
#endif	/* !X_NSUBJ_MODE */

#ifndef	INEWS_SIG_MODE
#define	INEWS_SIG_MODE		1	/* inews �� .signature ��
					   ����ˤĤ���⡼��	*/
#endif	/* !INEWS_SIG_MODE */

#ifndef	ASK_DISTRIB_MODE
#define	ASK_DISTRIB_MODE	1	/* Distribution: �ե������
					   ���ϥ⡼��		*/
#endif	/* !ASK_DISTRIB_MODE */

#ifndef	JST_MODE
#define	JST_MODE		1	/* ���ܻ����Ѵ��⡼��	*/
#endif	/* !JST_MODE */

#ifndef	DNAS_MODE
#define	DNAS_MODE		0	/* DNAS �б��⡼��	*/
#endif	/* !DNAS_MODE */

#ifndef	NNTP_AUTH_MODE
#define	NNTP_AUTH_MODE		1	/* NNTP ǧ�ڥ⡼��	*/
#endif	/* !NNTP_AUTH_MODE */

/*
 * �ǥե��������
 * (������ʬ�ϥ桼���ι��ߤˤ��ޥ��ޥ��ʤΤ� config.sh (config.jsh)
 * �Ǥ��ѹ�����ޤ����̾�ϥ桼���� ~/.mnews_setup ���ѹ��Ǥ���
 * �ΤǤ������ѹ�����ɬ�פϤ���ޤ���)
 */


#define	MAX_THREAD_NUMBER	100	/* ���缫ư�����ȿ�	*/
#define	GNUS_MODE		0	/* GNUS �饤���⡼��	*/
#define	GROUP_MASK		1	/* ���롼�ץޥ����⡼��	*/
#define	PAGER_MODE		0	/* �����ڡ����㡼�⡼��	*/
#define	MH_MARK_MODE		0	/* MH �ޡ����⡼��	*/
#define	MH_SELECT_MODE		1	/* MH ��ư���쥯�ȥ⡼��*/
#define	MH_ALIAS_MODE		0	/* MH �����ꥢ���⡼��	*/
#define	QUIET_MODE		0	/* �ٹ�ɽ���⡼��	*/
#define	MAIL_MODE		0	/* �᡼�뻲�����ѥ⡼��	*/
#define	ERROR_MODE		0	/* ���顼���⡼��	*/
#define	CROSS_MARK_MODE		0	/* �����ݥ��Ȼ��ޡ���	*/
#define	MH_MODE			1	/* MH �⡼�ɥե饰	*/
#define	UCBMAIL_MODE		1	/* UCB-mail �⡼�ɥե饰*/
#define	RMAIL_MODE		1	/* RMAIL �⡼�ɥե饰	*/
#define	BOARD_MODE		1	/* BOARD �⡼�ɥե饰	*/
#define	BOARD_COUNT_MODE	0	/* BOARD �������������	*/
#define	ADD_CC_MODE		1	/* Cc: �ղå⡼��	*/
#define	REPLY_CC_MODE		1	/* Cc: ��ץ饤�⡼��	*/
#define	ADD_LINES_MODE		0	/* Lines: �ղå⡼��	*/
#define	WIDE_MODE		0	/* �磻��ɽ���⡼��	*/
#define	BELL_MODE		1	/* �٥벻�⡼��		*/
#define	COLOR_MODE		0	/* ���顼�⡼��		*/
#define	MY_ADRS_MODE		0	/* �����ɥ쥹�Ѵ��⡼��	*/
#define	AUTO_SIG_MODE		0	/* .signature ��ư�ղ�	*/
#define	AUTO_INC_MODE		0	/* ������ư����⡼��	*/
#define	FROM_MODE		0	/* From �񼰥⡼��	*/
#define	UNIXFROM_MODE		0	/* UNIX-From ����������	*/
#define	XOVER_MODE		1	/* XOVER �б��⡼��	*/
#define	SCROLL_MODE		0	/* ��������⡼��	*/
#define	POP3_DEL_MODE		1	/* POP3 ����⡼��	*/
#define	NEWS_ARTICLE_MASK	0	/* �˥塼�������ޥ����⡼��	*/
#define	MAIL_ARTICLE_MASK	0	/* �᡼�뵭���ޥ����⡼��	*/
#define	NEWS_THREAD_MODE	1	/* �˥塼������åɥ⡼��	*/
#define	MAIL_THREAD_MODE	0	/* �᡼�륹��åɥ⡼��		*/

#define	ARTICLE_FORMAT		0	/* ���������ե����ޥå�	*/
#define	MIME_FORMAT		1	/* MIME�ե����ޥåȽ���	*/
#define	MSGCHK_RULE		1	/* �᡼������å���ˡ	*/
#define	SORT_RULE		0	/* ��������ˡ		*/

#define	REPLY_INDICATOR		">> "	/* ���󥸥�����		*/
#define	SEND_FILTER		""	/* �᡼�������ե��륿	*/
#define	POST_FILTER		""	/* �˥塼����ƥե��륿	*/

enum param_type {
  PARAM_SW_TYPE,
  PARAM_CODE_TYPE,
  PARAM_STRING_TYPE,
  PARAM_FORMAT_TYPE,
  PARAM_FILE1_TYPE,
  PARAM_FILE2_TYPE,
  PARAM_SELECT_TYPE,
  PARAM_GROUP1_TYPE,
  PARAM_GROUP2_TYPE,
  PARAM_COLOR_TYPE,
  PARAM_NUMBER_TYPE,
  PARAM_KILL_TYPE
};

/*
 * ���顼������
 */

#ifdef	X68K
#define	TITLE_COLOR_CODE	BG_SKYBLUE2_ATTRIB
#define	HEADER_COLOR_CODE	BG_YELLOW1_ATTRIB
#define	GUIDE_COLOR_CODE	BG_SKYBLUE1_ATTRIB
#define	ERROR_COLOR_CODE	BG_YELLOW2_ATTRIB
#define	CATEGORY_COLOR_CODE	FG_SKYBLUE1_ATTRIB
#define	MARK_COLOR_CODE		FG_YELLOW2_ATTRIB
#define	NUMBER_COLOR_CODE	FG_WHITE1_ATTRIB
#define	FROM_COLOR_CODE		FG_WHITE1_ATTRIB
#define	DATE_COLOR_CODE		FG_WHITE1_ATTRIB
#define	LINE_COLOR_CODE		FG_WHITE1_ATTRIB
#define	SUBJECT_COLOR_CODE	FG_SKYBLUE2_ATTRIB
#define	THREAD_COLOR_CODE	FG_SKYBLUE1_ATTRIB
#define	LOST_COLOR_CODE		FG_YELLOW2_ATTRIB
#define	FROM2_COLOR_CODE	FG_YELLOW2_ATTRIB
#define	TO_COLOR_CODE		FG_YELLOW2_ATTRIB
#define	CC_COLOR_CODE		FG_YELLOW1_ATTRIB
#define	DATE2_COLOR_CODE	FG_SKYBLUE1_ATTRIB
#define	GROUP_COLOR_CODE	FG_YELLOW2_ATTRIB
#define	REFER_COLOR_CODE	FG_SKYBLUE1_ATTRIB
#define	JNAMES_COLOR_CODE	FG_YELLOW1_ATTRIB
#else	/* !X68K */
#define	TITLE_COLOR_CODE	BG_BLUE_ATTRIB
#define	HEADER_COLOR_CODE	BG_RED_ATTRIB
#define	GUIDE_COLOR_CODE	BG_BLUE_ATTRIB
#define	ERROR_COLOR_CODE	BG_RED_ATTRIB
#define	CATEGORY_COLOR_CODE	FG_SKYBLUE_ATTRIB
#define	MARK_COLOR_CODE		FG_RED_ATTRIB
#define	NUMBER_COLOR_CODE	FG_WHITE_ATTRIB
#define	FROM_COLOR_CODE		FG_WHITE_ATTRIB
#define	DATE_COLOR_CODE		FG_WHITE_ATTRIB
#define	LINE_COLOR_CODE		FG_WHITE_ATTRIB
#define	SUBJECT_COLOR_CODE	FG_SKYBLUE_ATTRIB
#define	THREAD_COLOR_CODE	FG_BLUE_ATTRIB
#define	LOST_COLOR_CODE		FG_RED_ATTRIB
#define	FROM2_COLOR_CODE	FG_PURPLE_ATTRIB
#define	TO_COLOR_CODE		FG_GREEN_ATTRIB
#define	CC_COLOR_CODE		FG_GREEN_ATTRIB
#define	DATE2_COLOR_CODE	FG_YELLOW_ATTRIB
#define	GROUP_COLOR_CODE	FG_GREEN_ATTRIB
#define	REFER_COLOR_CODE	FG_GREEN_ATTRIB
#define	JNAMES_COLOR_CODE	FG_GREEN_ATTRIB
#endif	/* !X68K */
