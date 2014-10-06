#!/bin/sh
# Mini News Reader Ver 1.21
# ��ư����ե����졼����󥹥���ץ� for source/object release
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
DEFAULT_MH_PATH="/usr/local/bin/mh"
DEFAULT_MAIL_SPOOL="/var/spool/mail"
DEFAULT_BOARD_SPOOL="/var/spool/board"
DEFAULT_NEWS_SPOOL="/usr/spool/news"
DEFAULT_NEWS_LIB="/usr/lib/news"

SITE_DEPEND="site_dep.h"
MAKEFILE="Makefile"
K_STRING="ASCII_CODE JIS_CODE SJIS_CODE EUC_CODE UNKNOWN_CODE"

rm -f Y_or_n y_or_N K_code echotmp

echo "--- Mini News Reader ����ե����졼�����ġ��� ---"
echo "���줫����Ĥ��μ���������Ʋ�������"
echo "�㤨�С�'([y]/n)' �ǽ�������ñ�˥꥿�����á���Хǥե���ȤȤ��� 'y' �Ȥʤ�ޤ���"
echo ""

# for echo without line-feed
echo "���ԥ����ɤΰ�����Ĵ�٤Ƥ��ޤ�..."
(echo "12345\c" ; echo " ") > echotmp
if grep c echotmp >/dev/null 2>&1 ; then
	n='-n'
	c=''
else
	n=''
	c='\c'
fi
\rm -f echotmp
echo $n "CR(RETURN)�����򲡤���³���˿ʤߤޤ��������˥������뤬�������-->$c"
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
	echo "1:̵�Ѵ�"
	echo "2:JIS ������"
	echo "3:SJIS ������"
	echo "4:EUC ������"
	echo $n "Kanji code ? (�ǥե����:@2):$c"
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
	echo "����ѤΥ����ƥ�Υ������ƥ�����ϲ��Ǥ���?"
	echo "�ֹ������ǲ�������"
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

	echo $n "�������ƥ������ֹ�:$c" ; read INPUT
	case "$INPUT" in
		"1")	ARCH="SUNOS_41"
			DATE="date +'%y-%m/%d(%a) %H:%M:%S'"
			DEFAULT_CC="/usr/ucb/cc"
			DEFAULT_CFLAGS="-O -target `arch` -DUSE_TERMIOS"
			DEFAULT_TMP_DIR="/var/tmp"
			Y_or_n "����ѤΥ����ƥ�� SUN-OS 4.1.X �Ǥ���?"
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
			Y_or_n "����ѤΥ����ƥ�� Next Step 3.X �Ǥ���?"
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
			Y_or_n "����ѤΥ����ƥ�� UNI-OS 3.X �Ǥ���?"
			if [ $? -eq 1 ] ; then
				DEFAULT_CFLAGS="$DEFAULT_CFLAGS -DSTRCASECMP"
			else
				Y_or_n "����ѤΥ����ƥ�� DG/UX �Ǥ���?"
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
			Y_or_n "����ѤΥ����ƥ�� IRIX 5.X �ʾ�Ǥ���?"
			if [ $? -eq 0 ] ; then
				Y_or_n "NIS ����Ѥ��Ƥ��ޤ���?"
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
		*)	echo "�ֹ椬�����Ǥ���"
			;;
	esac
done
echo ""

BIN_DIR=""
while [ "$BIN_DIR" = "" ]
do
	echo "�¹ԥե����륤�󥹥ȡ���ǥ��쥯�ȥ�̾�����Ϥ��Ʋ�������"
	echo $n "�¹ԥե����륤�󥹥ȡ���ǥ��쥯�ȥ�(�ǥե����:$DEFAULT_BIN_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_BIN_DIR
	fi
	if [ -d "$INPUT" ] ; then
		BIN_DIR="$INPUT"
	else
		echo "\"$INPUT\" �����Ĥ���ޤ���"
	fi
done
echo ""

