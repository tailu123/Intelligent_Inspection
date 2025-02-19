#include "application/NavStateProcedure.hpp"
#include "application/x30_inspection_system.hpp"
#include "application/event_bus.hpp"
#include "state/x30_state_machine.hpp"
#include "communication/x30_communication.hpp"

namespace x30::application {

// ActionSendNavRequest 实现
void ActionSendNavRequest::execute(const std::vector<protocol::NavigationPoint>& points) {
    // 订阅2003响应
    subscriptions_.push_back(
        subscribe<MessageResponseEvent>(
            [this](const std::shared_ptr<Event>& evt) {
                auto msg_event = std::static_pointer_cast<MessageResponseEvent>(evt);
                if (msg_event->messageId == static_cast<uint32_t>(protocol::MessageType::NAVIGATION_TASK_RESP)) {
                    handleResp1003(msg_event);
                }
            }
        )
    );

    // 发送导航请求
    protocol::NavigationTaskRequest req;
    req.points = points;
    // 使用通信模块发送请求
    context_.communication.sendMessage(req);
}

void ActionSendNavRequest::handleResp1003(const std::shared_ptr<MessageResponseEvent>& event) {
    protocol::NavigationTaskResponse resp;
    resp.errorCode = event->success ? protocol::ErrorCode::SUCCESS : protocol::ErrorCode::FAILURE;
    state_machine_.process_event(resp);
    subscriptions_.clear(); // 清理订阅
}

// ActionSendCancelRequest 实现
void ActionSendCancelRequest::execute() {
    // 订阅2004响应
    subscriptions_.push_back(
        subscribe<MessageResponseEvent>(
            [this](const std::shared_ptr<Event>& evt) {
                auto msg_event = std::static_pointer_cast<MessageResponseEvent>(evt);
                if (msg_event->messageId == static_cast<uint32_t>(protocol::MessageType::CANCEL_TASK_RESP)) {
                    handleResp1004(msg_event);
                }
            }
        )
    );

    // 发送取消请求
    protocol::CancelTaskRequest req;
    context_.communication.sendMessage(req);
}

void ActionSendCancelRequest::handleResp1004(const std::shared_ptr<MessageResponseEvent>& event) {
    protocol::CancelTaskResponse resp;
    resp.errorCode = event->success ? protocol::ErrorCode::SUCCESS : protocol::ErrorCode::FAILURE;
    state_machine_.process_event(resp);
    subscriptions_.clear(); // 清理订阅
}

// NavStateProcedure 实现
NavStateProcedure::NavStateProcedure(
    std::vector<protocol::NavigationPoint>& points,
    communication::AsyncCommunicationManager& comm_manager,
    EventBus& event_bus,
    X30InspectionSystem& inspection_system
) : context_(points, event_bus, inspection_system, *comm_manager.getCommunication()) {

    // 创建状态机并传入context
    state_machine_ = std::make_unique<state::NavStateMachine>(context_);

    // 设置状态机终止回调
    state_machine_->set_terminate_callback([this]() {
        // 清理所有订阅
        subscriptions_.clear();

        // 通知系统导航完成
        if (auto* system = &context_.inspection_system) {
            system->updateNavigationStatus(true, "", "导航任务完成");
        }
    });

    // 创建动作实例
    action_send_nav_ = std::make_unique<ActionSendNavRequest>(context_, *state_machine_);
    action_send_cancel_ = std::make_unique<ActionSendCancelRequest>(context_, *state_machine_);
}

NavStateProcedure::~NavStateProcedure() = default;

void NavStateProcedure::start() {
    // 启动状态机
    std::cout << "启动状态机" << std::endl;
    state_machine_->start();

    // 发送初始导航请求
    action_send_nav_->execute(context_.points);
}

void NavStateProcedure::process_message(const std::shared_ptr<Event>& event) {
    if (auto msg_event = std::dynamic_pointer_cast<MessageResponseEvent>(event)) {
        handleMessageResponse(msg_event);
    }
}

void NavStateProcedure::handleMessageResponse(const std::shared_ptr<MessageResponseEvent>& event) {
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

} // namespace x30::application