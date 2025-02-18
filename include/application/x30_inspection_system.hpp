#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "../protocol/x30_protocol.hpp"
#include "../state/x30_state_machine.hpp"
#include "../communication/x30_communication.hpp"

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

    // 巡检任务管理
    bool startInspection(const std::vector<protocol::NavigationPoint>& points);
    bool cancelInspection();
    bool queryStatus();

    // 回调设置
    void setCallback(const InspectionCallback& callback);

    // 状态查询
    bool isConnected() const;
    bool isInspecting() const;

private:
    // 内部处理函数
    void handleMessage(std::unique_ptr<protocol::IMessage> message);
    void handleError(const std::string& error);
    void handleStateChange(const state::X30StateMachine& machine);

    // 成员变量
    std::unique_ptr<communication::AsyncCommunicationManager> comm_manager_;
    std::unique_ptr<state::X30StateMachine> state_machine_;
    InspectionCallback callback_;
    bool is_inspecting_;
};

} // namespace application
} // namespace x30