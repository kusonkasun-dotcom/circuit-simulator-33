#include <QtTest>
#include <QTemporaryDir>
#include "Helpers.h"
#include "project/Project.h"
#include "project/ProjectSerializer.h"

using namespace ardulab;

class TestProject : public QObject {
    Q_OBJECT
    QTemporaryDir dir_;

    ComponentInstance makeInstance(const QString& ref, PointMM pos, int rot) {
        auto def = ComponentDefinition::fromJson(
            makeValidComponent("RES-GEN-STD-R-0603", "Resistor", "Resistor"));
        auto inst = ComponentInstance::fromDefinition(def.value(), ref, pos);
        inst.rotation = rot;
        inst.value = "4k7";
        return inst;
    }

private slots:
    void saveLoadRoundtripPreservesSnapshot() {
        Project p;
        p.name = "Demo";
        p.instances.append(makeInstance("R1", PointMM(10, 20), 90));
        p.instances.append(makeInstance("R2", PointMM(-5.08, 2.54), 0));

        const QString path = dir_.path() + "/demo.fal";
        QVERIFY(ProjectSerializer::save(p, path).isOk());
        QVERIFY(QFile::exists(path));

        auto loaded = ProjectSerializer::load(path);
        QVERIFY(loaded.isOk());
        Project& q = loaded.value();
        QCOMPARE(q.name, QString("Demo"));
        QCOMPARE(q.instances.size(), 2);
        QCOMPARE(q.instances[0].reference, QString("R1"));
        QCOMPARE(q.instances[0].rotation, 90);
        QVERIFY(q.instances[0].position.nearlyEquals(PointMM(10, 20)));
        QCOMPARE(q.instances[0].value, QString("4k7"));
        // definition snapshot must survive independently of any catalog
        QCOMPARE(q.instances[0].definition.id, QString("RES-GEN-STD-R-0603"));
        QCOMPARE(q.instances[0].definition.pins.size(), 2);
    }

    void corruptFileReturnsErrorNotCrash() {
        const QString path = dir_.path() + "/broken.fal";
        QFile f(path); QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("{ not a valid project"); f.close();

        auto loaded = ProjectSerializer::load(path);
        QVERIFY(loaded.isError());
        QVERIFY(!loaded.error().message.isEmpty());
    }

    void wrongFormatRejected() {
        const QString path = dir_.path() + "/other.fal";
        QFile f(path); QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("{\"format\":\"SomethingElse\"}"); f.close();
        auto loaded = ProjectSerializer::load(path);
        QVERIFY(loaded.isError());
        QCOMPARE(loaded.error().code, QString("PROJECT_FORMAT"));
    }

    void nextReferenceIncrements() {
        Project p;
        p.instances.append(makeInstance("R1", PointMM(0, 0), 0));
        p.instances.append(makeInstance("R3", PointMM(0, 0), 0));
        QCOMPARE(p.nextReference("R"), QString("R2"));
        p.instances.append(makeInstance("R2", PointMM(0, 0), 0));
        QCOMPARE(p.nextReference("R"), QString("R4"));
        QCOMPARE(p.nextReference("C"), QString("C1"));
    }
};

QTEST_GUILESS_MAIN(TestProject)
#include "TestProject.moc"
