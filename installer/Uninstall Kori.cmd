@echo off
setlocal
title Kori Uninstaller
cls

echo Kori Uninstaller
echo =================
echo.

if not exist "%~dp0Uninstall-Kori.ps1" goto :incomplete
if not exist "%~dp0Get-Kori-Version.ps1" goto :incomplete

set /a existing_files=0
if exist "C:\Program Files\obs-studio\obs-plugins\64bit\kori.dll" set /a existing_files+=1
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\locale\en-US.ini" set /a existing_files+=1
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\effects\kori-motion-blur.effect" set /a existing_files+=1
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\version.txt" set /a existing_files+=1
if "%existing_files%"=="0" goto :not_installed
set "installed_version=Unknown"
for /f "usebackq delims=" %%V in (`powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Get-Kori-Version.ps1"`) do set "installed_version=%%V"

echo This will remove the Kori plugin files from OBS Studio.
echo.
echo Installed version: %installed_version%
echo Kori files found: %existing_files% of 4
echo.
echo Planned changes:
if exist "C:\Program Files\obs-studio\obs-plugins\64bit\kori.dll" (echo   [REMOVE] Kori plugin) else (echo   [MISSING] Kori plugin)
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\locale\en-US.ini" (echo   [REMOVE] English interface text) else (echo   [MISSING] English interface text)
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\effects\kori-motion-blur.effect" (echo   [REMOVE] Motion blur effect) else (echo   [MISSING] Motion blur effect)
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\version.txt" (echo   [REMOVE] Installed version information) else (echo   [MISSING] Installed version information)
echo.
echo Files will be removed from:
echo C:\Program Files\obs-studio
echo.
echo Your OBS scenes and saved Kori settings will not be removed.
echo OBS Studio must be closed before continuing.
echo.
choice /C YN /N /M "Proceed with these changes? [Y/N]: "
if errorlevel 2 (
    echo.
    echo Uninstall cancelled. No files were changed.
    exit /b 0
)

echo.
echo Requesting administrator permission...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Uninstall-Kori.ps1"
set "uninstall_exit=%errorlevel%"
if not "%uninstall_exit%"=="0" goto :failed

echo.
echo Kori was removed successfully.
echo Your OBS scenes and saved Kori settings remain available.
echo.
echo Press any key to close the uninstaller.
pause >nul
exit /b 0

:not_installed
echo.
echo Kori plugin files were not found in the standard OBS folder.
echo Kori may already be uninstalled.
echo.
echo Press any key to close the uninstaller.
pause >nul
exit /b 0

:incomplete
echo This uninstaller is incomplete.
echo.
echo Extract the complete Kori ZIP, then run Uninstall Kori.cmd again.
echo.
echo Press any key to close the uninstaller.
pause >nul
exit /b 1

:failed
echo.
echo Kori could not be removed.
echo.
echo Make sure OBS Studio is closed, then try again.
echo If the problem continues, contact kori.dev@askq.co.nz.
echo.
echo Press any key to close the uninstaller.
pause >nul
exit /b 1
