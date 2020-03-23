@echo off
setlocal

set COMPILE_OPTIONS= /FC /W4 /WX /nologo /Zi /std:c++latest /GR- /Od /EHa- /MT /wd4312 /wd4201 /wd4200 /wd4100 /wd4456 /wd4505 /wd4189 /wd4996
set LINK_OPTIONS= /incremental:no
set INCLUDE_PATHS=
set LIBRARY_PATHS=
set LIBRARIES= User32.lib Gdi32.lib Opengl32.lib

pushd "build"
cl %COMPILE_OPTIONS% %INCLUDE_PATHS% /Fe:SnailJumpy.exe ..\src\win32_snail_jumpy.cpp /link  %LINK_OPTIONS% %LIBRARY_PATHS% %LIBRARIES%
popd
