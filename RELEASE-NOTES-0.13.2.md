# Kori 0.13.2 - Windows beta

Kori 0.13.2 corrects the camera movement regression in 0.13.1.

## Fixed

- Restored correct zoom behaviour for sources using OBS bounding-box transforms,
  including fitted 1920x1080 cameras.
- The animation now updates the bounding box as well as the source position,
  preventing the image from lifting or sliding instead of zooming.

## Upgrade

Close OBS, extract the ZIP and run `Install Kori.cmd`. Installing over an
older Kori version preserves OBS scenes, hotkeys and saved Kori settings.

Kori remains beta software. Please report problems through GitHub Issues or at
kori.dev@askq.co.nz.
