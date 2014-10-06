/*
 *
 *  MIME 処理ライブラリ
 *
 *  Copyright 1993-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : MIME Manage Library
 *  Sub system  : MIME encode/decode define
 *  File        : mimelib.h
 *  Version     : 0.64
 *  First Edit  : 1993-08/09
 *  Last  Edit  : 1997-11/30
 *  Author      : MSR24 宅間 顯
 *
 */

#ifndef	__MIMELIB_H__
#define	__MIMELIB_H__

#ifdef	WINDOWS
#include	<windows.h>
#endif	/* WINDOWS */

#define	SUPPORT_Q_ENCODE
#define	SUPPORT_Q_DECODE
#define	SUPPORT_QP_ENCODE
#define	SUPPORT_QP_DECODE

#define	MIME_ENCODE_START	"=?"
#define	MIME_ENCODE_END		"?="
#define	MIME_SEP_CHAR		'?'
#define	MIME_PAD_CHAR		'='
#define	MIME_SPC_CHAR		'_'

#define	MIME_ENCODE_BASE64	'B'	/* Base 64 Encoding */
#ifdef	notdef
#define	MIME_ENCODE_QP		'Q'	/* Quoted Printable Encoding */
#else	/* !notdef */
#define	MIME_ENCODE_Q		'Q'	/* Quoted */
#endif	/* !notdef */

#define	MIME_CHARSET_ASCII	"US-ASCII"
#define	MIME_CHARSET_ISO8859	"ISO-8859-"
#define	MIME_CHARSET_ISO2022_1	"ISO-2022-JP"
#define	MIME_CHARSET_ISO2022_2	"ISO-2022-JP-2"
#define	MIME_CHARSET_UNKNOWN	"UNKNOWN"

#define	CONTENT_ID_FIELD	"Content-ID:"
#define	CONTENT_TYPE_FIELD	"Content-Type:"
#define	CONTENT_TRANS_FIELD	"Content-Transfer-Encoding:"
#define	CONTENT_DPOS_FIELD	"Content-Disposition:"
#define	CONTENT_LEN_FIELD	"Content-Length:"
#define	MIME_VER_FIELD		"Mime-Version:"

#define	MIME_VERSION		"1.0"

/*
 * MIME パラメータ
 */

#define	MIME_CHARSET		"charset="
#define	MIME_ACC_TYPE		"access-type="
#define	MIME_EXPIRATION		"expiration="
#define	MIME_SIZE		"size="
#define	MIME_PERMISSION		"permission="
#define	MIME_NAME		"name="
#define	MIME_SITE		"site="
#define	MIME_DIR		"dir="
#define	MIME_MODE		"mode="
#define	MIME_SERVER		"server="
#define	MIME_SUBJECT		"subject="
#define	MIME_TYPE		"type="
#define	MIME_PADDING		"padding="
#define	MIME_BOUNDARY		"boundary="
#define	MIME_ID			"id="
#define	MIME_NUMBER		"number="
#define	MIME_TOTAL		"total="
#define	MIME_FILENAME		"filename="

#define	MIME_PARAM_QUOTE	'\"'
#define	MIME_PARAM_SEPARATER	';'

/*
 * MIME メインタイプ
 */

#define	MIME_MTYPE_TEXT		"text"
#define	MIME_MTYPE_APP		"application"
#define	MIME_MTYPE_AUDIO	"audio"
#define	MIME_MTYPE_IMAGE	"image"
#define	MIME_MTYPE_MSG		"message"
#define	MIME_MTYPE_VIDEO	"video"
#define	MIME_MTYPE_EXT		"extension-token"
#define	MIME_MTYPE_MULTI	"multipart"

typedef enum mime_mtype {
  M_MTYPE_UNKNOWN,
  M_MTYPE_TEXT,
  M_MTYPE_APP,
  M_MTYPE_AUDIO,
  M_MTYPE_IMAGE,
  M_MTYPE_MSG,
  M_MTYPE_VIDEO,
  M_MTYPE_EXT,
  M_MTYPE_MULTI
} MIME_MTYPE;

/*
 * MIME サブタイプ
 */

#define	MIME_STYPE_PLAIN	"plain"
#define	MIME_STYPE_RICH		"richtext"
#define	MIME_STYPE_MIX		"mixed"
#define	MIME_STYPE_PARA		"parallel"
#define	MIME_STYPE_DIGEST	"digest"
#define	MIME_STYPE_ALT		"alternative"
#define	MIME_STYPE_RFC		"rfc822"
#define	MIME_STYPE_PART		"partial"
#define	MIME_STYPE_EXT		"external-body"
#define	MIME_STYPE_GIF		"gif"
#define	MIME_STYPE_JPEG		"jpeg"
#define	MIME_STYPE_BASIC	"basic"
#define	MIME_STYPE_MPEG		"mpeg"
#define	MIME_STYPE_OCT		"octet-stream"
#define	MIME_STYPE_PS		"postscript"
#define	MIME_STYPE_HTML		"html"

typedef enum mime_stype {
  M_STYPE_UNKNOWN,
  M_STYPE_PLAIN,
  M_STYPE_RICH,
  M_STYPE_MIX,
  M_STYPE_PARA,
  M_STYPE_DIGEST,
  M_STYPE_ALT,
  M_STYPE_RFC,
  M_STYPE_PART,
  M_STYPE_EXT,
  M_STYPE_GIF,
  M_STYPE_JPEG,
  M_STYPE_BASIC,
  M_STYPE_MPEG,
  M_STYPE_OCT,
  M_STYPE_PS,
  M_STYPE_HTML
} MIME_STYPE;

/*
 * MIME 転送エンコードタイプ
 */

