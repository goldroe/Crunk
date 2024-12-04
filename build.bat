@echo off

set MODE=%1
if [%1]==[] (
  set MODE=debug
)

set PROJECT=Crunk

if not exist bin mkdir bin
set WARNING_FLAGS=/W4 /wd4100 /wd4101 /wd4189 /wd4201 /wd4456 /wd4505 /wd4706
set INCLUDES=/Isrc\ /Iext\ /Iext\freetype\include\ /Iext\libnoise-1.0.0\include /Iext\noiseutils\
set COMMON_COMPILER_FLAGS=/nologo /FC /EHsc /Fdbin\ /Fobin\ %WARNING_FLAGS% %INCLUDES%

set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS%
if %MODE%==release (
  set COMPILER_FLAGS=/O2 /D %COMPILER_FLAGS%
) else if %mode%==debug (
  set COMPILER_FLAGS=/Zi /Od %COMPILER_FLAGS%
) else (
  echo Unkown build mode
  exit /B 2
)

set LIBS=/LIBPATH:ext\freetype\libs\x64\Release\ /LIBPATH:ext\libnoise-1.0.0\bin\ user32.lib shell32.lib kernel32.lib winmm.lib shlwapi.lib freetype.lib libnoise.lib
set COMMON_LINKER_FLAGS=/INCREMENTAL:NO /OPT:REF
set LINKER_FLAGS=%COMMON_LINKER_FLAGS% %LIBS%

:: Build Project
CL %COMPILER_FLAGS% src\crunk/crunk_main.cpp ext\noiseutils\noiseutils.cpp /Fe%PROJECT%.exe /link %LINKER_FLAGS% || exit /b 1
