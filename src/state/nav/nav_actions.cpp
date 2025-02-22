#include "state/nav/nav_actions.hpp"
#include <iostream>
#include "common/utils.hpp"
#include "state/nav/nav_context.hpp"

// namespace {
// // 辅助函数：获取当前时间戳
// std::string getCurrentTimestamp() {
//     auto now = std::chrono::system_clock::now();
//     auto time = std::chrono::system_clock::to_time_t(now);
//     std::stringstream ss;
//     ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
//     return ss.str();
// }
// } // namespace

namespace state {
void send_nav_request::execute(state::NavigationContext& context)
{
    std::cout << "[NavFsm:Action]: 发送1003请求--下发导航任务，导航点数量: " << context.points.size() << std::endl;
    // 发送导航请求
    protocol::NavigationTaskRequest req;
    req.points = context.points;
    req.timestamp = common::getCurrentTimestamp();
    context.communication.sendMessage(req);
}

void send_cancel_request::execute(state::NavigationContext& context)
{
    std::cout << "[NavFsm:Action]: 发送1004请求--取消导航任务" << std::endl;
    // 发送取消请求
    protocol::CancelTaskRequest req;
    req.timestamp = common::getCurrentTimestamp();
    context.communication.sendMessage(req);
}

void send_query_request::execute(state::NavigationContext& context)
{
    std::cout << "[NavFsm:Action]: 发送1007请求--查询设备运行状态" << std::endl;
    // 发送查询请求
    protocol::QueryStatusRequest req;
    req.timestamp = common::getCurrentTimestamp();
    context.communication.sendMessage(req);
}
} // namespace state