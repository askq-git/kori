# Kori 0.13.0 - Windows beta

Kori 0.13 improves control over where a zoom finishes.

## What's new

- Added **Centre selected point**, which places the clicked focus point at the
  canvas centre when the zoom completes.
- Added **Keep selected point in place** for the original anchored focus
  behaviour.
- Existing saved Kori profiles retain their original anchored framing when
  upgraded, so established scenes do not change unexpectedly.
- Added a dockable Kori live-controls panel with Play, Return, quick zoom,
  timing and movement-curve controls, plus access to detailed settings.
- The dock refreshes its saved target automatically when OBS opens, scenes
  change, or detailed settings are applied.
- Installer failures now show a copyable diagnostic report in the installer
  window and save it beside the installer.
- Improved local Windows development setup and release packaging reliability.

## Requirements

- Windows 10 or Windows 11
- OBS Studio 31.1 or newer
- Standard OBS scenes

## Upgrade

Close OBS, extract the ZIP and run `Install Kori.cmd`. Installing over an
older Kori version preserves OBS scenes, hotkeys and saved Kori settings.

Kori remains beta software. Please report problems through GitHub Issues or at
kori.dev@askq.co.nz.
