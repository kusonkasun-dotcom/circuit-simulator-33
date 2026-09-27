# ArduLab — PRD

## Problem statement (original)
Build ArduLab: an offline-first Windows/desktop EDA app in C++17/Qt6 (CMake +
Ninja, SQLite catalog, JSON component exchange, `.fal` project format). Core
must work without internet; AI is optional and never required to open/edit/save
projects. Must remain a native Qt app (not a web app). Current stage priority:
persistent SQLite catalog, JSON import, catalog↔UI integration, and cursor snap
readout — up to a defined "done" bar.

## Architecture
Layered; domain independent of Qt Widgets.
- `ardulab_domain` static lib (Qt Core + Sql): core, component (+geometry),
  catalog, import, project.
- `ardulab` executable (Qt Widgets): canvas, commands, ui.
- Tests: Qt Test, headless (offscreen).
Key ADRs in `ArduLab/docs/ADR.md`. Catalog = per-user SQLite via AppDataLocation;
project stores component-definition snapshots; Result/Status error handling;
mm coordinates in domain.

## Users
Electronics designers / hobbyists / engineers wanting a fully offline desktop tool.

## Implemented (2026-06)
Phase 1 MVP, verified on Linux/GCC12/Qt6.4 + headless tests:
- Core: Result/Status, typed IDs, mm units, EventBus, logging.
- Component model + canonical JSON schema v1.0 (definition vs instance),
  geometry (rotation/snap/pin transform).
- SQLite catalog: auto-create on first use, migration framework + Migration 001,
  `user_version` + `schema_migrations`, transactions, repository (add/get/
  search/filter/categories), clear open-failure errors.
- JSON import: full validation, DRAFT policy, skip+report duplicates, atomic
  accepted-set insert, malformed/missing-file handled without crash, import report.
- Canvas A3: grid, zoom, pan, drag-drop place, select/move/rotate/delete,
  2.54 mm snapping, property panel, undo/redo, status readout (cursor mm /
  snapped mm / nearest pin within pixel tolerance).
- `.fal` project: structured JSON, atomic write (QSaveFile), corrupt-file safe,
  definition snapshots.
- 5 example components + generated `examples/sample_project.fal`.
- Tests: 4 suites (geometry, catalog, import, project) — all passing.

## Verification status
- Linux + headless unit tests: PASS.
- Windows/MSVC: NOT verified.
- Interactive GUI: layout verified via offscreen screenshot; real interactive
  desktop session pending.

## Backlog (next)
- P1: Connection Core — wires, junctions, net model, pin-based ERC.
- P1: schematic editor interactions on top of connections.
- P2: explicit "update instances from catalog definition" action.
- P2: legacy JSON adapter (once real samples exist).
- P3: simulation backend (custom MNA / ngspice, clear capability limits).
- P3: PCB layout + manufacturing output.
- P3: AI import (datasheet→draft), AI design assistant, firmware generator
  (Arduino first). All AI optional; editor stays offline.
