#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

// Builds a minimal but valid canonical v1.0 component object for tests.
inline QJsonObject makeValidComponent(const QString& id,
                                      const QString& name = "Test Part",
                                      const QString& category = "Resistor") {
    QJsonObject phys{{"widthMm", 10.0}, {"heightMm", 2.0}};
    QJsonObject pin1{
        {"id", "1"}, {"name", "1"}, {"type", "passive"},
        {"anchorPosVisual", QJsonObject{{"xMm", -5.0}, {"yMm", 0.0}}},
        {"connectionPointVisual", QJsonObject{{"xMm", -5.0}, {"yMm", 0.0}}}};
    QJsonObject pin2{
        {"id", "2"}, {"name", "2"}, {"type", "passive"},
        {"anchorPosVisual", QJsonObject{{"xMm", 5.0}, {"yMm", 0.0}}},
        {"connectionPointVisual", QJsonObject{{"xMm", 5.0}, {"yMm", 0.0}}}};
    return QJsonObject{
        {"schemaVersion", "1.0"},
        {"id", id},
        {"name", name},
        {"category", category},
        {"value", "10k"},
        {"prefix", "R"},
        {"physical", phys},
        {"pins", QJsonArray{pin1, pin2}}
    };
}
