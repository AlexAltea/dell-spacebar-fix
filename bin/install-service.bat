@echo off

set service_dir=C:\Services\DellSpacebarFix
set service_name=DellSpacebarFix

for /f "tokens=2 delims==" %%f in ('wmic os get osarchitecture /value ^| find "="') do set "BITS=%%f"
if "%BITS%"=="32-bit" goto :config32
if "%BITS%"=="64-bit" goto :config64

echo Unsupported architecture!
exit 1

:config32
echo Detected 32-bit Windows
set service_exe=DellSpacebarFix-x32.exe
goto :install

:config64
echo Detected 64-bit Windows
set service_exe=DellSpacebarFix-x64.exe
goto :install

:install
mkdir %service_dir% 2>NUL
copy %service_exe% %service_dir% 1>NUL
set service_exe=%service_dir%\%service_exe%
echo Installing to %service_exe%
sc create %service_name% binpath="%service_exe% service" start=auto
sc start %service_name%

:success
echo Complete!
pause
