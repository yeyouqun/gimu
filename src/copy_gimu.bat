REM ########################################################
REM #### Copy nessisary files and call setup package bat ###
REM ########################################################

@echo off

REM #######################################################
@if not "%2" == "" (
	set sub_dir=_%2
)
hhc .\help\gimu.hhp
if "%1" == "i386_debug" goto i386_debug
if "%1" == "i386_release" goto i386_release
if "%1" == "x64_debug" goto x64_debug
if "%1" == "x64_release" goto x64_release
goto GM_PARAMS_ERROR

:i386_debug
copy ..\deps\ace\debug_i386\*.dll  ..\exe\i386\debug%sub_dir%
copy ..\deps\wx\debug_i386\*.dll ..\exe\i386\debug%sub_dir%
copy ..\obj\gimu\i386\debug%sub_dir%\*.mo ..\exe\i386\debug%sub_dir%
copy ..\obj\engine\i386\debug%sub_dir%\*.mo ..\exe\i386\debug%sub_dir%

copy .\help\gimu.chm ..\exe\i386\debug%sub_dir%

if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:i386_release
copy ..\deps\ace\release_i386\*.dll  ..\exe\i386\release%sub_dir%
copy ..\deps\wx\release_i386\*.dll ..\exe\i386\release%sub_dir%
copy ..\obj\gimu\i386\release%sub_dir%\*.mo ..\exe\i386\release%sub_dir%
copy ..\obj\engine\i386\release%sub_dir%\*.mo ..\exe\i386\release%sub_dir%
copy .\help\gimu.chm ..\exe\i386\release%sub_dir%
copy .\license\*.txt ..\exe\i386\release%sub_dir%
call .\make_package.bat i386 %2
if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:x64_debug
copy ..\deps\ace\debug_x64\ACEd.dll  ..\exe\x64\debug%sub_dir%
copy ..\deps\wx\debug_x64\*.dll ..\exe\x64\debug%sub_dir%
copy ..\obj\gimu\x64\debug%sub_dir%\*.mo ..\exe\x64\debug%sub_dir%
copy ..\obj\engine\x64\debug%sub_dir%\*.mo ..\exe\x64\debug%sub_dir%
copy .\help\gimu.chm ..\exe\x64\debug%sub_dir%
if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:x64_release
copy ..\deps\ace\release_x64\ACEd.dll  ..\exe\x64\release%sub_dir%
copy ..\deps\wx\release_x64\*.dll ..\exe\x64\release%sub_dir%
copy ..\obj\gimu\x64\release%sub_dir%\*.mo ..\exe\x64\release%sub_dir%
copy ..\obj\engine\x64\release%sub_dir%\*.mo ..\exe\x64\release%sub_dir%
copy .\help\gimu.chm ..\exe\x64\release%sub_dir%
copy .\license\*.txt ..\exe\x64\release%sub_dir%
call .\make_package.bat x64  %2
if errorlevel 1 goto GMERROREND
goto GMEND

:GM_PARAMS_ERROR
goto GMEND

:GMERROREND

echo Some command error, please check it.

exit 1

:GMEND

