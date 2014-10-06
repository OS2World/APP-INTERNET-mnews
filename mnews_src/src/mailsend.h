/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Mail send define
 *  File        : mailsend.h
 *  Version     : 1.21
 *  First Edit  : 1992-07/29
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 ��� ��
 *
 */

#ifdef	MSDOS
#define	MAILRC_FILE	"_mailrc"	/* �����ꥢ������ե�����	*/
#else	/* !MSDOS */
#define	MAILRC_FILE	".mailrc"	/* �����ꥢ������ե�����	*/
#endif	/* !MSDOS */
#define	ALIAS_PARAM1	"alias"		/* �����ꥢ���ѥ�᡼��(1)	*/
#define	ALIAS_PARAM2	"group"		/* �����ꥢ���ѥ�᡼��(2)	*/
#define	MAX_SEND_SIZE	(48L * 1024L)

#define	FORWARD_START_MESSAGE	"------- Forwarded Message\n"
#define	FORWARD_END_MESSAGE	"\n------- End of Forwarded Message"

#define	SF_FROM_MASK	0x0001
#define	SF_SUBJ_MASK	0x0002
#define	SF_TO_MASK	0x0004
#define	SF_CC_MASK	0x0008
#define	SF_BCC_MASK	0x0010
#define	SF_MIME_MASK	0x0020
#define	SF_CONTENT_MASK	0x0040
#define	SF_FILE_MASK	0x0080
#define	SF_ETC_MASK	0x0800

#define	SF_THROUGH_MASK	0x1000		/* MIME ���󥳡�����������	*/
#define	SF_DELETE_MASK	0x2000		/* �����������			*/
#define	SF_EXPAND_MASK	0x4000		/* �����ꥢ��Ÿ������		*/

#define	REPLY_QUOTE_MASK	0x01	/* ����				*/
#define	REPLY_BOARD_MASK	0x02	/* BOARD ��ץ饤�⡼��		*/
#define	FOLLOW_BOARD_MASK	0x04	/* BOARD �ե����⡼��		*/

extern short	add_cc_mode;		/* Cc: �ղå⡼�ɥե饰		*/
extern short	reply_cc_mode;		/* Cc: ��ץ饤�⡼�ɥե饰	*/
extern short	auto_sig_mode;		/* .signature ��ư�ղå⡼��	*/
extern short	from_mode;		/* From �񼰥⡼��		*/
extern short	mh_alias_mode;		/* MH �����ꥢ���⡼��		*/
extern char	send_filter[];		/* �᡼�������ե��륿		*/
extern char	mail_field[];		/* �᡼�뼫ư�����ե������	*/
extern char	reply_indicator[];	/* ���󥸥�����			*/
extern char	follow_message[];	/* �ե�����å�����		*/
extern char	reply_message[];	/* ��ץ饤��å�����		*/
extern char	forward_start[];	/* �ե���ɳ��ϥ�å�����	*/
extern char	forward_end[];		/* �ե���ɽ�λ��å�����	*/

int		mail_send();		/* �᡼������			*/
int		mail_reply();		/* �᡼���ֿ�			*/
int		mail_forward();		/* �᡼��ž��			*/
int		mail_relay();		/* �᡼�����			*/
int		exec_send();		/* �᡼������(���̴ؿ�)		*/
