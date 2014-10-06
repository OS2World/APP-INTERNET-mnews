/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : MH support define
 *  File        : mh.h
 *  Version     : 1.21
 *  First Edit  : 1992-07/16
 *  Last  Edit  : 1997-11/04
 *  Author      : MSR24 ��� ��
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
 * �ե�������ѥ졼��
 */

#define	FOLDER_SEPARATER	'/'

#define	MH_READ_FIELD		"Seen:"
#define	MH_REPLY_FIELD		"Replied:"
#define	MH_FORWARD_FIELD	"Forwarded:"

/*
 * MH ���ޥ��
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
 * �ե�����ꥹ��
 */

struct mh_folder {
  char		folder_name[MH_MAX_FOLDER_NAME];/* �ե����̾		*/
  int		current_article;		/* �����ȵ����ֹ�	*/
  int		max_article;			/* ���絭���ֹ�		*/
  int		min_article;			/* �Ǿ������ֹ�		*/
};

/*
 * �ե�����ꥹ��
 */

struct mh_field {
  char		*field_name;		/* �ե������̾			*/
  char		*field_buff;		/* �ե�����ɥХåե�		*/
  char		*field_default;		/* �ե�����ɥǥե������	*/
};

extern short	mh_mode;		/* MH �⡼�ɥե饰		*/
extern short	mh_select_mode;		/* MH ��ư����⡼��		*/
extern char	mh_command_path[];	/* MH ���ޥ�ɥѥ�		*/

int		mh_init();		/* MH �⡼�ɽ����		*/
int		mh_menu();		/* MH ��˥塼			*/
int		save_mh_folder();	/* MH �ե�����ص���������	*/
int		mh_expand_alias();	/* MH �����ꥢ��Ÿ��		*/
int		mh_chkmsg();		/* MH ����᡼������å�	*/
int		mh_create_folder();	/* MH �ե��������		*/
