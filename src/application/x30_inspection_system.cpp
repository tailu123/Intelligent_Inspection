#include "application/x30_inspection_system.hpp"
#include "procedure/nav_procedure/nav_procedure.hpp"
#include "common/event_bus.hpp"
#include "network/x30_communication.hpp"
#include "protocol/x30_protocol.hpp"
#include <iostream>
#include <chrono>
#include "common/utils.hpp"
#include "state/nav/nav_context.hpp"

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
    points_ = common::loadNavigationPoints();

    // 初始化通信管理器
    // 启动通信管理器
    comm_manager_ = std::make_unique<network::AsyncCommunicationManager>(message_queue_);
    comm_manager_->start();
    // 设置消息回调
    auto comm = comm_manager_->getCommunication();
    // comm->setMessageCallback([this](std::unique_ptr<protocol::IMessage> message) {
    //     // handleMessage(std::move(message));
    //     message_queue_.push(std::move(message));
    // });

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
    try {
        status_query_running_ = false;
        if (status_query_thread_.joinable()) {
            status_query_thread_.join();
        }

        if (running_) {
            running_ = false;
            if (message_thread_.joinable()) {
                message_thread_.join();
            }
        }

        if (comm_manager_) {
            comm_manager_->getCommunication()->disconnect();
        }
    } catch (const std::exception& e) {
        std::cerr << "系统关闭异常: " << e.what() << std::endl;
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
        // TODO: 隐藏构造细节
        // state::NavigationContext nav_context{
        //     points_,
        //     common::EventBus::getInstance(),
        //     message_queue_,
        //     comm_manager_->getCommunication()
        // };
        // nav_state_procedure_ = std::make_unique<procedure::NavigationProcedure>(nav_context);
        //
        //
        //
        nav_state_procedure_ = std::make_unique<procedure::NavigationProcedure>(
            points_,
            *comm_manager_,
            common::EventBus::getInstance(),
            message_queue_);
        // nav_state_procedure_ = std::make_unique<NavStateProcedure>(std::move(nav_context));
        nav_state_procedure_->start();

        // 发布导航请求事件
        // auto event = std::make_shared<SendNavRequestEvent>();
        // event->points = points_;
        // EventBus::getInstance().publish(event);

        is_inspecting_ = true;
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
    // if (!is_inspecting_) {
    //     return false;
    // }
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


    // stopStatusQuery();   // 停止状态查询
    // is_inspecting_ = false;

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
                    is_inspecting_ = false;
                    callback_.onCompleted();
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
        auto event = std::make_shared<common::MessageResponseEvent>();
        // event->messageId = message->getMessageId();
        event->success = true; // 根据实际情况设置
        // event->data = message->toString();
        // EventBus::getInstance().publish(event);

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

// void X30InspectionSystem::handleConnectionStatus(bool connected, const std::string& message) {
//     if (callback_.onStatusUpdate) {
//         callback_.onStatusUpdate(message);
//     }
// }

void X30InspectionSystem::handleNavigationStatus(bool completed, const std::string&, const std::string& status) {
    if (completed && callback_.onCompleted) {
        callback_.onCompleted();
        is_inspecting_ = false;
    } else if (callback_.onStatusUpdate) {
        callback_.onStatusUpdate(status);
    }
}

void X30InspectionSystem::startStatusQuery() {
    if (status_query_running_) {
        return;
    }
    status_query_running_ = true;
    status_query_thread_ = std::thread(&X30InspectionSystem::statusQueryLoop, this);
}

// void X30InspectionSystem::stopStatusQuery() {
//     status_query_running_ = false;
//     if (status_query_thread_.joinable()) {
//         status_query_thread_.join();
//     }
// }

void X30InspectionSystem::statusQueryLoop() {
    while (status_query_running_) {
        // 等待指定时间间隔
        std::this_thread::sleep_for(std::chrono::milliseconds(STATUS_QUERY_INTERVAL_MS));

        if (nav_state_procedure_) {
            std::cout << "状态查询发送\n";

        // if (is_inspecting_ && nav_state_procedure_) {
            // 创建并发送查询请求
            protocol::QueryStatusRequest request;
            request.timestamp = common::getCurrentTimestamp();
            // message_queue_.push(std::make_unique<protocol::QueryStatusRequest>(std::move(request)));
            comm_manager_->getCommunication()->sendMessage(request);
        }

    }
}

} // namespace application
