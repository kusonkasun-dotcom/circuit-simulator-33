#include "core/EventBus.h"

namespace ardulab {

EventBus& EventBus::instance() {
    static EventBus bus;
    return bus;
}

} // namespace ardulab
