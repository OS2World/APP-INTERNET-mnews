#!/bin/sh
# Mini News Reader Ver 1.21
# 自動コンフィグレーションスクリプト for source/object release
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

echo "--- Mini News Reader コンフィグレーションツール ---"
echo "これから幾つかの質問に答えて下さい。"
echo "例えば、'([y]/n)' で終る質問は単にリターンを叩けばデフォルトとして 'y' となります。"
echo ""

# for echo without line-feed
echo "改行コードの扱いを調べています..."
(echo "12345\c" ; echo " ") > echotmp
if grep c echotmp >/dev/null 2>&1 ; then
	n='-n'
	c=''
else
	n=''
	c='\c'
fi
\rm -f echotmp
echo $n "CR(RETURN)キーを押せば続きに進みます。ここにカーソルが見えれば-->$c"
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
	echo "1:無変換"
	echo "2:JIS コード"
	echo "3:SJIS コード"
	echo "4:EUC コード"
	echo $n "Kanji code ? (デフォルト:@2):$c"
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
	echo "御使用のシステムのアーキテクチャは何ですか?"
	echo "番号で選んで下さい。"
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

	echo $n "アーキテクチャ番号:$c" ; read INPUT
	case "$INPUT" in
		"1")	ARCH="SUNOS_41"
			DATE="date +'%y-%m/%d(%a) %H:%M:%S'"
			DEFAULT_CC="/usr/ucb/cc"
			DEFAULT_CFLAGS="-O -target `arch` -DUSE_TERMIOS"
			DEFAULT_TMP_DIR="/var/tmp"
			Y_or_n "御使用のシステムは SUN-OS 4.1.X ですか?"
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
			Y_or_n "御使用のシステムは Next Step 3.X ですか?"
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
			Y_or_n "御使用のシステムは UNI-OS 3.X ですか?"
			if [ $? -eq 1 ] ; then
				DEFAULT_CFLAGS="$DEFAULT_CFLAGS -DSTRCASECMP"
			else
				Y_or_n "御使用のシステムは DG/UX ですか?"
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
			Y_or_n "御使用のシステムは IRIX 5.X 以上ですか?"
			if [ $? -eq 0 ] ; then
				Y_or_n "NIS を使用していますか?"
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
		*)	echo "番号が不正です。"
			;;
	esac
done
echo ""

BIN_DIR=""
while [ "$BIN_DIR" = "" ]
do
	echo "実行ファイルインストールディレクトリ名を入力して下さい。"
	echo $n "実行ファイルインストールディレクトリ(デフォルト:$DEFAULT_BIN_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_BIN_DIR
	fi
	if [ -d "$INPUT" ] ; then
		BIN_DIR="$INPUT"
	else
		echo "\"$INPUT\" が見つかりません。"
	fi
done
echo ""

LIB_DIR=""
while [ "$LIB_DIR" = "" ]
do
	echo "ライブラリインストールディレクトリ名を入力して下さい。"
	echo $n "ライブラリインストールディレクトリ(デフォルト:$DEFAULT_LIB_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_LIB_DIR
	fi
	if [ -d "$INPUT" ] ; then
		LIB_DIR="$INPUT"
	else
		echo "\"$INPUT\" が見つかりません。"
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
	echo "マニュアルインストールディレクトリ名を入力して下さい。"
	echo $n "マニュアルインストールディレクトリ(デフォルト:$DEFAULT_MAN_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_MAN_DIR
	fi
	if [ -d "$INPUT" ] ; then
		MAN_DIR="$INPUT"
	else
		echo "\"$INPUT\" が見つかりません。"
	fi
done
echo ""

TMP_DIR=""
while [ "$TMP_DIR" = "" ]
do
	echo "作業ディレクトリ名を入力して下さい。"
	echo $n "作業ディレクトリ(デフォルト:$DEFAULT_TMP_DIR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_TMP_DIR
	fi
	if [ -d "$INPUT" ] ; then
		TMP_DIR="$INPUT"
	else
		echo "\"$INPUT\" が見つかりません。"
	fi
done
echo ""

echo "NNTP はニュースを参照するのに TCP/IP 通信を使用することを意味します。"
Y_or_n "NNTP モードをサポートしますか(NNTP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DNNTP"
fi
echo ""

