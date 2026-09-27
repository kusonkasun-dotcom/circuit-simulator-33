#pragma once
#include "import/ComponentImporter.h"
#include <QDialog>

namespace ardulab {

// Modal dialog presenting an ImportReport: summary line + per-component
// outcomes (imported / skipped duplicate / rejected) with messages.
class ImportReportDialog : public QDialog {
    Q_OBJECT
public:
    explicit ImportReportDialog(const ImportReport& report,
                                QWidget* parent = nullptr);
};

} // namespace ardulab
