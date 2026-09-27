#pragma once
#include "project/Project.h"
#include "component/ComponentInstance.h"
#include <QUndoCommand>

namespace ardulab {

class CanvasScene;

// Commands operate on the Project (source of truth) and mirror the change onto
// the CanvasScene. They are the only place that mutates placed instances, so
// undo/redo stays consistent.

ComponentInstance* findInstance(Project& p, const InstanceId& id);

class AddComponentCommand : public QUndoCommand {
public:
    AddComponentCommand(Project* p, CanvasScene* s, ComponentInstance inst);
    void undo() override;
    void redo() override;
private:
    Project* project_; CanvasScene* scene_; ComponentInstance inst_;
};

class DeleteComponentCommand : public QUndoCommand {
public:
    DeleteComponentCommand(Project* p, CanvasScene* s, const InstanceId& id);
    void undo() override;
    void redo() override;
private:
    Project* project_; CanvasScene* scene_; ComponentInstance inst_; int index_ = -1;
};

class MoveComponentCommand : public QUndoCommand {
public:
    MoveComponentCommand(Project* p, CanvasScene* s, const InstanceId& id,
                         PointMM oldPos, PointMM newPos);
    void undo() override;
    void redo() override;
private:
    void set(const PointMM& pos);
    Project* project_; CanvasScene* scene_; InstanceId id_;
    PointMM old_, new_;
};

class RotateComponentCommand : public QUndoCommand {
public:
    RotateComponentCommand(Project* p, CanvasScene* s, const InstanceId& id,
                           int oldRot, int newRot);
    void undo() override;
    void redo() override;
private:
    void set(int rot);
    Project* project_; CanvasScene* scene_; InstanceId id_;
    int old_, new_;
};

class ChangePropertyCommand : public QUndoCommand {
public:
    ChangePropertyCommand(Project* p, CanvasScene* s, const InstanceId& id,
                          QString oldRef, QString oldVal,
                          QString newRef, QString newVal);
    void undo() override;
    void redo() override;
private:
    void set(const QString& ref, const QString& val);
    Project* project_; CanvasScene* scene_; InstanceId id_;
    QString oldRef_, oldVal_, newRef_, newVal_;
};

} // namespace ardulab
