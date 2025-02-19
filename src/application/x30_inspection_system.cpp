#include "application/x30_inspection_system.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <optional>


namespace  {
// TODO: move to suitable place
// 辅助函数：加载默认导航点
std::vector<x30::protocol::NavigationPoint> loadDefaultNavigationPoints(const std::string& configPath) {
    std::vector<x30::protocol::NavigationPoint> points;
    try {
        // 检查文件是否存在
        if (!std::filesystem::exists(configPath)) {
            std::cerr << "配置文件不存在: " << configPath << std::endl;
            return points;
        }

        // 读取JSON文件
        std::ifstream file(configPath);
        nlohmann::json jsonArray;
        file >> jsonArray;

        // 解析每个导航点
        for (const auto& jsonPoint : jsonArray) {
            points.push_back(x30::protocol::NavigationPoint::fromJson(jsonPoint));
        }

        std::cout << "成功从配置文件加载了 " << points.size() << " 个导航点" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "加载配置文件失败: " << e.what() << std::endl;
    }
    return points;
}

// 辅助函数：加载默认导航点
std::vector<x30::protocol::NavigationPoint> loadNavigationPoints() {
    std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
    std::filesystem::path projectRoot = exePath.parent_path().parent_path();
    std::filesystem::path configPath = projectRoot / "config" / "default_params.json";

    std::vector<x30::protocol::NavigationPoint> points;
    points = loadDefaultNavigationPoints(configPath.string());
    if (points.empty()) {
        std::cout << "警告: 未能加载默认导航点，将使用示例导航点\n";
        points = {
            {0, 1, -4.2181582, 3.4758759, -0.056337897, -3.044234, 0, 0, 1, 0, 0, 0, 0, 0},
            {0, 2, -9.1335344, 2.9462891, 0.093159825, -1.4948614, 0, 0, 1, 0, 0, 0, 0, 0}
        };
    }

    return points;
}

// 辅助函数：获取当前时间戳
static std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
}   // namespace



namespace x30 {
namespace application {
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
        // 加载导航点
        points_ = loadNavigationPoints();

        // 启动通信管理器
        comm_manager_->start();
        auto comm = comm_manager_->getCommunication();

        // 设置消息回调
        comm->setMessageCallback([this](std::unique_ptr<protocol::IMessage> message) {
            // handleMessage(std::move(message));
            message_queue_.push(std::move(message));
        });

        // 设置错误回调
        comm->setErrorCallback([this](const std::string& error) {
            handleError(error);
        });

        // 连接到设备
        comm->connect(host, port);

