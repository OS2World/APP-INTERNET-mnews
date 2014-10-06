/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1991-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : File carbon copy routine
 *  File        : fcc.c
 *  Version     : 1.21
 *  First Edit  : 1993-05/07
 *  Last  Edit  : 1997-10/26
 *  Author      : MSR24 宅間 顯
 *
 */

#ifdef	FCC
#include	"compat.h"
#include	"nntplib.h"
#include	"field.h"
#include	"kanjilib.h"
#include	"mnews.h"
#include	"fcc.h"

int	fcc_save(file_name, from, date, fcc)
     char	*file_name;
     char	*from;
     char	*date;
     char	*fcc;
{
  FILE		*fp1;
  DIR_PTR	*dp;
  struct stat	stat_buff;
  FILE		*fp2;
  DIR		*dir_ptr;
  struct tm	*tm;
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		*ptr;
  time_t	now_time;
  int		i, j;
  int		status;

  fp1 = fopen(file_name, "r");
  if (fp1 == (FILE*)NULL) {
    return(1);
  }
  j = 1;
  fp2 = (FILE*)NULL;
  if (fcc) {
    ptr = fcc;
    while (((unsigned char)*ptr) > ' ') {
      ptr++;
    }
    *ptr = '\0';
    if (fcc[0] == SLASH_CHAR) {
      strcpy(buff1, fcc);
#ifdef	MSDOS
    } else if ((!strncmp(fcc, "~/", 2)) || (!strncmp(fcc, "~\\", 2))) {
#else	/* !MSDOS */
    } else if (!strncmp(fcc, "~/", 2)) {
#endif	/* !MSDOS */
      sprintf(buff1, "%s%c%s", home_dir, SLASH_CHAR, &fcc[2]);
    } else {
      sprintf(buff1, "%s%c%s", home_dir, SLASH_CHAR, fcc);
    }
    if ((!stat(buff1, &stat_buff)) && (stat_buff.st_mode & S_IFDIR)) {
      if ((dir_ptr = opendir(buff1)) != (DIR*)NULL) {
	status = 0;
	while ((dp = readdir(dir_ptr)) != (DIR_PTR*)NULL) {
	  ptr = buff2;
	  strcpy(ptr, dp->d_name);
	  i = 0;
	  while (1) {
	    j = *ptr++ - '0';
	    if ((j >= 0) && (j < 10)) {
	      i = i * 10 + j;
	    } else {
	      ptr--;
	      break;
	    }
	  }
	  if (!(*ptr) && i) {
	    if (status < i) {
	      status = i;
	    }
	  }
	}
	closedir(dir_ptr);
	sprintf(buff2, "%s%c%d", buff1, SLASH_CHAR, status + 1);
	fp2 = fopen(buff2, "w");
	chmod(buff2, S_IREAD | S_IWRITE);
	j = 0;	/* MH 形式		*/
      }
    } else {
      fp2 = fopen(buff1, "a");
      if (fp2 != (FILE*)NULL) {
	if (!ftell(fp2)) {
	  fputc('\n', fp2);
	}
      }
      chmod(buff1, S_IREAD | S_IWRITE);
      j = 1;	/* UCB-mail 形式	*/
    }
  }
  if (fp2 == (FILE*)NULL) {
    fclose(fp1);
    return(1);
  }
  now_time = time((time_t*)NULL);
  tm = localtime(&now_time);
  if (j) {
    fprintf(fp2, "%s%s %s %s %2d %02d:%02d:%02d 19%d\n", FCC_FROM_FIELD, from,
	    day_string[tm->tm_wday + 1], month_string[tm->tm_mon + 1],
	    tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_year);
  }
  if (!date[0]) {
    create_date_field(date, tm);
    fputs(date, fp2);
  }
  while (fgets(buff1, sizeof(buff1), fp1)) {
    switch (fcc_code) {
    case JIS_CODE:
      to_jis(buff2, buff1, default_code);
      break;
    case SJIS_CODE:
      to_sjis(buff2, buff1, default_code);
      break;
    case EUC_CODE:
      to_euc(buff2, buff1, default_code);
      break;
    default:
      strcpy(buff2, buff1);
      break;
    }
    if ((!strncmp(buff2, FCC_FROM_FIELD, sizeof(FCC_FROM_FIELD) - 1))
	& j) {
      fputc('>', fp2);
      fputs(buff2, fp2);
    } else {
      fputs(buff2, fp2);
    }
  }
  if (j) {
    fputc('\n', fp2);
  }
  if (fflush(fp2)) {
    print_fatal("Can't save FCC file.");
  }
  fclose(fp2);
  fclose(fp1);
  return(0);
}
#endif	/* FCC */
