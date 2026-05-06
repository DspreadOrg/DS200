@echo off
set CURDIR=%~dp0


if exist %CURDIR%\ql-cross-tool\FBFMake_CF (
	set ota_tool_path=%CURDIR%\ql-cross-tool\FBFMake_CF
)else (
	echo 请先将FBFMake_CF工具拷贝到%CURDIR%\ql-cross-tool目录
	echo Copy the FBFMake_CF tool to %CURDIR%\ql-cross-tool
    goto make_ota_end
)

if exist %CURDIR%\ql-cross-tool\FBFMake_CF\new\system.img (
	echo off
)else (
	echo 请先将system.img拷贝到%CURDIR%\ql-cross-tool\FBFMake_CF\new
	echo Copy system.img to %CURDIR%\ql-cross-tool\FBFMake_CF\new
    goto make_ota_end
)

if exist %CURDIR%\ql-cross-tool\FBFMake_CF\old\system.img (
	echo off
)else (
	echo 请先将system.img拷贝到%CURDIR%\ql-cross-tool\FBFMake_CF\old
	echo Copy system.img to %CURDIR%\ql-cross-tool\FBFMake_CF\old
    goto make_ota_end
)

pushd ql-cross-tool\FBFMake_CF\
call fbfmake_diff_system.bat
popd

if exist %CURDIR%\ql-cross-tool\FBFMake_CF\sys_fbf_dfota.bin (
	set ota_fw_path=%CURDIR%\ql-cross-tool\FBFMake_CF
)else (
	echo OTA固件制作失败
	echo OTA firmware production failed
    goto make_ota_end
)


set buildver_name=SYS_DFOTA.bin


move /y %ota_fw_path%\sys_fbf_dfota.bin %buildver_name%
echo OTA固件制作成功
echo OTA firmware was made successfully
:make_ota_end
echo finish