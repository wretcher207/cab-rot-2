@echo off
REM Removes the stale system-wide Cab Rot.vst3 that shadows the fresh user-scope build.
REM Self-elevates because C:\Program Files is admin-protected.

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Requesting administrator rights...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

set "STALE=C:\Program Files\Common Files\VST3\Cab Rot.vst3"

if exist "%STALE%" (
    echo Removing stale system copy:
    echo   %STALE%
    rmdir /s /q "%STALE%"
    if exist "%STALE%" (
        echo FAILED to remove. Close any DAW using the plugin and try again.
    ) else (
        echo Removed.
    )
) else (
    echo No stale system copy found - nothing to do.
)

echo.
echo Next: in Reaper, Options ^> Preferences ^> Plug-ins ^> VST ^> Re-scan,
echo then re-insert Cab Rot. It should show the full UI.
echo.
pause
