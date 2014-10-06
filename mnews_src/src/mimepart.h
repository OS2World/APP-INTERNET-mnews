/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1996-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : MIME multipart select define
 *  File        : mimepart.h
 *  Version     : 1.21
 *  First Edit  : 1996-12/08
 *  Last  Edit  : 1997-09/30
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	__MIMEPART_H__
#define	__MIMEPART_H__

#define	X_BIN_FILE_FIELD	"X-application/octet-stream:"
#define	MIME_ALLOC_COUNT	8
#define	MIME_SUBJECT_LEN	46

/*
 * MIME コンテンツ構造体
 */

typedef struct content_list	CONTENT_LIST;
struct content_list {
  CONTENT_LIST	*next_ptr;
  MIME_MTYPE	c_mtype;
  MIME_STYPE	c_stype;
  MIME_TTYPE	c_ttype;
  char		c_name[PATH_BUFF];
  long		c_size;
};

extern char	mpeg_command[];		/* MPEG  再生コマンド		*/
extern char	jpeg_command[];		/* JPEG  表示コマンド		*/
extern char	gif_command[];		/* GIF   表示コマンド		*/
extern char	audio_command[];	/* AUDIO 再生コマンド		*/
extern char	ps_command[];		/* PostScript 表示コマンド	*/

extern int	mime_manage();		/* MIME 処理			*/
extern int	mime_part_menu();	/* MIME パートメニュー		*/
extern void	mime_init_content();	/* MIME コンテンツ初期化	*/
extern int	mime_add_content();	/* MIME コンテンツ登録		*/
extern int	mime_get_content_num();	/* MIME コンテンツ数取得	*/
#endif	/* !__MIMEPART_H__ */
