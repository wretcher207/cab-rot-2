@echo off
REM Configure + build Cab Rot with the Visual Studio 2022 Build Tools that are
REM installed on this machine but not on PATH. An earlier session concluded
REM "no toolchain available" from four `where` lookups; MSVC and CMake were
REM both present, just not exposed. This script makes the real build path
REM reachable and reproducible.
setlocal
set VS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools
set CMAKE=%VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
set NINJA=%VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe

call "%VS%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
  echo FAILED: vcvars64.bat
  exit /b 1
)

echo === cl ===
cl 2>&1 | findstr /i "Version"
echo === cmake ===
"%CMAKE%" --version 2>&1 | findstr /i "version"

if "%1"=="configure" (
  echo === configure ===
  "%CMAKE%" -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
  exit /b %ERRORLEVEL%
)

if "%1"=="build" (
  echo === build %2 ===
  "%CMAKE%" --build build --config Release --target %2 --parallel
  exit /b %ERRORLEVEL%
)

echo usage: build-msvc.cmd [configure^|build TARGET]
exit /b 2
