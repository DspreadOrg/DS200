set TOP_DIR=%1
set OPTION=%2

set buildver=LTE01R06A05_C_SDK_A

set YEAR=%date:~0,4%
set MONTH=%date:~5,2%
set DAY=%date:~8,2%

set BUILD_TIME=%YEAR%%MONTH%%DAY%

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

set buildver_name=%devtype_name%_%build_version%_%project_name%
set PACKAGE_PROJECT=CRANEL

set board_Variant=CRANEL_QUEC_PRODUCT_8M_OPEN

set dsp_image=%soc_platform_path%\%PACKAGE_PROJECT%\dsp\dsp.bin
set boot2_image=%TOP_DIR%\ql-config\quec-project\aboot\images\boot2.bin
set preboot_image=%TOP_DIR%\ql-config\quec-project\aboot\images\preboot.bin
set flasher_image=%TOP_DIR%\ql-config\quec-project\aboot\images\flasher.bin
set rd_image=%TOP_DIR%\ql-config\config\%project_name%\%rd_image_type%\ReliableData.bin
set rfbin_image=%TOP_DIR%\ql-config\config\%project_name%\rf.bin
set rfbin_1_image=%TOP_DIR%\ql-config\config\%project_name%\rf_1.bin
set rfbin_2_image=%TOP_DIR%\ql-config\config\%project_name%\rf_2.bin
set customer_fs_image=%TOP_DIR%\ql-config\config\customer_fs.bin
set customer_backup_fs_image=%TOP_DIR%\ql-config\config\%project_name%\customer_backup_fs.bin
set logo_image=%TOP_DIR%\ql-config\quec-project\aboot\images\logo.bin

rem set cp_image=%TOP_DIR%\ql-config\quec-project\aboot\images\release_kernel.bin
set cp_image=%TOP_DIR%\ql-config\config\%project_name%\release_kernel.bin
set boot_image=%TOP_DIR%\ql-config\quec-project\aboot\images\boot33.bin
set customer_app_image=%TOP_DIR%\ql-config\quec-project\aboot\images\app.bin

if exist %TOP_DIR%\ql-bootloader\boot33\release (
	set boot_image=%TOP_DIR%\ql-bootloader\boot33\release\boot33.bin
)

if exist %TOP_DIR%\ql-application\threadx\build (
	set customer_app_image=%TOP_DIR%\ql-application\threadx\build\app.bin
)

goto nolzma
set cmd_lzma=%TOP_DIR%\ql-cross-tool\win32\host\tools\lzma.exe
if exist %TOP_DIR%\ql-application\threadx\build (
	rem set customer_app_image=%TOP_DIR%\ql-application\threadx\build\app.bin
	%cmd_lzma% ds 18 e %TOP_DIR%\ql-application\threadx\build\app.bin %TOP_DIR%\ql-application\threadx\build\app_lzma.bin
	set customer_app_image=%TOP_DIR%\ql-application\threadx\build\app_lzma.bin
)
rem	echo "%cmd_lzma%"
rem	echo "%customer_app_image%"

:nolzma 
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


if "%project_name%" == "EC800MCN_GA" set res=T

if "%res%"=="T" (
	set board_Variant=CRANEL_QUEC_PRODUCT_8M_GNSS_OPEN
	set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_image%,rfbin=%rfbin_image%,rfbin_1=%rfbin_1_image%,rfbin_2=%rfbin_2_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%,gnss_boot=%TOP_DIR%\ql-config\config\%project_name%\gnss_boot.pkg,gnss_firm=%TOP_DIR%\ql-config\config\%project_name%\gnss_firm.pkg"
)else (
	set standard_arelease_iamges="cp=%cp_image%,dsp=%dsp_image%,boot33=%boot_image%,rfbin=%rfbin_image%,rfbin_1=%rfbin_1_image%,rfbin_2=%rfbin_2_image%,rd=%rd_image%,customer_app=%customer_app_image%,logo=%logo_image%,preboot_bin=%preboot_image%,boot2_bin=%boot2_image%,customer_fs=%customer_fs_image%,customer_backup_fs=%customer_backup_fs_image%,flasher_bin=%flasher_image%"
)

set standard_package_path=%TOP_DIR%\target\%buildver_name%
set standard_package_file=%buildver_name%_%BUILD_TIME%.zip
mkdir %standard_package_path%
echo "=================="
echo "%aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p %board_Variant% -v %PACKAGE_PROJECT%_A0_08MB -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%"
echo "=================="
mkdir %standard_package_path%\HASH
rem %aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p %board_Variant% -v %PACKAGE_PROJECT%_A0_08MB -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file% --stage1 --stage-dir=%standard_package_path%\HASH
%aboot_dir%\arelease -c %TOP_DIR%\ql-config\quec-project\aboot  -g -p %board_Variant% -v %PACKAGE_PROJECT%_A0_08MB -i %standard_arelease_iamges%  %standard_package_path%\%standard_package_file%

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
::rename %standard_package_path%\DBG\app.elf CRANE_DS_XIP_DM_GENERIC_APP.axf
::rename %standard_package_path%\DBG\app.map CRANE_DS_XIP_DM_GENERIC_APP.map