echo "NSPL はニュースを参照するのにニューススプールを参照することを意味します。"
Y_or_n "NSPL モードをサポートしますか(NSPL)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DNSPL"
fi
echo ""

echo "SMTP はメールを送信するのに TCP/IP 通信を使用することを意味します。"
y_or_N "SMTP モードをサポートしますか(SMTP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DSMTP"
fi
echo ""

echo "MTAP はメールを送信するのに MTA(sendmail) をパイプ起動することを意味します。"
Y_or_n "MTAP モードをサポートしますか(MTAP)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DMTAP"
fi
echo ""

echo "POP3 はメールを受信するのに TCP/IP 通信を使用することを意味します。"
y_or_N "POP3 モードをサポートしますか(POP3)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DPOP3"
fi
echo ""

echo "MSPL はメールを受信するのにメールスプールを参照することを意味します。"
Y_or_n "MSPL モードをサポートしますか(MSPL)?"
if [ $? -eq 1 ] ; then
	NET_OPT="$NET_OPT -DMSPL"
fi
echo ""

NEWS_SPOOL=""
while [ "$NEWS_SPOOL" = "" ]
do
	echo "ニューススプールディレクトリ名を入力して下さい。"
	echo $n "ニューススプールディレクトリ(デフォルト:$DEFAULT_NEWS_SPOOL):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_NEWS_SPOOL"
	fi
	if [ -d "$INPUT" ] ; then
		NEWS_SPOOL="$INPUT"
	else
		echo "\"$INPUT\" が見つかりません。"
		Y_or_n "このエラーを無視しますか?"
		if [ $? -ne 0 ] ; then
			NEWS_SPOOL="$INPUT"
		fi
	fi
done
echo ""

NEWS_LIB=""
while [ "$NEWS_LIB" = "" ]
do
	echo "ニュースライブラリディレクトリ名を入力して下さい。"
	echo $n "ニュースライブラリディレクトリ(デフォルト:$DEFAULT_NEWS_LIB):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_NEWS_LIB"
	fi
	if [ -d "$INPUT" ] ; then
		NEWS_LIB="$INPUT"
	else
		echo "\"$INPUT\" が見つかりません。"
		Y_or_n "このエラーを無視しますか?"
		if [ $? -ne 0 ] ; then
			NEWS_LIB="$INPUT"
		fi
	fi
done
echo ""

MAIL_SPOOL=""
while [ "$MAIL_SPOOL" = "" ]
do
	echo "メールスプールディレクトリ名を入力して下さい。"
	echo $n "メールスプールディレクトリ(デフォルト:$DEFAULT_MAIL_SPOOL):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_MAIL_SPOOL"
	fi
	if [ -d "$INPUT" ] ; then
		MAIL_SPOOL="$INPUT"
	else
		echo "\"$INPUT\" が見つかりません。"
	fi
done
echo ""

POSTNEWS="$DEFAULT_POSTNEWS"
Y_or_n "ニュースを投稿する機能(NEWSPOST)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DNEWSPOST"
	POSTNEWS=""
	while [ "$POSTNEWS" = "" ]
	do
		echo "ニュースを投稿するプログラムとオプションを入力して下さい。"
		echo "もし、ニュースサーバ上で使用しないなら無視してリターンを押して下さい。"
		echo $n "ニュース投稿プログラムとオプション(デフォルト:$DEFAULT_POSTNEWS):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_POSTNEWS"
		fi
		if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
			POSTNEWS="$INPUT"
		else
			echo "警告:\"`echo "$INPUT" | $AWK '{print $1}'`\" が見つかりません。"
			POSTNEWS="$INPUT"
			Y_or_n "このエラーを無視しますか?"
			if [ $? -ne 1 ] ; then
				POSTNEWS=""
			fi
		fi
	done
fi
echo ""

INPUT=""
SENDMAIL="$DEFAULT_SENDMAIL"
Y_or_n "メールを送信する機能(MAILSEND)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMAILSEND"
	SENDMAIL=""
	while [ "$SENDMAIL" = "" ]
	do
		echo "メールを送信するプログラムとオプションを入力して下さい。"
		echo $n "メール送信プログラムとオプション(デフォルト:$DEFAULT_SENDMAIL):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_SENDMAIL"
		fi
		if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
			SENDMAIL="$INPUT"
		else
			echo "\"`echo "$INPUT" | $AWK '{print $1}'`\" が見つかりません。"
		fi
	done
