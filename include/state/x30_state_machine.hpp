#pragma once

#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <iostream>
#include "../protocol/x30_protocol.hpp"

namespace x30::state {

// 前向声明
struct X30StateMachine_;
typedef boost::msm::back::state_machine<X30StateMachine_> X30StateMachine;

// 事件定义
struct EvNavigationStart {
    std::vector<protocol::NavigationPoint> points;
};
struct EvNavigationCancel {};
struct EvNavigationComplete {};
struct EvNavigationError {
    int errorCode;
    std::string errorMessage;
};

// 状态定义
struct StateIdle : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) { std::cout << "进入空闲状态" << std::endl; }

    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) { std::cout << "离开空闲状态" << std::endl; }
};

struct StateNavigating : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) { std::cout << "进入导航状态" << std::endl; }

    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) { std::cout << "离开导航状态" << std::endl; }
};

struct StateError : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) { std::cout << "进入错误状态" << std::endl; }

    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) { std::cout << "离开错误状态" << std::endl; }
};

// 状态机定义
struct X30StateMachine_ : public boost::msm::front::state_machine_def<X30StateMachine_> {
    // 动作函数
    void startNavigation(const EvNavigationStart& evt) {
        std::cout << "开始导航任务，导航点数量: " << evt.points.size() << std::endl;
    }

    void cancelNavigation(const EvNavigationCancel& evt) {
        std::cout << "取消导航任务" << std::endl;
    }

    void completeNavigation(const EvNavigationComplete& evt) {
        std::cout << "导航任务完成" << std::endl;
    }

    void handleError(const EvNavigationError& evt) {
        std::cout << "导航错误: " << evt.errorCode << " - " << evt.errorMessage << std::endl;
    }

    // 守卫条件
    bool hasValidPoints(const EvNavigationStart& evt) const {
        return !evt.points.empty();
    }

    bool alwaysTrue(const EvNavigationCancel&) const { return true; }
    bool alwaysTrue(const EvNavigationComplete&) const { return true; }
    bool alwaysTrue(const EvNavigationError&) const { return true; }

    // 定义初始状态
    typedef StateIdle initial_state;

    // 动作和守卫的包装器
    struct start_navigation_action {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) {
            fsm.startNavigation(evt);
        }
    };

    struct cancel_navigation_action {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) {
            fsm.cancelNavigation(evt);
        }
    };

    struct complete_navigation_action {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) {
            fsm.completeNavigation(evt);
        }
    };

    struct handle_error_action {
        template <class EVT, class FSM, class SourceState, class TargetState>
        void operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) {
            fsm.handleError(evt);
        }
    };

    struct has_valid_points_guard {
        template <class EVT, class FSM, class SourceState, class TargetState>
        bool operator()(EVT const& evt, FSM& fsm, SourceState&, TargetState&) const {
            return fsm.hasValidPoints(evt);
        }
    };

    struct always_true_guard {
        template <class EVT, class FSM, class SourceState, class TargetState>
        bool operator()(EVT const&, FSM&, SourceState&, TargetState&) const {
            return true;
        }
    };

    // 定义转换表
    struct transition_table : boost::mpl::vector<
        //  +-----------+------------------+---------------+------------------------+----------------------+
        //  | 起始状态  | 触发事件         | 目标状态      | 动作                   | 守卫条件             |
        //  +-----------+------------------+---------------+------------------------+----------------------+

        // 从空闲状态开始导航
        boost::msm::front::Row<StateIdle, EvNavigationStart, StateNavigating, start_navigation_action, has_valid_points_guard>,

        // 取消导航任务
        boost::msm::front::Row<StateNavigating, EvNavigationCancel, StateIdle, cancel_navigation_action, always_true_guard>,

        // 导航任务完成
        boost::msm::front::Row<StateNavigating, EvNavigationComplete, StateIdle, complete_navigation_action, always_true_guard>,

        // 导航任务出错
        boost::msm::front::Row<StateNavigating, EvNavigationError, StateError, handle_error_action, always_true_guard>
    > {};

    // 为了提高可读性，定义一个别名
    private:
        template<class Source, class Event, class Target, class Action, class Guard>
        struct _row : boost::msm::front::Row<Source, Event, Target, Action, Guard> {};

    public:
    // 处理未定义的转换
    template <class FSM, class Event>
    void no_transition(Event const&, FSM&, int state) {
        std::cout << "无法处理当前状态(" << state << ")下的事件" << std::endl;
    }
};

// 状态查询函数
inline bool isNavigating(const X30StateMachine& machine) {
    return machine.is_flag_active<StateNavigating>();
}

inline bool isError(const X30StateMachine& machine) {
    return machine.is_flag_active<StateError>();
}

inline bool isIdle(const X30StateMachine& machine) {
    return machine.is_flag_active<StateIdle>();
}

} // namespace x30::state