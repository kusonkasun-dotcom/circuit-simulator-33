#pragma once
#include "component/ComponentInstance.h"
#include "core/Units.h"
#include <QGraphicsScene>
#include <QHash>

namespace ardulab {

class ComponentItem;

// Holds the drawing items. The Project remains the source of truth; the scene
// only mirrors it. Grid is drawn in scene mm.
class CanvasScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit CanvasScene(QObject* parent = nullptr);

    void setCanvasSize(Mm widthMm, Mm heightMm);
    Mm gridMm() const { return gridMm_; }

    ComponentItem* addInstanceItem(const ComponentInstance& inst);
    void removeInstanceItem(const InstanceId& id);
    void updateInstanceItem(const ComponentInstance& inst);
    ComponentItem* itemFor(const InstanceId& id) const;
    void clearInstances();

    // All pin world positions across every item (for snap readout).
    QVector<QPair<QPointF, QString>> allPinPositions() const;

    // Instance ids of all currently selected component items.
    QVector<InstanceId> selectedInstanceIds() const;

signals:
    // Emitted when a drag from the component browser is dropped on the canvas.
    void componentDropped(const QByteArray& definitionJson, QPointF sceneMM);
    // Emitted after an interactive move finishes (mouse released).
    void instanceMoved(const InstanceId& id, PointMM oldPos, PointMM newPos);
    void selectionChangedTo(InstanceId id, bool hasSelection);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent* e) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent* e) override;
    void dropEvent(QGraphicsSceneDragDropEvent* e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    QHash<QString, ComponentItem*> items_;
    Mm widthMm_ = 420.0;
    Mm heightMm_ = 297.0;
    Mm gridMm_ = 2.54;
    QHash<QString, PointMM> pressPositions_; // for move undo
};

} // namespace ardulab
