@echo off
:loop
tasklist /fi "ImageName eq WSAmateur.exe" /fo csv 2>NUL | find /I "WSAmateur.exe">NUL
if "%ERRORLEVEL%"=="0" goto :loop

FOR /f "tokens=*" %%G IN ('dir /b %~dp0\tmp\WSAmateur*.exe') DO %~dp0\tmp\%%G /S
RMDIR /S /Q %~dp0\tmp
%1
