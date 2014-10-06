/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Configuration define
 *  File        : config.h
 *  Version     : 1.21
 *  First Edit  : 1992-08/03
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 宅間 顯
 *
 * 【注意事項】
 * ・コマンド類を複数パイプでつなげる時は全体を ( ) で囲んで下さい。
 * ・MH を使用しない場合は MH コマンドパスは指定しなくても構いません。
 * ・デフォルトのドメイン名はバイナリに埋め込む形式から DOMAIN_FILE で
 *   示されるファイルに書くように変更されました。
 *
 */

/*
 * ページャーコマンド
 */

#ifndef	EXT_PAGER
#define	EXT_PAGER	"/usr/local/bin/less"	/* 通常使用されない */
#endif	/* !EXT_PAGER */

/*
 * デフォルト表示コード
 */

#ifndef	PRINT_CODE
#define	PRINT_CODE	EUC_CODE	/* EUC,JIS,SJIS or ASCII */
#endif	/* !PRINT_CODE */

/*
 * 判別不能時の漢字コード
 */

#ifndef	DEFAULT_CODE
#define	DEFAULT_CODE	EUC_CODE	/* EUC or SJIS だが通常は変更不要 */
#endif	/* !DEFAULT_CODE */

/*
 * 記事セーブ漢字コード
 */

#ifndef	SAVE_CODE
#define	SAVE_CODE	ASCII_CODE	/* デフォルトは無変換 */
#endif	/* !SAVE_CODE */

/*
 * エディタ漢字コード
 */

#ifndef	EDIT_CODE
#define	EDIT_CODE	EUC_CODE	/* EUC,JIS,SJIS or ASCII */
#endif	/* !EDIT_CODE */

/*
 * 1 行入力漢字コード
 */

#ifndef	INPUT_CODE
#define	INPUT_CODE	ASCII_CODE	/* ASCII にすると漢字を受け付けない */
#endif	/* !INPUT_CODE */

/*
 * 印刷漢字コード
 */

#ifndef	LPR_CODE
#define	LPR_CODE	EUC_CODE	/* EUC,JIS,SJIS or ASCII */
#endif	/* !LPR_CODE */

/*
 * エディタコマンド
 */

#ifndef	EDITOR_COMMAND
#define	EDITOR_COMMAND	"/usr/local/bin/emacs -nw"
#endif	/* !EDITOR_COMMAND */

/*
 * 印刷コマンド
 */

#ifndef	LPR_COMMAND
#define	LPR_COMMAND	"lpr"
#endif	/* !LPR_COMMAND */

/*
 * メール送信コマンド
 */

#ifndef	SEND_COMMAND
#if	defined(_386BSD) || defined(__FreeBSD__) || defined(NET2) || defined(BSD44)
#define	SEND_COMMAND	"/usr/sbin/sendmail -t -om -oi"
#else	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44) */
#define	SEND_COMMAND	"/usr/lib/sendmail -t -om -oi"
#endif	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44) */
#endif	/* !SEND_COMMAND */

/*
 * ニュース投稿コマンド
 */

#ifndef	POST_COMMAND
#define	POST_COMMAND	"/usr/lib/news/inews -h"
#endif	/* !POST_COMMAND */

/*
 * MPEG 再生コマンド
 */

#ifndef	MPEG_COMMAND
#define	MPEG_COMMAND	"mpeg_play"
#endif	/* !MPEG_COMMAND */

/*
 * JPEG 表示コマンド
 */

#ifndef	JPEG_COMMAND
#define	JPEG_COMMAND	"xv"
#endif	/* !JPEG_COMMAND */

/*
 * GIF 表示コマンド
 */

#ifndef	GIF_COMMAND
#define	GIF_COMMAND	"xv"
#endif	/* !GIF_COMMAND */

/*
 * AUDIO 再生コマンド
 */

#ifndef	AUDIO_COMMAND
#define	AUDIO_COMMAND	"audioplay"
#endif	/* !AUDIO_COMMAND */

