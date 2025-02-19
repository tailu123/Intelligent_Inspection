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
        protocol::NavigationTaskMessage msg;
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
        protocol::CancelTaskMessage msg;
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
        protocol::QueryStatusMessage msg;
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

void X30InspectionSystem::messageProcessingLoop() {
    while (running_) {
        auto msg = message_queue_.pop();
        if (msg) {
            switch (msg->getType()) {
                case x30::protocol::MessageType::NAVIGATION_TASK: {
                    startInspection();
                    break;
                }
                case x30::protocol::MessageType::CANCEL_TASK:
                    cancelInspection();
                    break;
                case x30::protocol::MessageType::QUERY_STATUS:
                    queryStatus();
                    break;
                default:
                    handleMessage(std::move(msg));
                    break;

            }
            handleMessage(std::move(msg));
        }
    }
}


} // namespace application
} // namespace x30