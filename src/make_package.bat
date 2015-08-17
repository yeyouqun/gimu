REM ########################################################
REM #### make the setup file for i386 and or x64         ###
REM ########################################################

if "%1" == "i386" goto make_i386
if "%1" == "x64" goto make_x64

REM ################### make i386 release setup file #######
:make_i386
hhc .\help\gimu.hhp

if "%2" == "pro" goto gimu32_pro
if "%2" == "enterprise"  goto gimu32_enterprise
if "%2" == "" goto gimu32_free

:gimu32_pro
iscc "./setup/gimu32_pro.iss"
if errorlevel 1 goto GMERROREND
goto GMEND

:gimu32_enterprise
iscc "./setup/gimu32_enterprise.iss"
if errorlevel 1 goto GMERROREND
goto GMEND

:gimu32_free
iscc "./setup/gimu32.iss"
if errorlevel 1 goto GMERROREND
goto GMEND

REM ################### make x64 release setup file #######
:make_x64
hhc .\help\gimu.hhp

if "%2" == "pro" goto gimu64_pro
if "%2" == "enterprise" gimu64_enterprise
if "%2" == "" gimu64_free

:gimu64_pro
iscc "./setup/gimu64_pro.iss"
if errorlevel 1 goto GMERROREND
goto GMEND

:gimu64_enterprise
iscc "./setup/gimu64_enterprise.iss"
if errorlevel 1 goto GMERROREND
goto GMEND

:gimu64_free
iscc "./setup/gimu64.iss"
if errorlevel 1 goto GMERROREND
goto GMEND

REM ########################################################
goto ERROR_PARAMS
:ERROR_PARAMS

echo the first parameters must be i386 or x64
goto GMEND

:GMERROREND
echo call command error.

:GMEND