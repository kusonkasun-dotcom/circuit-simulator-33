#include <QtTest>
#include "component/Geometry.h"
#include "component/ComponentInstance.h"

using namespace ardulab;

class TestGeometry : public QObject {
    Q_OBJECT
private slots:
    void rotateMultiplesExact() {
        PointMM p(3.0, 0.0);
        QVERIFY(geometry::rotateLocal(p, 0).nearlyEquals(PointMM(3, 0)));
        QVERIFY(geometry::rotateLocal(p, 90).nearlyEquals(PointMM(0, 3)));
        QVERIFY(geometry::rotateLocal(p, 180).nearlyEquals(PointMM(-3, 0)));
        QVERIFY(geometry::rotateLocal(p, 270).nearlyEquals(PointMM(0, -3)));
        // full turn back to start
        QVERIFY(geometry::rotateLocal(p, 360).nearlyEquals(PointMM(3, 0)));
    }

    void normalizeRotation() {
        QCOMPARE(geometry::normalizeRotation(0), 0);
        QCOMPARE(geometry::normalizeRotation(90), 90);
        QCOMPARE(geometry::normalizeRotation(450), 90);
        QCOMPARE(geometry::normalizeRotation(-90), 270);
    }

    void snapPositiveAndNegative() {
        const Mm g = 2.54;
        // 1.2 is closer to 0 than to 2.54 -> snaps to 0
        QVERIFY(geometry::snapToGrid(PointMM(1.2, 1.2), g)
                    .nearlyEquals(PointMM(0.0, 0.0)));
        // 1.6 is closer to 2.54 -> snaps up
        QVERIFY(geometry::snapToGrid(PointMM(1.6, 1.6), g)
                    .nearlyEquals(PointMM(2.54, 2.54)));
        // negative coordinates must snap symmetrically
        QVERIFY(geometry::snapToGrid(PointMM(-1.2, -1.2), g)
                    .nearlyEquals(PointMM(0.0, 0.0)));
        QVERIFY(geometry::snapToGrid(PointMM(-1.6, -1.6), g)
                    .nearlyEquals(PointMM(-2.54, -2.54)));
        QVERIFY(geometry::snapToGrid(PointMM(-3.9, 3.9), g)
                    .nearlyEquals(PointMM(-5.08, 5.08)));
    }

    void pinTransformWithMoveAndRotate() {
        // A pin located locally at (5,0). Move component to (10,20), rotate 90.
        ComponentDefinition def;
        def.widthMm = 10; def.heightMm = 2;
        Pin pin; pin.id = "2"; pin.connectionPoint = PointMM(5, 0);
        def.pins.append(pin);

        ComponentInstance inst =
            ComponentInstance::fromDefinition(def, "R1", PointMM(10, 20));
        // no rotation
        QVERIFY(inst.pinWorldPos(pin).nearlyEquals(PointMM(15, 20)));
        // rotate 90 -> local (5,0) becomes (0,5) then + origin
        inst.rotation = 90;
        QVERIFY(inst.pinWorldPos(pin).nearlyEquals(PointMM(10, 25)));
        // rotate 180 -> (-5,0)
        inst.rotation = 180;
        QVERIFY(inst.pinWorldPos(pin).nearlyEquals(PointMM(5, 20)));
    }
};

QTEST_GUILESS_MAIN(TestGeometry)
#include "TestGeometry.moc"
