#include "procedure/nav_procedure/nav_procedure.hpp"
#include "state/nav/nav_context.hpp"
#include "state/nav/nav_machine.hpp"
#include <iostream>
#include <memory>

namespace procedure {

NavigationProcedure::NavigationProcedure(state::NavigationContext context) {
    state_machine_ = std::make_unique<state::NavigationMachine>(std::move(context));

    // 设置状态机终止回调
    // TODO: 当业务复杂后，可以写清理代码
    state_machine_->set_terminate_callback([]() { });
}

NavigationProcedure::~NavigationProcedure() = default;

void NavigationProcedure::start() {
    // 启动状态机
    std::cout << "启动状态机" << std::endl;
    state_machine_->start();
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
            state_machine_->process_event(resp);
            break;
        }
        default: {
            std::cout << "[NavProc:WRN]: 无法处理当前消息类型" << std::endl;
            break;
        }
    }
}

} // namespace procedure
