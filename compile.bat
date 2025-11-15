@echo off
setlocal

set GAME=%1
if "%GAME%"=="" (
    echo Usage: %0 GameFolder
    exit /b
)

cd ..\%GAME%
echo [GAMEBYTE] Compiling 32-bit Resource
windres -F pe-i386 ..\\gamebyte\\gamebyte.rc -O coff -o GAMEBYTE32.o
windres -F pe-i386 %GAME%.rc -O coff -o %GAME%32.o
echo [GAMEBYTE] Compiling 32-bit Program
g++ -s -o %GAME%32.exe -flto %GAME%.cpp GAMEBYTE32.o %GAME%32.o -Os -m32 -mwindows -lmsimg32 -lwinmm -Wl,--gc-sections -D win32

echo [GAMEBYTE] Compiling 64-bit Resource
windres -F pe-x86-64 %GAME%.rc -O coff -o %GAME%64.o
windres -F pe-x86-64 ..\\gamebyte\\gamebyte.rc -O coff -o GAMEBYTE64.o
echo [GAMEBYTE] Compiling 64-bit Program
g++ -s -o %GAME%64.exe -flto %GAME%.cpp GAMEBYTE64.o %GAME%64.o -Os -m64 -mwindows -lmsimg32 -lwinmm -Wl,--gc-sections -D win64

echo [GAMEBYTE] Compressing programs
UPX -qq -9 %GAME%32.exe %GAME%64.exe >nul 2>&1

echo [GAMEBYTE] Deleting Resource
del %GAME%32.o
del %GAME%64.o

del GAMEBYTE32.o
del GAMEBYTE64.o

move /Y %GAME%32.exe build\ >nul 2>&1
move /Y %GAME%64.exe build\ >nul 2>&1

cd build\

set "ZIPARGS="
set "DONTZIP=0"
:collect
shift
if "%~1"=="" goto donecollect
if /I "%~1"=="dontzip" set DONTZIP=1 & goto collect
set "ZIPARGS=%ZIPARGS% "%~1""
goto collect
:donecollect

del %GAME%.7z

setlocal enabledelayedexpansion

for /f "tokens=3" %%A in ('dir /s ^| find "File(s)"') do set size=%%A

set size=!size:,=!

set /a kb=!size!/1024

if %DONTZIP%==0 (
	echo [GAMEBYTE] Zipping
    "C:\Program Files\7-Zip\7z.exe" a -bd -bb0 -y -t7z %GAME%.7z %GAME%32.exe %GAME%64.exe %ZIPARGS% -mx9 >nul 2>&1
)

echo [GAMEBYTE] Finished
echo [GAMEBYTE] Uncompressed size: !kb!KB

if %DONTZIP%==0 (
	for /f "tokens=3" %%A in ('dir !GAME!.7z /s ^| find "File(s)"') do set siz=%%A
	
	set siz=!siz:,=!

	set /a kb=!siz!/1024
	echo [GAMEBYTE] Compressed size: !kb!KB
)

pause