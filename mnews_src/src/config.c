/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Configuration routine
 *  File        : config.c
 *  Version     : 1.21
 *  First Edit  : 1992-08/03
 *  Last  Edit  : 1997-12/12
 *  Author      : MSR24 ��� ��
 *
 */

#ifdef	MIME
#define	MIME_MULTIMEDIA
#endif	/* MIME */

/*	����������		*/

typedef enum {
  ASCII_CODE,
  JIS_CODE,
  SJIS_CODE,
  EUC_CODE,
  UNKNOWN_CODE
} KANJICODE;

#ifndef	BUFF_SIZE
#define	BUFF_SIZE	1024
#endif	/* !BUFF_SIZE */
#ifndef	SMALL_BUFF
#define	SMALL_BUFF	256
#endif	/* !SMALL_BUFF */
#ifndef	MINIMUM_BUFF
#define	MINIMUM_BUFF	64
#endif	/* !MINIMUM_BUFF */
#ifndef	MESSAGE_FIELD
#define	MESSAGE_FIELD	"Message-ID:"
#endif	/* !MESSAGE_FIELD */

#include	"compat.h"
#include	"termlib.h"
#include	"site_dep.h"
#if	!(defined(MSDOS) || defined(OS2))
#include	"inst_inf.h"
#endif	/* !(MSDOS || OS2) */
#include	"config.h"

#ifndef	MSDOS
extern struct passwd	*passwd;	/* �ѥ���ɥ���ȥ�		*/
#endif	/* !MSDOS */
extern KANJICODE	print_code;	/* ɽ������������		*/
#ifdef	NSPL
extern char		news_spool[];	/* �˥塼�����ס���		*/
extern char		news_lib[];	/* �˥塼���饤�֥��		*/
#endif	/* NSPL */
#ifdef	MSPL
extern char		mail_spool[];	/* MAIL ���ס���ѥ�		*/
#endif	/* MSPL */
extern char		nntp_server[];	/* NNTP ������̾		*/
extern char		nntp_server_file[];
					/* NNTP �����л���ե�����	*/
extern char		smtp_server_file[];
					/* SMTP �����л���ե�����	*/
extern char		pop3_server_file[];
					/* POP3 �����л���ե�����	*/
extern char		post_program[];	/* ��ƥץ����		*/
extern char		send_program[];	/* �����ץ����		*/
extern short		force_use_inews;/* inews ��������		*/
#ifdef	      NNTP_AUTH
extern char		nntp_user[];    /* NNTP ǧ��̾			*/
extern short		nntp_auth_mode;	/* NNTP ǧ�ڥ⡼��		*/
#endif	      /* NNTP_AUTH */
#ifdef	YOUBIN
char	*youbin_server_file = YOUBIN_SERVER_FILE;
					/* ͹�إ����Хե�����		*/
#endif	/* YOUBIN */
extern char		ignore_groups[];/* ̵�뤹��˥塼�����롼�׷�	*/
extern short		ignore_mode;	/* ̵�뤹��⡼��		*/
#ifdef	XOVER
extern short		xover_mode;	/* XOVER ͭ�������å�		*/
#endif	/* XOVER */
extern char		auth_file[];	/* �桼��ǧ�ڥե�����		*/

#ifdef	MIME
extern int	mime_encode_jis();	/* MIME ���󥳡���(JIS ����) */
extern int	mime_decode_euc();	/* MIME �ǥ�����(EUC ����) */
#endif	/* MIME */
extern int	to_euc();
extern int	to_jis();
#if	defined(LARGE) && (!defined(JUST_KILL))
extern void	add_kill_list();	/* ��ư�ޡ��������ɲ�		*/
#endif	/* (LARGE || (!JUST_KILL)) */

short	max_thread_number = MAX_THREAD_NUMBER;	/* ���缫ư�����ȿ�	*/
short	japanese = JAPANESE;		/* ���ܸ�⡼�ɥե饰		*/
short	gnus_mode = GNUS_MODE;		/* GNUS �饤���⡼��		*/
short	group_mask = GROUP_MASK;	/* ���롼�ץޥ����⡼��		*/
short	news_article_mask = NEWS_ARTICLE_MASK;
					/* �˥塼�������ޥ����⡼��	*/
short	mail_article_mask = MAIL_ARTICLE_MASK;
					/* �᡼�뵭���ޥ����⡼��	*/
short	news_thread_mode = NEWS_THREAD_MODE;	
					/* �˥塼������åɥ⡼��	*/
short	mail_thread_mode = MAIL_THREAD_MODE;
					/* �᡼�륹��åɥ⡼��		*/
short	mime_format = MIME_FORMAT;	/* MIME �ե����ޥåȽ���	*/
short	article_format = ARTICLE_FORMAT;/* ���������ե����ޥå�		*/
#if	defined(MH) || defined(RMAIL) || defined(UCBMAIL)
short	msgchk_rule = MSGCHK_RULE;	/* �᡼������å���ˡ		*/
#endif	/* (MH || RMAIL || UCBMAIL) */
#ifdef	REF_SORT
short	sort_rule = SORT_RULE;		/* ��������ˡ			*/
#else	/* !REF_SORT */
short	sort_rule = 0;			/* ��������ˡ			*/
#endif	/* !REF_SORT */
short	nntp_mode = NNTP_MODE;		/* NNTP ���ѥ⡼��		*/
short	smtp_mode = SMTP_MODE;		/* SMTP ���ѥ⡼��		*/
short	pop3_mode = POP3_MODE;		/* POP3 ���ѥ⡼��		*/
short	rpop_mode = RPOP_MODE;		/* RPOP ���ѥ⡼��		*/
short	pop3_del_mode = POP3_DEL_MODE;	/* POP3 ����⡼��		*/
#ifdef	UCBMAIL
short	ucbmail_mode = UCBMAIL_MODE;	/* UCB-mail �⡼�ɥե饰	*/
#endif	/* UCBMAIL */
#ifdef	RMAIL
short	rmail_mode = RMAIL_MODE;	/* RMAIL �⡼�ɥե饰		*/
#endif	/* RMAIL */
#ifdef	MH
short	mh_mode = MH_MODE;		/* MH �⡼�ɥե饰		*/
short	mh_mark_mode = MH_MARK_MODE;	/* MH �ޡ����⡼��		*/
short	mh_select_mode = MH_SELECT_MODE;/* MH ��ư����⡼��		*/
#endif	/* MH */
#ifdef	BOARD
short	board_mode = BOARD_MODE;	/* BOARD �⡼�ɥե饰		*/
short	board_count_mode = BOARD_COUNT_MODE;
					/* BOARD ������������ȥե饰	*/