/*
 * PostScript 表示コマンド
 */

#ifndef	PS_COMMAND
#define	PS_COMMAND	"ghostview"
#endif	/* !PS_COMMAND */

/*
 * MH コマンドパス
 */

#ifndef	MH_COMMAND_PATH
#define	MH_COMMAND_PATH	"/usr/local/bin/mh"
#endif	/* !MH_COMMAND_PATH */

/*
 * MAIL スプールパス
 */

#ifndef	MAIL_SPOOL
#if	defined(_386BSD) || defined(__FreeBSD__) || defined(NET2) || defined(BSD44) || defined(SVR4)
#define	MAIL_SPOOL	"/var/mail"
#else	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44 || SVR4) */
#define	MAIL_SPOOL	"/usr/spool/mail"
#endif	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44 || SVR4) */
#endif	/* !MAIL_SPOOL */

/*
 * BOARD スプールパス
 */

#ifndef	BOARD_SPOOL
#if	defined(_386BSD) || defined(__FreeBSD__) || defined(NET2) || defined(BSD44) || defined(SVR4)
#define	BOARD_SPOOL	"/var/board"
#else	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44 || SVR4) */
#define	BOARD_SPOOL	"/usr/spool/board"
#endif	/* !(_386BSD || __FreeBSD__ || NET2 || BSD44 || SVR4) */
#endif	/* !BOARD_SPOOL */

/*
 * 初期化ファイル
 */

#ifndef	MARK_FILE
#if	defined(MSDOS) && !defined(USE_LONG_FNAME)
#define	MARK_FILE	"_newsrc"
#else	/* !MSDOS || USE_LONG_FNAME */
#define	MARK_FILE	".newsrc"
#endif	/* !MSDOS || USE_LONG_FNAME */
#endif	/* !MARK_FILE */

/*
 * 作業ディレクトリ
 */

#ifndef	TMP_DIR
#define	TMP_DIR		"/var/tmp"
#endif	/* !TMP_DIR */

/*
 * 記事セーブディレクトリ
 */

#ifndef	SAVE_DIR
#define	SAVE_DIR	"~/News"
#endif	/* !SAVE_DIR */

/*
 * メールボックスファイル
 */

#define	MBOX_FILE	"~/mbox"
#define	RMAIL_FILE	"~/RMAIL"

/*
 * コンフィグファイル
 */

#ifndef	CONFIG_FILE
#define	CONFIG_FILE	".mnews_setup"
#endif	/* !CONFIG_FILE */

/*
 * jnames データベース
 */

#ifndef	JNAMES_DB
#define	JNAMES_DB		"/usr/local/lib/jnames"
#endif	/* !JNAMES_DB */

/*
 * Organization ファイル 1992/10/27, 川上@NTTさん
 */

#ifndef	ORGAN_FILE
#define	ORGAN_FILE		"/usr/lib/news/organization"
#endif	/* !ORGAN_FILE */

/*
 * デフォルト初期化ファイル
 */

#ifndef	DEFAULT_INIT_FILE
#define	DEFAULT_INIT_FILE		"/usr/local/lib/mnews_setup"
#endif	/* !DEFAULT_INIT_FILE */

/*
 * ドメイン名記述ファイル
 */

#ifndef	DOMAIN_FILE
#define	DOMAIN_FILE		"/usr/local/lib/default-domain"
#endif	/* !DOMAIN_FILE */

/*
 * NNTP サーバー記述ファイル
 */

#ifndef	NNTP_SERVER_FILE
#define	NNTP_SERVER_FILE	"/usr/local/lib/default-server"
#endif	/* !NNTP_SERVER_FILE */

/*
 * SMTP サーバー記述ファイル
 */

#ifndef	SMTP_SERVER_FILE
#define	SMTP_SERVER_FILE	"/usr/local/lib/default-smtpsv"
#endif	/* !SMTP_SERVER_FILE */

/*
 * POP3 サーバー記述ファイル
 */

