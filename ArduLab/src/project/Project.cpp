#include "project/Project.h"
#include <QSet>

namespace ardulab {

QString Project::nextReference(const QString& prefix) const {
    const QString p = prefix.isEmpty() ? "U" : prefix;
    QSet<int> used;
    for (const ComponentInstance& inst : instances) {
        const QString ref = inst.reference;
        if (ref.startsWith(p)) {
            bool ok = false;
            const int n = ref.mid(p.size()).toInt(&ok);
            if (ok) used.insert(n);
        }
    }
    int n = 1;
    while (used.contains(n)) ++n;
    return p + QString::number(n);
}

} // namespace ardulab
