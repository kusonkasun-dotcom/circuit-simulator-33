#include "ui/ComponentBrowser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QDrag>
#include <QMimeData>
#include <QJsonDocument>

namespace ardulab {

static const char* kMimeType = "application/x-ardulab-component";
static const int kJsonRole = Qt::UserRole + 1;

// List widget that starts a drag carrying the component definition JSON.
class ComponentListWidget : public QListWidget {
public:
    using QListWidget::QListWidget;
protected:
    void startDrag(Qt::DropActions) override {
        QListWidgetItem* it = currentItem();
        if (!it) return;
        auto* mime = new QMimeData();
        mime->setData(kMimeType, it->data(kJsonRole).toByteArray());
        auto* drag = new QDrag(this);
        drag->setMimeData(mime);
        drag->exec(Qt::CopyAction);
    }
};

ComponentBrowser::ComponentBrowser(CatalogRepository* repo, QWidget* parent)
    : QWidget(parent), repo_(repo) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);

    layout->addWidget(new QLabel(tr("Cari komponen")));
    search_ = new QLineEdit(this);
    search_->setPlaceholderText(tr("nama / kategori / id"));
    search_->setObjectName("catalog-search");
    layout->addWidget(search_);

    auto* filters = new QHBoxLayout();
    category_ = new QComboBox(this);
    status_ = new QComboBox(this);
    status_->addItem(tr("Semua status"), "ALL");
    status_->addItem(tr("DRAFT"), "DRAFT");
    status_->addItem(tr("VALIDATED"), "VALIDATED");
    filters->addWidget(category_);
    filters->addWidget(status_);
    layout->addLayout(filters);

    list_ = new ComponentListWidget(this);
    list_->setObjectName("catalog-list");
    list_->setDragEnabled(true);
    list_->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(list_, 1);

    connect(search_, &QLineEdit::textChanged, this,
            &ComponentBrowser::applyFilterAndReload);
    connect(category_, &QComboBox::currentTextChanged, this,
            &ComponentBrowser::applyFilterAndReload);
    connect(status_, &QComboBox::currentTextChanged, this,
            &ComponentBrowser::applyFilterAndReload);
    connect(list_, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem* it) {
                emit placeRequested(it->data(kJsonRole).toByteArray());
            });

    refresh();
}

void ComponentBrowser::refresh() {
    // Rebuild the category combo preserving selection.
    const QString curCat = category_->currentData().toString();
    category_->blockSignals(true);
    category_->clear();
    category_->addItem(tr("Semua kategori"), "");
    for (const QString& c : repo_->categories())
        category_->addItem(c, c);
    int idx = category_->findData(curCat);
    category_->setCurrentIndex(idx >= 0 ? idx : 0);
    category_->blockSignals(false);
    applyFilterAndReload();
}

void ComponentBrowser::applyFilterAndReload() {
    CatalogFilter f;
    f.text = search_->text().trimmed();
    f.category = category_->currentData().toString();
    const QString st = status_->currentData().toString();
    if (st == "DRAFT" || st == "VALIDATED") {
        f.onlyStatus = true;
        f.status = statusFromString(st);
    }

    list_->clear();
    for (const ComponentDefinition& def : repo_->search(f)) {
        auto* item = new QListWidgetItem(
            QString("%1  —  %2 [%3]")
                .arg(def.name, def.category, statusToString(def.status)));
        item->setToolTip(QString("id: %1\nvalue: %2\npins: %3")
                             .arg(def.id, def.value)
                             .arg(def.pins.size()));
        item->setData(kJsonRole,
            QJsonDocument(def.toJson()).toJson(QJsonDocument::Compact));
        if (def.status == ComponentStatus::Draft)
            item->setForeground(QColor(150, 90, 0));
        list_->addItem(item);
    }
}

} // namespace ardulab
