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
        throw "Administrator permission is required to uninstall Kori. $($_.Exception.Message)"
    }
}

if (Get-Process -Name "obs64" -ErrorAction SilentlyContinue) {
    throw "OBS Studio is running. Close OBS completely, then run the uninstaller again."
}

$targets = @(
    @{ Name = "Kori plugin"; Path = (Join-Path $ObsPath "obs-plugins\64bit\kori.dll") },
    @{ Name = "English interface text"; Path = (Join-Path $ObsPath "data\obs-plugins\kori\locale\en-US.ini") },
    @{ Name = "Motion blur effect"; Path = (Join-Path $ObsPath "data\obs-plugins\kori\effects\kori-motion-blur.effect") },
    @{ Name = "Installed version information"; Path = (Join-Path $ObsPath "data\obs-plugins\kori\version.txt") }
)
Write-Host ""
Write-Host "Removing Kori files..."
foreach ($target in $targets) {
    if (Test-Path -LiteralPath $target.Path) {
        Remove-Item -LiteralPath $target.Path -Force
        if (Test-Path -LiteralPath $target.Path) {
            throw "Kori uninstallation could not be verified: '$($target.Path)' remains."
        }
        Write-Host "  [Removed] $($target.Name)"
    }
    else {
        Write-Host "  [Already absent] $($target.Name)"
    }
}

$directories = @(
    (Join-Path $ObsPath "data\obs-plugins\kori\locale"),
    (Join-Path $ObsPath "data\obs-plugins\kori\effects"),
    (Join-Path $ObsPath "data\obs-plugins\kori")
)
foreach ($directory in $directories) {
    if ((Test-Path -LiteralPath $directory) -and -not (Get-ChildItem -LiteralPath $directory -Force)) {
        Remove-Item -LiteralPath $directory -Force
    }
}

Write-Host ""
Write-Host "Kori was removed from OBS." -ForegroundColor Green
Write-Host "Saved profiles remain in the OBS user configuration."
