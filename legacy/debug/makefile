.SUFFIXES:
.SUFFIXES: .c .rc .obj .h .exe .res

.SILENT:

BASENAME=HalfWorm

all: $(BASENAME).exe

SRCDIR=..

{..}.c.obj:
   icc /C /Gd+ /Gm+ /Ti+ /O- /Q+ /I. /DDEBUG /DDEBUG_TERM /Wall+ppc-ppt- %s

{..}.rc.res:
   rc -n -r -i .. %s -d DEBUG $(@B).res

!include ..\makeglobals

$(BASENAME).exe: $(OBJS) $(@B).def $(@B).res
   ilink /nologo @<<
      /noinformation /exec /out:$(@B) /map:$(@B) /debug /base:0x00010000
      $(OBJS)
      $(@B).def
      $(LLIBS)
   <<
   rc -n $(@B).res $@
   dllrname /Q /R $@ cppom30=os2om30
   %do successful_build

clean:
   -del *.res
   -del *.obj
   -del *.exe
   -del *.map
   -del *.ini
   -del *.log

