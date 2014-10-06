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
LLATW32_ddbmimelibdlib =  -LC:\BC45\LIB -Tpd -aa -c
RLATW32_ddbmimelibdlib =  -w32
BLATW32_ddbmimelibdlib = 
CNIEAT_ddbmimelibdlib = -IC:\BC45\INCLUDE;..\KANJILIB;..\SHARE -D
LNIEAT_ddbmimelibdlib = -x
LEAT_ddbmimelibdlib = $(LLATW32_ddbmimelibdlib)
REAT_ddbmimelibdlib = $(RLATW32_ddbmimelibdlib)
BEAT_ddbmimelibdlib = $(BLATW32_ddbmimelibdlib)

#
# Dependency List
#
Dep_mimelib = \
   ..\mimelib.lib

mimelib : BccW32.cfg $(Dep_mimelib)
  echo MakeNode 

..\mimelib.lib : ..\mimelib.dll
  $(IMPLIB) $@ ..\mimelib.dll


Dep_ddbmimelibddll = \
   mimelib.obj\
   mimelib.res

..\mimelib.dll : $(Dep_ddbmimelibddll)
  $(TLINK32) @&&|
 /v $(IDE_LFLAGS32) $(LEAT_ddbmimelibdlib) $(LNIEAT_ddbmimelibdlib) +
F:\BC45\LIB\c0d32.obj+
mimelib.obj
$<,$*
F:\BC45\LIB\import32.lib+
F:\BC45\LIB\cw32mt.lib

|
   $(BRC32) mimelib.res $<

mimelib.obj :  mimelib.cpp
  $(BCC32) -c @&&|
 $(CEAT_ddbmimelibdlib) $(CNIEAT_ddbmimelibdlib) -o$@ mimelib.cpp
|

mimelib.res :  mimelib.rc
  $(BRC32) $(IDE_RFLAGS32) $(REAT_ddbmimelibdlib) $(CNIEAT_ddbmimelibdlib) -R -FO$@ mimelib.rc

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-R
-v
-vi
-H
-H=mimelib.csm
-ps
-Ot
-O-d
-Og
-Om
-Op
-Oi
-Ov
-Z
-O
-Oe
-Ol
-Ob
-OW
-v-
-R-
-WM
-WD
-d
-f-
| $@

