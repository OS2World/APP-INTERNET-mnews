/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Group select define
 *  File        : group.h
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 宅間 顯
 *
 */

#define	NEWS_JN_DOMAIN		"newsgroupname"

#define	UNSUB_MASK	0x01		/* 1 でなければならない */
#define	TRANS_MASK	0x02

enum post_mail_mode {
  POST_MH = POST_UNKNOWN + 1,
  POST_RMAIL,
  POST_UCBMAIL,
  POST_BOARD
};

/*
 * 選択グループ管理構造体
 */

typedef	struct select_struct	SELECT_LIST;
struct select_struct {
  short		group_index;		/* ニュースグループインデックス	*/
  int		max_article;		/* 最大記事番号			*/
  long		unread_article;		/* 未読記事数			*/
  short		group_mode;		/* ニュースグループモード	*/
  short		mark_mode;		/* マークモード			*/
};

extern short	gnus_mode;		/* GNUS ライクモード		*/
extern short	group_mask;		/* グループマスクモード		*/
extern char	select_name[];		/* 選択されている名前		*/
extern char	jump_name[];		/* ジャンプする名前		*/
extern int	skip_direction;		/* グループ検索方向		*/

int		news_group_menu();	/* グループ選択メニュー		*/
int		news_prev_unread_group();
					/* 前未読グループ取得		*/
int		news_next_unread_group();
					/* 次未読グループ取得		*/
#ifdef	notdef
int		gnus_prev_group();	/* GNUS モード前 NG 取得	*/
int		gnus_next_group();	/* GNUS モード次 NG 取得	*/
#endif	/* notdef */
