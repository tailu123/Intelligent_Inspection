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

    // ProtocolHeader作为数据类的话，适合定义构造函数吗？
    explicit ProtocolHeader();
    explicit ProtocolHeader(uint16_t length);

    bool validateSyncBytes() const;
    uint16_t getBodySize() const;
};
#pragma pack(pop)

} // namespace protocol
