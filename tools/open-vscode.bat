@echo off
REM Launches VS Code with the Visual Studio 2022 Build Tools compiler
REM environment (PATH/INCLUDE/LIB) already loaded, so CMake Tools can find
REM cl.exe and the Windows SDK libs without needing a matching "kit".
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cd /d D:\AI_C++
code .
