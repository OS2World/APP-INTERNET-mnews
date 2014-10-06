/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Main define
 *  File        : mnews.h
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-12/23
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef		DONT_HAVE_FTIME
#include	<sys/timeb.h>
#endif		/* DONT_HAVE_FTIME */

#define	READER_NAME	"mnews"
#define	MNEWS_VERSION	"1.21"
#define	MNEWS_DATE	"1997-12/23(Tue)"
#define	ERROR_FILE	"mnewserr"

#if	defined(MSDOS) && !defined(USE_LONG_FNAME)
#ifdef	X68K
#define	SIGNATURE_FILE	"_signature"
#else	/* !X68K */
#define	SIGNATURE_FILE	"_sign"
#endif	/* !X68K */
#define	MESSAGE_FILE	"_message"
#define	DEAD_FILE	"deadmail"
#else	/* !MSDOS || USE_LONG_FNAME */
#define	SIGNATURE_FILE	".signature"
#define	MESSAGE_FILE	".message"
#define	DEAD_FILE	"dead.letter"
#endif	/* !MSDOS || USE_LONG_FNAME */

#define	FROM_JN_DOMAIN	"familyname"

#define	MAX_MULTI_NUMBER	30	/* 記事マルチ処理最大数		*/
#ifdef	LARGE
#define	ALIAS_BUFF		(BUFF_SIZE * 4)
#else	/* !LARGE */
#define	ALIAS_BUFF		BUFF_SIZE
#endif	/* !LARGE */

/*
 * その他
 */

#ifndef	LF
#define	LF	0x0a
#endif	/* !LF */
#define	REPLY_SUBJECT	"Re:"	/* リプライサブジェクト		*/
#define	REPLY_MESSAGE	"Your message of "
#define	TEXT_SEPARATER	"--text follows this line--"
#define	MULTI_END_MARK	"\001\n"/* マルチ記事のセパレータ	*/
#define	ERROR_SLEEP	3	/* エラー発生時の待ち時間	*/
#define	HEAD_LINES	3
#define	BOTTOM_LINES	1
#define	MODE_LINES	(HEAD_LINES + BOTTOM_LINES)
#define	MAX_MAIL_GROUP	6
#define	MAX_WRITE_FILE	5
#define	JST_OFFSET	(-9)

/*
 * マーク記事リスト構造体
 */

typedef	struct mark_struct	MARK_LIST;
struct mark_struct {
  MARK_LIST	*prev_ptr;				/* 前マーク情報	*/
  MARK_LIST	*next_ptr;				/* 次マーク情報	*/
  int		start_number;				/* 開始番号	*/
  int		end_number;				/* 終了番号	*/
};

/*
 * グループ管理構造体
 */

typedef	struct group_struct	GROUP_LIST;
struct group_struct {
  int		unread_article;				/* 未読記事数	*/
  MARK_LIST	*mark_ptr;				/* マーク情報	*/
  short		match_flag;				/* 適合フラグ	*/
  short		unsubscribe;				/* 購読モード	*/
  short		mark_order;				/* マーク順序	*/
};

/*
 * 無効グループ管理構造体
 */

typedef	struct bogus_struct	BOGUS_LIST;
struct bogus_struct {
  char		group_name[MAX_GROUP_NAME];		/* グループ名	*/
  BOGUS_LIST	*next_ptr;				/* 次情報	*/
  MARK_LIST	*mark_ptr;				/* マーク情報	*/
  short		mark_order;				/* マーク順序	*/
  short		unsubscribe;				/* 購読モード	*/
};

/*
 * Message-ID 管理構造体
 */

#ifdef	REF_SORT
typedef	struct message_struct	MESSAGE_LIST;
struct message_struct {
  char		msg_id[MAX_FIELD_LEN];		/* メッセージ ID	*/
  char		ref_id[MAX_FIELD_LEN];		/* リファレンス ID	*/
};
#endif	/* REF_SORT */

/*
 * 記事管理構造体
 */

typedef	struct article_struct	ARTICLE_LIST;
struct article_struct {
  int		real_number;			/* 実記事番号		*/
  char		from[MAX_FROM_LEN];		/* 差出人アドレス	*/
  char		subject[MAX_SUBJECT_LEN];	/* サブジェクト		*/
  short		mark;				/* マーク		*/
#ifdef	REF_SORT
  short		year;				/* 年			*/
#endif	/* REF_SORT */
  short		month;				/* 月			*/
  short		date;				/* 日			*/
  short		lines;				/* 行数			*/
};

typedef enum {
  NORMAL_YN_MODE,
  CARE_YN_MODE,
  FILE_YN_MODE,
  JUMP_YN_MODE,
  MIME_YN_MODE,
  SEND_YN_MODE,
  MARK_YN_MODE
} yn_mode;

