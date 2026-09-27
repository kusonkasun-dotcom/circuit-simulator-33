#pragma once
#include "core/Units.h"
#include "core/Result.h"
#include "component/PinType.h"
#include <QString>
#include <QVector>
#include <QVariantMap>
#include <QJsonObject>

namespace ardulab {

struct Pin {
    QString id;                 // unique within the component
    QString name;
    PinType type = PinType::Passive;
    PointMM anchor;             // anchorPosVisual : where the pin graphic ends
    PointMM connectionPoint;    // connectionPointVisual : where wires attach
};

// A drawing primitive expressed in the component local mm frame. Kept generic
// (type + params) so new shapes can be added without schema churn.
struct VisualPrimitive {
    QString type;               // line | rect | circle | ellipse | polyline | text
    QVariantMap params;         // mm coordinates keyed by name (xMm, x1Mm, ...)
};

enum class ComponentStatus { Draft, Validated };

QString statusToString(ComponentStatus s);
ComponentStatus statusFromString(const QString& s);

// A catalog *definition*. This is the single source of truth for what a part
// looks like and which pins it exposes. Project instances snapshot a copy of
// this so later catalog edits never silently mutate saved circuits.
class ComponentDefinition {
public:
    QString schemaVersion = "1.0";
    QString id;                 // CATEGORY-ID-VARIANT-PART-PACKAGE
    QString name;
    QString category;
    QString value;
    QString prefix = "U";       // reference designator prefix
    ComponentStatus status = ComponentStatus::Draft;
    QString source;             // origin (import file / "builtin")

    Mm widthMm = 0.0;
    Mm heightMm = 0.0;

    QVector<VisualPrimitive> visual;
    QVector<Pin> pins;

    // Strict parse+validation of a canonical v1.0 component object. On failure
    // returns an Error describing the first problem. Warnings (non-fatal) are
    // appended to `warnings` when provided.
    static Result<ComponentDefinition> fromJson(const QJsonObject& obj,
                                                 QStringList* warnings = nullptr);

    QJsonObject toJson() const;

    // Validate that the canonical id has 5 non-empty segments.
    static bool isValidId(const QString& id);
};

} // namespace ardulab
