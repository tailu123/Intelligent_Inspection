#include "procedure/nav_procedure/nav_procedure.hpp"
#include "state/nav/nav_context.hpp"
#include "state/nav/nav_machine.hpp"
#include <iostream>
#include <memory>
#include "common/utils.hpp"

namespace procedure {

NavigationProcedure::NavigationProcedure(state::NavigationContext context) {
    state_machine_ = std::make_unique<state::NavigationMachine>(std::move(context));

    // 设置状态机终止回调
    // TODO: 当业务复杂后，可以写清理代码

    // 设置状态机终止回调
    state_machine_->set_terminate_callback([this]() {
        // 清理代码
        stopStatusQuery();
    });
}

NavigationProcedure::~NavigationProcedure()
{
    stopStatusQuery();
}

void NavigationProcedure::start() {
    // 启动状态机
    std::cout << "启动状态机" << std::endl;
    state_machine_->start();
    startStatusQuery();
}

void NavigationProcedure::process_event(const protocol::IMessage& message) {
    // TODO: 如果交互协议多，可以使用EventBus进行消息分发
    switch (message.getType()) {
        case protocol::MessageType::NAVIGATION_TASK_RESP: {
            auto& resp = dynamic_cast<const protocol::NavigationTaskResponse&>(message);
            state_machine_->process_event(resp);
            break;
        }
        case protocol::MessageType::CANCEL_TASK_RESP: {
            auto& resp = dynamic_cast<const protocol::CancelTaskResponse&>(message);
            state_machine_->process_event(resp);
            break;
        }
        case protocol::MessageType::QUERY_STATUS_RESP: {
            auto& resp = dynamic_cast<const protocol::QueryStatusResponse&>(message);
            auto event = common::QueryStatusEvent::fromResponse(resp);
            common::EventBus::getInstance().publish(event);
            state_machine_->process_event(resp);
            break;
        }
        default: {
            std::cout << "[NavProc:WRN]: 无法处理当前消息类型" << std::endl;
            break;
        }
    }
}

void NavigationProcedure::statusQueryLoop() {
    while (status_query_running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(STATUS_QUERY_INTERVAL_MS));
        protocol::QueryStatusRequest request;
        request.timestamp = common::getCurrentTimestamp();
        // std::cout << "定时发送1007 Request: " << request.timestamp << std::endl;
        state_machine_->context_.network_model->sendMessage(request);
    }
}

void NavigationProcedure::startStatusQuery() {
    if (status_query_running_) {
        return;
    }

    status_query_running_ = true;
    status_query_thread_ = std::thread(&NavigationProcedure::statusQueryLoop, this);
}

void NavigationProcedure::stopStatusQuery() {
    status_query_running_ = false;
    if (status_query_thread_.joinable()) {
        status_query_thread_.join();
    }
}

} // namespace procedure
