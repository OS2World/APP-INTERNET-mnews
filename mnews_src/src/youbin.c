/*
 * Youbin support routines for mnews
 *	protocol version 2
 *
 * interfaces:
 *	void youbin_init(char *server);
 *		initialize this module
 *	void youbin_end(void);
 *		end use of youbin
 *	int  youbin_getchar(void);
 *		read one char from stdin with youbin checking
 *	void youbin_poll(void);
 *		youbin check only
 *
 *	int have_mail;
 *		1 if you have mail in mail spool on server
 *
 *	They do nothing if youbin is unavailable.
 *
 * This is public domain code.
 * written by Yasha (ITOH Yasufumi).
 *
 * $Revision: 1.3 $  $Date: 1996/10/22 12:41:53 $
 */

#include <sys/types.h>
#include <sys/time.h>

#ifdef	MNEWS
# include "compat.h"
# include "nntplib.h"
# include "field.h"
# include "kanjilib.h"
# include "mnews.h"
# include "termlib.h"
#else	/* !MNEWS */
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# ifndef NO_UNISTD
#  include <unistd.h>
# endif	/* !NO_UNISTD */
# ifndef NO_STDLIB
#  include <stdlib.h>
# endif	/* !NO_STDLIB */
# include <pwd.h>
# include <string.h>
# include <stdio.h>
#endif	/* !MNEWS */

#define	PROTOCOL_VERSION	"2"
#ifndef	UNIT_TIME
# define UNIT_TIME		180
#endif	/* !UNIT_TIME */

#ifndef	YOUBIN_SERVICE_NAME
# define YOUBIN_SERVICE_NAME		"biff"
#endif	/* !YOUBIN_SERVICE_NAME */

/*
 * 1 if you have mail, 0 otherwise
 */
int have_mail = 0;

/*
 * for state machine
 */
typedef enum {
    Y_NOT_CONNECT,		/* not connected */
    Y_HUP,			/* not connected (suspended) */
    Y_IN_PROGRESS,		/* waiting for the response for 'W' packet */
    Y_CONNECTED,		/* waiting for 'S' packet */
    Y_ST,			/* got 'S' and will send 'T' */
} ystat_t;

static ystat_t	ystate = Y_NOT_CONNECT;

/*
 * for socket
 */
static int	youbin_fd = -1;		/* youbin socket descriptor */
static struct sockaddr_in server_saddr;	/* server socket address */

/*
 * values for youbin protocol
 */
static char	buf_T[34] = "T ";
#define	userid	(buf_T + 2)	/* user identifier from server */
static int	interval;	/* timeout interval in seconds from server */

/*
 * for timing
 */
static int	youbin_active = 0;	/* nonzero if youbin is active */
static struct timeval	youbin_timeout;
static struct timeval	youbin_lasttime;
static struct timeval	youbin_packet_time;

/*
 * set timeout for select()
 */
static void
set_wait(sec)
  int sec;
{
    youbin_timeout.tv_sec = sec;
    youbin_timeout.tv_usec = 0;

    youbin_active = sec;
}

/*
 * send udp packet to youbind
 */
static void
send_packet(str)
  char *str;
{
    int len;

#ifdef	DEBUG
    {
	time_t t;

	t = time((time_t *) 0);
	printf("send: %s\t\t%s", str, asctime(localtime(&t)));
    }
#endif	/* DEBUG */
    len = strlen(str);
    if (sendto(youbin_fd, str, len, 0,
		(struct sockaddr *)&server_saddr, sizeof server_saddr) != len)
	perror("youbin: sendto");
}

/*
 * request service
 */
