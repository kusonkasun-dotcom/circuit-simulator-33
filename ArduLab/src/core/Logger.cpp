#include "core/Logger.h"
#include <QDebug>

namespace ardulab {
namespace log {

void info(const QString& msg)  { qInfo().noquote()    << "[INFO ]" << msg; }
void warn(const QString& msg)  { qWarning().noquote() << "[WARN ]" << msg; }
void error(const QString& msg) { qWarning().noquote() << "[ERROR]" << msg; }

} // namespace log
} // namespace ardulab
