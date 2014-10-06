#!/bin/sh
# Mini News Reader Ver 1.21
# Auto configuration script for source/object release
#
VERSION="1.21"
PATH=.:$PATH

DEFAULT_CC="/bin/cc"
DEFAULT_INSTALL="/bin/install"
AWK=awk
SED=sed
LIB=""
RANLIB=ranlib
DEFAULT_CPP=""
DEFAULT_BIN_DIR="/usr/local/bin"
DEFAULT_LIB_DIR="/usr/local/lib"
DEFAULT_MAN_DIR="/usr/local/man"
DEFAULT_TMP_DIR="/tmp"
DEFAULT_CONF_OPT="-DCTRL_L -DCOMPLETION -DREF_SORT -DLARGE -DDISPLAY_CTRL"
DEFAULT_CFLAGS="-O"
DEFAULT_POSTNEWS="/usr/lib/news/inews -h"
DEFAULT_SENDMAIL="/usr/lib/sendmail -t -om -oi"
DEFAULT_EDITOR="/usr/local/bin/emacs -nw"
DEFAULT_PAGER="/usr/local/bin/less"
DEFAULT_LPR_COMMAND="lpr"
DEFAULT_MPEG_COMMAND="mpeg_play"
DEFAULT_JPEG_COMMAND="xv"
DEFAULT_GIF_COMMAND="xv"
DEFAULT_AUDIO_COMMAND="audioplay"
DEFAULT_PS_COMMAND="ghostview"
DEFAULT_MH_PATH="/usr/local/bin/mh"
DEFAULT_MAIL_SPOOL="/var/spool/mail"
DEFAULT_BOARD_SPOOL="/var/spool/board"
DEFAULT_NEWS_SPOOL="/usr/spool/news"
DEFAULT_NEWS_LIB="/usr/lib/news"

SITE_DEPEND="site_dep.h"
MAKEFILE="Makefile"
K_STRING="ASCII_CODE JIS_CODE SJIS_CODE EUC_CODE UNKNOWN_CODE"

rm -f Y_or_n y_or_N K_code echotmp

echo "--- Mini News Reader Configuration Tool ---"
echo "Please answer some questions."
echo "For example,'([y]/n)' means 'y' is default answer."
echo ""

# for echo without line-feed
echo "Checking echo to see how to suppress newlines..."
(echo "12345\c" ; echo " ") > echotmp
if grep c echotmp >/dev/null 2>&1 ; then
	n='-n'
	c=''
else
	n=''
	c='\c'
fi
\rm -f echotmp
echo $n "Type carriage return to continue.  Your cursor should be here-->$c"
read ANS

$SED -e  's/@/$/g' > Y_or_n <<EOF
#!/bin/sh
while true
do
	echo $n "@1 ([y]/n):$c"
	read INPUT
	if [ "@INPUT" = "" ] ; then
		exit 1
	fi
	if [ "@INPUT" = "y" ] ; then
		exit 1
	fi
	if [ "@INPUT" = "n" ] ; then
		exit 0
	fi
done
EOF
$SED -e  's/@/$/g' > y_or_N <<EOF
#!/bin/sh
while true
do
	echo $n "@1 (y/[n]):$c"
	read INPUT
	if [ "@INPUT" = "" ] ; then
		exit 0
	fi
	if [ "@INPUT" = "y" ] ; then
		exit 1
	fi
	if [ "@INPUT" = "n" ] ; then
		exit 0
	fi
done
EOF
$SED -e  's/@/$/g' > K_code <<EOF
#!/bin/sh
while true
do
	echo "@1"
	echo "1:No conversion"
	echo "2:JIS code"
	echo "3:SJIS code"
	echo "4:EUC code"
	echo $n "Kanji code ? (default:@2):$c"
	read INPUT
	if [ "@INPUT" = "" ] ; then
		INPUT="@2"
	fi
	case "@INPUT" in
		"1" | "2" | "3" | "4")
			exit @INPUT
			;;
		*)
			;;
	esac
