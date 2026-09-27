#pragma once
#include <QObject>

namespace ardulab {

// Lightweight application-wide signal hub used to decouple modules: e.g. the
// catalog repository announces changes and the component browser reacts,
// without either holding a pointer to the other.
class EventBus : public QObject {
    Q_OBJECT
public:
    static EventBus& instance();

signals:
    void catalogChanged();

private:
    explicit EventBus(QObject* parent = nullptr) : QObject(parent) {}
};

} // namespace ardulab
