
@echo off

del /s /q /f app_sys_fbf_dfota.bin
del /s /q /f a\patchfolder\*
FBFMake_CF_V1.6-150.exe -f config_app_system -d 0x10000 -a new -b old -o app_sys_fbf_dfota.bin -q
