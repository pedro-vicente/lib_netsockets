@echo off
if not defined DevEnvDir (
 call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
 if errorlevel 1 goto :eof
)
set MSVC_VERSION="Visual Studio 16 2019"
echo using %MSVC_VERSION%

set root=%cd%
:: replace the character string '\' with '/' needed for cmake
set root_cmake=%root:\=/%
echo cmake root is %root_cmake%

rm -rf CMakeCache.txt CMakeFiles
cmake -H. -Bbuild -G "Visual Studio 16 2019" ^
-A x64 ^
-G %MSVC_VERSION% 
msbuild %root%\build\lib_netsockets.sln /target:build /property:configuration=debug /property:Platform=x64 /nologo /verbosity:minimal