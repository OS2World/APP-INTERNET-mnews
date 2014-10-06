/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Group select define
 *  File        : group.h
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 ��� ��
 *
 */

#define	NEWS_JN_DOMAIN		"newsgroupname"

#define	UNSUB_MASK	0x01		/* 1 �Ǥʤ���Фʤ�ʤ� */
#define	TRANS_MASK	0x02

enum post_mail_mode {
  POST_MH = POST_UNKNOWN + 1,
  POST_RMAIL,
  POST_UCBMAIL,
  POST_BOARD
};

/*
 * ���򥰥롼�״�����¤��
 */

typedef	struct select_struct	SELECT_LIST;
struct select_struct {
  short		group_index;		/* �˥塼�����롼�ץ���ǥå���	*/
  int		max_article;		/* ���絭���ֹ�			*/
  long		unread_article;		/* ̤�ɵ�����			*/
  short		group_mode;		/* �˥塼�����롼�ץ⡼��	*/
  short		mark_mode;		/* �ޡ����⡼��			*/
};

extern short	gnus_mode;		/* GNUS �饤���⡼��		*/
extern short	group_mask;		/* ���롼�ץޥ����⡼��		*/
extern char	select_name[];		/* ���򤵤�Ƥ���̾��		*/
extern char	jump_name[];		/* �����פ���̾��		*/
extern int	skip_direction;		/* ���롼�׸�������		*/

int		news_group_menu();	/* ���롼�������˥塼		*/
int		news_prev_unread_group();
					/* ��̤�ɥ��롼�׼���		*/
int		news_next_unread_group();
					/* ��̤�ɥ��롼�׼���		*/
#ifdef	notdef
int		gnus_prev_group();	/* GNUS �⡼���� NG ����	*/
int		gnus_next_group();	/* GNUS �⡼�ɼ� NG ����	*/
#endif	/* notdef */
