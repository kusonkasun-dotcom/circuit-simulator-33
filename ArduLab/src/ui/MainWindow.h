#pragma once
#include "core/Units.h"
#include "core/Ids.h"
#include "project/Project.h"
#include "catalog/Database.h"
#include "catalog/CatalogRepository.h"
#include <QMainWindow>
#include <memory>

class QUndoStack;
class QLabel;

namespace ardulab {

class CanvasScene;
class CanvasView;
class ComponentBrowser;
class PropertyPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* e) override;

private slots:
    void newProject();
    void openProject();
    bool saveProject();
    bool saveProjectAs();
    void importComponents();
    void rotateSelected();
    void deleteSelected();

private:
    void setupUi();
    void setupMenusAndToolbar();
    void setupStatusBar();
    void wireSignals();
    bool initCatalog();
    void seedExamplesIfEmpty();

    void placeDefinitionJson(const QByteArray& json, PointMM pos, bool atCenter);
    void onComponentDropped(const QByteArray& json, QPointF sceneMM);
    void onInstanceMoved(const InstanceId& id, PointMM oldPos, PointMM newPos);
    void onSelectionChanged(InstanceId id, bool hasSelection);
    void onPropertyEdited(const InstanceId& id, const QString& ref, const QString& val);
    void onCursorReadout(PointMM raw, PointMM snapped, bool hasPin,
                         QString pinLabel, PointMM pinPos);

    void loadProjectIntoScene();
    bool maybeSave();
    void updateTitle();
    void refreshSelectedProperties();

    std::unique_ptr<Database> db_;
    std::unique_ptr<CatalogRepository> repo_;
    Project project_;
    QString currentPath_;

    QUndoStack* undo_ = nullptr;
    CanvasScene* scene_ = nullptr;
    CanvasView* view_ = nullptr;
    ComponentBrowser* browser_ = nullptr;
    PropertyPanel* props_ = nullptr;

    QLabel* rawLabel_ = nullptr;
    QLabel* snapLabel_ = nullptr;
    QLabel* pinLabel_ = nullptr;
    QLabel* catalogLabel_ = nullptr;

    InstanceId selectedId_;
    bool hasSelection_ = false;
};

} // namespace ardulab
