#include "state/nav/nav_guards.hpp"
namespace state
{
bool CheckStatusQueryResponseExecuting::execute(const protocol::QueryStatusResponse& resp) const
{
    return resp.status == protocol::NavigationStatus::EXECUTING;
}

bool CheckStatusQueryResponseCompleted::execute(const protocol::QueryStatusResponse& resp) const
{
    return resp.status == protocol::NavigationStatus::COMPLETED;
}

bool CheckCancelTaskResponseSuccess::execute(const protocol::CancelTaskResponse& resp) const
{
    return resp.errorCode == protocol::ErrorCode::SUCCESS;
}
} // namespace state
