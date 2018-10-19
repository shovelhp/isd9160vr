@echo off
rem %1: version vx_xx

if not exist "C:\Nuvoton NuVoice Tool\AudioTool\AudioTool.exe" (
echo [WARINING] Can not find AudioTool.exe.
goto END
)

:AUDIOTOOL_BUILD
"C:\Nuvoton NuVoice Tool\AudioTool\AudioTool.exe" -Hide -Build ..\AudioRes\AudioRes.wba
if  ERRORLEVEL 1 goto ERROR

if not exist ..\AudioRes\Output\AudioRes_AudioInfoMerge.ROM (
echo [WARINING] Can not find Audio ROM file.
goto END
)
copy /b ..\AudioRes\Output\AudioRes_AudioInfoMerge.ROM  /b ..\AudioRes_AudioInfoMerge.bin 

:PACK_BINARY_FILES
if exist ..\OneCommandAllMerged.bin (
del /f ..\OneCommandAllMerged.bin
)

.\PackBin.exe ..\\VRModels v
.\PackBin.exe ..\\..\\OneCommand a
rem move ..\..\VtechProject_v1_01AllBinID.h ..\
copy /b ..\..\OneCommandAllBinID.h /b ..\OneCommandAllBinID.h
del /f ..\..\OneCommandAllBinID.h
rem move ..\..\VtechProject_v1_01AllMerged.bin ..\
copy /b ..\..\OneCommandAllMerged.bin /b ..\OneCommandAllMerged.bin
del /f ..\..\OneCommandAllMerged.bin

:END
exit 0

:ERROR
> MessageBox.vbs   Echo Set objArgs = WScript.Arguments 
>> MessageBox.vbs Echo messageText = "Build audio resource faild."  +  vbCrLf + _
>> MessageBox.vbs Echo "Check the output window of keil to see error message."  +  vbCrLf + _
>> MessageBox.vbs Echo "Double click the error message and can navigate to the error line." 
>> MessageBox.vbs Echo MsgBox messageText, vbCritical, "Audio Resource Building Error"  
cscript MessageBox.vbs
rem del /s Messagebox.vbs
exit 1
