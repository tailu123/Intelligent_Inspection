#pragma once
#include <cstdint>
#include <array>

namespace protocol {

#pragma pack(push, 1)
struct ProtocolHeader {
    uint8_t sync_byte1;
    uint8_t sync_byte2;
    uint8_t sync_byte3;
    uint8_t sync_byte4;
    uint16_t length;
    uint16_t message_id;
    std::array<uint8_t, 8> reserved;

    ProtocolHeader();
    explicit ProtocolHeader(uint16_t length);

    bool validateSyncBytes() const;
    uint16_t getBodySize() const;

    // static constexpr size_t SIZE = 16;  // 头部固定16字节
};
#pragma pack(pop)

} // namespace protocol