#ifndef	POP3_SERVER_FILE
#define	POP3_SERVER_FILE	"/usr/local/lib/default-pop3sv"
#endif	/* !POP3_SERVER_FILE */

/*
 * 郵便サーバー記述ファイル
 */

#ifndef	YOUBIN_SERVER_FILE
#define	YOUBIN_SERVER_FILE	"/usr/local/lib/default-mailsv"
#endif	/* !YOUBIN_SERVER_FILE */

/*
 * 認証ファイル
 */

#ifndef	AUTHORITY_FILE
#ifdef	USE_NEWSAUTH
#define	AUTHORITY_FILE		".newsauth"
#else	/* !USE_NEWSAUTH */
#define	AUTHORITY_FILE		".netrc"
#endif	/* !USE_NEWSAUTH */
#endif	/* !AUTHORITY_FILE */

/*
 * BOARD マークファイル
 */

#ifndef	BOARD_MARK_FILE
#define	BOARD_MARK_FILE		".mnews_boardrc"
#endif	/* !BOARD_MARK_FILE */

/*
 * 環境変数
 * (特殊な環境変数を使用したい場合以外は変更しないで下さい)
 */

#define	HOME_ENV		"HOME"
#define	USER_ENV		"USER"
#define	NAME_ENV		"NAME"
#define	EDITOR_ENV		"EDITOR"
#define	PAGER_ENV		"PAGER"
#define	DOMAIN_ENV		"DOMAINNAME"
#define	OFFICIAL_ENV		"OFFICIALNAME"
#define	ORGANIZATION_ENV	"ORGANIZATION"
#define	NEWSRC_ENV		"NEWSRC"
#define	MNEWSRC_ENV		"MNEWSRC"
#define	DOTDIR_ENV		"DOTDIR"
#ifdef	MSDOS
#ifdef	X68K
#define	TMPDIR_ENV		"temp"
#else	/* !X68K */
#define	TMPDIR_ENV		"TEMP"
#endif	/* !X68K */
#else	/* !MSDOS */
#define	TMPDIR_ENV		"TMPDIR"
#endif	/* !MSDOS */
#define	SAVEDIR_ENV		"SAVEDIR"
#define	MBOX_ENV		"MBOX"
#define	RMAIL_ENV		"RMAIL"
#define	NEWS_SPOOL_ENV		"NEWSSPOOL"
#define	NEWS_LIB_ENV		"NEWSLIB"
#define	MAIL_SPOOL_ENV		"MAILSPOOL"

/*
 * デフォルト設定
 * (通常は config.sh (config.jsh) で自動定義されるので変更しないで下さい)
 */

#ifndef	NNTP_MODE
#define	NNTP_MODE		1	/* NNTP 使用モード	*/
#endif	/* !NNTP_MODE */

#ifndef	SMTP_MODE
#define	SMTP_MODE		0	/* SMTP 使用モード	*/
#endif	/* !SMTP_MODE */

#ifndef	POP3_MODE
#define	POP3_MODE		0	/* POP3 使用モード	*/
#endif	/* !POP3_MODE */

#ifndef	RPOP_MODE
#define	RPOP_MODE		0	/* RPOP 使用モード	*/
#endif	/* !RPOP_MODE */

#ifndef	JAPANESE
#define	JAPANESE		1	/* 日本語モード		*/
#endif	/* !JAPANESE */

#ifndef	X_NSUBJ_MODE
#define	X_NSUBJ_MODE		0	/* X-Nsubject: モード	*/
#endif	/* !X_NSUBJ_MODE */

#ifndef	INEWS_SIG_MODE
#define	INEWS_SIG_MODE		1	/* inews が .signature を
					   勝手につけるモード	*/
#endif	/* !INEWS_SIG_MODE */

#ifndef	ASK_DISTRIB_MODE
#define	ASK_DISTRIB_MODE	1	/* Distribution: フィールド
					   入力モード		*/
#endif	/* !ASK_DISTRIB_MODE */

