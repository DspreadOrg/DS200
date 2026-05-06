set TOP_DIR=%1
set OPTION=%2

set buildver=LTE01R06A11_C_SDK_A

set YEAR=%date:~0,4%
set MONTH=%date:~5,2%
set DAY=%date:~8,2%

set HOUR=%time:~0,2%
if %HOUR% lss 10 (set HOUR=0%time:~1,1%)

set MINUTE=%time:~3,2%

set SECOND=%time:~6,2%

rem set BUILD_TIME=%YEAR%%MONTH%%DAY%_%HOUR%%MINUTE%%SECOND%
set BUILD_TIME=%YEAR%%MONTH%%DAY%

rem echo "%date%"
rem echo "%time%"
rem echo "%BUILD_TIME%"

if exist %TOP_DIR%\devtype.conf (
	set /p devtype_name=<%TOP_DIR%\devtype.conf
)else (
	set devtype_name=FW
)

if exist %TOP_DIR%\project_name.conf (
	set /p build_version=<%TOP_DIR%\project_name.conf
)else (
	set build_version=%buildver%
)

set /P project_name=<%TOP_DIR%\ql-config\config\QuecCurPrj.txt
set soc_platform_path=%TOP_DIR%\ql-config\soc_platform
rem echo "=================="
rem set /p OEM_PROJECT=<%TOP_DIR%\ql-config\config\QuecOemPrj.txt
rem echo "=================="

rem set /P RAM_SIZE=<%TOP_DIR%\ql-kernel\threadx\config\ram\RAM_SIZE.ini
set /P RAM_SIZE=<%TOP_DIR%\ql-config\config\%project_name%\RAM_SIZE.ini
set /P FLASHTYPE=<%TOP_DIR%\ql-config\config\QuecFLashType.ini
set /P PACKAGE_PROJECT=<%TOP_DIR%\ql-config\config\QuecPlatformType.ini

set buildver_name=%devtype_name%_%build_version%_%project_name%
if "%PACKAGE_PROJECT%"=="CRANEM" (
	set QuecProduct=CRANEM_QUEC_PRODUCT
	set boot2_image=%soc_platform_path%/%PACKAGE_PROJECT%/images/2020.07.30/boot2.bin
	set preboot_image=%soc_platform_path%/%PACKAGE_PROJECT%/images/2020.07.30/preboot.bin
	set flasher_image=%soc_platform_path%/%PACKAGE_PROJECT%/images/2020.07.30/flasher.bin
	set rd_image=%TOP_DIR%\ql-config\config\%project_name%\ReliableData.bin
	set rfbin_image=%TOP_DIR%\ql-config\config\%project_name%\rf.bin
) else (
	set QuecProduct=CRANEL_QUEC_PRODUCT
	set boot2_image=%TOP_DIR%\ql-config\quec-project\aboot\images\boot2.bin
	set preboot_image=%TOP_DIR%\ql-config\quec-project\aboot\images\preboot.bin
	set flasher_image=%TOP_DIR%\ql-config\quec-project\aboot\images\flasher.bin
	set rd_image=%TOP_DIR%\ql-config\config\%project_name%\%rd_image_type%\ReliableData_SingleSim.bin
	set rfbin_image=%TOP_DIR%\ql-config\config\%project_name%\rf.bin
	set rfbin_1_image=%TOP_DIR%\ql-config\config\%project_name%\rf_1.bin
	set rfbin_2_image=%TOP_DIR%\ql-config\config\%project_name%\rf_2.bin
	rem set customer_backup_fs_image=%TOP_DIR%\ql-config\config\%project_name%\customer_backup_fs.bin
	set customer_backup_fs_image=%TOP_DIR%\ql-config\config\customer_backup_fs.bin
	set gnss_boot_image=%TOP_DIR%\ql-config\config\%project_name%\gnss_boot.pkg
	set gnss_firm_image=%TOP_DIR%\ql-config\config\%project_name%\gnss_firm.pkg
	set jacana_fw_image=%TOP_DIR%\ql-config\quec-project\aboot\images\jacana_fw.bin
	set btbin_image=%TOP_DIR%\ql-config\quec-project\aboot\images\btbin.bin
	set btlst_image=%TOP_DIR%\ql-config\quec-project\aboot\images\btlst.bin
)