#endif	/* BOARD */
short	pager_mode = PAGER_MODE;	/* �����ڡ����㡼�⡼��		*/
short	quiet_mode = QUIET_MODE;	/* �ٹ�ɽ���⡼��		*/
short	mail_mode = MAIL_MODE;		/* �᡼�뻲�����ѥ⡼��		*/
short	error_mode = ERROR_MODE;	/* ���顼���⡼��		*/
short	cross_mark_mode = CROSS_MARK_MODE;/* �����ݥ��Ȼ��ޡ����⡼��	*/
KANJICODE default_code = DEFAULT_CODE;	/* Ƚ����ǽ���δ���������	*/
#if	defined(NEWSPOST) || defined(MAILSEND)
KANJICODE edit_code = EDIT_CODE;	/* ���ǥ�������������		*/
short	auto_sig_mode = AUTO_SIG_MODE;	/* .signature ��ư�ղå⡼��	*/
short	from_mode = FROM_MODE;		/* From �񼰥⡼��		*/
#endif	/* (NEWSPOST || MAILSEND) */
short	wide_mode = WIDE_MODE;		/* �磻�ɥ⡼��			*/
short	bell_mode = BELL_MODE;		/* �᡼��������Υ٥벻�⡼��	*/
short	x_nsubj_mode = X_NSUBJ_MODE;	/* X-Nsubject: �⡼��		*/
short	x_nsubj_org  = X_NSUBJ_MODE;	/* X-Nsubject: ���ꥸ�ʥ�⡼��	*/
KANJICODE save_code = SAVE_CODE;	/* ���������ִ���������		*/
KANJICODE input_code = INPUT_CODE;	/* 1 �����ϴ���������		*/
#ifdef	FCC
KANJICODE fcc_code = FCC_CODE;		/* Fcc: ��¸����������		*/
#endif	/* FCC */
#ifdef	MAILSEND
short	add_cc_mode = ADD_CC_MODE;	/* Cc: �ղå⡼��		*/
short	reply_cc_mode = REPLY_CC_MODE;	/* Cc: ��ץ饤�⡼��		*/
short	mh_alias_mode = MH_ALIAS_MODE;	/* MH �����ꥢ���⡼��		*/
#endif	/* MAILSEND */
#ifdef	NEWSPOST
short	add_lines_mode = ADD_LINES_MODE;/* Lines: �ղå⡼��		*/
short	inews_sig_mode = INEWS_SIG_MODE;/* inews �� .signature �⡼��	*/
short	ask_distrib_mode = ASK_DISTRIB_MODE;
					/* Distribution: �ե����������	*/
					/* �⡼��			*/
#endif	/* NEWSPOST */
short	jst_mode = JST_MODE;		/* ���ܻ����Ѵ��⡼��		*/
short	dnas_mode = DNAS_MODE;		/* DNAS �б��⡼��		*/
short	unixfrom_mode = UNIXFROM_MODE;	/* UNIX-From �⡼��		*/
short	color_mode = COLOR_MODE;	/* ���顼�⡼��			*/
short	my_adrs_mode = MY_ADRS_MODE;	/* �����ɥ쥹�Ѵ��⡼��		*/
#if	defined(MH) || defined(RMAIL) || defined(UCBMAIL)
short	auto_inc_mode = AUTO_INC_MODE;	/* ������ư����⡼��		*/
#endif	/* (MH || RMAIL || UCBMAIL) */
#ifndef	SMALL
KANJICODE pipe_code = PIPE_CODE;	/* �ѥ��״���������		*/
#endif	/* !SMALL */
#ifdef	PAGER
short	scroll_mode = SCROLL_MODE;	/* ��������⡼��		*/
#else	/* !PAGER */
int	last_key = 0;			/* �Ǹ�����Ϥ�������		*/
char	guide_message[MINIMUM_BUFF];	/* �����ɥ�å�����		*/
#endif	/* !PAGER */
short	color_code[] = {		/* ���顼������			*/
  TITLE_COLOR_CODE,
  HEADER_COLOR_CODE,
  GUIDE_COLOR_CODE,
  ERROR_COLOR_CODE,
  CATEGORY_COLOR_CODE,
  MARK_COLOR_CODE,
  NUMBER_COLOR_CODE,
  FROM_COLOR_CODE,
  DATE_COLOR_CODE,
  LINE_COLOR_CODE,
  SUBJECT_COLOR_CODE,
  THREAD_COLOR_CODE,
  LOST_COLOR_CODE,
  FROM2_COLOR_CODE,
  TO_COLOR_CODE,
  CC_COLOR_CODE,
  DATE2_COLOR_CODE,
  GROUP_COLOR_CODE,
  REFER_COLOR_CODE,
  JNAMES_COLOR_CODE,
};

#ifdef	JNAMES
char	jnames_file[PATH_BUFF] = JNAMES_DB;
					/* JNAMES �ǡ����ե�����	*/
#endif	/* JNAMES */
char	user_name[MINIMUM_BUFF];	/* �桼��̾			*/
#if	defined(NEWSPOST) || defined(MAILSEND)
char	user_gecos[MINIMUM_BUFF];	/* �桼������			*/
#endif	/* (NEWSPOST || MAILSEND) */
char	home_dir[PATH_BUFF];		/* �ۡ���ǥ��쥯�ȥ�		*/
char	dot_dir[PATH_BUFF];		/* �ɥåȥǥ��쥯�ȥ�		*/
char	domain_name[MINIMUM_BUFF];	/* �ɥᥤ��̾			*/
char	official_name[MINIMUM_BUFF];	/* ���ե������̾		*/
#ifdef	RESOLVE
struct hostent	*host_ent;		/* �ۥ��Ⱦ���Υ���ȥ�		*/
#endif	/* RESOLVE */
#if	defined(NEWSPOST) || defined(MAILSEND)
char	edit_command[PATH_BUFF] = EDITOR_COMMAND;
					/* ���ǥ������ޥ��		*/
#endif	/* (NEWSPOST || MAILSEND) */
#ifdef	NEWSPOST
char	organization[BUFF_SIZE] = "";	/* ��°				*/
#endif	/* NEWSPOST */
char	mark_file[PATH_BUFF];		/* �ޡ����ե�����		*/
char	tmp_dir[PATH_BUFF];		/* ��ȥǥ��쥯�ȥ�		*/
char	save_dir[PATH_BUFF];		/* ���������֥ǥ��쥯�ȥ�	*/
char	pager[PATH_BUFF] = EXT_PAGER;	/* �ڡ����㡼			*/
#ifdef	UCBMAIL
char	ucbmail_mbox[PATH_BUFF];	/* UCB-mail �ѥ᡼��ܥå���	*/
#endif	/* UCBMAIL */
#ifdef	RMAIL
char	rmail_mbox[PATH_BUFF];		/* RMAIL �ѥ᡼��ܥå���	*/
#endif	/*RMAIL */
#ifdef	MH
char	mh_command_path[PATH_BUFF] = MH_COMMAND_PATH;
					/* MH ���ޥ�ɥѥ�		*/
#endif	/* MH */
#if	defined(MH) || defined(RMAIL) || defined(UCBMAIL)
char	auto_inc_folder[MINIMUM_BUFF];	/* ��ư�����ߥե����		*/
#endif	/* (MH || RMAIL || UCBMAIL) */
#ifdef	BOARD
char	board_spool[PATH_BUFF] = BOARD_SPOOL;
					/* BOARD ���ס���ѥ�		*/
