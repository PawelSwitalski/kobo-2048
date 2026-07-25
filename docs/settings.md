# Settings

Configuration lives in two places: the in-game **Settings** screen, and
environment variables in the launcher script `.adds/kobo_2048/start.sh` on
the device.

## In-game settings

Tap **Settings** on the main screen while playing:

- **Color mode** — Color / Black & White. On first launch this is
  preselected automatically from the device panel's color capability;
  changing it switches the entire UI (background, score panel, buttons, and
  tile fills), not just the tiles. Choosing Color on a panel that can't
  display it shows a warning but still applies the choice — tiles fall back
  to the same grayscale shading Black & White mode uses.
- **Screen refresh every** — how many quick partial e-ink updates happen
  before a full flashing refresh clears ghosting: **5 / 10 / 25 / Never**.
  Lower means a cleaner screen but more flashing; **Never** disables forced
  full refreshes entirely (screen transitions still flash).

## Launcher settings (`.adds/kobo_2048/start.sh`)

Edit the script on the device over USB and uncomment/set the variables you
need before the `./kobo_2048` line.

### Idle auto-exit

```sh
export KOBO_2048_IDLE_EXIT_SEC=300   # auto-exit after N seconds idle, 0 disables
```

While the app runs, Nickel (the Kobo home software) is paused so it can't
draw over the app or steal input — which also means the power button and
Nickel's own inactivity sleep don't work meanwhile. If you want to sleep the
device immediately, exit the app first. If you just set the device down,
the app exits on its own after 5 minutes without a tap (default), handing
control back to Nickel so its normal sleep timer resumes. Setting `0`
disables the auto-exit — not recommended, since a forgotten paused Nickel
keeps the device awake.

The launcher also pauses `sickel`, Kobo's watchdog daemon (firmware
4.28+), which would otherwise power the device off when it sees Nickel
unresponsive — even mid-session. Both are resumed when the app exits.

### Touch calibration

Kobo touch panels don't all report coordinates in the same orientation as
the screen. If taps land in the wrong place (e.g. the right edge behaves
like the bottom), set the matching variables:

```sh
export KOBO_2048_TOUCH_SWAP_XY=1    # swap raw x/y first
export KOBO_2048_TOUCH_MIRROR_X=1   # mirror x after swap
export KOBO_2048_TOUCH_MIRROR_Y=1   # mirror y after swap
```

To work out which combination your model needs, also set
`KOBO_2048_TOUCH_DEBUG=1`, tap around, then check `.adds/kobo_2048/crash.log`
for `tap raw=(..) -> (..)` lines — raw coordinates that span the screen's
*height* on what should be the *x* axis are the usual sign that `SWAP_XY`
is needed.

Confirmed working settings by model (please share yours if you test a new
one):

| Model | Settings |
|---|---|
| Kobo Libra Colour (FW 4.5) | `SUDOKU_TOUCH_SWAP_XY=1`, `SUDOKU_TOUCH_MIRROR_Y=1` |

**Known limitation:** calibration assumes the device is held in its normal
(non-inverted) portrait orientation — the app doesn't read the
accelerometer, so holding the device upside-down maps taps incorrectly.

## Files on the device

Everything lives in `.adds/kobo_2048/` on the USB-visible storage:

| File | Purpose |
|---|---|
| `kobo_2048` | The app binary |
| `start.sh` | Launch wrapper (pause/resume Nickel, env settings) |
| `assets/` | Fonts |
| `save.json`, `best.json`, `settings.json` | Created by the app at runtime |
| `crash.log` | stderr of the last run — first thing to check if something goes wrong |
