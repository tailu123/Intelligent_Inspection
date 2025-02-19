#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "protocol/x30_protocol.hpp"
#include "state/x30_state_machine.hpp"
#include "communication/x30_communication.hpp"
#include "application/MessageQueue.hpp"
#include "application/event_bus.hpp"

namespace x30 {
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
    void shutdown();

    // 回调设置
    void setCallback(const InspectionCallback& callback);

    // 状态查询
    bool isConnected() const;
    bool isInspecting() const;

    // 新增统一消息处理接口
    void handleCommand(std::unique_ptr<protocol::IMessage> command) {
        message_queue_.push(std::move(command));
    }

    // 事件总线相关方法
    template<typename T>
    std::string subscribeEvent(std::function<void(const std::shared_ptr<Event>&)> handler) {
        return EventBus::getInstance().subscribe<T>(handler);
    }

    void unsubscribeEvent(const std::string& eventType, const std::string& handlerId) {
        EventBus::getInstance().unsubscribe(eventType, handlerId);
    }

protected:
    // 事件发布方法
    void publishEvent(const std::shared_ptr<Event>& event) {
        EventBus::getInstance().publish(event);
    }

private:
    // 巡检任务管理
    bool startInspection();
    bool cancelInspection();
    bool queryStatus();

    // 内部处理函数
    void handleMessage(std::unique_ptr<protocol::IMessage> message);
    void handleError(const std::string& error);
    void handleStateChange(const state::X30StateMachine& machine);

    // 消息处理循环
    void messageProcessingLoop();

    // 事件处理方法
    void handleMessageResponse(const protocol::IMessage& message);
    void handleConnectionStatus(bool connected, const std::string& message);
    void handleNavigationStatus(bool completed, const std::string& point, const std::string& status);

    // 成员变量
    std::unique_ptr<communication::AsyncCommunicationManager> comm_manager_;
    std::unique_ptr<state::X30StateMachine> state_machine_;
    InspectionCallback callback_;
    std::atomic<bool> is_inspecting_;

    // 消息队列
    MessageQueue message_queue_;
    std::thread message_thread_;
    std::atomic<bool> running_{false};
    std::vector<protocol::NavigationPoint> points_;
};

} // namespace application
} // namespace x30