char	board_mark_file[PATH_BUFF];	/* BOARD �ޡ����ե�����		*/
#endif	/* BOARD */
#ifdef	FCC
char	mail_fcc_field[PATH_BUFF] = "";/* �᡼����   Fcc: �ե������	*/
char	news_fcc_field[PATH_BUFF] = "";/* �˥塼���� Fcc: �ե������	*/
#endif	/* FCC */
#ifdef	MAILSEND
char	mail_field[BUFF_SIZE] = "";	/* �᡼�뼫ư�����ե������	*/
char	reply_message[BUFF_SIZE] = "";
					/* ��ץ饤��å�����		*/
#ifdef	LARGE
char	forward_start[BUFF_SIZE] = "";	/* �ե���ɳ��ϥ�å�����	*/
char	forward_end[BUFF_SIZE] = "";	/* �ե���ɽ�λ��å�����	*/
char	send_filter[PATH_BUFF] = SEND_FILTER;
					/* �᡼�������ե��륿		*/
#endif	/* LARGE */
#endif	/* MAILSEND */
#ifdef	LARGE
char	lpr_command[PATH_BUFF] = LPR_COMMAND;
					/* �������ޥ��			*/
KANJICODE lpr_code = LPR_CODE;		/* ��������������		*/
char	second_adrs[BUFF_SIZE] = "";	/* ������ɥ��ɥ쥹		*/
#endif	/* LARGE */
#ifdef	NEWSPOST
char	news_field[BUFF_SIZE] = "";	/* �˥塼����ư�����ե������	*/
#ifdef	LARGE
char	post_filter[PATH_BUFF] = POST_FILTER;
					/* �˥塼����ƥե��륿		*/
#endif	/* LARGE */
#endif	/* NEWSPOST */
#if	defined(NEWSPOST) || defined(MAILSEND)
char	follow_message[BUFF_SIZE] = "";
					/* �ե�����å�����		*/
char	reply_indicator[BUFF_SIZE] = REPLY_INDICATOR;
					/* ���󥸥�����			*/
#endif	/* (NEWSPOST || MAILSEND) */
#ifdef	MIME_MULTIMEDIA
char	mpeg_command[PATH_BUFF] = MPEG_COMMAND;
					/* MPEG  �������ޥ��		*/
char	jpeg_command[PATH_BUFF] = JPEG_COMMAND;
					/* JPEG  ɽ�����ޥ��		*/
char	gif_command[PATH_BUFF] = GIF_COMMAND;
					/* GIF   ɽ�����ޥ��		*/
char	audio_command[PATH_BUFF] = AUDIO_COMMAND;
					/* AUDIO �������ޥ��		*/
char	ps_command[PATH_BUFF] = PS_COMMAND;
					/* PostScript ɽ�����ޥ��	*/
#endif	/* MIME_MULTIMEDIA */

#if	!(defined(MSDOS) || defined(OS2))
char	*net_opt      = NET_OPT;	/* �ͥåȥ����³���ץ����	*/
char	*cons_opt     = CONS_OPT;	/* �������åȹ������ץ����	*/
char	*conf_opt     = CONF_OPT;	/* �������å����ꥪ�ץ����	*/
char	*arch_opt     = ARCH_OPT;	/* �������ƥ����㥪�ץ����	*/
char	*install_host = INSTALL_HOST;	/* ���󥹥ȡ���ۥ���̾		*/
char	*install_user = INSTALL_USER;	/* ���󥹥ȡ���桼��̾		*/
char	*install_date = INSTALL_DATE;	/* ���󥹥ȡ�������		*/
#endif	/* !(MSDOS || OS2) */

int	mime_form_char;			/* MIME �ե����ޥåȥ⡼��ʸ��	*/
int	(*mime_encode_func)();		/* MIME ���󥳡��ɴؿ�		*/
int	(*mime_decode_func)();		/* MIME �ǥ����ɴؿ�		*/

