#include <QtTest>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonArray>
#include "Helpers.h"
#include "catalog/Database.h"
#include "catalog/CatalogRepository.h"
#include "import/ComponentImporter.h"

using namespace ardulab;

class TestImport : public QObject {
    Q_OBJECT
    QTemporaryDir dir_;
    int conn_ = 0;
    Database* openDb(const QString& name) {
        auto o = Database::open(dir_.path() + "/" + name + ".db",
                                QString("imp_%1").arg(++conn_));
        return o.isOk() ? o.value() : nullptr;
    }
    QByteArray toBytes(const QJsonValue& v) {
        if (v.isArray()) return QJsonDocument(v.toArray()).toJson();
        return QJsonDocument(v.toObject()).toJson();
    }

private slots:
    void importsValidAsDraft() {
        Database* db = openDb("v"); QVERIFY(db);
        CatalogRepository repo(db->handle());
        ComponentImporter imp(&repo);

        auto report = imp.importJsonBytes(
            toBytes(makeValidComponent("RES-GEN-STD-R-0603")), "mem");
        QVERIFY(!report.fileError);
        QCOMPARE(report.imported, 1);
        QCOMPARE(report.rejected, 0);
        auto got = repo.getById("RES-GEN-STD-R-0603");
        QVERIFY(got.isOk());
        QCOMPARE(got.value().status, ComponentStatus::Draft);
        delete db;
    }

    void rejectsInvalidWithClearReport() {
        Database* db = openDb("inv"); QVERIFY(db);
        CatalogRepository repo(db->handle());
        ComponentImporter imp(&repo);

        // missing physical + bad id
        QJsonObject bad{{"schemaVersion", "1.0"}, {"id", "not-canonical"},
                        {"name", "X"}, {"category", "Y"}};
        auto report = imp.importJsonBytes(toBytes(bad), "mem");
        QVERIFY(!report.fileError);
        QCOMPARE(report.imported, 0);
        QCOMPARE(report.rejected, 1);
        QVERIFY(!report.entries.first().message.isEmpty());
        QCOMPARE(repo.all().size(), 0);
        delete db;
    }

    void skipsDuplicatesConsistently() {
        Database* db = openDb("dup"); QVERIFY(db);
        CatalogRepository repo(db->handle());
        ComponentImporter imp(&repo);

        imp.importJsonBytes(toBytes(makeValidComponent("RES-GEN-STD-R-0603")), "a");
        // second import of same id -> skipped, catalog unchanged
        auto r2 = imp.importJsonBytes(
            toBytes(makeValidComponent("RES-GEN-STD-R-0603")), "b");
        QCOMPARE(r2.imported, 0);
        QCOMPARE(r2.skipped, 1);
        QCOMPARE(r2.entries.first().outcome, ImportOutcome::SkippedDuplicate);
        QCOMPARE(repo.all().size(), 1);
        delete db;
    }

    void arrayImportMixesOutcomes() {
        Database* db = openDb("arr"); QVERIFY(db);
        CatalogRepository repo(db->handle());
        ComponentImporter imp(&repo);
        // one valid, one duplicate-within-file, one invalid
        QJsonArray arr;
        arr.append(makeValidComponent("RES-GEN-STD-R-0603"));
        arr.append(makeValidComponent("RES-GEN-STD-R-0603")); // dup in file
        arr.append(QJsonObject{{"schemaVersion", "1.0"}, {"id", "bad"}}); // invalid
        auto report = imp.importJsonBytes(toBytes(arr), "mem");
        QCOMPARE(report.total, 3);
        QCOMPARE(report.imported, 1);
        QCOMPARE(report.skipped, 1);
        QCOMPARE(report.rejected, 1);
        QCOMPARE(repo.all().size(), 1);
        delete db;
    }

    void malformedJsonDoesNotCrash() {
        Database* db = openDb("mal"); QVERIFY(db);
        CatalogRepository repo(db->handle());
        ComponentImporter imp(&repo);
        auto report = imp.importJsonBytes("{ this is not json ", "mem");
        QVERIFY(report.fileError);
        QVERIFY(!report.fileErrorMessage.isEmpty());
        QCOMPARE(repo.all().size(), 0);
        delete db;
    }

    void validatedInFileIsForcedToDraftWithWarning() {
        Database* db = openDb("valf"); QVERIFY(db);
        CatalogRepository repo(db->handle());
        ComponentImporter imp(&repo);
        QJsonObject c = makeValidComponent("RES-GEN-STD-R-0603");
        c["status"] = "VALIDATED";
        auto report = imp.importJsonBytes(toBytes(c), "mem");
        QCOMPARE(report.imported, 1);
        // stored as DRAFT regardless of the file
        QCOMPARE(repo.getById("RES-GEN-STD-R-0603").value().status,
                 ComponentStatus::Draft);
        // and the report warns about the ignored status
        QVERIFY(!report.entries.first().warnings.isEmpty());
        delete db;
    }

    void missingFileReportsError() {
        Database* db = openDb("miss"); QVERIFY(db);
        CatalogRepository repo(db->handle());
        ComponentImporter imp(&repo);
        auto report = imp.importFile(dir_.path() + "/does-not-exist.json");
        QVERIFY(report.fileError);
        delete db;
    }
};

QTEST_GUILESS_MAIN(TestImport)
#include "TestImport.moc"
