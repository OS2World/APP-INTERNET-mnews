/*
 *
 *  ミニ・ニュースリーダ
 *
 *  Copyright 1994-1997 Matsushita Soft-Research, INC. A.Takuma
 *
 *  System      : Mini News Reader
 *  Sub system  : Japanese name conversion (hash version)
 *  File        : jnhash.c
 *  Version     : 1.21
 *  First Edit  : 1994-09/08
 *  Last  Edit  : 1997-11/26
 *  Author      : MSR24 宅間 顯
 *
 */

#include	"compat.h"
#include	"kanjilib.h"

#define		INC_KEYWORD	"include"

#ifdef	JNAMES

#ifdef	MNEWS
#if	defined(__STDC__) && !defined(DONT_HAVE_DOPRNT)
void		print_warning(char*, ...);
					/* 警告エラー表示		*/
void		print_fatal(char*, ...);/* 致命的エラー表示		*/
#else	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
void		print_warning();	/* 警告エラー表示		*/
void		print_fatal();		/* 致命的エラー表示		*/
#endif	/* (!__STDC__ && !DONT_HAVE_DOPRNT) */
#endif	/* !MNEWS */

#define	JN_SEPARATER	'.'
#define	JN_DOMAIN_SIZE	0x20
#define	JN_HASH_SIZE	0x20		/* 2 のべき乗数であること */
#define	JN_ALLOC_SIZE	1024

#ifndef	BUFF_SIZE
#define	BUFF_SIZE	1024
#endif	/* !BUFF_SIZE */
#ifndef	SMALL_BUFF
#define	SMALL_BUFF	256
#endif	/* !SMALL_BUFF */

typedef struct jn_domain	JN_DOMAIN;

struct jn_domain {
  char			domain_name[JN_DOMAIN_SIZE];
  unsigned int		hash_table[JN_HASH_SIZE];
  unsigned short	hash_size[JN_HASH_SIZE];
  JN_DOMAIN		*next_ptr;
};
#if	defined(MNEWS) || defined(MMH)
extern KANJICODE	default_code;
#else	/* !(MNEWS || MMH) */
#if	defined(MSDOS) || defined(OS2)
static KANJICODE	default_code = SJIS_CODE;
#else	/* !(MSDOS || OS2) */
static KANJICODE	default_code = EUC_CODE;
#endif	/* !(MSDOS || OS2) */
#endif	/* !(MNEWS || MMH) */
static JN_DOMAIN	*first_domain = (JN_DOMAIN*)NULL;
static char		*jnames_ptr = (char*)NULL;
static int		jnames_size = 0;
static int		jnames_used = 0;

static int		load_database();	/* データベースのロード	*/
static JN_DOMAIN	*jn_add_domain();	/* ドメイン追加		*/
static char		*jn_strcpy();		/* データコピー		*/

/*
 * JNAMES データベースのオープン
 */

int	jnOpen(private_db, public_db)
     char	*private_db;
     char	*public_db;
{
  int	status = -1;

  if (first_domain) {
#ifdef	MNEWS
    print_fatal("JNAMES data base already used.");
#else	/* !MNEWS */
    fprintf(stderr, "JNAMES data base already used.\n");
#endif	/* !MNEWS */
    return(-1);
  }
  if (jnames_ptr != (char*)NULL) {
    jnames_ptr = (char*)realloc(jnames_ptr, JN_ALLOC_SIZE);
  } else {
    jnames_ptr = (char*)malloc(JN_ALLOC_SIZE);
  }
  if (jnames_ptr != (char*)NULL) {
    *jnames_ptr = '\0';	/* 未使用領域をクリア	*/
  } else {
#ifdef	MNEWS
    print_fatal("Can't allocate jnames hash table.");
#else	/* !MNEWS */
    fprintf(stderr, "Can't allocate jnames hash table.\n");
#endif	/* !MNEWS */
    return(-1);
  }
  jnames_size = JN_ALLOC_SIZE;
  jnames_used = 1;	/* 0 は未使用 INDEX として使うため */
  if (!load_database(private_db)) {
    status = 0;
  }
  if (!load_database(public_db)) {
    status = 0;
  }
  return(status);
}