#ifndef	JST_MODE
#define	JST_MODE		1	/* 日本時間変換モード	*/
#endif	/* !JST_MODE */

#ifndef	DNAS_MODE
#define	DNAS_MODE		0	/* DNAS 対応モード	*/
#endif	/* !DNAS_MODE */

#ifndef	NNTP_AUTH_MODE
#define	NNTP_AUTH_MODE		1	/* NNTP 認証モード	*/
#endif	/* !NNTP_AUTH_MODE */

/*
 * デフォルト設定
 * (この部分はユーザの好みによりマチマチなので config.sh (config.jsh)
 * では変更されません。通常はユーザの ~/.mnews_setup で変更できる
 * のでここで変更する必要はありません)
 */


#define	MAX_THREAD_NUMBER	100	/* 最大自動ソート数	*/
#define	GNUS_MODE		0	/* GNUS ライクモード	*/
#define	GROUP_MASK		1	/* グループマスクモード	*/
#define	PAGER_MODE		0	/* 外部ページャーモード	*/
#define	MH_MARK_MODE		0	/* MH マークモード	*/
#define	MH_SELECT_MODE		1	/* MH 自動セレクトモード*/
#define	MH_ALIAS_MODE		0	/* MH エイリアスモード	*/
#define	QUIET_MODE		0	/* 警告表示モード	*/
#define	MAIL_MODE		0	/* メール参照専用モード	*/
#define	ERROR_MODE		0	/* エラーログモード	*/
#define	CROSS_MARK_MODE		0	/* クロスポスト時マーク	*/
#define	MH_MODE			1	/* MH モードフラグ	*/
#define	UCBMAIL_MODE		1	/* UCB-mail モードフラグ*/
#define	RMAIL_MODE		1	/* RMAIL モードフラグ	*/
#define	BOARD_MODE		1	/* BOARD モードフラグ	*/
#define	BOARD_COUNT_MODE	0	/* BOARD 記事数カウント	*/
#define	ADD_CC_MODE		1	/* Cc: 付加モード	*/
#define	REPLY_CC_MODE		1	/* Cc: リプライモード	*/
#define	ADD_LINES_MODE		0	/* Lines: 付加モード	*/
#define	WIDE_MODE		0	/* ワイド表示モード	*/
#define	BELL_MODE		1	/* ベル音モード		*/
#define	COLOR_MODE		0	/* カラーモード		*/
#define	MY_ADRS_MODE		0	/* 自アドレス変換モード	*/
#define	AUTO_SIG_MODE		0	/* .signature 自動付加	*/
#define	AUTO_INC_MODE		0	/* 記事自動取込モード	*/
#define	FROM_MODE		0	/* From 書式モード	*/
#define	UNIXFROM_MODE		0	/* UNIX-From 形式セーブ	*/
#define	XOVER_MODE		1	/* XOVER 対応モード	*/
#define	SCROLL_MODE		0	/* スクロールモード	*/
#define	POP3_DEL_MODE		1	/* POP3 削除モード	*/
#define	NEWS_ARTICLE_MASK	0	/* ニュース記事マスクモード	*/
#define	MAIL_ARTICLE_MASK	0	/* メール記事マスクモード	*/
#define	NEWS_THREAD_MODE	1	/* ニューススレッドモード	*/
#define	MAIL_THREAD_MODE	0	/* メールスレッドモード		*/

#define	ARTICLE_FORMAT		0	/* 記事一覧フォーマット	*/
#define	MIME_FORMAT		1	/* MIMEフォーマット処理	*/
#define	MSGCHK_RULE		1	/* メールチェック方法	*/
#define	SORT_RULE		0	/* ソート方法		*/

#define	REPLY_INDICATOR		">> "	/* インジケータ		*/
#define	SEND_FILTER		""	/* メール送信フィルタ	*/
#define	POST_FILTER		""	/* ニュース投稿フィルタ	*/

