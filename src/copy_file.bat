REM ########################################################
REM #### Copy nessisary files and call setup package bat ###
REM ########################################################

@echo off

REM #######################################################
if "%1" == "i386_debug" goto i386_debug
if "%1" == "i386_release" goto i386_release
if "%1" == "x64_debug" goto x64_debug
if "%1" == "x64_release" goto x64_release
goto GM_PARAMS_ERROR

:i386_debug
copy ..\deps\ace\debug_i386\*.dll  ..\exe\i386\debug
copy ..\deps\ace\debug_i386\*.dll .\

copy ..\deps\wx\debug_i386\*.dll ..\exe\i386\debug
copy ..\deps\wx\debug_i386\*.dll .\

copy ..\obj\gimu\i386\debug\*.mo ..\exe\i386\debug
copy ..\obj\gimu\i386\debug\*.mo .\

copy ..\obj\engine\i386\debug\*.mo ..\exe\i386\debug
copy ..\obj\engine\i386\debug\*.mo .\

copy ..\exe\i386\debug\engine.dll .\

if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:i386_release
copy ..\deps\ace\release_i386\*.dll  ..\exe\x64\release
copy ..\deps\ace\release_i386\*.dll .\

copy ..\deps\wx\release_i386\*.dll ..\exe\x64\release
copy ..\deps\wx\release_i386\*.dll .\

copy ..\obj\gimu\i386\release\*.mo ..\exe\i386\release
copy ..\obj\gimu\i386\release\*.mo .\

copy ..\obj\engine\i386\release\*.mo ..\exe\i386\release
copy ..\obj\engine\i386\release\*.mo .\

copy ..\exe\i386\release\engine.dll .\

call .\make_pakage.bat i386
if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:x64_debug
copy ..\deps\ace\debug_x64\ACEd.dll  ..\exe\x64\debug
copy ..\deps\ace\debug_x64\ACEd.dll .\

copy ..\deps\wx\debug_x64\*.dll ..\exe\x64\debug
copy ..\deps\wx\debug_x64\*.dll .\

copy ..\obj\gimu\x64\debug\*.mo ..\exe\x64\debug
copy ..\obj\gimu\x64\debug\*.mo .\

copy ..\obj\engine\x64\debug\*.mo ..\exe\x64\debug
copy ..\obj\engine\x64\debug\*.mo .\

copy ..\exe\x64\debug\engine.dll .\

if errorlevel 1 goto GMERROREND
goto GMEND

REM #######################################################
:x64_release
copy ..\deps\ace\release_x64\ACEd.dll  ..\exe\x64\release
copy ..\deps\ace\release_x64\ACEd.dll .\

copy ..\deps\wx\release_x64\*.dll ..\exe\x64\release
copy ..\deps\wx\release_x64\*.dll .\

copy ..\obj\gimu\x64\release\*.mo ..\exe\x64\release
copy ..\obj\gimu\x64\release\*.mo .\

copy ..\obj\engine\x64\release\*.mo ..\exe\x64\release
copy ..\obj\engine\x64\release\*.mo .\
copy ..\exe\x64\release\engine.dll .\

call .\make_pakage.bat x64
if errorlevel 1 goto GMERROREND
goto GMEND

:GM_PARAMS_ERROR
goto GMEND

:GMERROREND

echo Some command error, please check it.

exit 1

:GMEND
