#pragma once

#include <vector>
#include "protocol/x30_protocol.hpp"
#include "common/event_bus.hpp"
// #include "network/x30_communication.hpp"
#include "network/base_network_model.hpp"

namespace common {
class MessageQueue;
} // namespace common

namespace state
{
// 导航上下文
struct NavigationContext {

    // NavigationContext(
    //     const std::vector<protocol::NavigationPoint>& p,
    //     common::EventBus& eb,
    //     common::MessageQueue& mq,
    //     network::BaseNetworkModel& comm
    //     // std::shared_ptr<network::X30Communication> comm
    // ) : points(p)
    // , event_bus(eb)
    // , message_queue{mq}
    // , communication(comm) {}

    // const std::vector<protocol::NavigationPoint>& points;
    common::EventBus& event_bus;
    common::MessageQueue& message_queue;
    // network::BaseNetworkModel& communication;
    std::shared_ptr<network::BaseNetworkModel> communication;
};

}  // namespace state
