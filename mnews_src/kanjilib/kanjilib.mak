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
LLATW32_ddbkanjilibdlib =  -LC:\BC45\LIB -Tpd -aa -c -went
RLATW32_ddbkanjilibdlib =  -w32
BLATW32_ddbkanjilibdlib = 
CNIEAT_ddbkanjilibdlib = -IC:\BC45\INCLUDE;..\SHARE -DDELETE_X0212
LNIEAT_ddbkanjilibdlib = -x
LEAT_ddbkanjilibdlib = $(LLATW32_ddbkanjilibdlib)
REAT_ddbkanjilibdlib = $(RLATW32_ddbkanjilibdlib)
BEAT_ddbkanjilibdlib = $(BLATW32_ddbkanjilibdlib)

#
# Dependency List
#
Dep_kanjilib = \
   ..\kanjilib.lib

kanjilib : BccW32.cfg $(Dep_kanjilib)
  echo MakeNode 

..\kanjilib.lib : ..\kanjilib.dll
  $(IMPLIB) $@ ..\kanjilib.dll


Dep_ddbkanjilibddll = \
   hankaku.obj\
   kanjilib.obj\
   kanjilib.res

..\kanjilib.dll : $(Dep_ddbkanjilibddll)
  $(TLINK32) @&&|
 /v $(IDE_LFLAGS32) $(LEAT_ddbkanjilibdlib) $(LNIEAT_ddbkanjilibdlib) +
F:\BC45\LIB\c0d32.obj+
hankaku.obj+
kanjilib.obj
$<,$*
F:\BC45\LIB\import32.lib+
F:\BC45\LIB\cw32mt.lib

|
   $(BRC32) kanjilib.res $<

hankaku.obj :  hankaku.cpp
  $(BCC32) -c @&&|
 $(CEAT_ddbkanjilibdlib) $(CNIEAT_ddbkanjilibdlib) -o$@ hankaku.cpp
|

kanjilib.obj :  kanjilib.cpp
  $(BCC32) -c @&&|
 $(CEAT_ddbkanjilibdlib) $(CNIEAT_ddbkanjilibdlib) -o$@ kanjilib.cpp
|

kanjilib.res :  kanjilib.rc
  $(BRC32) $(IDE_RFLAGS32) $(REAT_ddbkanjilibdlib) $(CNIEAT_ddbkanjilibdlib) -R -FO$@ kanjilib.rc

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-R
-v
-vi
-H
-H=kanjilib.csm
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


