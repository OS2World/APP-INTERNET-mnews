#
#	TCP/IP Library
#	Makefile
#

SHELL	= /bin/sh
ARCH    = SUNOS_41
CC      = cc -O
#CC      = cc -g -DDEBUG
C_FLAG  = -D${ARCH}
LIBS    = 
RM      = rm
RM_FLAG = -f
STRIP   = strip
INSTALL = /bin/install
EXE     = 
OBJ     = tcplib.o
SRC     = tcplib.c
INC     = tcplib.h
DOC     = readme.doc tcplib.doc tcplib.3 update.doc
MAKE    = Makefile
ARC     = tcplib.tar

all: ${OBJ}

.c.o: ${INC}
	${CC} ${C_FLAG} -c $*.c

help:
	@echo "--- TCP/IP Library Makefile ---"
	@echo "make          --- make object file"
	@echo "make clean    --- remove backup & object file"
	@echo "make remove   --- remove object file"
	@echo "make help     --- print help message"
	@echo "make src_dist --- archive source & document file"

clean:
	-${RM} ${RM_FLAG} ${EXE} ${OBJ} *~ \#* *\#

remove:
	-${RM} ${RM_FLAG} ${EXE} ${OBJ}

src_dist:
	tar cvfh ${ARC} ${SRC} ${INC} ${DOC} ${MAKE}
	gzip -f ${ARC}
