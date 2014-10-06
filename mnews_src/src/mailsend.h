/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Mail send define
 *  File        : mailsend.h
 *  Version     : 1.21
 *  First Edit  : 1992-07/29
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 宅間 顯
 *
 */

#ifdef	MSDOS
#define	MAILRC_FILE	"_mailrc"	/* エイリアス設定ファイル	*/
#else	/* !MSDOS */
#define	MAILRC_FILE	".mailrc"	/* エイリアス設定ファイル	*/
#endif	/* !MSDOS */
#define	ALIAS_PARAM1	"alias"		/* エイリアスパラメータ(1)	*/
#define	ALIAS_PARAM2	"group"		/* エイリアスパラメータ(2)	*/
#define	MAX_SEND_SIZE	(48L * 1024L)

#define	FORWARD_START_MESSAGE	"------- Forwarded Message\n"
#define	FORWARD_END_MESSAGE	"\n------- End of Forwarded Message"

#define	SF_FROM_MASK	0x0001
#define	SF_SUBJ_MASK	0x0002
#define	SF_TO_MASK	0x0004
#define	SF_CC_MASK	0x0008
#define	SF_BCC_MASK	0x0010
#define	SF_MIME_MASK	0x0020
#define	SF_CONTENT_MASK	0x0040
#define	SF_FILE_MASK	0x0080
#define	SF_ETC_MASK	0x0800

#define	SF_THROUGH_MASK	0x1000		/* MIME エンコード抑制指定	*/
#define	SF_DELETE_MASK	0x2000		/* 強制削除指定			*/
#define	SF_EXPAND_MASK	0x4000		/* エイリアス展開指定		*/

#define	REPLY_QUOTE_MASK	0x01	/* 引用				*/
#define	REPLY_BOARD_MASK	0x02	/* BOARD リプライモード		*/
#define	FOLLOW_BOARD_MASK	0x04	/* BOARD フォローモード		*/

extern short	add_cc_mode;		/* Cc: 付加モードフラグ		*/
extern short	reply_cc_mode;		/* Cc: リプライモードフラグ	*/
extern short	auto_sig_mode;		/* .signature 自動付加モード	*/
extern short	from_mode;		/* From 書式モード		*/
extern short	mh_alias_mode;		/* MH エイリアスモード		*/
extern char	send_filter[];		/* メール送信フィルタ		*/
extern char	mail_field[];		/* メール自動挿入フィールド	*/
extern char	reply_indicator[];	/* インジケータ			*/
extern char	follow_message[];	/* フォローメッセージ		*/
extern char	reply_message[];	/* リプライメッセージ		*/
extern char	forward_start[];	/* フォワード開始メッセージ	*/
extern char	forward_end[];		/* フォワード終了メッセージ	*/

int		mail_send();		/* メール送信			*/
int		mail_reply();		/* メール返信			*/
int		mail_forward();		/* メール転送			*/
int		mail_relay();		/* メール回送			*/
int		exec_send();		/* メール送信(下位関数)		*/
