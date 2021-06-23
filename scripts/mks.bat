@ECHO OFF
REM Danmakufu shader maker
REM Usage: (shaderc) (vs/fs/cs) (name) (in_dir) (out_dir) (bgfx_dir) (pe)

IF NOT EXIST "%~5" MD "%~5"
IF NOT EXIST "%~5\hlsl3" MD "%~5\hlsl3"
IF NOT EXIST "%~5\hlsl5" MD "%~5\hlsl5"
IF NOT EXIST "%~5\spirv" MD "%~5\spirv"
IF NOT EXIST "%~5\metal" MD "%~5\metal"
IF NOT EXIST "%~5\glsl" MD "%~5\glsl"
IF NOT EXIST "%~5\essl" MD "%~5\essl"
REM IF NOT EXIST "%~5\nacl" MD "%~5\nacl"
REM IF NOT EXIST "%~5\pssl" MD "%~5\pssl"
IF NOT EXIST "%~5\hlsl3\%~7" MD "%~5\hlsl3\%~7"
IF NOT EXIST "%~5\hlsl5\%~7" MD "%~5\hlsl5\%~7"
IF NOT EXIST "%~5\spirv\%~7" MD "%~5\spirv\%~7"
IF NOT EXIST "%~5\metal\%~7" MD "%~5\metal\%~7"
IF NOT EXIST "%~5\glsl\%~7" MD "%~5\glsl\%~7"
IF NOT EXIST "%~5\essl\%~7" MD "%~5\essl\%~7"
REM IF NOT EXIST "%~5\nacl\%~7" MD "%~5\nacl\%~7"
REM IF NOT EXIST "%~5\pssl\%~7" MD "%~5\pssl\%~7"

CALL :execcomp %1 %2 %3 %4 %5 %6 %7
GOTO quit


REM (shaderc) (rendertype) (vs/fs/cs) (name) (in) (out) (bgfx) (pe)
:comp

REM ms specific flags ...
IF "%3"=="vs" (
	SET D3DX11=-p vs_5_0 -O 3
	SET D3DX9=-p vs_3_0 -O 3
) ELSE IF "%3"=="fs" (
	SET D3DX11=-p ps_5_0 -O 3
	SET D3DX9=-p ps_3_0 -O 3
) ELSE IF "%3"=="cs" (
	SET D3DX11=-p cs_5_0 -O 1
	SET D3DX9=
)
	
IF "%2"=="0" (
	SET ARGS=--platform windows %D3DX11%
	SET SDIR=hlsl5
) ELSE IF "%2"=="1" (
	SET ARGS=--platform windows %D3DX9%
	SET SDIR=hlsl3
) ELSE IF "%2"=="2" (
	SET ARGS=--platform linux -p 120
	SET SDIR=glsl
) ELSE IF "%2"=="3" (
	SET ARGS=--platform android
	SET SDIR=essl
) ELSE IF "%2"=="4" (
	SET ARGS=--platform osx -p metal
	SET SDIR=metal
) ELSE IF "%2"=="5" (
	SET ARGS=--platform linux -p spirv
	SET SDIR=spirv
) ELSE IF "%2"=="6" (
	SET ARGS=--platform orbis -p pssl
	SET SDIR=pssl
) ELSE IF "%2"=="7" (
	SET ARGS=--platform nacl
	SET SDIR=nacl
)

"%~1" %ARGS% --type %3 -i "%~7\src" -i "%~7\examples\common" -f "%~5\%~8\%~4.%3" -o "%~6\%SDIR%\%~8\%~4_%~3.bin" --varyingdef "%~5\%~8\%~4.def"
EXIT /B %ERRORLEVEL%

:execcomp
CALL :comp %1 0 %2 %3 %4 %5 %6 %7
CALL :comp %1 1 %2 %3 %4 %5 %6 %7
CALL :comp %1 2 %2 %3 %4 %5 %6 %7
CALL :comp %1 3 %2 %3 %4 %5 %6 %7
CALL :comp %1 4 %2 %3 %4 %5 %6 %7
CALL :comp %1 5 %2 %3 %4 %5 %6 %7
REM CALL :comp %1 6 %2 %3 %4 %5 %6 %7
REM CALL :comp %1 7 %2 %3 %4 %5 %6 %7
EXIT /B %ERRORLEVEL%

:quit
EXIT /B %ERRORLEVEL%
