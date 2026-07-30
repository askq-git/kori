# Kori development testing checklist

This is Kori's manual regression suite for official development and release
testing.

## Load and installation

- [ ] OBS starts without a missing-module or missing-DLL warning.
- [ ] The OBS log contains `[Kori] Loaded version`.
- [ ] Kori Settings and all Kori hotkeys appear.
- [ ] The packaged installer refuses to run while OBS is open.
- [ ] Install and Uninstall touch only Kori files.
- [ ] OBS closes normally with Kori loaded.

## Focus and motion

- [ ] Clicking and dragging places one transparent focus marker.
- [ ] Preview Zoom and Preview Return work while Settings remains open.
- [ ] Each motion style previews correctly.
- [ ] Upper-centre and edge focus points move in the expected direction.
- [ ] Return restores the complete starting transform.
- [ ] Returning midway through a zoom works.
- [ ] Repeated Play/Return cycles do not drift.

## Targeting

- [ ] The target list contains compatible top-level scene items.
- [ ] Duplicate source instances have distinct labels.
- [ ] Individual-source zoom leaves other items unchanged.
- [ ] Entire scene moves cameras, overlays, logos and groups together.
- [ ] Whole-scene Return restores every item's exact transform.
- [ ] Selecting a source temporarily overrides the saved manual default.
- [ ] With no selection, Play uses the scene's saved manual default.
- [ ] Deleting the manual default falls back safely and logs recovery.
- [ ] Hidden, locked or removed targets do not crash OBS.

## Saved profiles

- [ ] Different sources retain separate focus, zoom and timing settings.
- [ ] Different scenes containing the same source retain separate profiles.
- [ ] Apply saves without closing the window.
- [ ] Save & Close saves and closes the window.
- [ ] Switching targets with unsaved changes offers Save, Discard and Cancel.
- [ ] Cancel keeps the current target selected.
- [ ] Applied changes remain saved if the main dialog is later cancelled.
- [ ] The settings area scrolls while preview and footer remain usable.
- [ ] Resizing the settings window retains a usable layout.

## Automatic activation

- [ ] A configured Program scene starts after its saved delay.
- [ ] Only one automatic target exists per scene.
- [ ] Saving another manual target does not replace the automatic target.
- [ ] Assigning a new automatic target transfers the role.
- [ ] Disabling the automatic target removes the scene behaviour.
- [ ] Stay zoomed holds the completed framing.
- [ ] Hold, then return waits and restores smoothly.
- [ ] Leaving during delay, zoom or hold cancels pending actions.
- [ ] Reset-on-inactive restores the target for its next activation.
- [ ] Deleting the automatic target disables automation safely.
- [ ] Studio Mode Preview changes do not trigger automation.
- [ ] Completing a Program transition triggers automation.

## Reporting results

Include the Kori, OBS and Windows versions, source type, Studio Mode state,
reproduction steps and all relevant `[Kori]` log lines.
