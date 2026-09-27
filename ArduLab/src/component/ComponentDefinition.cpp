#include "component/ComponentDefinition.h"
#include <QJsonArray>
#include <QJsonValue>
#include <QRegularExpression>
#include <QSet>

namespace ardulab {

QString statusToString(ComponentStatus s) {
    return s == ComponentStatus::Validated ? "VALIDATED" : "DRAFT";
}

ComponentStatus statusFromString(const QString& s) {
    return s.trimmed().toUpper() == "VALIDATED" ? ComponentStatus::Validated
                                                : ComponentStatus::Draft;
}

bool ComponentDefinition::isValidId(const QString& id) {
    static const QRegularExpression re(
        "^[A-Za-z0-9]+(-[A-Za-z0-9]+){4}$");
    return re.match(id).hasMatch();
}

namespace {

Result<double> readNumber(const QJsonObject& o, const QString& key,
                          const QString& ctx) {
    if (!o.contains(key))
        return Result<double>::fail("MISSING_FIELD",
            QString("%1: field '%2' wajib ada").arg(ctx, key));
    const QJsonValue v = o.value(key);
    if (!v.isDouble())
        return Result<double>::fail("BAD_TYPE",
            QString("%1: field '%2' harus berupa angka").arg(ctx, key));
    return Result<double>::ok(v.toDouble());
}

Result<PointMM> readPoint(const QJsonObject& parent, const QString& key,
                          const QString& ctx) {
    if (!parent.contains(key) || !parent.value(key).isObject())
        return Result<PointMM>::fail("MISSING_FIELD",
            QString("%1: objek '%2' {xMm,yMm} wajib ada").arg(ctx, key));
    const QJsonObject p = parent.value(key).toObject();
    auto x = readNumber(p, "xMm", ctx + "." + key);
    if (x.isError()) return Result<PointMM>::fail(x.error().code, x.error().message);
    auto y = readNumber(p, "yMm", ctx + "." + key);
    if (y.isError()) return Result<PointMM>::fail(y.error().code, y.error().message);
    return Result<PointMM>::ok(PointMM(x.value(), y.value()));
}

} // namespace

Result<ComponentDefinition> ComponentDefinition::fromJson(
        const QJsonObject& obj, QStringList* warnings) {
    ComponentDefinition def;

    // schemaVersion
    def.schemaVersion = obj.value("schemaVersion").toString();
    if (def.schemaVersion != "1.0")
        return Result<ComponentDefinition>::fail("UNSUPPORTED_SCHEMA",
            QString("schemaVersion '%1' tidak didukung (harus '1.0')")
                .arg(def.schemaVersion));

    // id
    def.id = obj.value("id").toString().trimmed();
    if (def.id.isEmpty())
        return Result<ComponentDefinition>::fail("MISSING_FIELD",
            "field 'id' wajib ada");
    if (!isValidId(def.id))
        return Result<ComponentDefinition>::fail("BAD_ID",
            QString("id '%1' tidak mengikuti pola CATEGORY-ID-VARIANT-PART-PACKAGE")
                .arg(def.id));

    // name, category
    def.name = obj.value("name").toString().trimmed();
    if (def.name.isEmpty())
        return Result<ComponentDefinition>::fail("MISSING_FIELD",
            QString("%1: field 'name' wajib ada").arg(def.id));
    def.category = obj.value("category").toString().trimmed();
    if (def.category.isEmpty())
        return Result<ComponentDefinition>::fail("MISSING_FIELD",
            QString("%1: field 'category' wajib ada").arg(def.id));

    def.value = obj.value("value").toString();
    if (obj.contains("prefix")) def.prefix = obj.value("prefix").toString();

    // status : honor the value when present (default DRAFT). The catalog
    // import path enforces the DRAFT policy separately, so instance snapshots
    // round-trip their real status without being downgraded.
    def.status = obj.contains("status")
        ? statusFromString(obj.value("status").toString())
        : ComponentStatus::Draft;

    // physical
    if (!obj.contains("physical") || !obj.value("physical").isObject())
        return Result<ComponentDefinition>::fail("MISSING_FIELD",
            QString("%1: objek 'physical' wajib ada").arg(def.id));
    {
        const QJsonObject phys = obj.value("physical").toObject();
        auto w = readNumber(phys, "widthMm", def.id + ".physical");
        if (w.isError()) return Result<ComponentDefinition>::fail(w.error().code, w.error().message);
        auto h = readNumber(phys, "heightMm", def.id + ".physical");
        if (h.isError()) return Result<ComponentDefinition>::fail(h.error().code, h.error().message);
        def.widthMm = w.value();
        def.heightMm = h.value();
        if (def.widthMm <= 0 || def.heightMm <= 0)
            return Result<ComponentDefinition>::fail("BAD_VALUE",
                QString("%1: physical widthMm/heightMm harus > 0").arg(def.id));
    }

    // visual (optional but validated when present)
    if (obj.contains("visual")) {
        const QJsonObject vis = obj.value("visual").toObject();
        const QJsonArray prims = vis.value("primitives").toArray();
        for (const QJsonValue& pv : prims) {
            const QJsonObject po = pv.toObject();
            VisualPrimitive prim;
            prim.type = po.value("type").toString();
            if (prim.type.isEmpty()) {
                if (warnings) warnings->append(
                    QString("%1: primitive visual tanpa 'type' dilewati").arg(def.id));
                continue;
            }
            for (auto it = po.begin(); it != po.end(); ++it) {
                if (it.key() == "type") continue;
                prim.params.insert(it.key(), it.value().toVariant());
            }
            def.visual.append(prim);
        }
    }

    // pins (required, at least one, unique ids)
    const QJsonArray pinsArr = obj.value("pins").toArray();
    if (pinsArr.isEmpty())
        return Result<ComponentDefinition>::fail("MISSING_FIELD",
            QString("%1: minimal satu 'pins' wajib ada").arg(def.id));
    QSet<QString> seenPinIds;
    for (const QJsonValue& pv : pinsArr) {
        if (!pv.isObject())
            return Result<ComponentDefinition>::fail("BAD_TYPE",
                QString("%1: setiap pin harus berupa objek").arg(def.id));
        const QJsonObject po = pv.toObject();
        Pin pin;
        pin.id = po.value("id").toString().trimmed();
        if (pin.id.isEmpty())
            return Result<ComponentDefinition>::fail("MISSING_FIELD",
                QString("%1: setiap pin wajib memiliki 'id'").arg(def.id));
        if (seenPinIds.contains(pin.id))
            return Result<ComponentDefinition>::fail("DUP_PIN",
                QString("%1: id pin '%2' duplikat").arg(def.id, pin.id));
        seenPinIds.insert(pin.id);

        pin.name = po.value("name").toString(pin.id);
        pin.type = pinTypeFromString(po.value("type").toString("passive"));
        if (pin.type == PinType::Unknown && warnings)
            warnings->append(QString("%1: pin '%2' tipe tidak dikenal, "
                "dianggap 'passive'").arg(def.id, pin.id));
        if (pin.type == PinType::Unknown) pin.type = PinType::Passive;

        auto anchor = readPoint(po, "anchorPosVisual", def.id + ".pin[" + pin.id + "]");
        if (anchor.isError())
            return Result<ComponentDefinition>::fail(anchor.error().code, anchor.error().message);
        pin.anchor = anchor.value();

        if (po.contains("connectionPointVisual")) {
            auto cp = readPoint(po, "connectionPointVisual",
                                def.id + ".pin[" + pin.id + "]");
            if (cp.isError())
                return Result<ComponentDefinition>::fail(cp.error().code, cp.error().message);
            pin.connectionPoint = cp.value();
        } else {
            pin.connectionPoint = pin.anchor; // sensible default
        }
        def.pins.append(pin);
    }

    if (obj.contains("source")) def.source = obj.value("source").toString();
    return Result<ComponentDefinition>::ok(def);
}

QJsonObject ComponentDefinition::toJson() const {
    QJsonObject o;
    o["schemaVersion"] = schemaVersion;
    o["id"] = id;
    o["name"] = name;
    o["category"] = category;
    o["value"] = value;
    o["prefix"] = prefix;
    o["status"] = statusToString(status);
    if (!source.isEmpty()) o["source"] = source;

    QJsonObject phys;
    phys["widthMm"] = widthMm;
    phys["heightMm"] = heightMm;
    o["physical"] = phys;

    QJsonArray prims;
    for (const VisualPrimitive& p : visual) {
        QJsonObject po = QJsonObject::fromVariantMap(p.params);
        po["type"] = p.type;
        prims.append(po);
    }
    QJsonObject vis; vis["primitives"] = prims;
    o["visual"] = vis;

    QJsonArray pinsArr;
    for (const Pin& pin : pins) {
        QJsonObject po;
        po["id"] = pin.id;
        po["name"] = pin.name;
        po["type"] = pinTypeToString(pin.type);
        po["anchorPosVisual"] = QJsonObject{{"xMm", pin.anchor.x}, {"yMm", pin.anchor.y}};
        po["connectionPointVisual"] =
            QJsonObject{{"xMm", pin.connectionPoint.x}, {"yMm", pin.connectionPoint.y}};
        pinsArr.append(po);
    }
    o["pins"] = pinsArr;
    return o;
}

} // namespace ardulab