/*
 * JNAMES データベースのクローズ
 */

int	jnClose()
{
  JN_DOMAIN	*domain_ptr;

  while (first_domain != (JN_DOMAIN*)NULL) {
    domain_ptr = first_domain;
    first_domain = domain_ptr->next_ptr;
    free(domain_ptr);
  }
  first_domain = (JN_DOMAIN*)NULL;
  if (jnames_ptr != (char*)NULL) {
    free(jnames_ptr);
    jnames_ptr = (char*)NULL;
    jnames_size = jnames_used = 0;
  }
  return(0);
}

/*
 * JNAMES データベースの検索
 */

unsigned char	*jnFetch(domain, name)
     char	*domain;
     char	*name;
{
  JN_DOMAIN	*domain_ptr;
  register int	i;
  register int	code;
  char		*ptr1, *ptr2;

  domain_ptr = first_domain;
  while (domain_ptr != (JN_DOMAIN*)NULL) {
    if (!strcmp(domain_ptr->domain_name, domain)) {
      code = 0;
      ptr1 = name;
      while (*ptr1) {
	code += *ptr1++;
      }
      code &= (JN_HASH_SIZE - 1);
      if (domain_ptr->hash_table[code]) {
	ptr1 = &jnames_ptr[domain_ptr->hash_table[code]];
	code = domain_ptr->hash_size[code];
	for (i = 0; i < code; i++) {
	  ptr2 = name;
	  while (*ptr1) {
	    if (*ptr1 != *ptr2) {
	      break;
	    }
	    ptr1++;
	    ptr2++;
	  }
	  if (*ptr1) {
	    while (*ptr1++) {	/* KEY		*/
	    }
	    while (*ptr1++) {	/* VALUE	*/
	    }
	  } else {
	    if (*ptr2) {
	      ptr1++;
	      while (*ptr1++) {	/* VALUE	*/
	      }
	    } else {
	      return((unsigned char*)(ptr1 + 1));
	    }
	  }
	}
      }
      return((unsigned char*)NULL);
    }
    domain_ptr = domain_ptr->next_ptr;
  }
  return((unsigned char*)NULL);
}

#ifdef	DEBUG
/*
 * JNAMES データベースの一覧表示
 */

void	jnList(domain)
     char	*domain;
{
  JN_DOMAIN	*domain_ptr;
  register int	i, j;
  char		*ptr1, *ptr2;

  domain_ptr = first_domain;
  while (domain_ptr != (JN_DOMAIN*)NULL) {
    fprintf(stderr, "DOMAIN:%s\n", domain_ptr->domain_name);
    if (!strcmp(domain_ptr->domain_name, domain)) {
      for (i = 0; i < JN_HASH_SIZE; i++) {
	ptr1 = jnames_ptr + domain_ptr->hash_table[i];
	for (j = 0; j < domain_ptr->hash_size[i]; j++) {
	  ptr2 = ptr1 + strlen(ptr1) + 1;
	  fprintf(stderr, "%s:%s\n", ptr1, ptr2);
	  ptr1 = ptr2 + strlen(ptr2) + 1;
	}
      }
      return;
    }
    domain_ptr = domain_ptr->next_ptr;
  }
}
#endif	/* DEBUG */

/*
 * JNAMES データベースのロード
 */

