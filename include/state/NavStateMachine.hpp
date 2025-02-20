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
#include "../protocol/x30_protocol.hpp"
#include <functional>
#include "state/NavigationContext.hpp"
#include "state/NavigationAction.hpp"
// #include "../application/NavStateProcedure.hpp"

namespace x30 {
namespace application {
// struct NavigationContext;
} // namespace application
} // namespace x30

namespace x30::state {

// // 前向声明
// struct NavStateMachine_;
// typedef boost::msm::back::state_machine<NavStateMachine_> NavStateMachine;

// 状态定义
struct Init : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) { std::cout << "进入初始状态" << std::endl; }
    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) { std::cout << "离开初始状态" << std::endl; }
};

struct PrepareEnterNav : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) { std::cout << "进入准备导航状态" << std::endl; }
    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) { std::cout << "离开准备导航状态" << std::endl; }
};

struct Nav : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) { std::cout << "进入导航状态" << std::endl; }
    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) { std::cout << "离开导航状态" << std::endl; }
};

struct Done : public boost::msm::front::terminate_state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM& fsm) {
        std::cout << "进入完成状态" << std::endl;
        fsm.context_.message_queue.clear(); // TODO: can optimize by cleanup_pending
        fsm.context_.message_queue.push(std::make_unique<protocol::ProcedureReset>());
        fsm.on_terminate();
    }
};

// 使用协议定义的消息类型
using protocol::NavigationTaskRequest;
using protocol::CancelTaskRequest;
using protocol::QueryStatusRequest;
using protocol::NavigationTaskResponse;
using protocol::CancelTaskResponse;
using protocol::QueryStatusResponse;

// 状态机定义
struct NavStateMachine_ : public boost::msm::front::state_machine_def<NavStateMachine_> {

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

