/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Mark routine
 *  File        : mark.c
 *  Version     : 1.21
 *  First Edit  : 1991-07/25
 *  Last  Edit  : 1997-10/29
 *  Author      : MSR24 宅間 顯
 *
 */

#include	"compat.h"
#include	"nntplib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"mnews.h"
#include	"mark.h"

short		mark_lock = 0;		/* 未読管理操作セマフォ		*/

static int	max_line;		/* .newsrc の最大行番号		*/

/*
 * マークファイルロード
 */

void	news_read_initfile(file_name)
     char	*file_name;
{
  FILE			*fp;
  BOGUS_LIST		*alloc_ptr;
  BOGUS_LIST		*bogus_ptr;
  char			group_name[SMALL_BUFF];	/* buff より前に確保 */
  char			buff[BUFF_SIZE];
  char			*ptr;
  int			start_number;
  int			end_number;
  int			current_group;
  int			unsubscribe;
  int			line;
  register int		i, j;
#ifdef	TRI
  struct trienode	*triroot;
#endif	/* TRI */

  max_line = 0;
  if (group_number <= 0) {
    return;
  }
#ifdef	TRI
  triroot = make_tri(news_group, group_number);
#endif	/* TRI */
  mark_lock++;
  if (group_list == (GROUP_LIST*)NULL) {
    group_list = (GROUP_LIST*)malloc(group_number * sizeof(GROUP_LIST));
    if (group_list == (GROUP_LIST*)NULL) {
      print_fatal("Can't allocate memory for group struct.");
      group_number = 0;
      mark_lock--;
      return;
    }
  }
  for (i = 0; i < group_number; i++) {
    group_list[i].mark_ptr = (MARK_LIST*)NULL;
    group_list[i].mark_order = -1;
    if (news_group[i].max_article) {
      group_list[i].unread_article =
	news_group[i].max_article - news_group[i].min_article + 1;
    } else {
      group_list[i].unread_article = 0;
    }
    group_list[i].unsubscribe = 0;
  }
  line = j = 0;
  sprintf(buff, "%s-%s", file_name, nntp_server);
  fp = fopen(buff, "r");
  if (fp != (FILE*)NULL) {
    strcpy(file_name, buff);	/* ファイル名を強引に変更してしまう */
  } else {
    fp = fopen(file_name, "r");
    if (fp == (FILE*)NULL) {
      print_fatal("Can't load mark-file.");
    }
  }
  if (fp != (FILE*)NULL) {
    print_mode_line(japanese ? "未読管理ファイルをロード中です。" :
		    "Loading mark-file.");
    while (1) {
      if (!fgets(buff, sizeof(buff), fp)) {
	break;
      }
      line++;
      ptr = buff;
      while ((*ptr == '\t') || (*ptr == ' ')) {
	ptr++;
      }
      if ((*ptr == '\n') || (*ptr == '#') || (*ptr == '\0')) {
	continue;
      }
      i = 0;
      unsubscribe = 0;
      while (*ptr) {
	if (((*ptr == ':') || (*ptr == '!')) &&
	    ((*(ptr + 1) == ' ') || (*(ptr + 1) == '\t')
	     || (*(ptr + 1) == '\n'))) {
	  if (*ptr == '!') {
	    unsubscribe = 1;
	  }
	  ptr++;
	  group_name[i] = '\0';
	  if (i >= (MAX_GROUP_NAME - 1)) {
	    break;
	  }
	  i = -1;
	  while ((*ptr == ' ') || (*ptr == '\t')) {
	    ptr++;
	  }
	  break;
	}
	group_name[i++] = *ptr++;
      }
      if (i >= 0) {
	if (i >= (MAX_GROUP_NAME - 1)) {
	  print_warning("Line %4d : Too long news group name %s.", line,
			group_name);
	} else {
	  print_warning("Line %4d : Syntax error-1 in mark-file.", line);
	}
	continue;
      }
      current_group = -1;
#ifdef	TRI
      current_group = find_tri( triroot, group_name );
#else	/* !TRI */
      for (i = j; i < group_number; i++) {
	if (!strcmp(group_name, news_group[i].group_name)) {
	  current_group = i;
	  break;
	} else if (i == j) {
	  if (group_list[i].mark_order >= 0) {
	    j++;
	  }
	}
      }
#endif	/* !TRI */
      if (current_group < 0) {
	if (!dnas_mode) {
	  continue;
	}
	print_warning("Line %4d : Bogus news group \"%s\" in mark-file.",
		      line, group_name);
	alloc_ptr = (BOGUS_LIST*)malloc(sizeof(BOGUS_LIST));
	if (alloc_ptr != (BOGUS_LIST*)NULL) {
	  if (bogus_list != (BOGUS_LIST*)NULL) {
	    bogus_ptr = bogus_list;
	    while (bogus_ptr->next_ptr != (BOGUS_LIST*)NULL) {
	      bogus_ptr = bogus_ptr->next_ptr;
	    }
	    bogus_ptr->next_ptr = alloc_ptr;
	  } else {
	    bogus_list = alloc_ptr;
	  }
	  strcpy(alloc_ptr->group_name, group_name);
	  alloc_ptr->mark_ptr = (MARK_LIST*)NULL;
	  alloc_ptr->next_ptr = (BOGUS_LIST*)NULL;
	  alloc_ptr->mark_order = line;
	  alloc_ptr->unsubscribe = unsubscribe;
	} else {
	  print_fatal("Can't allocate memory for bogus struct.");
	  continue;
	}
      } else {
	group_list[current_group].unsubscribe = unsubscribe;
	group_list[current_group].mark_order = line;
      }
      max_line = line;
      while (1) {
	if ((*ptr == '\\') && ((*(ptr + 1) == '\0') || (*(ptr + 1) == '\n'))) {
	  if (!fgets(buff, sizeof(buff), fp)) {
	    break;
	  }
	  line++;
	  ptr = buff;
	}
	if ((start_number = read_integer(fp, buff, sizeof(buff),
					 &ptr)) <= 0) {
	  if (start_number) {
	    print_warning("Line %4d : Syntax error-2 in mark-file.", line);
	  }
	  break;
	}
	end_number = start_number;
	if (*ptr == ',') {
	  ptr++;
	} else if (*ptr == '-') {
	  ptr++;
	  if ((end_number = read_integer(fp, buff, sizeof(buff),
					 &ptr)) < start_number) {
	    if (end_number) {
	      print_warning("Line %4d : Syntax error-3 in mark-file.", line);
	    }
	    break;
	  }
	  if (*ptr == ',') {
	    ptr++;
	  } else if (*ptr != '\n') {
	    print_warning("Line %4d : Syntax error-4 in mark-file.", line);
	    break;
	  }
	}
	if (current_group >= 0) {
	  news_add_mark(current_group, start_number, end_number);
	} else {
	  add_mark(&alloc_ptr->mark_ptr, start_number, end_number);
	}
      }
      if (current_group >= 0) {
	news_count_unread(current_group);
      }
    }
    fclose(fp);
  }
#ifdef	TRI
  free_trienode(triroot);
#endif	/* TRI */
  mark_lock--;
}

