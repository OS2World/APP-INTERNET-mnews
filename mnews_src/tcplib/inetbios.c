
/*	inetbios.c	InetBIOS support libraly	Ver 0.44
 *
 *	Copyleft(c)	1992,1993 sawada@murai-electric.co.jp
 *	Permit anyone to use, modify, redistribute this software.
 *	last modified at 13 Jun 1994 by sawada@murai-electric.co.jp
 */
/* "peek" replaced with "inetpeek" by Misick, 94/ 4/ 9 */

#if	defined(MSDOS) && defined(INETBIOS)

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dos.h>
#include	"inetbios.h"

extern	int	debug;

int		find_inet( void );
int		htons( int );
int		ntohs( int );
int		socket( int, int, int );
int		connect( int, struct sockaddr_in *, int );
int		inetpeek( int );
int		listen( int, int );
int		accept( int, struct sockaddr_in *, int * );
int		bind( int, struct sockaddr_in *, int );
int		_recv( int, char *, int, int );
int		recv( int, char *, int, int );
int		recvfrom( int, char *, int, int, struct sockaddr_in *, int * );
int		send( int, char *, int, int );
int		sendto( int, char *, int, int, struct sockaddr_in *, int * );
int		_shutdown( int, int );
int		_soclose( int );
int		shutdown( int, int );
int		soclose( int );
int		sallclose( void );
int		gethostname( char *, int );
unsigned long	inet_addr( char * );
struct hostent	*gethostbyname( char * );
struct servent	*getservbyname( char *, char * );

#define	MAXCONNECTION	8
#ifdef	GNUDOS
#define	TCP_BUFF_SIZE	32768			/* �����Хåե���������	*/
#else	/* !GNUDOS */
#define	TCP_BUFF_SIZE	(size_t)32768		/* �����Хåե���������	*/
#endif	/* !GNUDOS */

static	struct	TCB	{			/* Inet Bios ���湽¤��	*/
			char	reserved[16];		/* ͽ���ΰ�	*/
			unsigned char	command;	/* ���ޥ��	*/
			unsigned char	result;		/* �������	*/
			void		(* async)();	/* ��Ʊ������	*/
			unsigned char	complete;	/* ��λ�ե饰	*/
			unsigned char	adapter;	/* �����ץ��ֹ�	*/
			unsigned char	service;	/* �����ӥ������� */
			unsigned char	padding;	/* ̤����	*/
			int		flags;		/* ���ץ����	*/
			int		endpoint;	/* ü�����̻�	*/
			int		connection;	/* ��³���̻�	*/
			char *		prmbuff;	/* ��1buff����	*/
			unsigned int	prmlen;		/* ��1buffĹ	*/
			char *		scndbuff;	/* ��2buff����	*/
			unsigned int	scndlen;	/* ��2buffĹ	*/
			int		locleng;	/* ���ɥ쥹Ĺ	*/
			u_short		locport;	/* �ݡ����ֹ�	*/
			u_long		locaddr;	/* IP���ɥ쥹	*/
			char		locaddrs[8];	/* IP���ɥ쥹	*/
			int		remleng;	/* ���ɥ쥹Ĺ	*/
			u_short		remport;	/* �ݡ����ֹ�	*/
			u_long		remaddr;	/* IP���ɥ쥹	*/
			char		remaddrs[8];	/* IP���ɥ쥹	*/
			char		useflag;	/* ������ե饰	*/
#ifdef	GNUDOS
			short		selector;	/* ���쥯��	*/
			char *		address;	/* �ݥ���	*/
			short		segment;	/* ��������	*/
			char *		buffers;	/* �ݥ���	*/
#endif	/* GNUDOS */
				} TCB[ MAXCONNECTION ];

#define	INETB_OPEN	1
#define	INETB_CLOSE	2
#define	INETB_CALL	3
#define	INETB_LISTEN	4
#define	INETB_HANGUP	5
#define	INETB_SEND	6
#define	INETB_CAPACITY	7
#define	INETB_RECV	8
#define	INETB_PEEK	9
#define	INETB_ADPINFO	0x41
#define	INETB_NO_WAIT	0x80

#define	SRV_DG		0
#define	SRV_STRM	5

static	int		InetVect = 0x7E;
static	int		loaded = 0;
static	char		*TCB_buff[MAXCONNECTION];
static	long		TCB_fpos[MAXCONNECTION];
#ifdef	GNUDOS
static	long		TCB_fsiz[MAXCONNECTION];
#endif	/* GNUDOS */