LIB_DIR=""
while [ "$LIB_DIR" = "" ]
do
	echo "�饤�֥�ꥤ�󥹥ȡ���ǥ��쥯�ȥ�̾�����Ϥ��Ʋ�������"
	echo $n "�饤�֥�ꥤ�󥹥ȡ���ǥ��쥯�ȥ�(�ǥե����:$DEFAULT_LIB_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_LIB_DIR
	fi
	if [ -d "$INPUT" ] ; then
		LIB_DIR="$INPUT"
	else
		echo "\"$INPUT\" �����Ĥ���ޤ���"
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
	echo "�ޥ˥奢�륤�󥹥ȡ���ǥ��쥯�ȥ�̾�����Ϥ��Ʋ�������"
	echo $n "�ޥ˥奢�륤�󥹥ȡ���ǥ��쥯�ȥ�(�ǥե����:$DEFAULT_MAN_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_MAN_DIR
	fi
	if [ -d "$INPUT" ] ; then
		MAN_DIR="$INPUT"
	else
		echo "\"$INPUT\" �����Ĥ���ޤ���"
	fi
done
echo ""

TMP_DIR=""
while [ "$TMP_DIR" = "" ]
do
	echo "��ȥǥ��쥯�ȥ�̾�����Ϥ��Ʋ�������"
	echo $n "��ȥǥ��쥯�ȥ�(�ǥե����:$DEFAULT_TMP_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_TMP_DIR
	fi
	if [ -d "$INPUT" ] ; then
		TMP_DIR="$INPUT"
	else
		echo "\"$INPUT\" �����Ĥ���ޤ���"
	fi
done
echo ""

echo "NNTP �ϥ˥塼���򻲾Ȥ���Τ� TCP/IP �̿�����Ѥ��뤳�Ȥ��̣���ޤ���"
Y_or_n "NNTP �⡼�ɤ򥵥ݡ��Ȥ��ޤ���(NNTP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DNNTP"
fi
echo ""

echo "NSPL �ϥ˥塼���򻲾Ȥ���Τ˥˥塼�����ס���򻲾Ȥ��뤳�Ȥ��̣���ޤ���"
Y_or_n "NSPL �⡼�ɤ򥵥ݡ��Ȥ��ޤ���(NSPL)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DNSPL"
fi
echo ""

echo "SMTP �ϥ᡼�����������Τ� TCP/IP �̿�����Ѥ��뤳�Ȥ��̣���ޤ���"
y_or_N "SMTP �⡼�ɤ򥵥ݡ��Ȥ��ޤ���(SMTP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DSMTP"
fi
echo ""

echo "MTAP �ϥ᡼�����������Τ� MTA(sendmail) ��ѥ��׵�ư���뤳�Ȥ��̣���ޤ���"
Y_or_n "MTAP �⡼�ɤ򥵥ݡ��Ȥ��ޤ���(MTAP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DMTAP"
fi
echo ""

echo "POP3 �ϥ᡼����������Τ� TCP/IP �̿�����Ѥ��뤳�Ȥ��̣���ޤ���"
y_or_N "POP3 �⡼�ɤ򥵥ݡ��Ȥ��ޤ���(POP3)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DPOP3"
fi
echo ""

echo "MSPL �ϥ᡼����������Τ˥᡼�륹�ס���򻲾Ȥ��뤳�Ȥ��̣���ޤ���"
Y_or_n "MSPL �⡼�ɤ򥵥ݡ��Ȥ��ޤ���(MSPL)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DMSPL"
fi
echo ""

NEWS_SPOOL=""
while [ "$NEWS_SPOOL" = "" ]
do
	echo "�˥塼�����ס���ǥ��쥯�ȥ�̾�����Ϥ��Ʋ�������"
	echo $n "�˥塼�����ס���ǥ��쥯�ȥ�(�ǥե����:$DEFAULT_NEWS_SPOOL):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_NEWS_SPOOL"
	fi
	if [ -d "$INPUT" ] ; then
		NEWS_SPOOL="$INPUT"
	else
		echo "\"$INPUT\" �����Ĥ���ޤ���"
		Y_or_n "���Υ��顼��̵�뤷�ޤ���?"
		if [ $? -ne 0 ] ; then
			NEWS_SPOOL="$INPUT"
		fi
	fi
