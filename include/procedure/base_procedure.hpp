#pragma once

namespace protocol {
class IMessage;
} // namespace protocol
namespace procedure {

class BaseProcedure
{
public:
    virtual void start() = 0;
    virtual void process_event(const protocol::IMessage& message) = 0;
};

} // namespace procedure
