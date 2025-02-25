#include "procedure/nav_procedure/nav_procedure.hpp"
#include "state/nav/nav_context.hpp"
#include "state/nav/nav_machine.hpp"
#include <iostream>
#include <memory>
#include "common/utils.hpp"
#include "common/event_bus.hpp"
#include "common/utils.hpp"
#include <fmt/core.h>

namespace procedure {

NavigationProcedure::NavigationProcedure(state::NavigationContext context) {
    state_machine_ = std::make_unique<state::NavigationMachine>(std::move(context));

    // 设置状态机终止回调
    state_machine_->set_terminate_callback([this]() {
        stopStatusQuery();
    });
}

NavigationProcedure::~NavigationProcedure()
{
    stopStatusQuery();
}

void NavigationProcedure::start() {
    // 启动状态机
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
            std::cout << fmt::format("[{}]: [NavProc:WRN]: 无法处理当前消息类型", common::getCurrentTimestamp()) << std::endl;
            break;
        }
    }
}

void NavigationProcedure::statusQueryLoop() {
    while (status_query_running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(STATUS_QUERY_INTERVAL_MS));
        protocol::QueryStatusRequest request;
        request.timestamp = common::getCurrentTimestamp();
        state_machine_->context_.network_model->sendMessage(request);
        std::cout << fmt::format("[{}]: [NavProc:INFO]: 定时发送1007 Request", request.timestamp) << std::endl;
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
