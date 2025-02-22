#include "procedure/nav_procedure/nav_procedure.hpp"
#include "common/message_queue.hpp"
// #include "application/x30_inspection_system.hpp"
#include "common/event_bus.hpp"
#include "state/nav/nav_context.hpp"
#include "state/nav/nav_machine.hpp"
#include "network/x30_communication.hpp"
#include <iostream>
#include <memory>

namespace procedure {

// ActionSendNavRequest 实现
// void ActionSendNavRequest::execute(const std::vector<protocol::NavigationPoint>& points) {
//     // 订阅2003响应
//     subscriptions_.push_back(
//         subscribe<common::MessageResponseEvent>(
//             [this](const std::shared_ptr<common::Event>& evt) {
//                 auto msg_event = std::static_pointer_cast<common::MessageResponseEvent>(evt);
//                 if (msg_event->messageId == static_cast<uint32_t>(protocol::MessageType::NAVIGATION_TASK_RESP)) {
//                     handleResp1003(msg_event);
//                 }
//             }
//         )
//     );

//     // 发送导航请求
//     protocol::NavigationTaskRequest req;
//     req.points = points;
//     // 使用通信模块发送请求
//     context_.communication.sendMessage(req);
// }

// void ActionSendNavRequest::handleResp1003(const std::shared_ptr<common::MessageResponseEvent>& event) {
//     protocol::NavigationTaskResponse resp;
//     resp.errorCode = event->success ? protocol::ErrorCode::SUCCESS : protocol::ErrorCode::FAILURE;
//     state_machine_.process_event(resp);
//     subscriptions_.clear(); // 清理订阅
// }

// // ActionSendCancelRequest 实现
// void ActionSendCancelRequest::execute() {
//     // 订阅2004响应
//     subscriptions_.push_back(
//         subscribe<common::MessageResponseEvent>(
//             [this](const std::shared_ptr<common::Event>& evt) {
//                 auto msg_event = std::static_pointer_cast<common::MessageResponseEvent>(evt);
//                 if (msg_event->messageId == static_cast<uint32_t>(protocol::MessageType::CANCEL_TASK_RESP)) {
//                     handleResp1004(msg_event);
//                 }
//             }
//         )
//     );

//     // 发送取消请求
//     protocol::CancelTaskRequest req;
//     context_.communication.sendMessage(req);
// }

// void ActionSendCancelRequest::handleResp1004(const std::shared_ptr<common::MessageResponseEvent>& event) {
//     protocol::CancelTaskResponse resp;
//     resp.errorCode = event->success ? protocol::ErrorCode::SUCCESS : protocol::ErrorCode::FAILURE;
//     state_machine_.process_event(resp);
//     subscriptions_.clear(); // 清理订阅
// }

// NavigationProcedure 实现
NavigationProcedure::NavigationProcedure(
    // state::NavigationContext context)
    // : context_{std::move(context)} {
    std::vector<protocol::NavigationPoint>& points,
    network::AsyncCommunicationManager& comm_manager,
    common::EventBus& event_bus,
    common::MessageQueue& message_queue)
    : communication_{comm_manager.getCommunication()}
    , context_{points, event_bus, message_queue, *communication_} {

    // 创建状态机并传入context
    state_machine_ = std::make_unique<state::NavigationMachine>(context_);

    // 设置状态机终止回调
    state_machine_->set_terminate_callback([this]() {
        // 清理所有订阅
        subscriptions_.clear();

        // 通知系统导航完成
        // if (auto* system = &context_.inspection_system) {
        //     system->updateNavigationStatus(true, "", "导航任务完成");
        // }
    });

    // 创建动作实例
    // action_send_nav_ = std::make_unique<ActionSendNavRequest>(context_, *state_machine_);
    // action_send_cancel_ = std::make_unique<ActionSendCancelRequest>(context_, *state_machine_);
}

NavigationProcedure::~NavigationProcedure() = default;

void NavigationProcedure::start() {
    // 启动状态机
    std::cout << "启动状态机" << std::endl;
    state_machine_->start();
    state_machine_->process_event(protocol::NavigationTaskRequest{});

    // 发送初始导航请求
    // action_send_nav_->execute(context_.points);
}

void NavigationProcedure::cancelInspection() {
    // 发送取消请求
    // action_send_cancel_->execute();
    state_machine_->process_event(protocol::CancelTaskRequest{});
}

void NavigationProcedure::queryStatus() {
    // 实现状态查询逻辑
    state_machine_->process_event(protocol::QueryStatusRequest{});
}

void NavigationProcedure::process_message(const protocol::IMessage& message) {
    // 根据消息类型处理
    // 如果交互协议多，可以使用EventBus进行消息分发
    switch (message.getType()) {
        case protocol::MessageType::NAVIGATION_TASK_RESP: {
            auto& resp = dynamic_cast<const protocol::NavigationTaskResponse&>(message);
            state_machine_->process_event(resp);
            break;
        }
        case protocol::MessageType::CANCEL_TASK_RESP: {
            auto& resp = dynamic_cast<const protocol::CancelTaskResponse&>(message);
            state_machine_->process_event(resp);
            break;
        }
        case protocol::MessageType::QUERY_STATUS_RESP: {
            auto& resp = dynamic_cast<const protocol::QueryStatusResponse&>(message);
            state_machine_->process_event(resp);
            break;
        }
        default: {
            std::cout << "[NavProc:WRN]: 无法处理当前消息类型" << std::endl;
            break;
        }
    }
}

// void NavStateProcedure::process_event(const std::shared_ptr<Event>& event) {
//     state_machine_->process_event(*event);
// }

void NavigationProcedure::handleMessageResponse(const std::shared_ptr<common::MessageResponseEvent>& event) {
    // 根据消息类型处理响应
    switch (event->messageId) {
        case static_cast<uint32_t>(protocol::MessageType::NAVIGATION_TASK_RESP): {
            protocol::NavigationTaskResponse resp;
            resp.errorCode = event->success ? protocol::ErrorCode::SUCCESS : protocol::ErrorCode::FAILURE;
            state_machine_->process_event(resp);
            break;
        }
        case static_cast<uint32_t>(protocol::MessageType::CANCEL_TASK_RESP): {
            protocol::CancelTaskResponse resp;
            resp.errorCode = event->success ? protocol::ErrorCode::SUCCESS : protocol::ErrorCode::FAILURE;
            state_machine_->process_event(resp);
            break;
        }
        case static_cast<uint32_t>(protocol::MessageType::QUERY_STATUS_RESP): {
            protocol::QueryStatusResponse resp;
            resp.status = event->success ? protocol::NavigationStatus::EXECUTING : protocol::NavigationStatus::FAILED;
            state_machine_->process_event(resp);
            break;
        }
    }
}

} // namespace procedure
