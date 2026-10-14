param(
    [string]$ObsPath = "C:\Program Files\obs-studio",
    [string]$LogPath = (Join-Path $PSScriptRoot "Kori-install-error.txt")
)

$ErrorActionPreference = "Stop"

function Test-IsAdministrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

Remove-Item -LiteralPath $LogPath -Force -ErrorAction SilentlyContinue

trap {
    $details = @(
        "Kori installation failed.",
        "Time: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')",
        "OBS folder: $ObsPath",
        "",
        "Reason:",
        $_.Exception.Message
    ) -join [Environment]::NewLine
    Set-Content -LiteralPath $LogPath -Value $details -Encoding utf8
    Write-Host ""
    Write-Host $details -ForegroundColor Red
    Write-Host ""
    Write-Host "A copy was saved to: $LogPath"
    exit 1
}

if (-not (Test-IsAdministrator)) {
    $arguments = @(
        "-NoProfile",
        "-ExecutionPolicy", "Bypass",
        "-File", "`"$PSCommandPath`"",
        "-ObsPath", "`"$ObsPath`"",
        "-LogPath", "`"$LogPath`""
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
$sourceMotionBlur = Join-Path $packageRoot "plugin\data\effects\kori-motion-blur.effect"
$sourceVersion = Join-Path $packageRoot "VERSION.txt"
if (-not (Test-Path -LiteralPath $sourceDll)) {
    throw "The Kori package is incomplete: plugin\kori.dll is missing."
}
if (-not (Test-Path -LiteralPath $sourceLocale)) {
    throw "The Kori package is incomplete: the language file is missing."
}
if (-not (Test-Path -LiteralPath $sourceMotionBlur)) {
    throw "The Kori package is incomplete: the motion blur effect is missing."
}
if (-not (Test-Path -LiteralPath $sourceVersion)) {
    throw "The Kori package is incomplete: VERSION.txt is missing."
}

$pluginDirectory = Join-Path $ObsPath "obs-plugins\64bit"
$localeDirectory = Join-Path $ObsPath "data\obs-plugins\kori\locale"
$effectsDirectory = Join-Path $ObsPath "data\obs-plugins\kori\effects"
New-Item -ItemType Directory -Force -Path $pluginDirectory | Out-Null
New-Item -ItemType Directory -Force -Path $localeDirectory | Out-Null
New-Item -ItemType Directory -Force -Path $effectsDirectory | Out-Null

$installedDll = Join-Path $pluginDirectory "kori.dll"
$installedLocale = Join-Path $localeDirectory "en-US.ini"
$installedMotionBlur = Join-Path $effectsDirectory "kori-motion-blur.effect"
$installedVersion = Join-Path (Split-Path $localeDirectory -Parent) "version.txt"
$files = @(
    @{ Name = "Kori plugin"; Source = $sourceDll; Destination = $installedDll },
    @{ Name = "English interface text"; Source = $sourceLocale; Destination = $installedLocale },
    @{ Name = "Motion blur effect"; Source = $sourceMotionBlur; Destination = $installedMotionBlur },
    @{ Name = "Installed version information"; Source = $sourceVersion; Destination = $installedVersion }
)

Write-Host ""
Write-Host "Applying Kori files..."
foreach ($file in $files) {
    $replacing = Test-Path -LiteralPath $file.Destination
    Copy-Item -LiteralPath $file.Source -Destination $file.Destination -Force
    if (-not (Test-Path -LiteralPath $file.Destination)) {
        throw "Kori installation could not be verified: '$($file.Destination)' is missing."
    }
    $sourceHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $file.Source).Hash
    $installedHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $file.Destination).Hash
    if ($sourceHash -ne $installedHash) {
        throw "Kori installation verification failed for '$($file.Destination)'."
    }
    $result = if ($replacing) { "Replaced" } else { "Installed" }
    Write-Host "  [$result] $($file.Name)"
}

Write-Host ""
Write-Host "Kori installed successfully." -ForegroundColor Green
Write-Host "All installed files were verified."
Write-Host "OBS scenes, hotkeys and saved Kori settings were not changed."
Write-Host "Start OBS, then look for Kori Settings under Tools."