done
echo ""

NEWS_LIB=""
while [ "$NEWS_LIB" = "" ]
do
	echo "�˥塼���饤�֥��ǥ��쥯�ȥ�̾�����Ϥ��Ʋ�������"
	echo $n "�˥塼���饤�֥��ǥ��쥯�ȥ�(�ǥե����:$DEFAULT_NEWS_LIB):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_NEWS_LIB"
	fi
	if [ -d "$INPUT" ] ; then
		NEWS_LIB="$INPUT"
	else
		echo "\"$INPUT\" �����Ĥ���ޤ���"
		Y_or_n "���Υ��顼��̵�뤷�ޤ���?"
		if [ $? -ne 0 ] ; then
			NEWS_LIB="$INPUT"
		fi
	fi
done
echo ""

MAIL_SPOOL=""
while [ "$MAIL_SPOOL" = "" ]
do
	echo "�᡼�륹�ס���ǥ��쥯�ȥ�̾�����Ϥ��Ʋ�������"
	echo $n "�᡼�륹�ס���ǥ��쥯�ȥ�(�ǥե����:$DEFAULT_MAIL_SPOOL):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_MAIL_SPOOL"
	fi
	if [ -d "$INPUT" ] ; then
		MAIL_SPOOL="$INPUT"
	else
		echo "\"$INPUT\" �����Ĥ���ޤ���"
	fi
done
echo ""

POSTNEWS="$DEFAULT_POSTNEWS"
Y_or_n "�˥塼������Ƥ��뵡ǽ(NEWSPOST)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DNEWSPOST"
	POSTNEWS=""
	while [ "$POSTNEWS" = "" ]
	do
		echo "�˥塼������Ƥ���ץ����ȥ��ץ��������Ϥ��Ʋ�������"
		echo "�⤷���˥塼�������о�ǻ��Ѥ��ʤ��ʤ�̵�뤷�ƥ꥿����򲡤��Ʋ�������"
		echo $n "�˥塼����ƥץ����ȥ��ץ����(�ǥե����:$DEFAULT_POSTNEWS):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_POSTNEWS"
		fi
		if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
			POSTNEWS="$INPUT"
		else
			echo "�ٹ�:\"`echo "$INPUT" | $AWK '{print $1}'`\" �����Ĥ���ޤ���"
			POSTNEWS="$INPUT"
			Y_or_n "���Υ��顼��̵�뤷�ޤ���?"
			if [ $? -ne 1 ] ; then
				POSTNEWS=""
			fi
		fi
	done
fi
echo ""

INPUT=""
SENDMAIL="$DEFAULT_SENDMAIL"
Y_or_n "�᡼����������뵡ǽ(MAILSEND)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMAILSEND"
	SENDMAIL=""
	while [ "$SENDMAIL" = "" ]
	do
		echo "�᡼�����������ץ����ȥ��ץ��������Ϥ��Ʋ�������"
		echo $n "�᡼�������ץ����ȥ��ץ����(�ǥե����:$DEFAULT_SENDMAIL):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_SENDMAIL"
		fi
		if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
			SENDMAIL="$INPUT"
		else
			echo "\"`echo "$INPUT" | $AWK '{print $1}'`\" �����Ĥ���ޤ���"
		fi
	done
fi
echo ""

Y_or_n "�ǡ����١����ˤ�����ܸ��Ѵ���ǽ(JNAMES)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DJNAMES"
	echo "�ϥå��奢�르�ꥺ�� JNHASH �⥸�塼��Ϲ�®�Ǥ������̾�⥸�塼����"
	echo "���̤Υ������񤷤ޤ���"
	Y_or_n "�ϥå��奢�르�ꥺ��⥸�塼��(JNHASH)����Ѥ��ޤ���?"
	if [ $? -eq 1 ] ; then
		MOD_OPT="$MOD_OPT -DJNHASH"
	fi
	if [ ! -f "$JNAMES_DB" ] ; then
		echo "�ٹ�:JNAMES �ǡ����١��� \"$JNAMES_DB\" �����Ĥ���ޤ���"
	fi
