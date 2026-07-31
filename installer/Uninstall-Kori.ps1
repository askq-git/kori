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
    (Join-Path $ObsPath "obs-plugins\64bit\kori.dll"),
    (Join-Path $ObsPath "data\obs-plugins\kori\locale\en-US.ini")
)
foreach ($target in $targets) {
    if (Test-Path -LiteralPath $target) {
        Remove-Item -LiteralPath $target -Force
    }
}

$directories = @(
    (Join-Path $ObsPath "data\obs-plugins\kori\locale"),
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
