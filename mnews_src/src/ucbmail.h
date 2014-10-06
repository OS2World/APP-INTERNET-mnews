/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : UCB-mail support define
 *  File        : ucbmail.h
 *  Version     : 1.21
 *  First Edit  : 1992-07/27
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 宅間 顯
 *
 */

#define	UCBMAIL_STATUS_FIELD	"Status:"
#define	UCBMAIL_OLD_STATUS	"O"
#define	UCBMAIL_UNREAD_STATUS	"U"
#define	UCBMAIL_READ_STATUS	"RO"

#define	UCBMAIL_MAX_FOLDER_NAME	128
#define	UCBMAIL_ALLOC_COUNT	16
#define	UCBMAIL_JN_DOMAIN	"ucbmailfoldername"

/*
 * フォルダリスト
 */

struct ucbmail_folder {
  char		folder_name[UCBMAIL_MAX_FOLDER_NAME];	/* フォルダ名	*/
  int		max_article;				/* 最大記事番号	*/
  int		unread_article;				/* 未読記事数	*/
};

extern short	ucbmail_mode;		/* UCB-mail モードフラグ	*/
extern char	ucbmail_mbox[];		/* UCB-mail 用メールボックス	*/

int		ucbmail_init();		/* UCB-mail モード初期化	*/
int		ucbmail_menu();		/* UCB-mail メニュー		*/
