# Kori 0.13.1 - Windows beta

Kori 0.13.1 fixes click-to-focus framing for a common OBS camera setup.

## Fixed

- Corrected zoom positioning for sources configured with an OBS bounding box,
  including cameras fitted to the 1920x1080 canvas.
- The selected focus point now remains aligned with the finished zoom framing
  instead of being offset by OBS’s bounds scaling.

## Upgrade

Close OBS, extract the ZIP and run `Install Kori.cmd`. Installing over an
older Kori version preserves OBS scenes, hotkeys and saved Kori settings.

Kori remains beta software. Please report problems through GitHub Issues or at
kori.dev@askq.co.nz.