fi
echo ""

MH_PATH="$DEFAULT_MH_PATH"
Y_or_n "MH �⡼��(MH)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMH"
	MH_PATH=""
	while [ "$MH_PATH" = "" ]
	do
		echo "MH ���ޥ�ɥǥ��쥯�ȥ�����Ϥ��Ʋ�������"
		echo $n "MH ���ޥ�ɥǥ��쥯�ȥ�(�ǥե����:$DEFAULT_MH_PATH):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_MH_PATH"
		fi
		if [ -d "$INPUT" ] ; then
			MH_PATH="$INPUT"
		else
			echo "\"$INPUT\" �����Ĥ���ޤ���"
		fi
	done
fi
echo ""

Y_or_n "UCB-mail �⡼��(UCBMAIL)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DUCBMAIL"
fi
echo ""

Y_or_n "RMAIL �⡼��(RMAIL)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DRMAIL"
fi
echo ""

BOARD_SPOOL="$DEFAULT_BOARD_SPOOL"
y_or_N "BOARD (�ŻҷǼ���)�⡼��(BOARD)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DBOARD"
	BOARD_SPOOL=""
	while [ "$BOARD_SPOOL" = "" ]
	do
		echo "BOARD ���ס���ǥ��쥯�ȥ�����Ϥ��Ʋ�������"
		echo $n "�ܡ��ɥ��ס���ǥ��쥯�ȥ�(�ǥե����:$DEFAULT_BOARD_SPOOL):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_BOARD_SPOOL"
		fi
		if [ -d "$INPUT" ] ; then
			BOARD_SPOOL="$INPUT"
		else
			echo "\"$INPUT\" �����Ĥ���ޤ���"
			BOARD_SPOOL="$INPUT"
			Y_or_n "���Υ��顼��̵�뤷�ޤ���?"
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
echo "�⤷ MIME �򥵥ݡ��Ȥ���м㴳®�٤��㲼���ޤ���"
Y_or_n "���ܸ�إå�,�ޥ���ѡ�������,�ޥ����ǥ�������(MIME)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMIME"
	echo "MPEG �ץ쥤�䡼�ȥ��ץ��������Ϥ��Ʋ�������"
	echo $n "MPEG �ץ쥤�䡼�ȥ��ץ����(�ǥե����:$DEFAULT_MPEG_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		MPEG_COMMAND="$INPUT"
	fi
	echo ""
	echo "JPEG �ӥ塼�����ȥ��ץ��������Ϥ��Ʋ�������"
	echo $n "JPEG �ӥ塼�����ȥ��ץ����(�ǥե����:$DEFAULT_JPEG_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		JPEG_COMMAND="$INPUT"
	fi
	echo ""
	echo "GIF �ӥ塼�����ȥ��ץ��������Ϥ��Ʋ�������"
	echo $n "GIF �ӥ塼�����ȥ��ץ����(�ǥե����:$DEFAULT_GIF_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		GIF_COMMAND="$INPUT"
	fi
	echo ""
	echo "�����ǥ����ץ쥤�䡼�ȥ��ץ��������Ϥ��Ʋ�������"
	echo $n "�����ǥ����ץ쥤�䡼�ȥ��ץ����(�ǥե����:$DEFAULT_AUDIO_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		AUDIO_COMMAND="$INPUT"
	fi
	echo ""
	echo "Post Script �ӥ塼�����ȥ��ץ��������Ϥ��Ʋ�������"
	echo $n "Post Script �ӥ塼�����ȥ��ץ����(�ǥե����:$DEFAULT_PS_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		PS_COMMAND="$INPUT"
	fi
	echo ""
fi
echo ""

