/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : File carbon copy define
 *  File        : fcc.h
 *  Version     : 1.21
 *  First Edit  : 1993-05/07
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 ��� ��
 *
 */

#define	FCC_FROM_FIELD	"From "

extern KANJICODE fcc_code;		/* Fcc: ��¸����������		*/
extern char	mail_fcc_field[];	/* �᡼����   Fcc: �ե������	*/
extern char	news_fcc_field[];	/* �˥塼���� Fcc: �ե������	*/

int		fcc_save();		/* FCC �ե�������¸		*/