set dsp_image=%soc_platform_path%\%PACKAGE_PROJECT%\images\DSP_Reduce_SuLog\dsp.bin
rem set customer_fs_image=%TOP_DIR%\ql-config\config\%project_name%\customer_fs.bin
set customer_fs_image=%TOP_DIR%\ql-config\config\customer_fs.bin
set logo_image=%TOP_DIR%\ql-config\quec-project\aboot\images\logo.bin

rem set cp_image=%TOP_DIR%\ql-config\quec-project\aboot\images\release_kernel.bin
set cp_image=%TOP_DIR%\ql-config\config\%project_name%\release_kernel.bin
set boot_image=%TOP_DIR%\ql-config\quec-project\aboot\images\boot33.bin
set boot_nologo_image=%TOP_DIR%\ql-config\quec-project\aboot\images\boot33_NoLogo.bin
set customer_app_image=%TOP_DIR%\ql-config\quec-project\aboot\images\app.bin
set updater_image=%TOP_DIR%\ql-config\quec-project\aboot\images\updater.bin

if exist %TOP_DIR%\ql-bootloader\boot33\release (
	set boot_image=%TOP_DIR%\ql-bootloader\boot33\release\boot33.bin
)

if exist %TOP_DIR%\ql-application\threadx\build (
	set customer_app_image=%TOP_DIR%\ql-application\threadx\build\app.bin
	rem %TOP_DIR%\ql-cross-tool\win32\host\tools\lzma.exe ds 18 e %TOP_DIR%\ql-application\threadx\build\app.bin %TOP_DIR%\ql-application\threadx\build\app_lzma.bin
	rem set customer_app_image=%TOP_DIR%\ql-application\threadx\build\app_lzma.bin
)

rem only use for ota customer_app.bin
if exist %TOP_DIR%\ql-application\threadx\build (
	copy /Y %customer_app_image% %TOP_DIR%\ql-application\threadx\build\customer_app.bin
)

if exist %TOP_DIR%\ql-kernel\threadx\build (
	set cp_image=%TOP_DIR%\ql-kernel\threadx\build\release_kernel.bin
)

set aboot_dir=%TOP_DIR%\ql-cross-tool\win32\host\tools\aboot-tools-win-x86
set ImageInfoGet=%TOP_DIR%\ql-cross-tool\win32\host\tools\ImageInfoGet
set AddCheck=%TOP_DIR%\ql-cross-tool\win32\host\tools\AddCheck
set cmd_7z=%TOP_DIR%\ql-cross-tool\win32\host\tools\7z\7z

if not exist %TOP_DIR%\target mkdir %TOP_DIR%\target
rd /q/s %TOP_DIR%\target

if "%FLASHTYPE%"=="HX_GPS"  (
	set QuecProductName=QUEC_CRANEL_DS_08MB_HX_GNSS_OPEN
	set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%"
) else if "%FLASHTYPE%"=="ASR_GPS" (
	set QuecProductName=QUEC_CRANEL_DS_08MB_ASR_GNSS_OPEN
	set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,jacana_fw=%jacana_fw_image%"
) else if "%FLASHTYPE%"=="HX_GPS_BT" (
	set QuecProductName=QUEC_CRANEL_DS_08MB_HX_GNSS_BT_OPEN
	set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,gnss_boot=%gnss_boot_image%,gnss_firm=%gnss_firm_image%,btbin=%btbin_image%,btlst=%btlst_image%"
) else if "%FLASHTYPE%"=="DEFAULT_BT" (
	set QuecProductName=QUEC_CRANEL_DS_08MB_BT_OPEN
	set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,btbin=%btbin_image%,btlst=%btlst_image%"
) else if "%FLASHTYPE%"=="ASR_GPS_BT" (
	set QuecProductName=QUEC_CRANEL_DS_08MB_ASR_GNSS_BT_OPEN
	set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,jacana_fw=%jacana_fw_image%,btbin=%btbin_image%,btlst=%btlst_image%"
) else (
	if "%PACKAGE_PROJECT%"=="CRANEM" (
		set QuecProductName=QUEC_CRANEM_SS_08M_08MB_OPEN
		set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,flasher_bin=%flasher_image%"
	) else (
		set QuecProductName=QUEC_CRANEL_DS_08M_08MB_OPEN
		set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_nologo_image%,updater=%updater_image%,rfbin=%rfbin_image%,rd=%rd_image%,customer_app=%customer_app_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%"
	)
	
)
echo "========================="
echo QuecProductName=%QuecProductName%
echo standard_arelease_iamges=%standard_arelease_iamges%
echo "========================="
echo %QuecProductName% >%TOP_DIR%\ql-config\quec-project\aboot\config\product\QuecProductName.txt
set standard_package_path=%TOP_DIR%\target\%buildver_name%
set standard_package_file=%buildver_name%_%BUILD_TIME%.zip
mkdir %standard_package_path%
%aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p %QuecProduct% -v %QuecProductName% -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%
rem echo "========================="
rem echo %aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p %QuecProduct% -v %QuecProductName% -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%
rem echo "========================="
if not exist %standard_package_path%\%standard_package_file% (
	echo =====================ERROR!!!=======================
	echo   ********  *****     *****       ***     *****
	echo   ********  **  **    **  **     ** **    **  **
	echo   **        **   **   **   **   **   **   **   **
	echo   ********  **  **    **  **    **   **   **  **
	echo   ********  *****     *****     **   **   *****
	echo   **        **  **    **  **    **   **   **  **		 
	echo   ********  **   **   **   **    ** **    **   **
	echo   ********  **    **  **    **    ***     **    ** 
	echo ====================================================
	goto end
)

