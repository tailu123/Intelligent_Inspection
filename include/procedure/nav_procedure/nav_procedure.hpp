#pragma once

#include <memory>
#include "state/nav/nav_machine.hpp"
#include "procedure/base_procedure.hpp"
#include <atomic>
#include <thread>

namespace procedure {

class NavigationProcedure : public BaseProcedure {
public:
    explicit NavigationProcedure(state::NavigationContext context);
    ~NavigationProcedure();

    void start() override;
    void process_event(const protocol::IMessage& message) override;

private:
    void statusQueryLoop();
    void startStatusQuery();
    void stopStatusQuery();
    
    std::unique_ptr<state::NavigationMachine> state_machine_;

    // 定时1007状态查询相关
    std::atomic<bool> status_query_running_{false};
    std::thread status_query_thread_;
    static constexpr int STATUS_QUERY_INTERVAL_MS = 1000;
};

} // namespace procedure
