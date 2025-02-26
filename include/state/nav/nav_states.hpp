#pragma once

#include "state/base_state.hpp"
#include "protocol/x30_protocol.hpp"
#include <memory>
#include "common/message_queue.hpp"
#include "common/utils.hpp"
#include <fmt/core.h>

namespace state {

// 状态定义
struct Init : BaseState<Init> {
    const char* get_state_name() const override { return "初始化"; }
};

struct PrepareEnterNav : BaseState<PrepareEnterNav> {
    const char* get_state_name() const override { return "准备导航"; }
};

struct Nav : public boost::msm::front::state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM&) {
        static bool isfirst = true;
        if (isfirst) {
            std::cout << fmt::format("[{}]: [NavFsm:State]: 进入导航状态", common::getCurrentTimestamp()) << std::endl;
            isfirst = false;
        }
    }

    template <class Event, class FSM>
    void on_exit(Event const&, FSM&) {
    }
};

struct Done : public boost::msm::front::terminate_state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM& fsm) {
        std::cout << fmt::format("[{}]: [NavFsm:State]: 进入结束状态", common::getCurrentTimestamp()) << std::endl;
        fsm.context_.message_queue.clear();
        fsm.context_.message_queue.push(std::make_unique<protocol::ProcedureReset>());
        fsm.on_terminate();
    }
};
} // namespace state
