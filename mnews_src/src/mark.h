/*
 *
 *  �ߥˡ��˥塼���꡼��
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Mark define
 *  File        : mark.h
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-08/23
 *  Author      : MSR24 ��� ��
 *
 */

#ifdef	TRI
/*
 * TRI ��¤�Υ���ץ����
 */

struct trienode {
  char			value;
  struct trienode	*child;		/* �ҥΡ��ɤؤΥݥ��� */
  struct trienode	*next;		/* ����Ρ��ɤؤΥݥ��� */
  int			group_number;	/* ���̤� -1,��ü�Ρ��ɤξ��
					 * news_group[]���ֹ�
					 */
};
#endif	/* TRI */

extern short	dnas_mode;		/* DNAS �б��⡼��		*/
extern short	mark_lock;		/* ̤�ɴ������ޥե�		*/

void		news_read_initfile();	/* ������ե�����꡼��		*/
void		news_save_initfile();	/* ������ե����륻����		*/
void		save_group();		/* 1 ���롼�ץ�����		*/
#ifdef	DEBUG
void		news_print_mark();	/* �ޡ����ꥹ������ɽ��		*/
#endif	/* DEBUG */
void		news_add_mark();	/* �ޡ����ꥹ�������ɲ�		*/
void		news_delete_mark();	/* �ޡ����ꥹ�����Ǻ��		*/
void		news_fill_mark();	/* �ޡ����ꥹ���������ɲ�	*/
void		news_clean_mark();	/* �ޡ����ꥹ�������Ǻ��	*/
int		news_check_mark();	/* �ޡ����ꥹ�ȥ����å�		*/
void		news_count_unread();	/* ̤�ɵ���������		*/
int		read_integer();		/* �����ե���������		*/
void		add_mark();		/* �ޡ����ꥹ�������ɲ�(����)	*/
void		delete_mark();		/* �ޡ����ꥹ�����Ǻ��(����)	*/
void		news_delete_bogus();	/* ̵���˥塼�����롼�׺��	*/

#ifdef	TRI
struct trienode	*make_tri();		/* tri ����			*/
void		insert_tri();		/* tri �������			*/
int		find_tri();		/* tri ��측��			*/
struct trienode	*const_trienode();	/* tri entity ����		*/
void		free_trienode();	/* tri �������		*/
struct trienode	*find_trienode();	/* tri ����Ρ�����������	*/
void		print_trienode();	/* tri ɽ��			*/
#endif	/* TRI */
