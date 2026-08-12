[CmdletBinding()]
param(
    [string]$Version = "0.12.0"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$releaseRoot = Join-Path $projectRoot "release"
$packageName = "Kori-$Version-Windows-x64"
$packageRoot = Join-Path $releaseRoot $packageName
$zipPath = Join-Path $releaseRoot "$packageName.zip"
$checksumPath = "$zipPath.sha256"
$pluginCandidates = @(
    (Join-Path $projectRoot "build\windows-x64\Release\kori.dll"),
    (Join-Path $projectRoot "build\nmake-x64\kori.dll")
)
$pluginDll = $pluginCandidates |
    Where-Object { Test-Path -LiteralPath $_ } |
    Sort-Object { (Get-Item -LiteralPath $_).LastWriteTimeUtc } -Descending |
    Select-Object -First 1

if (-not $pluginDll) {
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
$effectsRoot = Join-Path $pluginRoot "data\effects"
New-Item -ItemType Directory -Force -Path $localeRoot | Out-Null
New-Item -ItemType Directory -Force -Path $effectsRoot | Out-Null

$semanticVersion = ([regex]::Match($Version, '^\d+\.\d+\.\d+')).Value
if (-not $semanticVersion) {
    throw "Version must begin with a semantic version such as 0.12.0."
}
$semanticVersion | Set-Content -LiteralPath (Join-Path $packageRoot "VERSION.txt") -Encoding ascii

Copy-Item -LiteralPath $pluginDll `
    -Destination (Join-Path $pluginRoot "kori.dll")
Copy-Item -LiteralPath (Join-Path $projectRoot "data\locale\en-US.ini") `
    -Destination (Join-Path $localeRoot "en-US.ini")
Copy-Item -LiteralPath (Join-Path $projectRoot "data\effects\kori-motion-blur.effect") `
    -Destination (Join-Path $effectsRoot "kori-motion-blur.effect")

$packageFiles = @(
    "BETA-TESTING.md",
    "LICENSE",
    "NOTICE.md",
    "README.md",
    "RELEASE-NOTES-0.12.0.md",
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
    "Get-Kori-Version.ps1",
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
