#include "common/event_bus.hpp"
#include <iostream>
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
    // std::cout << "publish 1" << std::endl;
    auto it = handlers_.find(event->getType());
    if (it != handlers_.end()) {
        // std::cout << "publish 2" << std::endl;
        for (const auto& [_, handler] : it->second) {
            // std::cout << "publish 3" << std::endl;
            handler(event);
        }
    }
    else {
        // std::cout << "publish 4 not find event:" << event->getType() << std::endl;
    }
}

void EventBus::publish(const std::shared_ptr<Event>& event, const std::string& handlerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = handlers_.find(event->getType());
    if (it != handlers_.end()) {
        auto handler = it->second.find(handlerId);
        if (handler != it->second.end()) {
            handler->second(event);
        }
        else {
            std::cout << "找不到事件处理函数: " << event->getType() << ", handlerId=" << handlerId << std::endl;
        }
    }
}

} // namespace common
