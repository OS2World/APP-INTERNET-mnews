/*
 *
 *  System      : Mini News Reader
 *  Sub system  : Site depend define [for MS-DOS,OS/2 only]
 *  File        : site_DOS.h
 *  Version     : 1.21
 *
 *  NOTE:
 *   This file is used for MS-DOS,OS/2 only.
 *
 */

/*
 * If you define DOMAIN_NAME, DOMAIN_NAME used in
 * executable file, and DOMAIN_FILE does not need.
 */
/*
#define	DOMAIN_NAME	"msr.mei.co.jp"
*/
#ifdef	USE_LONG_FNAME
#define	DEFAULT_INIT_FILE	"C:\\usr\\local\\lib\\mnews_setup"
#define	DOMAIN_FILE		"C:\\usr\\local\\lib\\default-domain"
#define	NNTP_SERVER_FILE	"C:\\usr\\local\\lib\\default-server"
#define	SMTP_SERVER_FILE	"C:\\usr\\local\\lib\\default-smtpsv"
#define	POP3_SERVER_FILE	"C:\\usr\\local\\lib\\default-pop3sv"
#define	YOUBIN_SERVER_FILE	"C:\\usr\\local\\lib\\default-mailsv"
#define	ORGAN_FILE		"C:\\usr\\lib\\news\\organization"
#define	CONFIG_FILE		".mnews_setup"
#ifdef	USE_NEWS_AUTH
#define	AUTHORITY_FILE		".newsauth"
#else	/* !USE_NEWS_AUTH */
#define	AUTHORITY_FILE		".netrc"
#endif	/* !USE_NEWS_AUTH */
#define	BOARD_MARK_FILE		".mnews_boardrc"
#else	/* !USE_LONG_FNAME */
#define	DEFAULT_INIT_FILE	"C:\\usr\\local\\lib\\mnews.sup"
#define	DOMAIN_FILE		"C:\\usr\\local\\lib\\domain.def"
#define	NNTP_SERVER_FILE	"C:\\usr\\local\\lib\\server.def"
#define	SMTP_SERVER_FILE	"C:\\usr\\local\\lib\\smtpsv.def"
#define	POP3_SERVER_FILE	"C:\\usr\\local\\lib\\pop3sv.def"
#define	YOUBIN_SERVER_FILE	"C:\\usr\\local\\lib\\mailsv.def"
#define	ORGAN_FILE		"C:\\usr\\lib\\news\\organ.def"
#define	CONFIG_FILE		"_mnews.sup"
#ifdef	USE_NEWS_AUTH
#define	AUTHORITY_FILE		"_newsaut"
#else	/* !USE_NEWS_AUTH */
#define	AUTHORITY_FILE		"_netrc"
#endif	/* !USE_NEWS_AUTH */
#define	BOARD_MARK_FILE		"_boardrc"
#endif	/* !USE_LONG_FNAME */

#define	EXT_PAGER	"less.exe"
#define	PRINT_CODE	SJIS_CODE
#define	DEFAULT_CODE	SJIS_CODE
#define	SAVE_CODE	ASCII_CODE
#define	EDIT_CODE	SJIS_CODE
#define	INPUT_CODE	SJIS_CODE
#define	FCC_CODE	ASCII_CODE
#define	PIPE_CODE	ASCII_CODE
#define	LPR_CODE	SJIS_CODE
#define	EDITOR_COMMAND	"mule"
#define	SEND_COMMAND	"sendmail"
#define	POST_COMMAND	"inews"
#define	LPR_COMMAND	""
#define	MH_COMMAND_PATH	""
#define	JNAMES_DB	"C:\\usr\\local\\lib\\jnames"
#define	NEWS_SPOOL	"C:\\usr\\spool\\news"
#define	NEWS_LIB	"C:\\usr\\lib\\news"
#define	MAIL_SPOOL	"C:\\usr\\spool\\mail"
#define	BOARD_SPOOL	"C:\\usr\\spool\\board"
#define	TMP_DIR		"."

/*
 * MIME multimedia extension
 */

#define	MPEG_COMMAND	""
#define	JPEG_COMMAND	""
#define	GIF_COMMAND	""
#define	AUDIO_COMMAND	""
#define	PS_COMMAND	""

/*
#define	IGNORE_GROUPS	""
*/

#define	NNTP_MODE		1
#define	SMTP_MODE		0
#define	POP3_MODE		0
#define	RPOP_MODE		0
#define	JAPANESE		1
#define	X_NSUBJ_MODE		0
#define	INEWS_SIG_MODE		0
#define	JST_MODE		1
#define	DNAS_MODE		0
#define	NNTP_AUTH_MODE		1
