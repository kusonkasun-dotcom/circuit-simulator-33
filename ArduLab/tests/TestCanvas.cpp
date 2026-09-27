#include <QtTest>
#include "Helpers.h"
#include "canvas/CanvasScene.h"
#include "canvas/ComponentItem.h"
#include "commands/Commands.h"
#include "project/Project.h"
#include <QUndoStack>

using namespace ardulab;

class TestCanvas : public QObject {
    Q_OBJECT
    ComponentInstance make(const QString& ref, PointMM pos) {
        auto def = ComponentDefinition::fromJson(
            makeValidComponent("RES-GEN-STD-R-0603")).value();
        return ComponentInstance::fromDefinition(def, ref, pos);
    }

private slots:
    void selectedInstanceIdsReportsAll() {
        CanvasScene scene;
        auto* a = scene.addInstanceItem(make("R1", PointMM(0, 0)));
        auto* b = scene.addInstanceItem(make("R2", PointMM(20, 0)));
        a->setSelected(true);
        b->setSelected(true);
        QCOMPARE(scene.selectedInstanceIds().size(), 2);
    }

    void deleteAllSelectedRemovesEveryComponent() {
        // Mirrors MainWindow::deleteSelected : macro of DeleteComponentCommand
        // over every selected id must clear the whole selection, not just one.
        CanvasScene scene;
        Project project;
        QUndoStack stack;

        for (const QString& ref : {"R1", "R2", "R3"}) {
            auto inst = make(ref, PointMM(0, 0));
            project.instances.append(inst);
            scene.addInstanceItem(inst)->setSelected(true);
        }
        QCOMPARE(project.instances.size(), 3);

        const auto ids = scene.selectedInstanceIds();
        QCOMPARE(ids.size(), 3);
        stack.beginMacro("del");
        for (const InstanceId& id : ids)
            stack.push(new DeleteComponentCommand(&project, &scene, id));
        stack.endMacro();

        QCOMPARE(project.instances.size(), 0);

        // Undo restores all three atomically.
        stack.undo();
        QCOMPARE(project.instances.size(), 3);
    }

    void rotateAllSelectedRotatesEveryComponent() {
        CanvasScene scene;
        Project project;
        QUndoStack stack;
        for (const QString& ref : {"R1", "R2"}) {
            auto inst = make(ref, PointMM(0, 0));
            project.instances.append(inst);
            scene.addInstanceItem(inst)->setSelected(true);
        }
        stack.beginMacro("rot");
        for (const InstanceId& id : scene.selectedInstanceIds()) {
            auto* inst = findInstance(project, id);
            stack.push(new RotateComponentCommand(&project, &scene, id,
                inst->rotation, (inst->rotation + 90) % 360));
        }
        stack.endMacro();
        for (const auto& inst : project.instances)
            QCOMPARE(inst.rotation, 90);
    }
};

QTEST_MAIN(TestCanvas)
#include "TestCanvas.moc"
