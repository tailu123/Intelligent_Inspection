#pragma once

#include <memory>
#include "state/nav/nav_machine.hpp"
#include "procedure/base_procedure.hpp"

namespace procedure {

class NavigationProcedure : public BaseProcedure {
public:
    explicit NavigationProcedure(state::NavigationContext context);
    ~NavigationProcedure();

    void start() override;
    void process_event(const protocol::IMessage& message) override;

private:
    std::unique_ptr<state::NavigationMachine> state_machine_;
};

} // namespace procedure