/*
 * マークリスト要素追加(ニュース専用上位関数)
 */

void	news_add_mark(current_group, start_number, end_number)
     int	current_group;
     int	start_number;
     int	end_number;
{
  if (current_group > group_number) {
    print_fatal("Illegal group number.");
    return;
  }
  if ((start_number < 1) || (start_number > end_number)) {
    print_fatal("Illegal start or end article list number.");
    return;
  }
  if ((start_number > news_group[current_group].max_article) &&
      (news_group[current_group].max_article > 0)) {
    print_warning("Illegal start article list number.");
    return;
  }
  if ((end_number > news_group[current_group].max_article) &&
      (news_group[current_group].max_article > 0)) {
    print_warning("Illegal end article list number.");
    end_number = news_group[current_group].max_article;
  }
#ifndef	DONT_OPTIMIZE_NEWSRC
  if (!news_group[current_group].min_article) {
    return;
  } else if (start_number < news_group[current_group].min_article) {
    start_number = 1;
    if (end_number < news_group[current_group].min_article) {
      end_number = news_group[current_group].min_article - 1;
    }
  }
#endif	/* !DONT_OPTIMIZE_NEWSRC */
#ifdef	DEBUG
  fprintf(stderr, "group %s add mark %d - %d\n",
	  news_group[current_group].group_name, start_number, end_number);
#endif	/* DEBUG */
  add_mark(&group_list[current_group].mark_ptr, start_number, end_number);
}

