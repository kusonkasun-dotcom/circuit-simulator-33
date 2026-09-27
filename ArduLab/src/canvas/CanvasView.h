#pragma once
#include "core/Units.h"
#include <QGraphicsView>

namespace ardulab {

class CanvasScene;

// Handles zoom (wheel), pan (middle/space drag), and computes the cursor
// readout: raw mm, grid-snapped mm, and the nearest pin within a screen-pixel
// tolerance (so it stays comfortable at every zoom level).
class CanvasView : public QGraphicsView {
    Q_OBJECT
public:
    explicit CanvasView(CanvasScene* scene, QWidget* parent = nullptr);

    void zoomIn()  { applyZoom(1.2); }
    void zoomOut() { applyZoom(1.0 / 1.2); }
    void resetZoom();

signals:
    void cursorReadout(PointMM rawMM, PointMM snappedMM,
                       bool hasPin, QString pinLabel, PointMM pinPos);

protected:
    void wheelEvent(QWheelEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    void applyZoom(qreal factor);
    CanvasScene* canvas_;
    qreal pinTolPixels_ = 12.0;
    bool panning_ = false;
    QPoint lastPan_;
};

} // namespace ardulab
