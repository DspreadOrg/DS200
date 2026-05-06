cls
@ECHO OFF
CLS
set vmbit=

set CURDIR=%~dp0

:menu
Title Functional Configuration
color 0a
cd /d "%~dp0"
cls
set choice=
echo.
rem echo                   main menu
echo         ===========================
echo.
rem echo         1、关闭防切机，无外挂FLASH
rem echo         2、关闭防切机，有外挂FLASH
rem echo         3、开启防切机，无外挂FLASH
rem echo         4、开启防切机，有外挂FLASH
echo         1、Close SecBoot，no ext FLASH
echo         2、Close SecBoot，have ext FLASH
echo         3、OPEN SecBoot，no ext FLASH
echo         4、OPEN SecBoot，have ext FLASH
echo         0、quit
:cl
echo.
set /p choice=         please select item number, then click enter button:
if /i "%choice%"=="1" goto secclose
if /i "%choice%"=="2" goto secclose_extflash
if /i "%choice%"=="3" goto secopen
if /i "%choice%"=="4" goto secopen_extflash
if /i "%choice%"=="0" goto EX

echo.
echo         Invalid selection, please enter again
echo.
goto cl

:secclose
rem echo "secclose"
		set NAME_EXTDEF=8M
		set NAME_EXT1=nosec
		set NAME_EXT2=nolzma
rem goto menu
goto EX

:secclose_extflash
rem echo "secclose_extflash"
		set NAME_EXTDEF=8M_8M
		set NAME_EXT1=nosec
		set NAME_EXT2=nolzma_extflash
rem goto menu
goto EX

:secopen
rem echo "secopen"
		set NAME_EXTDEF=8M
		set NAME_EXT1=secureboot
		set NAME_EXT2=secureboot
rem goto menu
goto EX


:secopen_extflash
rem echo "secopen_extflash"
		set NAME_EXTDEF=8M_8M
		set NAME_EXT1=secureboot
		set NAME_EXT2=secureboot_extflash
rem goto menu
goto EX


rem pause

:EX
rem echo "EX========="
rem echo on

set COPYFILE1=%CURDIR%\ql-config\quec-project\aboot\config\partition\CRANEL_QUEC_FLASH_LAYOUT_DS_%NAME_EXTDEF%_OPEN.json
set COPYFILE2=%CURDIR%\ql-config\quec-project\aboot\config\product\CRANEL_QUEC_PRODUCT.json
set COPYFILE3=%CURDIR%\ql-config\quec-project\aboot\config\template\CRANEL_QUEC_TEMPLATE_DS_%NAME_EXTDEF%_OPEN.json
set COPYFILE4=%CURDIR%\ql-config\quec-project\scripts\win32\build_package.bat


copy /Y %COPYFILE1%.%NAME_EXT1% %COPYFILE1%
copy /Y %COPYFILE2%.%NAME_EXT1% %COPYFILE2%
copy /Y %COPYFILE3%.%NAME_EXT1% %COPYFILE3%
copy /Y %COPYFILE4%.%NAME_EXT2% %COPYFILE4%

pause >nul

exit