/*
 * 記事管理マーク
 */

#define	READ_MARK	0x0001
#define	FORWARD_MARK	0x0002
#define	ANSWER_MARK	0x0004
#define	DELETE_MARK	0x0008
#define	MULTI_MARK	0x0010
#define	UNFETCH_MARK	0x0040
#define	CANCEL_MARK	0x0080
#define	SORT_MARK	0x0100
#define	THREAD_MARK	0x0200

/*
 * キー入力指定マスク
 */

#define	GLOBAL_MODE_MASK	0x01
#define	GROUP_MODE_MASK		0x02
#define	SUBJECT_MODE_MASK	0x04
#define	LINE_MODE_MASK		0x08
#define	YN_MODE_MASK		0x10
#define	NEWS_MODE_MASK		0x20
#define	MOUSE_MODE_MASK		0x40

#define	INPUT_SPCCUT_MASK	0x01
#define	INPUT_EXPAND_MASK	0x02
#define	INPUT_COMP_MASK		0x04
#define	INPUT_FOLDER_MASK	0x08
#define	INPUT_SHADOW_MASK	0x10

/*
 * カラーインデックス
 */

enum color_code {
  TITLE_COLOR,
  HEADER_COLOR,
  GUIDE_COLOR,
  ERROR_COLOR,
  CATEGORY_COLOR,
  MARK_COLOR,
  NUMBER_COLOR,
  FROM_COLOR,
  DATE_COLOR,
  LINE_COLOR,
  SUBJECT_COLOR,
  THREAD_COLOR,
  LOST_COLOR,
  FROM2_COLOR,
  TO_COLOR,
  CC_COLOR,
  DATE2_COLOR,
  GROUP_COLOR,
  REFER_COLOR,
  JNAMES_COLOR
};

extern GROUP_LIST	*group_list;
extern BOGUS_LIST	*bogus_list;
extern ARTICLE_LIST	*article_list;
#ifndef	MSDOS
extern struct passwd	*passwd;
#endif	/* !MSDOS */

extern char	*month_string[];	/* 月名				*/
extern char	*day_string[];		/* 曜日名			*/
extern char	*day_jstring[];		/* 曜日名(日本語)		*/
extern char	jnames_file[];		/* JNAMES データファイル	*/
extern char	domain_name[];		/* ドメイン名			*/
extern char	user_name[];		/* ユーザ名			*/
extern char	user_gecos[];		/* ユーザ情報			*/
extern char	home_dir[];		/* ホームディレクトリ		*/
extern char	dot_dir[];		/* ドットディレクトリ		*/
extern char	edit_command[];		/* エディタコマンド		*/
extern char	*new_mail_string[];	/* 新着メールメッセージ		*/
extern char	*net_opt;		/* ネットワーク接続オプション	*/
extern char	*cons_opt;		/* ターゲット構成オプション	*/
extern char	*conf_opt;		/* ターゲット設定オプション	*/
extern char	*arch_opt;		/* アーキテクチャオプション	*/
extern char	*install_host;		/* インストールホスト名		*/
extern char	*install_user;		/* インストールユーザ名		*/
extern char	*install_date;		/* インストール日時		*/
extern char	mark_file[];		/* マークファイル		*/
extern char	pager[];		/* ページャーコマンド		*/
extern char	tmp_dir[];		/* 作業ディレクトリ		*/
extern char	save_dir[];		/* 記事セーブディレクトリ	*/
extern char	second_adrs[];		/* セカンドアドレス		*/

extern int	multi_list[];		/* マルチフォロー/リプライリスト*/
extern int	multi_number;		/* マルチフォロー/リプライ数	*/
extern short	nntp_mode;		/* NNTP 使用モード		*/
extern short	smtp_mode;		/* SMTP 使用モード		*/
extern short	pop3_mode;		/* POP3 使用モード		*/
extern short	rpop_mode;		/* RPOP 使用モード		*/
extern short	pop3_del_mode;		/* POP3 削除モード		*/
extern short	pager_mode;		/* 外部ページャーモード		*/
extern short	japanese;		/* 日本語モードフラグ		*/
extern short	jnames;			/* JNAMES 使用フラグ		*/
extern short	quiet_mode;		/* 警告表示モード		*/
extern short	skip_mode;		/* ヘッダスキップモード		*/
extern short	mail_mode;		/* メール参照専用モード		*/
extern short	error_mode;		/* エラーログモード		*/
extern short	mh_mark_mode;		/* MH マークモード		*/
extern short	color_mode;		/* カラーモード			*/
extern short	wide_mode;		/* ワイドモード			*/
extern short	bell_mode;		/* メール到着時のベル音モード	*/
extern short	x_nsubj_mode;		/* X-Nsubject: モード		*/
extern short	x_nsubj_org;		/* X-Nsubject: オリジナルモード	*/
extern short	jst_mode;		/* 日本時間変換モード		*/
extern short	msgchk_rule;		/* メールチェック方法		*/
extern short	mime_format;		/* MIME フォーマット処理	*/
extern KANJICODE default_code;		/* 判別不能時の漢字コード	*/
extern KANJICODE edit_code;		/* エディタ漢字コード		*/
extern KANJICODE save_code;		/* 記事セーブ漢字コード		*/
extern KANJICODE input_code;		/* 1 行入力漢字コード		*/
extern int	mime_form_char;		/* MIME フォーマットモード文字	*/
extern int	(*mime_encode_func)();	/* MIME デコード関数		*/
extern int	(*mime_decode_func)();	/* MIME デコード関数		*/

