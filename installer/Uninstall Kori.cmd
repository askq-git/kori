@echo off
setlocal
title Kori Uninstaller
cls

echo Kori Uninstaller
echo =================
echo.

if not exist "%~dp0Uninstall-Kori.ps1" goto :incomplete

if not exist "C:\Program Files\obs-studio\obs-plugins\64bit\kori.dll" if not exist "C:\Program Files\obs-studio\data\obs-plugins\kori\locale\en-US.ini" goto :not_installed

echo This will remove the Kori plugin files from OBS Studio.
echo.
echo Files will be removed from:
echo C:\Program Files\obs-studio
echo.
echo Your OBS scenes and saved Kori settings will not be removed.
echo OBS Studio must be closed before continuing.
echo.
choice /C YN /N /M "Do you want to uninstall Kori? [Y/N]: "
if errorlevel 2 goto :cancelled

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

:cancelled
echo.
echo Uninstall cancelled. No files were changed.
timeout /t 2 >nul
exit /b 0