done
EOF
$SED -e  's/@/$/g' > whoami.sh <<EOF
#!/bin/sh
echo \`id | sed -e 'y/()/  /' -e 's/uid=[0-9]* *//' -e 's/ *gid=[0-9]*.*//'\`
EOF

chmod 755 Y_or_n y_or_N K_code whoami.sh

ARCH=""
NET_OPT=""
CONS_OPT=""
MOD_OPT=""
DATE="date"

while [ "$ARCH" = "" ]
do
	echo "What is your system architecture?"
	echo "Please select architecture number."
	echo ""
	echo "1.SUN 4.X for SUN SUN-OS 4.X(Solaris 1.X)"
	echo "              Solbourne OS-MP 4.X, Panasonic P-Series PanaOS/S 1.X"
	echo "              Toshiba AS-Series OS/AS V4.X"
	echo "2.SUN 3.X for SUN SUN-OS 3.X, Toshiba AS-Series OS/AS V3.X"
	echo "3.DEC     for DECstation.(Ultrix 4.X)"
	echo "4.NeXT    for NeXT series.(NeXT STEP 2.X)"
	echo "5.NEWS    for NEWS series.(NEWS-OS 4.X)"
	echo "6.BSD     for 386BSD, NetBSD, FreeBSD on PC-AT/PC-9801."
	echo "7.Linux   for Linux"
	echo "8.LUNA    for LUNA series.(UNIOS-B 1.X/2.X/3.X/MACH 2.X),BSD 4.3"
	echo "9.RS6000  for IBM RS6000(AIX 3.X)"
	echo "10.HP     for HP-UX 8.X/9.X"
	echo "11.ALPHA  for DEC ALPHA-AXP"
	echo "12.SGI    for SGI IRIS Crimson,Indigo2,Indy IRIX 4.X/5.X"
	echo "13.SVR4   for other System-V Release4."
	echo "              NEC EWS4800(EWS UX/V Rel4.X), UP4800(UP-UX/V)"
	echo "              OKIstation, SUN SUN-OS 5.X(Solaris 2.X)..."
#	echo "14.SVR3   for System-V Release3."
#	echo "              FACOM A-100 (compact-A/Astation) SX/A E60b"
#	echo "              NEC EWS4800/30 (CISC) EWS-UX/V (EWOS21) rel 8.1"
#	echo "              Mitsubishi ME (CISC) series.(ME/UX)..."
#	echo "15.MS-DOS for MS-DOS with socket library"
	echo "Q.quit"
	echo ""

	echo $n "Architecture number:$c" ; read INPUT
	case "$INPUT" in
		"1")	ARCH="SUNOS_41"
			DATE="date +'%y-%m/%d(%a) %H:%M:%S'"
			DEFAULT_CC="/usr/ucb/cc"
			DEFAULT_CFLAGS="-O -target `arch`" -DUSE_TERMIOS
			DEFAULT_TMP_DIR="/var/tmp"
			Y_or_n "Is your system SUN-OS 4.1.X?"
			if [ $? -eq 0 ] ; then
				ARCH="SUNOS_40"
			fi
			;;
		"2")	ARCH="SUNOS_3X"
			DATE="date +'%y-%m/%d(%a) %H:%M:%S'"
			DEFAULT_CC="/bin/cc"
			DEFAULT_CFLAGS="-O"
			DEFAULT_INSTALL="/usr/bin/install"
			DEFAULT_MAIL_SPOOL="/usr/spool/mail"
			DEFAULT_BOARD_SPOOL="/usr/spool/board"
			;;
		"3")	ARCH="DEC"
			DEFAULT_CFLAGS="-Olimit 1024"
			;;
		"4")	ARCH="NEXT"
			DEFAULT_CFLAGS="-O -DPRINTF_ZERO"
			DEFAULT_INSTALL="/usr/bin/install"
			DEFAULT_MAIL_SPOOL="/usr/spool/mail"
			Y_or_n "Is your system Next Step 3.X?"
			if [ $? -ne 0 ] ; then
				DEFAULT_CPP="/lib/cpp"
			fi
			;;
		"5")	ARCH="NEWS"
			DEFAULT_MH_PATH="/usr/new/mh"
			;;
		"6")	ARCH="_386BSD"
			DEFAULT_CFLAGS="-O -DDONT_HAVE_FTIME -DUSE_TERMIOS"
			DEFAULT_SENDMAIL="/usr/sbin/sendmail -t -om -oi"
			DEFAULT_CC="/usr/bin/cc"
			DEFAULT_INSTALL="/usr/bin/install"
			DEFAULT_MAIL_SPOOL="/var/mail"
			DEFAULT_BOARD_SPOOL="/var/board"
			;;
		"7")	ARCH="LINUX"
			DEFAULT_CFLAGS="-O -DUSE_TERMIOS"
			DEFAULT_SENDMAIL="/usr/lib/sendmail -t -om -oi"
			DEFAULT_CC="/usr/bin/cc"
			DEFAULT_INSTALL="/usr/bin/install"
			DEFAULT_MAIL_SPOOL="/usr/spool/mail"
			DEFAULT_BOARD_SPOOL="/usr/spool/board"
			;;
		"8")	ARCH="LUNA"
			DEFAULT_CFLAGS="-O -DPRINTF_ZERO -DDONT_HAVE_VSPRINTF"
			DEFAULT_INSTALL="/usr/bin/install"
			DEFAULT_MAIL_SPOOL="/usr/spool/mail"
			DEFAULT_MH_PATH="/usr/new/mh"
			Y_or_n "Is your system UNI-OS 3.X?"
			if [ $? -eq 1 ] ; then
				DEFAULT_CFLAGS="$DEFAULT_CFLAGS -DSTRCASECMP"
			else
				Y_or_n "Is your system DG/UX?"
				if [ $? -eq 1 ] ; then
					DEFAULT_CFLAGS="-O -DPRINTF_ZERO -DTERMIO"
				fi
			fi
			;;
		"9")	ARCH="SVR4"
			DEFAULT_CFLAGS="-O -DDONT_HAVE_FTIME -DUSE_TZNAME -DUSE_TERMIOS"
			DEFAULT_CC="/bin/cc"
			DEFAULT_INSTALL="/usr/ucb/install"
			RANLIB=true
			DEFAULT_TMP_DIR="/var/tmp"
			DEFAULT_MAIL_SPOOL="/usr/spool/mail"
			DEFAULT_BOARD_SPOOL="/var/board"
			LIB="-lbsd"
			;;
		"10")	ARCH="SVR4"
			DEFAULT_CFLAGS="-O -DDONT_HAVE_FTIME -DUSE_TZNAME -DUSE_HPUX"
			DEFAULT_CC="/bin/cc"
			DEFAULT_INSTALL="/etc/install"
			RANLIB=true
			DEFAULT_TMP_DIR="/tmp"
			DEFAULT_MAIL_SPOOL="/usr/mail"
			DEFAULT_BOARD_SPOOL="/var/board"
			;;
		"11")	ARCH="DEC"
			DEFAULT_CFLAGS="-O -DALPHA -DPRINTF_ZERO -DUSE_TERMIOS"
			DEFAULT_INSTALL="/usr/bin/install"
			DEFAULT_MAIL_SPOOL="/usr/spool/mail"
			LIB="-lbsd"
			;;
		"12")	ARCH="SVR4"
			DEFAULT_CFLAGS="-cckr -O -DDONT_HAVE_FTIME -DUSE_TZNAME -DSTRCASECMP -DIRIX -USVR3 -DUSE_TERMIOS"
			DEFAULT_CC="/usr/bin/cc"
			DEFAULT_INSTALL="/usr/ucb/install"
			RANLIB=true
			DEFAULT_TMP_DIR="/usr/tmp"
			DEFAULT_MAIL_SPOOL="/usr/mail"
			DEFAULT_BOARD_SPOOL="/var/board"
			LIB=""
			Y_or_n "Is your system newer than IRIX 5.X?"
			if [ $? -eq 0 ] ; then
				Y_or_n "Is your system using NIS?"
				if [ $? -eq 1 ] ; then
					LIB="$LIB -lsun"
				fi
				DEFAULT_INSTALL="/usr/bin/X11/bsdinst.sh"
			else
				DEFAULT_INSTALL="/usr/bin/X11/bsdinst"
				DEFAULT_CFLAGS="$DEFAULT_CFLAGS -DUSE_TERMIOS"
			fi
			;;
		"13")	ARCH="SVR4"
			DEFAULT_CFLAGS="-O -DDONT_HAVE_FTIME -DUSE_TZNAME -DUSE_TERMIOS"
			DEFAULT_CC="/bin/cc"
			DEFAULT_INSTALL="/usr/ucb/install"
			RANLIB=true
			DEFAULT_TMP_DIR="/var/tmp"
			DEFAULT_MAIL_SPOOL="/var/mail"
			DEFAULT_BOARD_SPOOL="/var/board"
			LIB="-lsocket -lnsl"
			y_or_N "Is your system Solaris 2.X?"
			if [ $? -eq 1 ] ; then
				DEFAULT_CFLAGS="$DEFAULT_CFLAGS -DNEED_FFLUSH -DNEED_LNEXT"
			fi
			;;
		"14")	ARCH="SVR3"
			DEFAULT_CFLAGS="-O -DDONT_HAVE_FTIME -DUSE_TZNAME"
			DEFAULT_CC="/bin/cc"
			DEFAULT_INSTALL="/etc/install"
			RANLIB=true
			DEFAULT_TMP_DIR="/tmp"
			DEFAULT_MAIL_SPOOL="/usr/mail"
			DEFAULT_BOARD_SPOOL="/usr/board"
			;;
		"15")	ARCH="MSDOS"
			DEFAULT_CFLAGS="-O"
			;;
		"Q")	exit 0
			;;
		*)	echo "Illegal number."
			;;
	esac
done
echo ""

BIN_DIR=""
while [ "$BIN_DIR" = "" ]
do
	echo "Please input binary install directory."
	echo $n "Binary install directory(default:$DEFAULT_BIN_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_BIN_DIR
	fi
	if [ -d "$INPUT" ] ; then
		BIN_DIR="$INPUT"
	else
		echo "\"$INPUT\" not found."
	fi
done
echo ""

LIB_DIR=""
while [ "$LIB_DIR" = "" ]
do
	echo "Please input library install directory."
	echo $n "Library install directory(default:$DEFAULT_LIB_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_LIB_DIR
	fi
	if [ -d "$INPUT" ] ; then
		LIB_DIR="$INPUT"
	else
		echo "\"$INPUT\" not found."
	fi
done
echo ""
JNAMES_DB="$LIB_DIR/jnames"
INIT_FILE="$LIB_DIR/mnews_setup"
DOMAIN_FILE="$LIB_DIR/default-domain"
NNTP_SERVER_FILE="$LIB_DIR/default-server"
SMTP_SERVER_FILE="$LIB_DIR/default-smtpsv"
POP3_SERVER_FILE="$LIB_DIR/default-pop3sv"
YOUBIN_SERVER_FILE="$LIB_DIR/default-mailsv"

MAN_DIR=""
while [ "$MAN_DIR" = "" ]
do
	echo "Please input manual install directory."
	echo $n "Manual install directory(default:$DEFAULT_MAN_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_MAN_DIR
	fi
	if [ -d "$INPUT" ] ; then
		MAN_DIR="$INPUT"
	else
		echo "\"$INPUT\" not found."
	fi
done
echo ""

TMP_DIR=""
while [ "$TMP_DIR" = "" ]
do
	echo "Please input temporary directory."
	echo $n "Temporary directory(default:$DEFAULT_TMP_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_TMP_DIR
	fi
	if [ -d "$INPUT" ] ; then
		TMP_DIR="$INPUT"
	else
		echo "\"$INPUT\" not found."
	fi
done
echo ""

echo "NNTP means TCP/IP communication for reading news."
Y_or_n "Support NNTP mode(NNTP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DNNTP"
fi
echo ""

echo "NSPL means looking at news spool for reading news."
Y_or_n "Support NSPL mode(NSPL)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DNSPL"
fi
echo ""

echo "SMTP means TCP/IP communication for sending mail."
y_or_N "Support SMTP mode(SMTP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DSMTP"
fi
echo ""

echo "MTAP means using MTA(sendmail) pipe for sending mail."
Y_or_n "Support MTAP mode(MTAP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DMTAP"
fi
echo ""

echo "POP3 means TCP/IP communication for receiving mail."
y_or_N "Support POP3 mode(POP3)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DPOP3"
fi
echo ""

echo "MSPL means looking at mail spool for receiving mail."
Y_or_n "Support MSPL mode(MSPL)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DMSPL"
fi
echo ""

NEWS_SPOOL=""
while [ "$NEWS_SPOOL" = "" ]
do
	echo "Please input news spool directory."
	echo $n "News spool directory(default:$DEFAULT_NEWS_SPOOL):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_NEWS_SPOOL"
	fi
	if [ -d "$INPUT" ] ; then
		NEWS_SPOOL="$INPUT"
	else
		echo "\"$INPUT\" not found."
		Y_or_n "Ignore this error?"
		if [ $? -ne 0 ] ; then
			NEWS_SPOOL="$INPUT"
		fi
	fi
done
echo ""

NEWS_LIB=""
while [ "$NEWS_LIB" = "" ]
do
	echo "Please input news library directory."
	echo $n "News lib directory(default:$DEFAULT_NEWS_LIB):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_NEWS_LIB"
	fi
	if [ -d "$INPUT" ] ; then
		NEWS_LIB="$INPUT"
	else
		echo "\"$INPUT\" not found."
		Y_or_n "Ignore this error?"
		if [ $? -ne 0 ] ; then
			NEWS_LIB="$INPUT"
		fi
	fi
done
echo ""

MAIL_SPOOL=""
while [ "$MAIL_SPOOL" = "" ]
do
	echo "Please input mail spool directory."
	echo $n "Mail spool directory(default:$DEFAULT_MAIL_SPOOL):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_MAIL_SPOOL"
	fi
	if [ -d "$INPUT" ] ; then
		MAIL_SPOOL="$INPUT"
	else
		echo "\"$INPUT\" not found."
	fi
done
echo ""

POSTNEWS="$DEFAULT_POSTNEWS"
Y_or_n "Use posting news article(NEWSPOST)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DNEWSPOST"
	POSTNEWS=""
	while [ "$POSTNEWS" = "" ]
	do
		echo "Please input news post program and options."
		echo "If your host is not news server, you can ignore it.(type return to continue)"
		echo $n "News post program and options(default:$DEFAULT_POSTNEWS):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_POSTNEWS"
		fi
		if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
			POSTNEWS="$INPUT"
		else
			echo "Warning:\"`echo "$INPUT" | $AWK '{print $1}'`\" not found."
			POSTNEWS="$INPUT"
			Y_or_n "Ignore this error?"
			if [ $? -ne 1 ] ; then
				POSTNEWS=""
			fi
		fi
	done
fi
echo ""

INPUT=""
SENDMAIL="$DEFAULT_SENDMAIL"
Y_or_n "Use sending mail article(MAILSEND)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMAILSEND"
	SENDMAIL=""
	while [ "$SENDMAIL" = "" ]
	do
		echo "Please input mail send program and options."
		echo $n "Mail send program and options(default:$DEFAULT_SENDMAIL):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_SENDMAIL"
		fi
		if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
			SENDMAIL="$INPUT"
		else
			echo "\"`echo "$INPUT" | $AWK '{print $1}'`\" not found."
		fi
	done
fi
echo ""

Y_or_n "Use Japanese user name conversion(JNAMES)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DJNAMES"
	echo "Hash algorithm JNHASH module use more memory as normal module."
	Y_or_n "Use hash module(JNHASH)?"
	if [ $? -eq 1 ] ; then
		MOD_OPT="$MOD_OPT -DJNHASH"
	fi
	if [ ! -f "$JNAMES_DB" ] ; then
		echo "Warning:JNAMES database \"$JNAMES_DB\" not found."
	fi
fi
echo ""

MH_PATH="$DEFAULT_MH_PATH"
Y_or_n "Use MH mode(MH)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMH"
	MH_PATH=""
	while [ "$MH_PATH" = "" ]
	do
		echo "Please input MH command directory."
		echo $n "MH command directory(default:$DEFAULT_MH_PATH):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_MH_PATH"
		fi
		if [ -d "$INPUT" ] ; then
			MH_PATH="$INPUT"
		else
			echo "\"$INPUT\" not found."
		fi
	done
fi
echo ""

Y_or_n "Use UCB-mail mode(UCBMAIL)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DUCBMAIL"
fi
echo ""

Y_or_n "Use RMAIL mode(RMAIL)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DRMAIL"
fi
echo ""

BOARD_SPOOL="$DEFAULT_BOARD_SPOOL"
y_or_N "Use board mode(BOARD)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DBOARD"
	BOARD_SPOOL=""
	while [ "$BOARD_SPOOL" = "" ]
	do
		echo "Please input board spool directory."
		echo $n "Board spool directory(default:$DEFAULT_BOARD_SPOOL):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_BOARD_SPOOL"
		fi
		if [ -d "$INPUT" ] ; then
			BOARD_SPOOL="$INPUT"
		else
			echo "\"$INPUT\" not found."
			BOARD_SPOOL="$INPUT"
			Y_or_n "Ignore this error?"
			if [ $? -ne 1 ] ; then
				BOARD_SPOOL=""
			fi
		fi
	done
fi
echo ""

MPEG_COMMAND="$DEFAULT_MPEG_COMMAND"
JPEG_COMMAND="$DEFAULT_JPEG_COMMAND"
GIF_COMMAND="$DEFAULT_GIF_COMMAND"
AUDIO_COMMAND="$DEFAULT_AUDIO_COMMAND"
PS_COMMAND="DEFAULT_PS_COMMAND"
echo "If support MIME, performance will be affect."
Y_or_n "Use Japanese header,multipart selection and multimedia article(MIME)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMIME"
	echo "Please input MPEG player and options."
	echo $n "MPEG player and options(default:$DEFAULT_MPEG_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		MPEG_COMMAND="$INPUT"
	fi
	echo ""
	echo "Please input JPEG viewer and options."
	echo $n "JPEG viewer and options(default:$DEFAULT_JPEG_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		JPEG_COMMAND="$INPUT"
	fi
	echo ""
	echo "Please input GIF viewer and options."
	echo $n "GIF viewer and options(default:$DEFAULT_GIF_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		GIF_COMMAND="$INPUT"
	fi
	echo ""
	echo "Please input AUDIO player and options."
	echo $n "AUDIO player and options(default:$DEFAULT_AUDIO_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		AUDIO_COMMAND="$INPUT"
	fi
	echo ""
	echo "Please input Post Script viewer and options."
	echo $n "Post Script viewer and options(default:$DEFAULT_PS_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		PS_COMMAND="$INPUT"
	fi
	echo ""
fi
echo ""

Y_or_n "Use internal pager(PAGER)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DPAGER"
#	echo "High speed pager use more memory as normal pager."
#	y_or_N "Use high speed pager(HPAGER)?"
#	if [ $? -eq 1 ] ; then
#		MOD_OPT="$MOD_OPT -DHPAGER"
#	fi
fi
echo ""

PAGER=""
while [ "$PAGER" = "" ]
do
	echo "If internal pager included, external pager used with only -p"
	echo "option or pager_mode:off by setup-file."
	echo "Please input external pager program and options."
	echo $n "Pager program and options(default:$DEFAULT_PAGER):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_PAGER"
	fi
	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
		PAGER="$INPUT"
	else
		echo "Warning:\"`echo "$INPUT" | $AWK '{print $1}'`\" not found."
		PAGER="$INPUT"
		Y_or_n "Ignore this error?"
		if [ $? -ne 1 ] ; then
			PAGER=""
		fi
	fi
done
echo ""

Y_or_n "Use file carbon copy function(FCC)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DFCC"
fi
echo ""

y_or_N "Use 'Youbin' protocol instead of spool for checking mail(YOUBIN)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DYOUBIN"
fi
echo ""

echo "If support termcap or terminfo, performance will be affect."
y_or_N "Use termcap(TERMCAP)?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DTERMCAP"
	DEFAULT_TERMLIB="-ltermcap"
	echo $n "Input termcap library(default:$DEFAULT_TERMLIB):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_TERMLIB"
	fi
	LIB="$LIB $INPUT"
else
	y_or_N "Use terminfo(TERMINFO)?"
	if [ $? -eq 1 ] ; then
		CONS_OPT="$CONS_OPT -DTERMINFO"
		DEFAULT_TERMLIB="-ltermlib"
		echo $n "Input terminfo library(default:$DEFAULT_TERMLIB):$c" ; read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_TERMLIB"
		fi
		LIB="$LIB $INPUT"
	fi
fi
echo ""

INPUT=""
while [ "$INPUT" = "" ]
do
	echo "Please input configuration options."
	echo "-DCTRL_L            support ^L page break."
	echo "-DCOMPLETION        support file and folder completion."
	echo "-DREF_SORT          support reference sort."
	echo "-DXOVER             support NNTP XOVER for speedup."
	echo "-DCOLOR             support color screen."
	echo "-DADD_HOSTNAME      add hostname to your domain name."
	echo "-DADD_HOSTPATH      add hostname to path field."
	echo "-DADD_MESSAGE_ID    add Message-ID by mnews."
	echo "-DIGNORE_EDITOR     ignore editor exit status."
	echo "-DSELECT_EDITOR     ask editor when writting article."
	echo "-DUSE_NLINK         use link count for speedup MH/BOARD mode."
	echo "-DDISPLAY_CTRL      print control code as '^'+character."
	echo "-DRESOLVE           use resolver."
	echo "-DLOCAL_POST        always post by inews."
	echo "-DTRI               use TRI tree-search algorithm for speedup."
	echo "-DNNTP_AUTH         support INN authority function."
	echo "-DSEND_CONFIRM      confirm when news post or mail send."
	echo "-DSUPPORT_X0201     support JIS-X0201 kana character."
	echo "-DLARGE             support extend commands."
	echo $n "Options (default:$DEFAULT_CONF_OPT):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_CONF_OPT"
	fi
	CONF_OPT="$INPUT"
done
echo ""
echo "$CONF_OPT" | grep 'RESOLVE' > /dev/null
if [ $? -eq 0 ] ; then
	DOMAINNAME=""
	y_or_N "Does your standard C library contain name resolver?"
	if [ $? -eq 1 ] ; then
		LIB="$LIB -lresolv"
	fi
	echo ""
else
	INPUT=""
	while [ "$INPUT" = "" ]
	do
		echo "Input your domain(MX) name.(for example:msr.mei.co.jp)"
		if [ "$DOMAINNAME" != "" ] ; then
			echo $n "Domainname(default:$DOMAINNAME):$c"
		else
			echo $n "Domainname:$c"
			DOMAINNAME=""
		fi
		read INPUT
		if [ "$INPUT" = "" ]; then
			INPUT=$DOMAINNAME
		fi
		DOMAINNAME=$INPUT
	done
echo ""
fi

K_code "Please select default display code." 4
AWK_FLAG="{ print \$$? }"
DISPLAY_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "Please select default unknown code.(not serious)" 4
AWK_FLAG="{ print \$$? }"
UNKNOWN_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "Please select default save code." 1
AWK_FLAG="{ print \$$? }"
SAVE_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "Please select default edit code." 4
AWK_FLAG="{ print \$$? }"
EDIT_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

echo "If your site don't allow Japanese subject, please select \"No conversion\"."
K_code "Please select default subject input code." 1
AWK_FLAG="{ print \$$? }"
INPUT_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "Please select default fcc code." 1
AWK_FLAG="{ print \$$? }"
FCC_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "Please select default pipe code." 1
AWK_FLAG="{ print \$$? }"
PIPE_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "Please select default print out code." 1
AWK_FLAG="{ print \$$? }"
LPR_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

echo "Please input default print out command and options."
echo $n "Print out command and options(default:$DEFAULT_LPR_COMMAND):$c"
read INPUT
if [ "$INPUT" = "" ] ; then
	LPR_COMMAND="$DEFAULT_LPR_COMMAND"
else
	LPR_COMMAND="$INPUT"
fi
echo ""

echo "NNTP mode means use TCP/IP to accessing news instead of reading news spool."
echo "If your machine doesn't news server, please answer 'Y'."
Y_or_n "For default, use NNTP mode(NNTP_MODE)?"
NNTP_MODE=$?
echo ""

echo "SMTP mode means use TCP/IP to sending mail instead of using MTA pipe."
echo "If your machine doesn't have MTA(sendmail), please answer 'Y'."
y_or_N "For default, use SMTP mode(SMTP_MODE)?"
SMTP_MODE=$?
echo ""

echo "POP3 mode means use TCP/IP to receiving mail instead of reading mail spool."
echo "If your machine doesn't have mail spool, please answer 'Y'."
y_or_N "For default, use POP3 mode(POP3_MODE)?"
POP3_MODE=$?
echo ""

#y_or_N "For POP3 mode, use RPOP protocol(RPOP_MODE)?"
#RPOP_MODE=$?
#echo ""
RPOP_MODE=0

Y_or_n "For default, use Japanese message mode(JAPANESE_MODE)?"
JAPANESE_MODE=$?
echo ""

echo "If you support X-Nsubject field, you can send and receive Japanese subject"
echo "without changing news/mail system."
y_or_N "For default, support X-Nsubject field(X_NSUBJ_MODE)?"
X_NSUBJ_MODE=$?
echo ""

echo "In NSPL mode mnews uses inews for posting article."
echo "If inews add signature automatically, mnews don't add signature."
Y_or_n "News post program(inews) automatically add signature(INEWS_SIG_MODE)?"
INEWS_SIG_MODE=$?
echo ""

Y_or_n "For default, print date in Japanese time zone?"
JST_MODE=$?
echo ""

echo "In DNAS mode mnews don't delete bogus news-group automatically."
y_or_N "For default, support DNAS mode?"
DNAS_MODE=$?
echo ""

#echo "In thread mode mnews sorts articles by subject, but performance will be lost."
#Y_or_n "For default, display news article list in thread mode?"
#NEWS_THREAD_MODE=$?
#echo ""

#y_or_N "For default, display mail article list in thread mode?"
#MAIL_THREAD_MODE=$?
#echo ""

CC=""
while [ "$CC" = "" ]
do
	echo "Please input compiler program."
	echo $n "Compiler(default:$DEFAULT_CC):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_CC
	fi
	if [ -f "$INPUT" ] ; then
		CC="$INPUT"
	else
		echo "\"$INPUT\" not found."
	fi
done
echo ""

if [ "$DEFAULT_CPP" = "" ] ; then
	DEFAULT_CPP="$CC -E"
fi
CPP="$DEFAULT_CPP"
#CPP=""
#while [ "$CPP" = "" ]
#do
#	echo "Please input pre-processor program."
#	echo "If 'cc -E' make mistake, use 'cpp' instead of 'cc -E'."
#	echo $n "Pre-processor(default:$DEFAULT_CPP):$c" ; read INPUT
#	if [ "$INPUT" = "" ] ; then
#		INPUT=$DEFAULT_CPP
#	fi
#	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
#		CPP="$INPUT"
#	else
#		echo "\"$INPUT\" not found."
#	fi
#done
#echo ""

echo "Please input compiler optimize options."
echo $n "Options(default:$DEFAULT_CFLAGS):$c" ; read INPUT
if [ "$INPUT" = "" ] ; then
	CFLAGS=$DEFAULT_CFLAGS
else
	CFLAGS="$INPUT"
fi
echo ""

EDITOR=""
while [ "$EDITOR" = "" ]
do
	echo "Please input default editor and options."
	echo $n "Editor and options(default:$DEFAULT_EDITOR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_EDITOR"
	fi
	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
		EDITOR="$INPUT"
	else
		echo "\"`echo "$INPUT" | $AWK '{print $1}'`\" not found."
	fi
done
echo ""

#INSTALL=""
#while [ "$INSTALL" = "" ]
#do
#	echo "Please input install program."
#	echo $n "Installer(default:$DEFAULT_INSTALL):$c" ; read INPUT
#	if [ "$INPUT" = "" ] ; then
#		INPUT=$DEFAULT_INSTALL
#	fi
#	if [ -f "$INPUT" ] ; then
#		INSTALL="$INPUT"
#	else
#		echo "\"$INPUT\" not found."
#	fi
#done
#echo ""

rm -f default-domain

CONS_OPT=`echo $CONS_OPT | $SED -e "s/XXX //"`
echo "Target architecture         = $ARCH"
echo "Target network option       = $NET_OPT"
echo "Target construction option  = $CONS_OPT"
echo "Target configuration option = $CONF_OPT"
echo "High speed Module option    = $MOD_OPT"

if [ "$DOMAINNAME" = "" ] ; then
	echo "Get domain name by resolver."
else
	echo "Your domain name            = \"$DOMAINNAME\""
	echo $DOMAINNAME > default-domain
fi
Y_or_n "Is it correct?"
if [ $? -eq 0 ] ; then
	exit 0
fi
echo ""

echo "Making configuration files."

cp /dev/null "$SITE_DEPEND"
ORGAN_FILE="$NEWS_LIB/organization"
echo "/*"							>> $SITE_DEPEND
echo " *"							>> $SITE_DEPEND
echo " *  System      : Mini News Reader"			>> $SITE_DEPEND
echo " *  Sub system  : Site depend define"			>> $SITE_DEPEND
echo " *  File        : $SITE_DEPEND"				>> $SITE_DEPEND
echo " *  Version     : $VERSION"				>> $SITE_DEPEND
echo " *"							>> $SITE_DEPEND
echo " *  NOTE:"						>> $SITE_DEPEND
echo " *   This file made by config.sh program."		>> $SITE_DEPEND
echo " *   If you want to change this file, please execute"	>> $SITE_DEPEND
echo " *   config.sh program again."				>> $SITE_DEPEND
echo " *"                                         		>> $SITE_DEPEND
echo " */"                                         		>> $SITE_DEPEND
echo ""								>> $SITE_DEPEND
echo "/*"							>> $SITE_DEPEND
echo " * If you define DOMAIN_NAME, DOMAIN_NAME used in"	>> $SITE_DEPEND
echo " * executable file, and DOMAIN_FILE does not need."	>> $SITE_DEPEND
echo " */"							>> $SITE_DEPEND
echo "/*"                                          		>> $SITE_DEPEND
echo "#define	DOMAIN_NAME	\"$DOMAINNAME\""	 	>> $SITE_DEPEND
echo "*/"                                          		>> $SITE_DEPEND
echo "#define	DEFAULT_INIT_FILE	\"$INIT_FILE\""		>> $SITE_DEPEND
echo "#define	DOMAIN_FILE		\"$DOMAIN_FILE\""	>> $SITE_DEPEND
echo "#define	NNTP_SERVER_FILE	\"$NNTP_SERVER_FILE\""	>> $SITE_DEPEND
echo "#define	SMTP_SERVER_FILE	\"$SMTP_SERVER_FILE\""	>> $SITE_DEPEND
echo "#define	POP3_SERVER_FILE	\"$POP3_SERVER_FILE\""	>> $SITE_DEPEND
echo "#define	YOUBIN_SERVER_FILE	\"$YOUBIN_SERVER_FILE\"" >> $SITE_DEPEND
echo "#define	ORGAN_FILE		\"$ORGAN_FILE\"" 	>> $SITE_DEPEND
echo "" 							>> $SITE_DEPEND
echo "#define	EXT_PAGER	\"$PAGER\""			>> $SITE_DEPEND
echo "#define	PRINT_CODE	$DISPLAY_CODE"		 	>> $SITE_DEPEND
echo "#define	DEFAULT_CODE	$UNKNOWN_CODE"		 	>> $SITE_DEPEND
echo "#define	SAVE_CODE	$SAVE_CODE"			>> $SITE_DEPEND
echo "#define	EDIT_CODE	$EDIT_CODE"			>> $SITE_DEPEND
echo "#define	INPUT_CODE	$INPUT_CODE"			>> $SITE_DEPEND
echo "#define	FCC_CODE	$FCC_CODE"			>> $SITE_DEPEND
echo "#define	PIPE_CODE	$PIPE_CODE"			>> $SITE_DEPEND
echo "#define	LPR_CODE	$LPR_CODE"			>> $SITE_DEPEND
echo "#define	EDITOR_COMMAND	\"$EDITOR\""			>> $SITE_DEPEND
echo "#define	SEND_COMMAND	\"$SENDMAIL\""		 	>> $SITE_DEPEND
echo "#define	POST_COMMAND	\"$POSTNEWS\""		 	>> $SITE_DEPEND
echo "#define	LPR_COMMAND	\"$LPR_COMMAND\""	 	>> $SITE_DEPEND
echo "#define	MH_COMMAND_PATH	\"$MH_PATH\""			>> $SITE_DEPEND
echo "#define	JNAMES_DB	\"$JNAMES_DB\""			>> $SITE_DEPEND
echo "#define	NEWS_SPOOL	\"$NEWS_SPOOL\""	 	>> $SITE_DEPEND
echo "#define	NEWS_LIB	\"$NEWS_LIB\""		 	>> $SITE_DEPEND
echo "#define	MAIL_SPOOL	\"$MAIL_SPOOL\""	 	>> $SITE_DEPEND
echo "#define	BOARD_SPOOL	\"$BOARD_SPOOL\""		>> $SITE_DEPEND
echo "#define	TMP_DIR		\"$TMP_DIR\""			>> $SITE_DEPEND
echo ""								>> $SITE_DEPEND
echo "/*"							>> $SITE_DEPEND
echo " * MIME multimedia extension"				>> $SITE_DEPEND
echo " */"							>> $SITE_DEPEND
echo ""								>> $SITE_DEPEND
echo "#define	MPEG_COMMAND	\"$MPEG_COMMAND\""		>> $SITE_DEPEND
echo "#define	JPEG_COMMAND	\"$JPEG_COMMAND\""		>> $SITE_DEPEND
echo "#define	GIF_COMMAND	\"$GIF_COMMAND\""		>> $SITE_DEPEND
echo "#define	AUDIO_COMMAND	\"$AUDIO_COMMAND\""		>> $SITE_DEPEND
echo "#define	PS_COMMAND	\"$PS_COMMAND\""		>> $SITE_DEPEND
echo ""								>> $SITE_DEPEND
echo "#define	NNTP_MODE	$NNTP_MODE"			>> $SITE_DEPEND
echo "#define	SMTP_MODE	$SMTP_MODE"			>> $SITE_DEPEND
echo "#define	POP3_MODE	$POP3_MODE"			>> $SITE_DEPEND
echo "#define	RPOP_MODE	$RPOP_MODE"			>> $SITE_DEPEND
echo "#define	JAPANESE	$JAPANESE_MODE"			>> $SITE_DEPEND
echo "#define	X_NSUBJ_MODE	$X_NSUBJ_MODE"		 	>> $SITE_DEPEND
echo "#define	INEWS_SIG_MODE	$INEWS_SIG_MODE"	 	>> $SITE_DEPEND
echo "#define	JST_MODE	$JST_MODE"			>> $SITE_DEPEND
echo "#define	DNAS_MODE	$DNAS_MODE"			>> $SITE_DEPEND
#echo "#define	NEWS_THREAD_MODE	$NEWS_THREAD_MODE" 	>> $SITE_DEPEND
#echo "#define	MAIL_THREAD_MODE	$MAIL_THREAD_MODE" 	>> $SITE_DEPEND

rm -f $MAKEFILE temp.c

MAKESRC=""
if [ -f Makefile.obj ] ; then
	MAKESRC="Makefile.obj"
fi
if [ -f makefile.obj ] ; then
	MAKESRC="makefile.obj"
fi
if [ -f Makefile.src ] ; then
	MAKESRC="Makefile.src"
fi
if [ -f makefile.src ] ; then
	MAKESRC="makefile.src"
fi
if [ "$MAKESRC" != "" ] ; then
	echo "Using \"$MAKESRC\"."
	cp "$MAKESRC" temp.c
else
	echo "Makefile.src and Makefile.obj not found."
	exit 1
fi

$CPP -D$ARCH $CONS_OPT $MOD_OPT temp.c | $SED -e "s/_CONS_OPT_/$CONS_OPT/g" \
	-e "s/_CONF_OPT_/$CONF_OPT/g" -e "s/_MOD_OPT_/$MOD_OPT/g" \
	-e "s/_NET_OPT_/$NET_OPT/g" -e "s/__ARCH__/$ARCH/g" \
	-e "s@__CC__@$CC@g" \
	-e "s@_BIN_DIR_@$BIN_DIR@g" -e "s@_MAN_DIR_@$MAN_DIR@g" \
	-e "s@_LIB_DIR_@$LIB_DIR@g" -e "s@_CFLAGS_@$CFLAGS@g" \
	-e "s@__LIB__@$LIB@g" -e "s@_RANLIB_@$RANLIB@g" \
	-e "s@__WHEN__@$DATE@g" \
	-e "s@_INSTALL_@$INSTALL@g" > $MAKEFILE
\rm -f temp.c

echo "OK, Please make sure \"$MAKEFILE\" and \"$SITE_DEPEND\"."
echo "And then, to create the executable, type \"make\"."

exit 0
