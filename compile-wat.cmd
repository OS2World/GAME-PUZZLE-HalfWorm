@echo off
rem HalfWorm for OS/2 - OpenWatcom build script
rem Logs output to compile-wat.log

set LOGFILE=compile-wat.log
echo. > %LOGFILE%
echo HalfWorm OpenWatcom Build >> %LOGFILE%
echo ========================= >> %LOGFILE%

rem Auto-detect OpenWatcom
if exist C:\watcom\binp\wmake.exe set WATCOM=C:\watcom
if exist C:\OpenWatcom\binp\wmake.exe set WATCOM=C:\OpenWatcom
if exist D:\watcom\binp\wmake.exe set WATCOM=D:\watcom

if "%WATCOM%"=="" goto :nowatcom

rem Set up OpenWatcom environment
set PATH=%WATCOM%\binp;%WATCOM%\bin;%PATH%
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os2
set EMXOMFLD_TYPE=WLINK
set EMXOMFLD_LINKER=wl.exe
set EMXOMFLD_PRELINK=0

rem Set OS/2 Toolkit
if "%OS2TK%"=="" set OS2TK=C:\os2tk45
set WIPFC=%WATCOM%\wipfc

echo WATCOM=%WATCOM% | tee -a %LOGFILE%
echo OS2TK=%OS2TK% | tee -a %LOGFILE%

rem Ensure bin directory exists
if not exist bin md bin
if not exist bin\help md bin\help

echo. | tee -a %LOGFILE%
echo Running wmake... | tee -a %LOGFILE%
%WATCOM%\binp\wmake -f makefile.wat 2>&1 | tee -a %LOGFILE%

if not exist bin\HalfWorm.exe goto :failed
if not exist bin\help\HalfWorm_en.hlp goto :failed

echo. | tee -a %LOGFILE%
echo BUILD SUCCESSFUL: bin\HalfWorm.exe and bin\help\*.hlp | tee -a %LOGFILE%
goto :end

:nowatcom
echo ERROR: OpenWatcom not found. | tee -a %LOGFILE%
echo Set WATCOM environment variable to your OpenWatcom path. | tee -a %LOGFILE%
goto :end

:failed
echo. | tee -a %LOGFILE%
echo BUILD FAILED - see %LOGFILE% for details | tee -a %LOGFILE%

:end
