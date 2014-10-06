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
LLATW32_ddbsmtplibdlib =  -LC:\BC45\LIB -Tpd -aa -c -went
RLATW32_ddbsmtplibdlib =  -w32
BLATW32_ddbsmtplibdlib = 
CNIEAT_ddbsmtplibdlib = -IC:\BC45\INCLUDE;..\SHARE -DDELETE_X0212
LNIEAT_ddbsmtplibdlib = -x
LEAT_ddbsmtplibdlib = $(LLATW32_ddbsmtplibdlib)
REAT_ddbsmtplibdlib = $(RLATW32_ddbsmtplibdlib)
BEAT_ddbsmtplibdlib = $(BLATW32_ddbsmtplibdlib)

#
# Dependency List
#
Dep_smtplib = \
   ..\smtplib.lib

smtplib : BccW32.cfg $(Dep_smtplib)
  echo MakeNode 

..\smtplib.lib : ..\smtplib.dll
  $(IMPLIB) $@ ..\smtplib.dll


Dep_ddbsmtplibddll = \
   smtplib.obj\
   smtplib.res

..\smtplib.dll : $(Dep_ddbsmtplibddll)
  $(TLINK32) @&&|
 /v $(IDE_LFLAGS32) $(LEAT_ddbsmtplibdlib) $(LNIEAT_ddbsmtplibdlib) +
F:\BC45\LIB\c0d32.obj+
smtplib.obj
$<,$*
F:\BC45\LIB\import32.lib+
F:\BC45\LIB\cw32mt.lib

|
   $(BRC32) smtplib.res $<

smtplib.obj :  smtplib.cpp
  $(BCC32) -c @&&|
 $(CEAT_ddbsmtplibdlib) $(CNIEAT_ddbsmtplibdlib) -o$@ smtplib.cpp
|

smtplib.res :  smtplib.rc
  $(BRC32) $(IDE_RFLAGS32) $(REAT_ddbsmtplibdlib) $(CNIEAT_ddbsmtplibdlib) -R -FO$@ smtplib.rc

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-R
-v
-vi
-H
-H=smtplib.csm
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