static void
send_W()
{
#ifndef	MNEWS
    struct passwd *pw;
#endif	/* !MNEWS */
    char buf[256];

#ifdef	MNEWS
    sprintf(buf, "W %s %s", passwd->pw_name, PROTOCOL_VERSION);
#else	/* !MNEWS */
    if (!(pw = getpwuid(getuid()))) {
	fprintf(stderr, "youbin: user unknown\n");
	exit(1);
    }
    sprintf(buf, "W %s %s", pw->pw_name, PROTOCOL_VERSION);
#endif	/* !MNEWS */
    send_packet(buf);
    ystate = Y_IN_PROGRESS;
    set_wait(UNIT_TIME);
}

/*
 * end use (by client)
 */
static void
send_Q()
{
    char buf[64];

    sprintf(buf, "Q %s", userid);
    send_packet(buf);
}

/*
 * check 'S' packet
 */
static void
check_status(p)
  char *p;
{
    static unsigned long mbox_osize = 0, mbox_odate = 0;
    unsigned long mbox_size, mbox_date;

    if (sscanf(p, " %lu %ld", &mbox_size, (long *) &mbox_date) != 2) {
#ifndef	MNEWS
	fprintf(stderr, "youbin: illegal 'S' packet\n");
#endif	/* !MNEWS */
	return;
    }
    if (mbox_size > mbox_osize || (mbox_size && mbox_date > mbox_odate)) {
	/*
	 * new mail arrived
	 */
#ifdef	TEST
	printf("\7You have new mail\r\n");
#else	/* !TEST */
#ifdef	MNEWS
	if (bell_mode) {
	    term_bell();
	    fflush(stdout);
	}
#else	/* !MNEWS */
	write(1, "\7", 1);
#endif	/* !MNEWS */
#endif	/* !TEST */
    }

    have_mail = mbox_size ? 1 : 0;

    mbox_osize = mbox_size;
    mbox_odate = mbox_date;

    /*
     * packet received time
     */
    gettimeofday(&youbin_packet_time, (struct timezone *) 0);
}

/*
 * packet received
 */
static void
packet_receive()
{
    char buf[256];
    struct sockaddr addr;
    int len, addrlen;

    /*
     * receive UDP packet
     */
    addrlen = sizeof addr;
    if ((len = recvfrom(youbin_fd, buf, sizeof buf, 0, &addr, &addrlen)) < 0) {
	perror("youbin: recvfrom");
	return;
    }
    buf[len] = '\0';
#ifdef	DEBUG
    {
	time_t t;

	t = time((time_t *) 0);
	printf("recv: %s\t\t%s", buf, asctime(localtime(&t)));
    }
#endif	/* DEBUG */

    /*
     * packets to be received:
     *	R userid interval	registered
     *	NAK reason		negative acknowledge
     *	S size date		status
     *	Q hup			quit (suspend)
     *	Q quit			quit
     */
    if (*buf == 'Q') {
	/*
	 * disconnected by server
	 */
	if (!strcmp(buf + 2, "hup")) {
	    ystate = Y_HUP;
	    set_wait(UNIT_TIME *2  + rand() % UNIT_TIME);
	} else {
	    ystate = Y_NOT_CONNECT;
	    youbin_active = 0;	/* no timeout */
#ifndef	MNEWS
	    fprintf(stderr, "youbin: biff service closed\n");
#endif	/* !MNEWS */
	}
	have_mail = 0;
    } else {
	switch (ystate) {
	case Y_NOT_CONNECT:	/* do nothing */
	case Y_HUP:
#ifdef	DEBUG
	    fprintf(stderr, "youbin: unexpected packet ignored\n");
#endif	/* DEBUG */
	    break;
	case Y_IN_PROGRESS:
	    switch (*buf) {
	    case 'R':
		sscanf(buf+1, " %s %d", userid, &interval);
		ystate = Y_CONNECTED;
		set_wait(UNIT_TIME *2  + rand() % UNIT_TIME);
		gettimeofday(&youbin_packet_time, (struct timezone *) 0); /*?*/
		break;
	    case 'N':
		ystate = Y_NOT_CONNECT;
#ifdef	MNEWS
		/*
		 * This is wrong for pager mode, but worth warning.
		 */
		print_warning("youbin refused: %s", buf+4);
#else	/* !MNEWS */
		fprintf(stderr, "youbin refused: %s\n", buf+4);
#endif	/* !MNEWS */
		break;
	    default:
#ifdef	DEBUG
		fprintf(stderr, "youbin: unexpected packet ignored\n");
#endif	/* DEBUG */
		break;
	    }
	    break;
	case Y_CONNECTED:
	    if (*buf == 'S') {
		check_status(buf + 2);
		ystate = Y_ST;
		set_wait(rand() % (UNIT_TIME * 2 / 3));
	    } else {
#ifdef	DEBUG
		fprintf(stderr, "youbin: unexpected packet ignored\n");
#endif	/* DEBUG */
	    }
	    break;
	case Y_ST:
	    if (*buf == 'S') {
		check_status(buf + 2);
		send_packet(buf_T);		/* send 'T' packet */
		ystate = Y_CONNECTED;
		set_wait(interval);
		gettimeofday(&youbin_packet_time, (struct timezone *) 0);
	    } else {
#ifdef	DEBUG
		fprintf(stderr, "youbin: unexpected packet ignored\n");
#endif	/* DEBUG */
	    }
	    break;
	}
    }
}

