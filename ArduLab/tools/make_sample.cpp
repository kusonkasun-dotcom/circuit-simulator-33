// One-off generator for examples/sample_project.fal (not part of the app build).
#include "project/Project.h"
#include "project/ProjectSerializer.h"
#include "import/ComponentImporter.h"
#include "component/ComponentDefinition.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>

using namespace ardulab;

static ComponentDefinition load(const QString& path) {
    QFile f(path); f.open(QIODevice::ReadOnly);
    auto doc = QJsonDocument::fromJson(f.readAll());
    return ComponentDefinition::fromJson(doc.object()).value();
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    const QString base = argv[1]; // examples/components dir
    const QString out = argv[2];

    Project p;
    p.name = "sample_project";

    auto res = load(base + "/resistor.json");
    auto led = load(base + "/led.json");
    auto gnd = load(base + "/ground.json");

    auto r1 = ComponentInstance::fromDefinition(res, "R1", PointMM(60, 40));
    r1.value = "220";
    auto d1 = ComponentInstance::fromDefinition(led, "D1", PointMM(90, 40));
    d1.rotation = 90;
    auto g1 = ComponentInstance::fromDefinition(gnd, "GND1", PointMM(90, 70));

    p.instances << r1 << d1 << g1;
    auto st = ProjectSerializer::save(p, out);
    return st.isOk() ? 0 : 1;
}