/*
 * マークリスト要素削除(ニュース専用上位関数)
 */

void	news_delete_mark(current_group, start_number, end_number)
     int	current_group;
     int	start_number;
     int	end_number;
{
  if ((start_number < 1) || (start_number > end_number)) {
    print_fatal("Illegal start or end article list number.");
    return;
  }
  if ((start_number > news_group[current_group].max_article) &&
      (news_group[current_group].max_article > 0)) {
    print_warning("Illegal start article list number.");
    return;
  }
  if ((end_number > news_group[current_group].max_article) &&
      (news_group[current_group].max_article > 0)) {
    print_warning("Illegal end article list number.");
    end_number = news_group[current_group].max_article;
  }

#ifdef	DEBUG
  fprintf(stderr, "group %s delete mark %d - %d\n",
	  news_group[current_group].group_name, start_number, end_number);
#endif	/* DEBUG */
  delete_mark(&group_list[current_group].mark_ptr, start_number, end_number);
}

/*
 * マークリスト全要素追加
 */

void	news_fill_mark(current_group)
     int	current_group;
{
  MARK_LIST	*mark_ptr;

  mark_lock++;
  news_clean_mark(current_group);
  if ((news_group[current_group].max_article > 0) &&
      (news_group[current_group].min_article <=
       news_group[current_group].max_article)) {
    mark_ptr = (MARK_LIST*)malloc(sizeof(MARK_LIST));
    if (mark_ptr != (MARK_LIST*)NULL) {
      group_list[current_group].mark_ptr = mark_ptr;
      mark_ptr->prev_ptr = (MARK_LIST*)NULL;
      mark_ptr->next_ptr = (MARK_LIST*)NULL;
      mark_ptr->start_number = 1;
      mark_ptr->end_number   = news_group[current_group].max_article;
    } else {
      print_fatal("Can't allocate memory for mark list.");
    }
  }
  mark_lock--;
}

/*
 * マークリスト全要素削除
 */

void	news_clean_mark(current_group)
     int	current_group;
{
  MARK_LIST	*mark_ptr;
  MARK_LIST	*free_ptr;

  mark_lock++;
  mark_ptr = group_list[current_group].mark_ptr;
  while (mark_ptr != (MARK_LIST*)NULL) {
    free_ptr = mark_ptr;
    mark_ptr = mark_ptr->next_ptr;
    free(free_ptr);
  }
  group_list[current_group].mark_ptr = (MARK_LIST*)NULL;
  mark_lock--;
}

/*
 * マークリスト要素表示
 */

#ifdef	DEBUG
void	print_mark(mark_ptr)
     MARK_LIST	*mark_ptr;
{
  while (mark_ptr != (MARK_LIST*)NULL) {
    fprintf(stderr, "START:%05d - END:%05d\n", mark_ptr->start_number,
	    mark_ptr->end_number);
    mark_ptr = mark_ptr->next_ptr;
  }
}
#endif	/* DEBUG */

/*
 * マークファイルセーブ
 */

