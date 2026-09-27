#pragma once
#include "catalog/CatalogRepository.h"
#include <QWidget>

class QLineEdit;
class QComboBox;
class QListWidget;

namespace ardulab {

// Left panel: search + filter + a draggable list of catalog components.
// Talks to the catalog only through CatalogRepository (no SQL here).
class ComponentBrowser : public QWidget {
    Q_OBJECT
public:
    explicit ComponentBrowser(CatalogRepository* repo, QWidget* parent = nullptr);

public slots:
    void refresh();

signals:
    // Double-click / Enter : ask the main window to place this definition.
    void placeRequested(const QByteArray& definitionJson);

private:
    void applyFilterAndReload();
    CatalogRepository* repo_;
    QLineEdit* search_;
    QComboBox* category_;
    QComboBox* status_;
    QListWidget* list_;
};

} // namespace ardulab
