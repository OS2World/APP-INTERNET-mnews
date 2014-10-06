@echo off
mode co80,35

SET MNEWSRC=.newsrc
SET NNTPSERVER=news.hogehoge.ne.jp
SET SMTPSERVER=mail.hogehoge.ne.jp
SET POP3SERVER=pop3.hogehoge.ne.jp
SET DOMAINNAME=hogehoge.ne.jp
SET NAME=Your Name
SET ORGANIZATION=Your Organization
SET SAVEDIR=d:\home\news\save
SET RMAIL=d:\home\spool\mail\rmail
SET MBOX=d:\home\spool\mail\mbox
SET EDITOR=TEDIT.EXE

rem --- use when offline mode ---
rem SET NEWSSPOOL=d:\gnspool\usr\spool\news
rem SET NEWSLIB=d:\gnspool\usr\lib\news

d:\usr\local\bin\mnews.exe %1 %2 %3 %4 %5 %6 %7 %8 %9