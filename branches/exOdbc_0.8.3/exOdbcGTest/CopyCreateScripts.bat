@echo off

echo Deleting old Scripts..
rd /S /Q %2\CreateScripts

echo Copying Create Scripts..
xcopy /E /Y %1\sql\* %2\CreateScripts\
