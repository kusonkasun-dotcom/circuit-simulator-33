# Canonical Component JSON — Schema v1.0

ArduLab imports component *definitions* from JSON. A file may contain:

- a single component object, **or**
- a bare array of component objects, **or**
- a wrapper object `{ "schemaVersion": "1.0", "components": [ ... ] }`.

Every imported component enters the catalog as **DRAFT** (any `status` in the
file is ignored, with a warning).

## Fields

| Field | Type | Required | Notes |
|-------|------|----------|-------|
| `schemaVersion` | string | yes | must be `"1.0"` |
| `id` | string | yes | canonical `CATEGORY-ID-VARIANT-PART-PACKAGE` (5 non-empty `[A-Za-z0-9]` segments) |
| `name` | string | yes | display name |
| `category` | string | yes | used for filtering |
| `value` | string | no | default value (e.g. `10k`) |
| `prefix` | string | no | reference designator prefix (e.g. `R`, `C`, `D`); default `U` |
| `physical` | object | yes | `{ "widthMm": >0, "heightMm": >0 }` |
| `visual` | object | no | `{ "primitives": [ ... ] }` — see below |
| `pins` | array | yes | at least one; pin `id` unique within the component |

All coordinates are in **millimetres**, in the component local frame with the
origin at the component reference point.

### Pin object

| Field | Type | Required | Notes |
|-------|------|----------|-------|
| `id` | string | yes | unique within the component |
| `name` | string | no | defaults to `id` |
| `type` | string | no | `passive`,`input`,`output`,`bidirectional`,`power`,`ground`,`no_connect` |
| `anchorPosVisual` | `{xMm,yMm}` | yes | where the pin graphic ends |
| `connectionPointVisual` | `{xMm,yMm}` | no | where wires attach; defaults to the anchor |

### Visual primitives

Each primitive is `{ "type": <kind>, ...params }` with mm coordinates:

- `rect` : `xMm, yMm, widthMm, heightMm`
- `line` : `x1Mm, y1Mm, x2Mm, y2Mm`
- `circle` : `xMm, yMm, radiusMm`
- `ellipse` : `xMm, yMm, widthMm, heightMm`
- `polyline` : `pointsMm: [ {xMm,yMm}, ... ]`

If `visual` is omitted the component is drawn as its `physical` rectangle.

## Validation & import policy

- Missing/mistyped required fields → the component is **rejected** with a clear
  message; the rest of the file still imports.
- `id` already present in the catalog (or duplicated within the same file) →
  **skipped** and reported (skip policy).
- Accepted components for one file are inserted in a **single transaction**
  (all-or-nothing), so a DB failure never leaves a partial import.
- Malformed JSON / missing file → a file-level error is reported; the catalog
  is left untouched and the app never crashes.

## Definition vs. instance

The catalog stores **definitions** (the single source of truth for a part).
When a component is placed in a project, the project stores a **snapshot** of
the definition alongside the instance (id, reference, value, position,
rotation). Editing or deleting a catalog entry therefore never silently
mutates a saved circuit. Legacy formats are intentionally **not** supported in
v1.0 (no real samples exist); the importer is structured so a legacy adapter
can be added later without touching the canonical path.
