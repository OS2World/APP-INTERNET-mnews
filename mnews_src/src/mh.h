/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : MH support define
 *  File        : mh.h
 *  Version     : 1.21
 *  First Edit  : 1992-07/16
 *  Last  Edit  : 1997-11/04
 *  Author      : MSR24 宅間 顯
 *
 */

#define	MH_MAX_FOLDER_NAME	128
/*
#define	MH_MAX_ARTICLE		8192
*/
#define	MH_ALLOC_COUNT		8
#define	MH_DEFAULT_PATH		"Mail"
#define	MH_DEFAULT_FOLDER	"inbox"
#define	MH_DEFAULT_F_PROTECT	(S_IREAD | S_IWRITE | S_IEXEC)
#define	MH_DEFAULT_M_PROTECT	(S_IREAD | S_IWRITE)
#define	MH_DRAFT_FOLDER		"drafts"

#if	defined(MSDOS) && !defined(USE_LONG_FNAME)
#ifdef	X68K
#define	MH_INIT_FILE		"_mh_profile"
#define	MH_SEQ_FILE		"_mh_sequences"
#else	/* !X68K */
#define	MH_INIT_FILE		"_mh_prof"
#define	MH_SEQ_FILE		"_mh_sequ"
#endif	/* !X68K */
#else	/* !MSDOS || USE_LONG_FNAME */
#define	MH_INIT_FILE		".mh_profile"
#define	MH_SEQ_FILE		".mh_sequences"
#endif	/* !MSDOS || USE_LONG_FNAME */

#define	MH_CONTEXT_FILE		"context"
#define	MH_CONTEXT_FIELD	"Current-Folder:"

#define	MH_CURRENT_FIELD	"cur:"

#define	MH_PATH_FIELD		"Path:"
#define	MH_UNSEEN_FIELD		"Unseen-Sequence:"
#define	MH_DRAFT_FIELD		"Draft-folder:"
#define	MH_F_PROTECT_FIELD	"Folder-Protect:"
#define	MH_M_PROTECT_FIELD	"Msg-Protect:"

#define	MH_JN_DOMAIN		"mhfoldername"

/*
 * フォルダセパレータ
 */

#define	FOLDER_SEPARATER	'/'

#define	MH_READ_FIELD		"Seen:"
#define	MH_REPLY_FIELD		"Replied:"
#define	MH_FORWARD_FIELD	"Forwarded:"

/*
 * MH コマンド
 */

#define	MH_FOLDER_COMMAND	"folder"
#define	MH_REFILE_COMMAND	"refile"
#define	MH_SORTM_COMMAND	"sortm"
#define	MH_RMM_COMMAND		"rmm"
#define	MH_RMF_COMMAND		"rmf"
#define	MH_INC_COMMAND		"inc"
#define	MH_PICK_COMMAND		"pick"
#define	MH_ALI_COMMAND		"ali"
#define	MH_MARK_COMMAND		"mark"
#define	MH_MSGCHK_COMMAND	"msgchk"
#define	MH_BURST_COMMAND	"burst"

/*
 * フォルダリスト
 */

struct mh_folder {
  char		folder_name[MH_MAX_FOLDER_NAME];/* フォルダ名		*/
  int		current_article;		/* カレント記事番号	*/
  int		max_article;			/* 最大記事番号		*/
  int		min_article;			/* 最小記事番号		*/
};

/*
 * フォルダリスト
 */

struct mh_field {
  char		*field_name;		/* フィールド名			*/
  char		*field_buff;		/* フィールドバッファ		*/
  char		*field_default;		/* フィールドデフォルト値	*/
};

extern short	mh_mode;		/* MH モードフラグ		*/
extern short	mh_select_mode;		/* MH 自動選択モード		*/
extern char	mh_command_path[];	/* MH コマンドパス		*/

int		mh_init();		/* MH モード初期化		*/
int		mh_menu();		/* MH メニュー			*/
int		save_mh_folder();	/* MH フォルダへ記事セーブ	*/
int		mh_expand_alias();	/* MH エイリアス展開		*/
int		mh_chkmsg();		/* MH 新着メールチェック	*/
int		mh_create_folder();	/* MH フォルダ作成		*/
