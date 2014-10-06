/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : BOARD support define
 *  File        : board.h
 *  Version     : 1.21
 *  First Edit  : 1992-10/05
 *  Last  Edit  : 1997-10/12
 *  Author      : MSR24 ��� ��
 *
 */

#define	BOARD_MAX_GROUP_NAME	128
#if	defined(SMALL) || (defined(MSDOS) && !defined(__GO32__) && !defined(__WIN32__) && !defined(X68K))
/*
 * 16bit �� MS-DOS �Ǥ� malloc �� 64KB ���ɤ����뤿���礭�����ƤϤ����ޤ���
 */
#define	BOARD_MAX_ARTICLE	256
#else	/* !(SMALL || (MSDOS && !__GO32__ && !__WIN32__ & !X68K)) */
#define	BOARD_MAX_ARTICLE	1024
#endif	/* !(SMALL || (MSDOS && !__GO32__ && !__WIN32__ & !X68K)) */
#define	BOARD_ALLOC_COUNT	8
#define	BOARD_JN_DOMAIN		"boardgroupname"

/*
 * ���롼�ץ��ѥ졼��
 */

#define	BOARD_GROUP_SEPARATER	'/'

/*
 * �ܡ��ɥ�����ȥե�����
 */

#ifdef	BOARD_COUNT
#if	defined(MSDOS) && !defined(USE_LONG_FNAME)
#define	BOARD_COUNT_FILE	"_count"
#else	/* !MSDOS || USE_LONG_FNAME */
#define	BOARD_COUNT_FILE	".count"
#endif	/* !MSDOS || USE_LONG_FNAME */
#endif	/* BOARD_COUNT */

/*
 * ���롼�ץꥹ��
 */

struct board_group {
  char		group_name[BOARD_MAX_GROUP_NAME];	/* ���롼��̾	*/
  int		max_article;				/* ���絭���ֹ�	*/
  int		min_article;				/* �Ǿ������ֹ�	*/
  int		unread_article;				/* ̤�ɵ�����	*/
  short		unsubscribe;				/* ���ɥ⡼��	*/
  MARK_LIST	*mark_ptr;
};

extern short	board_mode;		/* BOARD �⡼�ɥե饰		*/
extern short	board_count_mode;	/* BOARD ������������ȥե饰	*/
extern char	board_spool[];		/* BOARD ���ס���ѥ�		*/
extern char	board_mark_file[];	/* BOARD �ޡ����ե�����		*/

int		board_init();		/* BOARD �⡼�ɽ����		*/
int		board_menu();		/* BOARD ��˥塼		*/
int		board_prev_unread_group();
					/* BOARD ��̤�ɥ��롼�׼���	*/
int		board_next_unread_group();
					/* BOARD ��̤�ɥ��롼�׼���	*/
