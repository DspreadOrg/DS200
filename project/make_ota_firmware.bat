@echo off
set CURDIR=%~dp0

if exist %CURDIR%\ql-application\threadx\build\app.bin (
	set app_path=%CURDIR%\ql-application\threadx\build
)else (
	echo 请先编译再制作OTA固件
    goto make_ota_end
)


if exist %CURDIR%\ql-cross-tool\FBFMake_CF (
	set ota_tool_path=%CURDIR%\ql-cross-tool\FBFMake_CF
)else (
	echo 请先将FBFMake_CF工具拷贝到%CURDIR%\ql-cross-tool目录
    goto make_ota_end
)

copy /Y %app_path%\customer_app.bin %ota_tool_path%\full\customer_app.bin

pushd ql-cross-tool\FBFMake_CF\
call fbfmake_full_app.bat
popd

if exist %CURDIR%\ql-cross-tool\FBFMake_CF\fbf.bin (
	set ota_fw_path=%CURDIR%\ql-cross-tool\FBFMake_CF
)else (
	echo OTA固件制作失败
    goto make_ota_end
)

if exist %CURDIR%\project_name.conf (
	set /p build_version=<%CURDIR%\project_name.conf
)else (
	set build_version=FW_4GW
)
set buildver_name=%build_version%_OTA.bin


move /y %ota_fw_path%\fbf.bin %buildver_name%
echo OTA固件制作成功

:make_ota_end
echo finish