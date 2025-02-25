#pragma once

#include <boost/mpl/aux_/fold_impl.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/mpl/vector.hpp>
#include "protocol/x30_protocol.hpp"

namespace state {

// Guard包装器
struct CheckStatusQueryResponseExecuting {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) const {
        return execute(evt);
    }
private:
    bool execute(const protocol::QueryStatusResponse& resp) const;
};

struct CheckStatusQueryResponseCompleted {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) const {
        return execute(evt);
    }
private:
    bool execute(const protocol::QueryStatusResponse& resp) const;
};

struct CheckCancelTaskResponseSuccess {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    bool operator()(EVT const& evt, FSM&, SourceState&, TargetState&) const {
        return execute(evt);
    }
private:
    bool execute(const protocol::CancelTaskResponse& resp) const;
};

} // namespace state
