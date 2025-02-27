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
// #include <fmt/core.h>
#include <spdlog/spdlog.h>
#include "common/utils.hpp"

namespace state {

// 使用协议定义的消息类型
using protocol::NavigationTaskRequest;
using protocol::CancelTaskRequest;
using protocol::QueryStatusRequest;
using protocol::NavigationTaskResponse;
using protocol::CancelTaskResponse;
using protocol::QueryStatusResponse;
using protocol::GetRealTimeStatusResponse;
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
    boost::msm::front::Row<Init, boost::msm::front::none, PrepareEnterNav, SendNavRequest, boost::msm::front::none>,
    boost::msm::front::Row<PrepareEnterNav, NavigationTaskResponse, Done, boost::msm::front::none, boost::msm::front::none>,
    boost::msm::front::Row<PrepareEnterNav, CancelTaskResponse, Done, boost::msm::front::none, CheckCancelTaskResponseSuccess>,
    boost::msm::front::Row<PrepareEnterNav, QueryStatusResponse, Done, boost::msm::front::none, CheckStatusQueryResponseCompleted>,
    boost::msm::front::Row<PrepareEnterNav, QueryStatusResponse, Nav, boost::msm::front::none, CheckStatusQueryResponseExecuting>,
    boost::msm::front::Row<Nav, CancelTaskResponse, Done, boost::msm::front::none, CheckCancelTaskResponseSuccess>,
    boost::msm::front::Row<Nav, NavigationTaskResponse, Done, boost::msm::front::none, boost::msm::front::none>,
    boost::msm::front::Row<Nav, QueryStatusResponse, Nav, SendGetRealTimeStatusRequest, CheckStatusQueryResponseExecuting>,
    boost::msm::front::Row<Nav, QueryStatusResponse, Done, boost::msm::front::none, CheckStatusQueryResponseCompleted>
    > {};

    // 处理未定义的转换
    template <class FSM, class Event>
    void no_transition(Event const&, FSM&, int state) {
        spdlog::warn("[{}]: [NavFsm:WRN]: 无法处理当前状态({})下的事件", common::getCurrentTimestamp(), state);
        // std::cout << fmt::format("[{}]: [NavFsm:WRN]: 无法处理当前状态({})下的事件", common::getCurrentTimestamp(), state) << std::endl;
    }

    NavigationMachine_(NavigationContext context)
        : context_(std::move(context)) { }

    NavigationContext context_;
};

// 后端状态机定义
typedef boost::msm::back::state_machine<NavigationMachine_> NavigationMachine;

} // namespace state
