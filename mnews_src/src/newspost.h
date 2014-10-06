/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : News post define
 *  File        : newspost.h
 *  Version     : 1.21
 *  First Edit  : 1992-08/03
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 宅間 顯
 *
 */

#define	POSTER		"poster"
#define	MAX_POST_SIZE	(60L * 1024L)

#define	PF_FROM_MASK	0x0001
#define	PF_SUBJ_MASK	0x0002
#define	PF_ORGAN_MASK	0x0004
#define	PF_NG_MASK	0x0008
#define	PF_PATH_MASK	0x0010
#define	PF_LINE_MASK	0x0020
#define	PF_MIME_MASK	0x0040
#define	PF_CONTENT_MASK	0x0080
#define	PF_MAIL_MASK	0x0100
#define	PF_FILE_MASK	0x0200
#define	PF_ETC_MASK	0x0800

#define	PF_THROUGH_MASK	0x1000		/* MIME エンコード抑制指定	*/
#define	PF_DELETE_MASK	0x2000		/* 強制削除指定			*/

#define	FOLLOW_QUOTE_MASK	0x01	/* 引用				*/

extern short	add_lines_mode;		/* Lines: 付加モード		*/
extern short	inews_sig_mode;		/* inews の .signature モード	*/
extern short	ask_distrib_mode;	/* Distribution: フィールド入力	*/
					/* モード			*/
extern short	auto_sig_mode;		/* .signature 自動付加モード	*/
extern short	from_mode;		/* From 書式モード		*/
extern char	post_filter[];		/* ニュース投稿フィルタ		*/
extern char	organization[];		/* 所属				*/
extern char	news_field[];		/* ニュース自動挿入フィールド	*/
extern char	reply_indicator[];	/* インジケータ			*/
extern char	follow_message[];	/* フォローメッセージ		*/

int		news_post();		/* ニュース投稿			*/
int		news_follow();		/* ニュースフォロー		*/
int		news_cancel();		/* ニュースキャンセル		*/
int		exec_post();		/* ニュース投稿(下位関数)	*/