fi
echo ""

Y_or_n "データベースによる日本語変換機能(JNAMES)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DJNAMES"
	echo "ハッシュアルゴリズム JNHASH モジュールは高速ですが、通常モジュールより"
	echo "大量のメモリを消費します。"
	Y_or_n "ハッシュアルゴリズムモジュール(JNHASH)を使用しますか?"
	if [ $? -eq 1 ] ; then
		MOD_OPT="$MOD_OPT -DJNHASH"
	fi
	if [ ! -f "$JNAMES_DB" ] ; then
		echo "警告:JNAMES データベース \"$JNAMES_DB\" が見つかりません。"
	fi
fi
echo ""

MH_PATH="$DEFAULT_MH_PATH"
Y_or_n "MH モード(MH)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMH"
	MH_PATH=""
	while [ "$MH_PATH" = "" ]
	do
		echo "MH コマンドディレクトリを入力して下さい。"
		echo $n "MH コマンドディレクトリ(デフォルト:$DEFAULT_MH_PATH):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_MH_PATH"
		fi
		if [ -d "$INPUT" ] ; then
			MH_PATH="$INPUT"
		else
			echo "\"$INPUT\" が見つかりません。"
		fi
	done
fi
echo ""

Y_or_n "UCB-mail モード(UCBMAIL)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DUCBMAIL"
fi
echo ""

Y_or_n "RMAIL モード(RMAIL)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DRMAIL"
fi
echo ""

BOARD_SPOOL="$DEFAULT_BOARD_SPOOL"
y_or_N "BOARD (電子掲示板)モード(BOARD)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DBOARD"
	BOARD_SPOOL=""
	while [ "$BOARD_SPOOL" = "" ]
	do
		echo "BOARD スプールディレクトリを入力して下さい。"
		echo $n "ボードスプールディレクトリ(デフォルト:$DEFAULT_BOARD_SPOOL):$c"
		read INPUT
		if [ "$INPUT" = "" ] ; then
			INPUT="$DEFAULT_BOARD_SPOOL"
		fi
		if [ -d "$INPUT" ] ; then
			BOARD_SPOOL="$INPUT"
		else
			echo "\"$INPUT\" が見つかりません。"
			BOARD_SPOOL="$INPUT"
			Y_or_n "このエラーを無視しますか?"
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
echo "もし MIME をサポートすれば若干速度が低下します。"
Y_or_n "日本語ヘッダ,マルチパート選択,マルチメディア記事(MIME)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DMIME"
	echo "MPEG プレイヤーとオプションを入力して下さい。"
	echo $n "MPEG プレイヤーとオプション(デフォルト:$DEFAULT_MPEG_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		MPEG_COMMAND="$INPUT"
	fi
	echo ""
	echo "JPEG ビューアーとオプションを入力して下さい。"
	echo $n "JPEG ビューアーとオプション(デフォルト:$DEFAULT_JPEG_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		JPEG_COMMAND="$INPUT"
	fi
	echo ""
	echo "GIF ビューアーとオプションを入力して下さい。"
	echo $n "GIF ビューアーとオプション(デフォルト:$DEFAULT_GIF_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		GIF_COMMAND="$INPUT"
	fi
	echo ""
	echo "オーディオプレイヤーとオプションを入力して下さい。"
	echo $n "オーディオプレイヤーとオプション(デフォルト:$DEFAULT_AUDIO_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		AUDIO_COMMAND="$INPUT"
	fi
	echo ""
	echo "Post Script ビューアーとオプションを入力して下さい。"
	echo $n "Post Script ビューアーとオプション(デフォルト:$DEFAULT_PS_COMMAND):$c"
	read INPUT
	if [ "$INPUT" != "" ] ; then
		PS_COMMAND="$INPUT"
	fi
	echo ""
fi
echo ""

Y_or_n "内蔵ページャー(PAGER)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DPAGER"
#	echo "高速ページャーは通常ページャーより大量のメモリを消費します。"
#	y_or_N "高速ページャー(HPAGER)を使用しますか?"
#	if [ $? -eq 1 ] ; then
#		MOD_OPT="$MOD_OPT -DHPAGER"
#	fi
fi
echo ""

