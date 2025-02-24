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



}   // namespace state
