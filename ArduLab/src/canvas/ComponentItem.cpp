#include "canvas/ComponentItem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace ardulab {

ComponentItem::ComponentItem(const ComponentInstance& inst) : inst_(inst) {
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setPos(inst_.position.x, inst_.position.y);
    setRotation(inst_.rotation);
    recomputeBounds();
}

void ComponentItem::applyInstance(const ComponentInstance& inst) {
    prepareGeometryChange();
    inst_ = inst;
    setPos(inst_.position.x, inst_.position.y);
    setRotation(inst_.rotation);
    recomputeBounds();
    update();
}

void ComponentItem::recomputeBounds() {
    qreal minx = -inst_.definition.widthMm / 2.0;
    qreal miny = -inst_.definition.heightMm / 2.0;
    qreal maxx =  inst_.definition.widthMm / 2.0;
    qreal maxy =  inst_.definition.heightMm / 2.0;
    auto expand = [&](qreal x, qreal y) {
        minx = qMin(minx, x); miny = qMin(miny, y);
        maxx = qMax(maxx, x); maxy = qMax(maxy, y);
    };
    for (const Pin& pin : inst_.definition.pins) {
        expand(pin.anchor.x, pin.anchor.y);
        expand(pin.connectionPoint.x, pin.connectionPoint.y);
    }
    const qreal margin = 2.0; // mm, room for labels/pin dots
    bounds_ = QRectF(QPointF(minx - margin, miny - margin),
                     QPointF(maxx + margin, maxy + margin));
}

QRectF ComponentItem::boundingRect() const { return bounds_; }

QVector<QPair<QPointF, QString>> ComponentItem::pinWorldPositions() const {
    QVector<QPair<QPointF, QString>> out;
    for (const Pin& pin : inst_.definition.pins) {
        const PointMM w = inst_.pinWorldPos(pin);
        out.append({ QPointF(w.x, w.y),
                     QString("%1:%2").arg(inst_.reference, pin.name) });
    }
    return out;
}

void ComponentItem::paint(QPainter* p, const QStyleOptionGraphicsItem* opt,
                          QWidget*) {
    p->setRenderHint(QPainter::Antialiasing, true);
    const bool selected = opt->state & QStyle::State_Selected;
    const bool draft = inst_.definition.status == ComponentStatus::Draft;

    QColor bodyColor = draft ? QColor(255, 244, 214) : QColor(232, 240, 255);
    QColor lineColor = draft ? QColor(180, 120, 0) : QColor(30, 60, 120);
    if (selected) lineColor = QColor(220, 40, 40);

    QPen pen(lineColor);
    pen.setWidthF(0.22);
    pen.setJoinStyle(Qt::RoundJoin);
    p->setPen(pen);
    p->setBrush(bodyColor);

    // Draw primitives; fall back to a body rectangle if none provided.
    if (inst_.definition.visual.isEmpty()) {
        p->drawRect(QRectF(-inst_.definition.widthMm / 2.0,
                           -inst_.definition.heightMm / 2.0,
                           inst_.definition.widthMm, inst_.definition.heightMm));
    } else {
        for (const VisualPrimitive& prim : inst_.definition.visual) {
            const QVariantMap& m = prim.params;
            auto d = [&](const char* k) { return m.value(k).toDouble(); };
            if (prim.type == "rect") {
                p->drawRect(QRectF(d("xMm"), d("yMm"), d("widthMm"), d("heightMm")));
            } else if (prim.type == "line") {
                QPen lp = pen; p->setBrush(Qt::NoBrush);
                p->drawLine(QPointF(d("x1Mm"), d("y1Mm")),
                            QPointF(d("x2Mm"), d("y2Mm")));
                p->setBrush(bodyColor);
            } else if (prim.type == "circle") {
                const qreal r = d("radiusMm");
                p->drawEllipse(QPointF(d("xMm"), d("yMm")), r, r);
            } else if (prim.type == "ellipse") {
                p->drawEllipse(QRectF(d("xMm"), d("yMm"), d("widthMm"), d("heightMm")));
            } else if (prim.type == "polyline") {
                const QVariantList pts = m.value("pointsMm").toList();
                QPolygonF poly;
                for (const QVariant& pv : pts) {
                    const QVariantMap pm = pv.toMap();
                    poly << QPointF(pm.value("xMm").toDouble(),
                                    pm.value("yMm").toDouble());
                }
                p->setBrush(Qt::NoBrush);
                p->drawPolyline(poly);
                p->setBrush(bodyColor);
            }
        }
    }

    // Pins : short stub to connection point + a dot.
    QPen pinPen(QColor(20, 120, 60));
    pinPen.setWidthF(0.25);
    p->setPen(pinPen);
    p->setBrush(QColor(20, 150, 70));
    for (const Pin& pin : inst_.definition.pins) {
        p->drawLine(QPointF(pin.anchor.x, pin.anchor.y),
                    QPointF(pin.connectionPoint.x, pin.connectionPoint.y));
        p->drawEllipse(QPointF(pin.connectionPoint.x, pin.connectionPoint.y),
                       0.35, 0.35);
    }

    // Reference + value label above the body.
    QFont f = p->font();
    f.setPointSizeF(2.2);
    p->setFont(f);
    p->setPen(QColor(40, 40, 40));
    const qreal topY = -inst_.definition.heightMm / 2.0 - 1.5;
    p->drawText(QPointF(-inst_.definition.widthMm / 2.0, topY),
                QString("%1 %2").arg(inst_.reference, inst_.effectiveValue()));

    if (draft) {
        QFont bf = f; bf.setBold(true); p->setFont(bf);
        p->setPen(QColor(180, 100, 0));
        p->drawText(QPointF(-inst_.definition.widthMm / 2.0,
                            inst_.definition.heightMm / 2.0 + 3.0), "DRAFT");
    }
}

} // namespace ardulab
