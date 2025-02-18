#include "application/x30_inspection_system.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace x30 {
namespace application {

// 辅助函数：获取当前时间戳
static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

X30InspectionSystem::X30InspectionSystem()
    : comm_manager_(std::make_unique<communication::AsyncCommunicationManager>())
    , state_machine_(std::make_unique<state::X30StateMachine>())
    , is_inspecting_(false) {
}

X30InspectionSystem::~X30InspectionSystem() {
    shutdown();
}

bool X30InspectionSystem::initialize(const std::string& host, uint16_t port) {
    try {
        // 启动通信管理器
        comm_manager_->start();
        auto comm = comm_manager_->getCommunication();

        // 设置消息回调
        comm->setMessageCallback([this](std::unique_ptr<protocol::IMessage> message) {
            handleMessage(std::move(message));
        });

        // 设置错误回调
        comm->setErrorCallback([this](const std::string& error) {
            handleError(error);
        });

        // 连接到设备
        comm->connect(host, port);

        return true;
    } catch (const std::exception& e) {
        if (callback_.onError) {
            callback_.onError(-1, std::string("初始化失败: ") + e.what());
        }
        return false;
    }
}

void X30InspectionSystem::shutdown() {
    if (is_inspecting_) {
        cancelInspection();
    }

    if (comm_manager_) {
        comm_manager_->stop();
    }
}

bool X30InspectionSystem::startInspection(const std::vector<protocol::NavigationPoint>& points) {
    if (is_inspecting_) {
        if (callback_.onError) {
            callback_.onError(-1, "已有巡检任务在执行中");
        }
        return false;
    }

    try {
        auto comm = comm_manager_->getCommunication();
        if (!comm || !comm->isConnected()) {
            throw std::runtime_error("通信未连接");
        }

        // 创建并发送导航任务消息
        protocol::NavigationTaskMessage msg;
        msg.points = points;
        msg.timestamp = getCurrentTimestamp();
        comm->sendMessage(msg);

        // 更新状态机
        state::EvNavigationStart evt;
        evt.points = points;
        state_machine_->process_event(evt);

        is_inspecting_ = true;

        if (callback_.onStarted) {
            callback_.onStarted();
        }

        return true;
    } catch (const std::exception& e) {
        if (callback_.onError) {
            callback_.onError(-1, std::string("启动巡检失败: ") + e.what());
        }
        return false;
    }
}

bool X30InspectionSystem::cancelInspection() {
    if (!is_inspecting_) {
        return true;
    }

    try {
        auto comm = comm_manager_->getCommunication();
        if (!comm || !comm->isConnected()) {
            throw std::runtime_error("通信未连接");
        }

        // 创建并发送取消任务消息
        protocol::CancelTaskMessage msg;
        msg.timestamp = getCurrentTimestamp();
        comm->sendMessage(msg);

        // 更新状态机
        state::EvNavigationCancel evt;
        state_machine_->process_event(evt);

        is_inspecting_ = false;

        return true;
    } catch (const std::exception& e) {
        if (callback_.onError) {
            callback_.onError(-1, std::string("取消巡检失败: ") + e.what());
        }
        return false;
    }
}

bool X30InspectionSystem::queryStatus() {
    try {
        auto comm = comm_manager_->getCommunication();
        if (!comm || !comm->isConnected()) {
            throw std::runtime_error("通信未连接");
        }

        // 创建并发送状态查询消息
        protocol::QueryStatusMessage msg;
        msg.timestamp = getCurrentTimestamp();
        comm->sendMessage(msg);

        return true;
    } catch (const std::exception& e) {
        if (callback_.onError) {
            callback_.onError(-1, std::string("查询状态失败: ") + e.what());
        }
        return false;
    }
}

void X30InspectionSystem::setCallback(const InspectionCallback& callback) {
    callback_ = callback;
}

bool X30InspectionSystem::isConnected() const {
    auto comm = comm_manager_->getCommunication();
    return comm && comm->isConnected();
}

bool X30InspectionSystem::isInspecting() const {
    return is_inspecting_;
}

void X30InspectionSystem::handleMessage(std::unique_ptr<protocol::IMessage> message) {
    switch (message->getType()) {
        case protocol::MessageType::NAVIGATION_TASK: {
            // 处理导航任务响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("导航任务已接收");
            }
            break;
        }
        case protocol::MessageType::CANCEL_TASK: {
            // 处理取消任务响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("取消任务已接收");
            }
            break;
        }
        case protocol::MessageType::QUERY_STATUS: {
            // 处理状态查询响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("状态查询已完成");
            }
            break;
        }
    }
}

void X30InspectionSystem::handleError(const std::string& error) {
    if (callback_.onError) {
        callback_.onError(-1, error);
    }
}

void X30InspectionSystem::handleStateChange(const state::X30StateMachine& machine) {
    // 根据状态机状态更新系统状态
    if (state::isNavigating(machine)) {
        is_inspecting_ = true;
        if (callback_.onStatusUpdate) {
            callback_.onStatusUpdate("正在执行巡检任务");
        }
    } else if (state::isError(machine)) {
        is_inspecting_ = false;
        if (callback_.onError) {
            callback_.onError(-1, "巡检任务出错");
        }
    } else {
        is_inspecting_ = false;
        if (callback_.onCompleted) {
            callback_.onCompleted();
        }
    }
}

} // namespace application
} // namespace x30