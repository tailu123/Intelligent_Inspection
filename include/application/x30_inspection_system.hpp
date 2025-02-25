#pragma once

#include <atomic>
#include <memory>
#include <string>
#include "network/x30_communication.hpp"
#include "common/message_queue.hpp"
#include "procedure/nav_procedure/nav_procedure.hpp"

namespace procedure {
class NavigationProcedure;
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

private:
    // 析构清理资源
    void shutdown();

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

    // 网络通信接口
    std::unique_ptr<network::NetworkModelManager> network_model_manager_;

    // 业务核心
    std::unique_ptr<procedure::NavigationProcedure> nav_state_procedure_;

    // 消息队列
    common::MessageQueue message_queue_;
    std::thread message_thread_;
    std::atomic<bool> message_queue_running_{false};
};

} // namespace application
