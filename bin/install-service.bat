@echo off
@setlocal enableextensions
@cd /d "%~dp0"

set service_dir=C:\Services\DellSpacebarFix
set service_name=DellSpacebarFix

:admin
net session >nul 2>&1
if %errorLevel% NEQ 0 (
  echo Error: This script requires administrator privileges.
  echo Please, right-click "%~n0%~x0" and press "Run as administrator".
  pause
  exit 0
)

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
echo Y | call uninstall-service.bat >NUL
mkdir %service_dir% 2>NUL
copy %service_exe% %service_dir% 1>NUL
set service_exe=%service_dir%\%service_exe%
echo Installing to %service_exe%
reg add HKLM\Software\Microsoft\Windows\CurrentVersion\Run /f /v %service_name% /d %service_exe% 1>NUL
start %service_exe%

:success
echo Complete!
pause
