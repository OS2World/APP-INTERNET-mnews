/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : RMAIL support define
 *  File        : rmail.h
 *  Version     : 1.21
 *  First Edit  : 1992-07/23
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 宅間 顯
 *
 */

#define	RMAIL_SEPARATER		""
#define	RMAIL_SEPARATER2	""
#define	RMAIL_EOOH		"*** EOOH ***"
#define	RMAIL_EMACS_FIELD1	"BABYL OPTIONS:\nVersion: 5\nLabels:\n"
#define	RMAIL_EMACS_FIELD2	"BABYL OPTIONS:\n"
#define	RMAIL_MULE_FIELD1	"BABYL OPTIONS: -*- rmail -*-\nVersion: 5\nLabels:\n"
#define	RMAIL_MULE_FIELD2	"BABYL OPTIONS: -*- rmail -*-\n"
#define	RMAIL_NOTE_FIELD1	"Note:   This is the header of an rmail file made by mnews %s.\n"
#define	RMAIL_NOTE_FIELD2	"Note:   If you are seeing it in rmail,\n"
#define	RMAIL_NOTE_FIELD3	"Note:    it means the file has no messages in it.\n"

#define	RMAIL_UNSEEN_ATTRIB	"unseen"
#define	RMAIL_FORWARD_ATTRIB	"forwarded"
#define	RMAIL_ANSWER_ATTRIB	"answered"
#define	RMAIL_DELETE_ATTRIB	"deleted"
#define	RMAIL_DELETE_STRING	" deleted,"

#define	RMAIL_MAX_FOLDER_NAME	128
#define	RMAIL_ALLOC_COUNT	16
#define	RMAIL_JN_DOMAIN		"rmailfoldername"

/*
 * フォルダリスト
 */

struct rmail_folder {
  char		folder_name[RMAIL_MAX_FOLDER_NAME];	/* フォルダ名	*/
  int		max_article;				/* 最大記事番号	*/
  int		unread_article;				/* 未読記事数	*/
};

extern short	rmail_mode;		/* RMAIL モードフラグ		*/
extern char	rmail_mbox[];		/* RMAIL 用メールボックス	*/

int		rmail_init();		/* RMAIL モード初期化		*/
int		rmail_menu();		/* RMAIL メニュー		*/