void	news_save_initfile(file_name)
     char	*file_name;
{
  BOGUS_LIST	*bogus_ptr;
  FILE		*fp;
  struct stat	stat_buff;
  char		backup_file[PATH_BUFF];
  register int	current_group;
  register int	order;
  register int	rest;
  int		status;

  if (group_number <= 0) {
    return;
  }

  /*	バックアップファイルへリネーム	*/

  create_backup_name(backup_file, file_name);
#if	defined(MSDOS) || defined(OS2)
  unlink(backup_file);	/* for X68K libc rename bug */
#endif	/* (MSDOS || OS2) */
  if (rename(file_name, backup_file) && (errno != ENOENT)) {
    print_fatal("Can't rename mark-file.");
    return;
  }
  if (stat(backup_file, &stat_buff)) {
    stat_buff.st_mode = 00644;
    if (errno != ENOENT) {
      print_fatal("Can't get stat of old mark-file.");
    }
  }

  mark_lock++;
  fp = fopen(file_name, "w");
  if (fp == (FILE*)NULL) {
    print_fatal("Can't save mark-file.");
    mark_lock--;
    return;
  }
  print_mode_line(japanese ? "未読管理ファイルをセーブ中です。" :
		  "Saving mark-file.");
  rest = group_number;
  bogus_ptr = bogus_list;
  for (order = 0; rest > 0; order++) {
    if (order > max_line) {
      print_fatal("Program error.unexpected order group exists.");
      break;
    }
    while (bogus_ptr != (BOGUS_LIST*)NULL) {
      if (bogus_ptr->mark_order > order) {
	break;
      }
      save_group(fp, bogus_ptr->group_name, bogus_ptr->mark_ptr,
		 bogus_ptr->unsubscribe);
      bogus_ptr = bogus_ptr->next_ptr;
    }
    current_group = 0;
    if (order) {
      status = 0;
      for (; current_group < group_number; current_group++) {
	if (group_list[current_group].mark_order == order) {
	  save_group(fp, news_group[current_group].group_name,
		     group_list[current_group].mark_ptr,
		     group_list[current_group].unsubscribe);
	  current_group++;
	  rest--;
	  status = 1;
	  break;
	}
      }
      if (!status) {
	continue;
      }
    }
    for (; current_group < group_number; current_group++) {
      if (group_list[current_group].mark_order < 0) {
	save_group(fp, news_group[current_group].group_name,
		   group_list[current_group].mark_ptr,
		   group_list[current_group].unsubscribe);
	rest--;
      } else {
	break;
      }
    }
  }
  while (bogus_ptr != (BOGUS_LIST*)NULL) {
    save_group(fp, bogus_ptr->group_name, bogus_ptr->mark_ptr,
	       bogus_ptr->unsubscribe);
    bogus_ptr = bogus_ptr->next_ptr;
  }
  fclose(fp);
  chmod(file_name, ((stat_buff.st_mode & 01777) | S_IREAD | S_IWRITE));
  mark_lock--;
}

/*
 * 1 グループセーブ
 */

void	save_group(fp, group_name, mark_ptr, unsubscribe)
     FILE	*fp;
     char	*group_name;
     MARK_LIST	*mark_ptr;
     int	unsubscribe;
{
  register int	count;

  count = 0;
  if (unsubscribe) {
    fprintf(fp, "%s! ", group_name);
  } else {
    fprintf(fp, "%s: ", group_name);
  }
  while (mark_ptr != (MARK_LIST*)NULL) {
    /* 1 行が長くなる場合の対応
    if (count > 72) {
      fprintf(fp, "\\\n\t");
      count = 0;
    }
    */
    if (count) {
      fputc(',', fp);
      count++;
    }
    if (mark_ptr->start_number == mark_ptr->end_number) {
      
      /*
       * DECstation では setenv PROG_ENV POSIX して compile しないと
       * printf の戻り値が正常時に 0 なのでうまく動きません。
       */
      
      count += fprintf2(fp, "%d", mark_ptr->start_number);
    } else {
      count += fprintf2(fp, "%d-%d", mark_ptr->start_number,
			mark_ptr->end_number);
    }
    mark_ptr = mark_ptr->next_ptr;
  }
  fputc('\n', fp);
}

/*
 * マークリストチェック
 */

int	news_check_mark(current_group, article_number)
     int	current_group;
     int	article_number;
{
  MARK_LIST	*mark_ptr;

  if ((current_group < 0) || (current_group > group_number)) {
    print_fatal("Illegal group number.");
    return(-1);
  }
  mark_ptr = group_list[current_group].mark_ptr;
  while (mark_ptr != (MARK_LIST*)NULL) {
    if ((mark_ptr->start_number <= article_number) &&
	(mark_ptr->end_number >= article_number)) {
      return(1);
    }
    mark_ptr = mark_ptr->next_ptr;
  }
  return(0);
}

/*
 * 未読記事数算出
 */

