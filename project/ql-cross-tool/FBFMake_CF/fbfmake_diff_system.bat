
@echo off

del /s /q /f sys_fbf_dfota.bin
del /s /q /f a\patchfolder\*
FBFMake_CF_V1.6-150.exe -f config_system -d 0x10000 -a new -b old -o sys_fbf_dfota.bin -q
