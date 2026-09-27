#include "catalog/Database.h"
#include "catalog/Migrations.h"
#include "core/Logger.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

namespace ardulab {

QString Database::defaultCatalogPath() {
    QString base = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    if (base.isEmpty())
        base = QDir::homePath() + "/.ardulab";
    QDir().mkpath(base);
    return base + "/catalog.db";
}

Result<Database*> Database::open(const QString& path,
                                 const QString& connectionName) {
    // Ensure parent directory exists.
    const QFileInfo fi(path);
    if (!QDir().mkpath(fi.absolutePath()))
        return Result<Database*>::fail("DB_DIR",
            QString("Tidak dapat membuat direktori database: %1")
                .arg(fi.absolutePath()));

    if (QSqlDatabase::contains(connectionName))
        QSqlDatabase::removeDatabase(connectionName);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        const QString err = db.lastError().text();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        return Result<Database*>::fail("DB_OPEN",
            QString("Gagal membuka database katalog '%1': %2")
                .arg(path, err));
    }

    // Pragmas for integrity and referential safety.
    QSqlQuery pragma(db);
    pragma.exec("PRAGMA journal_mode = WAL;");
    pragma.exec("PRAGMA foreign_keys = ON;");

    auto* self = new Database();
    self->db_ = db;
    self->path_ = path;
    self->connectionName_ = connectionName;

    Status mig = Migrations::apply(db);
    if (mig.isError()) {
        db.close();
        QSqlDatabase::removeDatabase(connectionName);
        delete self;
        return Result<Database*>::fail(mig.error().code, mig.error().message);
    }

    log::info(QString("Katalog terbuka: %1 (schema v%2)")
                  .arg(path).arg(self->schemaVersion()));
    return Result<Database*>::ok(self);
}

int Database::schemaVersion() const {
    QSqlQuery q(db_);
    if (q.exec("PRAGMA user_version;") && q.next())
        return q.value(0).toInt();
    return -1;
}

Database::~Database() {
    const QString name = connectionName_;
    if (db_.isOpen()) db_.close();
    db_ = QSqlDatabase();
    if (!name.isEmpty() && QSqlDatabase::contains(name))
        QSqlDatabase::removeDatabase(name);
}

} // namespace ardulab
