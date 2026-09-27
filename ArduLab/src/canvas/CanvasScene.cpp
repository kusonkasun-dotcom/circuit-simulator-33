#include "canvas/CanvasScene.h"
#include "canvas/ComponentItem.h"
#include "component/Geometry.h"
#include <QPainter>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMimeData>

namespace ardulab {

static const char* kMimeType = "application/x-ardulab-component";

CanvasScene::CanvasScene(QObject* parent) : QGraphicsScene(parent) {
    setCanvasSize(widthMm_, heightMm_);
    connect(this, &QGraphicsScene::selectionChanged, this, [this]() {
        const auto sel = selectedItems();
        if (sel.isEmpty()) {
            emit selectionChangedTo(InstanceId(), false);
        } else if (auto* ci = dynamic_cast<ComponentItem*>(sel.first())) {
            emit selectionChangedTo(ci->instanceId(), true);
        }
    });
}

void CanvasScene::setCanvasSize(Mm w, Mm h) {
    widthMm_ = w; heightMm_ = h;
    const qreal margin = 20.0;
    setSceneRect(-margin, -margin, w + 2 * margin, h + 2 * margin);
    update();
}

ComponentItem* CanvasScene::itemFor(const InstanceId& id) const {
    return items_.value(id.value, nullptr);
}

ComponentItem* CanvasScene::addInstanceItem(const ComponentInstance& inst) {
    auto* item = new ComponentItem(inst);
    addItem(item);
    items_.insert(inst.instanceId.value, item);
    return item;
}

void CanvasScene::removeInstanceItem(const InstanceId& id) {
    if (auto* item = items_.take(id.value)) {
        removeItem(item);
        delete item;
    }
}

void CanvasScene::updateInstanceItem(const ComponentInstance& inst) {
    if (auto* item = items_.value(inst.instanceId.value))
        item->applyInstance(inst);
}

void CanvasScene::clearInstances() {
    for (auto* item : items_) { removeItem(item); delete item; }
    items_.clear();
}

QVector<QPair<QPointF, QString>> CanvasScene::allPinPositions() const {
    QVector<QPair<QPointF, QString>> out;
    for (auto* item : items_) out += item->pinWorldPositions();
    return out;
}

QVector<InstanceId> CanvasScene::selectedInstanceIds() const {
    QVector<InstanceId> ids;
    for (auto* it : selectedItems())
        if (auto* ci = dynamic_cast<ComponentItem*>(it))
            ids.append(ci->instanceId());
    return ids;
}

void CanvasScene::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(250, 250, 248));

    // Sheet (A3) area.
    const QRectF sheet(0, 0, widthMm_, heightMm_);
    painter->fillRect(sheet, Qt::white);

    // Grid dots/lines restricted to the sheet.
    painter->setClipRect(sheet.intersected(rect));
    QPen minor(QColor(228, 228, 228)); minor.setWidthF(0.05);
    QPen major(QColor(205, 205, 205)); major.setWidthF(0.1);
    const qreal g = gridMm_;
    const int majorEvery = 4; // heavier line every 4 grid steps (~1cm)
    int i = 0;
    for (qreal x = 0; x <= widthMm_ + 1e-6; x += g, ++i) {
        painter->setPen((i % majorEvery == 0) ? major : minor);
        painter->drawLine(QPointF(x, 0), QPointF(x, heightMm_));
    }
    i = 0;
    for (qreal y = 0; y <= heightMm_ + 1e-6; y += g, ++i) {
        painter->setPen((i % majorEvery == 0) ? major : minor);
        painter->drawLine(QPointF(0, y), QPointF(widthMm_, y));
    }
    painter->setClipping(false);

    QPen border(QColor(120, 120, 120)); border.setWidthF(0.3);
    painter->setPen(border);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sheet);
}

void CanvasScene::dragEnterEvent(QGraphicsSceneDragDropEvent* e) {
    if (e->mimeData()->hasFormat(kMimeType)) e->acceptProposedAction();
    else e->ignore();
}
void CanvasScene::dragMoveEvent(QGraphicsSceneDragDropEvent* e) {
    if (e->mimeData()->hasFormat(kMimeType)) e->acceptProposedAction();
    else e->ignore();
}
void CanvasScene::dropEvent(QGraphicsSceneDragDropEvent* e) {
    if (!e->mimeData()->hasFormat(kMimeType)) { e->ignore(); return; }
    const QByteArray json = e->mimeData()->data(kMimeType);
    const PointMM snapped = geometry::snapToGrid(
        PointMM(e->scenePos().x(), e->scenePos().y()), gridMm_);
    emit componentDropped(json, QPointF(snapped.x, snapped.y));
    e->acceptProposedAction();
}

void CanvasScene::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    pressPositions_.clear();
    QGraphicsScene::mousePressEvent(e);
    for (auto* it : selectedItems())
        if (auto* ci = dynamic_cast<ComponentItem*>(it))
            pressPositions_.insert(ci->instanceId().value, ci->instance().position);
}

void CanvasScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    QGraphicsScene::mouseReleaseEvent(e);
    for (auto* it : selectedItems()) {
        auto* ci = dynamic_cast<ComponentItem*>(it);
        if (!ci) continue;
        const QString key = ci->instanceId().value;
        if (!pressPositions_.contains(key)) continue;
        const PointMM oldPos = pressPositions_.value(key);
        // Snap the raw item position from the interactive drag.
        const PointMM raw(ci->pos().x(), ci->pos().y());
        const PointMM snapped = geometry::snapToGrid(raw, gridMm_);
        if (!snapped.nearlyEquals(oldPos))
            emit instanceMoved(ci->instanceId(), oldPos, snapped);
        else
            ci->setPos(oldPos.x, oldPos.y); // undo sub-grid jitter
    }
    pressPositions_.clear();
}

} // namespace ardulab
