#include "import/ComponentImporter.h"
#include "catalog/CatalogRepository.h"
#include "core/Logger.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>

namespace ardulab {

QString ImportReport::summaryLine() const {
    if (fileError)
        return QString("Impor gagal: %1").arg(fileErrorMessage);
    return QString("%1 diproses: %2 diimpor, %3 dilewati (duplikat), %4 ditolak")
        .arg(total).arg(imported).arg(skipped).arg(rejected);
}

ImportReport ComponentImporter::importFile(const QString& path) {
    QFile f(path);
    if (!f.exists()) {
        ImportReport r; r.sourcePath = path; r.fileError = true;
        r.fileErrorMessage = QString("File tidak ditemukan: %1").arg(path);
        return r;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        ImportReport r; r.sourcePath = path; r.fileError = true;
        r.fileErrorMessage = QString("Tidak dapat membaca file: %1").arg(path);
        return r;
    }
    const QByteArray bytes = f.readAll();
    ImportReport r = importJsonBytes(bytes, path);
    r.sourcePath = path;
    return r;
}

ImportReport ComponentImporter::importJsonBytes(const QByteArray& bytes,
                                                const QString& sourceLabel) {
    ImportReport report;
    report.sourcePath = sourceLabel;

    QJsonParseError perr;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &perr);
    if (perr.error != QJsonParseError::NoError) {
        report.fileError = true;
        report.fileErrorMessage =
            QString("JSON tidak valid pada offset %1: %2")
                .arg(perr.offset).arg(perr.errorString());
        return report;
    }

    // Accept: a single component object, an array of objects, or a wrapper
    // { "schemaVersion": "1.0", "components": [...] }.
    QJsonArray items;
    if (doc.isArray()) {
        items = doc.array();
    } else if (doc.isObject()) {
        const QJsonObject root = doc.object();
        if (root.contains("components") && root.value("components").isArray()) {
            report.schemaVersion = root.value("schemaVersion").toString();
            items = root.value("components").toArray();
        } else {
            items.append(root); // single component
        }
    } else {
        report.fileError = true;
        report.fileErrorMessage = "Root JSON harus objek atau array komponen";
        return report;
    }

    report.total = items.size();

    // Validate all, collecting accepted (new & valid) for one atomic insert.
    QVector<ComponentDefinition> accepted;
    QSet<QString> acceptedIds; // duplicates *within the same file*

    for (const QJsonValue& v : items) {
        ImportEntry entry;
        if (!v.isObject()) {
            entry.outcome = ImportOutcome::Rejected;
            entry.message = "Elemen bukan objek komponen";
            report.rejected++;
            report.entries.append(entry);
            continue;
        }
        QStringList warns;
        auto parsed = ComponentDefinition::fromJson(v.toObject(), &warns);
        if (parsed.isError()) {
            entry.outcome = ImportOutcome::Rejected;
            entry.id = v.toObject().value("id").toString();
            entry.message = parsed.error().message;
            report.rejected++;
            report.entries.append(entry);
            continue;
        }
        ComponentDefinition def = parsed.value();
        def.status = ComponentStatus::Draft;   // policy
        if (def.source.isEmpty()) def.source = sourceLabel;
        entry.id = def.id;
        entry.warnings = warns;

        if (acceptedIds.contains(def.id) || repo_->exists(def.id)) {
            entry.outcome = ImportOutcome::SkippedDuplicate;
            entry.message =
                QString("id '%1' sudah ada; dilewati (kebijakan: skip)")
                    .arg(def.id);
            report.skipped++;
            report.entries.append(entry);
            continue;
        }

        entry.outcome = ImportOutcome::Imported;
        accepted.append(def);
        acceptedIds.insert(def.id);
        report.entries.append(entry);
    }

    // Atomic insert of the accepted set. If the DB write fails, mark those
    // entries as rejected so the report stays truthful and the catalog is
    // left untouched.
    if (!accepted.isEmpty()) {
        Status st = repo_->addManyAtomic(accepted);
        if (st.isError()) {
            for (ImportEntry& e : report.entries) {
                if (e.outcome == ImportOutcome::Imported) {
                    e.outcome = ImportOutcome::Rejected;
                    e.message = "Gagal menulis ke katalog: " + st.error().message;
                    report.rejected++;
                }
            }
            report.imported = 0;
        } else {
            report.imported = accepted.size();
        }
    }

    log::info(report.summaryLine());
    return report;
}

} // namespace ardulab
