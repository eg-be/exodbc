@echo off

rem Use as Post-Build step like:
rem call "$(ProjectDir)\CopyBoostDlls.bat" "$(ProjectDir)" "$(TargetDir)" "$(Configuration)" "$(Platform)"
rem to copy the boost dlls required by the passed build configuration to the target directory.

@echo on & setlocal enabledelayedexpansion

set BOOST_DLL_LIST=(boost_filesystem boost_system)
set BOOST_VERSION=1_59
set TOOLSET=vc140

set PROJECT_DIR=%~1
set TARGET_DIR=%~2
set CONFIGURATION=%~3
set PLATFORM=%~4

for %%i in %BOOST_DLL_LIST% do (
	IF %CONFIGURATION% == Debug_DLL (
	    set FILENAME=%%i-%TOOLSET%-mt-gd-%BOOST_VERSION%.dll
	) ELSE (
	    set FILENAME="%%i-%TOOLSET%-mt-%BOOST_VERSION%.dll"
	)
	if %PLATFORM% == Win32 (
		set BOOST_DLL_DIR=%PROJECT_DIR%\..\3rdParty\boost\stage\lib\x86
	) ELSE (
		set BOOST_DLL_DIR=%PROJECT_DIR%\..\3rdParty\boost\stage\lib\x64
	)

	set FULL_FILE_PATH=!BOOST_DLL_DIR!\!FILENAME!
	xcopy /Y !FULL_FILE_PATH! %TARGET_DIR%
)

