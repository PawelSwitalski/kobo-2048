# Contract: Device Package & Launch Integration

Installation = unzip one archive to the Kobo's USB mass-storage root (e.g. `D:\`), eject, done. One-time prerequisite: NickelMenu and/or KFMon installed (see [installation.md](../installation.md)).

## Package layout (zip root = device root)

```text
.adds/
├── kobo_2048/
│   ├── kobo_2048               # armhf binary (koxtoolchain build)
│   ├── start.sh                # launch wrapper (see below)
│   ├── assets/                 # fonts (bundled TTF)
│   └── (save.json / best.json / settings.json created at runtime)
├── nm/
│   └── kobo_2048                # NickelMenu entry (FW 4.x)
└── kfmon/
    └── config/
        └── kobo_2048.ini         # KFMon watch config

kfmon-kobo_2048.png               # "book cover" trigger image at device root
```

## KFMon config (`kobo_2048.ini`)

```ini
[watch]
filename = /mnt/onboard/kfmon-kobo_2048.png
action = /mnt/onboard/.adds/kobo_2048/start.sh
```

Tapping the "Kobo 2048" cover in the Kobo library launches the app. Works on firmware 4.x and 5.x (KFMon is inotify-based; NickelMenu is FW-4.x-only).

## `start.sh` obligations

- `cd` into `/mnt/onboard/.adds/kobo_2048`, set `LD_LIBRARY_PATH` if any bundled `.so` (target: none — static/vendored).
- Run `./kobo_2048`; on exit, control returns to Nickel.
- Log stderr to `.adds/kobo_2048/crash.log` (truncate per run) for field debugging.

## NickelMenu entry (`.adds/nm/kobo_2048`, FW 4.x only)

```text
menu_item : main : Kobo 2048 : cmd_spawn : quiet : exec /mnt/onboard/.adds/kobo_2048/start.sh
```

## Compatibility contract

- Binary: ARMv7 hard-float, linked against koxtoolchain glibc floor → runs on every Nickel-era Kobo (Touch C and newer), including sunxi (Sage/Elipsa) and 2024 colour devices (Clara/Libra Colour) via FBInk's device abstraction.
- Layout: all geometry computed from `Renderer::info()` (width/height/dpi) — no hardcoded pixel positions.
- Uninstall: delete `.adds/kobo_2048/`, `.adds/nm/kobo_2048`, `.adds/kfmon/config/kobo_2048.ini`, `kfmon-kobo_2048.png`.
