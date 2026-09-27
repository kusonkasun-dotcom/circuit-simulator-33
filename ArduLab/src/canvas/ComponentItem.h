#pragma once
#include "component/ComponentInstance.h"
#include <QGraphicsItem>
#include <QVector>
#include <QPair>

namespace ardulab {

// A QGraphicsItem that renders a ComponentInstance in scene millimetres.
// The item origin (0,0) maps to the instance origin; rotation uses the item's
// own transform so it stays consistent with geometry::rotateLocal.
class ComponentItem : public QGraphicsItem {
public:
    explicit ComponentItem(const ComponentInstance& inst);

    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

    const InstanceId& instanceId() const { return inst_.instanceId; }
    const ComponentInstance& instance() const { return inst_; }

    void applyInstance(const ComponentInstance& inst);

    // World (scene, mm) positions of every pin, with a display label.
    QVector<QPair<QPointF, QString>> pinWorldPositions() const;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

private:
    ComponentInstance inst_;
    QRectF bounds_;
    void recomputeBounds();
};

} // namespace ardulab
