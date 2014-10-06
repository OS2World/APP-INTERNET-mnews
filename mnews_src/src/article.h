/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Article select define
 *  File        : article.h
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	CONFIRM_NUMBER
#define	CONFIRM_NUMBER	256
#endif	/* !CONFIRM_NUMBER */
#define	THREAD_SUBJECT1	" | "
#define	THREAD_SUBJECT2	" ||"
#define	THREAD_SUBJECT3	"| "
#define	THREAD_CHAR	'|'
#define	MY_ADRS_CHAR	'='

#if	defined(SMALL) || (defined(MSDOS) && !defined(__GO32__) && !defined(__WIN32__) && !defined(X68K))
/*
 * 16bit の MS-DOS 版は malloc の 64KB の壁があるため大きくしてはいけません。
 */
#define	NEWS_MAX_ARTICLE	256
#else	/* !(SMALL || (MSDOS && !__GO32__ && !__WIN32__ & !X68K)) */
#define	NEWS_MAX_ARTICLE	1024
#endif	/* !(SMALL || (MSDOS && !__GO32__ && !__WIN32__ & !X68K)) */

/*
 * 自動マーク情報
 */

typedef	struct kill_struct	KILL_LIST;
struct kill_struct {
  int			mode;	/* マークモード(0:Subject,1:From)	*/
  char			field[MAX_FIELD_LEN];
  KILL_LIST		*next_ptr;
};

extern short	max_thread_number;	/* 最大自動ソート数		*/
extern short	news_article_mask;	/* ニュース記事マスクモード	*/
extern short	mail_article_mask;	/* メール記事マスクモード	*/
extern short	news_thread_mode;	/* ニューススレッドモード	*/
extern short	mail_thread_mode;	/* メールスレッドモード		*/
extern short	article_format;		/* 記事一覧フォーマット		*/
extern short	cross_mark_mode;	/* クロスポスト時マークモード	*/
extern short	my_adrs_mode;		/* 自アドレス変換モード		*/
extern short	unixfrom_mode;		/* UNIX-From モード		*/
extern short	auto_inc_mode;		/* メール自動取込モード		*/
extern char	auto_inc_folder[];	/* メール自動取込フォルダ	*/
extern char	lpr_command[];		/* 印刷コマンド			*/
extern KANJICODE pipe_code;		/* パイプ漢字コード		*/
extern KANJICODE lpr_code;		/* 印刷漢字コード		*/
extern short	sort_rule;		/* ソート方法			*/
#ifdef	REF_SORT
extern MESSAGE_LIST	*message_list;	/* メッセージリスト		*/
#endif	/* REF_SORT */

int		news_select_article();	/* 記事選択			*/
int		search_subjects();	/* 記事サブジェクト検索(複数)	*/
int		search_subject();	/* 記事サブジェクト検索(単一)	*/
int		search_articles();	/* 記事内容検索(複数)		*/
int		search_article();	/* 記事内容検索(単一)		*/
void		kill_subjects();	/* 同一サブジェクトマーク	*/
void		add_kill_list();	/* 自動マーク情報追加		*/
void		multi_mark();		/* マルチマーク設定/解除	*/
void		multi_clear();		/* マルチマーク消去		*/
void		multi_add_mark();	/* 複数記事マーク		*/
int		multi_extract();	/* 複数記事抽出			*/
void		print_articles();	/* 記事一覧表示			*/
int		sort_articles();	/* 記事ソート			*/
int		pack_articles();	/* 記事パック			*/
void		toggle_mark();		/* マーク反転/表示		*/
int		multi_save();		/* 複数記事セーブ		*/
int		multi_pipe();		/* 複数記事パイプ実行		*/
int		multi_print();		/* 複数記事印刷			*/
int		save_article();		/* 記事セーブ(下位関数)		*/
int		change_sort_rule();	/* パック/ソート方法切替	*/
void		create_temp_name();	/* 作業ファイル名作成		*/
