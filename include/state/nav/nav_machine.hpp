#pragma once

#include <boost/mpl/aux_/fold_impl.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/function.hpp>
#include <boost/proto/matches.hpp>
#include <iostream>
#include "protocol/x30_protocol.hpp"
#include <functional>
#include "state/nav/nav_context.hpp"
#include "state/nav/nav_actions.hpp"
#include "state/nav/nav_guards.hpp"
#include "state/nav/nav_states.hpp"

namespace state {

// 使用协议定义的消息类型
using protocol::NavigationTaskRequest;
using protocol::CancelTaskRequest;
using protocol::QueryStatusRequest;
using protocol::NavigationTaskResponse;
using protocol::CancelTaskResponse;
using protocol::QueryStatusResponse;

// 状态机定义
struct NavigationMachine_ : public boost::msm::front::state_machine_def<NavigationMachine_> {

    // 初始状态
    typedef Init initial_state;

    // 终止回调
    std::function<void()> terminate_callback_;

    // 设置终止回调
    void set_terminate_callback(std::function<void()> callback) {
        terminate_callback_ = std::move(callback);
    }

    // 终止处理
    void on_terminate() {
        if (terminate_callback_) {
            terminate_callback_();
        }
    }

    // 转换表
    struct transition_table : boost::mpl::vector<
    boost::msm::front::Row<Init, boost::msm::front::none, PrepareEnterNav, send_nav_request, boost::msm::front::none>,
    // boost::msm::front::Row<Init, NavigationTaskRequest, PrepareEnterNav, send_nav_request, boost::msm::front::none>,
    // boost::msm::front::Row<PrepareEnterNav, CancelTaskRequest, PrepareEnterNav, send_cancel_request, boost::msm::front::none>,
    // boost::msm::front::Row<PrepareEnterNav, QueryStatusRequest, PrepareEnterNav, send_query_request, boost::msm::front::none>,
    boost::msm::front::Row<PrepareEnterNav, NavigationTaskResponse, Done, boost::msm::front::none, boost::msm::front::none>,
    boost::msm::front::Row<PrepareEnterNav, CancelTaskResponse, Done, boost::msm::front::none, check_resp2004_guard>,
    boost::msm::front::Row<PrepareEnterNav, QueryStatusResponse, Done, boost::msm::front::none, check_resp_status_completed_guard>,
    boost::msm::front::Row<PrepareEnterNav, QueryStatusResponse, Nav, boost::msm::front::none, check_resp_status_executing_guard>,
    // boost::msm::front::Row<Nav, CancelTaskRequest, Nav, send_cancel_request, boost::msm::front::none>,
    // boost::msm::front::Row<Nav, QueryStatusRequest, Nav, send_query_request, boost::msm::front::none>,
    boost::msm::front::Row<Nav, CancelTaskResponse, Done, boost::msm::front::none, check_resp2004_guard>,
    boost::msm::front::Row<Nav, NavigationTaskResponse, Done, boost::msm::front::none, boost::msm::front::none>,
    boost::msm::front::Row<Nav, QueryStatusResponse, Done, boost::msm::front::none, check_resp_status_completed_guard>
    > {};

    // 处理未定义的转换
    template <class FSM, class Event>
    void no_transition(Event const&, FSM&, int state) {
        std::cout << "[NavFsm:WRN]: 无法处理当前状态(" << state << ")下的事件" << std::endl;
    }

    // context引用
    // NavigationMachine_(NavigationContext& context)
    // : context_(context) {}
    NavigationMachine_(NavigationContext context)
        : context_(std::move(context)) { }

    NavigationContext context_;
};

// 后端状态机定义
typedef boost::msm::back::state_machine<NavigationMachine_> NavigationMachine;

// 状态查询函数
inline bool isInInit(const NavigationMachine& machine) {
    return machine.is_flag_active<Init>();
}

inline bool isInPrepareEnterNav(const NavigationMachine& machine) {
    return machine.is_flag_active<PrepareEnterNav>();
}

inline bool isInNav(const NavigationMachine& machine) {
    return machine.is_flag_active<Nav>();
}

inline bool isInDone(const NavigationMachine& machine) {
    return machine.is_flag_active<Done>();
}

} // namespace state
