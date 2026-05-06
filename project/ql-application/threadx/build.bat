@echo off

set CURDIR=%~dp0
set target=%1

set /P PlatformType=<..\..\ql-config\config\QuecPlatformType.ini
if "%PlatformType%"=="CRANEM" (
	set APP_INPSRAM=ENABLE
)

if "%target%" == "clean" (
	gnumake -f Makefile clean TOP_DIR=" %CURDIR%\" CROSS_DIR="%cross_tool%"
) else (
	gnumake -f Makefile TOP_DIR=" %CURDIR%\" CROSS_DIR="%cross_tool%"
)
exit /b 0
