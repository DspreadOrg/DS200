
@echo off

del /s /q /f fbf.bin
rem del /s /q /f a\patchfolder\*
FBFMake_CF_V1.6-150.exe -o fbf.bin -f config_full_app -a full -b full
