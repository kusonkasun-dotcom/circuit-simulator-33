#include "project/ProjectSerializer.h"
#include "core/Logger.h"
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

namespace ardulab {

Status ProjectSerializer::save(const Project& project, const QString& path) {
    QJsonObject root;
    root["format"] = "ArduLab.Project";
    root["schemaVersion"] = project.schemaVersion;
    root["name"] = project.name;
    QJsonObject canvas;
    canvas["widthMm"] = project.canvasWidthMm;
    canvas["heightMm"] = project.canvasHeightMm;
    root["canvas"] = canvas;

    QJsonArray insts;
    for (const ComponentInstance& inst : project.instances)
        insts.append(inst.toJson());
    root["instances"] = insts;

    const QByteArray bytes =
        QJsonDocument(root).toJson(QJsonDocument::Indented);

    // Atomic write: QSaveFile writes to a temp file then commits with rename.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return Status::fail("PROJECT_WRITE",
            QString("Tidak dapat membuka '%1' untuk ditulis: %2")
                .arg(path, file.errorString()));
    }
    if (file.write(bytes) != bytes.size()) {
        file.cancelWriting();
        return Status::fail("PROJECT_WRITE",
            QString("Gagal menulis seluruh isi proyek ke '%1'").arg(path));
    }
    if (!file.commit()) {
        return Status::fail("PROJECT_WRITE",
            QString("Gagal menyelesaikan penyimpanan '%1': %2")
                .arg(path, file.errorString()));
    }
    log::info(QString("Proyek disimpan: %1 (%2 instance)")
                  .arg(path).arg(project.instances.size()));
    return Status::ok();
}

Result<Project> ProjectSerializer::load(const QString& path) {
    QFile file(path);
    if (!file.exists())
        return Result<Project>::fail("PROJECT_READ",
            QString("File proyek tidak ditemukan: %1").arg(path));
    if (!file.open(QIODevice::ReadOnly))
        return Result<Project>::fail("PROJECT_READ",
            QString("Tidak dapat membaca proyek: %1").arg(path));

    const QByteArray bytes = file.readAll();
    QJsonParseError perr;
    QJsonDocument doc = QJsonDocument::fromJson(bytes, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject())
        return Result<Project>::fail("PROJECT_CORRUPT",
            QString("Proyek rusak / bukan JSON valid: %1")
                .arg(perr.errorString()));

    const QJsonObject root = doc.object();
    if (root.value("format").toString() != "ArduLab.Project")
        return Result<Project>::fail("PROJECT_FORMAT",
            "File bukan proyek ArduLab (.fal) yang dikenal");

    Project project;
    project.schemaVersion = root.value("schemaVersion").toString("1.0");
    project.name = root.value("name").toString("Untitled");
    const QJsonObject canvas = root.value("canvas").toObject();
    project.canvasWidthMm = canvas.value("widthMm").toDouble(420.0);
    project.canvasHeightMm = canvas.value("heightMm").toDouble(297.0);

    const QJsonArray insts = root.value("instances").toArray();
    for (const QJsonValue& v : insts) {
        auto inst = ComponentInstance::fromJson(v.toObject());
        if (inst.isError())
            return Result<Project>::fail("PROJECT_CORRUPT",
                "Instance rusak dalam proyek: " + inst.error().message);
        project.instances.append(inst.value());
    }
    log::info(QString("Proyek dimuat: %1 (%2 instance)")
                  .arg(path).arg(project.instances.size()));
    return Result<Project>::ok(project);
}

} // namespace ardulab