#ifdef	GNUDOS
char *
conventional_malloc( socket_num )
short	socket_num;
{
union	REGS	reg;
	char	*address;

	reg.x.ax = 0x0000FF48;
	reg.x.bx = ( TCP_BUFF_SIZE + 80 ) / 16;
	intdos( &reg, &reg );
	if ( reg.x.ax == 0 )	return 0;
	TCB[socket_num].selector = reg.x.ax;
	address	= (char *)( reg.x.bx + 0xE0000000 );
	TCB[socket_num].address = address;
	memset( address, 0, TCP_BUFF_SIZE + 80 );
	TCB[socket_num].segment = ( (int)address & 0x000FFFF0 ) >> 4;
	TCB[socket_num].buffers = address + 80;
	return address;
}

int
conventional_free( socket_num )
short	socket_num;
{
	short	selector, result;

	if ( selector = TCB[socket_num].selector ) {
asm volatile( "push %%es; movw %2,%%es; int $0x21; pop %%es"
		: "=a"(result) : "a"((short)0xFF49), "r"(selector) );
	    if ( result == 0 ) {
		TCB[socket_num].selector = 0;
		TCB[socket_num].address  = 0;
		TCB[socket_num].segment  = 0;
		TCB[socket_num].buffers  = 0;
	    }
	    return (int)result;
	}
	return 0;
}
#endif	/* GNUDOS */