/*
 * timeout occurred
 */
static void
y_timeout()
{
    switch (ystate) {
    case Y_NOT_CONNECT:	/* do nothing */
#ifdef	DEBUG
	fprintf(stderr, "youbin: this can\'t happen\n");
#endif	/* DEBUG */
	break;
    case Y_HUP:
    case Y_IN_PROGRESS:
    case Y_CONNECTED:
	send_W();	/* retry connection */
	break;
    case Y_ST:
	send_packet(buf_T);	/* send 'T' packet */
	ystate = Y_CONNECTED;
	set_wait(interval);
	gettimeofday(&youbin_packet_time, (struct timezone *) 0);
	break;
    }
}

/*
 * calculate x - y of struct timeval
 * and set the answer to x
 */
static void
sub_timeval(x, y)
  struct timeval *x, *y;
{
    if (x->tv_sec < y->tv_sec) {
	x->tv_sec = x->tv_usec = 0;
    } else {
	x->tv_sec -= y->tv_sec;
	if (x->tv_usec < y->tv_usec) {
	    if (x->tv_sec) {
		x->tv_sec--;
		x->tv_usec += 1000000 - y->tv_usec;
	    } else
		x->tv_usec = 0;
	} else
	    x->tv_usec -= y->tv_usec;
    }
}

/*
 * socket handling
 */
static int
open_socket(hostname)
  char *hostname;	/* server name */
{
    struct hostent *host;
    struct servent *service;
    struct sockaddr_in client_saddr;

    /*
     * create server socket address (server_saddr)
     */
    memset((char *) &server_saddr, 0, sizeof server_saddr);
    if ((host = gethostbyname(hostname))) {
	/*
	 * host found
	 */
	memcpy((char *) &server_saddr.sin_addr.s_addr, (char *) host->h_addr,
	       host->h_length);
	server_saddr.sin_family = host->h_addrtype;
    } else if ((server_saddr.sin_addr.s_addr = inet_addr(hostname)) != -1) {
	/*
	 * IP address
	 */
	server_saddr.sin_family = AF_INET;
    } else {
#ifdef	MNEWS
	print_warning("youbin: Can't get address: %s", hostname);
#else	/* !MNEWS */
	fprintf(stderr, "youbin: Can't get address: %s\n", hostname);
#endif	/* !MNEWS */
	return 0;
    }
    if (!(service = getservbyname(YOUBIN_SERVICE_NAME, "udp"))) {
#ifdef	MNEWS
	print_warning("youbin: No such service: %s", YOUBIN_SERVICE_NAME);
#else	/* !MNEWS */
	fprintf(stderr, "youbin: No such service: %s\n", YOUBIN_SERVICE_NAME);
#endif	/* !MNEWS */
	return 0;
    }
    server_saddr.sin_port = service->s_port;

    /*
     * create local socket address (client_saddr)
     */
    memset((char *) &client_saddr, 0, sizeof client_saddr);
    client_saddr.sin_family = AF_INET;
    client_saddr.sin_port = htonl(0);

    /*
     * open socket
     */
    if ((youbin_fd = socket(server_saddr.sin_family, SOCK_DGRAM, 0)) < 0) {
	perror("youbin: socket");
	return 0;
    }

    /*
     * bind local address
     */
    if (bind(youbin_fd, (struct sockaddr *) &client_saddr,
					sizeof client_saddr) < 0) {
	close(youbin_fd);
	youbin_fd = -1;
	perror("youbin: bind");
	return 0;
    }

    return 1;		/* success */
}

