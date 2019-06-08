
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

:build_ssl
if exist %root%\external\openssl-1.1.0g\libssl.lib (
 echo skipping openssl-1.1.0g build
) else (
  echo building openssl-1.1.0g
  pushd external
  pushd openssl-1.1.0g
  perl Configure VC-WIN32 no-asm no-shared --debug --prefix=%root%\ssl 
  nmake
  nmake install
  popd
  popd
)

rm -rf CMakeCache.txt CMakeFiles
cmake -H. -Bbuild -G "Visual Studio 16 2019" ^
-A x64 ^
-G %MSVC_VERSION% ^
-DOPENSSL_INCLUDE:PATH=%root_cmake%/ssl/include ^
-DOPENSSL_LIBRARY:FILE=%root_cmake%/ssl/lib/libssl.lib ^
-DCRYPTO_LIBRARY:FILE=%root_cmake%/ssl/lib/libcrypto.lib
msbuild %root%\build\lib_netsockets.sln /target:build /property:configuration=debug /property:Platform=x64 /nologo /verbosity:minimal