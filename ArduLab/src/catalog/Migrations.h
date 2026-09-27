#pragma once
#include "core/Result.h"
#include <QSqlDatabase>
#include <QString>
#include <QVector>

namespace ardulab {

struct Migration {
    int version;            // target schema version this migration produces
    QString name;
    QStringList statements; // executed in order, inside one transaction
};

// Minimal forward-only migration framework. Schema version is tracked with
// SQLite's PRAGMA user_version; a schema_migrations table records history.
// Re-running is safe (idempotent): already-applied migrations are skipped.
class Migrations {
public:
    static const QVector<Migration>& all();
    static int targetVersion();

    // Apply every migration whose version is greater than the current
    // user_version. Each migration runs in its own transaction; a failure
    // rolls back that migration and aborts with a clear error.
    static Status apply(QSqlDatabase& db);
};

} // namespace ardulab
