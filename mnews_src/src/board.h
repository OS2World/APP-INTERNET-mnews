/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : BOARD support define
 *  File        : board.h
 *  Version     : 1.21
 *  First Edit  : 1992-10/05
 *  Last  Edit  : 1997-10/12
 *  Author      : MSR24 宅間 顯
 *
 */

#define	BOARD_MAX_GROUP_NAME	128
#if	defined(SMALL) || (defined(MSDOS) && !defined(__GO32__) && !defined(__WIN32__) && !defined(X68K))
/*
 * 16bit の MS-DOS 版は malloc の 64KB の壁があるため大きくしてはいけません。
 */
#define	BOARD_MAX_ARTICLE	256
#else	/* !(SMALL || (MSDOS && !__GO32__ && !__WIN32__ & !X68K)) */
#define	BOARD_MAX_ARTICLE	1024
#endif	/* !(SMALL || (MSDOS && !__GO32__ && !__WIN32__ & !X68K)) */
#define	BOARD_ALLOC_COUNT	8
#define	BOARD_JN_DOMAIN		"boardgroupname"

/*
 * グループセパレータ
 */

#define	BOARD_GROUP_SEPARATER	'/'

/*
 * ボードカウントファイル
 */

#ifdef	BOARD_COUNT
#if	defined(MSDOS) && !defined(USE_LONG_FNAME)
#define	BOARD_COUNT_FILE	"_count"
#else	/* !MSDOS || USE_LONG_FNAME */
#define	BOARD_COUNT_FILE	".count"
#endif	/* !MSDOS || USE_LONG_FNAME */
#endif	/* BOARD_COUNT */

/*
 * グループリスト
 */

struct board_group {
  char		group_name[BOARD_MAX_GROUP_NAME];	/* グループ名	*/
  int		max_article;				/* 最大記事番号	*/
  int		min_article;				/* 最小記事番号	*/
  int		unread_article;				/* 未読記事数	*/
  short		unsubscribe;				/* 購読モード	*/
  MARK_LIST	*mark_ptr;
};

extern short	board_mode;		/* BOARD モードフラグ		*/
extern short	board_count_mode;	/* BOARD 記事数カウントフラグ	*/
extern char	board_spool[];		/* BOARD スプールパス		*/
extern char	board_mark_file[];	/* BOARD マークファイル		*/

int		board_init();		/* BOARD モード初期化		*/
int		board_menu();		/* BOARD メニュー		*/
int		board_prev_unread_group();
					/* BOARD 前未読グループ取得	*/
int		board_next_unread_group();
					/* BOARD 次未読グループ取得	*/
