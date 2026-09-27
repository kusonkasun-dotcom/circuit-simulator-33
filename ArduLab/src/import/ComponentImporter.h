#pragma once
#include "component/ComponentDefinition.h"
#include <QString>
#include <QVector>

namespace ardulab {

enum class ImportOutcome { Imported, SkippedDuplicate, Rejected };

struct ImportEntry {
    ImportOutcome outcome;
    QString id;                 // may be empty if unparseable
    QString message;            // reason for skip/reject
    QStringList warnings;
};

// Human-readable summary of one import operation.
struct ImportReport {
    QString sourcePath;
    QString schemaVersion;
    int total = 0;
    int imported = 0;
    int skipped = 0;
    int rejected = 0;
    bool fileError = false;      // file missing / not JSON / wrong root
    QString fileErrorMessage;
    QVector<ImportEntry> entries;

    QString summaryLine() const;
};

class CatalogRepository;

// Reads a canonical v1.0 component JSON file (single object or array),
// validates each component, skips duplicates already in the catalog, and
// atomically inserts the accepted set. Never throws; never leaves the catalog
// partially updated for a given file.
class ComponentImporter {
public:
    explicit ComponentImporter(CatalogRepository* repo) : repo_(repo) {}

    ImportReport importFile(const QString& path);
    ImportReport importJsonBytes(const QByteArray& bytes,
                                 const QString& sourceLabel);

private:
    CatalogRepository* repo_;
};

} // namespace ardulab