#define	MIME_TRANS_7BIT		"7bit"
#define	MIME_TRANS_8BIT		"8bit"
#define	MIME_TRANS_QP		"quoted-printable"
#define	MIME_TRANS_BASE64	"base64"
#define	MIME_TRANS_BIN		"binary"

typedef enum mime_ttype {
  M_TTYPE_UNKNOWN,
  M_TTYPE_7BIT,
  M_TTYPE_8BIT,
  M_TTYPE_QP,
  M_TTYPE_B64,
  M_TTYPE_BIN
} MIME_TTYPE;

#define	MAX_ENCODE_COLUMN	75
#define	MIME_BIN_SIZE		54	/* バイナリ 1 行サイズ	*/
#define	MIME_SEP_SIZE1		16	/* "=?ISO-2022-JP?B?"	*/
#define	MIME_SEP_SIZE2		2	/* "?="			*/

/*
 * define すると B64 エンコード文字列に隣接する空白エンコード/デコード
 * ルールを適用する
 */

/*
#define	ENCODE_SPACE
*/

#ifdef	__STDC__
#ifdef	WINDOWS

/* "Ignoring unrecognized code in MIME-Q." */
#define	MIME_ERROR_CODE_1	1
/* "Ignoring unrecognized boundary line in MIME-B64." */
#define	MIME_ERROR_CODE_2	2
/* "Unexpected EOL in MIME-B64." */
#define	MIME_ERROR_CODE_3	3

#ifdef	BUILD_MIMELIB_DLL
#define	MIMELIBFUNC __stdcall __export
#else	/* !BUILD_MIMELIB_DLL */
#define	MIMELIBFUNC __stdcall __import
#endif	/* !BUILD_MIMELIB_DLL */
#else	/* !WINDOWS */
#define	MIMELIBFUNC
#endif	/* !WINDOWS */
#endif	/* __STDC__ */

#ifdef	__STDC__
extern int MIMELIBFUNC	mime_encode_jis(char*, char*, int);
					/* MIME エンコード(JIS 出力)	*/
extern int MIMELIBFUNC	mime_decode_euc(char*, char*, int);
					/* MIME デコード(EUC 出力)	*/
extern int MIMELIBFUNC	mime_encode(char*, char*);
					/* MIME エンコード		*/
extern int MIMELIBFUNC	mime_decode(char*, char*);
					/* MIME デコード		*/
extern int MIMELIBFUNC	mime_encode_file(MIME_TTYPE, FILE*, FILE*);
					/* MIME ファイルエンコード	*/
extern int MIMELIBFUNC	mime_decode_file(MIME_TTYPE, FILE*, FILE*);
					/* MIME ファイルデコード	*/
extern char		*mime_encodeB64(char*, char*);
					/* MIME B64 エンコード		*/
extern char		*mime_decodeB64(char*, char*);
					/* MIME B64 デコード		*/
#ifdef	SUPPORT_Q_ENCODE
extern char		*mime_encodeQ(char*, char*);
					/* MIME Q エンコード		*/
#endif	/* SUPPORT_Q_ENCODE */
#ifdef	SUPPORT_Q_DECODE
extern char		*mime_decodeQ(char*, char*);
					/* MIME Q デコード		*/
#endif	/* SUPPORT_Q_DECODE */
#ifdef	SUPPORT_QP_ENCODE
extern char		*mime_encodeQP(char*, char*);
					/* MIME QP エンコード		*/
#endif	/* SUPPORT_QP_ENCODE */
#ifdef	SUPPORT_QP_DECODE
extern char		*mime_decodeQP(char*, char*);
					/* MIME QP デコード		*/
#endif	/* SUPPORT_QP_DECODE */
extern char		*mime_get_type(char*, MIME_MTYPE*, MIME_STYPE*);
					/* MIME メイン/サブタイプ取得	*/
extern int		mime_get_param(char*, char*, char*);
					/* MIME パラメータ取得		*/
extern int		mime_get_ttype(char*, MIME_TTYPE*);
					/* MIME 転送エンコードタイプ取得*/
#ifdef	WINDOWS
extern DWORD MIMELIBFUNC	mime_GetLastError(void);
#endif	/* WINDOWS */
#else	/* !__STDC__ */
extern int		mime_encode_jis();
					/* MIME エンコード(JIS 出力)	*/
extern int		mime_decode_euc();
					/* MIME デコード(EUC 出力)	*/
extern int		mime_encode();	/* MIME エンコード		*/
extern int		mime_decode();	/* MIME デコード		*/
extern int		mime_encode_file();
					/* MIME ファイルエンコード	*/
extern int		mime_decode_file();
					/* MIME ファイルデコード	*/
extern char		*mime_encodeB64();
					/* MIME B64 エンコード		*/
extern char		*mime_decodeB64();
					/* MIME B64 デコード		*/
#ifdef	SUPPORT_Q_ENCODE
extern char		*mime_encodeQ();/* MIME Q エンコード		*/
#endif	/* SUPPORT_Q_ENCODE */
#ifdef	SUPPORT_Q_DECODE
extern char		*mime_decodeQ();/* MIME Q デコード		*/
#endif	/* SUPPORT_Q_DECODE */
#ifdef	SUPPORT_QP_ENCODE
extern char		*mime_encodeQP();
					/* MIME QP エンコード		*/
#endif	/* SUPPORT_QP_ENCODE */
#ifdef	SUPPORT_QP_DECODE
extern char		*mime_decodeQP();
					/* MIME QP デコード		*/
#endif	/* SUPPORT_QP_DECODE */
extern char		*mime_get_type();
					/* MIME タイプ取得		*/
extern int		mime_get_param();
					/* MIME パラメータ取得		*/
extern int		mime_get_ttype();
					/* MIME 転送エンコードタイプ取得*/
#endif	/* !__STDC__ */
#endif	/* !__MIMELIB_H__ */
