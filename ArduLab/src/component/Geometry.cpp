#include "component/Geometry.h"
#include <cmath>

namespace ardulab {
namespace geometry {

int normalizeRotation(int degrees) {
    int d = degrees % 360;
    if (d < 0) d += 360;
    // snap to nearest of 0/90/180/270
    return ((d + 45) / 90 * 90) % 360;
}

PointMM rotateLocal(const PointMM& local, int degrees) {
    switch (normalizeRotation(degrees)) {
        case 90:  return { -local.y,  local.x };
        case 180: return { -local.x, -local.y };
        case 270: return {  local.y, -local.x };
        default:  return {  local.x,  local.y };
    }
}

PointMM transformPin(const PointMM& componentOrigin, int rotationDeg,
                     const PointMM& pinLocal) {
    return componentOrigin + rotateLocal(pinLocal, rotationDeg);
}

PointMM snapToGrid(const PointMM& p, Mm gridMm) {
    if (gridMm <= 0.0) return p;
    return { std::round(p.x / gridMm) * gridMm,
             std::round(p.y / gridMm) * gridMm };
}

} // namespace geometry
} // namespace ardulab
