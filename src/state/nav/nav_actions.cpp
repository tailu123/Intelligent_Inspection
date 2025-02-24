#include "state/nav/nav_actions.hpp"
#include <iostream>
#include "common/utils.hpp"
#include "state/nav/nav_context.hpp"

namespace state {
void send_nav_request::execute(state::NavigationContext& context)
{
    static auto points = common::loadNavigationPoints();
    std::cout << "[NavFsm:Action]: 发送1003请求--下发导航任务，导航点数量: " << points.size() << std::endl;
    protocol::NavigationTaskRequest req;
    req.points = points;
    req.timestamp = common::getCurrentTimestamp();
    context.communication->sendMessage(req);
}
} // namespace state
