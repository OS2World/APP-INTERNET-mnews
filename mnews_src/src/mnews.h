/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Main define
 *  File        : mnews.h
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-12/23
 *  Author      : MSR24 ��� ��
 *
 */

#ifndef		DONT_HAVE_FTIME
#include	<sys/timeb.h>
#endif		/* DONT_HAVE_FTIME */

#define	READER_NAME	"mnews"
#define	MNEWS_VERSION	"1.21"
#define	MNEWS_DATE	"1997-12/23(Tue)"
#define	ERROR_FILE	"mnewserr"

#if	defined(MSDOS) && !defined(USE_LONG_FNAME)
#ifdef	X68K
#define	SIGNATURE_FILE	"_signature"
#else	/* !X68K */
#define	SIGNATURE_FILE	"_sign"
#endif	/* !X68K */
#define	MESSAGE_FILE	"_message"
#define	DEAD_FILE	"deadmail"
#else	/* !MSDOS || USE_LONG_FNAME */
#define	SIGNATURE_FILE	".signature"
#define	MESSAGE_FILE	".message"
#define	DEAD_FILE	"dead.letter"
#endif	/* !MSDOS || USE_LONG_FNAME */

#define	FROM_JN_DOMAIN	"familyname"

#define	MAX_MULTI_NUMBER	30	/* �����ޥ�����������		*/
#ifdef	LARGE
#define	ALIAS_BUFF		(BUFF_SIZE * 4)
#else	/* !LARGE */
#define	ALIAS_BUFF		BUFF_SIZE
#endif	/* !LARGE */

/*
 * ����¾
 */

#ifndef	LF
#define	LF	0x0a
#endif	/* !LF */
#define	REPLY_SUBJECT	"Re:"	/* ��ץ饤���֥�������		*/
#define	REPLY_MESSAGE	"Your message of "
#define	TEXT_SEPARATER	"--text follows this line--"
#define	MULTI_END_MARK	"\001\n"/* �ޥ�������Υ��ѥ졼��	*/
#define	ERROR_SLEEP	3	/* ���顼ȯ�������Ԥ�����	*/
#define	HEAD_LINES	3
#define	BOTTOM_LINES	1
#define	MODE_LINES	(HEAD_LINES + BOTTOM_LINES)
#define	MAX_MAIL_GROUP	6
#define	MAX_WRITE_FILE	5
#define	JST_OFFSET	(-9)

/*
 * �ޡ��������ꥹ�ȹ�¤��
 */

typedef	struct mark_struct	MARK_LIST;
struct mark_struct {
  MARK_LIST	*prev_ptr;				/* ���ޡ�������	*/
  MARK_LIST	*next_ptr;				/* ���ޡ�������	*/
  int		start_number;				/* �����ֹ�	*/
  int		end_number;				/* ��λ�ֹ�	*/
};

/*
 * ���롼�״�����¤��
 */

typedef	struct group_struct	GROUP_LIST;
struct group_struct {
  int		unread_article;				/* ̤�ɵ�����	*/
  MARK_LIST	*mark_ptr;				/* �ޡ�������	*/
  short		match_flag;				/* Ŭ��ե饰	*/
  short		unsubscribe;				/* ���ɥ⡼��	*/
  short		mark_order;				/* �ޡ������	*/
};

/*
 * ̵�����롼�״�����¤��
 */

typedef	struct bogus_struct	BOGUS_LIST;
struct bogus_struct {
  char		group_name[MAX_GROUP_NAME];		/* ���롼��̾	*/
  BOGUS_LIST	*next_ptr;				/* ������	*/
  MARK_LIST	*mark_ptr;				/* �ޡ�������	*/
  short		mark_order;				/* �ޡ������	*/
  short		unsubscribe;				/* ���ɥ⡼��	*/
};

/*
 * Message-ID ������¤��
 */

#ifdef	REF_SORT
typedef	struct message_struct	MESSAGE_LIST;
struct message_struct {
  char		msg_id[MAX_FIELD_LEN];		/* ��å����� ID	*/
  char		ref_id[MAX_FIELD_LEN];		/* ��ե���� ID	*/
};
#endif	/* REF_SORT */

/*
 * ����������¤��
 */

typedef	struct article_struct	ARTICLE_LIST;
struct article_struct {
  int		real_number;			/* �µ����ֹ�		*/
  char		from[MAX_FROM_LEN];		/* ���пͥ��ɥ쥹	*/
  char		subject[MAX_SUBJECT_LEN];	/* ���֥�������		*/
  short		mark;				/* �ޡ���		*/
#ifdef	REF_SORT
  short		year;				/* ǯ			*/
#endif	/* REF_SORT */
  short		month;				/* ��			*/
  short		date;				/* ��			*/
  short		lines;				/* �Կ�			*/
};

typedef enum {
  NORMAL_YN_MODE,
  CARE_YN_MODE,
  FILE_YN_MODE,
  JUMP_YN_MODE,
  MIME_YN_MODE,
  SEND_YN_MODE,
  MARK_YN_MODE
} yn_mode;

/*
 * ���������ޡ���
 */