enum param_type {
  PARAM_SW_TYPE,
  PARAM_CODE_TYPE,
  PARAM_STRING_TYPE,
  PARAM_FORMAT_TYPE,
  PARAM_FILE1_TYPE,
  PARAM_FILE2_TYPE,
  PARAM_SELECT_TYPE,
  PARAM_GROUP1_TYPE,
  PARAM_GROUP2_TYPE,
  PARAM_COLOR_TYPE,
  PARAM_NUMBER_TYPE,
  PARAM_KILL_TYPE
};

/*
 * カラーコード
 */

#ifdef	X68K
#define	TITLE_COLOR_CODE	BG_SKYBLUE2_ATTRIB
#define	HEADER_COLOR_CODE	BG_YELLOW1_ATTRIB
#define	GUIDE_COLOR_CODE	BG_SKYBLUE1_ATTRIB
#define	ERROR_COLOR_CODE	BG_YELLOW2_ATTRIB
#define	CATEGORY_COLOR_CODE	FG_SKYBLUE1_ATTRIB
#define	MARK_COLOR_CODE		FG_YELLOW2_ATTRIB
#define	NUMBER_COLOR_CODE	FG_WHITE1_ATTRIB
#define	FROM_COLOR_CODE		FG_WHITE1_ATTRIB
#define	DATE_COLOR_CODE		FG_WHITE1_ATTRIB
#define	LINE_COLOR_CODE		FG_WHITE1_ATTRIB
#define	SUBJECT_COLOR_CODE	FG_SKYBLUE2_ATTRIB
#define	THREAD_COLOR_CODE	FG_SKYBLUE1_ATTRIB
#define	LOST_COLOR_CODE		FG_YELLOW2_ATTRIB
#define	FROM2_COLOR_CODE	FG_YELLOW2_ATTRIB
#define	TO_COLOR_CODE		FG_YELLOW2_ATTRIB
#define	CC_COLOR_CODE		FG_YELLOW1_ATTRIB
#define	DATE2_COLOR_CODE	FG_SKYBLUE1_ATTRIB
#define	GROUP_COLOR_CODE	FG_YELLOW2_ATTRIB
#define	REFER_COLOR_CODE	FG_SKYBLUE1_ATTRIB
#define	JNAMES_COLOR_CODE	FG_YELLOW1_ATTRIB
#else	/* !X68K */
#define	TITLE_COLOR_CODE	BG_BLUE_ATTRIB
#define	HEADER_COLOR_CODE	BG_RED_ATTRIB
#define	GUIDE_COLOR_CODE	BG_BLUE_ATTRIB
#define	ERROR_COLOR_CODE	BG_RED_ATTRIB
#define	CATEGORY_COLOR_CODE	FG_SKYBLUE_ATTRIB
#define	MARK_COLOR_CODE		FG_RED_ATTRIB
#define	NUMBER_COLOR_CODE	FG_WHITE_ATTRIB
#define	FROM_COLOR_CODE		FG_WHITE_ATTRIB
#define	DATE_COLOR_CODE		FG_WHITE_ATTRIB
#define	LINE_COLOR_CODE		FG_WHITE_ATTRIB
#define	SUBJECT_COLOR_CODE	FG_SKYBLUE_ATTRIB
#define	THREAD_COLOR_CODE	FG_BLUE_ATTRIB
#define	LOST_COLOR_CODE		FG_RED_ATTRIB
#define	FROM2_COLOR_CODE	FG_PURPLE_ATTRIB
#define	TO_COLOR_CODE		FG_GREEN_ATTRIB
#define	CC_COLOR_CODE		FG_GREEN_ATTRIB
#define	DATE2_COLOR_CODE	FG_YELLOW_ATTRIB
#define	GROUP_COLOR_CODE	FG_GREEN_ATTRIB
#define	REFER_COLOR_CODE	FG_GREEN_ATTRIB
#define	JNAMES_COLOR_CODE	FG_GREEN_ATTRIB
#endif	/* !X68K */
