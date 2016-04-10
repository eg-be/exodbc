@echo off

rem Use as Post-Build step like:
rem call "$(ProjectDir)\CopyCreateScripts.bat" "$(ProjectDir)" "$(TargetDir)"
rem to copy all directories in $(ProjectDir)\sql to directory $(TargetDir)\CreateScripts

echo Deleting old Scripts..
rd /S /Q %2\CreateScripts

echo Copying Create Scripts..
xcopy /E /Y %1\sql\* %2\CreateScripts\
