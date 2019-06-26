@echo off
set compilerFlags=/Zi /fp:fast /arch:AVX2 -DPLATFORM_WIN32=1 -DWIN32_GPU

IF "%~1"=="-d" (
    :: Debug build
    set compilerFlags=/Od -DSINGLE_THREAD=1 %compilerFlags%
) ELSE (
    set compilerFlags=/O2 %compilerFlags%
)

pushd ..\bin\
cl %compilerFlags% ..\win32gpu\main_win32.cpp ..\win32gpu\glad.c ..\win32gpu\glad_wgl.c /link user32.lib opengl32.lib Gdi32.lib Kernel32.lib
IF %ERRORLEVEL%==0 (
    .\main_win32.exe
)
popd
