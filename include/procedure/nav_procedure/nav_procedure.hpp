#pragma once

#include <vector>
#include <memory>
#include <string>
#include "common/event_bus.hpp"
#include "network/x30_communication.hpp"
// #include "protocol/x30_protocol.hpp"
// #include "state/nav/nav_context.hpp"
#include "state/nav/nav_machine.hpp"

namespace common {
class EventBus;
} // namespace common

namespace protocol {
struct NavigationPoint;
} // namespace protocol

namespace network {
class AsyncCommunicationManager;
class X30Communication;
} // namespace network

namespace application {
class X30InspectionSystem;
} // namespace application

// namespace state {
// class NavigationMachine;
// } // namespace state

namespace procedure {

// 导航相关事件定义
// struct SendNavRequestEvent : public common::Event {
//     std::string getType() const override { return "SendNavRequest"; }
//     std::vector<protocol::NavigationPoint> points;
// };

// struct CancelRequestEvent : public common::Event {
//     std::string getType() const override { return "CancelRequest"; }
// };

// 前向声明
// class EventSubscriptionGuard;
// class ActionBase;
// class ActionSendNavRequest;
// class ActionSendCancelRequest;

// 导航上下文
// struct NavigationContext {
//     std::vector<protocol::NavigationPoint>& points;
//     EventBus& event_bus;
//     X30InspectionSystem& inspection_system;
//     communication::X30Communication& communication;

//     NavigationContext(
//         std::vector<protocol::NavigationPoint>& p,
//         EventBus& eb,
//         X30InspectionSystem& is,
//         communication::X30Communication& comm
//     ) : points(p)
//       , event_bus(eb)
//       , inspection_system(is)
//       , communication(comm) {}
// };

// RAII 事件订阅管理器
class EventSubscriptionGuard {
public:
    EventSubscriptionGuard(common::EventBus& event_bus, const std::string& event_type, const std::string& handler_id)
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
    EventSubscriptionGuard& operator=(EventSubscriptionGuard&&) = delete;

private:
    common::EventBus& event_bus_;
    std::string event_type_;
    std::string handler_id_;
};

// // 动作基类
// class ActionBase {
// public:
//     ActionBase(state::NavigationContext& context, state::NavigationMachine& state_machine)
//         : context_(context)
//         , state_machine_(state_machine) {}
//     virtual ~ActionBase() = default;

// protected:
//     state::NavigationContext& context_;
//     state::NavigationMachine& state_machine_;
//     std::vector<std::unique_ptr<EventSubscriptionGuard>> subscriptions_;

//     template<typename T>
//     std::unique_ptr<EventSubscriptionGuard> subscribe(
//         std::function<void(const std::shared_ptr<common::Event>&)> handler,
//         std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {

//         auto handler_id = context_.event_bus.subscribe<T>(handler);
//         if (timeout.count() > 0) {
//             // TODO: 实现超时处理
//         }
//         return std::make_unique<EventSubscriptionGuard>(
//             context_.event_bus, typeid(T).name(), handler_id);
//     }
// };

// // 发送导航请求动作
// class ActionSendNavRequest : public ActionBase {
// public:
//     using ActionBase::ActionBase;
//     void execute(const std::vector<protocol::NavigationPoint>& points);

// private:
//     void handleResp1003(const std::shared_ptr<common::MessageResponseEvent>& event);
// };

// // 发送取消请求动作
// class ActionSendCancelRequest : public ActionBase {
// public:
//     using ActionBase::ActionBase;
//     void execute();

// private:
//     void handleResp1004(const std::shared_ptr<common::MessageResponseEvent>& event);
// };

class NavigationProcedure {
public:
    // NavigationProcedure(state::NavigationContext context);
    NavigationProcedure(
        std::vector<protocol::NavigationPoint>& points,
        network::AsyncCommunicationManager& comm_manager,
        common::EventBus& event_bus,
        common::MessageQueue& message_queue
    );
    ~NavigationProcedure();

    void start();
    void cancelInspection();
    void queryStatus();
    void process_message(const protocol::IMessage& message);
    // void process_event(const std::shared_ptr<Event>& event);

    // 获取状态机实例
    // const state::NavigationMachine* getStateMachine() const { return state_machine_.get(); }

private:
    void handleMessageResponse(const std::shared_ptr<common::MessageResponseEvent>& event);

    std::shared_ptr<network::X30Communication> communication_;
    // std::unique_ptr<state::NavigationContext> context_;
    state::NavigationContext context_;
    std::unique_ptr<state::NavigationMachine> state_machine_;
    std::vector<std::unique_ptr<EventSubscriptionGuard>> subscriptions_;

    // 动作实例
    // std::unique_ptr<ActionSendNavRequest> action_send_nav_;
    // std::unique_ptr<ActionSendCancelRequest> action_send_cancel_;
};

} // namespace procedure
