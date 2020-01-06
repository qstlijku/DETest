@echo off
setlocal enabledelayedexpansion

tools\win\premake5 --file=code/premake5.lua vs2019

for /f "usebackq tokens=*" %%i in (`%cd%/tools/win/vswhere.exe -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBUILDDIR=%%i
)

"%MSBUILDDIR%" "%cd%/build/DisruptEditor.sln" /p:Configuration=Release

7z a DisruptEditor.zip .\bin\Release\*
