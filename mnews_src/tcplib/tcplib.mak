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
LLATW32_ddbtcplibdlib =  -LC:\BC45\LIB -Tpd -aa -c -went
RLATW32_ddbtcplibdlib =  -w32
BLATW32_ddbtcplibdlib = 
CNIEAT_ddbtcplibdlib = -IC:\BC45\INCLUDE;..\SHARE -DDELETE_X0212
LNIEAT_ddbtcplibdlib = -x
LEAT_ddbtcplibdlib = $(LLATW32_ddbtcplibdlib)
REAT_ddbtcplibdlib = $(RLATW32_ddbtcplibdlib)
BEAT_ddbtcplibdlib = $(BLATW32_ddbtcplibdlib)

#
# Dependency List
#
Dep_tcplib = \
   ..\tcplib.lib

tcplib : BccW32.cfg $(Dep_tcplib)
  echo MakeNode 

..\tcplib.lib : ..\tcplib.dll
  $(IMPLIB) $@ ..\tcplib.dll


Dep_ddbtcplibddll = \
   tcplib.obj\
   tcplib.res

..\tcplib.dll : $(Dep_ddbtcplibddll)
  $(TLINK32) @&&|
 /v $(IDE_LFLAGS32) $(LEAT_ddbtcplibdlib) $(LNIEAT_ddbtcplibdlib) +
F:\BC45\LIB\c0d32.obj+
tcplib.obj
$<,$*
F:\BC45\LIB\import32.lib+
F:\BC45\LIB\cw32mt.lib

|
   $(BRC32) tcplib.res $<

tcplib.obj :  tcplib.cpp
  $(BCC32) -c @&&|
 $(CEAT_ddbtcplibdlib) $(CNIEAT_ddbtcplibdlib) -o$@ tcplib.cpp
|

tcplib.res :  tcplib.rc
  $(BRC32) $(IDE_RFLAGS32) $(REAT_ddbtcplibdlib) $(CNIEAT_ddbtcplibdlib) -R -FO$@ tcplib.rc

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-R
-v
-vi
-H
-H=tcplib.csm
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