void	news_count_unread(current_group)
     int	current_group;
{
  MARK_LIST	*mark_ptr;
  register int	start_number;
  register int	end_number;
  register int	max_number;
  register int	min_number;
  register int	unread_number;

  max_number = news_group[current_group].max_article;
  min_number = news_group[current_group].min_article;
  if (max_number < 1) {
    news_group[current_group].min_article = min_number =
      news_group[current_group].max_article = max_number = 0;
    unread_number = 0;
  } else {
    unread_number = max_number - min_number + 1;
  }
  mark_ptr = group_list[current_group].mark_ptr;
  while (mark_ptr != (MARK_LIST*)NULL) {
    start_number = mark_ptr->start_number;
    end_number = mark_ptr->end_number;
    if (start_number <= min_number) {
      if (end_number >= max_number) {
	unread_number = 0;
      } else if (end_number >= min_number) {
	unread_number -= (end_number - min_number + 1);
      }
    } else {
      if (end_number <= max_number) {
	unread_number -= (end_number - start_number + 1);
      } else if (start_number <= max_number) {
	unread_number -= (max_number - start_number + 1);
      }
    }
    if (unread_number < 0) {
      print_warning("Illegal unread article number.");
#ifdef	DEBUG
      print_mark(group_list[current_group].mark_ptr);
      exit(1);
#endif	/* DEBUG */
      unread_number = 0;
    }
    mark_ptr = mark_ptr->next_ptr;
  }
  group_list[current_group].unread_article = unread_number;
}

/*
 * 整数ファイル入力
 */

int	read_integer(fp, buff, length, ptr1)
     FILE	*fp;
     char	*buff;
     int	length;
     char	**ptr1;
{
  int	number;
  char	*ptr2;

  number = 0;
  ptr2 = *ptr1;
  while ((*ptr2 == '\t') || (*ptr2 == ' ')) {
    ptr2++;
  }
  if (*ptr2 == '\n') {
    ptr2++;
  } else {
    if (!(*ptr2)) {
      if (!fgets(buff, length, fp)) {
	return(-1);
      }
      ptr2 = buff;
    }
    if (!isdigit(*ptr2)) {
      while (*ptr2) {		/* 継続行は捨てる */
	if (!(*ptr2)) {
	  if (!fgets(buff, length, fp)) {
	    break;
	  }
	  ptr2 = buff;
	}
	if (*ptr2++ == '\n') {
	  break;
	}
      }
      return(-1);
    } else {
      while (1) {
	if (!(*ptr2)) {
	  if (!fgets(buff, length, fp)) {
	    return(-1);
	  }
	  ptr2 = buff;
	}
	if (!isdigit(*ptr2)) {
	  break;
	}
	number = number * 10 + *ptr2++ - '0';
      }
      while ((*ptr2 == '\t') || (*ptr2 == ' ')) {
	ptr2++;
      }
      if (!(*ptr2)) {
	if (!fgets(buff, length, fp)) {
	  return(-1);
	}
	ptr2 = buff;
      }
    }
  }
  *ptr1 = ptr2;
  return(number);
}

/*
 * マークリスト要素追加(汎用下位関数)
 */

