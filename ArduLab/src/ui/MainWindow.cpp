#include "ui/MainWindow.h"
#include "ui/ComponentBrowser.h"
#include "ui/PropertyPanel.h"
#include "ui/ImportReportDialog.h"
#include "canvas/CanvasScene.h"
#include "canvas/CanvasView.h"
#include "commands/Commands.h"
#include "import/ComponentImporter.h"
#include "project/ProjectSerializer.h"
#include "component/Geometry.h"
#include "core/EventBus.h"
#include "core/Logger.h"

#include <QApplication>
#include <QUndoStack>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QJsonDocument>
#include <QDir>
#include <QFileInfo>

namespace ardulab {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    undo_ = new QUndoStack(this);

    if (!initCatalog()) {
        // initCatalog already showed a message; still build UI so the app is
        // usable read-only-ish (but repo_ is null -> browser disabled).
    }

    setupUi();
    setupMenusAndToolbar();
    setupStatusBar();
    wireSignals();

    if (repo_) seedExamplesIfEmpty();
    if (browser_) browser_->refresh();

    loadProjectIntoScene();
    updateTitle();
    resize(1280, 820);

    // Verification helper: auto-open a project when ARDULAB_OPEN is set.
    const QByteArray openPath = qgetenv("ARDULAB_OPEN");
    if (!openPath.isEmpty()) {
        auto loaded = ProjectSerializer::load(QString::fromLocal8Bit(openPath));
        if (loaded.isOk()) {
            project_ = loaded.value();
            currentPath_ = QString::fromLocal8Bit(openPath);
            undo_->clear();
            loadProjectIntoScene();
            updateTitle();
        }
    }
}

MainWindow::~MainWindow() = default;

bool MainWindow::initCatalog() {
    const QString path = Database::defaultCatalogPath();
    auto opened = Database::open(path);
    if (opened.isError()) {
        QMessageBox::critical(this, tr("Katalog gagal dibuka"),
            tr("%1\n\nAplikasi tetap berjalan, namun katalog tidak tersedia.")
                .arg(opened.error().message));
        log::error(opened.error().message);
        return false;
    }
    db_.reset(opened.value());
    repo_ = std::make_unique<CatalogRepository>(db_->handle());
    return true;
}

void MainWindow::seedExamplesIfEmpty() {
    if (!repo_->all().isEmpty()) return;
    ComponentImporter importer(repo_.get());
    const QStringList files = {
        ":/examples/resistor.json", ":/examples/capacitor.json",
        ":/examples/led.json", ":/examples/ground.json",
        ":/examples/vsource_dc.json"
    };
    int total = 0;
    for (const QString& f : files)
        total += importer.importFile(f).imported;
    log::info(QString("Seed katalog contoh: %1 komponen").arg(total));
}

void MainWindow::setupUi() {
    scene_ = new CanvasScene(this);
    scene_->setCanvasSize(project_.canvasWidthMm, project_.canvasHeightMm);
    view_ = new CanvasView(scene_, this);
    view_->setObjectName("canvas-view");
    setCentralWidget(view_);

    auto* leftDock = new QDockWidget(tr("Katalog Komponen"), this);
    leftDock->setObjectName("dock-catalog");
    browser_ = repo_ ? new ComponentBrowser(repo_.get(), leftDock)
                     : nullptr;
    if (browser_) leftDock->setWidget(browser_);
    else leftDock->setWidget(new QLabel(tr("Katalog tidak tersedia")));
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    auto* rightDock = new QDockWidget(tr("Properti"), this);
    rightDock->setObjectName("dock-properties");
    props_ = new PropertyPanel(rightDock);
    rightDock->setWidget(props_);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);
}

void MainWindow::setupMenusAndToolbar() {
    auto* fileMenu = menuBar()->addMenu(tr("&Berkas"));
    auto* aNew = fileMenu->addAction(tr("&Baru"), QKeySequence::New, this, &MainWindow::newProject);
    auto* aOpen = fileMenu->addAction(tr("&Buka…"), QKeySequence::Open, this, &MainWindow::openProject);
    auto* aSave = fileMenu->addAction(tr("&Simpan"), QKeySequence::Save, this, &MainWindow::saveProject);
    fileMenu->addAction(tr("Simpan &Sebagai…"), QKeySequence::SaveAs, this, &MainWindow::saveProjectAs);
    fileMenu->addSeparator();
    auto* aImport = fileMenu->addAction(tr("&Impor Komponen (JSON)…"), this, &MainWindow::importComponents);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("K&eluar"), QKeySequence::Quit, this, &QWidget::close);

    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    auto* aUndo = undo_->createUndoAction(this, tr("Urungkan"));
    aUndo->setShortcut(QKeySequence::Undo);
    auto* aRedo = undo_->createRedoAction(this, tr("Ulangi"));
    aRedo->setShortcut(QKeySequence::Redo);
    editMenu->addAction(aUndo);
    editMenu->addAction(aRedo);
    editMenu->addSeparator();
    auto* aRotate = editMenu->addAction(tr("Putar 90°"), QKeySequence(Qt::Key_R), this, &MainWindow::rotateSelected);
    auto* aDelete = editMenu->addAction(tr("Hapus"), QKeySequence::Delete, this, &MainWindow::deleteSelected);

    auto* viewMenu = menuBar()->addMenu(tr("&Tampilan"));
    viewMenu->addAction(tr("Perbesar"), QKeySequence::ZoomIn, view_, &CanvasView::zoomIn);
    viewMenu->addAction(tr("Perkecil"), QKeySequence::ZoomOut, view_, &CanvasView::zoomOut);
    viewMenu->addAction(tr("Reset Zoom"), QKeySequence(Qt::CTRL | Qt::Key_0), view_, &CanvasView::resetZoom);

    auto* tb = addToolBar(tr("Utama"));
    tb->setObjectName("main-toolbar");
    tb->addAction(aNew); tb->addAction(aOpen); tb->addAction(aSave);
    tb->addSeparator();
    tb->addAction(aImport);
    tb->addSeparator();
    tb->addAction(aUndo); tb->addAction(aRedo);
    tb->addSeparator();
    tb->addAction(aRotate); tb->addAction(aDelete);
}

