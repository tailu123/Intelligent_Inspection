#include "common/event_bus.hpp"
#include <iostream>
#include "common/utils.hpp"
// #include <fmt/core.h>
#include <spdlog/spdlog.h>
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
    auto it = handlers_.find(event->getType());
    if (it != handlers_.end()) {
        for (const auto& [_, handler] : it->second) {
            handler(event);
        }
    }
    else {
        spdlog::warn("[{}]: [EventBus:WRN]: 找不到事件处理函数: {}", common::getCurrentTimestamp(), event->getType());
        // std::cout << fmt::format("[{}]: 找不到事件处理函数: {}", common::getCurrentTimestamp(), event->getType()) << std::endl;
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
            spdlog::warn("[{}]: [EventBus:WRN]: 找不到事件处理函数: {}, handlerId={}", common::getCurrentTimestamp(), event->getType(), handlerId);
            // std::cout << fmt::format("[{}]: 找不到事件处理函数: {}, handlerId={}", common::getCurrentTimestamp(), event->getType(), handlerId) << std::endl;
        }
    }
}

} // namespace common