void	add_mark(first_ptr, start_number, end_number)
     MARK_LIST	**first_ptr;
     int	start_number;
     int	end_number;
{
  MARK_LIST	*mark_ptr;
  MARK_LIST	*alloc_ptr;

  mark_ptr = *first_ptr;
  alloc_ptr = (MARK_LIST*)malloc(sizeof(MARK_LIST));
  if (alloc_ptr == (MARK_LIST*)NULL) {
    print_fatal("Can't allocate memory for mark list.");
    return;
  }
  alloc_ptr->prev_ptr     = (MARK_LIST*)NULL;
  alloc_ptr->next_ptr     = (MARK_LIST*)NULL;
  alloc_ptr->start_number = start_number;
  alloc_ptr->end_number   = end_number;
  if (mark_ptr == (MARK_LIST*)NULL) {

    /*	最初のマークを新規追加	*/

    *first_ptr = alloc_ptr;
    return;
  }
  mark_lock++;
  if (mark_ptr->start_number >= start_number) {

    /*	先頭にマークを追加	*/

    *first_ptr = alloc_ptr;
    alloc_ptr->next_ptr = mark_ptr;
    mark_ptr->prev_ptr  = alloc_ptr;
  } else {
    while (1) {
      if (mark_ptr->next_ptr == (MARK_LIST*)NULL) {

	/*	最後にマーク追加	*/

	alloc_ptr->prev_ptr = mark_ptr;
	mark_ptr->next_ptr  = alloc_ptr;
	break;
      }
      if ((mark_ptr->start_number <= start_number) &&
	  (mark_ptr->next_ptr->start_number >= start_number)) {

	/*	中間にマーク追加	*/

	alloc_ptr->next_ptr          = mark_ptr->next_ptr;
	mark_ptr->next_ptr->prev_ptr = alloc_ptr;
	alloc_ptr->prev_ptr          = mark_ptr;
	mark_ptr->next_ptr           = alloc_ptr;
	break;
      }
      mark_ptr = mark_ptr->next_ptr;
    }
  }

  /*	マークリスト最適化	*/

#ifdef	DEBUG
  mark_ptr = *first_ptr;
  fprintf(stderr, "Added article mark list.\n");
  while (mark_ptr != (MARK_LIST*)NULL) {
    fprintf(stderr, "%5d-%5d\n", mark_ptr->start_number,
	    mark_ptr->end_number);
    mark_ptr = mark_ptr->next_ptr;
  }
#endif	/* DEBUG */
  mark_ptr = *first_ptr;
  while (mark_ptr->next_ptr != (MARK_LIST*)NULL) {
    alloc_ptr = mark_ptr->next_ptr;
    if (mark_ptr->end_number >= alloc_ptr->end_number) {
      if (alloc_ptr->next_ptr != (MARK_LIST*)NULL) {
	alloc_ptr->next_ptr->prev_ptr = mark_ptr;
	mark_ptr->next_ptr = alloc_ptr->next_ptr;
	free(alloc_ptr);
      } else {
	mark_ptr->next_ptr = (MARK_LIST*)NULL;
	free(alloc_ptr);
      }
    } else if (mark_ptr->end_number >= (alloc_ptr->start_number - 1)) {
      mark_ptr->end_number = alloc_ptr->end_number;
      if (alloc_ptr->next_ptr != (MARK_LIST*)NULL) {
	alloc_ptr->next_ptr->prev_ptr = mark_ptr;
	mark_ptr->next_ptr = alloc_ptr->next_ptr;
	free(alloc_ptr);
      } else {
	mark_ptr->next_ptr = (MARK_LIST*)NULL;
	free(alloc_ptr);
      }
    } else {
      mark_ptr = mark_ptr->next_ptr;
    }
  }
#ifdef	DEBUG
  mark_ptr = *first_ptr;
  fprintf(stderr, "Optimized article mark list.\n");
  while (mark_ptr != (MARK_LIST*)NULL) {
    fprintf(stderr, "%5d-%5d\n", mark_ptr->start_number,
	    mark_ptr->end_number);
    mark_ptr = mark_ptr->next_ptr;
  }
#endif	/* DEBUG */
  mark_lock--;
}

/*
 * マークリスト要素削除(汎用下位関数)
 */

