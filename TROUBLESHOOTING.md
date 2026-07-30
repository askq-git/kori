# Kori troubleshooting

## Kori does not appear in OBS

1. Confirm OBS is 64-bit and version 31.1 or newer.
2. Confirm `kori.dll` exists at:
   `C:\Program Files\obs-studio\obs-plugins\64bit\kori.dll`
3. Restart OBS.
4. Open **Help > Log Files > View Current Log**.
5. Search for `[Kori] Loaded version`.

## Play controls the wrong source

- A selected source temporarily overrides the manual default.
- Deselect all sources to use the scene's saved manual target.
- Open Kori Settings and click Apply on the target that should be default.

## Automatic animation does not run

- Confirm the target says **automatic target** in the target list.
- Automatic activation runs when the scene reaches Program, not when it is only
  selected in Studio Mode Preview.
- Only one target per scene can be automatic.

## A saved source was deleted

Kori safely falls back to another available source for manual control.
Deleting the automatic target disables that automatic action instead of moving
it unexpectedly.

## The focus point does not move to the centre

This is expected. The current focus mode anchors the selected point in place.
It is a zoom origin, not a destination point.

## Camera errors mention MJPEG or JPEG

Those messages come from the capture device or its selected video format, not
Kori. Test Kori using another supported camera format or source while
diagnosing the device separately.

## Reporting a problem

Include:

- OBS version
- Kori version from the window title or OBS log
- Normal mode or Studio Mode
- Individual source or Entire scene target
- Source types involved
- Every current-log line containing `[Kori]`

