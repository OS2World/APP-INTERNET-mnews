/*
 *
 *  TCP/IP �饤�֥��
 *
 *  Copyright 1996-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : NNTP/SMTP/POP3 library
 *  Sub system  : RFC field define
 *  File        : field.h
 *  Version     : 0.62
 *  First Edit  : 1996-10/25
 *  Last  Edit  : 1997-06/22
 *  Author      : MSR24 ��� ��
 *
 */

#ifndef	__FIELD_H__
#define	__FIELD_H__

/*
 * �᡼��ե����������
 */

#define	FROM_FIELD		"From:"
#define	TO_FIELD		"To:"
#define	CC_FIELD		"Cc:"
#define	BCC_FIELD		"Bcc:"
#define	FCC_FIELD		"Fcc:"
#define	DATE_FIELD		"Date:"
#define	SUBJECT_FIELD		"Subject:"
#define	REPLY_FIELD		"Reply-To:"
#define	IN_REPLY_FIELD		"In-Reply-To:"
#define	APP_TO_FIELD		"Apparently-To:"
#define	MESSAGE_FIELD		"Message-ID:"
#define	SENDER_FIELD		"Sender:"
#define	X_NSUBJ_FIELD		"X-Nsubject:"
#define	LENGTH_FIELD		"Content-Length:"
#define	RETURN_FIELD		"Return-Path:"

#define	RESENT_FROM_FIELD	"Resent-From:"
#define	RESENT_TO_FIELD		"Resent-To:"
#define	RESENT_CC_FIELD		"Resent-Cc:"
#define	RESENT_BCC_FIELD	"Resent-Bcc:"
#define	RESENT_REPLY_FIELD	"Resent-Reply-To:"
#define	RESENT_MESSAGE_FIELD	"Resent-Message-ID:"

#define	X_MAILER_FIELD		"X-Mailer:"

/*
 * �˥塼���ե������̾
 */

#define	GROUP_FIELD		"Newsgroups:"
#define	PATH_FIELD		"Path:"
#define	DIST_FIELD		"Distribution:"
#define	FOLLOWUP_FIELD		"Followup-To:"
#define	LINE_FIELD		"Lines:"
#define	REFERENCE_FIELD		"References:"
#define	CONTROL_FIELD		"Control:"
#define	XREF_FIELD		"Xref:"
#define	PATH_FIELD		"Path:"
#define	ORGAN_FIELD		"Organization:"

#define	X_READER_FIELD		"X-Newsreader:"

/*
 * �Ƽ���������
 */

#define	MAX_FIELD_LEN		256	/* ����ե������Ĺ		*/
#define	MAX_FROM_LEN		64	/* MAX_FIELD_LEN ��꾮�������� */
#define	MAX_SUBJECT_LEN		128	/* MAX_FIELD_LEN ��꾮�������� */
#define	MAX_FIELD_COLUMN	76	/* ����ե��������		*/

#endif	/* !__FIELD_H__ */