static struct parameter	{
  char		*name;
  int		type;
  CASTPTR	variable;
} parameter[] = {
  {"max_thread_number",	PARAM_NUMBER_TYPE,	(CASTPTR)&max_thread_number},
  {"gnus_mode",		PARAM_SW_TYPE,		(CASTPTR)&gnus_mode},
  {"group_mask",	PARAM_SW_TYPE,		(CASTPTR)&group_mask},
  {"article_mask",	PARAM_SW_TYPE,		(CASTPTR)&news_article_mask},
  {"news_article_mask",	PARAM_SW_TYPE,		(CASTPTR)&news_article_mask},
  {"mail_article_mask",	PARAM_SW_TYPE,		(CASTPTR)&mail_article_mask},
  {"thread_mode",	PARAM_SW_TYPE,		(CASTPTR)&news_thread_mode},
  {"news_thread_mode",	PARAM_SW_TYPE,		(CASTPTR)&news_thread_mode},
  {"mail_thread_mode",	PARAM_SW_TYPE,		(CASTPTR)&mail_thread_mode},
  {"mail_mode",		PARAM_SW_TYPE,		(CASTPTR)&mail_mode},
  {"error_mode",	PARAM_SW_TYPE,		(CASTPTR)&error_mode},
  {"cross_mark_mode",	PARAM_SW_TYPE,		(CASTPTR)&cross_mark_mode},
  {"quiet_mode",	PARAM_SW_TYPE,		(CASTPTR)&quiet_mode},
  {"japanese_mode",	PARAM_SW_TYPE,		(CASTPTR)&japanese},
  {"pager_mode",	PARAM_SW_TYPE,		(CASTPTR)&pager_mode},
  {"nntp_mode",		PARAM_SW_TYPE,		(CASTPTR)&nntp_mode},
#ifdef	NNTP_AUTH
  {"nntp_user",		PARAM_STRING_TYPE,	(CASTPTR)nntp_user},
  {"nntp_auth_mode",	PARAM_SW_TYPE,		(CASTPTR)&nntp_auth_mode},
#endif	/* NNTP_AUTH */
  {"smtp_mode", 	PARAM_SW_TYPE,		(CASTPTR)&smtp_mode},
  {"pop3_mode", 	PARAM_SW_TYPE,		(CASTPTR)&pop3_mode},
  {"pop3_del_mode", 	PARAM_SW_TYPE,		(CASTPTR)&pop3_del_mode},
  {"rpop_mode", 	PARAM_SW_TYPE,		(CASTPTR)&rpop_mode},
  {"wide_mode",		PARAM_SW_TYPE,		(CASTPTR)&wide_mode},
  {"bell_mode",		PARAM_SW_TYPE,		(CASTPTR)&bell_mode},
  {"x_nsubj_mode",	PARAM_SW_TYPE,		(CASTPTR)&x_nsubj_org},
  {"jst_mode",		PARAM_SW_TYPE,		(CASTPTR)&jst_mode},
  {"dnas_mode",		PARAM_SW_TYPE,		(CASTPTR)&dnas_mode},
  {"unixfrom_mode",	PARAM_SW_TYPE,		(CASTPTR)&unixfrom_mode},
  {"color_mode",	PARAM_SW_TYPE,		(CASTPTR)&color_mode},
  {"my_adrs_mode",	PARAM_SW_TYPE,		(CASTPTR)&my_adrs_mode},
#ifdef	XOVER
  {"xover_mode",	PARAM_SW_TYPE,		(CASTPTR)&xover_mode},
#endif	/* XOVER */
  {"print_code", 	PARAM_CODE_TYPE,	(CASTPTR)&print_code},
  {"save_code", 	PARAM_CODE_TYPE,	(CASTPTR)&save_code},
  {"input_code", 	PARAM_CODE_TYPE,	(CASTPTR)&input_code},
  {"default_code", 	PARAM_CODE_TYPE,	(CASTPTR)&default_code},
#ifdef	UCBMAIL
  {"ucbmail_mode",	PARAM_SW_TYPE,		(CASTPTR)&ucbmail_mode},
  {"ucbmail_mbox",	PARAM_FILE1_TYPE,	(CASTPTR)ucbmail_mbox},
#endif	/* UCBMAIL */
#ifdef	RMAIL
  {"rmail_mode",	PARAM_SW_TYPE,		(CASTPTR)&rmail_mode},
  {"rmail_mbox",	PARAM_FILE1_TYPE,	(CASTPTR)rmail_mbox},
#endif	/*RMAIL */
#ifdef	MH
  {"mh_mode",		PARAM_SW_TYPE,		(CASTPTR)&mh_mode},
  {"mh_mark_mode",	PARAM_SW_TYPE,		(CASTPTR)&mh_mark_mode},
  {"mh_select_mode",	PARAM_SW_TYPE,		(CASTPTR)&mh_select_mode},
#endif	/* MH */
#ifdef	BOARD
  {"board_mode",	PARAM_SW_TYPE,		(CASTPTR)&board_mode},
  {"board_count_mode",	PARAM_SW_TYPE,		(CASTPTR)&board_count_mode},
  {"board_dir",		PARAM_FILE2_TYPE,	(CASTPTR)board_spool},
#endif	/* BOARD */
#if	defined(NEWSPOST) || defined(MAILSEND)
  {"edit_code", 	PARAM_CODE_TYPE,	(CASTPTR)&edit_code},
  {"auto_sig_mode",	PARAM_SW_TYPE,		(CASTPTR)&auto_sig_mode},
  {"edit_command",	PARAM_FILE2_TYPE,	(CASTPTR)edit_command},
  {"from_mode",		PARAM_SW_TYPE,		(CASTPTR)&from_mode},
#endif	/* (NEWSPOST || MAILSEND) */
#ifdef	MAILSEND
  {"add_cc_mode",	PARAM_SW_TYPE,		(CASTPTR)&add_cc_mode},
  {"reply_cc_mode",	PARAM_SW_TYPE,		(CASTPTR)&reply_cc_mode},
  {"mh_alias_mode",	PARAM_SW_TYPE,		(CASTPTR)&mh_alias_mode},
  {"mail_field",	PARAM_STRING_TYPE,	(CASTPTR)mail_field},
  {"reply_message",	PARAM_FORMAT_TYPE,	(CASTPTR)reply_message},
#ifdef	LARGE
  {"forward_start",	PARAM_FORMAT_TYPE,	(CASTPTR)forward_start},
  {"forward_end",	PARAM_FORMAT_TYPE,	(CASTPTR)forward_end},
  {"send_filter",	PARAM_FILE2_TYPE,	(CASTPTR)send_filter},
  {"lpr_code",		PARAM_CODE_TYPE,	(CASTPTR)&lpr_code},
  {"lpr_command",	PARAM_FILE2_TYPE,	(CASTPTR)lpr_command},
  {"second_adrs",	PARAM_STRING_TYPE,	(CASTPTR)second_adrs},
#endif	/* LARGE */
#endif	/* MAILSEND */
#ifdef	NEWSPOST
  {"inews_sig_mode",	PARAM_SW_TYPE,		(CASTPTR)&inews_sig_mode},
  {"add_lines_mode",	PARAM_SW_TYPE,		(CASTPTR)&add_lines_mode},
  {"news_field",	PARAM_STRING_TYPE,	(CASTPTR)news_field},
  {"ask_distrib_mode",	PARAM_SW_TYPE,		(CASTPTR)&ask_distrib_mode},
#ifdef	LARGE
  {"post_filter",	PARAM_FILE2_TYPE,	(CASTPTR)post_filter},
#endif	/* LARGE */
#endif	/* NEWSPOST */
#ifndef	SMALL
  {"pipe_code", 	PARAM_CODE_TYPE,	(CASTPTR)&pipe_code},
#endif	/* !SMALL */
  {"pager_command",	PARAM_FILE2_TYPE,	(CASTPTR)pager},
#if	defined(NEWSPOST) || defined(MAILSEND)
  {"follow_message",	PARAM_FORMAT_TYPE,	(CASTPTR)follow_message},
#ifdef	notdef
  {"indicater",		99,			(CASTPTR)reply_indicator},
#endif	/* notdef */
  {"indicator",		PARAM_FORMAT_TYPE,	(CASTPTR)reply_indicator},
#endif	/* (NEWSPOST || MAILSEND) */
  {"tmp_dir",		PARAM_FILE1_TYPE,	(CASTPTR)tmp_dir},
  {"save_dir",		PARAM_STRING_TYPE,	(CASTPTR)save_dir},
						/* ~/ ��Ÿ�����ʤ��Ǥ��� */
#ifdef	FCC
  {"fcc_code", 		PARAM_CODE_TYPE,	(CASTPTR)&fcc_code},
  {"fcc_field",		PARAM_STRING_TYPE,	(CASTPTR)mail_fcc_field},
  {"mail_fcc_field",	PARAM_STRING_TYPE,	(CASTPTR)mail_fcc_field},
  {"news_fcc_field",	PARAM_STRING_TYPE,	(CASTPTR)news_fcc_field},
#endif	/* FCC */
#if	defined(MH) || defined(RMAIL) || defined(UCBMAIL)
  {"auto_inc_mode",	PARAM_SW_TYPE,		(CASTPTR)&auto_inc_mode},
  {"auto_inc_folder",	PARAM_STRING_TYPE,	(CASTPTR)auto_inc_folder},
  {"msgchk_rule",	PARAM_SELECT_TYPE,	(CASTPTR)&msgchk_rule},
#endif	/* (MH || RMAIL || UCBMAIL) */
  {"article_format",	PARAM_SELECT_TYPE,	(CASTPTR)&article_format},
  {"mime_format",	PARAM_SELECT_TYPE,	(CASTPTR)&mime_format},
#ifdef	REF_SORT
  {"sort_rule",		PARAM_SELECT_TYPE,	(CASTPTR)&sort_rule},
#endif	/* REF_SORT */
  {"ignore_groups",	PARAM_GROUP1_TYPE,	(CASTPTR)ignore_groups},
  {"read_groups",	PARAM_GROUP2_TYPE,	(CASTPTR)ignore_groups},
#ifdef	COLOR
  {"color_code",	PARAM_COLOR_TYPE,	(CASTPTR)color_code},
#endif	/* COLOR */
#ifdef	PAGER
  {"scroll_mode",	PARAM_SW_TYPE,		(CASTPTR)&scroll_mode},
#endif	/* PAGER */
#if	defined(LARGE) && (!defined(JUST_KILL))
  {"kill_subject",	PARAM_KILL_TYPE,	(CASTPTR)0},
  {"kill_from",		PARAM_KILL_TYPE,	(CASTPTR)1},
#endif	/* (LARGE || (!JUST_KILL)) */
#ifdef	JNAMES
  {"jnames_file",	PARAM_FILE2_TYPE,	(CASTPTR)jnames_file},
#endif	/* JNAMES */
#ifdef	MIME_MULTIMEDIA
  {"mpeg_command",	PARAM_FILE2_TYPE,	(CASTPTR)mpeg_command},
  {"jpeg_command",	PARAM_FILE2_TYPE,	(CASTPTR)jpeg_command},
  {"gif_command",	PARAM_FILE2_TYPE,	(CASTPTR)gif_command},
  {"audio_command",	PARAM_FILE2_TYPE,	(CASTPTR)audio_command},
  {"ps_command",	PARAM_FILE2_TYPE,	(CASTPTR)ps_command},
#endif	/* MIME_MULTIMEDIA */
};
#ifndef	MSDOS
static char	delete_char[] = {	/* Gecos �˻��Ѥ���Ȥޤ���ʸ��	*/
  ',', '<', '>', '(', ')'
  };
