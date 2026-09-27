#include "catalog/CatalogRepository.h"
#include "core/EventBus.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QVariant>

namespace ardulab {

namespace {
QString nowIso() {
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}
QString toJsonString(const ComponentDefinition& def) {
    return QString::fromUtf8(
        QJsonDocument(def.toJson()).toJson(QJsonDocument::Compact));
}
} // namespace

ComponentDefinition CatalogRepository::rowToDefinition(const QString& json,
                                                       const QString& statusStr) {
    QJsonParseError perr;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &perr);
    ComponentDefinition def;
    auto parsed = ComponentDefinition::fromJson(doc.object());
    if (parsed.isOk()) def = parsed.value();
    // stored status is authoritative
    def.status = statusFromString(statusStr);
    return def;
}

Status CatalogRepository::add(const ComponentDefinition& def) {
    if (exists(def.id))
        return Status::fail("DUPLICATE",
            QString("Komponen dengan id '%1' sudah ada di katalog").arg(def.id));

    QSqlQuery q(db_);
    q.prepare("INSERT INTO components"
              "(id,name,category,value,prefix,status,schema_version,source,"
              " definition_json,created_at,updated_at)"
              " VALUES(?,?,?,?,?,?,?,?,?,?,?)");
    const QString ts = nowIso();
    q.addBindValue(def.id);
    q.addBindValue(def.name);
    q.addBindValue(def.category);
    q.addBindValue(def.value);
    q.addBindValue(def.prefix);
    q.addBindValue(statusToString(def.status));
    q.addBindValue(def.schemaVersion);
    q.addBindValue(def.source);
    q.addBindValue(toJsonString(def));
    q.addBindValue(ts);
    q.addBindValue(ts);
    if (!q.exec())
        return Status::fail("DB_WRITE",
            "Gagal menyimpan komponen: " + q.lastError().text());
    emit EventBus::instance().catalogChanged();
    return Status::ok();
}

Status CatalogRepository::update(const ComponentDefinition& def) {
    QSqlQuery q(db_);
    q.prepare("UPDATE components SET name=?,category=?,value=?,prefix=?,"
              "status=?,schema_version=?,source=?,definition_json=?,updated_at=? "
              "WHERE id=?");
    q.addBindValue(def.name);
    q.addBindValue(def.category);
    q.addBindValue(def.value);
    q.addBindValue(def.prefix);
    q.addBindValue(statusToString(def.status));
    q.addBindValue(def.schemaVersion);
    q.addBindValue(def.source);
    q.addBindValue(toJsonString(def));
    q.addBindValue(nowIso());
    q.addBindValue(def.id);
    if (!q.exec())
        return Status::fail("DB_WRITE",
            "Gagal memperbarui komponen: " + q.lastError().text());
    emit EventBus::instance().catalogChanged();
    return Status::ok();
}

bool CatalogRepository::exists(const QString& id) const {
    QSqlQuery q(db_);
    q.prepare("SELECT 1 FROM components WHERE id=? LIMIT 1");
    q.addBindValue(id);
    return q.exec() && q.next();
}

Result<ComponentDefinition> CatalogRepository::getById(const QString& id) const {
    QSqlQuery q(db_);
    q.prepare("SELECT definition_json,status FROM components WHERE id=?");
    q.addBindValue(id);
    if (!q.exec())
        return Result<ComponentDefinition>::fail("DB_READ", q.lastError().text());
    if (!q.next())
        return Result<ComponentDefinition>::fail("NOT_FOUND",
            QString("Komponen '%1' tidak ditemukan").arg(id));
    return Result<ComponentDefinition>::ok(
        rowToDefinition(q.value(0).toString(), q.value(1).toString()));
}

QVector<ComponentDefinition> CatalogRepository::search(
        const CatalogFilter& f) const {
    QString sql = "SELECT definition_json,status FROM components WHERE 1=1";
    if (!f.text.isEmpty())
        sql += " AND (id LIKE :t OR name LIKE :t OR category LIKE :t)";
    if (!f.category.isEmpty())
        sql += " AND category = :cat";
    if (f.onlyStatus)
        sql += " AND status = :st";
    sql += " ORDER BY category, name";

    QSqlQuery q(db_);
    q.prepare(sql);
    if (!f.text.isEmpty()) q.bindValue(":t", "%" + f.text + "%");
    if (!f.category.isEmpty()) q.bindValue(":cat", f.category);
    if (f.onlyStatus) q.bindValue(":st", statusToString(f.status));

    QVector<ComponentDefinition> out;
    if (q.exec())
        while (q.next())
            out.append(rowToDefinition(q.value(0).toString(), q.value(1).toString()));
    return out;
}

QVector<ComponentDefinition> CatalogRepository::all() const {
    return search(CatalogFilter{});
}

QStringList CatalogRepository::categories() const {
    QSqlQuery q(db_);
    QStringList out;
    if (q.exec("SELECT DISTINCT category FROM components ORDER BY category"))
        while (q.next()) out << q.value(0).toString();
    return out;
}

Status CatalogRepository::remove(const QString& id) {
    QSqlQuery q(db_);
    q.prepare("DELETE FROM components WHERE id=?");
    q.addBindValue(id);
    if (!q.exec())
        return Status::fail("DB_WRITE", q.lastError().text());
    emit EventBus::instance().catalogChanged();
    return Status::ok();
}

Status CatalogRepository::addManyAtomic(
        const QVector<ComponentDefinition>& defs) {
    if (defs.isEmpty()) return Status::ok();
    if (!db_.transaction())
        return Status::fail("DB_TX", "Tidak dapat memulai transaksi impor");

    for (const ComponentDefinition& def : defs) {
        QSqlQuery q(db_);
        q.prepare("INSERT INTO components"
                  "(id,name,category,value,prefix,status,schema_version,source,"
                  " definition_json,created_at,updated_at)"
                  " VALUES(?,?,?,?,?,?,?,?,?,?,?)");
        const QString ts = nowIso();
        q.addBindValue(def.id);
        q.addBindValue(def.name);
        q.addBindValue(def.category);
        q.addBindValue(def.value);
        q.addBindValue(def.prefix);
        q.addBindValue(statusToString(def.status));
        q.addBindValue(def.schemaVersion);
        q.addBindValue(def.source);
        q.addBindValue(toJsonString(def));
        q.addBindValue(ts);
        q.addBindValue(ts);
        if (!q.exec()) {
            const QString err = q.lastError().text();
            db_.rollback();
            return Status::fail("DB_WRITE",
                QString("Gagal menyimpan '%1': %2").arg(def.id, err));
        }
    }

    if (!db_.commit()) {
        db_.rollback();
        return Status::fail("DB_TX", "Commit impor gagal: " + db_.lastError().text());
    }
    emit EventBus::instance().catalogChanged();
    return Status::ok();
}

} // namespace ardulab
