#pragma once

// #include "state/nav/nav_context.hpp"

namespace state
{
struct NavigationContext;

// Action包装器
struct send_nav_request {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
        execute(fsm.context_);
    }
private:
    void execute(state::NavigationContext& context);
};

// 发送获取实时状态请求
struct send_get_real_time_status_request {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
        execute(fsm.context_);
    }
private:
    void execute(state::NavigationContext& context);
};

// 发送查询状态请求
// struct send_query_status_request {
// public:
//     template <class EVT, class FSM, class SourceState, class TargetState>
//     void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
//         execute(fsm.context_);
//     }
// private:
//     void execute(state::NavigationContext& context);
// };

}   // namespace state
