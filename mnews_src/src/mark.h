/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Mark define
 *  File        : mark.h
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 宅間 顯
 *
 */

#ifdef	TRI
/*
 * TRI 構造のインプリメント
 */

struct trienode {
  char			value;
  struct trienode	*child;		/* 子ノードへのポインタ */
  struct trienode	*next;		/* 兄弟ノードへのポインタ */
  int			group_number;	/* 普通は -1,末端ノードの場合
					 * news_group[]の番号
					 */
};
#endif	/* TRI */

extern short	dnas_mode;		/* DNAS 対応モード		*/
extern short	mark_lock;		/* 未読管理操作セマフォ		*/

void		news_read_initfile();	/* 初期化ファイルリード		*/
void		news_save_initfile();	/* 初期化ファイルセーブ		*/
void		save_group();		/* 1 グループセーブ		*/
#ifdef	DEBUG
void		news_print_mark();	/* マークリスト要素表示		*/
#endif	/* DEBUG */
void		news_add_mark();	/* マークリスト要素追加		*/
void		news_delete_mark();	/* マークリスト要素削除		*/
void		news_fill_mark();	/* マークリスト全要素追加	*/
void		news_clean_mark();	/* マークリスト全要素削除	*/
int		news_check_mark();	/* マークリストチェック		*/
void		news_count_unread();	/* 未読記事数算出		*/
int		read_integer();		/* 整数ファイル入力		*/
void		add_mark();		/* マークリスト要素追加(汎用)	*/
void		delete_mark();		/* マークリスト要素削除(汎用)	*/
void		news_delete_bogus();	/* 無効ニュースグループ削除	*/

#ifdef	TRI
struct trienode	*make_tri();		/* tri 作成			*/
void		insert_tri();		/* tri 一語挿入			*/
int		find_tri();		/* tri 一語検索			*/
struct trienode	*const_trienode();	/* tri entity 作成		*/
void		free_trienode();	/* tri メモリ解放		*/
struct trienode	*find_trienode();	/* tri 兄弟ノード中一字検索	*/
void		print_trienode();	/* tri 表示			*/
#endif	/* TRI */
