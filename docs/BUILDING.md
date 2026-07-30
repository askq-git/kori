# Building Kori from source

These instructions are for developers. Regular users should download the
installer ZIP from the repository's Releases page.

## Prerequisites

- Windows 10 or 11, 64-bit
- OBS Studio 31.1 or newer, 64-bit
- Visual Studio 2022 or newer with **Desktop development with C++**
- Windows 10 or 11 SDK
- CMake 3.28 or newer

OBS development headers and import libraries are required. Installing the
normal OBS application alone does not provide them.

## Automated setup

Open PowerShell in the Kori repository:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\tools\bootstrap-and-build.ps1 -ObsTag 32.1.2 -InstallForCurrentUser
```

The script checks the compiler and CMake installation, downloads the selected
OBS source tag, builds the required development libraries, builds Kori and
optionally installs it. The first run takes longer because it builds the OBS
dependencies.

If OBS does not scan the per-user plugin directory, close OBS, open PowerShell
as Administrator and run:

```powershell
.\tools\bootstrap-and-build.ps1 -ObsTag 32.1.2 -InstallSystemWide
```

Dependencies are stored beneath `%LOCALAPPDATA%\KoriBuild` to avoid Windows
path-length problems.

## Manual build

Build OBS 31.1 or newer first, then locate `libobsConfig.cmake` and
`obs-frontend-apiConfig.cmake` in the OBS build. From a Visual Studio x64
developer prompt:

```powershell
cmake -S . -B build\windows-x64 `
  -G "Visual Studio 18 2026" -A x64 `
  -Dlibobs_DIR="C:\path\to\obs-studio\build_x64\libobs" `
  -Dobs-frontend-api_DIR="C:\path\to\obs-studio\build_x64\frontend\api"

cmake --build build\windows-x64 --config Release
```

The expected output is:

```text
build\windows-x64\Release\kori.dll
```

## Create the Windows ZIP

After a successful Release build:

```powershell
.\tools\package-windows.ps1
```

The packaging script uses an explicit list of approved files and creates:

```text
release\Kori-0.10.0-Windows-x64.zip
release\Kori-0.10.0-Windows-x64.zip.sha256
```

## Manual installation

Close OBS before copying plugin files.

Per-user binary path:

```text
%APPDATA%\obs-studio\plugins\kori\bin\64bit\kori.dll
```

System-wide binary path:

```text
C:\Program Files\obs-studio\obs-plugins\64bit\kori.dll
```

The locale file at `data\locale\en-US.ini` belongs at:

```text
%APPDATA%\obs-studio\plugins\kori\data\locale\en-US.ini
```

## Architecture

Kori separates animation from target discovery:

```text
OBS action
    |
    v
TargetResolver -- MainCanvasTargetResolver
    |
    v
SceneItemTarget
    |
    v
AnimationEngine -- easing
    |
    v
OBS scene-item transforms
```

Future multi-canvas work belongs behind `TargetResolver`. Additional resolvers
can discover the relevant canvas and pass the same target abstraction to the
existing animation engine.
