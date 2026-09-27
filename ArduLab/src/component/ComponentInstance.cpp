#include "component/ComponentInstance.h"
#include "component/Geometry.h"

namespace ardulab {

ComponentInstance ComponentInstance::fromDefinition(
        const ComponentDefinition& def, const QString& reference,
        const PointMM& pos) {
    ComponentInstance inst;
    inst.instanceId = InstanceId::generate();
    inst.defId = ComponentDefId(def.id);
    inst.definition = def;
    inst.reference = reference;
    inst.value = def.value;
    inst.position = pos;
    inst.rotation = 0;
    return inst;
}

PointMM ComponentInstance::pinWorldPos(const Pin& pin) const {
    return geometry::transformPin(position, rotation, pin.connectionPoint);
}

QJsonObject ComponentInstance::toJson() const {
    QJsonObject o;
    o["instanceId"] = instanceId.value;
    o["defId"] = defId.value;
    o["reference"] = reference;
    o["value"] = value;
    o["xMm"] = position.x;
    o["yMm"] = position.y;
    o["rotation"] = rotation;
    o["definitionSnapshot"] = definition.toJson();
    return o;
}

Result<ComponentInstance> ComponentInstance::fromJson(const QJsonObject& obj) {
    ComponentInstance inst;
    inst.instanceId = InstanceId(obj.value("instanceId").toString());
    if (!inst.instanceId.isValid())
        inst.instanceId = InstanceId::generate();
    inst.defId = ComponentDefId(obj.value("defId").toString());
    inst.reference = obj.value("reference").toString();
    inst.value = obj.value("value").toString();
    inst.position = PointMM(obj.value("xMm").toDouble(), obj.value("yMm").toDouble());
    inst.rotation = geometry::normalizeRotation(obj.value("rotation").toInt());

    if (!obj.contains("definitionSnapshot") ||
        !obj.value("definitionSnapshot").isObject())
        return Result<ComponentInstance>::fail("BAD_INSTANCE",
            "instance tanpa definitionSnapshot");
    auto def = ComponentDefinition::fromJson(
        obj.value("definitionSnapshot").toObject());
    if (def.isError())
        return Result<ComponentInstance>::fail("BAD_INSTANCE",
            "definitionSnapshot tidak valid: " + def.error().message);
    inst.definition = def.value();
    if (!inst.defId.isValid()) inst.defId = ComponentDefId(inst.definition.id);
    return Result<ComponentInstance>::ok(inst);
}

} // namespace ardulab
