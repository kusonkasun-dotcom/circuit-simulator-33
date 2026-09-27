#pragma once
#include "core/Ids.h"
#include "core/Units.h"
#include "component/ComponentDefinition.h"
#include <QJsonObject>

namespace ardulab {

// A placed component inside a project. It carries a *snapshot* of the
// definition so the project remains openable even if the catalog entry later
// changes or is deleted.
class ComponentInstance {
public:
    InstanceId instanceId;
    ComponentDefId defId;
    ComponentDefinition definition; // snapshot

    QString reference;              // e.g. R1
    QString value;                  // overrides definition.value when non-empty
    PointMM position;               // origin in mm on the canvas
    int rotation = 0;               // 0/90/180/270

    ComponentInstance() = default;

    static ComponentInstance fromDefinition(const ComponentDefinition& def,
                                            const QString& reference,
                                            const PointMM& pos);

    // World position of a given pin taking position + rotation into account.
    PointMM pinWorldPos(const Pin& pin) const;

    QString effectiveValue() const {
        return value.isEmpty() ? definition.value : value;
    }

    QJsonObject toJson() const;
    static Result<ComponentInstance> fromJson(const QJsonObject& obj);
};

} // namespace ardulab
