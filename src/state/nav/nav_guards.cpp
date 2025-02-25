#include "state/nav/nav_guards.hpp"
#include <iostream>
namespace state
{
bool check_resp_status_executing_guard::execute(const protocol::QueryStatusResponse& resp) const
{
    // std::cout << "receive 1007 Resp with  value: " << resp.value << ", status: "
    //     << static_cast<int>(resp.status) << std::endl;
    return resp.status == protocol::NavigationStatus::EXECUTING;
}

bool check_resp_status_completed_guard::execute(const protocol::QueryStatusResponse& resp) const
{
    // std::cout << "receive 1007 Resp with  value: " << resp.value << ", status: " << static_cast<int>(resp.status) << std::endl;
    return resp.status == protocol::NavigationStatus::COMPLETED;
}

bool check_resp2004_guard::execute(const protocol::CancelTaskResponse& resp) const
{
    return resp.errorCode == protocol::ErrorCode::SUCCESS;
}


// // Guard包装器
// struct check_resp_status_executing_guard {
// public:
//     template <class EVT, class FSM, class SourceState, class TargetState>
//     bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) const {
//         return execute(evt);
//     }
// private:
//     bool execute(protocol::QueryStatusResponse const& resp) const {
//         std::cout << "receive 1007 Resp with  value: " << resp.value << ", status: " << static_cast<int>(resp.status) << std::endl;
//         return resp.status == protocol::NavigationStatus::EXECUTING;
//     }
// };

// struct check_resp_status_completed_guard {
// public:
//     template <class EVT, class FSM, class SourceState, class TargetState>
//     bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) const {
//         return execute(evt);
//     }
// private:
//     bool execute(protocol::QueryStatusResponse const& resp) const {
//         std::cout << "receive 1007 Resp with  value: " << resp.value << ", status: " << static_cast<int>(resp.status) << std::endl;
//         return resp.status == protocol::NavigationStatus::COMPLETED;
//     }
// };

// struct check_resp2004_guard {
// public:
//     template <class EVT, class FSM, class SourceState, class TargetState>
//     bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) const {
//         return execute(evt);
//     }
// private:
//     bool execute(protocol::CancelTaskResponse const& resp) const {
//         return resp.errorCode == protocol::ErrorCode::SUCCESS;
//     }
// };

} // namespace state
