# Architecture Decision Records — ArduLab

Short notes on decisions that are expensive to reverse.

## ADR-001 — Layered modules, domain independent of Qt Widgets
The domain (core, component, catalog, import, project) is compiled as a static
library `ardulab_domain` that links only `Qt6::Core` and `Qt6::Sql`. Widgets
(canvas, ui, commands) live in the `ardulab` executable. This keeps SQL and
business rules out of the UI and lets the headless tests exercise real domain
code without a display.

## ADR-002 — Millimetres everywhere in the domain
All domain coordinates are `Mm` (double) in a `PointMM`. The canvas maps 1
scene unit = 1 mm. Pixels only appear in the view layer (zoom, pin-pick
tolerance).

## ADR-003 — Result/Status instead of exceptions across module boundaries
Fallible operations return `Result<T>` / `Status` with a stable machine code
and a human message. Failures are explicit and never thrown across modules.

## ADR-004 — Catalog in per-user SQLite via Qt AppDataLocation
The catalog lives at `AppDataLocation/catalog.db`, created on first use — never
in the install directory. Schema is versioned with SQLite `PRAGMA user_version`
plus a `schema_migrations` history table. Migrations are forward-only and
idempotent; each runs in its own transaction.

## ADR-005 — Definition snapshot inside the project
A placed instance stores a full copy of its component definition. The project
stays openable even if the catalog entry changes or is removed. Updating a
catalog definition into existing instances will be an explicit action in a
later phase, never a silent mutation.

## ADR-006 — Import: reject-per-component, skip duplicates, atomic accepted set
Invalid components are rejected individually; duplicates (in catalog or within
the file) are skipped and reported; the accepted set is inserted in one
transaction. Imported components are always DRAFT.

## ADR-007 — .fal is structured JSON (v1.0), written atomically
Projects are human-diffable JSON with a `format`/`schemaVersion` marker, saved
via `QSaveFile` (temp + rename). A corrupt file yields a clear error and never
overwrites the in-memory project.

## ADR-008 — Undo/redo via QUndoStack over the Project as source of truth
Placement/move/rotate/delete/property changes are `QUndoCommand`s that mutate
the `Project` and mirror onto the scene. Dirty state derives from the stack's
clean marker.
