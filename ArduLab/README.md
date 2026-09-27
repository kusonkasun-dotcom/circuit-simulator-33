# ArduLab

Offline-first desktop EDA application (C++17 / Qt6). Phase 1 delivers the
foundation, a persistent per-user component catalog, JSON component import, an
A3 schematic canvas with placement/editing, `.fal` project save/load, and a
millimetre cursor/snap/pin status readout.

> This is a native Qt desktop application, **not** a web app.

## Requirements

- CMake ≥ 3.21, Ninja
- Qt 6 (Core, Gui, Widgets, Sql, Test)
- A C++17 compiler
- SQLite (via Qt's `QSQLITE` driver)

On Debian/Ubuntu:

```bash
sudo apt-get install cmake ninja-build qt6-base-dev qt6-base-dev-tools \
                     libgl1-mesa-dev libxkbcommon-dev
```

## Build

```bash
cd ArduLab
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

## Run

```bash
./build/ardulab
```

On first launch the catalog database is created automatically under the OS
user-data location (e.g. `~/.local/share/ArduLab/ArduLab/catalog.db` on Linux)
and is seeded with the bundled example components. Import your own via
**Berkas → Impor Komponen (JSON)** using files like those in
`examples/components/`.

### Usage

- **Place**: drag a component from the left catalog onto the canvas, or
  double-click it.
- **Move**: drag on the canvas (snaps to the 2.54 mm grid).
- **Rotate**: select, then `R` (or Edit → Putar 90°).
- **Delete**: select, then `Del`.
- **Edit properties**: reference/value in the right panel.
- **Undo/redo**: `Ctrl+Z` / `Ctrl+Shift+Z`.
- **Zoom/pan**: mouse wheel to zoom, middle-drag to pan.
- **Status bar**: cursor position (mm), grid-snapped position, and the nearest
  pin within a screen-pixel tolerance (comfortable at any zoom).
- **Project**: New / Open / Save / Save As (`.fal`). Unsaved changes prompt on
  close.

## Tests

```bash
cd build && ctest --output-on-failure
```

Covers geometry (rotation, snapping incl. negatives, pin transforms), catalog
persistence, idempotent migration, import validation/duplicate/atomicity, and
project save/load round-trip with definition snapshots.

## Layout

```
src/
  core/        Result/Status, IDs, mm units, EventBus, logging
  component/   ComponentDefinition, Pin, Instance, Geometry
  catalog/     Database, Migrations, CatalogRepository (all SQL here)
  import/      ComponentImporter + ImportReport (JSON v1.0 validation)
  project/     Project + ProjectSerializer (.fal)
  canvas/      CanvasScene, CanvasView, ComponentItem
  commands/    QUndoCommands
  ui/          MainWindow, ComponentBrowser, PropertyPanel, ImportReportDialog
docs/          schema v1.0 + architecture decisions
examples/      example components + a sample .fal project
tests/         Qt Test suites
```

See `docs/component-schema-v1.0.md` and `docs/ADR.md`.

## Verification status

- Linux / GCC 12 / Qt 6.4 + headless unit tests: **verified**.
- Windows / MSVC: **belum diverifikasi** (not built or tested on Windows).
- Interactive GUI: layout verified via offscreen capture; full interactive
  testing on a real desktop session is pending.
