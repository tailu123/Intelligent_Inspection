#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <functional>
#include "protocol/x30_protocol.hpp"
#include "network/x30_communication.hpp"
#include "common/message_queue.hpp"
#include "common/event_bus.hpp"
#include "procedure/nav_procedure/nav_procedure.hpp"

namespace procedure {
class NavigationProcedure;
} // namespace procedure

namespace application {
// 巡检任务状态回调
struct InspectionCallback {
    std::function<void()> onStarted;
    std::function<void()> onCompleted;
    std::function<void(int, const std::string&)> onError;
    std::function<void(const std::string&)> onStatusUpdate;
};

class X30InspectionSystem {
public:
    X30InspectionSystem();
    ~X30InspectionSystem();

    // 系统初始化和清理
    bool initialize(const std::string& host, uint16_t port);

    // 回调设置
    void setCallback(const InspectionCallback& callback);

    // 状态查询
    bool isConnected() const;

    // 处理用户输入的指令
    void handleCommand(const std::string& command);

    // // 事件总线相关方法
    // template<typename T>
    // std::string subscribeEvent(std::function<void(const std::shared_ptr<common::Event>&)> handler) {
    //     return common::EventBus::getInstance().subscribe<T>(handler);
    // }

    // void unsubscribeEvent(const std::string& eventType, const std::string& handlerId) {
    //     common::EventBus::getInstance().unsubscribe(eventType, handlerId);
    // }

private:
    // 事件发布方法
    void publishEvent(const std::shared_ptr<common::Event>& event) {
        common::EventBus::getInstance().publish(event);
    }

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

    // 响应处理方法
    void handleMessageResponse(const protocol::IMessage& message);

    // 网络通信接口
    std::unique_ptr<network::NetworkModelManager> network_model_manager_;

    // TODO: optimize by eventBus, instead of callback_
    InspectionCallback callback_;

    // 业务核心
    std::unique_ptr<procedure::NavigationProcedure> nav_state_procedure_;

    // 消息队列
    common::MessageQueue message_queue_;
    std::thread message_thread_;
    std::atomic<bool> message_queue_running_{false};
};

} // namespace application
