@echo off
set compilerFlags=/Zi /fp:fast /arch:AVX2 -DPLATFORM_WIN32=1

IF "%~1"=="-d" (
    :: Debug build
    set compilerFlags=/Od -DSINGLE_THREAD=1 %compilerFlags%
) ELSE (
    set compilerFlags=/O2 %compilerFlags%
)

pushd .\bin\
cl %compilerFlags% ..\main.cpp 
IF %ERRORLEVEL%==0 (
    .\main.exe
	start render.bmp
)
popd