/*
 * interfaces
 */

/*
 * initialize
 */
void
youbin_init(server)
  char *server;
{
    if (server && *server && open_socket(server)) {
	srand(getpid());
	youbin_timeout.tv_sec = youbin_timeout.tv_usec = 0;
#ifdef	MNEWS
	print_warning("youbin: Using service on %s", server);
#endif	/* MNEWS */
	send_W();
	gettimeofday(&youbin_lasttime, (struct timezone *) 0);
    } else {
#ifdef	MNEWS
	print_warning("youbin: Service unavailable");
#else	/* !MNEWS */
	fprintf(stderr, "youbin: Service unavailable\n");
#endif	/* !MNEWS */
    }
}

/*
 * end use
 */
void
youbin_end()
{
    switch (ystate) {
    case Y_CONNECTED:
    case Y_ST:
	send_Q();	/* disconnect by client */
	break;
    default:
	break;		/* do nothing */
    }
    ystate = Y_NOT_CONNECT;
    have_mail = 0;
    youbin_active = 0;	/* no timeout */
    if (youbin_fd != -1) {
	close(youbin_fd);
	youbin_fd = -1;
    }
}

/*
 * return true if the youbin service must be timed out
 */
static int
y_is_disconnected(now)
  struct timeval *now;
{
    struct timeval time1, timeout;

    if (ystate == Y_CONNECTED) {	/* 'T' is sent */
	timeout.tv_sec = interval;
	timeout.tv_usec = 0;
    } else if (ystate == Y_ST) {	/* 'S' is received */
	timeout.tv_sec = UNIT_TIME * 5;
	timeout.tv_usec = 0;
    } else
	return 0;

    time1 = *now;
    sub_timeval(&time1, &youbin_packet_time);
    sub_timeval(&timeout, &time1);

    return timeout.tv_sec == 0;		/* I don't mind subsecond :-) */
}

/*
 * discard all packets from youbind
 */
static void
y_discard_packets()
{
    fd_set fdset;
    struct timeval timeout0;
    int nactive;
    char buf[256];
    struct sockaddr addr;
    int addrlen;

    do {
	timeout0.tv_sec = timeout0.tv_usec = 0;
	FD_ZERO(&fdset);
	FD_SET(youbin_fd, &fdset);
	if ((nactive = select(youbin_fd + 1, &fdset, (fd_set *) 0,
				    (fd_set *) 0, &timeout0)) > 0) {
	    addrlen = sizeof addr;
	    recvfrom(youbin_fd, buf, sizeof buf, 0, &addr, &addrlen);
#ifdef	DEBUG
	    fprintf(stderr, "youbin: discarding %c packet\n", buf[0]);
#endif	/* DEBUG */
	}
    } while (nactive > 0);
}

/*
 * read keyboard with youbin checking
 */