Y_or_n "��¢�ڡ����㡼(PAGER)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DPAGER"
#	echo "��®�ڡ����㡼���̾�ڡ����㡼������̤Υ������񤷤ޤ���"
#	y_or_N "��®�ڡ����㡼(HPAGER)����Ѥ��ޤ���?"
#	if [ $? -eq 1 ] ; then
#		MOD_OPT="$MOD_OPT -DHPAGER"
#	fi
fi
echo ""

PAGER=""
while [ "$PAGER" = "" ]
do
	echo "��¢�ڡ����㡼���Ȥ߹�������� -p ���ץ����⤷����"
	echo "����ե�����ǻ��ꤷ�����Τ߳����ڡ����㡼�����Ѥ���ޤ���"
	echo "�����ڡ����㡼�ץ����ȥ��ץ��������Ϥ��Ʋ�������"
	echo $n "�ڡ����㡼�ץ����ȥ��ץ����(�ǥե����:$DEFAULT_PAGER):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_PAGER"
	fi
	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
		PAGER="$INPUT"
	else
		echo "�ٹ�:\"`echo "$INPUT" | $AWK '{print $1}'`\" �����Ĥ���ޤ���"
		PAGER="$INPUT"
		Y_or_n "���Υ��顼��̵�뤷�ޤ���?"
		if [ $? -ne 1 ] ; then
			PAGER=""
		fi
	fi
done
echo ""

Y_or_n "File carbon copy (FCC)��ǽ����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DFCC"
fi
echo ""

y_or_N "����᡼��Υ����å���͹��(YOUBIN)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DYOUBIN"
fi
echo ""

echo "�⤷ termcap �ޤ��� terminfo �򥵥ݡ��Ȥ���м㴳®�٤��㲼���ޤ���"
y_or_N "termcap (TERMCAP)����Ѥ��ޤ���?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DTERMCAP"
	DEFAULT_TERMLIB="-ltermcap"
	echo $n "termcap �饤�֥������Ϥ��Ʋ�������(�ǥե����:$DEFAULT_TERMLIB):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_TERMLIB"
	fi
	LIB="$LIB $INPUT"
else
	y_or_N "terminfo (TERMINFO)����Ѥ��ޤ���?"
	if [ $? -eq 1 ] ; then
		CONS_OPT="$CONS_OPT -DTERMINFO"
		DEFAULT_TERMLIB="-ltermlib"
		echo $n "terminfo �饤�֥������Ϥ��Ʋ�������(�ǥե����:$DEFAULT_TERMLIB):$c" ; read INPUT
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
	echo "����ե����졼����󥪥ץ��������Ϥ��Ʋ�������"
	echo "-DCTRL_L            ^L �ˤ����ڡ����򥵥ݡ��Ȥ��롣"
	echo "-DCOMPLETION        �ե�����ȥե����̾�䴰�򥵥ݡ��Ȥ��롣"
	echo "-DREF_SORT          ��ե���󥹥����Ȥ򥵥ݡ��Ȥ��롣"
	echo "-DXOVER             NNTP �� XOVER ����Ѥ��ƹ�®�����롣"
	echo "-DCOLOR             ���顼����ɽ���򥵥ݡ��Ȥ��롣"
	echo "-DADD_HOSTNAME      �ۥ���̾��ɥᥤ��̾���ɲä��롣"
	echo "-DADD_HOSTPATH      �ۥ���̾��ѥ��ե�����ɤ��ɲä��롣"
	echo "-DADD_MESSAGE_ID    Message-ID �� mnews ���ɲä��롣"
	echo "-DIGNORE_EDITOR     ���ǥ�����λ�����ɤ�̵�뤹�롣"
	echo "-DSELECT_EDITOR     ������񤯻��˥��ǥ��������򤹤롣"
	echo "-DUSE_NLINK         MH/BOARD �⡼�ɤǥ�󥯿�����Ѥ��ƹ�®�����롣"
	echo "-DDISPLAY_CTRL      ̵���ʥ���ȥ��륳���ɤ� '^'+ʸ���Ȥ��ƲĻ벽���롣"
	echo "-DRESOLVE           �쥾��Ф���Ѥ��롣"
	echo "-DLOCAL_POST        ��� inews ����Ѥ�����Ƥ��롣"
	echo "-DTRI               TRI õ���ڥ��르�ꥺ�����Ѥ��ƹ�®�����롣"
	echo "-DNNTP_AUTH         INN ��ǧ�ڵ�ǽ���б����롣"
	echo "-DSEND_CONFIRM      �˥塼�����/�᡼���������˳�ǧ����롣"
	echo "-DSUPPORT_X0201     JIS-X0201 ����ʸ���򥵥ݡ��Ȥ��롣"
	echo "-DLARGE             ��ĥ���ޥ�ɤ򥵥ݡ��Ȥ��롣"
	echo $n "���ץ����(�ǥե����:$DEFAULT_CONF_OPT):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_CONF_OPT"
	fi
	CONF_OPT="$INPUT"
