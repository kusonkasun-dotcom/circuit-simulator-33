#pragma once
#include <QString>
#include <QUuid>

namespace ardulab {

// Stable identity for a catalog component *definition*. The string carries the
// canonical form CATEGORY-ID-VARIANT-PART-PACKAGE.
struct ComponentDefId {
    QString value;
    ComponentDefId() = default;
    explicit ComponentDefId(QString v) : value(std::move(v)) {}
    bool isValid() const { return !value.isEmpty(); }
    bool operator==(const ComponentDefId& o) const { return value == o.value; }
};

// Stable identity for a component *instance* placed inside a project. Never
// reused, never derived from the definition id.
struct InstanceId {
    QString value;
    InstanceId() = default;
    explicit InstanceId(QString v) : value(std::move(v)) {}
    static InstanceId generate() {
        return InstanceId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
    bool isValid() const { return !value.isEmpty(); }
    bool operator==(const InstanceId& o) const { return value == o.value; }
};

} // namespace ardulab
