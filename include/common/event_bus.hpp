#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <any>

namespace common {

// 事件基类
class Event {
public:
    virtual ~Event() = default;
    virtual std::string getType() const = 0;
};

// 事件处理器类型
using EventHandler = std::function<void(const std::shared_ptr<Event>&)>;

// 事件总线类
class EventBus {
public:
    static EventBus& getInstance() {
        static EventBus instance;
        return instance;
    }

    // 注册事件处理器
    template<typename T>
    std::string subscribe(EventHandler handler) {
        static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

        std::lock_guard<std::mutex> lock(mutex_);
        std::string eventType = typeid(T).name();
        std::string handlerId = generateHandlerId();
        handlers_[eventType][handlerId] = handler;
        return handlerId;
    }

    // 取消注册事件处理器
    void unsubscribe(const std::string& eventType, const std::string& handlerId);
    // void unsubscribe(const std::string& eventType, const std::string& handlerId) {
    //     std::lock_guard<std::mutex> lock(mutex_);
    //     auto it = handlers_.find(eventType);
    //     if (it != handlers_.end()) {
    //         it->second.erase(handlerId);
    //     }
    // }

    // 发布事件
    void publish(const std::shared_ptr<Event>& event);
    // void publish(const std::shared_ptr<Event>& event) {
    //     std::lock_guard<std::mutex> lock(mutex_);
    //     auto it = handlers_.find(typeid(*event).name());
    //     if (it != handlers_.end()) {
    //     for (const auto& [_, handler] : it->second) {
    //         handler(event);
    //     }
    // }

private:
    EventBus() = default;
    ~EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // 生成唯一的处理器ID
    std::string generateHandlerId() {
        return std::to_string(++handler_counter_);
    }

    std::mutex mutex_;
    std::unordered_map<std::string, std::unordered_map<std::string, EventHandler>> handlers_;
    uint64_t handler_counter_ = 0;
};

// 常用事件定义
struct MessageResponseEvent : public Event {
    std::string getType() const override { return "MessageResponse"; }
    uint32_t messageId;
    bool success;
    std::string data;
};

struct ConnectionStatusEvent : public Event {
    std::string getType() const override { return "ConnectionStatus"; }
    bool connected;
    std::string message;
};

struct NavigationStatusEvent : public Event {
    std::string getType() const override { return "NavigationStatus"; }
    bool completed;
    std::string currentPoint;
    std::string status;
};

struct ErrorEvent : public Event {
    std::string getType() const override { return "Error"; }
    int code;
    std::string message;
};

} // namespace common
