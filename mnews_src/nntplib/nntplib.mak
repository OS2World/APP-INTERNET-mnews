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
LLATW32_ddbnntplibdlib =  -LC:\BC45\LIB -Tpd -aa -c -went
RLATW32_ddbnntplibdlib =  -w32
BLATW32_ddbnntplibdlib = 
CNIEAT_ddbnntplibdlib = -IC:\BC45\INCLUDE;..\SHARE -DDELETE_X0212
LNIEAT_ddbnntplibdlib = -x
LEAT_ddbnntplibdlib = $(LLATW32_ddbnntplibdlib)
REAT_ddbnntplibdlib = $(RLATW32_ddbnntplibdlib)
BEAT_ddbnntplibdlib = $(BLATW32_ddbnntplibdlib)

#
# Dependency List
#
Dep_nntplib = \
   ..\nntplib.lib

nntplib : BccW32.cfg $(Dep_nntplib)
  echo MakeNode 

..\nntplib.lib : ..\nntplib.dll
  $(IMPLIB) $@ ..\nntplib.dll


Dep_ddbnntplibddll = \
   nntplib.obj\
   nntplib.res

..\nntplib.dll : $(Dep_ddbnntplibddll)
  $(TLINK32) @&&|
 /v $(IDE_LFLAGS32) $(LEAT_ddbnntplibdlib) $(LNIEAT_ddbnntplibdlib) +
F:\BC45\LIB\c0d32.obj+
nntplib.obj
$<,$*
F:\BC45\LIB\import32.lib+
F:\BC45\LIB\cw32mt.lib

|
   $(BRC32) nntplib.res $<

nntplib.obj :  nntplib.cpp
  $(BCC32) -c @&&|
 $(CEAT_ddbnntplibdlib) $(CNIEAT_ddbnntplibdlib) -o$@ nntplib.cpp
|

nntplib.res :  nntplib.rc
  $(BRC32) $(IDE_RFLAGS32) $(REAT_ddbnntplibdlib) $(CNIEAT_ddbnntplibdlib) -R -FO$@ nntplib.rc

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-R
-v
-vi
-H
-H=nntplib.csm
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


