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
LLATW32_ddbtermlibdlib =  -LC:\BC45\LIB -Tpd -aa -c -went
RLATW32_ddbtermlibdlib =  -w32
BLATW32_ddbtermlibdlib = 
CNIEAT_ddbtermlibdlib = -IC:\BC45\INCLUDE;..\SHARE -DDELETE_X0212
LNIEAT_ddbtermlibdlib = -x
LEAT_ddbtermlibdlib = $(LLATW32_ddbtermlibdlib)
REAT_ddbtermlibdlib = $(RLATW32_ddbtermlibdlib)
BEAT_ddbtermlibdlib = $(BLATW32_ddbtermlibdlib)

#
# Dependency List
#
Dep_termlib = \
   ..\termlib.lib

termlib : BccW32.cfg $(Dep_termlib)
  echo MakeNode 

..\termlib.lib : ..\termlib.dll
  $(IMPLIB) $@ ..\termlib.dll


Dep_ddbtermlibddll = \
   termlib.obj\
   termlib.res

..\termlib.dll : $(Dep_ddbtermlibddll)
  $(TLINK32) @&&|
 /v $(IDE_LFLAGS32) $(LEAT_ddbtermlibdlib) $(LNIEAT_ddbtermlibdlib) +
F:\BC45\LIB\c0d32.obj+
termlib.obj
$<,$*
F:\BC45\LIB\import32.lib+
F:\BC45\LIB\cw32mt.lib

|
   $(BRC32) termlib.res $<

termlib.obj :  termlib.cpp
  $(BCC32) -c @&&|
 $(CEAT_ddbtermlibdlib) $(CNIEAT_ddbtermlibdlib) -o$@ termlib.cpp
|

termlib.res :  termlib.rc
  $(BRC32) $(IDE_RFLAGS32) $(REAT_ddbtermlibdlib) $(CNIEAT_ddbtermlibdlib) -R -FO$@ termlib.rc

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-R
-v
-vi
-H
-H=termlib.csm
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


