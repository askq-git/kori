# Changelog

## 0.10.0

- Established **Kori** as the project and plugin name.
- Preserved existing profiles and hotkey bindings from pre-release builds.
- Added installer migration and legacy-file cleanup.
- Updated Windows packaging, documentation, logs and OBS-facing labels.

## 0.9.1

- Removed empty Kori locale and data directories during uninstall.
- Kept folder removal conservative: directories are removed only when empty.
- Reorganized the README so ordinary installation appears before clearly
  labelled developer-only build prerequisites.
- Refreshed the packaged Windows beta and installer validation.

## 0.9.0

- Added explicit transform restoration during OBS shutdown and scene collection
  cleanup.
- Verified automatic activation follows Program scene changes after transitions
  finish and ignores Studio Mode Preview-only changes.
- Added the Kori version to the settings window title.
- Added Windows installation and uninstallation helpers with OBS-running and
  install-path validation.
- Added a concise operator user guide, troubleshooting reference and beta
  release notes.
- Added a redistributable Windows x64 release ZIP layout.

## 0.8.0

- Added **Entire scene (everything)** as a first-class Kori target.
- Added coordinated canvas-space zooming for every top-level scene item,
  including cameras, overlays, browser sources, logos and groups.
- Reused the existing focus picker, easing, delay, hold, return and automatic
  activation controls for whole-scene animations.
- Added exact multi-item transform and crop restoration.
- Added independent whole-scene settings profiles per OBS scene.
- Preserved selected-source override behaviour when a scene's manual default is
  Entire scene.
- Simplified the target explanation and removed the ambiguous initial Ready
  status.

## 0.7.3

- Moved Preview Zoom and Preview Return directly beneath the live preview.
- Added clear visual separation between preview actions and save actions.
- Made the settings window resizable with a practical minimum size.
- Added a scrollable settings area while keeping the preview and save footer
  visible.
- Prevented Advanced focus controls from resizing or shifting the window.
- Added a plain-language targeting model to the usage documentation.

## 0.7.2

- Added **Apply** to save the current source without closing Kori Settings.
- Added **Save & Close** for the original one-step completion workflow.
- Added Save, Discard and Cancel protection when switching targets with
  unsaved changes.
- Added a persistent status line for loaded, edited and applied states.
- Refreshed automatic-target labels immediately after Apply.
- Preserved already-applied profiles when the dialog is later cancelled.

## 0.7.1

- Separated each scene's manual default target from its automatic target.
- Prevented saving a later manual profile from silently disabling another
  source's automatic animation.
- Added explicit automatic-role transfer when automation is enabled on a
  different source.
- Added automatic-role clearing when activation is disabled on that source.
- Added safe fallback to an available source when a deleted manual target is
  recalled.
- Added clear logging and safe automation removal for deleted automatic
  targets.
- Migrated existing V0.7 target assignments without discarding profiles.

## 0.7.0

- Added a Target source selector directly inside Kori Settings.
- Added the active scene name to the settings window.
- Allowed switching the live preview and loading per-target settings without
  closing the window.
- Kept independent saved profiles for different sources in the same scene.
- Made the last saved source the scene's active manual and automatic target.
- Added distinct instance labels when one source appears multiple times.
- Added safe handling when a selected source disappears before saving.

## 0.6.0

- Added per-scene automatic activation when a normal OBS scene becomes active.
- Added configurable start delay for automatic animations.
- Added **Stay zoomed** and **Hold, then return** completion behaviours.
- Added configurable hold duration before the automatic return.
- Added safe cancellation and optional exact transform restoration when the
  operator leaves a scene.
- Kept manual Play and Return hotkeys immediate and backward compatible.

## 0.5.0

- Added per-profile motion styles: Smooth, Cinematic, Slow Burn, Punch and
  Linear.
- Preserved the existing cubic ease as the default Smooth style.
- Added an **Open settings for current source** OBS hotkey.
- Allowed the settings hotkey to recall a scene's saved target without requiring
  a manual source selection.
- Preserved easing choices independently for every scene-item profile.

## 0.4.1

- Stabilized the settings window when Advanced focus controls are expanded or
  collapsed.
- Prevented the preview and introductory text from shifting vertically.
- Removed residual spacing left by collapsed Advanced rows.
- Refined the transparent target into a cleaner ring, separated guide ticks and
  centre indicator.

## 0.4.0

- Replaced the native child-window marker with a transparent render-layer
  target.
- Added continuous click-and-drag focus positioning.
- Fixed doubled and offset target artifacts.
- Fixed advanced focus labels remaining visible while their fields were hidden.
- Added settings profiles keyed to stable scene UUID and scene-item ID.
- Added different focus, zoom and timing settings for each configured scene.
- Added automatic target recall, so a configured scene no longer requires the
  source to be selected before using the Play hotkey.

## 0.3.0

- Added a live source preview to Kori Settings.
- Added click-to-select focus with a visible target marker.
- Added Preview Zoom and Preview Return buttons.
- Moved percentage coordinates behind an advanced option.
- Required a selected normal-canvas source before opening settings.

## 0.2.0

- Added focus-point anchoring with percentage coordinates.
- Added face-oriented centre, left, right and custom focus presets.
- Added configurable zoom amount, zoom duration and return duration.
- Added a minimal settings window under the OBS Tools menu.
- Added persistent settings through the OBS user configuration.
- Changed Reset into a smooth Return that works mid-animation.
- Added exact full-transform and crop restoration after Return.
- Added system-wide installation support to the bootstrap script.

## 0.1.0

- Added native OBS module registration and clear lifecycle logging.
- Added Play and Reset frontend hotkeys.
- Added selected-item resolution for the active main OBS scene.
- Added a reference-safe animation target wrapper.
- Added a five-second, 1.30× cubic ease-in/out zoom.
- Added exact position and scale restoration.
- Separated target resolution from the animation core for future canvas
  integrations.
