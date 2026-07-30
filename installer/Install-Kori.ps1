param(
    [string]$ObsPath = "C:\Program Files\obs-studio"
)

$ErrorActionPreference = "Stop"

$obsExecutable = Join-Path $ObsPath "bin\64bit\obs64.exe"
if (-not (Test-Path -LiteralPath $obsExecutable)) {
    throw "OBS Studio was not found at '$ObsPath'. Run this script with -ObsPath followed by the correct OBS folder."
}
if (Get-Process -Name "obs64" -ErrorAction SilentlyContinue) {
    throw "OBS Studio is running. Close OBS completely, then run the installer again."
}

$packageRoot = $PSScriptRoot
$sourceDll = Join-Path $packageRoot "plugin\kori.dll"
$sourceLocale = Join-Path $packageRoot "plugin\data\locale\en-US.ini"
if (-not (Test-Path -LiteralPath $sourceDll)) {
    throw "The Kori package is incomplete: plugin\kori.dll is missing."
}

$pluginDirectory = Join-Path $ObsPath "obs-plugins\64bit"
$localeDirectory = Join-Path $ObsPath "data\obs-plugins\kori\locale"
New-Item -ItemType Directory -Force -Path $pluginDirectory | Out-Null
New-Item -ItemType Directory -Force -Path $localeDirectory | Out-Null

Copy-Item -LiteralPath $sourceDll -Destination (Join-Path $pluginDirectory "kori.dll") -Force
Copy-Item -LiteralPath $sourceLocale -Destination (Join-Path $localeDirectory "en-US.ini") -Force

Write-Host ""
Write-Host "Kori installed successfully." -ForegroundColor Green
Write-Host "Start OBS, then look for Kori Settings under Tools."
