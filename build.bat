@echo off
setlocal enabledelayedexpansion

rem call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64

:: Output folders
set "OUT_EXE=RCRapidClicker.exe"
set "OBJ_DIR=build\obj"
set "BIN_DIR=build\bin"
set "DBG_DIR=build\debug"

:: === Find all .c files in current directory (non-recursive) ===
set SOURCES=
for %%f in (*.c) do (
    set SOURCES=!SOURCES! %%f
)

:: Create directories if they don't exist
if not exist %OBJ_DIR% mkdir %OBJ_DIR%
if not exist %BIN_DIR% mkdir %BIN_DIR%
if not exist %DBG_DIR% mkdir %DBG_DIR%


:: Compile and link
echo Compiling...
cl /Zi /c !SOURCES! /Fo%OBJ_DIR%\ /Fd%DBG_DIR%\
pause
if errorlevel 1 (
    echo Build failed during compilation.
    pause
    exit /b 1
)

echo Linking...
link %OBJ_DIR%\*.obj /OUT:%BIN_DIR%\%OUT_EXE% /DEBUG /PDB:%DBG_DIR%\%OUT_EXE:.exe=.pdb%

if errorlevel 1 (
    echo Build failed during linking.
    pause
    exit /b 1
)

echo Build successful. Executable is in %BIN_DIR%\%OUT_EXE%
endlocal