done
echo ""
echo "$CONF_OPT" | grep 'RESOLVE' > /dev/null
if [ $? -eq 0 ] ; then
	DOMAINNAME=""
	y_or_N "����Ѥ� C ɸ��饤�֥��� name resolver ��ޤ�Ǥ��ޤ���?"
	if [ $? -eq 1 ] ; then
		LIB="$LIB -lresolv"
	fi
	echo ""
else
	INPUT=""
	while [ "$INPUT" = "" ]
	do
		echo "�ɥᥤ��(MX)̾�����Ϥ��Ʋ�������(��:msr.mei.co.jp)"
		if [ "$DOMAINNAME" != "" ] ; then
			echo $n "�ɥᥤ��̾(�ǥե����:$DOMAINNAME):$c"
		else
			echo $n "�ɥᥤ��̾:$c"
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

K_code "�ǥե���Ȥ�ɽ�������ɤ�����ǲ�������" 4
AWK_FLAG="{ print \$$? }"
DISPLAY_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "�ǥե���Ȥ�Ƚ����ǽ�������ɤ�����ǲ�������(���פǤϤʤ��Ǥ�)" 4
AWK_FLAG="{ print \$$? }"
UNKNOWN_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "�ǥե���ȤΥ����֥����ɤ�����ǲ�������" 1
AWK_FLAG="{ print \$$? }"
SAVE_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "�ǥե���ȤΥ��ǥ��åȥ����ɤ�����ǲ�������" 4
AWK_FLAG="{ print \$$? }"
EDIT_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

echo "�⤷���ܸ쥵�֥������Ȥ���Ƥ��Ƥ��ʤ������Ȥʤ�\"̵�Ѵ�\"������ǲ�������"
K_code "�ǥե���ȤΥ��֥����������ϥ����ɤ�����ǲ�������" 1
AWK_FLAG="{ print \$$? }"
INPUT_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "�ǥե���Ȥ� FCC �����ɤ�����ǲ�������" 1
AWK_FLAG="{ print \$$? }"
FCC_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "�ǥե���ȤΥѥ��ץ����ɤ�����ǲ�������" 1
AWK_FLAG="{ print \$$? }"
PIPE_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "�ǥե���Ȥΰ��������ɤ�����ǲ�������" 1
AWK_FLAG="{ print \$$? }"
LPR_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

LPR_COMMAND=""
echo "�ǥե���Ȥΰ������ޥ�ɤȥ��ץ��������Ϥ��Ʋ�������"
echo $n "�������ޥ�ɤȥ��ץ����(�ǥե����:$DEFAULT_LPR_COMMAND):$c"
read INPUT
if [ "$INPUT" = "" ] ; then
	LPR_COMMAND="$DEFAULT_LPR_COMMAND"
else
	LPR_COMMAND="$INPUT"
fi
echo ""