#endif	/* !MSDOS */
static char	*kanji_string = "����";
#ifdef	SJIS_SRC
static char	*kanji_code = "\212\277\216\232";
#else	/* !SJIS_SRC */
static char	*kanji_code = "\264\301\273\372";
#endif	/* !SJIS_SRC */

static int	read_config_file();	/* ����ե����ե�����Υ꡼��	*/
static void	get_organ_file();	/* ��°�ե�����Υ꡼��		*/
static void	get_file_env();		/* �ե�����̾�δĶ�����		*/

/*
 * �ͥåȥ������ե����졼�����
 */

int	net_config()
{
  char		*env_ptr;
  char		localhost[MINIMUM_BUFF];
#ifndef	DOMAIN_NAME
  FILE		*fp;
#endif	/* !DOMAIN_NAME */
  char		*ptr;
  char		buff[BUFF_SIZE];

#ifdef	NSPL
  env_ptr = (char*)getenv(NEWS_SPOOL_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(news_spool, env_ptr);
  } else {
    strcpy(news_spool, NEWS_SPOOL);
  }
  env_ptr = (char*)getenv(NEWS_LIB_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(news_lib, env_ptr);
  } else {
    strcpy(news_lib, NEWS_LIB);
  }
#endif	/* NSPL */
#ifdef	MSPL
  env_ptr = (char*)getenv(MAIL_SPOOL_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(mail_spool, env_ptr);
  } else {
    strcpy(mail_spool, MAIL_SPOOL);
  }
#endif	/* MSPL */
  strcpy(nntp_server_file, NNTP_SERVER_FILE);
  strcpy(smtp_server_file, SMTP_SERVER_FILE);
  strcpy(pop3_server_file, POP3_SERVER_FILE);
#ifdef	NEWSPOST
#ifdef	LOCAL_POST
  force_use_inews = 1;
#endif	/* LOCAL_POST */
  strcpy(post_program, POST_COMMAND);
#endif	/* NEWSPOST */
#ifdef	MAILSEND
  strcpy(send_program, SEND_COMMAND);
#endif	/* MAILSEND */

  /*
   * �ɥᥤ��̾
   */

  if (gethostname(localhost, sizeof(localhost))) {
    fprintf(stderr, "Can't get server host entry.\n");
    localhost[0] = '\0';
    return(1);
  }
#ifdef	RESOLVE
  if (host_ent = gethostbyname(localhost)) {
#ifdef	ADD_HOSTNAME
    sprintf(domain_name, "%s.", host_ent->h_name);
#else	/* !ADD_HOSTNAME */
    ptr = strchr(host_ent->h_name, '.');
    if (ptr != (char*)NULL) {
      sprintf(domain_name, "%s.", ptr + 1);
    } else {
      sprintf(domain_name, "%s.", host_ent->h_name);
    }
#endif	/* !ADD_HOSTNAME */
  } else {
    fprintf(stderr, "Can't get local host entry.\n");
    return(1);
  }
#else	/* !RESOLVE */
#ifdef	DOMAIN_NAME
  strcpy(domain_name, DOMAIN_NAME);
#else	/* !DOMAIN_NAME */
  buff[0] = '\0';
  fp = fopen(DOMAIN_FILE, "r");
  if (fp != (FILE*)NULL) {
    if (fgets(buff, sizeof(buff), fp)) {
      ptr = buff;
      while (*ptr) {
	if (*ptr == '\n') {
	  *ptr = '\0';
	  break;
	}
	ptr++;
      }
    }
    fclose(fp);
  }
#ifdef	ADD_HOSTNAME
  sprintf(domain_name, "%s.%s", localhost, buff);
#else	/* !ADD_HOSTNAME */
  strcpy(domain_name, buff);
#endif	/* !ADD_HOSTNAME */
#endif	/* !DOMAIN_NAME */
#endif	/* !RESOLVE */
  env_ptr = (char*)getenv(DOMAIN_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(domain_name, env_ptr);
  }
  if (!domain_name[0]) {
    fprintf(stderr,
	    "Installation problem,default-domain name is not configured.\n");
    return(1);
  }
#ifdef	OFFICIAL_NAME
  strcpy(official_name, OFFICIAL_NAME);
#else	/* !OFFICIAL_NAME */
  ptr = strchr(localhost, '.');
  if (ptr != (char*)NULL) {
    *ptr = '\0';
  }
  sprintf(official_name, "%s.%s", localhost, domain_name);
#endif	/* !OFFICIAL_NAME */
  env_ptr = (char*)getenv(OFFICIAL_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(official_name, env_ptr);
  }
  return(0);
}

/*
 * �Ķ�����ե����졼�����
 */

