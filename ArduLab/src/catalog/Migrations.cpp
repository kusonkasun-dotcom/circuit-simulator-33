#include "catalog/Migrations.h"
#include "core/Logger.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>

namespace ardulab {

const QVector<Migration>& Migrations::all() {
    static const QVector<Migration> migrations = {
        Migration{
            1, "initial_catalog",
            QStringList{
                "CREATE TABLE IF NOT EXISTS schema_migrations ("
                "  version INTEGER PRIMARY KEY,"
                "  name TEXT NOT NULL,"
                "  applied_at TEXT NOT NULL"
                ");",
                "CREATE TABLE IF NOT EXISTS components ("
                "  id TEXT PRIMARY KEY,"
                "  name TEXT NOT NULL,"
                "  category TEXT NOT NULL,"
                "  value TEXT,"
                "  prefix TEXT,"
                "  status TEXT NOT NULL,"
                "  schema_version TEXT NOT NULL,"
                "  source TEXT,"
                "  definition_json TEXT NOT NULL,"
                "  created_at TEXT NOT NULL,"
                "  updated_at TEXT NOT NULL"
                ");",
                "CREATE INDEX IF NOT EXISTS idx_components_category "
                "  ON components(category);",
                "CREATE INDEX IF NOT EXISTS idx_components_status "
                "  ON components(status);"
            }
        }
    };
    return migrations;
}

int Migrations::targetVersion() {
    int v = 0;
    for (const Migration& m : all()) v = qMax(v, m.version);
    return v;
}

Status Migrations::apply(QSqlDatabase& db) {
    QSqlQuery vq(db);
    if (!vq.exec("PRAGMA user_version;") || !vq.next())
        return Status::fail("DB_MIGRATE",
            "Tidak dapat membaca user_version: " + vq.lastError().text());
    int current = vq.value(0).toInt();

    for (const Migration& m : all()) {
        if (m.version <= current) continue;

        if (!db.transaction())
            return Status::fail("DB_MIGRATE",
                QString("Tidak dapat memulai transaksi migrasi %1: %2")
                    .arg(m.version).arg(db.lastError().text()));

        for (const QString& sql : m.statements) {
            QSqlQuery q(db);
            if (!q.exec(sql)) {
                const QString err = q.lastError().text();
                db.rollback();
                return Status::fail("DB_MIGRATE",
                    QString("Migrasi %1 (%2) gagal: %3")
                        .arg(m.version).arg(m.name, err));
            }
        }

        QSqlQuery ins(db);
        ins.prepare("INSERT OR IGNORE INTO schema_migrations"
                    "(version,name,applied_at) VALUES(?,?,?)");
        ins.addBindValue(m.version);
        ins.addBindValue(m.name);
        ins.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        if (!ins.exec()) {
            const QString err = ins.lastError().text();
            db.rollback();
            return Status::fail("DB_MIGRATE",
                QString("Gagal mencatat migrasi %1: %2").arg(m.version).arg(err));
        }

        QSqlQuery setv(db);
        // PRAGMA does not accept bound parameters.
        if (!setv.exec(QString("PRAGMA user_version = %1;").arg(m.version))) {
            const QString err = setv.lastError().text();
            db.rollback();
            return Status::fail("DB_MIGRATE",
                QString("Gagal menyetel user_version %1: %2").arg(m.version).arg(err));
        }

        if (!db.commit()) {
            db.rollback();
            return Status::fail("DB_MIGRATE",
                QString("Commit migrasi %1 gagal: %2")
                    .arg(m.version).arg(db.lastError().text()));
        }
        log::info(QString("Migrasi diterapkan: v%1 (%2)").arg(m.version).arg(m.name));
        current = m.version;
    }
    return Status::ok();
}

} // namespace ardulab
