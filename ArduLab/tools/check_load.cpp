#include "project/ProjectSerializer.h"
#include <QCoreApplication>
#include <cstdio>
using namespace ardulab;
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    auto r = ProjectSerializer::load(argv[1]);
    if (r.isError()) { printf("LOAD_FAIL: %s\n", qPrintable(r.error().message)); return 1; }
    printf("LOAD_OK instances=%d name=%s\n",
           r.value().instances.size(), qPrintable(r.value().name));
    for (const auto& i : r.value().instances)
        printf("  %s %s rot=%d pins=%d\n", qPrintable(i.reference),
               qPrintable(i.definition.id), i.rotation, i.definition.pins.size());
    return 0;
}
