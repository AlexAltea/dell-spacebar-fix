@echo off

set service_dir=C:\Services\DellSpacebarFix
set service_name=DellSpacebarFix

:uninstall
echo Uninstalling %service_name%
sc stop %service_name%
sc delete %service_name%
rmdir /s /q %service_dir% 2>NUL

:success
echo Complete!
pause
