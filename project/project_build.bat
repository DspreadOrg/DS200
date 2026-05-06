cls
@ECHO OFF
CLS
set vmbit=

set CURDIR=%~dp0
if exist %CURDIR%\module.conf (
    set /p module=<%CURDIR%\module.conf
)else (
    set module=
)
if exist %CURDIR%\devtype.conf (
    set /p devtype=<%CURDIR%\devtype.conf
)else (
    set devtype=
)
if /i %PROCESSOR_IDENTIFIER:~0,3% neq x86 set vmbit=64

:menu
rem Title %module%_%devtype% 编译选择
Title cloud code projection build
color 0a
cd /d "%~dp0"
cls
set choice=
echo.
echo                   main menu
echo         ===========================
echo.
rem echo         0、一键安装所有服务(USB、Hostd和虚拟打印机服务请另外安装)
echo         1、build clean
echo         2、build app
echo         3、packet firmware (zip file)
echo         4、packet APP OTA file(bin file)
rem echo         5、packet DIFF APP OTA file(bin file)
echo         6、packet DIFF SYS OTA file(bin file)
echo         7、packet DIFF APP SYS OTA file(bin file)
rem echo         8、安装结果检测并生成install.log日志(用于排错)
rem echo         8、选择型号
echo         8、set device type
echo         0、quit
echo.
echo         nots: if you select item 6 and item 7, kernel will be rebuilded , dn't do it if no needs. just only build app and packet firmware in normol
echo.
echo         prj-dir：%~dp0
:cl
echo.
set /p choice=         please select item number, then click enter button:
IF NOT "%choice%"=="" SET choice=%choice:~0,1%
rem if /i "%choice%"=="0" goto s0
if /i "%choice%"=="1" goto s_build_clean
if /i "%choice%"=="2" goto s_build_app
if /i "%choice%"=="3" goto s_make_firmware
if /i "%choice%"=="4" goto s_make_ota_firmware
rem if /i "%choice%"=="5" goto s_make_diff_app_ota_firmware
if /i "%choice%"=="6" goto s_make_diff_sys_ota_firmware
if /i "%choice%"=="7" goto s_make_diff_app_sys_ota_firmware
if /i "%choice%"=="8" goto s_select_devtype
rem if /i "%choice%"=="e" goto EX
if /i "%choice%"=="0" goto EX

echo.
echo         Invalid selection, please enter again.
echo.
goto cl

:s_select_devtype
set choice=
cls
echo.
echo            device type selection
echo         ===========================
echo.
echo         1、(EC600MCN_LA) DS200
echo         2、(EC600MCN_LA) DS200BT
echo         0、clean device type configuration and return main menu
echo.
:check_select
echo.
set /p choice=         please select item number, then click enter button:
set devtype=
IF NOT "%choice%"=="" SET choice=%choice:~0,1%
if /i "%choice%"=="1" (
	set devtype=DS200
	set module=EC600MCN_LA
)
if /i "%choice%"=="2" (
	set devtype=DS200BT
	set module=EC600MCN_LA
)
if /i "%choice%"=="0" (
	set devtype=UNKNOW
)
if "%devtype%" equ "" (
echo.
echo         Invalid selection, please enter again
echo.
goto check_select
)
:save_devtype

echo %module%>module.conf
call build.bat project_name %module%
echo.
rem pause >nul

echo *****************  MODULE:%module%	***************************
echo *****************  DEVTYPE:%devtype%	***************************

set dev_head_file=%CURDIR%ql-application\threadx\common\include\ymzn\conf_devtype.h
set config_mk_file=%CURDIR%ql-application\threadx\config.mk
if not exist %dev_head_file% (
    echo %dev_head_file% is not exist,please retry
    echo.
    set CURDIR=%~dp0
    pause >nul
    goto s_select_devtype
)
echo %devtype%>devtype.conf
echo #ifndef __CONF_DEVTYPE_H__ > %dev_head_file%
echo #define __CONF_DEVTYPE_H__ >> %dev_head_file%
echo #define CONF_DEVTYPE_DEFAULT "%devtype%">>%dev_head_file%
echo #endif >> %dev_head_file%
echo CONF_DEVTYPE_NAME=%devtype%> %config_mk_file%
echo OK
echo Configuration has been saved toz：
echo %dev_head_file%
echo.
pause >nul
goto menu



:s_build_clean
call build.bat clean
echo.
echo         clean successfully
echo.
pause >nul
goto menu



:s_build_app
rem set module=%choice%
call build.bat app
echo.
echo         App compilation successfully，Please confirm the compilation process prompt.
echo.
pause >nul
goto menu

:s_make_firmware
call build_firmware_nodbg.bat
rem call build.bat firmware
echo.
echo         Firmware packet successfully，Please confirm the compilation process prompt.
echo.
pause >nul
goto menu

:s_make_ota_firmware
call make_ota_firmware.bat
echo         OTA bin packet successfully，Please confirm the compilation process prompt.
pause >nul
goto menu



:s_make_diff_sys_ota_firmware
call make_diff_sys_ota_firmware.bat
echo         OTA bin packet successfully，Please confirm the compilation process prompt.
pause >nul
goto menu

:s_make_diff_app_sys_ota_firmware
call make_diff_app_sys_ota_firmware.bat
echo         OTA bin packet successfully，Please confirm the compilation process prompt.
pause >nul
goto menu



:s4
rem call build_app.bat
call build.bat app
call build_firmware_nodbg.bat
echo.
echo         APPand Firmware packet successfully，Please confirm the note.
echo.
pause >nul
goto menu

:s5
call build.bat bootloader
echo.
echo         bootloader compilation successfully，Please confirm the compilation process prompt
echo.
pause >nul
goto menu

:s6
echo.
set /p no=         pelase confirm again if build kernel? Y or N：
echo.
if /I "%no%"=="y" goto ks
if /I "%no%"=="n" goto menu
echo         input errpr，please re-confirm again...
goto s4
:ks
call build.bat kernel
echo.
echo         kernel compilation successfully，Please confirm the compilation process prompt
echo.
pause >nul
goto menu

:s7
echo.
set /p no=         Please confirm again if build all? Y or N：
echo.
if /I "%no%"=="y" goto ks2
if /I "%no%"=="n" goto menu
echo         input errpr，please re-confirm again...
goto s6
:ks2
call build.bat clean
call build.bat bootloader
call build.bat kernel
call build.bat app
call build.bat firmware
echo.
echo        All compilation successfully，Please confirm the note
echo.
pause >nul
goto menu


:EX
exit