extern short	head_lines;		/* ヘッダライン行数		*/
extern short	mode_lines;		/* モードライン行数		*/
extern short	color_code[];		/* カラーコード			*/
#ifdef	MOUSE
extern int	mouse_button, mouse_column, mouse_row;	/* マウス状態	*/
#endif	/* MOUSE */

void		help();			/* ヘルプ表示(使用方法)	*/
void		print_title();		/* タイトル表示			*/
#if	defined(__STDC__) && !defined(DONT_HAVE_DOPRNT)
void		print_mode_line(char*, ...);
					/* モードライン表示		*/
void		print_full_line(char*, ...);
					/* 1 行フル表示			*/
#else	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
void		print_mode_line();	/* モードライン表示		*/
void		print_full_line();	/* 1 行フル表示			*/
#endif	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
void		input_line();		/* 1 行入力			*/
int		check_abort();		/* 中断チェック			*/
char		*input_search_string();	/* 検索文字列入力		*/
#if	defined(__STDC__) && !defined(DONT_HAVE_DOPRNT)
int		yes_or_no(yn_mode, char*, char*, ...);
					/* Y/N 入力確認			*/
void		edit_fprintf(FILE*, char*, ...);
					/* エディタ用漢字文字列出力	*/
#else	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
int		yes_or_no();		/* Y/N 入力確認			*/
void		edit_fprintf();		/* エディタ用漢字文字列出力	*/
#endif	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
void		field_fprintf();	/* フィールドフォーマット出力	*/
void		suspend();		/* サスペンド			*/
char		*strindex();		/* 文字列検索			*/
int		fgets2();		/* ファイルから 1 行入力	*/
int		convert_code();		/* 漢字コード変換		*/
void		convert_article_date();	/* 記事日付フォーマット変換	*/
char		*next_param();		/* 次のパラメータ取得		*/
int		gmt_to_jst();		/* GMT 時間の JST 時間変換	*/
int		get_jadrs();		/* 日本語アドレス取得		*/
void		get_jname();		/* 日本語名取得			*/
int		check_my_adrs();	/* 自アドレスチェック		*/
int		get_key();		/* 1 文字キー入力		*/
int		get_top_position();	/* 再描画位置計算		*/
int		check_new_mail();	/* 新着メールチェック		*/
int		exec_pager();		/* ページャ起動			*/
int		exec_editor();		/* エディタ起動			*/
NNTP_ERR_CODE	open_nntp_server();	/* NNTP サーバ接続		*/
void		create_date_field();	/* Date: フィールド生成		*/
void		set_mime_decode_func();	/* MIME デコード関数登録	*/
void		pipe_error();		/* パイプエラーハンドラ		*/
void		force_exit();		/* 強制終了処理			*/
FILE		*fopen2();		/* ファイル作成			*/
int		fclose2();		/* ファイル取消			*/
int		funlink2();		/* ファイル削除			*/
void		usage();		/* ヘルプ表示			*/

int		net_config();		/* ネットコンフィグレーション	*/
int		env_config();		/* 環境コンフィグレーション	*/
void		setup_mono();		/* モノクロモードカラー設定	*/
void		create_msg_id();	/* Message-ID の生成		*/

#ifdef	DONT_USE_ABORT
#define	term_enable_abort()
#define	term_disable_abort()
#define	check_abort()		0
#else	/* !DONT_USE_ABORT */
#ifdef	USE_SELECT_ABORT
#define	term_enable_abort()
#define	term_disable_abort()
#else	/* !USE_SELECT_ABORT */
#define	check_abort()	(term_abort_flag)
#endif	/* !USE_SELECT_ABORT */
#endif	/* !DONT_USE_ABORT */