static int	load_database(file_name)
     char	*file_name;
{
  JN_DOMAIN	*domain_ptr1, *domain_ptr2;
  FILE		*fp;
  char		domain[JN_DOMAIN_SIZE];
  char		key[SMALL_BUFF];
  char		value[SMALL_BUFF];
  char		buff1[BUFF_SIZE];
  char		buff2[BUFF_SIZE];
  char		*ptr1, *ptr2;
  int		status;
  int		code;
  int		length;
  int		i, j;

  domain[0] = '\0';
  status = -1;
  fp = fopen(file_name, "r");
  if (fp != (FILE*)NULL) {
    while (fgets(buff2, sizeof(buff2), fp)) {
      to_euc(buff1, buff2, default_code);
      ptr1 = buff1;
      while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
	ptr1++;
      }
      if (*ptr1 == '#') {
	ptr1++;
	if (!strncasecmp(ptr1, INC_KEYWORD, strlen(INC_KEYWORD))) {
	  ptr1 += strlen(INC_KEYWORD);
	  while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
	    ptr1++;
	  }
	  if (*ptr1 == '\"') {
	    ptr1++;
	    ptr2 = ptr1;
	    while ((*ptr2) && (*ptr2 != '\"')) {
	      ptr2++;
	    }
	    *ptr2 = '\0';
	    load_database(ptr1);
	  }
	}
	continue;
      }
      ptr1 = jn_strcpy(key, ptr1);
      if (*ptr1 == JN_SEPARATER) {
	ptr1++;
	strcpy(domain, key);
	ptr1 = jn_strcpy(key, ptr1);
      }
      while ((*ptr1 == ' ') || (*ptr1 == '\t')) {
	ptr1++;
      }
      jn_strcpy(value, ptr1);
      if (domain[0] && key[0] && value[0]) {
#ifdef	DEBUG
	printf("DOMAIN:%s,KEY:%s,VALUE:%s\n", domain, key, value);
#endif	/* DEBUG */
	i = 1;
	domain_ptr1 = first_domain;
	while (domain_ptr1 != (JN_DOMAIN*)NULL) {
	  if (!strcmp(domain_ptr1->domain_name, domain)) {
	    i = 0;
	    break;
	  }
	  domain_ptr1 = domain_ptr1->next_ptr;
	}
	if (i) {	/* 新しいドメインを追加	*/
	  if (strlen(domain) >= JN_DOMAIN_SIZE) {
#ifdef	MNEWS
	    print_fatal("Too long jnames domain name.");
#else	/* !MNEWS */
	    fprintf(stderr, "Too long jnames domain name.\n");
#endif	/* !MNEWS */
	    fclose(fp);
	    return(-1);
	  }
	  domain_ptr1 = jn_add_domain(domain);
	  if (domain_ptr1 == (JN_DOMAIN*)NULL) {
#ifdef	MNEWS
	    print_fatal("Can't allocate jnames domain table.");
#else	/* !MNEWS */
	    fprintf(stderr, "Can't allocate jnames domain table.\n");
#endif	/* !MNEWS */
	    fclose(fp);
	    return(-1);
	  }
	}
	code = 0;
	ptr1 = key;
	while (*ptr1) {
	  code += *ptr1++;
	}
	code &= (JN_HASH_SIZE - 1);
	i = strlen(key) + strlen(value) + 2;
	if (jnames_size <= (jnames_used + i)) {

	  /*
	   * ハッシュテーブルを拡大
	   */

	  jnames_ptr = (char*)realloc(jnames_ptr,
				      jnames_size + JN_ALLOC_SIZE);
	  if (jnames_ptr == (char*)NULL) {
#ifdef	MNEWS
	    print_fatal("Can't allocate jnames hash table.");
#else	/* !MNEWS */
	    fprintf(stderr, "Can't allocate jnames hash table.\n");
#endif	/* !MNEWS */
	    fclose(fp);
	    return(-1);
	  }
	  jnames_size += JN_ALLOC_SIZE;
	}
	ptr1 = jnames_ptr + jnames_used;
	if (domain_ptr1->hash_table[code]) {

	  /*
	   * ハッシュテーブルにエントリ追加
	   */

	  length = 0;
	  ptr2 = jnames_ptr + domain_ptr1->hash_table[code];
	  for (j = 0; j < domain_ptr1->hash_size[code]; j++) {
	    length += (strlen(ptr2) + 1);
	    ptr2 = ptr2 + (strlen(ptr2) + 1);
	    length += (strlen(ptr2) + 1);
	    ptr2 = ptr2 + (strlen(ptr2) + 1);
	  }
#ifdef	DEBUG
	  if ((unsigned int)(jnames_ptr + jnames_size) <
	      (unsigned int)(ptr2 + i + jnames_used -
			     domain_ptr1->hash_table[code] - length)) {
#ifdef	MNEWS
	    print_fatal("JNAMES program error.%08X:%08X",
			jnames_ptr + jnames_size,
			ptr2 + i + jnames_used -
			domain_ptr1->hash_table[code] - length);
#else	/* !MNEWS */
	    fprintf(stderr, "JNAMES program error.%08X:%08X\n",
		    jnames_ptr + jnames_size,
		    ptr2 + i + jnames_used -
		    domain_ptr1->hash_table[code] - length);
#endif	/* !MNEWS */
	    break;
	  }
#endif	/* DEBUG */
	  length = jnames_used - domain_ptr1->hash_table[code] - length;
	  if (length) {
	    memmove(ptr2 + i, ptr2, length);
	  }
	  domain_ptr2 = first_domain;
	  while (domain_ptr2 != (JN_DOMAIN*)NULL) {
	    for (j = 0; j < JN_HASH_SIZE; j++) {
	      if (domain_ptr2->hash_table[j] > domain_ptr1->hash_table[code]) {
		domain_ptr2->hash_table[j] += i;
	      }
	    }
	    domain_ptr2 = domain_ptr2->next_ptr;
	  }
	  ptr1 = ptr2;
	} else {

	  /*
	   * ハッシュテーブルにエントリ新規作成
	   */

	  domain_ptr1->hash_table[code] = jnames_used;
	}
	strcpy(ptr1, key);
	ptr1 = ptr1 + strlen(ptr1) + 1;
	strcpy(ptr1, value);
	jnames_used += i;
	domain_ptr1->hash_size[code]++;
      }
    }
    fclose(fp);
    status = 0;
  }
  return(status);
}

