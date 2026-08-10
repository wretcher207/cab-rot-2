@echo off
REM Full acceptance run from a clean clone: configure, build every target,
REM run every gate. This is the check that would have caught the original
REM facelift report's unreproducible claims, so it is committed rather than
REM left as something a reviewer has to reconstruct.
REM
REM Usage: tools\acceptance.cmd [path-to-clone]
REM Defaults to the current repo.
setlocal enabledelayedexpansion

set REPO=%~1
if "%REPO%"=="" set REPO=%CD%
cd /d "%REPO%" || exit /b 1

set VS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools
set CMAKE=%VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
set CTEST=%VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe

call "%VS%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1 || (echo FAILED: vcvars & exit /b 1)

if not exist JUCE\modules (
  echo === JUCE missing; fetch it first ===
  echo   curl -L -o juce.tar.gz https://codeload.github.com/juce-framework/JUCE/tar.gz/refs/tags/8.0.12
  echo   then extract as .\JUCE
  exit /b 1
)

echo === configure ===
"%CMAKE%" -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release || exit /b 1

for %%T in (CabRot_DspTest CabRot_PassthroughTest CabRot_EditorTest CabRot_RenderTest CabRot_ThemeTest CabRot_VST3) do (
  echo === build %%T ===
  "%CMAKE%" --build build --config Release --target %%T --parallel || exit /b 1
)

echo === ctest ===
set CABROT_SKIP_CPU_BENCH=1
"%CTEST%" --test-dir build -C Release --output-on-failure || exit /b 1

echo === doc citation gates ===
python tools\verify-doc-citations.py || exit /b 1
python tools\test-verify-exit-codes.py || exit /b 1
python tools\test-harness-selfcheck.py || exit /b 1

echo.
echo ACCEPTANCE OK
exit /b 0
