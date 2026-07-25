# Contract: Persistent File Formats

Location on device: `.adds/kobo_2048/` (internal storage root; `persist::resolveDataDir`'s
existing resolution order, unchanged by this feature). All files JSON, UTF-8, `schemaVersion`
field required. Writes are atomic: write `<name>.tmp`, `fsync`, `rename`. Any unreadable/invalid
file is treated as absent (FR-013) — never a crash.

## `save.json` — in-progress game (FR-012)

```json
{
  "schemaVersion": 1,
  "board": [0, 2, 0, 4, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 2],
  "score": 148,
  "winShown": false
}
```

- `board`: exactly 16 entries, row-major (index `4*row + col`), each `0` or a power of 2 `>= 2`.
- `score`: current running score (FR-007).
- `winShown`: `true` once a 2048 tile has appeared in this game and the win notice has been shown
  (FR-009) — prevents re-showing it after a resume (User Story 3 Scenario 2, User Story 4).
- Validation on load: array length exactly 16; each value `0` or a power of 2; `score` fits
  `uint32`. Any failure ⇒ discard file, start a fresh game (FR-013).

## `best.json` — best score (FR-008)

```json
{
  "schemaVersion": 1,
  "value": 2420
}
```

Updated only when a `save.json` write's `score` exceeds the current `value` (data-model.md
BestScore update rule). Never reset by "New Game" (FR-011).

## `settings.json` — display settings (FR-020)

```json
{
  "schemaVersion": 1,
  "colorMode": "color",
  "fullRefreshEvery": 12
}
```

- `colorMode`: `"color"` or `"blackWhite"`.
- `fullRefreshEvery`: non-negative integer; `0` means "Never" (no forced full refresh — screen
  transitions still always flash, per the existing `Renderer::flushFull` contract).
- Missing file ⇒ first-launch defaults: `colorMode` from `DisplaySettings::autoDetect` against
  `Renderer::info().color` (FR-017); `fullRefreshEvery` defaults to `12` (this template's existing
  `FbinkRenderer::ghostingPartials_` default). Missing individual key ⇒ that key's documented
  default. Unknown keys preserved on rewrite.