int	env_config()
{
  char		config_file[PATH_BUFF];
  int		i;
  char		*env_ptr;
  char		*ptr;

  if (strcmp(kanji_code, kanji_string)) {
    fprintf(stderr,
	    "Installation problem,source kanji code mismatch.\n");
    return(1);
  }
#ifdef	NNTP_AUTH
  nntp_auth_mode = NNTP_AUTH_MODE;
#endif	/* NNTP_AUTH */
  print_code = PRINT_CODE;
  env_ptr = (char*)getenv(HOME_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(home_dir, env_ptr);
  } else {
#if	defined(MSDOS) || defined(OS2)
    fprintf(stderr, "Can't get %s environment.\n", HOME_ENV);
    exit(1);
#else	/* !(MSDOS || OS2) */
    strcpy(home_dir, passwd->pw_dir);
#endif	/* !(MSDOS || OS2) */
  }
  env_ptr = (char*)getenv(DOTDIR_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(dot_dir, env_ptr);
  } else {
    strcpy(dot_dir, home_dir);
  }
  env_ptr = (char*)getenv(USER_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(user_name, env_ptr);
  } else {
#if	defined(MSDOS) || defined(OS2)
    fprintf(stderr, "Can't get %s environment.\n", USER_ENV);
    exit(1);
#else	/* !(MSDOS || OS2) */
    strcpy(user_name, passwd->pw_name);
#endif	/* !(MSDOS || OS2) */
  }
#ifdef	MSPL
  ptr = mail_spool;
  while (*ptr) {
    ptr++;
  }
  *ptr++ = SLASH_CHAR;
  strcpy(ptr, user_name);
#endif	/* MSPL */
  sprintf(auth_file, "%s/%s", home_dir, AUTHORITY_FILE);
  env_ptr = (char*)getenv(PAGER_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(pager, env_ptr);
  }
#if	defined(NEWSPOST) || defined(MAILSEND)
  env_ptr = (char*)getenv(NAME_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(user_gecos, env_ptr);
  } else {
#if	defined(MSDOS) || defined(OS2)
    fprintf(stderr, "Can't get %s environment.\n", NAME_ENV);
    exit(1);
#else	/* !(MSDOS || OS2) */
    ptr = user_gecos;
    env_ptr = passwd->pw_gecos;
    if (*env_ptr == '*') {
      env_ptr++;
    }
    while (*env_ptr) {
      if (*env_ptr == '&') {
	strcpy(ptr, user_name);
	if (islower(*ptr)) {
	  *ptr = toupper(*ptr);
	}
	while (*ptr) {
	  ptr++;
	}
      } else {
	*ptr = *env_ptr;
	for (i = 0; i < sizeof(delete_char); i++) {
	  if (*ptr == delete_char[i]) {
	    *ptr = '\0';
	    break;
	  }
	}
	ptr++;
      }
      env_ptr++;
    }
    *ptr = '\0';
#endif	/* !(MSDOS || OS2) */
  }
  env_ptr = (char*)getenv(EDITOR_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(edit_command, env_ptr);
  }
#endif	/* (NEWSPOST || MAILSEND) */
#ifdef	NEWSPOST
  env_ptr = (char*)getenv(ORGANIZATION_ENV);
  if (env_ptr != (char*)NULL) {
    strcpy(organization, env_ptr);
  }
  get_organ_file();
#endif	/* NEWSPOST */
  env_ptr = (char*)getenv(MNEWSRC_ENV);
  if (env_ptr == (char*)NULL) {
    env_ptr = (char*)getenv(NEWSRC_ENV);
  }
  if (env_ptr != (char*)NULL) {
    if (*env_ptr == SLASH_CHAR) {
      strcpy(mark_file, env_ptr);
    } else {
      sprintf(mark_file, "%s%c%s", dot_dir, SLASH_CHAR, env_ptr);
    }
  } else {
    sprintf(mark_file, "%s%c%s", dot_dir, SLASH_CHAR, MARK_FILE);
  }
  get_file_env(tmp_dir, TMPDIR_ENV, TMP_DIR, 1);
  get_file_env(save_dir, SAVEDIR_ENV, SAVE_DIR, 0);
#ifdef	UCBMAIL
  get_file_env(ucbmail_mbox, MBOX_ENV, MBOX_FILE, 1);
#endif	/* UCBMAIL */
#ifdef	RMAIL
  get_file_env(rmail_mbox, RMAIL_ENV, RMAIL_FILE, 1);
#endif	/*RMAIL */
#ifdef	BOARD
  sprintf(board_mark_file, "%s%c%s", dot_dir, SLASH_CHAR, BOARD_MARK_FILE);
#endif	/* BOARD */

#ifdef	IGNORE_GROUPS
  ptr = IGNORE_GROUPS;
#else	/* !IGNORE_GROUPS */
#ifdef	READ_GROUPS
  ptr = READ_GROUPS;
  ignore_mode = 1;
#endif	/* READ_GROUPS */
#endif	/* !IGNORE_GROUPS */
#if	defined(IGNORE_GROUPS) || defined(READ_GROUPS)
  env_ptr = ignore_groups;
  while (*ptr) {
    while (*ptr > ' ') {
      *env_ptr++ = *ptr++;
    }
    *env_ptr++ = '\0';	/* SEPARATER */
    while (*ptr && (*ptr <= ' ')) {
      ptr++;
    }
  }
  *env_ptr = '\0';		/* END MARK */
#endif	/* (IGNORE_GROUPS || READ_GROUPS) */

  strcpy(config_file, DEFAULT_INIT_FILE);
  if (read_config_file(config_file) > 0) {
    sleep(3);
  }
  sprintf(config_file, "%s%c%s-%s", dot_dir, SLASH_CHAR, CONFIG_FILE,
	  nntp_server);
  i = read_config_file(config_file);
  if (i < 0) {
    sprintf(config_file, "%s%c%s", dot_dir, SLASH_CHAR, CONFIG_FILE);
    i = read_config_file(config_file);
    if (i < 0) {
      i = 0;
    }
  }
  x_nsubj_mode = x_nsubj_org;
#ifndef	PAGER
  pager_mode = 1;
#endif	/* !PAGER */
  return(-i);
}

/*
 * ����ե����ե��������
 * �����:(���:�ե����륪���ץ��Բ�,0:���ｪλ,����:���顼)
 */