if "%OPTION%" == "NODBG" (
	echo "build firmware without dbg compress"
    goto build_end
)

%cmd_7z% a -tzip  %TOP_DIR%\target\%standard_package_file% -r %standard_package_path%\*.*

:build_end
echo "buid done"

set buildver_cust=%buildver%_CUST
set standard_package_path_cust=%TOP_DIR%\target\%buildver_cust%
set standard_package_file_cust=%buildver_cust%.zip
mkdir %standard_package_path_cust%

Xcopy  %TOP_DIR%\ql-config\quec-project\aboot %standard_package_path_cust%  /y /e /i /q 
rem Xcopy  %TOP_DIR%\ql-config\quec-project\aboot\config %standard_package_path_cust%\config  /y /e /i /q 
rem mkdir %standard_package_path_cust%\images 

if exist %TOP_DIR%\ql-bootloader\boot33\build (
	copy /Y %TOP_DIR%\ql-bootloader\boot33\build\boot33.bin %standard_package_path_cust%\images
)

if exist %TOP_DIR%\ql-application\threadx\build (
	copy /Y %TOP_DIR%\ql-application\threadx\build\app.bin %standard_package_path_cust%\images
	ren %standard_package_path_cust%\images\app.bin customer_app.bin
)

if exist %TOP_DIR%\ql-kernel\threadx\build (
	copy /Y %TOP_DIR%\ql-kernel\threadx\build\release_kernel.bin %standard_package_path_cust%\images\cp.bin
	if exist %standard_package_path_cust%\images\kernel.bin del %standard_package_path_cust%\images\kernel.bin
)

::copy conifg
if exist %standard_package_path_cust%\config\security_cust (
	Xcopy  %TOP_DIR%\ql-config\quec-project\aboot\config\security_cust %standard_package_path_cust%\config\security  /y /e /i /q 
)

if exist %standard_package_path_cust%\config\security_cust rd /s/q %standard_package_path_cust%\config\security_cust

echo "==========6========"
copy /Y %cp_image% %standard_package_path_cust%\images\cp.bin
copy /Y %dsp_image% %standard_package_path_cust%\images
copy /Y %boot_image% %standard_package_path_cust%\images
copy /Y %rfbin_image% %standard_package_path_cust%\images
if exist %rfbin_1_image% copy /Y %rfbin_1_image% %standard_package_path_cust%\images
if exist %rfbin_2_image% copy /Y %rfbin_2_image% %standard_package_path_cust%\images
copy /Y %rd_image% %standard_package_path_cust%\images
copy /Y %customer_app_image% %standard_package_path_cust%\images\customer_app.bin
copy /Y %logo_image% %standard_package_path_cust%\images
copy /Y %preboot_image% %standard_package_path_cust%\images
copy /Y %boot2_image% %standard_package_path_cust%\images
copy /Y %customer_fs_image% %standard_package_path_cust%\images
if exist %customer_backup_fs_image% copy /Y %customer_backup_fs_image% %standard_package_path_cust%\images
copy /Y %flasher_image% %standard_package_path_cust%\images
echo "==========7========"
rem copy /Y %board_Variant% %standard_package_path_cust%\images\cp.bin
rem copy /Y %%PACKAGE_PROJECT%_A0_08MB %standard_package_path_cust%\images\cp.bin

rem copy /Y %soc_platform_path%\%soc_platform%\dsp\%dsp_image_type%\dsp.bin %standard_package_path_cust%\images
rem copy /Y %TOP_DIR%\ql-config\config\%project_name%\rf.bin %standard_package_path_cust%\images
rem copy /Y %TOP_DIR%\ql-config\config\%project_name%\rf_1.bin %standard_package_path_cust%\images
rem copy /Y %TOP_DIR%\ql-config\config\%project_name%\rf_2.bin %standard_package_path_cust%\images
rem copy /Y %TOP_DIR%\ql-config\config\%project_name%\%rd_image_type%\ReliableData.bin %standard_package_path_cust%\images
rem copy /Y %TOP_DIR%\ql-config\quec-project\aboot\images\logo.bin %standard_package_path_cust%\images
rem copy /Y %soc_platform_path%\%soc_platform%\preboot\preboot.bin %standard_package_path_cust%\images
rem copy /Y %soc_platform_path%\%soc_platform%\boot2\boot2.bin %standard_package_path_cust%\images
rem copy /Y %TOP_DIR%\ql-config\config\%project_name%\customer_fs.bin %standard_package_path_cust%\images
rem copy /Y %TOP_DIR%\ql-config\config\%project_name%\customer_backup_fs.bin %standard_package_path_cust%\images
rem copy /Y %soc_platform_path%\%soc_platform%\flasher\flasher.bin %standard_package_path_cust%\images
echo "===========8======="

rem %cmd_7z% a -tzip  %TOP_DIR%\target\%standard_package_file_cust% -r %standard_package_path_cust%\*.*

