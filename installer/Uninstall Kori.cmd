@echo off
net session >nul 2>&1
if %errorlevel% neq 0 (
    powershell -NoProfile -Command "Start-Process -Verb RunAs -FilePath '%ComSpec%' -ArgumentList '/c','\"%~f0\"'"
    exit /b
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Uninstall-Kori.ps1"
echo.
pause
