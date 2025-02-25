#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "protocol/x30_protocol.hpp"

namespace common {

// 事件基类
class Event {
public:
    virtual ~Event() = default;
    virtual std::string getType() const = 0;
};

// 事件处理器类型
using EventHandler = std::function<void(const std::shared_ptr<Event>&)>;

// 事件总线
// 优点：
// 1. 解耦：事件发送者和事件处理器解耦
// 2. 灵活：可以随时添加和删除事件处理器
// 3. 可扩展：可以随时添加和删除事件类型;   增加事件类型时， 不需要修改现有代码，比如日志， 监控等
// 缺点：
// 1. 性能：事件处理器需要遍历所有事件类型和处理器
// 2. 内存：事件处理器需要存储所有事件处理器
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
        std::string eventType = T().getType();
        std::string handlerId = generateHandlerId();
        handlers_[eventType][handlerId] = handler;
        return handlerId;
    }

    // 注册事件处理器
    template<typename T>
    std::string subscribe(EventHandler handler, const std::string& handlerId) {
        static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

        std::lock_guard<std::mutex> lock(mutex_);
        std::string eventType = typeid(T).name();
        handlers_[eventType][handlerId] = handler;
        return handlerId;
    }


    // 取消注册事件处理器
    void unsubscribe(const std::string& eventType, const std::string& handlerId);


    // 发布事件
    void publish(const std::shared_ptr<Event>& event);
    void publish(const std::shared_ptr<Event>& event, const std::string& handlerId);

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

struct NetworkErrorEvent : public Event {
    std::string getType() const override { return "NetworkError"; }
    std::string message;
};

struct QueryStatusEvent : public Event {
    std::string getType() const override { return "QueryStatus"; }

    protocol::NavigationStatus status;
    int value;
    std::string timestamp;
    protocol::ErrorCode errorCode;

    // 便于构造的静态工厂方法
    static std::shared_ptr<QueryStatusEvent> fromResponse(
        const protocol::QueryStatusResponse& resp) {
        auto event = std::make_shared<QueryStatusEvent>();
        event->status = resp.status;
        event->value = resp.value;
        event->timestamp = resp.timestamp;
        event->errorCode = resp.errorCode;
        return event;
    }
};

struct GetRealTimeStatusEvent : public Event {
    std::string getType() const override { return "GetRealTimeStatus"; }

    std::string timestamp;
    double posX;
    double posY;
    double posZ;
    double sumOdom;            // 累计里程
    int location;           // 位置  定位正常=0, 定位丢失=1

    // 便于构造的静态工厂方法
    static std::shared_ptr<GetRealTimeStatusEvent> fromResponse(
        const protocol::GetRealTimeStatusResponse& resp) {
        auto event = std::make_shared<GetRealTimeStatusEvent>();
        event->timestamp = resp.timestamp;
        event->posX = resp.posX;
        event->posY = resp.posY;
        event->posZ = resp.posZ;
        event->sumOdom = resp.sumOdom;
        event->location = resp.location;
        return event;
    }
};

struct ErrorEvent : public Event {
    std::string getType() const override { return "Error"; }
    int code;
    std::string message;
};

struct NavigationTaskEvent : public Event {
    std::string getType() const override { return "NavigationTask"; }

    std::string status;
};

} // namespace common
