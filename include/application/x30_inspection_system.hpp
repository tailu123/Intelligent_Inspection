#pragma once

#include <atomic>
#include <memory>
#include <string>
#include "network/network_model_manager.hpp"
#include "common/message_queue.hpp"
#include <thread>

namespace procedure {
class BaseProcedure;
} // namespace procedure

namespace application {

class X30InspectionSystem {
public:
    X30InspectionSystem();
    ~X30InspectionSystem();

    // 系统初始化和清理
    bool initialize(const std::string& host, uint16_t port);

    // 状态查询
    bool isConnected() const;

    // 处理用户输入的指令
    void handleCommand(const std::string& command);

    // 析构清理资源
    void shutdown();
private:

    // 巡检任务管理
    bool startInspection();
    bool cancelInspection();
    bool queryStatus();

    // 重置 nav_state_procedure_
    void resetNavProcedure();

    // 错误回调，目前即打印
    void handleError(const std::string& error) const;

    // 消息处理循环
    void messageProcessingLoop();

    // 打印日志
    void printLog(const protocol::QueryStatusResponse& resp) const;
    void printLog(const protocol::GetRealTimeStatusResponse& resp) const;

    // 消息队列
    common::MessageQueue message_queue_;
    std::atomic<bool> message_queue_running_{false};
    std::thread message_thread_;

    // 网络通信接口
    std::unique_ptr<network::NetworkModelManager> network_model_manager_;

    // 业务核心
    std::unique_ptr<procedure::BaseProcedure> nav_state_procedure_;
};

} // namespace application
