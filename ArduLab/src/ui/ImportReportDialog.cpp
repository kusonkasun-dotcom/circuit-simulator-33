#include "ui/ImportReportDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QDialogButtonBox>
#include <QHeaderView>

namespace ardulab {

static QString outcomeText(ImportOutcome o) {
    switch (o) {
        case ImportOutcome::Imported: return "IMPOR";
        case ImportOutcome::SkippedDuplicate: return "DILEWATI";
        default: return "DITOLAK";
    }
}
static QColor outcomeColor(ImportOutcome o) {
    switch (o) {
        case ImportOutcome::Imported: return QColor(20, 130, 60);
        case ImportOutcome::SkippedDuplicate: return QColor(170, 120, 0);
        default: return QColor(190, 40, 40);
    }
}

ImportReportDialog::ImportReportDialog(const ImportReport& report,
                                       QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Laporan Impor Komponen"));
    resize(640, 440);
    auto* layout = new QVBoxLayout(this);

    auto* summary = new QLabel(report.summaryLine());
    summary->setObjectName("import-summary");
    QFont f = summary->font(); f.setBold(true); summary->setFont(f);
    summary->setWordWrap(true);
    layout->addWidget(summary);

    if (!report.sourcePath.isEmpty())
        layout->addWidget(new QLabel(tr("Sumber: %1").arg(report.sourcePath)));

    if (report.fileError) {
        auto* err = new QLabel(report.fileErrorMessage);
        err->setStyleSheet("color:#b02020;");
        err->setWordWrap(true);
        layout->addWidget(err);
    } else {
        auto* tree = new QTreeWidget();
        tree->setObjectName("import-tree");
        tree->setColumnCount(3);
        tree->setHeaderLabels({tr("Hasil"), tr("ID"), tr("Keterangan")});
        tree->header()->setSectionResizeMode(2, QHeaderView::Stretch);
        for (const ImportEntry& e : report.entries) {
            auto* it = new QTreeWidgetItem(tree);
            it->setText(0, outcomeText(e.outcome));
            it->setForeground(0, outcomeColor(e.outcome));
            it->setText(1, e.id);
            it->setText(2, e.message);
            for (const QString& w : e.warnings) {
                auto* wc = new QTreeWidgetItem(it);
                wc->setText(2, tr("⚠ %1").arg(w));
                wc->setForeground(2, QColor(150, 110, 0));
            }
        }
        tree->expandAll();
        tree->resizeColumnToContents(0);
        tree->resizeColumnToContents(1);
        layout->addWidget(tree, 1);
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);
}

} // namespace ardulab
