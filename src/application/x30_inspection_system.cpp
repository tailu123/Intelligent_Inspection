#include "application/x30_inspection_system.hpp"
#include "application/NavStateProcedure.hpp"
#include "application/event_bus.hpp"
#include "communication/x30_communication.hpp"
#include "protocol/x30_protocol.hpp"
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
    : is_inspecting_(false)
    , running_(false) {
}

X30InspectionSystem::~X30InspectionSystem() {
    shutdown();
}

bool X30InspectionSystem::initialize(const std::string& host, uint16_t port) {
    // 加载导航点
    points_ = loadNavigationPoints();

    // 初始化通信管理器
    // 启动通信管理器
    comm_manager_ = std::make_unique<communication::AsyncCommunicationManager>();
    comm_manager_->start();
    // 设置消息回调
    auto comm = comm_manager_->getCommunication();
    comm->setMessageCallback([this](std::unique_ptr<protocol::IMessage> message) {
        // handleMessage(std::move(message));
        message_queue_.push(std::move(message));
    });

    // 设置错误回调
    comm->setErrorCallback([this](const std::string& error) {
        handleError(error);
    });
    comm->connect(host, port);
    // if (!comm->connect(host, port)) {
    //     return false;
    // }

    // 启动消息处理线程
    running_ = true;
    message_thread_ = std::thread(&X30InspectionSystem::messageProcessingLoop, this);

    return true;
}

void X30InspectionSystem::shutdown() {
    if (running_) {
        running_ = false;
        if (message_thread_.joinable()) {
            message_thread_.join();
        }
    }

    if (comm_manager_) {
        comm_manager_->getCommunication()->disconnect();
    }
}

void X30InspectionSystem::setCallback(const InspectionCallback& callback) {
    callback_ = callback;
}

bool X30InspectionSystem::isConnected() const {
    return comm_manager_ && comm_manager_->getCommunication()->isConnected();
}

bool X30InspectionSystem::isInspecting() const {
    return is_inspecting_;
}

bool X30InspectionSystem::startInspection() {
    if (is_inspecting_) {
        if (callback_.onError) {
            callback_.onError(-1, "已有巡检任务在执行中");
        }
        return false;
    }

    if (nav_state_procedure_) {
        if (callback_.onError) {
            callback_.onError(-1, "导航过程管理器已存在");
        }
        return false;
    }

    try {
        // 创建导航上下文
        // NavigationContext nav_context{
        //     points_,
        //     *comm_manager_,
        //     EventBus::getInstance(),
        //     *this,
        //     // *comm_manager_
        // };

        // 创建导航过程管理器
        nav_state_procedure_ = std::make_unique<NavStateProcedure>(
            points_,
            *comm_manager_,
            EventBus::getInstance(),
            *this,
            message_queue_);
        // nav_state_procedure_ = std::make_unique<NavStateProcedure>(std::move(nav_context));
        nav_state_procedure_->start();

        // 发布导航请求事件
        // auto event = std::make_shared<SendNavRequestEvent>();
        // event->points = points_;
        // EventBus::getInstance().publish(event);

        is_inspecting_ = true;
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
    if (!is_inspecting_) {
        return false;
    }
    // nav_state_procedure_->process_message(const std::shared_ptr<Event> &event);
    // 发布取消请求事件
    // auto event = std::make_shared<CancelRequestEvent>();
    // EventBus::getInstance().publish(event);
    if (nav_state_procedure_) {
        nav_state_procedure_->cancelInspection();
    }
    else {
        if (callback_.onError) {
            callback_.onError(-1, "导航过程管理器不存在");
        }
    }


    return true;
}

bool X30InspectionSystem::queryStatus() {
    if (!is_inspecting_) {
        return false;
    }

    if (nav_state_procedure_) {
        nav_state_procedure_->queryStatus();
    }
    else {
        if (callback_.onError) {
            callback_.onError(-1, "导航过程管理器不存在");
        }
    }

    // TODO: 实现状态查询逻辑
    return true;
}

void X30InspectionSystem::messageProcessingLoop() {
    while (running_) {
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
                    break;
                }
                default: {
                    handleMessage(std::move(message));
                    break;
                }
            }
        }
    }
}

void X30InspectionSystem::handleMessage(std::unique_ptr<protocol::IMessage> message) {
    if (!message) {
        return;
    }

    try {
        // 发布消息响应事件
        auto event = std::make_shared<MessageResponseEvent>();
        // event->messageId = message->getMessageId();
        event->success = true; // 根据实际情况设置
        // event->data = message->toString();
        EventBus::getInstance().publish(event);

        // 处理消息响应
        handleMessageResponse(*message);
    } catch (const std::exception& e) {
        handleError(e.what());
    }
}

void X30InspectionSystem::handleMessageResponse(const protocol::IMessage& message) {
    // 根据消息类型处理响应
    switch (message.getType()) {
        case protocol::MessageType::NAVIGATION_TASK_RESP: // 导航任务响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("导航任务响应已收到");
                if (nav_state_procedure_) {
                    nav_state_procedure_->process_message(message);
                }
            }
            break;
        case protocol::MessageType::CANCEL_TASK_RESP: // 取消任务响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("取消任务响应已收到");
                if (nav_state_procedure_) {
                    nav_state_procedure_->process_message(message);
                }
            }
            break;
        case protocol::MessageType::QUERY_STATUS_RESP: // 状态查询响应
            if (callback_.onStatusUpdate) {
                callback_.onStatusUpdate("状态查询响应已收到");
                if (nav_state_procedure_) {
                    nav_state_procedure_->process_message(message);
                }
            }
            break;
        default:
            break;
    }
}

void X30InspectionSystem::handleError(const std::string& error) {
    if (callback_.onError) {
        callback_.onError(-1, error);
    }
}

void X30InspectionSystem::handleConnectionStatus(bool connected, const std::string& message) {
    if (callback_.onStatusUpdate) {
        callback_.onStatusUpdate(message);
    }
}

void X30InspectionSystem::handleNavigationStatus(bool completed, const std::string& point, const std::string& status) {
    if (completed && callback_.onCompleted) {
        callback_.onCompleted();
        is_inspecting_ = false;
    } else if (callback_.onStatusUpdate) {
        callback_.onStatusUpdate(status);
    }
}

} // namespace application
} // namespace x30