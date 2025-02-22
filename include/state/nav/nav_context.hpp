#pragma once

#include <vector>
#include "protocol/x30_protocol.hpp"
#include "common/event_bus.hpp"
#include "network/x30_communication.hpp"

namespace application {
class X30InspectionSystem; // 前向声明 TODO: why need this? 循环依赖
} // namespace application

namespace state
{
// 导航上下文
struct NavigationContext {

    NavigationContext(
        std::vector<protocol::NavigationPoint>& p,
        common::EventBus& eb,
        common::MessageQueue& mq,
        network::X30Communication& comm
        // std::shared_ptr<network::X30Communication> comm
    ) : points(p)
    , event_bus(eb)
    , message_queue{mq}
    , communication(comm) {}

    std::vector<protocol::NavigationPoint>& points;
    common::EventBus& event_bus;
    common::MessageQueue& message_queue;
    network::X30Communication& communication;
    // std::shared_ptr<network::X30Communication> communication;
};

}  // namespace state
