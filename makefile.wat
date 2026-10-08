# HalfWorm for OS/2 - OpenWatcom wmake build file

!ifndef OS2TK
OS2TK = C:\os2tk45
!endif

CC     = wcc386
LINK   = wlink
RC     = wrc
RM     = rm -f
WIPFC  = wipfc

CFLAGS = -bt=os2 -mf -5 -fpi -Oaxt -W3 -ze -d0 &
         -fi=src\ow_compat.h &
         -i=$(OS2TK)\h -i=src

OBJS = bin\HalfWorm.obj &
       bin\ClientWindow.obj &
       bin\GameThread.obj &
       bin\FrameSubProc.obj &
       bin\StatusbarWindow.obj &
       bin\graphlib.obj &
       bin\SoundEngine.obj &
       bin\configuration.obj &
       bin\PMX.obj

TARGET = bin\HalfWorm.exe
HLPS   = bin\help\HalfWorm_en.hlp bin\help\HalfWorm_es.hlp bin\help\HalfWorm_nl.hlp bin\help\HalfWorm_de.hlp bin\help\HalfWorm_fr.hlp bin\help\HalfWorm_it.hlp

all : $(TARGET) $(HLPS)

$(TARGET) : $(OBJS) bin\HalfWorm.res
	$(LINK) system os2v2 pm &
		name $(TARGET) &
		modfile src\HalfWorm.def &
		file bin\HalfWorm.obj,bin\ClientWindow.obj,bin\GameThread.obj,bin\FrameSubProc.obj,bin\StatusbarWindow.obj,bin\graphlib.obj,bin\SoundEngine.obj,bin\configuration.obj,bin\PMX.obj &
		library $(OS2TK)\lib\mmpm2.lib &
		option stack=65536 &
		option map=bin\HalfWorm.map
	$(RC) bin\HalfWorm.res $(TARGET)

bin\HalfWorm.res : src\HalfWorm.rc src\resources.h src\HalfWorm.ico
	$(RC) -r -i=$(OS2TK)\h -i=src src\HalfWorm.rc -fo=bin\HalfWorm.res

bin\HalfWorm.obj : src\HalfWorm.c src\resources.h src\ClientWindow.h src\FrameSubProc.h src\StatusbarWindow.h src\ow_compat.h
	$(CC) $(CFLAGS) src\HalfWorm.c -fo=$@

bin\ClientWindow.obj : src\ClientWindow.c src\ClientWindow.h src\GameThread.h src\resources.h src\configuration.h src\pmx.h src\ow_compat.h
	$(CC) $(CFLAGS) src\ClientWindow.c -fo=$@

bin\GameThread.obj : src\GameThread.c src\GameThread.h src\resources.h src\configuration.h src\pmx.h src\ow_compat.h
	$(CC) $(CFLAGS) src\GameThread.c -fo=$@

bin\FrameSubProc.obj : src\FrameSubProc.c src\FrameSubProc.h src\resources.h src\ow_compat.h
	$(CC) $(CFLAGS) src\FrameSubProc.c -fo=$@

bin\StatusbarWindow.obj : src\StatusbarWindow.c src\StatusbarWindow.h src\ow_compat.h
	$(CC) $(CFLAGS) src\StatusbarWindow.c -fo=$@

bin\graphlib.obj : src\graphlib.c src\graphlib.h src\ow_compat.h
	$(CC) $(CFLAGS) src\graphlib.c -fo=$@

bin\SoundEngine.obj : src\SoundEngine.c src\SoundEngine.h src\resources.h src\ow_compat.h
	$(CC) $(CFLAGS) src\SoundEngine.c -fo=$@

bin\configuration.obj : src\configuration.c src\configuration.h src\resources.h src\ow_compat.h
	$(CC) $(CFLAGS) src\configuration.c -fo=$@

bin\PMX.obj : src\pmx.c src\pmx.h src\ow_compat.h
	$(CC) $(CFLAGS) src\pmx.c -fo=$@

bin\help :
	@if not exist bin\help mkdir bin\help

bin\help\HalfWorm_en.hlp : help\HalfWorm_en.ipf bin\help
	@echo Compiling help\HalfWorm_en.ipf
	@$(WIPFC) -o $@ help\HalfWorm_en.ipf

bin\help\HalfWorm_es.hlp : help\HalfWorm_es.ipf bin\help
	@echo Compiling help\HalfWorm_es.ipf
	@$(WIPFC) -o $@ help\HalfWorm_es.ipf

bin\help\HalfWorm_nl.hlp : help\HalfWorm_nl.ipf bin\help
	@echo Compiling help\HalfWorm_nl.ipf
	@$(WIPFC) -o $@ help\HalfWorm_nl.ipf

bin\help\HalfWorm_de.hlp : help\HalfWorm_de.ipf bin\help
	@echo Compiling help\HalfWorm_de.ipf
	@$(WIPFC) -l de_DE -o $@ help\HalfWorm_de.ipf

bin\help\HalfWorm_fr.hlp : help\HalfWorm_fr.ipf bin\help
	@echo Compiling help\HalfWorm_fr.ipf
	@$(WIPFC) -l fr_FR -o $@ help\HalfWorm_fr.ipf

bin\help\HalfWorm_it.hlp : help\HalfWorm_it.ipf bin\help
	@echo Compiling help\HalfWorm_it.ipf
	@$(WIPFC) -o $@ help\HalfWorm_it.ipf

clean : .SYMBOLIC
	$(RM) bin\*.obj bin\*.res bin\HalfWorm.exe bin\HalfWorm.map
	@if exist bin\help\*.hlp del bin\help\*.hlp >nul
