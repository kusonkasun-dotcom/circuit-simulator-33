#pragma once
#include "core/Result.h"
#include <QString>
#include <QSqlDatabase>

namespace ardulab {

// Owns the per-user SQLite connection for the catalog. The database lives in
// the OS user-data location (never the install directory) and is created on
// first use.
class Database {
public:
    // Returns the default per-user catalog path (creating the directory if
    // needed). Uses Qt's AppDataLocation.
    static QString defaultCatalogPath();

    // Open (creating if absent) a SQLite database at `path`. A clear error is
    // returned when the file/directory cannot be opened.
    static Result<Database*> open(const QString& path,
                                  const QString& connectionName = "ardulab_catalog");

    ~Database();

    QSqlDatabase& handle() { return db_; }
    QString path() const { return path_; }
    int schemaVersion() const;

private:
    Database() = default;
    QSqlDatabase db_;
    QString path_;
    QString connectionName_;
};

} // namespace ardulab