void MainWindow::setupStatusBar() {
    rawLabel_ = new QLabel(tr("Kursor: —"));
    snapLabel_ = new QLabel(tr("Snap: —"));
    pinLabel_ = new QLabel(tr("Pin: —"));
    catalogLabel_ = new QLabel();
    rawLabel_->setObjectName("status-cursor");
    snapLabel_->setObjectName("status-snap");
    pinLabel_->setObjectName("status-pin");
    statusBar()->addWidget(rawLabel_);
    statusBar()->addWidget(new QLabel(" | "));
    statusBar()->addWidget(snapLabel_);
    statusBar()->addWidget(new QLabel(" | "));
    statusBar()->addWidget(pinLabel_);
    statusBar()->addPermanentWidget(catalogLabel_);
    if (db_) catalogLabel_->setText(tr("Katalog: %1").arg(db_->path()));
}

void MainWindow::wireSignals() {
    connect(scene_, &CanvasScene::componentDropped, this, &MainWindow::onComponentDropped);
    connect(scene_, &CanvasScene::instanceMoved, this, &MainWindow::onInstanceMoved);
    connect(scene_, &CanvasScene::selectionChangedTo, this, &MainWindow::onSelectionChanged);
    connect(view_, &CanvasView::cursorReadout, this, &MainWindow::onCursorReadout);
    connect(props_, &PropertyPanel::propertyEdited, this, &MainWindow::onPropertyEdited);
    if (browser_)
        connect(browser_, &ComponentBrowser::placeRequested, this,
                [this](const QByteArray& json) {
                    placeDefinitionJson(json, PointMM(), true);
                });
    connect(undo_, &QUndoStack::cleanChanged, this,
            [this](bool) { updateTitle(); });
    if (browser_)
        connect(&EventBus::instance(), &EventBus::catalogChanged,
                browser_, &ComponentBrowser::refresh);
}

// ----------------------------------------------------------------------------
void MainWindow::placeDefinitionJson(const QByteArray& json, PointMM pos,
                                     bool atCenter) {
    QJsonParseError perr;
    QJsonDocument doc = QJsonDocument::fromJson(json, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::warning(this, tr("Gagal menempatkan"),
            tr("Definisi komponen tidak valid."));
        return;
    }
    auto def = ComponentDefinition::fromJson(doc.object());
    if (def.isError()) {
        QMessageBox::warning(this, tr("Gagal menempatkan"), def.error().message);
        return;
    }
    if (atCenter) {
        const QPointF c = view_->mapToScene(view_->viewport()->rect().center());
        pos = geometry::snapToGrid(PointMM(c.x(), c.y()), scene_->gridMm());
    }
    const QString ref = project_.nextReference(def.value().prefix);
    ComponentInstance inst =
        ComponentInstance::fromDefinition(def.value(), ref, pos);
    undo_->push(new AddComponentCommand(&project_, scene_, inst));
}

void MainWindow::onComponentDropped(const QByteArray& json, QPointF sceneMM) {
    placeDefinitionJson(json, PointMM(sceneMM.x(), sceneMM.y()), false);
}

void MainWindow::onInstanceMoved(const InstanceId& id, PointMM oldPos, PointMM newPos) {
    undo_->push(new MoveComponentCommand(&project_, scene_, id, oldPos, newPos));
    refreshSelectedProperties();
}

void MainWindow::onSelectionChanged(InstanceId id, bool hasSelection) {
    selectedId_ = id;
    hasSelection_ = hasSelection;
    refreshSelectedProperties();
}

void MainWindow::refreshSelectedProperties() {
    if (!hasSelection_) { props_->clearInstance(); return; }
    if (auto* inst = findInstance(project_, selectedId_))
        props_->showInstance(*inst);
    else
        props_->clearInstance();
}