PAGER=""
while [ "$PAGER" = "" ]
do
	echo "内蔵ページャーを組み込んだ場合は -p オプションもしくは"
	echo "設定ファイルで指定した時のみ外部ページャーが使用されます。"
	echo "外部ページャープログラムとオプションを入力して下さい。"
	echo $n "ページャープログラムとオプション(デフォルト:$DEFAULT_PAGER):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_PAGER"
	fi
	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
		PAGER="$INPUT"
	else
		echo "警告:\"`echo "$INPUT" | $AWK '{print $1}'`\" が見つかりません。"
		PAGER="$INPUT"
		Y_or_n "このエラーを無視しますか?"
		if [ $? -ne 1 ] ; then
			PAGER=""
		fi
	fi
done
echo ""

Y_or_n "File carbon copy (FCC)機能を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DFCC"
fi
echo ""

y_or_N "新着メールのチェックに郵便(YOUBIN)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DYOUBIN"
fi
echo ""

echo "もし termcap または terminfo をサポートすれば若干速度が低下します。"
y_or_N "termcap (TERMCAP)を使用しますか?"
if [ $? -eq 1 ] ; then
	CONS_OPT="$CONS_OPT -DTERMCAP"
	DEFAULT_TERMLIB="-ltermcap"
	echo $n "termcap ライブラリを入力して下さい。(デフォルト:$DEFAULT_TERMLIB):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_TERMLIB"
	fi
	LIB="$LIB $INPUT"
else
	y_or_N "terminfo (TERMINFO)を使用しますか?"
	if [ $? -eq 1 ] ; then
		CONS_OPT="$CONS_OPT -DTERMINFO"
		DEFAULT_TERMLIB="-ltermlib"
		echo $n "terminfo ライブラリを入力して下さい。(デフォルト:$DEFAULT_TERMLIB):$c" ; read INPUT
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
	echo "コンフィグレーションオプションを入力して下さい。"
	echo "-DCTRL_L            ^L により改ページをサポートする。"
	echo "-DCOMPLETION        ファイルとフォルダ名補完をサポートする。"
	echo "-DREF_SORT          リファレンスソートをサポートする。"
	echo "-DXOVER             NNTP の XOVER を使用して高速化する。"
	echo "-DCOLOR             カラー画面表示をサポートする。"
	echo "-DADD_HOSTNAME      ホスト名をドメイン名に追加する。"
	echo "-DADD_HOSTPATH      ホスト名をパスフィールドに追加する。"
	echo "-DADD_MESSAGE_ID    Message-ID を mnews が追加する。"
	echo "-DIGNORE_EDITOR     エディタ終了コードを無視する。"
	echo "-DSELECT_EDITOR     記事を書く時にエディタを選択する。"
	echo "-DUSE_NLINK         MH/BOARD モードでリンク数を使用して高速化する。"
	echo "-DDISPLAY_CTRL      無効なコントロールコードを '^'+文字として可視化する。"
	echo "-DRESOLVE           レゾルバを使用する。"
	echo "-DLOCAL_POST        常時 inews を使用して投稿する。"
	echo "-DTRI               TRI 探索木アルゴリズムを使用して高速化する。"
	echo "-DNNTP_AUTH         INN の認証機能に対応する。"
	echo "-DSEND_CONFIRM      ニュース投稿/メール送信時に確認を求める。"
	echo "-DSUPPORT_X0201     JIS-X0201 かな文字をサポートする。"
	echo "-DLARGE             拡張コマンドをサポートする。"
	echo $n "オプション(デフォルト:$DEFAULT_CONF_OPT):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_CONF_OPT"
	fi
	CONF_OPT="$INPUT"
done
echo ""
echo "$CONF_OPT" | grep 'RESOLVE' > /dev/null
if [ $? -eq 0 ] ; then
	DOMAINNAME=""
	y_or_N "御使用の C 標準ライブラリは name resolver を含んでいますか?"
	if [ $? -eq 1 ] ; then
		LIB="$LIB -lresolv"
	fi
	echo ""
