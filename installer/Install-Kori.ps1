param(
    [string]$ObsPath = "C:\Program Files\obs-studio"
)

$ErrorActionPreference = "Stop"

function Test-IsAdministrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-IsAdministrator)) {
    $arguments = @(
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-File", "`"$PSCommandPath`"",
        "-ObsPath", "`"$ObsPath`""
    )

    try {
        $process = Start-Process `
            -FilePath "powershell.exe" `
            -Verb RunAs `
            -ArgumentList $arguments `
            -Wait `
            -PassThru
        exit $process.ExitCode
    }
    catch {
        throw "Administrator permission is required to install Kori. $($_.Exception.Message)"
    }
}

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
