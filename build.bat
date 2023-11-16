@echo off

rem target arch must be x64 (amd64)

if "%VCToolsInstallDir%"=="" (
  if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=amd64
  )
)

cl /nologo /Zi /Gm- /Gy /Gw /c /std:c11 test.c

if %errorlevel% neq 0 (
  echo Compilation failed
  exit /b 1
)

cl /nologo /Zi /Gm- /Gy /Gw /c /std:c++20 main.cc

if %errorlevel% neq 0 (
  echo Compilation failed
  exit /b 1
)

link /nologo /out:main.exe test.obj main.obj /FUNCTIONPADMIN "/OPT:NOREF" "/OPT:NOICF" "/DEBUG:FULL"

if %errorlevel% neq 0 (
  echo Link failed
  exit /b 1
)

echo Build succeeded