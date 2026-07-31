@echo off
setlocal
title Kori Installer
cls

echo Kori Installer
echo ==============
echo.

if not exist "%~dp0Install-Kori.ps1" goto :not_extracted
if not exist "%~dp0plugin\kori.dll" goto :not_extracted

echo This will install Kori for OBS Studio.
echo.
echo Files will be copied from:
echo %~dp0plugin
echo.
echo Files will be installed to:
echo C:\Program Files\obs-studio
echo.
echo OBS Studio must be closed before continuing.
echo.
choice /C YN /N /M "Do you want to install Kori? [Y/N]: "
if errorlevel 2 goto :cancelled

echo.
echo Requesting administrator permission...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install-Kori.ps1"
if errorlevel 1 goto :failed

echo.
echo Kori installed successfully.
echo.
echo Open OBS Studio and select Tools ^> Kori Settings.
echo.
echo Press any key to close the installer.
pause >nul
exit /b 0

:not_extracted
echo Kori must be extracted before it can be installed.
echo.
echo Right-click the downloaded ZIP file, select Extract All,
echo then run Install Kori.cmd from the extracted folder.
echo.
echo Press any key to close the installer.
pause >nul
exit /b 1

:failed
echo.
echo Kori could not be installed.
echo.
echo Make sure OBS Studio is closed, then try again.
echo If the problem continues, contact kori.dev@askq.co.nz.
echo.
echo Press any key to close the installer.
pause
exit /b 1

:cancelled
echo.
echo Installation cancelled. No files were changed.
timeout /t 2 >nul
exit /b 0