void	delete_mark(first_ptr, start_number, end_number)
     MARK_LIST	**first_ptr;
     int	start_number;
     int	end_number;
{
  MARK_LIST	*mark_ptr;
  MARK_LIST	*alloc_ptr;
  int		alloc_flag;

  mark_ptr = *first_ptr;
  alloc_flag = 0;
  mark_lock++;
  while (mark_ptr != (MARK_LIST*)NULL) {
    if ((mark_ptr->start_number > end_number) ||
	(mark_ptr->end_number < start_number)) {
      mark_ptr = mark_ptr->next_ptr;
      continue;
    } else if ((mark_ptr->start_number >= start_number) &&
	       (mark_ptr->end_number <= end_number)) {
      alloc_ptr = mark_ptr;
      mark_ptr  = mark_ptr->next_ptr;
    } else if ((mark_ptr->start_number < start_number) &&
	       (mark_ptr->end_number > end_number)) {
      if (alloc_flag) {
	print_fatal("Program error-1.unexpected article list found.");
	break;
      }
      alloc_flag = 1;
      alloc_ptr = (MARK_LIST*)malloc(sizeof(MARK_LIST));
      if (alloc_ptr == (MARK_LIST*)NULL) {
	print_fatal("Can't allocate memory for mark list.");
	break;
      }
      alloc_ptr->prev_ptr     = mark_ptr;
      alloc_ptr->next_ptr     = mark_ptr->next_ptr;
      alloc_ptr->start_number = end_number + 1;
      alloc_ptr->end_number   = mark_ptr->end_number;
      mark_ptr->end_number    = start_number - 1;
      mark_ptr->next_ptr      = alloc_ptr;
      if ((mark_ptr->start_number > mark_ptr->end_number) ||
	  (alloc_ptr->start_number > alloc_ptr->end_number)) {
	print_fatal("Program error-2.unexpected article list found.");
	break;
      }
      continue;
    } else if ((mark_ptr->start_number >= start_number) &&
	       (mark_ptr->end_number > end_number)) {
      mark_ptr->start_number = end_number + 1;
      continue;
    } else if ((mark_ptr->start_number < start_number) &&
	       (mark_ptr->end_number <= end_number)) {
      mark_ptr->end_number = start_number - 1;
      continue;
    } else {
      print_fatal("Program error-3.unexpected article list found.");
      break;
    }

    if (alloc_ptr != (MARK_LIST*)NULL) {

      /*	削除	*/

      if (alloc_ptr == *first_ptr) {

	/*	先頭のマーク削除	*/

	if (alloc_ptr->next_ptr == (MARK_LIST*)NULL) {
	  *first_ptr = (MARK_LIST*)NULL;
	  free(alloc_ptr);
	  break;
	} else {
	  *first_ptr = alloc_ptr->next_ptr;
	  alloc_ptr->next_ptr->prev_ptr = (MARK_LIST*)NULL;
	  free(alloc_ptr);
	}
      } else if (alloc_ptr->next_ptr == (MARK_LIST*)NULL) {

	/*	最後のマーク削除	*/

	alloc_ptr->prev_ptr->next_ptr = (MARK_LIST*)NULL;
	free(alloc_ptr);
      } else {

	/*	中間のマーク削除	*/

	alloc_ptr->prev_ptr->next_ptr = alloc_ptr->next_ptr;
	alloc_ptr->next_ptr->prev_ptr = alloc_ptr->prev_ptr;
	free(alloc_ptr);
      }
    }
  }
  mark_lock--;
}

/*
 * 無効ニュースグループ削除
 */

void	news_delete_bogus()
{
  BOGUS_LIST	*bogus_ptr, *bogus_ptr2;
  register int	number;

  number = 0;
  bogus_ptr = bogus_list;
  while (bogus_ptr != (BOGUS_LIST*)NULL) {
    number++;
    bogus_ptr = bogus_ptr->next_ptr;
  }
  if (number) {
    if (yes_or_no(CARE_YN_MODE,
		  "%d個の無効なニュースグループを削除してよろしいですか?",
		  "Delete %d bogus newsgroup(s).Are you sure?", number)) {
      mark_lock++;
      bogus_ptr = bogus_list;
      while (bogus_ptr != (BOGUS_LIST*)NULL) {
	bogus_ptr2 = bogus_ptr->next_ptr;
	free(bogus_ptr);
	bogus_ptr = bogus_ptr2;
      }
      bogus_list = (BOGUS_LIST*)NULL;
      mark_lock--;
    }
  }
}

#ifdef	TRI
/*
 * TRI 構造を作る
 * 戻り値:作成した TRIの根ノードへのポインタ
 */

struct trienode	*make_tri(ng_array, ng_num)
     struct news_group	*ng_array;	/* struct trienode の配列	*/
     int		ng_num;		/* 配列の大きさ			*/
{
  register int		i;
  struct trienode	*root;

  if (ng_num <= 0) {
    return((struct trienode*)NULL);
  }
  root = const_trienode('#', (struct trienode*)NULL, (struct trienode*)NULL,
			-1);
  for (i = 0; i < ng_num; i++) {
    insert_tri(root, ng_array[i].group_name, i);
  }
  return(root);
}

