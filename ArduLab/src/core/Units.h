#pragma once
#include <cmath>

namespace ardulab {

// All domain coordinates are expressed in millimetres (mm). The canvas maps
// 1 scene unit == 1 mm; UI pixel concerns never leak into the domain.
using Mm = double;

struct PointMM {
    Mm x = 0.0;
    Mm y = 0.0;

    PointMM() = default;
    PointMM(Mm x_, Mm y_) : x(x_), y(y_) {}

    PointMM operator+(const PointMM& o) const { return {x + o.x, y + o.y}; }
    PointMM operator-(const PointMM& o) const { return {x - o.x, y - o.y}; }
    PointMM operator*(double s) const { return {x * s, y * s}; }

    bool nearlyEquals(const PointMM& o, Mm eps = 1e-6) const {
        return std::fabs(x - o.x) < eps && std::fabs(y - o.y) < eps;
    }

    Mm distanceTo(const PointMM& o) const {
        const Mm dx = x - o.x;
        const Mm dy = y - o.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

} // namespace ardulab