else
	INPUT=""
	while [ "$INPUT" = "" ]
	do
		echo "ドメイン(MX)名を入力して下さい。(例:msr.mei.co.jp)"
		if [ "$DOMAINNAME" != "" ] ; then
			echo $n "ドメイン名(デフォルト:$DOMAINNAME):$c"
		else
			echo $n "ドメイン名:$c"
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

K_code "デフォルトの表示コードを選んで下さい。" 4
AWK_FLAG="{ print \$$? }"
DISPLAY_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "デフォルトの判別不能時コードを選んで下さい。(重要ではないです)" 4
AWK_FLAG="{ print \$$? }"
UNKNOWN_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "デフォルトのセーブコードを選んで下さい。" 1
AWK_FLAG="{ print \$$? }"
SAVE_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "デフォルトのエディットコードを選んで下さい。" 4
AWK_FLAG="{ print \$$? }"
EDIT_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

echo "もし日本語サブジェクトを許容していないサイトなら\"無変換\"を選んで下さい。"
K_code "デフォルトのサブジェクト入力コードを選んで下さい。" 1
AWK_FLAG="{ print \$$? }"
INPUT_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "デフォルトの FCC コードを選んで下さい。" 1
AWK_FLAG="{ print \$$? }"
FCC_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "デフォルトのパイプコードを選んで下さい。" 1
AWK_FLAG="{ print \$$? }"
PIPE_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

K_code "デフォルトの印刷コードを選んで下さい。" 1
AWK_FLAG="{ print \$$? }"
LPR_CODE=`echo "$K_STRING" | $AWK "$AWK_FLAG"`
echo ""

LPR_COMMAND=""
echo "デフォルトの印刷コマンドとオプションを入力して下さい。"
echo $n "印刷コマンドとオプション(デフォルト:$DEFAULT_LPR_COMMAND):$c"
read INPUT
if [ "$INPUT" = "" ] ; then
	LPR_COMMAND="$DEFAULT_LPR_COMMAND"
else
	LPR_COMMAND="$INPUT"
fi
echo ""

echo "NNTP モードはニュースアクセス時にニューススプールを参照する代わりに TCP/IP 通信を使用することを意味しています。"
echo "もしあなたのマシンがニュースサーバでないなら 'Y' と答えて下さい。"
Y_or_n "デフォルトでは、NNTP モード(NNTP_MODE)を使用しますか?"
NNTP_MODE=$?
echo ""

echo "SMTP モードはメール送信時に MTA をパイプ起動する代わりに TCP/IP 通信を使用することを意味しています。"
echo "もしあなたのマシンが MTA(sendmail) を持っていないなら 'Y' と答えて下さい。"
y_or_N "デフォルトでは、SMTP モード(SMTP_MODE)を使用しますか?"
SMTP_MODE=$?
echo ""

echo "POP3 モードはメール受信時にメールスプールを参照する代わりに TCP/IP 通信を使用することを意味しています。"
echo "もしあなたのマシンがメールスプールを持っていないなら 'Y' と答えて下さい。"
y_or_N "デフォルトでは、POP3 モード(POP3_MODE)を使用しますか?"
POP3_MODE=$?
echo ""

#y_or_N "POP3 モード時に RPOP (RPOP_MODE)を使用しますか?"
#RPOP_MODE=$?
#echo ""
RPOP_MODE=0

Y_or_n "デフォルトでは、日本語メッセージ(JAPANESE_MODE)モードにしますか?"
JAPANESE_MODE=$?
echo ""

echo "もし X-Nsubject フィールド対応にすれば日本語サブジェクトがニュース/メールシステムの変更なしで送受信できます。"
y_or_N "デフォルトでは、X-Nsubject フィールド(X_NSUBJ_MODE)対応にしますか?"
X_NSUBJ_MODE=$?
echo ""

echo "NSPL モードでは mnews は inews を使用して記事の投稿を行ないます。"
echo "もし inews が自動的に signature を追加するなら mnews 側で signature をつけてはいけません。"
Y_or_n "ニュース投稿プログラム(inews)は、自動的に signature を追加(INEWS_SIG_MODE)しますか?"
INEWS_SIG_MODE=$?
echo ""

Y_or_n "デフォルトでは、日付を日本時間で表示しますか?"
JST_MODE=$?
echo ""

echo "DNAS モードでは mnews は無効なニュースグループを自動削除しません。"
y_or_N "デフォルトでは、DNAS モードをサポートしますか?"
DNAS_MODE=$?
echo ""

