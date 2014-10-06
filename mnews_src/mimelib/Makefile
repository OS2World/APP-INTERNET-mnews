#
#	MIME Library
#	Makefile
#

SHELL	= /bin/sh
ARCH    = SUNOS_41
CC      = cc -O
#CC      = cc -g -DDEBUG
C_FLAG  = -D${ARCH} -DMIME
LIBS    = 
RM      = rm
RM_FLAG = -f
STRIP   = strip
INSTALL = /bin/install
OBJ     = mimelib.o
SRC     = mimelib.c
INC     = mimelib.h
DOC     = readme.doc mimelib.doc mimelib.3 update.doc
ETC	= mimelib.rc
MAKE    = Makefile mimelib.mak
ARC     = mimelib.tar

all: ${OBJ}

.c.o: ${INC}
	${CC} ${C_FLAG} -c $*.c

help:
	@echo "--- MIME Library Makefile ---"
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
