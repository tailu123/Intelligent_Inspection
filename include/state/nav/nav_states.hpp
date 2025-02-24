#pragma once

// #include <boost/mpl/aux_/fold_impl.hpp>
// #include <boost/msm/front/state_machine_def.hpp>
// #include <boost/msm/back/state_machine.hpp>
// #include <boost/mpl/vector.hpp>
#include "state/base_state.hpp"
#include "protocol/x30_protocol.hpp"
#include <memory>
#include "common/message_queue.hpp"
// #include "state/nav/nav_context.hpp"
// #include "state/nav/nav_machine.hpp"

namespace state {

// 状态定义
struct Init : BaseState<Init> {
    const char* get_state_name() const override { return "NavFsm:State:Init"; }
};

struct PrepareEnterNav : BaseState<PrepareEnterNav> {
    const char* get_state_name() const override { return "NavFsm:State:PrepareEnterNav"; }
};

struct Nav : BaseState<Nav> {
    const char* get_state_name() const override { return "NavFsm:State:Nav"; }
};

struct Done : public boost::msm::front::terminate_state<> {
    template <class Event, class FSM>
    void on_entry(Event const&, FSM& fsm) {
        // TODO: can optimize by cleanup_pending when business complexity grows
        std::cout << "[NavFsm:State:Done]: 进入完成状态" << std::endl;
        fsm.context_.message_queue.clear();
        fsm.context_.message_queue.push(std::make_unique<protocol::ProcedureReset>());
        fsm.on_terminate();
    }
};
} // namespace state
