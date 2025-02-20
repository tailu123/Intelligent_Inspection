#pragma once
#include <vector>
#include <memory>
#include "protocol/x30_protocol.hpp"
#include "application/event_bus.hpp"
#include "application/x30_inspection_system.hpp"
#include "communication/x30_communication.hpp"

namespace x30
{
namespace state
{
// 导航上下文
struct NavigationContext {
    std::vector<protocol::NavigationPoint>& points;
    application::EventBus& event_bus;
    application::X30InspectionSystem& inspection_system;
    application::MessageQueue& message_queue; // TODO: optimize namespace, add procedure, add arch
    communication::X30Communication& communication;

    NavigationContext(
        std::vector<protocol::NavigationPoint>& p,
        application::EventBus& eb,
        application::X30InspectionSystem& is,
        application::MessageQueue& mq,
        communication::X30Communication& comm
    ) : points(p)
        , event_bus(eb)
        , inspection_system(is)
        , message_queue{mq}
        , communication(comm) {}
};

}  // namespace state
}  // namespace x30