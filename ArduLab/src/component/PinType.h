#pragma once
#include <QString>

namespace ardulab {

enum class PinType {
    Passive,
    Input,
    Output,
    Bidirectional,
    Power,
    Ground,
    NoConnect,
    Unknown
};

inline PinType pinTypeFromString(const QString& s) {
    const QString v = s.trimmed().toLower();
    if (v == "passive") return PinType::Passive;
    if (v == "input" || v == "in") return PinType::Input;
    if (v == "output" || v == "out") return PinType::Output;
    if (v == "bidirectional" || v == "bidir" || v == "io") return PinType::Bidirectional;
    if (v == "power" || v == "pwr") return PinType::Power;
    if (v == "ground" || v == "gnd") return PinType::Ground;
    if (v == "no_connect" || v == "nc") return PinType::NoConnect;
    return PinType::Unknown;
}

inline QString pinTypeToString(PinType t) {
    switch (t) {
        case PinType::Passive: return "passive";
        case PinType::Input: return "input";
        case PinType::Output: return "output";
        case PinType::Bidirectional: return "bidirectional";
        case PinType::Power: return "power";
        case PinType::Ground: return "ground";
        case PinType::NoConnect: return "no_connect";
        default: return "unknown";
    }
}

} // namespace ardulab
