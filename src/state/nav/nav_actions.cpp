#include "state/nav/nav_actions.hpp"
#include <iostream>
#include "common/utils.hpp"
#include "state/nav/nav_context.hpp"
#include <fmt/core.h>
#include "common/utils.hpp"
namespace state {
void SendNavRequest::execute(state::NavigationContext& context)
{
    static auto points = common::loadNavigationPoints();
    protocol::NavigationTaskRequest req;
    req.points = points;
    req.timestamp = common::getCurrentTimestamp();
    context.network_model->sendMessage(req);
    std::cout << fmt::format("[{}]: [NavFsm:Action]: 发送1003 Request, 导航点数量: {}", req.timestamp, points.size()) << std::endl;
}

void SendGetRealTimeStatusRequest::execute(state::NavigationContext& context)
{
    protocol::GetRealTimeStatusRequest req;
    req.timestamp = common::getCurrentTimestamp();
    context.network_model->sendMessage(req);
    std::cout << fmt::format("[{}]: [NavFsm:Action]: 发送1002 Request", req.timestamp) << std::endl;
}
} // namespace state
