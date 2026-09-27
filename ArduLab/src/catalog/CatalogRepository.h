#pragma once
#include "core/Result.h"
#include "component/ComponentDefinition.h"
#include <QSqlDatabase>
#include <QVector>
#include <QString>

namespace ardulab {

struct CatalogFilter {
    QString text;                 // matches id/name/category (LIKE)
    QString category;             // exact match when non-empty
    bool onlyStatus = false;      // when true, filter by `status`
    ComponentStatus status = ComponentStatus::Draft;
};

// All SQL for the catalog lives here; no other module touches QtSql directly.
class CatalogRepository {
public:
    explicit CatalogRepository(QSqlDatabase db) : db_(std::move(db)) {}

    // Insert a new definition. Duplicate id -> Error code "DUPLICATE"
    // (skip policy is enforced by the importer / caller).
    Status add(const ComponentDefinition& def);

    // Explicit update of an existing definition (used by future edit actions).
    Status update(const ComponentDefinition& def);

    Result<ComponentDefinition> getById(const QString& id) const;
    bool exists(const QString& id) const;

    QVector<ComponentDefinition> search(const CatalogFilter& filter) const;
    QVector<ComponentDefinition> all() const;

    // Distinct categories, for the browser filter.
    QStringList categories() const;

    Status remove(const QString& id);

    // Batch insert used by import: runs inside one transaction so a file
    // either imports its accepted set fully or not at all.
    Status addManyAtomic(const QVector<ComponentDefinition>& defs);

private:
    QSqlDatabase db_;
    static ComponentDefinition rowToDefinition(const QString& json,
                                               const QString& statusStr);
};

} // namespace ardulab
