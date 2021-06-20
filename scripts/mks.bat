@ECHO OFF
REM Danmakufu shader maker
REM Usage: (shaderc) (vs/fs/cs) (name) (in_dir) (out_dir) (bgfx_dir)

IF NOT EXIST "%~5" MD "%~5"
IF NOT EXIST "%~5\%~3" MD "%~5\%~3"
CALL :execcomp %1 %2 %3 %4 %5 %6
GOTO quit


REM (shaderc) (n) (vs/fs/cs) (in) (out) (vd)
:comp

REM ms specific flags ...
IF "%3"=="vs" (
	SET D3DX11=-p vs_5_0 -O 3
	SET D3DX9= -p vs_3_0 -O 3
) ELSE IF "%3"=="fs" (
	SET D3DX11=-p ps_5_0 -O 3
	SET D3DX9= -p ps_3_0 -O 3
) ELSE IF "%3"=="cs" (
	SET D3DX11=-p cs_5_0 -O 1
	SET D3DX9=
)
	
IF "%2"=="0" SET ARGS=--platform windows %D3DX11%
IF "%2"=="1" SET ARGS=--platform windows %D3DX9%
IF "%2"=="2" SET ARGS=--platform linux -p 120
IF "%2"=="3" SET ARGS=--platform android
IF "%2"=="4" SET ARGS=--platform osx -p metal
IF "%2"=="5" SET ARGS=--platform linux -p spirv
IF "%2"=="6" SET ARGS=--platform orbis -p pssl
IF "%2"=="7" SET ARGS=--platform nacl
"%~1" %ARGS% --type %3 -i "%~6\src" -i "%~6\examples\common" -f "%~4" -o "%~5" --varyingdef "%~7"
EXIT /B %ERRORLEVEL%

REM (shaderc) (vs/fs/cs) (in_name) (in_dir) (out_dir)
:execcomp
CALL :comp %1 0 %2 "%~4\%~3.%~2" "%~5\%~3\%~2.hlsl5" %6 "%~4\%~3.def"
CALL :comp %1 1 %2 "%~4\%~3.%~2" "%~5\%~3\%~2.hlsl3" %6 "%~4\%~3.def"
CALL :comp %1 2 %2 "%~4\%~3.%~2" "%~5\%~3\%~2.glsl" %6 "%~4\%~3.def"
CALL :comp %1 3 %2 "%~4\%~3.%~2" "%~5\%~3\%~2.essl" %6 "%~4\%~3.def"
CALL :comp %1 4 %2 "%~4\%~3.%~2" "%~5\%~3\%~2.metal" %6 "%~4\%~3.def"
CALL :comp %1 5 %2 "%~4\%~3.%~2" "%~5\%~3\%~2.spirv" %6 "%~4\%~3.def"
REM CALL :comp %1 6 %2 "%~4\%~3.%~2" "%~5\%~3\%~2.pssl" "%~4\%~3.def"
REM CALL :comp %1 7 %2 "%~4\%~3.%2" "%~5\%~3\%~2.nacl" "%~4\%~3.def"
EXIT /B %ERRORLEVEL%

:quit
EXIT /B %ERRORLEVEL%
