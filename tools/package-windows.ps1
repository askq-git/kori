[CmdletBinding()]
param(
    [string]$Version = "0.10.0"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$releaseRoot = Join-Path $projectRoot "release"
$packageName = "Kori-$Version-Windows-x64"
$packageRoot = Join-Path $releaseRoot $packageName
$zipPath = Join-Path $releaseRoot "$packageName.zip"
$checksumPath = "$zipPath.sha256"
$pluginDll = Join-Path $projectRoot "build\windows-x64\Release\kori.dll"

if (-not (Test-Path -LiteralPath $pluginDll)) {
    throw "kori.dll was not found. Build Kori before creating the package."
}

New-Item -ItemType Directory -Force -Path $releaseRoot | Out-Null

$resolvedReleaseRoot = [System.IO.Path]::GetFullPath($releaseRoot)
$resolvedPackageRoot = [System.IO.Path]::GetFullPath($packageRoot)
if (-not $resolvedPackageRoot.StartsWith(
        $resolvedReleaseRoot + [System.IO.Path]::DirectorySeparatorChar,
        [System.StringComparison]::OrdinalIgnoreCase
    )) {
    throw "Refusing to clean a package directory outside the release folder."
}

if (Test-Path -LiteralPath $packageRoot) {
    Remove-Item -LiteralPath $packageRoot -Recurse -Force
}
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}
if (Test-Path -LiteralPath $checksumPath) {
    Remove-Item -LiteralPath $checksumPath -Force
}

$pluginRoot = Join-Path $packageRoot "plugin"
$localeRoot = Join-Path $pluginRoot "data\locale"
New-Item -ItemType Directory -Force -Path $localeRoot | Out-Null

Copy-Item -LiteralPath $pluginDll `
    -Destination (Join-Path $pluginRoot "kori.dll")
Copy-Item -LiteralPath (Join-Path $projectRoot "data\locale\en-US.ini") `
    -Destination (Join-Path $localeRoot "en-US.ini")

$packageFiles = @(
    "BETA-TESTING.md",
    "LICENSE",
    "NOTICE.md",
    "README.md",
    "RELEASE-NOTES-0.10.0.md",
    "SECURITY.md",
    "SUPPORT.md",
    "TROUBLESHOOTING.md",
    "USER-GUIDE.md"
)

foreach ($relativePath in $packageFiles) {
    Copy-Item -LiteralPath (Join-Path $projectRoot $relativePath) `
        -Destination (Join-Path $packageRoot $relativePath)
}

$installerFiles = @(
    "Install Kori.cmd",
    "Install-Kori.ps1",
    "Uninstall Kori.cmd",
    "Uninstall-Kori.ps1"
)

foreach ($fileName in $installerFiles) {
    Copy-Item -LiteralPath (Join-Path $projectRoot "installer\$fileName") `
        -Destination (Join-Path $packageRoot $fileName)
}

Compress-Archive -LiteralPath $packageRoot -DestinationPath $zipPath

$hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $zipPath).Hash
"$hash  $packageName.zip" |
    Set-Content -LiteralPath $checksumPath -Encoding ascii

Write-Host "Kori package created: $zipPath"
Write-Host "SHA-256: $hash"