copy /y %TOP_DIR%\ql-config\quec-project\download\quec_download.json %standard_package_path%\%buildver%.json
%ImageInfoGet% 1 0 1 %cp_image% %standard_package_path%\imageinfo.bin
%ImageInfoGet% 2 0 1 %dsp_image% %standard_package_path%\imageinfo.bin
%ImageInfoGet% 3 0 1 %rfbin_image% %standard_package_path%\imageinfo.bin
%AddCheck% %standard_package_path%\imageinfo.bin

mkdir %standard_package_path%\DBG
if exist %TOP_DIR%\ql-kernel\threadx\build (
	copy /Y %TOP_DIR%\ql-kernel\threadx\build\kernel.axf %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-kernel\threadx\build\kernel.map %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-kernel\threadx\build\KERNEL_MDB.txt %standard_package_path%\DBG
) else (
	copy /Y %TOP_DIR%\ql-config\config\%project_name%\kernel.axf %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-config\config\%project_name%\kernel.map %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-config\config\%project_name%\KERNEL_MDB.txt %standard_package_path%\DBG
rem	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\kernel.axf %standard_package_path%\DBG
rem	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\kernel.map %standard_package_path%\DBG
rem	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\KERNEL_MDB.txt %standard_package_path%\DBG
)
::rename %standard_package_path%\DBG\kernel.axf CRANE_DS_XIP_DM_GENERIC.axf
::rename %standard_package_path%\DBG\kernel.map CRANE_DS_XIP_DM_GENERIC.map

if exist %TOP_DIR%\ql-application\threadx\build (
	copy /Y %TOP_DIR%\ql-application\threadx\build\app.elf %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-application\threadx\build\app.map %standard_package_path%\DBG
) else (
	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\app.elf %standard_package_path%\DBG
	copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\app.map %standard_package_path%\DBG
)

::记录每次使用的feedback到DBG中
if exist %TOP_DIR%\ql-kernel\threadx\config\common\feedbackLink_backup.txt (
	copy /Y %TOP_DIR%\ql-kernel\threadx\config\common\feedbackLink_backup.txt %standard_package_path%\DBG
	del %TOP_DIR%\ql-kernel\threadx\config\common\feedbackLink_backup.txt
)

::rename %standard_package_path%\DBG\app.elf CRANE_DS_XIP_DM_GENERIC_APP.axf
::rename %standard_package_path%\DBG\app.map CRANE_DS_XIP_DM_GENERIC_APP.map

if "%OPTION%" == "NODBG" (
	echo "build firmware without dbg compress"
    goto build_end
)

%cmd_7z% a -tzip  %TOP_DIR%\target\%standard_package_file% -r %standard_package_path%\*.*

:build_end
echo   ============================SUCCESS=================================
echo       **    **     **     ******  ******  ********     **       **
echo     **  **  **     **    **      **       **         **  **   **  **
echo    **       **     **   **      **        **        **       **
echo    **       **     **  **      **         **        **       **
echo     **      **     **  **      **         ********   **       **
echo       **    **     **  **      **         **           **      **
echo        **   **     **   **      **        **            **       **
echo    **  **    **   **     **      **       **        **  **   **  **
echo     ***       *****       ******  ******  ********   ***      ***
echo   ====================================================================
:end