/*
 * TRI に一語登録
 */

void	insert_tri(triroot, word, ng_id)
     struct trienode	*triroot;	/* TRI の根ノードへのポインタ	*/
     char		*word;		/* 登録したい語			*/
     int		ng_id;
{
  int			i, ng;
  char			c;
  struct trienode	*node, *tptr;
  
  node = triroot;
  for (i = 0; i < strlen(word); i++) {
    c = word[i];
    if (!(tptr = find_trienode(node->child, c ))) {
      tptr = const_trienode(c, node, node->child, -1);
    }
    node = tptr;
  }
  node->group_number = ng_id;
}

/*
 * TRI から一語検索
 * 戻り値	-1:エラー
 *		-1以外:グループナンバー
 */

int	find_tri(triroot, word)
     struct trienode	*triroot;	/* TRI の根ノードへのポインタ	*/
     char		*word;		/* 検索したい語			*/
{
  int			i, ng_id;
  struct trienode	*node;
  
  node = triroot;
  ng_id = 0;
  for (i = 0; i < strlen( word ); i++) {
    if (!(node = find_trienode(node->child, word[i]))) {
      ng_id = -1;
      break;
    }
  }
  if (!ng_id) {
    ng_id = node->group_number;
  }
  return(ng_id);
}

/*
 * TRI のノード作成
 * 戻り値:作成したノード自身へのポインタ
 */

struct trienode	*const_trienode(value, mother, brother, id)
     char		value;		/* ノードの値(一文字)		*/
     struct trienode	*mother;	/* 親ノードへのポインタ		*/
     struct trienode	*brother;	/* 兄弟ノードへのポインタ	*/
     int		id;		/*
					 * ノードが語の最終文字を持って
					 * いる時、グループナンバー
					 * それ以外の場合、-1
					 */
{
  struct trienode	*tptr;
  
  tptr = (struct trienode *)malloc(sizeof(struct trienode));
  tptr->value = value;
  tptr->group_number = id;
  tptr->child = (struct trienode*)NULL;
  tptr->next = brother;
  if (mother) {
    mother->child = tptr;
  }
  return(tptr);
}

/*
 * 兄弟ノードを検索
 * 戻り値:見つかった場合ノードへのポインタ、エラー時 NULL
 */

struct trienode	*find_trienode(node, value)
     struct trienode	*node;	/* 検索したいノードリストの先頭ポインタ	*/
     char		value;	/* 検索したい文字			*/
{
  struct trienode	*tptr;

  tptr = node;
  while (tptr != (struct trienode*)NULL) {
    if (tptr->value == value) {
      break;
    }
    tptr = tptr->next;
  }
  return(tptr);
}

/*
 * TRI をメモリ解放
 * (兄弟、子ノードも解放される)
 */

void	free_trienode(node)
     struct trienode	*node;	/* 解放したいノードへのポインタ	*/
{
  if (node->next) {
    free_trienode(node->next);
  }
  node->next = (struct trienode*)NULL;
  if (node->child) {
    free_trienode(node->child);
  }
  node->child = (struct trienode*)NULL;
  free(node);
}

#ifdef	DEBUG
/*
 * TRI を表示(デバッグ用)
 */

void	print_trienode(node, indent)
     struct trienode	*node;	/* 表示したいノードへのポインタ	*/
     int		indent;	/* インデントレベル		*/
{
  static int	flag = 0;
  register int	i;

  if (!flag) {
    fputc('\n', stderr);
    for (i = 0; i < indent ; i++) {
      fputc(' ', stderr);
    }
  }
  putchar(node->value);
  if (node->group_number >= 0) {
    fprintf(stderr, "  ##%d##", node->group_number);
    flag = 0;
  } else {
    flag = 1;
  }
  if (node->child) {
    print_trienode(node->child, indent + 1);
  }
  if (node->next) {
    print_trienode(node->next, indent);
  }
}
#endif	/* DEBUG */
#endif	/* TRI */
