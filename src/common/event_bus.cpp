#include "common/event_bus.hpp"

namespace common {

void EventBus::unsubscribe(const std::string& eventType, const std::string& handlerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = handlers_.find(eventType);
    if (it != handlers_.end()) {
        it->second.erase(handlerId);
    }
}

void EventBus::publish(const std::shared_ptr<Event>& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = handlers_.find(typeid(*event).name());
    if (it != handlers_.end()) {
    for (const auto& [_, handler] : it->second) {
            handler(event);
        }
    }
}

} // namespace common
