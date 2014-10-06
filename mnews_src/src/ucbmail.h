/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : UCB-mail support define
 *  File        : ucbmail.h
 *  Version     : 1.21
 *  First Edit  : 1992-07/27
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 ��� ��
 *
 */

#define	UCBMAIL_STATUS_FIELD	"Status:"
#define	UCBMAIL_OLD_STATUS	"O"
#define	UCBMAIL_UNREAD_STATUS	"U"
#define	UCBMAIL_READ_STATUS	"RO"

#define	UCBMAIL_MAX_FOLDER_NAME	128
#define	UCBMAIL_ALLOC_COUNT	16
#define	UCBMAIL_JN_DOMAIN	"ucbmailfoldername"

/*
 * �ե�����ꥹ��
 */

struct ucbmail_folder {
  char		folder_name[UCBMAIL_MAX_FOLDER_NAME];	/* �ե����̾	*/
  int		max_article;				/* ���絭���ֹ�	*/
  int		unread_article;				/* ̤�ɵ�����	*/
};

extern short	ucbmail_mode;		/* UCB-mail �⡼�ɥե饰	*/
extern char	ucbmail_mbox[];		/* UCB-mail �ѥ᡼��ܥå���	*/

int		ucbmail_init();		/* UCB-mail �⡼�ɽ����	*/
int		ucbmail_menu();		/* UCB-mail ��˥塼		*/
