#include "application/x30_inspection_system.hpp"
#include "procedure/nav_procedure/nav_procedure.hpp"
#include "common/event_bus.hpp"
#include "network/x30_communication.hpp"
#include "protocol/x30_protocol.hpp"
#include <iostream>
#include <chrono>
#include "common/utils.hpp"
#include "state/nav/nav_context.hpp"
#include <fmt/core.h>

namespace application {
X30InspectionSystem::X30InspectionSystem()
    : message_queue_running_(false) {
}

X30InspectionSystem::~X30InspectionSystem() {
    shutdown();
}

bool X30InspectionSystem::initialize(const std::string& host, uint16_t port) {
    // 初始化通信管理器
    network_model_manager_ = std::make_unique<network::NetworkModelManager>(
        network::NetworkModelType::ASIO, message_queue_);
    network_model_manager_->start();

    // TODO: 隐藏到 start() 中
    // 设置错误回调
    auto comm = network_model_manager_->getNetworkModel();
    comm->setErrorCallback([this](const std::string& error) {
        handleError(error);
    });

    if (!comm->connect(host, port)) {
        return false;
    }

    // 启动消息处理线程
    message_queue_running_ = true;
    message_thread_ = std::thread(&X30InspectionSystem::messageProcessingLoop, this);

    return true;
}

void X30InspectionSystem::shutdown() {
    try {
        status_query_running_ = false;
        if (status_query_thread_.joinable()) {
            status_query_thread_.join();
        }

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

void X30InspectionSystem::setCallback(const InspectionCallback& callback) {
    callback_ = callback;
}

bool X30InspectionSystem::isConnected() const {
    return network_model_manager_ && network_model_manager_->getNetworkModel()->isConnected();
}

bool X30InspectionSystem::startInspection() {
    if (nav_state_procedure_) {
        if (callback_.onError) {
            callback_.onError(-1, "已有巡检任务在执行中");
        }
        return false;
    }

    try {
        // TODO: add UT, introduce GoogleTest
        // TODO: 隐藏构造细节
        // 创建导航上下文
        state::NavigationContext nav_context{
            common::EventBus::getInstance(),
            message_queue_,
            network_model_manager_->getNetworkModel()
        };
        nav_state_procedure_ = std::make_unique<procedure::NavigationProcedure>(std::move(nav_context));
        nav_state_procedure_->start();

        startStatusQuery();  // 启动状态查询

        if (callback_.onStarted) {
            callback_.onStarted();
        }
        return true;
    } catch (const std::exception& e) {
        if (callback_.onError) {
            callback_.onError(-1, std::string("启动巡检失败: ") + e.what());
        }
        nav_state_procedure_.reset();
        return false;
    }
}

bool X30InspectionSystem::cancelInspection() {
    if (nav_state_procedure_) {
        protocol::CancelTaskRequest request;
        request.timestamp = common::getCurrentTimestamp();
        std::cout << "用户触发发送1004 Request: " << request.timestamp << std::endl;
        network_model_manager_->getNetworkModel()->sendMessage(request);
        return true;
    }

    if (callback_.onError) {
        callback_.onError(-1, "导航过程管理器不存在");
    }
    return false;

}

bool X30InspectionSystem::queryStatus() {
    if (nav_state_procedure_) {
        protocol::QueryStatusRequest request;
        request.timestamp = common::getCurrentTimestamp();
        std::cout << "用户触发发送1007 Request: " << request.timestamp << std::endl;
        network_model_manager_->getNetworkModel()->sendMessage(request);
        return true;
    }

    if (callback_.onError) {
        callback_.onError(-1, "导航过程管理器不存在");
    }
    return false;
}

// TODO: 当业务复杂度增加时，需要增加concurrency 逻辑， 包括两种 1.消息处理 2.状态机处理
void X30InspectionSystem::messageProcessingLoop() {
    while (message_queue_running_) {
        auto message = message_queue_.pop();
        if (message) {
            switch (message->getType()) {
                case protocol::MessageType::NAVIGATION_TASK_REQ: {
                    startInspection();
                    break;
                }
                case protocol::MessageType::CANCEL_TASK_REQ: {
                    cancelInspection();
                    break;
                }
                case protocol::MessageType::QUERY_STATUS_REQ: {
                    queryStatus();
                    break;
                }
                case protocol::MessageType::PROCEDURE_RESET: {
                    // 重置巡检任务
                    if (nav_state_procedure_) {
                        nav_state_procedure_.reset();
                    }
                    callback_.onCompleted();
                    stopStatusQuery();   // 停止状态查询
                    break;
                }
                case protocol::MessageType::NAVIGATION_TASK_RESP:
                case protocol::MessageType::CANCEL_TASK_RESP:
                case protocol::MessageType::QUERY_STATUS_RESP: {
                    handleMessageResponse(*message);
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

void X30InspectionSystem::handleMessageResponse(const protocol::IMessage& message) {
    // TODO: 当逻辑复杂度增加时可以改成EvenBus，比如增加messageId check等
    // 根据消息类型处理响应
    switch (message.getType()) {
        case protocol::MessageType::NAVIGATION_TASK_RESP: // 导航任务响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("导航任务响应已收到");
                if (nav_state_procedure_) {
                    nav_state_procedure_->process_event(message);
                }
            }
            break;
        case protocol::MessageType::CANCEL_TASK_RESP: // 取消任务响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("取消任务响应已收到");
                if (nav_state_procedure_) {
                    nav_state_procedure_->process_event(message);
                }
            }
            break;
        case protocol::MessageType::QUERY_STATUS_RESP: // 状态查询响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("状态查询响应已收到");
                if (nav_state_procedure_) {
                    nav_state_procedure_->process_event(message);
                }
            }
            break;
        default: {
            handleError("收到异常消息");
            break;
        }
    }
}

void X30InspectionSystem::handleError(const std::string& error) {
    if (callback_.onError) {
        callback_.onError(-1, error);
    }
}

void X30InspectionSystem::startStatusQuery() {
    if (status_query_running_) {
        return;
    }
    status_query_running_ = true;
    status_query_thread_ = std::thread(&X30InspectionSystem::statusQueryLoop, this);
}

void X30InspectionSystem::stopStatusQuery() {
    status_query_running_ = false;
    if (status_query_thread_.joinable()) {
        status_query_thread_.join();
    }
}

void X30InspectionSystem::statusQueryLoop() {
    while (status_query_running_) {
        // 等待指定时间间隔
        std::this_thread::sleep_for(std::chrono::milliseconds(STATUS_QUERY_INTERVAL_MS));

        if (nav_state_procedure_) {
            protocol::QueryStatusRequest request;
            request.timestamp = common::getCurrentTimestamp();
            std::cout << "定时发送1007 Request: " << request.timestamp << std::endl;
            network_model_manager_->getNetworkModel()->sendMessage(request);
        }

    }
}

void X30InspectionSystem::handleCommand(const std::string& command)
{
    std::unique_ptr<protocol::IMessage> msg;
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