    // 状态机行为定义
    struct transition_actions {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const&, FSM&, SourceState&, TargetState&) const {
            std::cout << "执行状态转换动作" << std::endl;
        }
    };

    // 动作函数
    // void sendNavRequest(const NavigationTaskRequest& evt) const {
    //     std::cout << "发送导航任务请求，导航点数量: " << evt.points.size() << std::endl;
    // }
    void sendNavRequest(state::NavigationContext& context) const {
        std::cout << "发送导航任务请求，导航点数量: " << context.points.size() << std::endl;
        auto message = NavigationTaskRequest{};
        message.points = context.points;
        message.timestamp = "2021-08-01 12:00:00";
        context.communication.sendMessage(message);
    }

    void sendCancelRequest(state::NavigationContext& context) const {
        std::cout << "发送取消请求" << std::endl;
        // auto message = CancelTaskRequest{};
        protocol::CancelTaskRequest req;
        req.timestamp = "2021-08-01 12:00:00";
        context.communication.sendMessage(req);
    }

    void sendQueryRequest(state::NavigationContext& context) const {
        std::cout << "发送查询请求" << std::endl;
        protocol::QueryStatusRequest req;
        req.timestamp = "2021-08-01 12:00:00";
        context.communication.sendMessage(req);
    }

    // void sendCancelRequest(const CancelTaskRequest&) const {
    //     std::cout << "发送取消请求" << std::endl;
    // }

    // 守卫条件
    bool checkResp2004(const CancelTaskResponse& evt) const {
        return evt.errorCode == protocol::ErrorCode::SUCCESS;
    }

    bool checkRespStatusCompleted(const QueryStatusResponse& evt) const {
        return evt.status == protocol::NavigationStatus::COMPLETED;
    }

    bool checkRespStatusExecuting(const QueryStatusResponse& evt) const {
        return evt.status == protocol::NavigationStatus::EXECUTING;
    }

    // 动作和守卫的包装器
    struct send_nav_request {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
            fsm.sendNavRequest(fsm.context_);
        }
    };

    struct send_cancel_request {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) {
            // fsm.sendCancelRequest(evt);
            fsm.sendCancelRequest(fsm.context_);
        }
    };


    struct send_query_request {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) {
            // fsm.sendCancelRequest(evt);
            fsm.sendQueryRequest(fsm.context_);
        }
    };

    struct check_resp2004_guard {
        template <class EVT, class FSM, class SourceState, class TargetState>
        bool operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) const {
            return fsm.checkResp2004(evt);
        }
    };

    struct check_resp_status_completed_guard {
        template <class EVT, class FSM, class SourceState, class TargetState>
        bool operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) const {
            return fsm.checkRespStatusCompleted(evt);
        }
    };

    struct check_resp_status_executing_guard {
        template <class EVT, class FSM, class SourceState, class TargetState>
        bool operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) const {
            return fsm.checkRespStatusExecuting(evt);
        }
    };

    // 转换表
    // typedef boost::msm::front::Row<Init, NavigationTaskRequest, PrepareEnterNav, send_nav_request, boost::msm::front::none> init_to_prepare;
    // typedef boost::msm::front::Row<PrepareEnterNav, CancelTaskRequest, PrepareEnterNav, send_cancel_request, boost::msm::front::none> prepare_cancel;
    // typedef boost::msm::front::Row<PrepareEnterNav, QueryStatusRequest, PrepareEnterNav, send_cancel_request, boost::msm::front::none> prepare_cancel;
    // typedef boost::msm::front::Row<PrepareEnterNav, NavigationTaskResponse, Done, boost::msm::front::none, boost::msm::front::none> prepare_resp2003;
    // typedef boost::msm::front::Row<PrepareEnterNav, CancelTaskResponse, Done, boost::msm::front::none, check_resp2004_guard> prepare_resp2004;
    // typedef boost::msm::front::Row<PrepareEnterNav, QueryStatusResponse, Done, boost::msm::front::none, check_resp_status_completed_guard> prepare_resp_status_done;
    // typedef boost::msm::front::Row<PrepareEnterNav, QueryStatusResponse, Nav, boost::msm::front::none, check_resp_status_executing_guard> prepare_resp_status_nav;
    // typedef boost::msm::front::Row<Nav, CancelTaskRequest, Nav, send_cancel_request, boost::msm::front::none> nav_cancel;
    // typedef boost::msm::front::Row<Nav, CancelTaskResponse, Done, boost::msm::front::none, check_resp2004_guard> nav_resp2004;
    // typedef boost::msm::front::Row<Nav, NavigationTaskResponse, Done, boost::msm::front::none, boost::msm::front::none> nav_resp2003;
    // typedef boost::msm::front::Row<Nav, QueryStatusResponse, Done, boost::msm::front::none, check_resp_status_completed_guard> nav_resp_status;

    struct transition_table : boost::mpl::vector<
    boost::msm::front::Row<Init, NavigationTaskRequest, PrepareEnterNav, send_nav_request, boost::msm::front::none>,
    boost::msm::front::Row<PrepareEnterNav, CancelTaskRequest, PrepareEnterNav, send_cancel_request, boost::msm::front::none>,
    boost::msm::front::Row<PrepareEnterNav, QueryStatusRequest, PrepareEnterNav, send_query_request, boost::msm::front::none>,
    boost::msm::front::Row<PrepareEnterNav, NavigationTaskResponse, Done, boost::msm::front::none, boost::msm::front::none>,
    boost::msm::front::Row<PrepareEnterNav, CancelTaskResponse, Done, boost::msm::front::none, check_resp2004_guard>,
    boost::msm::front::Row<PrepareEnterNav, QueryStatusResponse, Done, boost::msm::front::none, check_resp_status_completed_guard>,
    boost::msm::front::Row<PrepareEnterNav, QueryStatusResponse, Nav, boost::msm::front::none, check_resp_status_executing_guard>,
    boost::msm::front::Row<Nav, CancelTaskRequest, Nav, send_cancel_request, boost::msm::front::none>,
    boost::msm::front::Row<Nav, QueryStatusRequest, Nav, send_query_request, boost::msm::front::none>,
    boost::msm::front::Row<Nav, CancelTaskResponse, Done, boost::msm::front::none, check_resp2004_guard>,
    boost::msm::front::Row<Nav, NavigationTaskResponse, Done, boost::msm::front::none, boost::msm::front::none>,
    boost::msm::front::Row<Nav, QueryStatusResponse, Done, boost::msm::front::none, check_resp_status_completed_guard>
        // init_to_prepare,
        // prepare_cancel,
        // prepare_resp2003,
        // prepare_resp2004,
        // prepare_resp_status_done,
        // prepare_resp_status_nav,
        // nav_cancel,
        // nav_resp2004,
        // nav_resp2003,
        // nav_resp_status
    > {};

    // 处理未定义的转换
    template <class FSM, class Event>
    void no_transition(Event const&, FSM&, int state) {
        std::cout << "无法处理当前状态(" << state << ")下的事件" << std::endl;
    }

    // context引用
    NavStateMachine_(state::NavigationContext& context)
    : context_(context) {}
    state::NavigationContext context_;
};

// 后端状态机定义
typedef boost::msm::back::state_machine<NavStateMachine_> NavStateMachine;

// 状态查询函数
inline bool isInInit(const NavStateMachine& machine) {
    return machine.is_flag_active<Init>();
}

inline bool isInPrepareEnterNav(const NavStateMachine& machine) {
    return machine.is_flag_active<PrepareEnterNav>();
}

inline bool isInNav(const NavStateMachine& machine) {
    return machine.is_flag_active<Nav>();
}

inline bool isInDone(const NavStateMachine& machine) {
    return machine.is_flag_active<Done>();
}

} // namespace x30::state