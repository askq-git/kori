# Kori — Smooth Zoom Plugin for OBS Studio

Kori is a free Windows plugin for OBS Studio that creates smooth,
camera-style zoom animations without duplicating scenes or configuring
complex transitions.

Click a face or object to choose the focus point, then trigger the zoom with a
hotkey or automatically when a scene becomes active. Kori can return smoothly
to the exact starting view, zoom one camera or move the whole scene—including
logos, videos and overlays.

## See Kori in action

| Manual cinematic zoom | Automatic zoom on scene change |
| --- | --- |
| [![Watch Kori's manual cinematic zoom demonstration](https://img.youtube.com/vi/BjvPKBs2L_o/maxresdefault.jpg)](https://youtu.be/BjvPKBs2L_o) | [![Watch Kori's automatic scene zoom demonstration](https://img.youtube.com/vi/s_300woejUA/maxresdefault.jpg)](https://youtu.be/s_300woejUA) |
| [Watch the manual zoom demonstration](https://youtu.be/BjvPKBs2L_o) | [Watch the automatic scene zoom demonstration](https://youtu.be/s_300woejUA) |

## Download

**[Download Kori for Windows](https://github.com/askq-git/kori/releases/download/v0.10.0/Kori-0.10.0-Windows-x64.zip)**

Kori works with:

- Windows 10 or 11
- OBS Studio 31.1 or newer
- Normal OBS scenes

## Install

1. Download the Kori ZIP using the button above.
2. Open the downloaded ZIP and extract its contents.
3. Close OBS.
4. Double-click **Install Kori.cmd**.
5. Approve the Windows prompt.
6. Open OBS and choose **Tools > Kori Settings**.

Kori is still being tested and the installer is not yet signed. Windows may
show an unrecognized-app warning. Only download Kori from this GitHub page.

## Set up your first zoom

1. Open your scene in OBS.
2. Choose **Tools > Kori Settings**.
3. Choose the camera or item you want to zoom.
4. Click or drag the red target onto the person's face or another point of
   interest.
5. Adjust the zoom amount and speed.
6. Use **Preview Zoom** and **Preview Return** to check the result.
7. Choose **Save & Close**.
8. Open **Settings > Hotkeys** and assign keys for:
   - **Kori: Play slow zoom**
   - **Kori: Return smoothly to start**

Press your Play hotkey to zoom in and your Return hotkey to move back out.

## Zoom automatically when a scene appears

Kori can start an animation as soon as you switch to a scene.

In Kori Settings, change **Activation** to
**Automatically when scene becomes active**. You can also add a delay, leave
the scene zoomed in, or have it wait and return automatically.

## What you can zoom

- One camera or other item
- Everything in the scene at once
- Different items with their own saved zoom settings
- Different scenes with their own focus points and timings

## Why use Kori?

| Feature | Kori |
| --- | --- |
| Duplicate scenes required | No |
| Smooth zoom and return | Yes |
| Clickable focus-point picker | Yes |
| Manual hotkey control | Yes |
| Automatic zoom when a scene becomes active | Yes |
| Zoom an individual item or the entire scene | Yes |

## Frequently asked questions

### What is Kori?

Kori is a free plugin that adds smooth, camera-style zoom animations to OBS
Studio on Windows.

### Do I need to duplicate or nest scenes?

No. Kori can animate an item or the entire active scene without requiring a
duplicate scene.

### Can Kori start a zoom automatically?

Yes. A saved zoom can begin automatically when its scene becomes active. You
can also add a delay, hold the zoom or return automatically.

### Can I control Kori with a hotkey?

Yes. OBS hotkeys can play the saved zoom and return smoothly to the starting
view.

### Does Kori collect information about me?

No. Kori does not collect usage information or connect to the internet.

### Which systems does Kori currently support?

The current beta supports Windows 10 and 11, OBS Studio 31.1 or newer and the
standard OBS canvas. Other operating systems and additional canvases are not
supported yet.

### Is Kori an official OBS Project plugin?

No. Kori is an independent open-source plugin and is not affiliated with or
endorsed by the OBS Project.

## Things to know

- This version supports the standard OBS canvas. Support for additional
  canvases is planned.
- Items inside a group cannot be chosen separately yet, but they will still
  move when you zoom the whole scene.
- Some heavily cropped items may need the advanced focus controls.
- Automatic zoom follows the live Program scene, not the Studio Mode Preview.
- This version is for Windows.

## Help

- [Full user guide](USER-GUIDE.md)
- [Help test the beta](BETA-TESTING.md)
- [Fix a problem](TROUBLESHOOTING.md)
- [Report a bug or suggest an idea](SUPPORT.md)
- [See what changed](CHANGELOG.md)

You can also contact **kori.dev@askq.co.nz**.

Kori does not collect usage information or connect to the internet.

## Trademark acknowledgement

Kori is an independent plugin and is not affiliated with or endorsed by the
OBS Project. OBS, OBS Studio and the OBS Studio logo are registered trademarks
of Wizards of OBS LLC.

## Developer information

The sections below are only needed if you want to inspect, build or contribute
to Kori:

- [Build from source](docs/BUILDING.md)
- [Development testing checklist](docs/TESTING.md)
- [Development disclosure](docs/DEVELOPMENT.md)
- [Project contribution policy](CONTRIBUTING.md)
- [Security policy](SECURITY.md)
- [GPL-2.0-or-later licence](LICENSE)
- [Copyright and project-name notice](NOTICE.md)