int
youbin_getchar()
{
    fd_set fdset;
    int nactive;
    struct timeval time1, time2, ytimeout;
    char c;

#ifdef	MNEWS
    fflush(stdout);
#endif	/* MNEWS */
    if (youbin_active) {
	gettimeofday(&time1, (struct timezone *) 0);

	if (y_is_disconnected(&time1)) {
	    /*
	     * clearly disconnected by timeout
	     */
#ifdef	DEBUG
	    fprintf(stderr, "youbin: service must be timed out -- retrying\n");
#endif	/* DEBUG */
	    y_discard_packets();
	    send_W();
	} else {
	    sub_timeval(&time1, &youbin_lasttime);
	    sub_timeval(&youbin_timeout, &time1);
	}
	do {
	    gettimeofday(&time1, (struct timezone *) 0);
	    FD_ZERO(&fdset);
	    FD_SET(0, &fdset);		/* stdin */
	    FD_SET(youbin_fd, &fdset);
	    /*
	     * select(2) may or may not overwrite timeval structure
	     * in future releases, so we use new variable.
	     */
	    ytimeout = youbin_timeout;
	    nactive = select(youbin_fd + 1, &fdset, (fd_set *) 0,
						(fd_set *) 0, &ytimeout);
	    if (nactive != 0) {
		gettimeofday(&time2, (struct timezone *) 0);
		sub_timeval(&time2, &time1);
		sub_timeval(&youbin_timeout, &time2);
	    }
	    if (nactive < 0) {
		/*
		 * interrupt?
		 */
#ifdef	DEBUG
		perror("youbin: select");
#endif	/* DEBUG */
		continue;	/* ? */
	    } else if (nactive == 0) {
		/*
		 * timeout
		 */
		y_timeout();
	    } else if (FD_ISSET(youbin_fd, &fdset)) {
		/*
		 * packet come
		 */
		packet_receive();
	    }
	} while (!FD_ISSET(0, &fdset));	/* stdin */

	gettimeofday(&youbin_lasttime, (struct timezone *) 0);
    }

    return (read(0, &c, 1) == 1) ? (unsigned char) c : EOF;
}

/*
 * process already arrived packets
 */
void
youbin_poll()
{
    fd_set fdset;
    struct timeval timeout0;
    int nactive;

    if (youbin_active) {
	do {
	    timeout0.tv_sec = timeout0.tv_usec = 0;
	    FD_ZERO(&fdset);
	    FD_SET(youbin_fd, &fdset);
	    if ((nactive = select(youbin_fd + 1, &fdset, (fd_set *) 0,
					(fd_set *) 0, &timeout0)) > 0)
		packet_receive();
	} while (nactive > 0);
    }
}

#ifdef	MNEWS
extern char	*youbin_server_file;	/* in config.c */

char *
get_mail_server()
{
    FILE *fp;
    register char *p, *q;
    char buff[BUFF_SIZE];
    static char mail_server_name[64];

    if (p = getenv("MAILSERVER")) {
	if (strlen(p) >= sizeof mail_server_name) {
	    print_fatal("youbin: MAILSERVER: Hostname too long.");
	    sleep(ERROR_SLEEP);
	    return (char *) 0;
	}
	strcpy(mail_server_name, p);
	return mail_server_name;
    }
    if (!*youbin_server_file || !(fp = fopen(youbin_server_file, "r")))
	return (char *) 0;
    while (fgets(buff, sizeof buff, fp)) {
	for (p = buff; *p == '\t' || *p == ' '; p++)
	    ;
	q = p;
	if ((*p == '\n') || (*p == '#'))
	    continue;
	for (q = p; *q && !isspace(*q); q++)
	    ;
	*q = '\0';
	fclose(fp);
	strcpy(mail_server_name, p);
	return mail_server_name;
    }
    fclose(fp);
    return (char *) 0;
}
#endif	/* MNEWS */

#ifdef	TEST
int
main(argc, argv)
  int argc;
  char *argv[];
{
    int c;

    youbin_init(argv[1]);
    while ((c = youbin_getchar()) != 'q' && c != EOF) {
	putchar(c);
	fflush(stdout);
    }

    youbin_end();
    return 0;
}
#endif	/* TEST */
