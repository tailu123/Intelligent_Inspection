#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include <rapidxml/rapidxml.hpp>

namespace x30 {
namespace protocol {

// 消息类型枚举
enum class MessageType {
    NAVIGATION_TASK = 1003,
    CANCEL_TASK = 1004,
    QUERY_STATUS = 1007
};

// 错误码枚举
enum class ErrorCode {
    SUCCESS = 0,
    FAILURE = 1,
    CANCELLED = 2
};

// 导航点信息
struct NavigationPoint {
    int mapId = 0;
    int value = 0;
    double posX = 0.0;
    double posY = 0.0;
    double posZ = 0.0;
    double angleYaw = 0.0;
    int pointInfo = 0;
    int gait = 0;
    int speed = 0;
    int manner = 0;
    int obsMode = 0;
    int navMode = 0;
    int terrain = 0;
    int posture = 0;

    // 从JSON对象加载数据
    static NavigationPoint fromJson(const nlohmann::json& j) {
        NavigationPoint point;
        point.mapId = j.value("MapID", 0);
        point.value = j.value("Value", 0);
        point.posX = j.value("PosX", 0.0);
        point.posY = j.value("PosY", 0.0);
        point.posZ = j.value("PosZ", 0.0);
        point.angleYaw = j.value("AngleYaw", 0.0);
        point.pointInfo = j.value("PointInfo", 0);
        point.gait = j.value("Gait", 0);
        point.speed = j.value("Speed", 0);
        point.manner = j.value("Manner", 0);
        point.obsMode = j.value("ObsMode", 0);
        point.navMode = j.value("NavMode", 0);
        point.terrain = j.value("Terrain", 0);
        point.posture = j.value("Posture", 0);
        return point;
    }
};

// 基础消息接口
class IMessage {
public:
    virtual ~IMessage() = default;
    std::string serialize() const;

    virtual bool deserialize(const std::string& xml) = 0;
    virtual MessageType getType() const = 0;

protected:
    virtual std::string serializeToXml() const = 0;
};

// 导航任务消息
class NavigationTaskMessage : public IMessage {
public:
    MessageType getType() const override { return MessageType::NAVIGATION_TASK; }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    std::vector<NavigationPoint> points;
    std::string timestamp;
};

// 取消任务消息
class CancelTaskMessage : public IMessage {
public:
    MessageType getType() const override { return MessageType::CANCEL_TASK; }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    std::string timestamp;
};

// 查询状态消息
class QueryStatusMessage : public IMessage {
public:
    MessageType getType() const override { return MessageType::QUERY_STATUS; }
    std::string serializeToXml() const override;
    bool deserialize(const std::string& xml) override;

    std::string timestamp;
};

// 消息工厂
class MessageFactory {
public:
    static std::unique_ptr<IMessage> createMessage(MessageType type);
    static std::unique_ptr<IMessage> parseMessage(const std::string& xml);
};

} // namespace protocol
} // namespace x30