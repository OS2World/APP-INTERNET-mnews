/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Article select define
 *  File        : article.h
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 ��� ��
 *
 */

#ifndef	CONFIRM_NUMBER
#define	CONFIRM_NUMBER	256
#endif	/* !CONFIRM_NUMBER */
#define	THREAD_SUBJECT1	" | "
#define	THREAD_SUBJECT2	" ||"
#define	THREAD_SUBJECT3	"| "
#define	THREAD_CHAR	'|'
#define	MY_ADRS_CHAR	'='

#if	defined(SMALL) || (defined(MSDOS) && !defined(__GO32__) && !defined(__WIN32__) && !defined(X68K))
/*
 * 16bit �� MS-DOS �Ǥ� malloc �� 64KB ���ɤ����뤿���礭�����ƤϤ����ޤ���
 */
#define	NEWS_MAX_ARTICLE	256
#else	/* !(SMALL || (MSDOS && !__GO32__ && !__WIN32__ & !X68K)) */
#define	NEWS_MAX_ARTICLE	1024
#endif	/* !(SMALL || (MSDOS && !__GO32__ && !__WIN32__ & !X68K)) */

/*
 * ��ư�ޡ�������
 */

typedef	struct kill_struct	KILL_LIST;
struct kill_struct {
  int			mode;	/* �ޡ����⡼��(0:Subject,1:From)	*/
  char			field[MAX_FIELD_LEN];
  KILL_LIST		*next_ptr;
};

extern short	max_thread_number;	/* ���缫ư�����ȿ�		*/
extern short	news_article_mask;	/* �˥塼�������ޥ����⡼��	*/
extern short	mail_article_mask;	/* �᡼�뵭���ޥ����⡼��	*/
extern short	news_thread_mode;	/* �˥塼������åɥ⡼��	*/
extern short	mail_thread_mode;	/* �᡼�륹��åɥ⡼��		*/
extern short	article_format;		/* ���������ե����ޥå�		*/
extern short	cross_mark_mode;	/* �����ݥ��Ȼ��ޡ����⡼��	*/
extern short	my_adrs_mode;		/* �����ɥ쥹�Ѵ��⡼��		*/
extern short	unixfrom_mode;		/* UNIX-From �⡼��		*/
extern short	auto_inc_mode;		/* �᡼�뼫ư����⡼��		*/
extern char	auto_inc_folder[];	/* �᡼�뼫ư����ե����	*/
extern char	lpr_command[];		/* �������ޥ��			*/
extern KANJICODE pipe_code;		/* �ѥ��״���������		*/
extern KANJICODE lpr_code;		/* ��������������		*/
extern short	sort_rule;		/* ��������ˡ			*/
#ifdef	REF_SORT
extern MESSAGE_LIST	*message_list;	/* ��å������ꥹ��		*/
#endif	/* REF_SORT */

int		news_select_article();	/* ��������			*/
int		search_subjects();	/* �������֥������ȸ���(ʣ��)	*/
int		search_subject();	/* �������֥������ȸ���(ñ��)	*/
int		search_articles();	/* �������Ƹ���(ʣ��)		*/
int		search_article();	/* �������Ƹ���(ñ��)		*/
void		kill_subjects();	/* Ʊ�쥵�֥������ȥޡ���	*/
void		add_kill_list();	/* ��ư�ޡ��������ɲ�		*/
void		multi_mark();		/* �ޥ���ޡ�������/���	*/
void		multi_clear();		/* �ޥ���ޡ����õ�		*/
void		multi_add_mark();	/* ʣ�������ޡ���		*/
int		multi_extract();	/* ʣ���������			*/
void		print_articles();	/* ��������ɽ��			*/
int		sort_articles();	/* ����������			*/
int		pack_articles();	/* �����ѥå�			*/
void		toggle_mark();		/* �ޡ���ȿž/ɽ��		*/
int		multi_save();		/* ʣ������������		*/
int		multi_pipe();		/* ʣ�������ѥ��׼¹�		*/
int		multi_print();		/* ʣ����������			*/
int		save_article();		/* ����������(���̴ؿ�)		*/
int		change_sort_rule();	/* �ѥå�/��������ˡ����	*/
void		create_temp_name();	/* ��ȥե�����̾����		*/
