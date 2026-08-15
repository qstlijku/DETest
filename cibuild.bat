@echo off
setlocal enabledelayedexpansion

tools\win\premake5 --file=code/premake5.lua vs2022
if errorlevel 1 (
  echo premake5 failed
  exit /b 1
)

rem -products * so the standalone Build Tools SKU is found, not just the full
rem Visual Studio installs. VS2022 keeps MSBuild under MSBuild\Current\Bin.
for /f "usebackq tokens=*" %%i in (`%cd%/tools/win/vswhere.exe -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set MSBUILDDIR=%%i
)

if not defined MSBUILDDIR (
  echo Could not locate MSBuild. Install Visual Studio 2022 or the VS2022 Build Tools
  echo with the "Desktop development with C++" workload.
  exit /b 1
)

"%MSBUILDDIR%" "%cd%/build/DisruptEditor.sln" /p:Configuration=Release /m
if errorlevel 1 (
  echo build failed
  exit /b 1
)

where 7z >nul 2>&1
if errorlevel 1 (
  echo 7z not on PATH, skipping archive step
  exit /b 0
)

7z a DisruptEditor.zip .\bin\Release\*