void MainWindow::onPropertyEdited(const InstanceId& id, const QString& ref,
                                  const QString& val) {
    auto* inst = findInstance(project_, id);
    if (!inst) return;
    if (inst->reference == ref && inst->value == val) return;
    undo_->push(new ChangePropertyCommand(&project_, scene_, id,
        inst->reference, inst->value, ref, val));
    refreshSelectedProperties();
}

void MainWindow::onCursorReadout(PointMM raw, PointMM snapped, bool hasPin,
                                 QString pinLabel, PointMM pinPos) {
    rawLabel_->setText(tr("Kursor: %1, %2 mm")
        .arg(raw.x, 0, 'f', 2).arg(raw.y, 0, 'f', 2));
    snapLabel_->setText(tr("Snap: %1, %2 mm")
        .arg(snapped.x, 0, 'f', 2).arg(snapped.y, 0, 'f', 2));
    if (hasPin)
        pinLabel_->setText(tr("Pin: %1 @ %2, %3 mm")
            .arg(pinLabel).arg(pinPos.x, 0, 'f', 2).arg(pinPos.y, 0, 'f', 2));
    else
        pinLabel_->setText(tr("Pin: —"));
}

void MainWindow::rotateSelected() {
    if (!hasSelection_) return;
    if (auto* inst = findInstance(project_, selectedId_)) {
        const int newRot = geometry::normalizeRotation(inst->rotation + 90);
        undo_->push(new RotateComponentCommand(&project_, scene_,
            selectedId_, inst->rotation, newRot));
        refreshSelectedProperties();
    }
}

void MainWindow::deleteSelected() {
    if (!hasSelection_) return;
    undo_->push(new DeleteComponentCommand(&project_, scene_, selectedId_));
    hasSelection_ = false;
    props_->clearInstance();
}

// ---- Project lifecycle -----------------------------------------------------
void MainWindow::loadProjectIntoScene() {
    scene_->clearInstances();
    scene_->setCanvasSize(project_.canvasWidthMm, project_.canvasHeightMm);
    for (const ComponentInstance& inst : project_.instances)
        scene_->addInstanceItem(inst);
}

void MainWindow::newProject() {
    if (!maybeSave()) return;
    project_ = Project();
    currentPath_.clear();
    undo_->clear();
    loadProjectIntoScene();
    props_->clearInstance();
    hasSelection_ = false;
    updateTitle();
}

void MainWindow::openProject() {
    if (!maybeSave()) return;
    const QString path = QFileDialog::getOpenFileName(this,
        tr("Buka Proyek"), QDir::homePath(), tr("Proyek ArduLab (*.fal)"));
    if (path.isEmpty()) return;
    auto loaded = ProjectSerializer::load(path);
    if (loaded.isError()) {
        QMessageBox::critical(this, tr("Gagal membuka proyek"),
            loaded.error().message);
        return; // in-memory project untouched
    }
    project_ = loaded.value();
    currentPath_ = path;
    undo_->clear();
    loadProjectIntoScene();
    props_->clearInstance();
    hasSelection_ = false;
    updateTitle();
}

bool MainWindow::saveProject() {
    if (currentPath_.isEmpty()) return saveProjectAs();
    Status st = ProjectSerializer::save(project_, currentPath_);
    if (st.isError()) {
        QMessageBox::critical(this, tr("Gagal menyimpan"), st.error().message);
        return false;
    }
    undo_->setClean();
    updateTitle();
    return true;
}

bool MainWindow::saveProjectAs() {
    QString path = QFileDialog::getSaveFileName(this, tr("Simpan Proyek"),
        QDir::homePath() + "/" + project_.name + ".fal",
        tr("Proyek ArduLab (*.fal)"));
    if (path.isEmpty()) return false;
    if (!path.endsWith(".fal")) path += ".fal";
    currentPath_ = path;
    project_.name = QFileInfo(path).completeBaseName();
    return saveProject();
}

void MainWindow::importComponents() {
    if (!repo_) return;
    const QStringList paths = QFileDialog::getOpenFileNames(this,
        tr("Impor Komponen"), QDir::homePath(),
        tr("Komponen JSON (*.json)"));
    if (paths.isEmpty()) return;
    ComponentImporter importer(repo_.get());
    for (const QString& p : paths) {
        ImportReport report = importer.importFile(p);
        ImportReportDialog dlg(report, this);
        dlg.exec();
    }
    browser_->refresh();
}

// ---- Dirty tracking --------------------------------------------------------
bool MainWindow::maybeSave() {
    if (undo_->isClean()) return true;
    const auto ret = QMessageBox::warning(this, tr("Perubahan belum disimpan"),
        tr("Proyek memiliki perubahan yang belum disimpan. Simpan sekarang?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) return saveProject();
    if (ret == QMessageBox::Cancel) return false;
    return true; // discard
}

void MainWindow::closeEvent(QCloseEvent* e) {
    if (maybeSave()) e->accept();
    else e->ignore();
}

void MainWindow::updateTitle() {
    const QString name = currentPath_.isEmpty()
        ? tr("Untitled") : QFileInfo(currentPath_).fileName();
    setWindowModified(!undo_->isClean());
    setWindowTitle(QString("ArduLab — %1[*]").arg(name));
}

} // namespace ardulab
