/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1992-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Pager define
 *  File        : pager.h
 *  Version     : 1.21
 *  First Edit  : 1992-08/05
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 ��� ��
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

extern int	last_key;		/* �Ǹ�Υ���������	*/
extern char	guide_message[];	/* �����ɥ�å�����	*/
extern short	scroll_mode;		/* ��������⡼��	*/

int		view_file();		/* �ե����뻲��			*/
int		view_files();		/* ʣ���ե����뻲��		*/