        running_ = true;
        message_thread_ = std::thread(&X30InspectionSystem::messageProcessingLoop, this);

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

bool X30InspectionSystem::startInspection() {
    if (is_inspecting_) {
        if (callback_.onError) {
            callback_.onError(-1, "已有巡检任务在执行中");
        }
        return false;
    }

    try {
        if (not isConnected()) {
            throw std::runtime_error("通信未连接");
        }

        // 创建并发送导航任务消息
        protocol::NavigationTaskRequest msg;
        msg.points = points_;
        msg.timestamp = getCurrentTimestamp();
        auto comm = comm_manager_->getCommunication();
        comm->sendMessage(msg);

        // 更新状态机
        state::EvNavigationStart evt;
        evt.points = points_;
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
        if (not isConnected()) {
            throw std::runtime_error("通信未连接");
        }

        // 创建并发送取消任务消息
        protocol::CancelTaskRequest msg;
        msg.timestamp = getCurrentTimestamp();
        auto comm = comm_manager_->getCommunication();
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
        if (not isConnected()) {
            throw std::runtime_error("通信未连接");
        }

        // 创建并发送状态查询消息
        protocol::QueryStatusRequest msg;
        msg.timestamp = getCurrentTimestamp();
        auto comm = comm_manager_->getCommunication();
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
        case protocol::MessageType::NAVIGATION_TASK_RESP: {
            auto resp = static_cast<protocol::NavigationTaskResponse*>(message.get());
            auto responseEvent = std::make_shared<MessageResponseEvent>();
            responseEvent->messageId = static_cast<uint32_t>(message->getType());
            responseEvent->success = (resp->errorCode == protocol::ErrorCode::SUCCESS);

            if (resp->errorCode == protocol::ErrorCode::SUCCESS) {
                responseEvent->data = "导航任务已接受: " + std::to_string(resp->value);
            } else {
                responseEvent->data = "导航任务失败，错误状态: " + std::to_string(resp->errorStatus);
            }

            publishEvent(responseEvent);

            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate(responseEvent->data);
            }
            break;
        }
        case protocol::MessageType::CANCEL_TASK_RESP: {
            auto resp = static_cast<protocol::CancelTaskResponse*>(message.get());
            auto responseEvent = std::make_shared<MessageResponseEvent>();
            responseEvent->messageId = static_cast<uint32_t>(message->getType());
            responseEvent->success = (resp->errorCode == protocol::ErrorCode::SUCCESS);
            responseEvent->data = resp->errorCode == protocol::ErrorCode::SUCCESS ?
                "取消任务成功" : "取消任务失败";

            publishEvent(responseEvent);

            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate(responseEvent->data);
            }
            break;
        }
        case protocol::MessageType::QUERY_STATUS_RESP: {
            auto resp = static_cast<protocol::QueryStatusResponse*>(message.get());
            auto responseEvent = std::make_shared<MessageResponseEvent>();
            responseEvent->messageId = static_cast<uint32_t>(message->getType());
            responseEvent->success = (resp->errorCode == protocol::ErrorCode::SUCCESS);

            std::string statusStr;
            switch (resp->status) {
                case protocol::NavigationStatus::COMPLETED:
                    statusStr = "已完成";
                    break;
                case protocol::NavigationStatus::EXECUTING:
                    statusStr = "执行中";
                    break;
                case protocol::NavigationStatus::FAILED:
                    statusStr = "执行失败";
                    break;
            }

            responseEvent->data = "导航状态: " + statusStr + ", 值: " + std::to_string(resp->value);
            publishEvent(responseEvent);

            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate(responseEvent->data);
            }
            break;
        }
        default:
            if (callback_.onError) {
                callback_.onError(-1, "未知消息类型");
            }
            break;
    }
}

void X30InspectionSystem::handleError(const std::string& error) {
    auto errorEvent = std::make_shared<ErrorEvent>();
    errorEvent->code = -1;
    errorEvent->message = error;
    publishEvent(errorEvent);

    if (callback_.onError) {
        callback_.onError(-1, error);
    }
}

void X30InspectionSystem::handleStateChange(const state::X30StateMachine& machine) {
    auto statusEvent = std::make_shared<NavigationStatusEvent>();

    if (state::isNavigating(machine)) {
        statusEvent->completed = false;
        statusEvent->status = "导航中";
        statusEvent->currentPoint = "当前导航点信息"; // 这里可以添加具体的导航点信息
    } else if (state::isIdle(machine)) {
        statusEvent->completed = true;
        statusEvent->status = "空闲";
    } else if (state::isError(machine)) {
        statusEvent->completed = false;
        statusEvent->status = "错误";
    }

    publishEvent(statusEvent);
}

void X30InspectionSystem::handleMessageResponse(const protocol::IMessage& message) {
    auto responseEvent = std::make_shared<MessageResponseEvent>();
    responseEvent->messageId = static_cast<uint32_t>(message.getType());
    responseEvent->success = true;
    responseEvent->data = "消息处理成功";
    publishEvent(responseEvent);
}

void X30InspectionSystem::handleConnectionStatus(bool connected, const std::string& message) {
    auto statusEvent = std::make_shared<ConnectionStatusEvent>();
    statusEvent->connected = connected;
    statusEvent->message = message;
    publishEvent(statusEvent);
}

void X30InspectionSystem::handleNavigationStatus(bool completed, const std::string& point, const std::string& status) {
    auto statusEvent = std::make_shared<NavigationStatusEvent>();
    statusEvent->completed = completed;
    statusEvent->currentPoint = point;
    statusEvent->status = status;
    publishEvent(statusEvent);
}

void X30InspectionSystem::messageProcessingLoop() {
    while (running_) {
        auto msg = message_queue_.pop();
        if (msg) {
            switch (msg->getType()) {
            case x30::protocol::MessageType::NAVIGATION_TASK_REQ: {
                startInspection();
                break;
            }
            case x30::protocol::MessageType::CANCEL_TASK_REQ: {
                cancelInspection();
                break;
            }
            case x30::protocol::MessageType::QUERY_STATUS_REQ: {
                queryStatus();
                break;
            }
            default: {
                handleMessage(std::move(msg));
                break;
            }
            }
        }
    }
}


} // namespace application
} // namespace x30