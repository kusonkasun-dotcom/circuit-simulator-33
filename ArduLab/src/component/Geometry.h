#pragma once
#include "core/Units.h"

namespace ardulab {
namespace geometry {

// Normalise an arbitrary rotation to one of 0/90/180/270 degrees.
int normalizeRotation(int degrees);

// Rotate a point expressed in a component's local mm frame around the origin.
// Screen convention (y grows downwards): a +90 degree rotation is clockwise
// on screen. Multiples of 90 are handled exactly (no floating error).
PointMM rotateLocal(const PointMM& local, int degrees);

// World position of a pin: component origin + rotated local pin position.
PointMM transformPin(const PointMM& componentOrigin, int rotationDeg,
                     const PointMM& pinLocal);

// Snap a point to the nearest grid intersection. Works for negative
// coordinates too.
PointMM snapToGrid(const PointMM& p, Mm gridMm);

} // namespace geometry
} // namespace ardulab
