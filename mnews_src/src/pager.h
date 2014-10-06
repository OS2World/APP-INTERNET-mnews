/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1992-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Pager define
 *  File        : pager.h
 *  Version     : 1.21
 *  First Edit  : 1992-08/05
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	LF
#define	LF	0x0a
#endif	/* !LF */

#ifdef	MNEWS
#define	MAX_SHOW_FILE	4
#else	/* !MNEWS */
#define	MAX_SHOW_FILE	64
#endif	/* !MNEWS */

extern int	last_key;		/* 最後のキーコード	*/
extern char	guide_message[];	/* ガイドメッセージ	*/
extern short	scroll_mode;		/* スクロールモード	*/

int		view_file();		/* ファイル参照			*/
int		view_files();		/* 複数ファイル参照		*/
