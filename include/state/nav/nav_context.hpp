#pragma once
#include <vector>
#include <memory>
#include "protocol/x30_protocol.hpp"
#include "common/event_bus.hpp"
#include "application/x30_inspection_system.hpp"
#include "network/x30_communication.hpp"

namespace application {
class X30InspectionSystem; // 前向声明 TODO: why need this? 循环依赖
} // namespace application

namespace state
{
// 导航上下文
struct NavigationContext {
    std::vector<protocol::NavigationPoint>& points;
    common::EventBus& event_bus;
    application::X30InspectionSystem& inspection_system;
    common::MessageQueue& message_queue; // TODO: optimize namespace, add procedure, add arch
    network::X30Communication& communication;

    NavigationContext(
        std::vector<protocol::NavigationPoint>& p,
        common::EventBus& eb,
        application::X30InspectionSystem& is,
        common::MessageQueue& mq,
        network::X30Communication& comm
    ) : points(p)
        , event_bus(eb)
        , inspection_system(is)
        , message_queue{mq}
        , communication(comm) {}
};

}  // namespace state
