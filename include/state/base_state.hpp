#pragma once

#include <iostream>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/mpl/vector.hpp>

// 定义 BaseState
template <typename Derived>
struct BaseState : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) {
        std::cout << "[" << static_cast<Derived*>(this)->get_state_name() << "]: 进入状态" << std::endl;
        // static_cast<Derived*>(this)->do_on_entry();
    }

    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) {
        std::cout << "[" << static_cast<Derived*>(this)->get_state_name() << "]: 离开状态" << std::endl;
        // static_cast<Derived*>(this)->do_on_exit();
    }

    // 定义纯虚函数接口
    // virtual void do_on_entry() = 0;
    // virtual void do_on_exit() = 0;
    virtual const char* get_state_name() const = 0;
};