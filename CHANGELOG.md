# Changelog

## v1.0.0 — 2026-07-25

First full release of Kobo 2048: swipe-to-merge core loop, win/game-over
detection with New Game restart, persisted best score, full session resume
across close/reopen and interruption, a Color/Black & White display theme
(auto-detected, overridable) covering the whole UI, a tunable e-ink
full-refresh cadence, and an Exit control. Built from the template below via
spec-driven development (see `specs/001-2048-game/`).

## v0.1.0 — 2026-07-25

Initial template: 4-layer architecture (core/persist/platform/ui) carried
over from kobo-sudoku, generic tap-counter placeholder demo, SDL2 simulator
and FBInk/Kobo cross-build flavors, NickelMenu+KFMon packaging, spec-kit
tooling, and `tools/rename-project.sh` to rebrand a fresh copy into a new
project.