static int	read_config_file(config_file)
    char	*config_file;
{
  FILE		*fp;
  char		buff[BUFF_SIZE];
  char		*ptr, *ptr2;
  int		line;
  int		status;
  register int	i, j;
#ifdef	COLOR
  int		k;
  int		status2;
#endif	/* COLOR */

  line = status = 0;
#ifdef	COLOR
  status2 = 0;
#endif	/* COLOR */
  fp = fopen(config_file, "r");
  if (fp == (FILE*)NULL) {
    return(-1);
  }
  while (fgets(buff, sizeof(buff), fp)) {
    line++;
    ptr = buff;
    while (*ptr) {
      if (*ptr == '\n') {
	*ptr = '\0';
	break;
      }
      ptr++;
    }
    ptr = buff;
    while ((*ptr == '\t') || (*ptr == ' ')) {
      ptr++;
    }
    if ((*ptr == '#') || (*ptr == '\0')) {
      continue;
    }
    for (i = 0; i < sizeof(parameter) / sizeof(struct parameter); i++) {
      if (!strncasecmp(ptr, parameter[i].name, strlen(parameter[i].name))) {
	ptr += strlen(parameter[i].name);
	if (*ptr++ == ':') {
	  while ((*ptr == ' ') || (*ptr == '\t')) {
	    ptr++;
	  }
	  if (((int)strlen(ptr) >= (PATH_BUFF - 1)) &&
	      ((parameter[i].type == 4) || (parameter[i].type == 5))) {
	    fprintf(stderr, "Line %3d : Too long parameter data.\n", line);
	    status = 1;
	    break;
	  } else {
	    switch (parameter[i].type) {
	    case PARAM_SW_TYPE:	/* ON/OFF ����		*/
	      if (!strncasecmp(ptr, "on", 2)) {
		*(short*)parameter[i].variable = 1;
	      } else if (!strncasecmp(ptr, "off", 3)) {
		*(short*)parameter[i].variable = 0;
	      } else {
		fprintf(stderr,
			"Line %3d : Illegal parameter data.\n", line);
		status = 1;
	      }
	      break;
	    case PARAM_CODE_TYPE:	/* ���������ɻ���	*/
	      switch (*ptr) {
	      case 'e':
	      case 'E':
		*(int*)parameter[i].variable = (CASTPTR)EUC_CODE;
		break;
	      case 'j':
	      case 'J':
		*(int*)parameter[i].variable = (CASTPTR)JIS_CODE;
		break;
	      case 's':
	      case 'S':
		*(int*)parameter[i].variable = (CASTPTR)SJIS_CODE;
		break;
	      case 'a':
	      case 'A':
		*(int*)parameter[i].variable = (CASTPTR)ASCII_CODE;
		break;
	      default:
		fprintf(stderr,
			"Line %3d : Illegal kanji code specified.\n",
			line);
		status = 1;
		break;
	      }
	      break;
	    case PARAM_STRING_TYPE:	/* ñ��ʸ����		*/
	      to_euc((char*)parameter[i].variable, ptr, default_code);
	      ptr = (char*)parameter[i].variable;
	      while (*ptr) {
		if (*ptr == '%') {
		  ptr++;
		  if (*ptr != '%') {
		    fprintf(stderr, "Line %3d : Message too complex.\n", line);
		    *(char*)parameter[i].variable = '\0';
		    status = 1;
		    break;
		  } else {
		    ptr++;
		    continue;
		  }
		}
		if (*ptr == '\\') {
		  switch (*(ptr + 1)) {
		  case 'n':
		    *ptr++ = '\n';
		    strcpy(ptr, ptr + 1);
		    break;
		  case 't':
		    *ptr++ = '\t';
		    strcpy(ptr, ptr + 1);
		    break;
		  default:
		    ptr += 2;
		    break;
		  }
		} else {
		  ptr++;
		}
	      }
	      break;
#ifdef	notdef
	    case 99:	/* �Ѥ����������ڥ�ߥ��θߴ��ݻ�����	*/
	      fprintf(stderr,
		      "Line %3d : Notice : Sorry, please fix your setup file.(indicater -> indicator)\n",
		      line);
	      sleep(1);
#endif	/* notdef */
	    case PARAM_FORMAT_TYPE:	/* �ü�ե����ޥå�ʸ����	*/
	      to_euc((char*)parameter[i].variable, ptr, default_code);
	      break;
	    case PARAM_FILE1_TYPE:	/* �ե�����/�ǥ��쥯�ȥ�̾(HOME)*/
	      if (*ptr == SLASH_CHAR) {
		strcpy((char*)parameter[i].variable, ptr);
#ifdef	MSDOS
	      } else if ((!strncmp(ptr, "~/", 2)) ||
			 (!strncmp(ptr, "~\\", 2))) {
#else	/* !MSDOS */
	      } else if (!strncmp(ptr, "~/", 2)) {
#endif	/* !MSDOS */
		sprintf((char*)parameter[i].variable, "%s%c%s", home_dir,
			SLASH_CHAR, ptr + 2);
	      } else {
#ifdef	MSDOS
		strcpy((char*)parameter[i].variable, ptr);
#else	/* !MSDOS */
		sprintf((char*)parameter[i].variable, "%s%c%s", home_dir,
			SLASH_CHAR, ptr);
#endif	/* !MSDOS */
	      }
	      break;
	    case PARAM_FILE2_TYPE:	/* �ե�����̾/�ǥ��쥯�ȥ�̾(PATH)*/
	      if (!strncmp(ptr, "~/", 2)) {
		sprintf((char*)parameter[i].variable, "%s%c%s", home_dir,
			SLASH_CHAR, ptr + 2);
	      } else {
		strcpy((char*)parameter[i].variable, ptr);
	      }
	      break;
	    case PARAM_SELECT_TYPE:	/* �������(0-2)		*/
	      j = atoi(ptr);
	      if ((j >= 0) && (j < 3)) {
		*(short*)parameter[i].variable = j;
	      } else {
		fprintf(stderr,
			"Line %3d : Illegal parameter data.\n", line);
		status = 1;
	      }
	      break;
	    case PARAM_GROUP1_TYPE:	/* ̵�뤹��˥塼�����롼�׷�	*/
	      ignore_mode = -1;
	    case PARAM_GROUP2_TYPE:	/* ̵�뤷�ʤ��˥塼�����롼�׷�	*/
	      ignore_mode++;
	      ptr2 = (char*)parameter[i].variable;
	      while (*ptr) {
		while (*ptr > ' ') {
		  *ptr2++ = *ptr++;
		}
		*ptr2++ = '\0';	/* SEPARATER */
		while (*ptr && (*ptr <= ' ')) {
		  ptr++;
		}
	      }
	      *ptr2 = '\0';		/* END MARK */
	      break;
#ifdef	COLOR
	    case PARAM_COLOR_TYPE:	/* ���顼������		*/
	      for (j = 0; j < (sizeof(color_code) / sizeof(short)); j++) {
		while (*ptr && (*ptr <= ' ')) {
		  ptr++;
		}
		if (isdigit(*ptr)) {
		  k = *ptr++ - '0';
		  if (j < 4) {
		    if (k < 8) {
		      color_code[j] = BG_BLACK_ATTRIB + k;
		    } else {
		      color_code[j] = REVERSE_ATTRIB;
		    }
		  } else {
		    if (k < 8) {
		      color_code[j] = FG_BLACK_ATTRIB + k;
		    } else {
		      color_code[j] = RESET_ATTRIB;
		    }
		  }
		} else {
		  status2 = 1;
		}
		if (status2) {
		  fprintf(stderr,
			  "Line %3d : Illegal color code specified.\n",
			  line);
		  break;
		}
	      }
	      break;
#endif	/* COLOR */
	    case PARAM_NUMBER_TYPE:	/* ¨��		*/
	      j = atoi(ptr);
	      if ((j >= 0) && (j <= 32767)) {
		*(short*)parameter[i].variable = j;
	      } else {
		fprintf(stderr,
			"Line %3d : Illegal parameter number.\n", line);
		status = 1;
	      }
	      break;
#if	defined(LARGE) && (!defined(JUST_KILL))
	    case PARAM_KILL_TYPE:	/* ��ư�ޡ���	*/
	      add_kill_list((int)parameter[i].variable, ptr);
	      break;
#endif	/* (LARGE || (!JUST_KILL)) */
	    default:
	      break;
	    }
	    i = -1;
	  }
	} else {
	  fprintf(stderr, "Line %3d : Syntax error.\n", line);
	  status = 1;
	}
	break;
      }
    }
    if (i >= 0) {
      fprintf(stderr, "Line %3d : Illegal or restricted parameter name.\n",
	      line);
      status = 1;
    }
  }
  fclose(fp);
  if (article_format > 2) {
    article_format = 0;
  }
#ifdef	COLOR
  return(status || status2);
#else	/* !COLOR */
  return(status);
#endif	/* !COLOR */
}

