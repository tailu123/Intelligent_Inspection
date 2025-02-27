#pragma once

namespace state {
struct NavigationContext;

// Action包装器
// 发送导航请求
struct SendNavRequest {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
        execute(fsm.context_);
    }

private:
    void execute(state::NavigationContext& context);
};

// 发送获取实时状态请求
struct SendGetRealTimeStatusRequest {
public:
    template <class EVT, class FSM, class SourceState, class TargetState>
    void operator()(EVT const&, FSM& fsm, SourceState&, TargetState&) {
        execute(fsm.context_);
    }

private:
    void execute(state::NavigationContext& context);
};

}  // namespace state
