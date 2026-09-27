#pragma once
#include <QString>

namespace ardulab {

// Thin logging facade. Keeps a single place to route diagnostics; the domain
// layer uses this instead of touching qDebug directly so behaviour can evolve
// without spreading dependencies.
namespace log {
void info(const QString& msg);
void warn(const QString& msg);
void error(const QString& msg);
}

} // namespace ardulab
