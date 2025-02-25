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
    context.network_model->sendMessage(req);
}

void send_get_real_time_status_request::execute(state::NavigationContext& context)
{
    // std::cout << "[NavFsm:Action]: 发送1002请求--获取实时状态" << std::endl;
    protocol::GetRealTimeStatusRequest req;
    req.timestamp = common::getCurrentTimestamp();
    context.network_model->sendMessage(req);
}

// void send_query_status_request::execute(state::NavigationContext& context)
// {
//     std::cout << "[NavFsm:Action]: 发送1007请求--查询状态" << std::endl;
//     protocol::QueryStatusRequest req;
//     req.timestamp = common::getCurrentTimestamp();
//     context.network_model->sendMessage(req);
// }
} // namespace state
