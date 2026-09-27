#include <QtTest>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QVariant>
#include "Helpers.h"
#include "catalog/Database.h"
#include "catalog/Migrations.h"
#include "catalog/CatalogRepository.h"

using namespace ardulab;

class TestCatalog : public QObject {
    Q_OBJECT
    QTemporaryDir dir_;
    int conn_ = 0;
    QString newConn() { return QString("test_cat_%1").arg(++conn_); }
    QString dbPath(const QString& f) { return dir_.path() + "/" + f; }

private slots:
    void databaseCreatedOnFirstUse() {
        const QString path = dbPath("first.db");
        QVERIFY(!QFile::exists(path));
        auto opened = Database::open(path, newConn());
        QVERIFY2(opened.isOk(), qPrintable(
            opened.isError() ? opened.error().message : QString()));
        QVERIFY(QFile::exists(path));
        QCOMPARE(opened.value()->schemaVersion(), Migrations::targetVersion());
        delete opened.value();
    }

    void migrationIsIdempotent() {
        const QString path = dbPath("mig.db");
        // First open applies migrations.
        {
            auto o1 = Database::open(path, newConn());
            QVERIFY(o1.isOk());
            QCOMPARE(o1.value()->schemaVersion(), Migrations::targetVersion());
            delete o1.value();
        }
        // Second open must not fail and must not change the version.
        {
            auto o2 = Database::open(path, newConn());
            QVERIFY(o2.isOk());
            QCOMPARE(o2.value()->schemaVersion(), Migrations::targetVersion());
            // schema_migrations should record exactly the applied versions.
            QSqlQuery q(o2.value()->handle());
            QVERIFY(q.exec("SELECT COUNT(*) FROM schema_migrations"));
            QVERIFY(q.next());
            QCOMPARE(q.value(0).toInt(), Migrations::all().size());
            delete o2.value();
        }
    }

    void addGetDuplicateSearch() {
        auto opened = Database::open(dbPath("cat.db"), newConn());
        QVERIFY(opened.isOk());
        Database* db = opened.value();
        CatalogRepository repo(db->handle());

        auto d1 = ComponentDefinition::fromJson(
            makeValidComponent("RES-GEN-STD-R-0603", "Resistor", "Resistor"));
        QVERIFY(d1.isOk());
        QVERIFY(repo.add(d1.value()).isOk());

        // duplicate id must be rejected
        Status dup = repo.add(d1.value());
        QVERIFY(dup.isError());
        QCOMPARE(dup.error().code, QString("DUPLICATE"));

        auto d2 = ComponentDefinition::fromJson(
            makeValidComponent("CAP-GEN-STD-C-0603", "Capacitor", "Capacitor"));
        QVERIFY(repo.add(d2.value()).isOk());

        QCOMPARE(repo.all().size(), 2);

        // text search
        CatalogFilter f; f.text = "Resis";
        QCOMPARE(repo.search(f).size(), 1);

        // category filter
        CatalogFilter fc; fc.category = "Capacitor";
        QCOMPARE(repo.search(fc).size(), 1);

        // getById returns correct definition and DRAFT status default
        auto got = repo.getById("RES-GEN-STD-R-0603");
        QVERIFY(got.isOk());
        QCOMPARE(got.value().name, QString("Resistor"));
        delete db;
    }

    void componentsPersistAcrossRestart() {
        const QString path = dbPath("persist.db");
        {
            auto o = Database::open(path, newConn());
            QVERIFY(o.isOk());
            CatalogRepository repo(o.value()->handle());
            auto d = ComponentDefinition::fromJson(
                makeValidComponent("LED-GEN-STD-D-0805", "LED", "Diode"));
            QVERIFY(repo.add(d.value()).isOk());
            delete o.value(); // simulate app close
        }
        {
            auto o = Database::open(path, newConn());
            QVERIFY(o.isOk());
            CatalogRepository repo(o.value()->handle());
            QCOMPARE(repo.all().size(), 1);
            QVERIFY(repo.exists("LED-GEN-STD-D-0805"));
            delete o.value();
        }
    }

    void openFailsClearlyOnBadPath() {
        // A path whose parent cannot be created (a file used as a directory).
        const QString filePath = dbPath("afile");
        QFile f(filePath); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("x"); f.close();
        auto opened = Database::open(filePath + "/nested/catalog.db", newConn());
        QVERIFY(opened.isError());
        QVERIFY(!opened.error().message.isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestCatalog)
#include "TestCatalog.moc"
