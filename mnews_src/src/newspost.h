/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : News post define
 *  File        : newspost.h
 *  Version     : 1.21
 *  First Edit  : 1992-08/03
 *  Last  Edit  : 1997-12/10
 *  Author      : MSR24 ��� ��
 *
 */

#define	POSTER		"poster"
#define	MAX_POST_SIZE	(60L * 1024L)

#define	PF_FROM_MASK	0x0001
#define	PF_SUBJ_MASK	0x0002
#define	PF_ORGAN_MASK	0x0004
#define	PF_NG_MASK	0x0008
#define	PF_PATH_MASK	0x0010
#define	PF_LINE_MASK	0x0020
#define	PF_MIME_MASK	0x0040
#define	PF_CONTENT_MASK	0x0080
#define	PF_MAIL_MASK	0x0100
#define	PF_FILE_MASK	0x0200
#define	PF_ETC_MASK	0x0800

#define	PF_THROUGH_MASK	0x1000		/* MIME ���󥳡�����������	*/
#define	PF_DELETE_MASK	0x2000		/* �����������			*/

#define	FOLLOW_QUOTE_MASK	0x01	/* ����				*/

extern short	add_lines_mode;		/* Lines: �ղå⡼��		*/
extern short	inews_sig_mode;		/* inews �� .signature �⡼��	*/
extern short	ask_distrib_mode;	/* Distribution: �ե����������	*/
					/* �⡼��			*/
extern short	auto_sig_mode;		/* .signature ��ư�ղå⡼��	*/
extern short	from_mode;		/* From �񼰥⡼��		*/
extern char	post_filter[];		/* �˥塼����ƥե��륿		*/
extern char	organization[];		/* ��°				*/
extern char	news_field[];		/* �˥塼����ư�����ե������	*/
extern char	reply_indicator[];	/* ���󥸥�����			*/
extern char	follow_message[];	/* �ե�����å�����		*/

int		news_post();		/* �˥塼�����			*/
int		news_follow();		/* �˥塼���ե���		*/
int		news_cancel();		/* �˥塼������󥻥�		*/
int		exec_post();		/* �˥塼�����(���̴ؿ�)	*/
