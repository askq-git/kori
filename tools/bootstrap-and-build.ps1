[CmdletBinding()]
param(
    [string]$ObsTag = "32.1.2",
    [string]$DependencyRoot = (Join-Path $env:LOCALAPPDATA "KoriBuild"),
    [switch]$InstallForCurrentUser,
    [switch]$InstallSystemWide
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# Some launch environments provide both Path and PATH. MSBuild treats those
# as duplicate keys when starting the compiler, so normalize them first.
$processPath = $env:Path
[Environment]::SetEnvironmentVariable("PATH", $null, "Process")
[Environment]::SetEnvironmentVariable("Path", $processPath, "Process")

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$dependencyRootPath = [System.IO.Path]::GetFullPath($DependencyRoot)
$obsSourcePath = Join-Path $dependencyRootPath "obs-studio"
$obsBuildPath = Join-Path $obsSourcePath "build_x64"
$koriBuildPath = Join-Path $projectRoot "build\windows-x64"

function Find-Executable {
    param(
        [Parameter(Mandatory)]
        [string]$Name,
        [string[]]$Candidates = @()
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    foreach ($candidate in $Candidates) {
        if (Test-Path -LiteralPath $candidate) {
            return $candidate
        }
    }

    throw "Required tool '$Name' was not found."
}

function Find-PackageDirectory {
    param(
        [Parameter(Mandatory)]
        [string]$Root,
        [Parameter(Mandatory)]
        [string]$ConfigFile
    )

    $match = Get-ChildItem -LiteralPath $Root -Recurse -Filter $ConfigFile |
        Select-Object -First 1

    if (-not $match) {
        throw "Could not find $ConfigFile beneath $Root."
    }

    return $match.Directory.FullName
}

$git = Find-Executable -Name "git.exe" -Candidates @(
    "C:\Program Files\Git\cmd\git.exe"
)

$cmake = Find-Executable -Name "cmake.exe" -Candidates @(
    "C:\Program Files\CMake\bin\cmake.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)

$vsWhere = Find-Executable -Name "vswhere.exe" -Candidates @(
    "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
)

$visualStudio = & $vsWhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath

if (-not $visualStudio) {
    throw "Visual Studio Build Tools with the x64 C++ workload are required."
}

$visualStudioGenerator = if ($visualStudio -match "\\18\\") {
    "Visual Studio 18 2026"
} else {
    "Visual Studio 17 2022"
}

Write-Host "Kori prerequisite check passed."
Write-Host "Git: $git"
Write-Host "CMake: $cmake"
Write-Host "Visual Studio: $visualStudio"
Write-Host "CMake generator: $visualStudioGenerator"

New-Item -ItemType Directory -Force -Path $dependencyRootPath | Out-Null

if (-not (Test-Path -LiteralPath (Join-Path $obsSourcePath ".git"))) {
    Write-Host "Downloading OBS Studio $ObsTag source and submodules..."
    & $git -c core.longpaths=true clone `
        --branch $ObsTag --depth 1 --recurse-submodules `
        https://github.com/obsproject/obs-studio.git $obsSourcePath
    if ($LASTEXITCODE -ne 0) {
        throw "OBS source download failed."
    }
}

Write-Host "Ensuring all OBS submodules are complete..."
& $git -C $obsSourcePath -c core.longpaths=true `
    submodule update --init --recursive --depth 1
if ($LASTEXITCODE -ne 0) {
    throw "OBS submodule download failed."
}

Write-Host "Configuring the OBS development build..."
& $cmake -S $obsSourcePath -B $obsBuildPath `
    -G $visualStudioGenerator -A x64 `
    -DENABLE_BROWSER=OFF `
    -DENABLE_AJA=OFF `
    -DENABLE_UI=ON
if ($LASTEXITCODE -ne 0) {
    throw "OBS configuration failed. Review the preceding CMake output."
}

Write-Host "Building the OBS libraries required by Kori..."
& $cmake --build $obsBuildPath --config Release `
    --target libobs obs-frontend-api
if ($LASTEXITCODE -ne 0) {
    throw "OBS development library build failed."
}

$libobsDirectory = Find-PackageDirectory `
    -Root $obsBuildPath `
    -ConfigFile "libobsConfig.cmake"

$frontendDirectory = Find-PackageDirectory `
    -Root $obsBuildPath `
    -ConfigFile "obs-frontend-apiConfig.cmake"

$pthreadsDirectory = Find-PackageDirectory `
    -Root $obsBuildPath `
    -ConfigFile "w32-pthreadsConfig.cmake"

$obsDepsDirectory = Get-ChildItem `
    -LiteralPath (Join-Path $obsSourcePath ".deps") `
    -Directory -Filter "obs-deps-*-x64" |
    Select-Object -First 1

if (-not $obsDepsDirectory) {
    throw "Could not locate the downloaded x64 OBS dependency package."
}

$qtDepsDirectory = Get-ChildItem `
    -LiteralPath (Join-Path $obsSourcePath ".deps") `
    -Directory -Filter "obs-deps-qt6-*-x64" |
    Select-Object -First 1

if (-not $qtDepsDirectory) {
    throw "Could not locate the downloaded x64 OBS Qt package."
}

# CMake propagates these values into compiler checks. Forward slashes avoid
# Windows backslashes being interpreted as escape sequences in generated files.
$libobsCMakePath = $libobsDirectory.Replace("\", "/")
$frontendCMakePath = $frontendDirectory.Replace("\", "/")
$pthreadsCMakePath = $pthreadsDirectory.Replace("\", "/")
$findersCMakePath = (Join-Path $obsSourcePath "cmake\finders").Replace("\", "/")
$obsDepsCMakePath = $obsDepsDirectory.FullName.Replace("\", "/")
$qtDepsCMakePath = $qtDepsDirectory.FullName.Replace("\", "/")
$combinedPrefixPath = "$obsDepsCMakePath;$qtDepsCMakePath"

Write-Host "Configuring Kori..."
& $cmake -S $projectRoot -B $koriBuildPath `
    -G $visualStudioGenerator -A x64 `
    "-Dlibobs_DIR=$libobsCMakePath" `
    "-Dobs-frontend-api_DIR=$frontendCMakePath" `
    "-Dw32-pthreads_DIR=$pthreadsCMakePath" `
    "-DCMAKE_MODULE_PATH=$findersCMakePath" `
    "-DCMAKE_PREFIX_PATH=$combinedPrefixPath"
if ($LASTEXITCODE -ne 0) {
    throw "Kori configuration failed."
}

Write-Host "Building Kori..."
& $cmake --build $koriBuildPath --config Release
if ($LASTEXITCODE -ne 0) {
    throw "Kori compilation failed."
}

$pluginDll = Join-Path $koriBuildPath "Release\kori.dll"
if (-not (Test-Path -LiteralPath $pluginDll)) {
    throw "The build completed but kori.dll was not found at $pluginDll."
}

Write-Host "Kori built successfully: $pluginDll"

if ($InstallForCurrentUser) {
    $pluginRoot = Join-Path $env:APPDATA "obs-studio\plugins\kori"
    $pluginBin = Join-Path $pluginRoot "bin\64bit"
    $pluginLocale = Join-Path $pluginRoot "data\locale"

    New-Item -ItemType Directory -Force -Path $pluginBin | Out-Null
    New-Item -ItemType Directory -Force -Path $pluginLocale | Out-Null

    Copy-Item -LiteralPath $pluginDll `
        -Destination (Join-Path $pluginBin "kori.dll") -Force
    Copy-Item -LiteralPath (Join-Path $projectRoot "data\locale\en-US.ini") `
        -Destination (Join-Path $pluginLocale "en-US.ini") -Force

    Write-Host "Installed Kori for the current user: $pluginRoot"
}

if ($InstallSystemWide) {
    $systemPluginBin = "C:\Program Files\obs-studio\obs-plugins\64bit"
    $systemPluginLocale =
        "C:\Program Files\obs-studio\data\obs-plugins\kori\locale"

    try {
        New-Item -ItemType Directory -Force `
            -Path $systemPluginLocale | Out-Null
        Copy-Item -LiteralPath $pluginDll `
            -Destination (Join-Path $systemPluginBin "kori.dll") -Force
        Copy-Item `
            -LiteralPath (Join-Path $projectRoot "data\locale\en-US.ini") `
            -Destination (Join-Path $systemPluginLocale "en-US.ini") -Force
    } catch {
        throw "System-wide installation failed. Close OBS and run PowerShell as Administrator, then retry with -InstallSystemWide. $($_.Exception.Message)"
    }

    Write-Host "Installed Kori system-wide: $systemPluginBin"
}
