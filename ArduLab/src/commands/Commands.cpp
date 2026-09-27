#include "commands/Commands.h"
#include "canvas/CanvasScene.h"

namespace ardulab {

ComponentInstance* findInstance(Project& p, const InstanceId& id) {
    for (ComponentInstance& inst : p.instances)
        if (inst.instanceId == id) return &inst;
    return nullptr;
}

// ---- Add -------------------------------------------------------------------
AddComponentCommand::AddComponentCommand(Project* p, CanvasScene* s,
                                         ComponentInstance inst)
    : project_(p), scene_(s), inst_(std::move(inst)) {
    setText(QObject::tr("Tambah %1").arg(inst_.reference));
}
void AddComponentCommand::redo() {
    project_->instances.append(inst_);
    scene_->addInstanceItem(inst_);
}
void AddComponentCommand::undo() {
    scene_->removeInstanceItem(inst_.instanceId);
    for (int i = 0; i < project_->instances.size(); ++i)
        if (project_->instances[i].instanceId == inst_.instanceId) {
            project_->instances.remove(i); break;
        }
}

// ---- Delete ----------------------------------------------------------------
DeleteComponentCommand::DeleteComponentCommand(Project* p, CanvasScene* s,
                                               const InstanceId& id)
    : project_(p), scene_(s) {
    for (int i = 0; i < p->instances.size(); ++i)
        if (p->instances[i].instanceId == id) {
            inst_ = p->instances[i]; index_ = i; break;
        }
    setText(QObject::tr("Hapus %1").arg(inst_.reference));
}
void DeleteComponentCommand::redo() {
    scene_->removeInstanceItem(inst_.instanceId);
    if (index_ >= 0 && index_ < project_->instances.size())
        project_->instances.remove(index_);
}
void DeleteComponentCommand::undo() {
    if (index_ >= 0 && index_ <= project_->instances.size())
        project_->instances.insert(index_, inst_);
    else
        project_->instances.append(inst_);
    scene_->addInstanceItem(inst_);
}

// ---- Move ------------------------------------------------------------------
MoveComponentCommand::MoveComponentCommand(Project* p, CanvasScene* s,
        const InstanceId& id, PointMM oldPos, PointMM newPos)
    : project_(p), scene_(s), id_(id), old_(oldPos), new_(newPos) {
    setText(QObject::tr("Pindah komponen"));
}
void MoveComponentCommand::set(const PointMM& pos) {
    if (auto* inst = findInstance(*project_, id_)) {
        inst->position = pos;
        scene_->updateInstanceItem(*inst);
    }
}
void MoveComponentCommand::redo() { set(new_); }
void MoveComponentCommand::undo() { set(old_); }

// ---- Rotate ----------------------------------------------------------------
RotateComponentCommand::RotateComponentCommand(Project* p, CanvasScene* s,
        const InstanceId& id, int oldRot, int newRot)
    : project_(p), scene_(s), id_(id), old_(oldRot), new_(newRot) {
    setText(QObject::tr("Putar komponen"));
}
void RotateComponentCommand::set(int rot) {
    if (auto* inst = findInstance(*project_, id_)) {
        inst->rotation = rot;
        scene_->updateInstanceItem(*inst);
    }
}
void RotateComponentCommand::redo() { set(new_); }
void RotateComponentCommand::undo() { set(old_); }

// ---- Change property -------------------------------------------------------
ChangePropertyCommand::ChangePropertyCommand(Project* p, CanvasScene* s,
        const InstanceId& id, QString oldRef, QString oldVal,
        QString newRef, QString newVal)
    : project_(p), scene_(s), id_(id),
      oldRef_(std::move(oldRef)), oldVal_(std::move(oldVal)),
      newRef_(std::move(newRef)), newVal_(std::move(newVal)) {
    setText(QObject::tr("Ubah properti"));
}
void ChangePropertyCommand::set(const QString& ref, const QString& val) {
    if (auto* inst = findInstance(*project_, id_)) {
        inst->reference = ref;
        inst->value = val;
        scene_->updateInstanceItem(*inst);
    }
}
void ChangePropertyCommand::redo() { set(newRef_, newVal_); }
void ChangePropertyCommand::undo() { set(oldRef_, oldVal_); }

} // namespace ardulab