#define	READ_MARK	0x0001
#define	FORWARD_MARK	0x0002
#define	ANSWER_MARK	0x0004
#define	DELETE_MARK	0x0008
#define	MULTI_MARK	0x0010
#define	UNFETCH_MARK	0x0040
#define	CANCEL_MARK	0x0080
#define	SORT_MARK	0x0100
#define	THREAD_MARK	0x0200

/*
 * �������ϻ���ޥ���
 */

#define	GLOBAL_MODE_MASK	0x01
#define	GROUP_MODE_MASK		0x02
#define	SUBJECT_MODE_MASK	0x04
#define	LINE_MODE_MASK		0x08
#define	YN_MODE_MASK		0x10
#define	NEWS_MODE_MASK		0x20
#define	MOUSE_MODE_MASK		0x40

#define	INPUT_SPCCUT_MASK	0x01
#define	INPUT_EXPAND_MASK	0x02
#define	INPUT_COMP_MASK		0x04
#define	INPUT_FOLDER_MASK	0x08
#define	INPUT_SHADOW_MASK	0x10

/*
 * ���顼����ǥå���
 */

enum color_code {
  TITLE_COLOR,
  HEADER_COLOR,
  GUIDE_COLOR,
  ERROR_COLOR,
  CATEGORY_COLOR,
  MARK_COLOR,
  NUMBER_COLOR,
  FROM_COLOR,
  DATE_COLOR,
  LINE_COLOR,
  SUBJECT_COLOR,
  THREAD_COLOR,
  LOST_COLOR,
  FROM2_COLOR,
  TO_COLOR,
  CC_COLOR,
  DATE2_COLOR,
  GROUP_COLOR,
  REFER_COLOR,
  JNAMES_COLOR
};

extern GROUP_LIST	*group_list;
extern BOGUS_LIST	*bogus_list;
extern ARTICLE_LIST	*article_list;
#ifndef	MSDOS
extern struct passwd	*passwd;
#endif	/* !MSDOS */

extern char	*month_string[];	/* ��̾				*/
extern char	*day_string[];		/* ����̾			*/
extern char	*day_jstring[];		/* ����̾(���ܸ�)		*/
extern char	jnames_file[];		/* JNAMES �ǡ����ե�����	*/
extern char	domain_name[];		/* �ɥᥤ��̾			*/
extern char	user_name[];		/* �桼��̾			*/
extern char	user_gecos[];		/* �桼������			*/
extern char	home_dir[];		/* �ۡ���ǥ��쥯�ȥ�		*/
extern char	dot_dir[];		/* �ɥåȥǥ��쥯�ȥ�		*/
extern char	edit_command[];		/* ���ǥ������ޥ��		*/
extern char	*new_mail_string[];	/* ����᡼���å�����		*/
extern char	*net_opt;		/* �ͥåȥ����³���ץ����	*/
extern char	*cons_opt;		/* �������åȹ������ץ����	*/
extern char	*conf_opt;		/* �������å����ꥪ�ץ����	*/
extern char	*arch_opt;		/* �������ƥ����㥪�ץ����	*/
extern char	*install_host;		/* ���󥹥ȡ���ۥ���̾		*/
extern char	*install_user;		/* ���󥹥ȡ���桼��̾		*/
extern char	*install_date;		/* ���󥹥ȡ�������		*/
extern char	mark_file[];		/* �ޡ����ե�����		*/
extern char	pager[];		/* �ڡ����㡼���ޥ��		*/
extern char	tmp_dir[];		/* ��ȥǥ��쥯�ȥ�		*/
extern char	save_dir[];		/* ���������֥ǥ��쥯�ȥ�	*/
extern char	second_adrs[];		/* ������ɥ��ɥ쥹		*/

extern int	multi_list[];		/* �ޥ���ե���/��ץ饤�ꥹ��*/
extern int	multi_number;		/* �ޥ���ե���/��ץ饤��	*/
extern short	nntp_mode;		/* NNTP ���ѥ⡼��		*/
extern short	smtp_mode;		/* SMTP ���ѥ⡼��		*/
extern short	pop3_mode;		/* POP3 ���ѥ⡼��		*/
extern short	rpop_mode;		/* RPOP ���ѥ⡼��		*/
extern short	pop3_del_mode;		/* POP3 ����⡼��		*/
extern short	pager_mode;		/* �����ڡ����㡼�⡼��		*/
extern short	japanese;		/* ���ܸ�⡼�ɥե饰		*/
extern short	jnames;			/* JNAMES ���ѥե饰		*/
extern short	quiet_mode;		/* �ٹ�ɽ���⡼��		*/
extern short	skip_mode;		/* �إå������åץ⡼��		*/
extern short	mail_mode;		/* �᡼�뻲�����ѥ⡼��		*/
extern short	error_mode;		/* ���顼���⡼��		*/
extern short	mh_mark_mode;		/* MH �ޡ����⡼��		*/
extern short	color_mode;		/* ���顼�⡼��			*/
extern short	wide_mode;		/* �磻�ɥ⡼��			*/
extern short	bell_mode;		/* �᡼��������Υ٥벻�⡼��	*/
extern short	x_nsubj_mode;		/* X-Nsubject: �⡼��		*/
extern short	x_nsubj_org;		/* X-Nsubject: ���ꥸ�ʥ�⡼��	*/
extern short	jst_mode;		/* ���ܻ����Ѵ��⡼��		*/
extern short	msgchk_rule;		/* �᡼������å���ˡ		*/
extern short	mime_format;		/* MIME �ե����ޥåȽ���	*/
extern KANJICODE default_code;		/* Ƚ����ǽ���δ���������	*/
extern KANJICODE edit_code;		/* ���ǥ�������������		*/
extern KANJICODE save_code;		/* ���������ִ���������		*/
extern KANJICODE input_code;		/* 1 �����ϴ���������		*/
extern int	mime_form_char;		/* MIME �ե����ޥåȥ⡼��ʸ��	*/
extern int	(*mime_encode_func)();	/* MIME �ǥ����ɴؿ�		*/
extern int	(*mime_decode_func)();	/* MIME �ǥ����ɴؿ�		*/

