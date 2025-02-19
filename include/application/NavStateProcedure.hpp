#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <chrono>
#include <string>
#include "event_bus.hpp"
#include "communication/x30_communication.hpp"
#include "protocol/x30_protocol.hpp"
#include "state/NavStateMachine.hpp"
namespace x30 {

namespace protocol {
class NavigationPoint;
} // namespace protocol

namespace communication {
class AsyncCommunicationManager;
class X30Communication;
} // namespace communication

// namespace state {
// class NavStateMachine;
// } // namespace state

namespace application {

class X30InspectionSystem;
class EventBus;

// 导航相关事件定义
struct SendNavRequestEvent : public Event {
    std::string getType() const override { return "SendNavRequest"; }
    std::vector<protocol::NavigationPoint> points;
};

struct CancelRequestEvent : public Event {
    std::string getType() const override { return "CancelRequest"; }
};

// 前向声明
class EventSubscriptionGuard;
class ActionBase;
class ActionSendNavRequest;
class ActionSendCancelRequest;

// 导航上下文
struct NavigationContext {
    std::vector<protocol::NavigationPoint>& points;
    EventBus& event_bus;
    X30InspectionSystem& inspection_system;
    communication::X30Communication& communication;

    NavigationContext(
        std::vector<protocol::NavigationPoint>& p,
        EventBus& eb,
        X30InspectionSystem& is,
        communication::X30Communication& comm
    ) : points(p)
      , event_bus(eb)
      , inspection_system(is)
      , communication(comm) {}
};

// RAII 事件订阅管理器
class EventSubscriptionGuard {
public:
    EventSubscriptionGuard(EventBus& event_bus, const std::string& event_type, const std::string& handler_id)
        : event_bus_(event_bus)
        , event_type_(event_type)
        , handler_id_(handler_id) {}

    ~EventSubscriptionGuard() {
        event_bus_.unsubscribe(event_type_, handler_id_);
    }

    // 禁止拷贝
    EventSubscriptionGuard(const EventSubscriptionGuard&) = delete;
    EventSubscriptionGuard& operator=(const EventSubscriptionGuard&) = delete;

    // 允许移动
    EventSubscriptionGuard(EventSubscriptionGuard&&) = default;
    EventSubscriptionGuard& operator=(EventSubscriptionGuard&&) = default;

private:
    EventBus& event_bus_;
    std::string event_type_;
    std::string handler_id_;
};

// 动作基类
class ActionBase {
public:
    ActionBase(NavigationContext& context, state::NavStateMachine& state_machine)
        : context_(context)
        , state_machine_(state_machine) {}
    virtual ~ActionBase() = default;

protected:
    NavigationContext& context_;
    state::NavStateMachine& state_machine_;
    std::vector<std::unique_ptr<EventSubscriptionGuard>> subscriptions_;

    template<typename T>
    std::unique_ptr<EventSubscriptionGuard> subscribe(
        std::function<void(const std::shared_ptr<Event>&)> handler,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {

        auto handler_id = context_.event_bus.subscribe<T>(handler);
        if (timeout.count() > 0) {
            // TODO: 实现超时处理
        }
        return std::make_unique<EventSubscriptionGuard>(
            context_.event_bus, typeid(T).name(), handler_id);
    }
};

// 发送导航请求动作
class ActionSendNavRequest : public ActionBase {
public:
    using ActionBase::ActionBase;
    void execute(const std::vector<protocol::NavigationPoint>& points);

private:
    void handleResp1003(const std::shared_ptr<MessageResponseEvent>& event);
};

// 发送取消请求动作
class ActionSendCancelRequest : public ActionBase {
public:
    using ActionBase::ActionBase;
    void execute();

private:
    void handleResp1004(const std::shared_ptr<MessageResponseEvent>& event);
};

class NavStateProcedure {
public:
    NavStateProcedure(
        std::vector<protocol::NavigationPoint>& points,
        communication::AsyncCommunicationManager& comm_manager,
        EventBus& event_bus,
        X30InspectionSystem& inspection_system
    );
    ~NavStateProcedure();

    void start();
    void process_message(const std::shared_ptr<Event>& event);

    // 获取状态机实例
    const state::NavStateMachine* getStateMachine() const { return state_machine_.get(); }

private:
    void handleMessageResponse(const std::shared_ptr<MessageResponseEvent>& event);

    std::unique_ptr<state::NavStateMachine> state_machine_;
    NavigationContext context_;
    std::vector<std::unique_ptr<EventSubscriptionGuard>> subscriptions_;

    // 动作实例
    std::unique_ptr<ActionSendNavRequest> action_send_nav_;
    std::unique_ptr<ActionSendCancelRequest> action_send_cancel_;
};

} // namespace application
} // namespace x30