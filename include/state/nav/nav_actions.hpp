#pragma once

#include "state/nav/nav_context.hpp"
#include "state/nav/nav_state_machine.hpp"

namespace state
{
// 发送导航请求动作
class ActionSendNavRequest {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
        execute(fsm.context_);
    }

private:
    void execute(state::NavigationContext& context)
    {
        // 发送导航请求
        protocol::NavigationTaskRequest req;
        req.points = context.points;
        req.timestamp = "2021-08-01 12:00:00";
        context.communication.sendMessage(req);
        // TODO: getCurrentTimestamp();
        // 使用通信模块发送请求
        // context.asyncCommunicationManager.getCommunication()->sendMessage(req);
        // auto communication = context.asyncCommunicationManager.getCommunication();
        // communication->sendMessage(req);
    }
};

// 发送取消请求动作
class ActionSendCancelRequest {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
        execute(fsm.context_);
    }

private:
    void execute(state::NavigationContext& context)
    {
        // 发送取消请求
        protocol::CancelTaskRequest req;
        req.timestamp = "2021-08-01 12:00:00";
        context.communication.sendMessage(req);
    }
};


}   // namespace state