extern short	head_lines;		/* �إå��饤��Կ�		*/
extern short	mode_lines;		/* �⡼�ɥ饤��Կ�		*/
extern short	color_code[];		/* ���顼������			*/
#ifdef	MOUSE
extern int	mouse_button, mouse_column, mouse_row;	/* �ޥ�������	*/
#endif	/* MOUSE */

void		help();			/* �إ��ɽ��(������ˡ)	*/
void		print_title();		/* �����ȥ�ɽ��			*/
#if	defined(__STDC__) && !defined(DONT_HAVE_DOPRNT)
void		print_mode_line(char*, ...);
					/* �⡼�ɥ饤��ɽ��		*/
void		print_full_line(char*, ...);
					/* 1 �ԥե�ɽ��			*/
#else	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
void		print_mode_line();	/* �⡼�ɥ饤��ɽ��		*/
void		print_full_line();	/* 1 �ԥե�ɽ��			*/
#endif	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
void		input_line();		/* 1 ������			*/
int		check_abort();		/* ���ǥ����å�			*/
char		*input_search_string();	/* ����ʸ��������		*/
#if	defined(__STDC__) && !defined(DONT_HAVE_DOPRNT)
int		yes_or_no(yn_mode, char*, char*, ...);
					/* Y/N ���ϳ�ǧ			*/
void		edit_fprintf(FILE*, char*, ...);
					/* ���ǥ����Ѵ���ʸ�������	*/
#else	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
int		yes_or_no();		/* Y/N ���ϳ�ǧ			*/
void		edit_fprintf();		/* ���ǥ����Ѵ���ʸ�������	*/
#endif	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
void		field_fprintf();	/* �ե�����ɥե����ޥåȽ���	*/
void		suspend();		/* �����ڥ��			*/
char		*strindex();		/* ʸ���󸡺�			*/
int		fgets2();		/* �ե����뤫�� 1 ������	*/
int		convert_code();		/* �����������Ѵ�		*/
void		convert_article_date();	/* �������եե����ޥå��Ѵ�	*/
char		*next_param();		/* ���Υѥ�᡼������		*/
int		gmt_to_jst();		/* GMT ���֤� JST �����Ѵ�	*/
int		get_jadrs();		/* ���ܸ쥢�ɥ쥹����		*/
void		get_jname();		/* ���ܸ�̾����			*/
int		check_my_adrs();	/* �����ɥ쥹�����å�		*/
int		get_key();		/* 1 ʸ����������		*/
int		get_top_position();	/* ��������ַ׻�		*/
int		check_new_mail();	/* ����᡼������å�		*/
int		exec_pager();		/* �ڡ����㵯ư			*/
int		exec_editor();		/* ���ǥ�����ư			*/
NNTP_ERR_CODE	open_nntp_server();	/* NNTP ��������³		*/
void		create_date_field();	/* Date: �ե����������		*/
void		set_mime_decode_func();	/* MIME �ǥ����ɴؿ���Ͽ	*/
void		pipe_error();		/* �ѥ��ץ��顼�ϥ�ɥ�		*/
void		force_exit();		/* ������λ����			*/
FILE		*fopen2();		/* �ե��������			*/
int		fclose2();		/* �ե�������			*/
int		funlink2();		/* �ե�������			*/
void		usage();		/* �إ��ɽ��			*/

int		net_config();		/* �ͥåȥ���ե����졼�����	*/
int		env_config();		/* �Ķ�����ե����졼�����	*/
void		setup_mono();		/* ��Υ���⡼�ɥ��顼����	*/
void		create_msg_id();	/* Message-ID ������		*/

#ifdef	DONT_USE_ABORT
#define	term_enable_abort()
#define	term_disable_abort()
#define	check_abort()		0
#else	/* !DONT_USE_ABORT */
#ifdef	USE_SELECT_ABORT
#define	term_enable_abort()
#define	term_disable_abort()
#else	/* !USE_SELECT_ABORT */
#define	check_abort()	(term_abort_flag)
#endif	/* !USE_SELECT_ABORT */
#endif	/* !DONT_USE_ABORT */