#echo "スレッドモードでは mnews は記事をサブジェクト順にソートしますが、速度が若干落ちます。"
#Y_or_n "デフォルトでは、スレッドモードでニュース記事一覧を表示しますか?"
#NEWS_THREAD_MODE=$?
#echo ""

#y_or_N "デフォルトでは、スレッドモードでメール記事一覧を表示しますか?"
#MAIL_THREAD_MODE=$?
#echo ""

CC=""
while [ "$CC" = "" ]
do
	echo "コンパイラプログラムを入力して下さい。"
	echo $n "コンパイラ(デフォルト:$DEFAULT_CC):$c" ; read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT=$DEFAULT_CC
	fi
	if [ -f "$INPUT" ] ; then
		CC="$INPUT"
	else
		echo "\"$INPUT\" が見つかりません。"
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
#	echo "プリ・プロセッサプログラム名を入力して下さい。"
#	echo "もし 'cc -E' が失敗するなら 'cc -E' の代わりに 'cpp' を試して下さい。"
#	echo $n "プリ・プロセッサ(デフォルト:$DEFAULT_CPP):$c" ; read INPUT
#	if [ "$INPUT" = "" ] ; then
#		INPUT=$DEFAULT_CPP
#	fi
#	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
#		CPP="$INPUT"
#	else
#		echo "\"$INPUT\" が見つかりません。"
#	fi
#done
#echo ""

echo "コンパイラの(最適化)オプションを入力して下さい。"
echo $n "オプション(デフォルト:$DEFAULT_CFLAGS):$c" ; read INPUT
if [ "$INPUT" = "" ] ; then
	CFLAGS=$DEFAULT_CFLAGS
else
	CFLAGS="$INPUT"
fi
echo ""

EDITOR=""
while [ "$EDITOR" = "" ]
do
	echo "デフォルトで使用するエディタとオプションを入力して下さい。"
	echo $n "エディタとオプション(デフォルト:$DEFAULT_EDITOR):$c"
	read INPUT
	if [ "$INPUT" = "" ] ; then
		INPUT="$DEFAULT_EDITOR"
	fi
	if [ -f `echo "$INPUT" | $AWK '{print $1}'` ] ; then
		EDITOR="$INPUT"
	else
		echo "\"`echo "$INPUT" | $AWK '{print $1}'`\" が見つかりません。"
	fi
done
echo ""

#INSTALL=""
#while [ "$INSTALL" = "" ]
#do
#	echo "インストールプログラムを入力して下さい。"
#	echo $n "インストーラ(デフォルト:$DEFAULT_INSTALL):$c" ; read INPUT
#	if [ "$INPUT" = "" ] ; then
#		INPUT=$DEFAULT_INSTALL
#	fi
#	if [ -f "$INPUT" ] ; then
#		INSTALL="$INPUT"
#	else
#		echo "\"$INPUT\" が見つかりません。"
#	fi
#done
#echo ""

rm -f default-domain

CONS_OPT=`echo $CONS_OPT | $SED -e "s/XXX //"`
echo "ターゲットアーキテクチャ         = $ARCH"
echo "ターゲットネットワークオプション = $NET_OPT"
echo "ターゲット構成オプション         = $CONS_OPT"
echo "ターゲット設定オプション         = $CONF_OPT"
echo "高速モジュールオプション         = $MOD_OPT"

if [ "$DOMAINNAME" = "" ] ; then
	echo "ドメイン名をリゾルバより取得します。"
else
	echo "ドメイン名                       = \"$DOMAINNAME\""
	echo $DOMAINNAME > default-domain
fi
Y_or_n "以上であっていますか?"
if [ $? -eq 0 ] ; then
	exit 0
fi
echo ""

echo "コンフィグレーションファイルを作成しています。"

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
	echo "\"$MAKESRC\" を使います。"
	cp "$MAKESRC" temp.c
else
	echo "Makefile.src と Makefile.obj が見つかりません。"
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

echo "設定は終了しました。\"$MAKEFILE\" と \"$SITE_DEPEND\" を確かめて下さい。"
echo "確かめ終ったら実行ファイルを作成するために \"make\" とタイプして下さい。"

exit 0
