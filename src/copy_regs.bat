REM ########################################################
REM #### Copy nessisary files and call setup package bat ###
REM ########################################################

@echo off
@if not "%2" == "" (
	set sub_dir=_%2
)
REM #######################################################
if "%1" == "i386_debug" goto i386_debug
if "%1" == "i386_release" goto i386_release
if "%1" == "x64_debug" goto x64_debug
if "%1" == "x64_release" goto x64_release
goto GM_PARAMS_ERROR

:i386_debug

copy ..\obj\gimureg\i386\debug%sub_dir%\*.mo ..\exe\i386\debug%sub_dir%

if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:i386_release

copy ..\obj\gimureg\i386\release%sub_dir%\*.mo ..\exe\i386\release%sub_dir%
if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:x64_debug

copy ..\obj\gimureg\x64\debug%sub_dir%\*.mo ..\exe\x64\debug%sub_dir%

if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:x64_release
copy ..\obj\gimureg\x64\release%sub_dir%\*.mo ..\exe\x64\release%sub_dir%

if errorlevel 1 goto GMERROREND
goto GMEND

:GM_PARAMS_ERROR
goto GMEND

:GMERROREND

echo Some command error, please check it.

exit 1

:GMEND
