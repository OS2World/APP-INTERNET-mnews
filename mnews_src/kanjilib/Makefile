#
#	Kanji Library
#	Makefile
#

SHELL	= /bin/sh
ARCH    = SUNOS_41
CC      = cc -O
#CC      = cc -g -DDEBUG
C_FLAG  = -D${ARCH} -DSUPPORT_X0201
LIBS    = 
RM      = rm
RM_FLAG = -f
STRIP   = strip
INSTALL = /bin/install
EXE     = 
OBJ     = kanjilib.o
SRC     = kanjilib.c kanjiext.c
INC     = kanjilib.h kanjiext.h
DOC     = readme.doc kanjilib.doc kanjilib.3 update.doc
ETC	= kanjilib.rc
MAKE    = Makefile kanjilib.mak
ARC     = kanjilib.tar

all: ${OBJ}

.c.o: ${INC}
	${CC} ${C_FLAG} -c $*.c

help:
	@echo "--- Kanji Library Makefile ---"
	@echo "make          --- make object file"
	@echo "make clean    --- remove backup & object file"
	@echo "make remove   --- remove object file"
	@echo "make help     --- print help message"
	@echo "make src_dist --- archive source & document file"

clean:
	-${RM} ${RM_FLAG} ${OBJ} *~ \#* *\#

remove:
	-${RM} ${RM_FLAG} ${OBJ}

src_dist:
	tar cvfh ${ARC} ${SRC} ${INC} ${DOC} ${ETC} ${MAKE}
	gzip -f ${ARC}
