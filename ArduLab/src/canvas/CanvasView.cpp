#include "canvas/CanvasView.h"
#include "canvas/CanvasScene.h"
#include "component/Geometry.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <cmath>

namespace ardulab {

CanvasView::CanvasView(CanvasScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent), canvas_(scene) {
    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::RubberBandDrag);
    setMouseTracking(true);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    // Initial scale: mm -> pixels so an A3 sheet is comfortably visible.
    scale(2.2, 2.2);
}

void CanvasView::resetZoom() {
    resetTransform();
    scale(2.2, 2.2);
}

void CanvasView::applyZoom(qreal factor) {
    const qreal cur = transform().m11();
    const qreal next = cur * factor;
    if (next < 0.3 || next > 40.0) return;
    scale(factor, factor);
}

void CanvasView::wheelEvent(QWheelEvent* e) {
    // Wheel zooms about the cursor (common in EDA canvases).
    applyZoom(e->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15);
    e->accept();
}

void CanvasView::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::MiddleButton) {
        panning_ = true;
        lastPan_ = e->pos();
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }
    QGraphicsView::mousePressEvent(e);
}

void CanvasView::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::MiddleButton && panning_) {
        panning_ = false;
        setCursor(Qt::ArrowCursor);
        e->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(e);
}

void CanvasView::mouseMoveEvent(QMouseEvent* e) {
    if (panning_) {
        const QPoint delta = e->pos() - lastPan_;
        lastPan_ = e->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        e->accept();
        return;
    }

    const QPointF scenePt = mapToScene(e->pos());
    const PointMM raw(scenePt.x(), scenePt.y());
    const PointMM snapped = geometry::snapToGrid(raw, canvas_->gridMm());

    // Convert pixel tolerance to scene mm using the current view scale.
    const qreal scale = transform().m11();
    const qreal tolMM = (scale > 1e-6) ? (pinTolPixels_ / scale) : pinTolPixels_;

    bool hasPin = false;
    QString label;
    PointMM pinPos;
    qreal best = tolMM;
    for (const auto& pin : canvas_->allPinPositions()) {
        const qreal d = raw.distanceTo(PointMM(pin.first.x(), pin.first.y()));
        if (d <= best) {
            best = d;
            hasPin = true;
            label = pin.second;
            pinPos = PointMM(pin.first.x(), pin.first.y());
        }
    }

    emit cursorReadout(raw, snapped, hasPin, label, pinPos);
    QGraphicsView::mouseMoveEvent(e);
}

} // namespace ardulab
