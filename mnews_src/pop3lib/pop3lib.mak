#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC32   = Bcc32 +BccW32.cfg 
TLINK32 = TLink32
TLIB    = TLib
BRC32   = Brc32
TASM32  = Tasm32
#
# IDE macros
#


#
# Options
#
IDE_LFLAGS32 =  -LD:\BC4\LIB
IDE_RFLAGS32 = 
LLATW32_ddbpop3libdlib =  -LC:\BC45\LIB -Tpd -aa -c -went
RLATW32_ddbpop3libdlib =  -w32
BLATW32_ddbpop3libdlib = 
CNIEAT_ddbpop3libdlib = -IC:\BC45\INCLUDE;..\SHARE -DDELETE_X0212
LNIEAT_ddbpop3libdlib = -x
LEAT_ddbpop3libdlib = $(LLATW32_ddbpop3libdlib)
REAT_ddbpop3libdlib = $(RLATW32_ddbpop3libdlib)
BEAT_ddbpop3libdlib = $(BLATW32_ddbpop3libdlib)

#
# Dependency List
#
Dep_pop3lib = \
   ..\pop3lib.lib

pop3lib : BccW32.cfg $(Dep_pop3lib)
  echo MakeNode 

..\pop3lib.lib : ..\pop3lib.dll
  $(IMPLIB) $@ ..\pop3lib.dll


Dep_ddbpop3libddll = \
   pop3lib.obj\
   pop3lib.res

..\pop3lib.dll : $(Dep_ddbpop3libddll)
  $(TLINK32) @&&|
 /v $(IDE_LFLAGS32) $(LEAT_ddbpop3libdlib) $(LNIEAT_ddbpop3libdlib) +
F:\BC45\LIB\c0d32.obj+
pop3lib.obj
$<,$*
F:\BC45\LIB\import32.lib+
F:\BC45\LIB\cw32mt.lib

|
   $(BRC32) pop3lib.res $<

pop3lib.obj :  pop3lib.cpp
  $(BCC32) -c @&&|
 $(CEAT_ddbpop3libdlib) $(CNIEAT_ddbpop3libdlib) -o$@ pop3lib.cpp
|

pop3lib.res :  pop3lib.rc
  $(BRC32) $(IDE_RFLAGS32) $(REAT_ddbpop3libdlib) $(CNIEAT_ddbpop3libdlib) -R -FO$@ pop3lib.rc

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-R
-v
-vi
-H
-H=pop3lib.csm
-ps
-v-
-R-
-Og
-Ot
-Z
-O
-Oe
-Ol
-Ob
-OW
-Om
-Op
-Oi
-Ov
-WM
-WD
-d
-f-
-wrch
-weff
-waus
-wpar
-wcpt
-wrpt
-wrng
-w-sig
-w-ucp
| $@