int
find_inet()			/* InetBIOS �Ȥ߹��߳�ǧ	*/
{
	char 	*cp, *ep;
#ifdef	GNUDOS
	int	ad;
#endif	/* GNUDOS */

	if ( ep = getenv( "INETVECT" ) )
	    sscanf( ep, "%x", &InetVect );
#ifdef	GNUDOS
	ad = *(int *)( ( InetVect * 4 ) + 0xE0000000 );
	cp = (char *)( ( ( ad >> 12 ) & 0x000FFFF0 )
					+ ( ad & 0x0000FFFF ) | 0xE0000000 );
#else	/* !GNUDOS */
	cp = *(char **)(InetVect * 4L);
#endif	/* !GNUDOS */
#ifdef	GNUDOS
	while( (int)cp & 0x000FFFFF ) {		/* Next table address	*/
#else	/* !GNUDOS */
	while( cp ) {				/* Next table address	*/
#endif	/* !GNUDOS */
	    if ( *( cp + 10 ) == 'A' && *( cp + 11 ) == 'S' &&
		  *( cp + 12 ) == 'C' && *( cp + 13 ) == '!' &&
		    *( cp + 14 ) == 'I' && *( cp + 15 ) == 'N' &&
		      *( cp + 16 ) == 'E' && *( cp + 17 ) == 'T' ) {
		loaded = 1;
		return 1;				/* Found	*/
	    }
#ifdef	GNUDOS
	    ad = *(int *)( cp + 6 );
	    cp = (char *)( ( ( ad >> 12 ) & 0x000FFFF0 )
					| ( ad & 0x0000FFFF ) | 0xE0000000 );
#else	/* !GNUDOS */
	    cp = *(char **)( cp + 6 );
#endif	/* !GNUDOS */
	}
	return 0;					/* Not found	*/
}

int
htons( n )			/* CPU -> Network ordr		*/
int	n;
{
	return ( (( n & 0xFF00 ) >> 8 ) | (( n & 0x00FF ) << 8 ) ) ;
}

int
ntohs( n )			/* Network ordr -> CPU		*/
int	n;
{
	return ( (( n & 0xFF00 ) >> 8 ) | (( n & 0x00FF ) << 8 ) ) ;
}

int
tcb_print( tcb )
struct TCB	*tcb;
{
	unsigned char	*cp, adr[8];

	switch( tcb->command ) {
	    case INETB_OPEN:		cp = "OPEN";		break;
	    case INETB_CLOSE:		cp = "CLOSE";		break;
	    case INETB_CALL:		cp = "CALL";		break;
	    case INETB_LISTEN:		cp = "LISTEN";		break;
	    case INETB_HANGUP:		cp = "HANGUP";		break;
	    case INETB_SEND:		cp = "SEND";		break;
	    case INETB_CAPACITY:	cp = "CAPACITY";	break;
	    case INETB_RECV:		cp = "RECV";		break;
	    case INETB_PEEK:		cp = "PEEK";		break;
	}
	printf( "Command %-8s  Service %-3s  eid %d  cid %d  Result %d  ", cp,
				(tcb->service == SRV_STRM ? "TCP" : "UDP"),
				tcb->endpoint, tcb->connection, tcb->result );
	printf( "Length %d\n", tcb->prmlen );
	*(long *)adr = tcb->locaddr;
	printf( "LocADDR %d.%d.%d.%d  Port %d    ",
			adr[0], adr[1],	adr[2], adr[3], ntohs( tcb->locport ));
	*(long *)adr = tcb->remaddr;
	printf( "RemADDR %d.%d.%d.%d  Port %d\n",
			adr[0], adr[1],	adr[2], adr[3], ntohs( tcb->remport ));
	printf( "Data %-70.70s\n", tcb->prmbuff );
	return 0;
}

int
ib_request( tcb )		/* Inet BIOS ������		*/
struct TCB	*tcb;
{
union	REGS	reg;
struct	SREGS	sreg;

#ifdef	GNUDOS
	short	selector, result;
	char	*s, *d;
	short	n;

#ifdef	DEBUGGING
	if ( debug & 32768 ) {
	    printf( "Inet Call\n" );	tcb_print( tcb );
	}
#endif	/* DEBUGGING */
	*( tcb->address + 16 ) = tcb->command;
	*( tcb->address + 23 ) = tcb->adapter;
	*( tcb->address + 24 ) = tcb->service;
	*(unsigned short *)( tcb->address + 26 ) = tcb->flags;
	*(unsigned short *)( tcb->address + 28 ) = tcb->endpoint;
	*(unsigned short *)( tcb->address + 30 ) = tcb->connection;
	*(unsigned short *)( tcb->address + 32 ) = 80;
	*(unsigned short *)( tcb->address + 34 ) = tcb->segment;
	*(unsigned short *)( tcb->address + 36 ) = tcb->prmlen;
	if ( tcb->command == INETB_SEND ) {
	    memcpy( tcb->buffers, tcb->prmbuff, tcb->prmlen );		/* */
	}
	*(unsigned short *)( tcb->address + 44 ) = tcb->locleng;
	*(unsigned short *)( tcb->address + 46 ) = tcb->locport;
	*(unsigned int   *)( tcb->address + 48 ) = tcb->locaddr;
	*(unsigned short *)( tcb->address + 60 ) = tcb->remleng;
	*(unsigned short *)( tcb->address + 62 ) = tcb->remport;
	*(unsigned int   *)( tcb->address + 64 ) = tcb->remaddr;
	selector = tcb->selector;

asm( "push %%esi; push %%edi; push %%es; movw %1,%%es; int $0x7f; pop %%es; pop %%edi; pop %%esi"
					: :"b"((short)0), "r"(selector));

	tcb->remaddr	= *(unsigned int   *)( tcb->address + 64 );
	tcb->remport	= *(unsigned short *)( tcb->address + 62 );
	tcb->remleng	= *(unsigned short *)( tcb->address + 60 );
	tcb->locaddr	= *(unsigned int   *)( tcb->address + 48 );
	tcb->locport	= *(unsigned short *)( tcb->address + 46 );
	tcb->locleng	= *(unsigned short *)( tcb->address + 44 );
	tcb->prmlen	= *(unsigned short *)( tcb->address + 36 );
	if ( tcb->command == INETB_RECV ) {
	    memcpy( tcb->prmbuff, tcb->buffers, tcb->prmlen );		/* */
	}
	tcb->connection = *(unsigned short *)( tcb->address + 30 );
	tcb->endpoint   = *(unsigned short *)( tcb->address + 28 );
	tcb->flags      = *(unsigned short *)( tcb->address + 26 );
	tcb->complete   = *( tcb->address + 22 );
	tcb->result     = *( tcb->address + 17 );
#else	/* !GNUDOS */
	char	*p;

#ifdef	DEBUGGING
	if ( debug & 32768 ) {
	    printf( "Inet Call\n" );	tcb_print( tcb );
	}
#endif	/* DEBUGGING */
	p = (char *)tcb->reserved;
	sreg.es  = FP_SEG( p );
	reg.x.bx = FP_OFF( p );
	int86x( InetVect + 1, &reg, &reg, &sreg );
#endif	/* !GNUDOS */
	if ( tcb->result == 58 )			/* Eshutdown	*/
	    tcb->useflag = -1;
#ifdef	DEBUGGING
	if ( debug & 32768 ) {
	    printf( "Inet Return\n" );	tcb_print( tcb );
	}
#endif	/* DEBUGGING */
	return (int)tcb->result ;
}

int
socket( domain, type, protocol )
int	domain, type, protocol;
{
static	int	endflag = 0;
	int	n;

	if ( domain != PF_INET )	return -2;	/* PF_INET onry	*/
	if ( loaded == 0 )
	    if ( find_inet() == 0 )
		return -9;			/* InetBIOS not loaded	*/
	for ( n=0 ; n<MAXCONNECTION ; n++ ) {
	    if ( TCB[ n ].useflag == 0 )
		break;
	}
	if ( n == MAXCONNECTION )	return -3;
	switch( type ) {
	    case SOCK_STREAM:	TCB[ n ].service = SRV_STRM;	break;
	    case SOCK_DGRAM:	TCB[ n ].service = SRV_DG;	break;
	    default:			return -4;
	}
	if ( endflag == 0 ) {
#ifdef	GNUDOS
	    if ( atexit( (void (*)())sallclose ) != 0 )	/* ��λ�ؿ���Ͽ	*/
#else	/* !GNUDOS */
	    if ( onexit( sallclose ) == NULL )		/* ��λ�ؿ���Ͽ	*/
#endif	/* !GNUDOS */
					return -5;
	    endflag = 1;
	}							/* */
#ifdef	GNUDOS
	if ( TCB[ n ].selector == 0 )
	    if ( conventional_malloc( n ) == 0 )
					return -6;
#endif	/* GNUDOS */
	TCB[ n ].useflag = -1;
	return ( n + 1 );
}

int
connect( s, name, namelen )
int	s;
struct sockaddr_in *name;
int	namelen;
{
	int	n;

	if ( name->sin_family != PF_INET ) return -2;	/* PF_INET onry	*/
	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	if ( TCB[ n ].useflag == 0 )	return -8;
	if ( TCB[ n ].endpoint == 0 ) {
	    TCB[ n ].command = INETB_OPEN;		/* �ݡ��ȥ����ץ� */
	    if ( ib_request( &TCB[ n ] ) )	return -1;
	}
	TCB[ n ].command  = INETB_CALL;			/* �ݡ�����³�׵� */
	TCB[ n ].remleng  = 8;
	TCB[ n ].remport  = name->sin_port;
	TCB[ n ].remaddr  = name->sin_addr.s_addr;
	if ( ib_request( &TCB[ n ] ) )	return -1;	/* Connect error */
	TCB[ n ].useflag  = 1;
	return TCB[ n ].connection;
}

int
inetpeek( s )
int	s;
{
	int	n;

	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	if ( TCB[ n ].useflag == 0 )	return -8;
	TCB[ n ].command = INETB_PEEK;			/* PEEK command */
	if ( ib_request( &TCB[ n ] ) )	return -1;	/* Peek error	*/
	if ( TCB[ n ].result == 58 )	return 0;	/* EShutdown	*/
	return TCB[ n ].prmlen;
}

int
listen( s, backlog )
int	s, backlog;
{
	int	n;

	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	if ( TCB[ n ].useflag == 0 )	return -8;
	if ( backlog > 5 )		return -5;
	if ( TCB[ n ].service != SRV_STRM )	return -4;
	TCB[ n ].command = INETB_LISTEN;		/* LISN command */
	if ( ib_request( &TCB[ n ] ) )	return -1;	/* Lisn error	*/
	if ( TCB[ n ].result == 58 )	return 0;	/* EShutdown	*/
	return TCB[ n ].result;
}

int
accept( s, name, namelen )
int	s;
struct sockaddr_in *name;
int	*namelen;
{
	int	n;

	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	if ( TCB[ n ].useflag == 0 )	return -8;
	name->sin_port = TCB[ n ].remport;
	name->sin_addr.s_addr = TCB[ n ].remaddr;
	*namelen = 8;
#ifdef	DEBUGGING
	if ( debug & 32768 ) {
	    printf( "Inet Accept\n" );	tcb_print( &TCB[n] );
	}
#endif	/* DEBUGGING */
	return s;
}

int
bind( s, name, namelen )
int	s;
struct sockaddr_in *name;
int	namelen;
{
	int	n;
	unsigned long	address;

#ifdef	DEBUGGING
	if ( debug & 32768 )
	    printf( "Inet Bind\n" );
#endif	/* DEBUGGING */
	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	if ( TCB[ n ].useflag == 0 )	return -8;
	TCB[ n ].locport = name->sin_port;
	TCB[ n ].locaddr = name->sin_addr.s_addr;
	TCB[ n ].command = INETB_OPEN;		/* �ݡ��ȥ����ץ��׵�	*/
	if ( ib_request( &TCB[ n ] ) )	return -1;	/* Open error	*/
	name->sin_port   = TCB[ n ].locport;
	name->sin_addr.s_addr = TCB[ n ].locaddr;
	return 0;
}

int
_recv( s, buf, len, flags )
int	s;
char	*buf;
int	len, flags;
{
	int	n, p;

#ifndef	NORCVWAIT
	n = p = 0 ;
	while( 1 ) {
#ifdef	GNUDOS
	    if ( inportb( 0x60 ) & 0x20 ) {	/* PC9801��¸ ��ʿ����	*/
#else	/* !GNUDOS */
	    if ( inp( 0x60 ) & 0x20 ) {
#endif	/* !GNUDOS */
		if ( p == 0 ) {
		    if ( inetpeek( s ) ) {	/* ����ʸ������ǧ	*/
			break;
		    } else {
			if ( ++n > 168 )	/* 17.72mS * 168 = 2977mS */
			    return -9 ;		/* Recive time out return */
		    }
		    p = n ;
		}
	    } else   p = 0 ;
	}
#endif	/* NORCVWAIT */
	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	if ( TCB[ n ].useflag == 0 )	return -8;
	TCB[ n ].command = INETB_RECV;			/* RECV command */
	TCB[ n ].flags   = flags;
	TCB[ n ].prmlen  = len;
	TCB[ n ].prmbuff = buf;
	ib_request( &TCB[ n ] );
	if ( TCB[ n ].result ) {
	    if ( TCB[ n ].result == 58 )
		return 0;				/* EShutdown	*/
	    return -1;					/* Recv error	*/
	}
	return TCB[ n ].prmlen;
}

int
recv( s, buf, len, flags )
int	s;
char	*buf;
int	len, flags;
{
#ifdef	GNUDOS
	int	n;
	unsigned int l;
	char	*cp, *mp;

	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	while( 1 ) {
	    l = inetpeek( s );
	    if ( l > 0 ) {
		if ( ( mp = malloc( TCP_BUFF_SIZE ) )== NULL )	return -2;
		if ( ( l = _recv( s, mp, TCP_BUFF_SIZE, 0 ) ) < 0 )  return l;
		if ( TCB_buff[ n ] != NULL ) {
		    TCB_buff[ n ] = realloc( TCB_buff[n], TCB_fsiz[n] + l );
		    memcpy( TCB_buff[n]+TCB_fsiz[n], mp, l );
		    TCB_fsiz[ n ] = TCB_fsiz[ n ] + l;
		    free( mp );
		} else {
		    TCB_buff[ n ] = realloc( mp, l );
		    TCB_fpos[ n ] = 0;
		    TCB_fsiz[ n ] = l;
		}
	    }
	    if ( TCB_buff[ n ] != NULL )  {
		cp = TCB_buff[ n ] + TCB_fpos[ n ];
		for( l=0 ; l<len && TCB_fpos[n]+l<TCB_fsiz[n] ; l++, cp++ ) {
		    if (( buf[ l ] = *cp )== '\n')
			break;
		}
		if ( *cp == '\n' ) {
		    buf[ ++l ] = 0;
		    if ( TCB_fpos[ n ] + l == TCB_fsiz[ n ] ) {
			free( TCB_buff[ n ] );
			TCB_buff[ n ] = NULL;
			TCB_fpos[ n ] = 0;
			TCB_fsiz[ n ] = 0;
		    } else {
			TCB_fpos[ n ] = TCB_fpos[ n ] + l;
		    }
		    return (int)strlen( buf );
		}
	    }
	}
#else	/* !GNUDOS */
	FILE	*fp;
	int	n;
	unsigned int l;
	char	*cp, *mp;

	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	while( 1 ) {
	    l = inetpeek( s );
	    if ( TCB_buff[ n ] == NULL ) {
		if ( ( cp = malloc( 64 + 1 ) )== NULL )	return -2;
		*cp = 0;
		if ( mp = getenv( "TMP" ) )
		    strcpy( cp, mp );
		strcat( cp, "\\$$$inet.$$$"  );
		cp[ strlen( cp ) - 2 ] = '0' + n;
		mp = cp;
		while( *mp ) {
		    if ( mp[0] == '\\' && mp[1] == '\\' )
			strcpy( mp, mp + 1 );
		    ++mp;
		}
		TCB_buff[ n ] = cp;
	    }
	    if ( l > 0 ) {
		if ( ( mp = malloc( TCP_BUFF_SIZE ) )== NULL )	return -2;
		if ( ( l = _recv( s, mp, TCP_BUFF_SIZE, 0 ) ) < 0 )  return l;
		if ( ( fp = fopen( TCB_buff[ n ], "ab" ) )== NULL ) return -4;
		fwrite( mp, l, 1, fp );
		fclose( fp );
		free( mp );
	    }
	    if ( ( fp = fopen( TCB_buff[ n ], "rb" ) )!= NULL )  {
		fseek( fp, TCB_fpos[ n ], SEEK_SET );
		*buf = 0;
		fgets( buf, len, fp );
		if ( strchr( buf, '\n' ) != NULL )  {
		    TCB_fpos[ n ] = ftell( fp );
		    fgetc( fp );			/* dummy read \n */
		    if ( feof( fp ) ) {
			fclose( fp );
			unlink( TCB_buff[ n ] );
			free( TCB_buff[ n ] );
			TCB_buff[ n ] = NULL;
			TCB_fpos[ n ] = 0;
		    } else {
			fclose( fp );
		    }
		    return (int)strlen( buf );
		}
		fclose( fp );
	    }
	}
#endif	/* !GNUDOS */
}

int
recvfrom( s, buf, len, flags, name, namelen )
int	s;
char	*buf;
int	len, flags;
struct sockaddr_in *name;
int	*namelen;
{
	int	ret ;

#ifdef	DEBUGGING
	if ( debug & 32768 )
	    printf( "Recvfrom %d Bytes buffer\n", len );
#endif	/* DEBUGGING */
	if ( TCB[ s - 1 ].endpoint == 0 ) {
	    if ( *namelen < sizeof( struct sockaddr_in ) )
		return -5;
	    if ( ( ret = connect( s, name, *namelen ) ) < 0 )
		return ret;
	}
	return _recv( s, buf, len, flags );
}

int
send( s, buf, len, flags )
int	s;
char	*buf;
int	len, flags;
{
	int	n;

	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	if ( TCB[ n ].useflag == 0 )	return -8;
	TCB[ n ].command  = INETB_SEND;			/* SEND command */
	TCB[ n ].flags    = flags;
	TCB[ n ].prmlen   = len;
	TCB[ n ].prmbuff  = (char *)buf;
	if ( ib_request( &TCB[ n ] ) )	return -1;	/* Send error	*/
	return TCB[ n ].prmlen;
}

int
sendto( s, buf, len, flags, name, namelen )
int	s;
char	*buf;
int	len, flags;
struct sockaddr_in *name;
int	*namelen;
{
	int	ret;

#ifdef	DEBUGGING
	if ( debug & 32768 )
	    printf( "Send to %d Bytes\n", len );
#endif	/* DEBUGGING */
	if ( TCB[ s - 1 ].endpoint == 0 ) {
	    if ( *namelen < sizeof( struct sockaddr_in ) )
		return -5;
	    if ( ( ret = connect( s, name, *namelen ) ) < 0 )
		return ret;
	}
	return send( s, buf, len, flags );
}

int
_shutdown( s, how )
int	s, how;
{
	int	n;

	n = s - 1;
	if ( TCB[ n ].service == SRV_STRM && TCB[ n ].connection ) {
	    TCB[ n ].command  = INETB_HANGUP;		/* H_UP command */
	    TCB[ n ].prmlen   = 0;
	    TCB[ n ].prmbuff  = 0;
	    if ( ib_request( &TCB[ n ] ) )	return -1; /* Hangup error */
	}
	return 0;
}

int
_soclose( s )
int	s;
{
	int	n;

	n = s - 1;
	TCB[ n ].command = INETB_CLOSE;			/* CLOSE command */
	if ( ib_request( &TCB[ n ] ) )	return -1;	/* Close error	*/
	memset( (char *)&TCB[ n ], 0, sizeof( struct TCB ) );
	if ( TCB_buff[ n ] != NULL ) {
	    unlink( TCB_buff[ n ] );
	    free( TCB_buff[ n ] );
	    TCB_buff[ n ] = NULL;
	    TCB_fpos[ n ] = 0;
	}
#ifdef	GNUDOS
	conventional_free( n );
#endif	/* GNUDOS */
	return 0;
}

int
shutdown( s, how )
int	s, how;
{
	return soclose( s );
}

int
soclose( s )
int	s;
{
	int	n;

	n = s - 1;
	if ( n >= MAXCONNECTION )	return -3;
	if ( TCB[ n ].useflag == 0 )	return  0;
	if ( TCB[ n ].useflag > 0 )	_shutdown( s, 0 );	/* SHUTDOWN */
	return _soclose( s );
}

int
sallclose()
{
	int	n;

	for ( n=0 ; n<MAXCONNECTION ; n++ ) {
	    if ( TCB[ n ].useflag != 0 ) {
		TCB[ n ].useflag = -1 ;
		shutdown( n + 1, 3 );
		soclose( n + 1 );
	    }
	}
	return 0;
}

unsigned long
inet_addr( addr )
char	*addr;
{
	char	adrs[8], work[16], *cp;

	strncpy( work, addr, 15 );
	adrs[0] = adrs[1] = adrs[2] = adrs[3] = 0;
	adrs[0] = (char)( ( cp = strtok( work, "."   ) ) ? atoi( cp ) : 0 );
	adrs[1] = (char)( ( cp = strtok( NULL, "."   ) ) ? atoi( cp ) : 0 );
	adrs[2] = (char)( ( cp = strtok( NULL, "."   ) ) ? atoi( cp ) : 0 );
	adrs[3] = (char)( ( cp = strtok( NULL, "\t " ) ) ? atoi( cp ) : 0 );
	return *(u_long *)adrs;
}

int
gethostname( name, namelen )
char	*name;
int	namelen;
{
	struct TCB   tcb;
	unsigned char adpinfo[68];
	char	*ep, nameb[16], address[16];
	int	n;

	if ( loaded == 0 )
	    if ( find_inet() == 0 )
		return -9;			/* InetBIOS not loaded	*/
	memset( adpinfo, 0, sizeof( adpinfo ) );
	tcb.command = INETB_ADPINFO;	    /* Adapter infomation command */
	tcb.prmlen  = sizeof( adpinfo );
	tcb.prmbuff = (char *)adpinfo;
	if ( !ib_request( &tcb ) ) {
	    sprintf( address, "%u.%u.%u.%u",
			adpinfo[12], adpinfo[13], adpinfo[14], adpinfo[15] );
	}
	if ( ep = getenv( "HOST" ) ) {
	    strncpy( name, ep, namelen );
	    return 0;
	}
	if ( ep = getenv( "HOSTNAME" ) ) {
	    strncpy( name, ep, namelen );
	    return 0;
	}
	puts("INETBIOS.C: �Ķ��ѿ� HOST �����Ĥ���ޤ���.\n\r");
	return -1;
}

struct hostent *
gethostbyname( name )
char	*name;
{
static	struct	hostent host_p;
static	char	buff[128], *alias_p[8];
	FILE	*fp;
	char	*ep, pathname[64], **cp;
	int	n;

	if ( ( ep = getenv( "INETBIOS" ) )== NULL ) {
	    puts( "INETBIOS.C: �Ķ��ѿ� INETBIOS �����Ĥ���ޤ���.\n\r" );
	    return NULL;
	}
	strcpy( pathname, ep );
	strcat( pathname, "\\HOSTS" );
	if ( ( fp = fopen( pathname, "r" ) )== NULL ) {
	    puts( "INETBIOS.C: HOSTS �ե����뤬���Ĥ���ޤ���.\n\r" );
	    return NULL;
	}
	while ( 1 ) {
	    fgets( buff, 127, fp );
	    if ( feof( fp ) )	break;
	    if ( ferror( fp ) )	break;
	    if ( *buff == '#' || *buff == '\r' || *buff == '\n' )
		continue;
	    host_p.h_addrtype = PF_INET;
	    host_p.h_length   = 4;
	    host_p.h_addr     = inet_addr( buff );
	    strtok( buff, "\t " );		/* dumy read ip address	*/
	    host_p.h_name     = strtok( NULL, "\t \n\r" );
	    n = 0;
	    while( alias_p[n] = strtok( NULL, "\t \n\r" ) ) {
		if ( *alias_p[n] == '#' ) {
		    *alias_p[n] = 0;
		    break;
		} else {
		    ++n;
		}
	    }
	    host_p.h_aliases = alias_p;
	    if ( strcmp( name, host_p.h_name ) == 0 )
		break;
	    for ( cp = host_p.h_aliases ; *cp ; cp++ )
		if ( strcmp( name, *cp ) == 0 )
		    break;
 	    if ( *cp )			/* FIX yamasita@omronsoft.co.jp	*/
 	    	break;
	    host_p.h_addr = 0;
	}
	fclose( fp );
#if	defined(DEBUG) || defined(DEBUGGING)
#ifdef	DEBUGGING
	if ( debug & 32768 ) {
#endif	/* DEBUGGING */
	printf("gethostbyname: %s %u.%u.%u.%u\n", host_p.h_name,
		*(((unsigned char *)&host_p.h_addr)+0),
			*(((unsigned char *)&host_p.h_addr)+1),
				*(((unsigned char *)&host_p.h_addr)+2),
				       *(((unsigned char *)&host_p.h_addr)+3));
#ifdef	DEBUGGING
	}
#endif	/* DEBUGGING */
#endif	/* (DEBUG || DEBUGGING) */
	if ( host_p.h_addr ) return ( &host_p );
	else		     return NULL;
}

struct servent *
getservbyname( name, protcol )
char	*name, *protcol;
{
static	struct	servent serv_p;
static	char	buff[128], *alias_p[8];
	FILE	*fp;
	char	*ep, pathname[64], **cp, *tp;
	int	n;

	if ( ( ep = getenv( "INETBIOS" ) )== NULL ) {
	    puts( "INETBIOS.C: �Ķ��ѿ� INETBIOS �����Ĥ���ޤ���.\n\r" );
	    return NULL;
	}
	strcpy( pathname, ep );
	strcat( pathname, "\\SERVICES" );
	if ( ( fp = fopen( pathname, "r" ) )== NULL ) {
	    puts( "INETBIOS.C: SERVICES �ե����뤬���Ĥ���ޤ���.\n\r" );
	    return NULL;
	}
	while ( 1 ) {
	    fgets( buff, 127, fp );
	    if ( feof( fp ) )	break;
	    if ( ferror( fp ) )	break;
	    n = 0;
	    serv_p.s_name  = strtok( buff, "\t \n\r" );
	    if ( tp = strtok( NULL, "/" ) )
		serv_p.s_port  = atoi( tp );
	    serv_p.s_proto = strtok( NULL, "\t \n\r" );
	    while( alias_p[n] = strtok( NULL, "\t \n\r" ) ) {
		if ( *alias_p[n] == '#' ) {
		    *alias_p[n] = 0;
		    break;
		} else {
		    ++n;
		}
	    }
	    serv_p.s_aliases = alias_p;
	    if ( strcmp( name, serv_p.s_name ) == 0 )
		goto gotname;
	    for ( cp = serv_p.s_aliases ; *cp ; cp++ )
		if ( strcmp( name, *cp ) == 0 )
		    goto gotname;
	    continue;
gotname:
	    if ( protcol == 0 || strcmp( serv_p.s_proto, protcol ) == 0 ) {
		fclose( fp );
#if	defined(DEBUG) || defined(DEBUGGING)
#ifdef	DEBUGGING
		if ( debug & 32768 ) {
#endif	/* DEBUGGING */
		    printf("getservbyname: %s %s %u\n", serv_p.s_name,
					serv_p.s_proto, serv_p.s_port );
#ifdef	DEBUGGING
		}
#endif	/* DEBUGGING */
#endif	/* (DEBUG || DEBUGGING) */
		return ( &serv_p );
	    }
	}
	fclose( fp );
	return NULL;
}

#endif	/* (MSDOS && INETBIOS) */

/****************************************************************************

1993/10/26  gethostbyname �˱����� cr �ΤߤιԤ�ͭ��� segment vioration ��ȯ��
            ����Τ��н褷����

1993/10/27  inet_addr �˱����� Class A �λ��꤬ͭ��Ȥ��������ʤäƤ����Τ���

1993/10/29  _recv     ����3�äμ��������ॢ���Ȥ����ꤷ����(PC98��¸)

1994/04/02  bind �ǥݡ����ֹ�򥻥åȤ���褦���ѹ�������

1994/04/22  peek() �� inetpeek() ���ѹ�

1994/04/13  gethostname() �� adpinfo �� char ���� unsigned char ���ѹ�������
*****************************************************************************/