/*
 * JNAMES ドメイン追加
 */

static JN_DOMAIN	*jn_add_domain(domain)
     char	*domain;
{
  JN_DOMAIN	*domain_ptr, *alloc_ptr;
  
#ifdef	DEBUG
  printf("ADD DOMAIN:%s\n", domain);
#endif	/* DEBUG */
  alloc_ptr = (JN_DOMAIN*)malloc(sizeof(JN_DOMAIN));
  if (alloc_ptr != (JN_DOMAIN*)NULL) {
    if (first_domain) {
      domain_ptr = first_domain;
      while (domain_ptr->next_ptr != (JN_DOMAIN*)NULL) {
	domain_ptr = domain_ptr->next_ptr;
      }
      domain_ptr->next_ptr = alloc_ptr;
    } else {
      first_domain = alloc_ptr;
    }
    memset(alloc_ptr, 0, sizeof(JN_DOMAIN));
    strcpy(alloc_ptr->domain_name, domain);
    alloc_ptr->next_ptr = (JN_DOMAIN*)NULL;	/* 不要ですが… */
  }
  return(alloc_ptr);
}

/*
 * JNAMES データコピー
 */

static char	*jn_strcpy(ptr1, ptr2)
     unsigned char	*ptr1;
     unsigned char	*ptr2;
{
  int	key;

  key = *ptr2;
  if ((key == '\"') || (key == '\'')) {
    ptr2++;
    while ((*ptr2 >= ' ') || (*ptr2 == '\t')) {
      if (*ptr2 == key) {
	ptr2++;
	break;
      }
      *ptr1++ = *ptr2++;
    }
  } else {
    while ((*ptr2 > ' ') && (*ptr2 != JN_SEPARATER)) {
      *ptr1++ = *ptr2++;
    }
  }
  *ptr1 = '\0';
  return((char*)ptr2);
}
#endif	/* JNAMES */
