#include "application/x30_inspection_system.hpp"
#include "procedure/nav_procedure/nav_procedure.hpp"
#include "common/event_bus.hpp"
#include "network/x30_communication.hpp"
#include "protocol/x30_protocol.hpp"
#include <iostream>
#include "common/utils.hpp"
#include "state/nav/nav_context.hpp"
#include <fmt/core.h>
#include <fmt/chrono.h>
#include "common/event_bus.hpp"

namespace application {
X30InspectionSystem::X30InspectionSystem()
    : message_queue_running_(false) {
}

X30InspectionSystem::~X30InspectionSystem() {
    shutdown();
}

bool X30InspectionSystem::initialize(const std::string& host, uint16_t port) {
    // 订阅网络错误事件
    common::EventBus::getInstance().subscribe<common::NetworkErrorEvent>([this](const std::shared_ptr<common::Event>&) {
        // 网络错误关闭连接
        if (network_model_manager_) {
            if (auto network_model = network_model_manager_->getNetworkModel(); network_model) {
                network_model->disconnect();
            }
        }

        if (nav_state_procedure_) {
            nav_state_procedure_.reset();
        }
    });

    // 初始化通信管理器
    network_model_manager_ = std::make_unique<network::NetworkModelManager>(
        network::NetworkModelType::ASIO, message_queue_);
    if (!network_model_manager_->start(host, port)) {
        return false;
    }

    // 启动消息处理线程
    message_queue_running_ = true;
    message_thread_ = std::thread(&X30InspectionSystem::messageProcessingLoop, this);

    return true;
}

void X30InspectionSystem::shutdown() {
    try {
        message_queue_running_ = false;
        if (message_thread_.joinable()) {
            message_thread_.join();
        }

        if (network_model_manager_) {
            network_model_manager_->getNetworkModel()->disconnect();
        }
    } catch (const std::exception& e) {
        std::cerr << "系统关闭异常: " << e.what() << std::endl;
    }
}

bool X30InspectionSystem::isConnected() const {
    if (network_model_manager_) {
        if (auto network_model = network_model_manager_->getNetworkModel(); network_model) {
            return network_model->isConnected();
        }
    }
    return false;
}

bool X30InspectionSystem::startInspection() {
    if (nav_state_procedure_) {
        handleError("Command: start: 执行失败，已有巡检任务在执行中");
        return false;
    }

    try {
        // TODO: add UT, introduce GoogleTest
        // 创建导航上下文
        state::NavigationContext nav_context{
            message_queue_,
            network_model_manager_->getNetworkModel()
        };
        nav_state_procedure_ = std::make_unique<procedure::NavigationProcedure>(std::move(nav_context));
        nav_state_procedure_->start();

        std::shared_ptr<common::NavigationTaskEvent> event = std::make_shared<common::NavigationTaskEvent>();
        event->status = "已启动";
        common::EventBus::getInstance().publish(event);

        return true;
    } catch (const std::exception& e) {
        handleError(fmt::format("Command: start: 执行失败，{}", e.what()));
        nav_state_procedure_.reset();

        return false;
    }
}

bool X30InspectionSystem::cancelInspection() {
    if (nav_state_procedure_) {
        protocol::CancelTaskRequest request;
        request.timestamp = common::getCurrentTimestamp();
        network_model_manager_->getNetworkModel()->sendMessage(request);
        std::cout << fmt::format("[{}]: 用户触发发送1004 Request", request.timestamp) << std::endl;
        return true;
    }

    handleError("Command: cancel: 执行失败，由于导航过程管理器不存在");

    return false;

}

bool X30InspectionSystem::queryStatus() {
    if (nav_state_procedure_) {
        protocol::QueryStatusRequest request;
        request.timestamp = common::getCurrentTimestamp();
        network_model_manager_->getNetworkModel()->sendMessage(request);
        std::cout << fmt::format("[{}]: 用户触发发送1007 Request", request.timestamp) << std::endl;
        return true;
    }

    handleError("Command: status: 执行失败，由于导航过程管理器不存在");

    return false;
}

void X30InspectionSystem::resetNavProcedure() {
    nav_state_procedure_.reset();

    std::shared_ptr<common::NavigationTaskEvent> event = std::make_shared<common::NavigationTaskEvent>();
    event->status = "已完成";
    common::EventBus::getInstance().publish(event);
}

// TODO: 当业务复杂度增加时，需要增加concurrency 逻辑， 包括两种 1.消息处理 2.状态机处理
// TODO: 针对Resp消息，当逻辑复杂度增加时可以改成EvenBus，比如增加messageId check等
void X30InspectionSystem::messageProcessingLoop() {
    while (message_queue_running_) {
        auto message = message_queue_.pop();
        if (message) {
            switch (message->getType()) {
                case protocol::MessageType::NAVIGATION_TASK_REQ: { // 导航任务请求
                    startInspection();
                    break;
                }
                case protocol::MessageType::CANCEL_TASK_REQ: { // 取消任务请求
                    cancelInspection();
                    break;
                }
                case protocol::MessageType::QUERY_STATUS_REQ: { // 状态查询请求
                    queryStatus();
                    break;
                }
                case protocol::MessageType::PROCEDURE_RESET: { // 导航过程重置
                    resetNavProcedure();
                    break;
                }
                case protocol::MessageType::GET_REAL_TIME_STATUS_RESP: { // 实时状态响应
                    auto event = common::GetRealTimeStatusEvent::fromResponse(
                        dynamic_cast<const protocol::GetRealTimeStatusResponse&>(*message));
                    common::EventBus::getInstance().publish(event);
                    break;
                }
                case protocol::MessageType::NAVIGATION_TASK_RESP: { // 导航任务响应
                    if (nav_state_procedure_) {
                        nav_state_procedure_->process_event(*message);
                    }
                    break;
                }
                case protocol::MessageType::CANCEL_TASK_RESP: { // 取消任务响应
                    if (nav_state_procedure_) {
                        nav_state_procedure_->process_event(*message);
                    }
                    break;
                }
                case protocol::MessageType::QUERY_STATUS_RESP: { // 状态查询响应
                    if (nav_state_procedure_) {
                        nav_state_procedure_->process_event(*message);
                    }

                    auto event = common::QueryStatusEvent::fromResponse(
                            dynamic_cast<const protocol::QueryStatusResponse&>(*message));
                    common::EventBus::getInstance().publish(event);
                    break;
                }
                default : {
                    handleError("收到异常消息");
                    break;
                }
            }
        }
    }
}

void X30InspectionSystem::handleError(const std::string& error) const {
    std::shared_ptr<common::ErrorEvent> event = std::make_shared<common::ErrorEvent>();
    event->code = -1;
    event->message = error;
    common::EventBus::getInstance().publish(event);
}

void X30InspectionSystem::handleCommand(const std::string& command)
{
    std::unique_ptr<protocol::IMessage> msg{};
    if (command == "start") {
        msg = std::make_unique<protocol::NavigationTaskRequest>();
    }
    else if (command == "cancel") {
        msg = std::make_unique<protocol::CancelTaskRequest>();
    }
    else if (command == "status") {
        msg = std::make_unique<protocol::QueryStatusRequest>();
    }

    if (msg) {
        message_queue_.push(std::move(msg));
    }
    else {
        handleError(fmt::format("command转换消息失败 {}", command));
    }
}

} // namespace application
