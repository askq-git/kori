@echo off
setlocal
title Kori Installer
cls

echo Kori Installer
echo ==============
echo.

if not exist "%~dp0Install-Kori.ps1" goto :not_extracted
if not exist "%~dp0Get-Kori-Version.ps1" goto :not_extracted
if not exist "%~dp0VERSION.txt" goto :not_extracted
if not exist "%~dp0plugin\kori.dll" goto :not_extracted

echo This will install Kori for OBS Studio.
echo.
set /a existing_files=0
if exist "C:\Program Files\obs-studio\obs-plugins\64bit\kori.dll" set /a existing_files+=1
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\locale\en-US.ini" set /a existing_files+=1
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\effects\kori-motion-blur.effect" set /a existing_files+=1
set "installed_version=Unknown"
for /f "usebackq delims=" %%V in (`powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Get-Kori-Version.ps1"`) do set "installed_version=%%V"
set /p package_version=<"%~dp0VERSION.txt"

if "%existing_files%"=="0" (
    echo Install type: Fresh installation
) else if "%existing_files%"=="3" (
    echo Install type: Upgrade or repair
    echo Existing Kori files found: 3 of 3
) else (
    echo Install type: Repair incomplete installation
    echo Existing Kori files found: %existing_files% of 3
)
echo Current version: %installed_version%
echo Installing version: %package_version%
echo.
echo Planned changes:
if exist "C:\Program Files\obs-studio\obs-plugins\64bit\kori.dll" (echo   [REPLACE] Kori plugin) else (echo   [ADD]     Kori plugin)
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\locale\en-US.ini" (echo   [REPLACE] English interface text) else (echo   [ADD]     English interface text)
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\effects\kori-motion-blur.effect" (echo   [REPLACE] Motion blur effect) else (echo   [ADD]     Motion blur effect)
if exist "C:\Program Files\obs-studio\data\obs-plugins\kori\version.txt" (echo   [REPLACE] Installed version information) else (echo   [ADD]     Installed version information)
echo.
echo Files will be copied from:
echo %~dp0plugin
echo.
echo Files will be installed to:
echo C:\Program Files\obs-studio
echo.
echo OBS Studio must be closed before continuing.
echo.
echo Your OBS scenes, hotkeys and saved Kori settings will not be changed.
echo.
choice /C YN /N /M "Proceed with these changes? [Y/N]: "
if errorlevel 2 (
    echo.
    echo Installation cancelled. No files were changed.
    exit /b 0
)

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
if exist "%~dp0Kori-install-error.txt" (
    echo Installer details:
    echo ------------------
    type "%~dp0Kori-install-error.txt"
    echo ------------------
    echo.
    echo Copy the details above if you need help.
) else (
    echo No detailed installer report was created.
)
echo Make sure OBS Studio is closed, then try again.
echo If the problem continues, contact kori.dev@askq.co.nz.
echo.
echo Press any key to close the installer.
pause
exit /b 1
