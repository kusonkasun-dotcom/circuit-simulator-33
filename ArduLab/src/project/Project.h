#pragma once
#include "component/ComponentInstance.h"
#include <QString>
#include <QVector>

namespace ardulab {

// In-memory project model. Coordinates are in mm; the canvas is A3 by default.
class Project {
public:
    QString schemaVersion = "1.0";
    QString name = "Untitled";
    Mm canvasWidthMm = 420.0;   // A3 landscape
    Mm canvasHeightMm = 297.0;

    QVector<ComponentInstance> instances;

    // Reference designator counter helper: next free "R1", "C3", ...
    QString nextReference(const QString& prefix) const;
};

} // namespace ardulab