echo "NNTP �⡼�ɤϥ˥塼�������������˥˥塼�����ס���򻲾Ȥ�������� TCP/IP �̿�����Ѥ��뤳�Ȥ��̣���Ƥ��ޤ���"
echo "�⤷���ʤ��Υޥ��󤬥˥塼�������ФǤʤ��ʤ� 'Y' �������Ʋ�������"
Y_or_n "�ǥե���ȤǤϡ�NNTP �⡼��(NNTP_MODE)����Ѥ��ޤ���?"
NNTP_MODE=$?
echo ""

echo "SMTP �⡼�ɤϥ᡼���������� MTA ��ѥ��׵�ư��������� TCP/IP �̿�����Ѥ��뤳�Ȥ��̣���Ƥ��ޤ���"
echo "�⤷���ʤ��Υޥ��� MTA(sendmail) ����äƤ��ʤ��ʤ� 'Y' �������Ʋ�������"
y_or_N "�ǥե���ȤǤϡ�SMTP �⡼��(SMTP_MODE)����Ѥ��ޤ���?"
SMTP_MODE=$?
echo ""

echo "POP3 �⡼�ɤϥ᡼��������˥᡼�륹�ס���򻲾Ȥ�������� TCP/IP �̿�����Ѥ��뤳�Ȥ��̣���Ƥ��ޤ���"
echo "�⤷���ʤ��Υޥ��󤬥᡼�륹�ס������äƤ��ʤ��ʤ� 'Y' �������Ʋ�������"
y_or_N "�ǥե���ȤǤϡ�POP3 �⡼��(POP3_MODE)����Ѥ��ޤ���?"
POP3_MODE=$?
echo ""

#y_or_N "POP3 �⡼�ɻ��� RPOP (RPOP_MODE)����Ѥ��ޤ���?"
#RPOP_MODE=$?
#echo ""
RPOP_MODE=0

Y_or_n "�ǥե���ȤǤϡ����ܸ��å�����(JAPANESE_MODE)�⡼�ɤˤ��ޤ���?"
JAPANESE_MODE=$?
echo ""

echo "�⤷ X-Nsubject �ե�������б��ˤ�������ܸ쥵�֥������Ȥ��˥塼��/�᡼�륷���ƥ���ѹ��ʤ����������Ǥ��ޤ���"
y_or_N "�ǥե���ȤǤϡ�X-Nsubject �ե������(X_NSUBJ_MODE)�б��ˤ��ޤ���?"
X_NSUBJ_MODE=$?
echo ""

echo "NSPL �⡼�ɤǤ� mnews �� inews ����Ѥ��Ƶ�������Ƥ�Ԥʤ��ޤ���"
echo "�⤷ inews ����ưŪ�� signature ���ɲä���ʤ� mnews ¦�� signature ��Ĥ��ƤϤ����ޤ���"
Y_or_n "�˥塼����ƥץ����(inews)�ϡ���ưŪ�� signature ���ɲ�(INEWS_SIG_MODE)���ޤ���?"
INEWS_SIG_MODE=$?
echo ""

Y_or_n "�ǥե���ȤǤϡ����դ����ܻ��֤�ɽ�����ޤ���?"
JST_MODE=$?
echo ""

echo "DNAS �⡼�ɤǤ� mnews ��̵���ʥ˥塼�����롼�פ�ư������ޤ���"
y_or_N "�ǥե���ȤǤϡ�DNAS �⡼�ɤ򥵥ݡ��Ȥ��ޤ���?"
DNAS_MODE=$?
echo ""

#echo "����åɥ⡼�ɤǤ� mnews �ϵ����򥵥֥������Ƚ�˥����Ȥ��ޤ�����®�٤��㴳����ޤ���"
#Y_or_n "�ǥե���ȤǤϡ�����åɥ⡼�ɤǥ˥塼������������ɽ�����ޤ���?"
#NEWS_THREAD_MODE=$?
#echo ""

#y_or_N "�ǥե���ȤǤϡ�����åɥ⡼�ɤǥ᡼�뵭��������ɽ�����ޤ���?"
#MAIL_THREAD_MODE=$?
#echo ""

