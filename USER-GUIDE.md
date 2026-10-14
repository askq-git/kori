# Kori user guide

## The 60-second setup

1. In OBS, open the scene you want to animate.
2. Open **Tools > Kori Settings**.
3. Choose a **Target source**, or choose **Entire scene (everything)**.
4. Click the important part of the preview.
5. Choose a **Camera style**.
6. Click **Preview Zoom**, then **Preview Return**.
7. Click **Apply**.
8. Assign the Kori Play and Return actions under **Settings > Hotkeys**.

That is enough for a manual production-quality zoom.

## Kori dock

Open **Docks > Kori** for live controls. The dock follows the scene's saved
Kori target and refreshes automatically when OBS opens, when the scene changes
and when detailed settings are applied. Use it to Play, Return, or quickly
adjust zoom amount, timing and movement curve. Click **Apply changes** to save
an adjustment. Use **Set focus point** for the visual click-to-focus control.

## Camera styles and Fine-tune

Camera Styles combine the zoom amount, zoom and return timing, movement curve
and motion blur into one creative choice:

- **Natural Push** - balanced movement for interviews and podcasts.
- **Slow Drama** - a longer move that gradually builds intensity.
- **Documentary Push** - subtle, restrained movement.
- **Punch In** - fast movement with a small overshoot and settle.
- **Awkward Zoom** - a deliberately long, increasingly uncomfortable push.

The compact summary shows the values used by the selected style. Open
**Fine-tune** to change them individually. Changing a style value switches the
Camera style to **Custom** automatically.

## How Kori chooses what to animate

- With nothing selected in the OBS Sources dock, Play uses the scene's last
  applied manual target.
- Selecting one source temporarily directs Play to that source.
- **Entire scene (everything)** moves all top-level elements together.
- One target per scene can run automatically when that scene reaches Program.
- Every target retains its own zoom, focus, timing and motion settings.

## Configure several cameras without closing

1. Choose Cam 1, adjust it and click **Apply**.
2. Choose Cam 2, adjust it and click **Apply**.
3. Continue for any other target.
4. Click **Save & Close** when finished.

If you change targets before applying, Kori asks whether to save or discard
the changes.

## Automatic scene-entry zoom

1. Choose the target.
2. Set **Activation** to **Automatically when scene becomes active**.
3. Optionally add a start delay.
4. Choose **Stay zoomed** or **Hold, then return**.
5. Click **Apply**.

Each scene has one automatic target. Assigning automation to another target
transfers the role. The automatic target is separate from the manual default.

## Studio Mode

Kori follows the Program output. Merely selecting a Preview scene does not
start its automatic animation. The animation starts after the scene transition
finishes and the scene reaches Program.

## Understanding the focus point

**Keep selected point in place** is the safe default. The clicked point remains
anchored where it currently appears while the target grows around it.

**Centre selected point** places the clicked point at the canvas centre when
the zoom completes. Choose a sufficiently large zoom factor for the selected
point so the source still covers the canvas; otherwise empty edges can appear.

## Safe live operation

- Return works during an active zoom.
- Starting a new Play restores the previous captured target first.
- Leaving an automatic scene cancels pending delay, hold and return actions.
- With reset-on-exit enabled, Kori restores exact captured transforms.
- Kori also restores captured transforms during OBS shutdown and scene
  collection cleanup.