#ifdef	NEWSPOST
/*
 * ��°�ե�����Υ꡼��
 */

static void	get_organ_file()
{
  FILE		*fp;
  char		organ_file[PATH_BUFF];
  char		*ptr;
  int		code;

  if (organization[0]) {
    switch (organization[0]) {
    case '~':
      sprintf(organ_file, "%s%s", dot_dir, &organization[1]);
      break;
    case SLASH_CHAR:
      strcpy(organ_file, organization);
      break;
    default:
#ifdef	MSDOS
      strcpy(organ_file, organization);
#else	/* !MSDOS */
      sprintf(organ_file, "%s%c%s", dot_dir, SLASH_CHAR, organization);
#endif	/* !MSDOS */
      break;
    }
  } else {
    strcpy(organ_file, ORGAN_FILE);
  }
  fp = fopen(organ_file, "r");
  if (fp != (FILE*)NULL) {
    ptr = organization;
    while (1) {
      code = getc(fp);
      if ((code == '\n') || (code == EOF) || (!code)) {
	break;
      }
      *ptr++ = code;
    }
    *ptr = '\0';
    fclose(fp);
  }
}
#endif	/* NEWSPOST */

/*
 * �ե�����̾�δĶ�����
 */

static void	get_file_env(ptr, env, value, mode)
     char	*ptr;
     char	*env;
     char	*value;
     int	mode;	/* 0:~/ ��Ÿ�����ʤ�,0�ʳ�:~/ ��Ÿ������ */
{
  char	*env_ptr;

  env_ptr = (char*)getenv(env);
  if (env_ptr != (char*)NULL) {
    value = env_ptr;
  }
#if	defined(MSDOS) || defined(OS2)
  if ((*value == SLASH_CHAR) || (isalpha(*value) && (*(value + 1) == ':'))) {
#else	/* !(MSDOS || OS2) */
  if (*value == SLASH_CHAR) {
#endif	/* !(MSDOS || OS2) */
    strcpy(ptr, value);
#ifdef	MSDOS
  } else if ((!strncmp(value, "~/", 2)) || (!strncmp(value, "~\\", 2))) {
#else	/* !MSDOS */
  } else if (!strncmp(value, "~/", 2)) {
#endif	/* !MSDOS */
    if (mode) {
      sprintf(ptr, "%s%c%s", home_dir, SLASH_CHAR, value + 2);
    } else {
      strcpy(ptr, value);
    }
  } else {
#ifdef	MSDOS
    strcpy(ptr, value);
#else	/* !MSDOS */
    sprintf(ptr, "%s%c%s", home_dir, SLASH_CHAR, value);
#endif	/* !MSDOS */
  }
}

/*
 * ���ߡ��⥸�塼�뷲
 */

#ifndef	NEWSPOST
int	news_post()
{
  return(1);
}

int	news_follow()
{
}

int	news_cancel()
{
}
#endif	/* !NEWSPOST */

#ifndef	MAILSEND
int	mail_send()
{
  return(1);
}

int	mail_reply()
{
  return(1);
}

int	mail_forward()
{
  return(1);
}
#endif	/* !MAILSEND */

#ifndef	JNAMES
int	jnOpen()
{
  return(-2);
}

int	jnClose()
{
  return(0);
}

unsigned char	*jnFetch()
{
  return((unsigned char*)NULL);
}
#endif	/* !JNAMES */

#ifndef	MH
int	mh_init()
{
  return(0);
}

int	mh_menu()
{
  return(0);
}

int	save_mh_folder()
{
  return(1);
}
#endif	/* !MH */

#ifndef	UCBMAIL
int	ucbmail_init()
{
  return(0);
}

int	ucbmail_menu()
{
  return(0);
}
#endif	/* !UCBMAIL */

#ifndef	RMAIL
int	rmail_init()
{
  return(0);
}

int	rmail_menu()
{
  return(0);
}
#endif	/* !RMAIL */

#ifndef	BOARD
int	board_init()
{
  return(0);
}

int	board_menu()
{
  return(0);
}
#endif	/* !BOARD */

#ifndef	PAGER
int	view_file()
{
  return(1);
}
#endif	/* !PAGER */

#ifndef	FCC
int	fcc_save()
{
  return(1);
}
#endif	/* FCC */

/*
 * MIME ���󥳡���/�ǥ����ɴؿ���Ͽ
 */

void	set_mime_decode_func()
{
  static char	mime_char[] = "-mM";

#ifdef	MIME
  if (mime_format == 2) {
    mime_encode_func = mime_encode_jis;
  } else {
#ifdef	EUC_SPOOL
    mime_encode_func = (int (*)())to_euc;
#else	/* !EUC_SPOOL */
    mime_encode_func = (int (*)())to_jis;
#endif	/* !EUC_SPOOL */
  }
  if (mime_format) {
    mime_decode_func = mime_decode_euc;
  } else {
    mime_decode_func = (int (*)())to_euc;
  }
#else	/* !MIME */
#ifdef	EUC_SPOOL
  mime_encode_func = (int (*)())to_euc;
#else	/* !EUC_SPOOL */
  mime_encode_func = (int (*)())to_jis;
#endif	/* !EUC_SPOOL */
  mime_decode_func = (int (*)())to_euc;
#endif	/* !MIME */
  mime_form_char = (int)mime_char[mime_format];
}

/*
 * ��Υ���⡼�ɥ��顼����
 */

void	setup_mono()
{
  register int	i;

  for (i = 0; i < (sizeof(color_code) / sizeof(short)); i++) {
    color_code[i] = (i < 4) ? REVERSE_ATTRIB : RESET_ATTRIB;
  }
}

#if	defined(MAILSEND) || defined(NEWSPOST)
#ifdef	ADD_MESSAGE_ID
/*
 * Message-ID: �ե�����ɤ�����
 * <YYMMDDhhmmss.MCNTpid@host.domain>
 */

void	create_msg_id(ptr)
     char	*ptr;
{
  static int	cnt = 0;
  struct tm	*tm;
  time_t	now_time;

  now_time = time((time_t*)NULL);
  tm = localtime(&now_time);
  sprintf(ptr, "%s <%02d%02d%02d%02d%02d%02d.M%02d%05d@%s>\n",
	  MESSAGE_FIELD, tm->tm_year % 100, tm->tm_mon + 1, tm->tm_mday,
	  tm->tm_hour, tm->tm_min, tm->tm_sec, ++cnt % 100, getpid(),
	  official_name);
}
#endif	/* ADD_MESSAGE_ID */
#endif	/* (MAILSEND || NEWSPOST) */