CC=""
while [ "$CC" = "" ]
do
	echo "����ѥ���ץ��������Ϥ��Ʋ�������"
	echo $n "����ѥ���(�ǥե����:$DEFAULT_CC):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_CC
	fi
	if [ -f "$INPUT" ] ; then
		CC="$INPUT"
	else
		echo "\"$INPUT\" �����Ĥ���ޤ���"
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
#	echo "�ץꡦ�ץ��å��ץ����̾�����Ϥ��Ʋ�������"
#	echo "�⤷ 'cc -E' �����Ԥ���ʤ� 'cc -E' ������� 'cpp' ���Ʋ�������"
#	echo $n "�ץꡦ�ץ��å�(�ǥե����:$DEFAULT_CPP):$c" ; read INPUT
#	if [ "$INPUT" = "" ] ; then
#		INPUT=$DEFAULT_CPP
#	fi
#	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
#		CPP="$INPUT"
#	else
#		echo "\"$INPUT\" �����Ĥ���ޤ���"
#	fi
#done
#echo ""

echo "����ѥ����(��Ŭ��)���ץ��������Ϥ��Ʋ�������"
echo $n "���ץ����(�ǥե����:$DEFAULT_CFLAGS):$c" ; read INPUT
if [ "$INPUT" = "" ] ; then
	CFLAGS=$DEFAULT_CFLAGS
else
	CFLAGS="$INPUT"
fi
echo ""

EDITOR=""
while [ "$EDITOR" = "" ]
do
	echo "�ǥե���Ȥǻ��Ѥ��륨�ǥ����ȥ��ץ��������Ϥ��Ʋ�������"
	echo $n "���ǥ����ȥ��ץ����(�ǥե����:$DEFAULT_EDITOR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_EDITOR"
	fi
	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
		EDITOR="$INPUT"
	else
		echo "\"`echo "$INPUT" | $AWK '{print $1}'`\" �����Ĥ���ޤ���"
	fi
done
echo ""

#INSTALL=""
#while [ "$INSTALL" = "" ]
#do
#	echo "���󥹥ȡ���ץ��������Ϥ��Ʋ�������"
#	echo $n "���󥹥ȡ���(�ǥե����:$DEFAULT_INSTALL):$c" ; read INPUT
#	if [ "$INPUT" = "" ] ; then
#		INPUT=$DEFAULT_INSTALL
#	fi
#	if [ -f "$INPUT" ] ; then
#		INSTALL="$INPUT"
#	else
#		echo "\"$INPUT\" �����Ĥ���ޤ���"
#	fi
#done
#echo ""

rm -f default-domain

CONS_OPT=`echo $CONS_OPT | $SED -e "s/XXX //"`
echo "�������åȥ������ƥ�����         = $ARCH"
echo "�������åȥͥåȥ�����ץ���� = $NET_OPT"
echo "�������åȹ������ץ����         = $CONS_OPT"
echo "�������å����ꥪ�ץ����         = $CONF_OPT"
echo "��®�⥸�塼�륪�ץ����         = $MOD_OPT"

if [ "$DOMAINNAME" = "" ] ; then
	echo "�ɥᥤ��̾��꥾��Ф��������ޤ���"
else
	echo "�ɥᥤ��̾                       = \"$DOMAINNAME\""
	echo $DOMAINNAME > default-domain
fi
Y_or_n "�ʾ�Ǥ��äƤ��ޤ���?"
if [ $? -eq 0 ] ; then
	exit 0
fi
echo ""

echo "����ե����졼�����ե������������Ƥ��ޤ���"

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
	echo "\"$MAKESRC\" ��Ȥ��ޤ���"
	cp "$MAKESRC" temp.c
else
	echo "Makefile.src �� Makefile.obj �����Ĥ���ޤ���"
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

echo "����Ͻ�λ���ޤ�����\"$MAKEFILE\" �� \"$SITE_DEPEND\" ��Τ���Ʋ�������"
echo "�Τ��Ὢ�ä���¹ԥե������������뤿��� \"make\" �ȥ����פ��Ʋ�������"

exit 0
