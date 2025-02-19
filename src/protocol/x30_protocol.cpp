#include "protocol/x30_protocol.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "protocol/protocol_header.hpp"

namespace x30::protocol {

// IMessage实现
std::string IMessage::serialize() const {
    std::string xml_data = serializeToXml();
    ProtocolHeader header(xml_data.size());

    std::string result;
    result.reserve(ProtocolHeader::SIZE + xml_data.size());
    result.append(reinterpret_cast<const char*>(&header), ProtocolHeader::SIZE);
    result.append(xml_data);
    return result;
}

// NavigationTaskMessage实现
std::string NavigationTaskMessage::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::NAVIGATION_TASK) << "</Type>\n";
    xml << "<Command>1</Command>\n";
    xml << "<Time>" << timestamp << "</Time>\n";

    for (const auto& point : points) {
        xml << "<Items>\n";
        xml << "  <MapID>" << point.mapId << "</MapID>\n";
        xml << "  <Value>" << point.value << "</Value>\n";
        xml << "  <PosX>" << point.posX << "</PosX>\n";
        xml << "  <PosY>" << point.posY << "</PosY>\n";
        xml << "  <PosZ>" << point.posZ << "</PosZ>\n";
        xml << "  <AngleYaw>" << point.angleYaw << "</AngleYaw>\n";
        xml << "  <PointInfo>" << point.pointInfo << "</PointInfo>\n";
        xml << "  <Gait>" << point.gait << "</Gait>\n";
        xml << "  <Speed>" << point.speed << "</Speed>\n";
        xml << "  <Manner>" << point.manner << "</Manner>\n";
        xml << "  <ObsMode>" << point.obsMode << "</ObsMode>\n";
        xml << "  <NavMode>" << point.navMode << "</NavMode>\n";
        xml << "  <Terrain>" << point.terrain << "</Terrain>\n";
        xml << "  <Posture>" << point.posture << "</Posture>\n";
        xml << "</Items>\n";
    }

    xml << "</PatrolDevice>";
    return xml.str();
}

bool NavigationTaskMessage::deserialize(const std::string& xml) {
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(const_cast<char*>(xml.c_str()));

        auto root = doc.first_node("PatrolDevice");
        if (!root) return false;

        auto timeNode = root->first_node("Time");
        if (timeNode) timestamp = timeNode->value();

        points.clear();
        for (auto itemNode = root->first_node("Items");
             itemNode;
             itemNode = itemNode->next_sibling("Items")) {
            NavigationPoint point;

            auto getValue = [](rapidxml::xml_node<>* parent, const char* name) -> std::string {
                auto node = parent->first_node(name);
                return node ? node->value() : "";
            };

            point.mapId = std::stoi(getValue(itemNode, "MapID"));
            point.value = std::stoi(getValue(itemNode, "Value"));
            point.posX = std::stod(getValue(itemNode, "PosX"));
            point.posY = std::stod(getValue(itemNode, "PosY"));
            point.posZ = std::stod(getValue(itemNode, "PosZ"));
            point.angleYaw = std::stod(getValue(itemNode, "AngleYaw"));
            point.pointInfo = std::stoi(getValue(itemNode, "PointInfo"));
            point.gait = std::stoi(getValue(itemNode, "Gait"));
            point.speed = std::stoi(getValue(itemNode, "Speed"));
            point.manner = std::stoi(getValue(itemNode, "Manner"));
            point.obsMode = std::stoi(getValue(itemNode, "ObsMode"));
            point.navMode = std::stoi(getValue(itemNode, "NavMode"));
            point.terrain = std::stoi(getValue(itemNode, "Terrain"));
            point.posture = std::stoi(getValue(itemNode, "Posture"));

            points.push_back(point);
        }

        return true;
    } catch (const rapidxml::parse_error& e) {
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}

// CancelTaskMessage实现
std::string CancelTaskMessage::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::CANCEL_TASK) << "</Type>\n";
    xml << "<Command>1</Command>\n";
    xml << "<Time>" << timestamp << "</Time>\n";
    xml << "<Items/>\n";
    xml << "</PatrolDevice>";
    return xml.str();
}

bool CancelTaskMessage::deserialize(const std::string& xml) {
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(const_cast<char*>(xml.c_str()));
        auto root = doc.first_node("PatrolDevice");
        if (!root) return false;

        auto timeNode = root->first_node("Time");
        if (timeNode) timestamp = timeNode->value();

        return true;
    } catch (const rapidxml::parse_error& e) {
        return false;
    }
}

// QueryStatusMessage实现
std::string QueryStatusMessage::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::QUERY_STATUS) << "</Type>\n";
    xml << "<Command>1</Command>\n";
    xml << "<Time>" << timestamp << "</Time>\n";
    xml << "<Items/>\n";
    xml << "</PatrolDevice>";
    return xml.str();
}

bool QueryStatusMessage::deserialize(const std::string& xml) {
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(const_cast<char*>(xml.c_str()));
        auto root = doc.first_node("PatrolDevice");
        if (!root) return false;

        auto timeNode = root->first_node("Time");
        if (timeNode) timestamp = timeNode->value();

        return true;
    } catch (const rapidxml::parse_error& e) {
        return false;
    }
}

// MessageFactory实现
std::unique_ptr<IMessage> MessageFactory::createMessage(MessageType type) {
    switch (type) {
        case MessageType::NAVIGATION_TASK:
            return std::make_unique<NavigationTaskMessage>();
        case MessageType::CANCEL_TASK:
            return std::make_unique<CancelTaskMessage>();
        case MessageType::QUERY_STATUS:
            return std::make_unique<QueryStatusMessage>();
        default:
            return nullptr;
    }
}

std::unique_ptr<IMessage> MessageFactory::parseMessage(const std::string& xml) {
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(const_cast<char*>(xml.c_str()));
        auto root = doc.first_node("PatrolDevice");
        if (!root) return nullptr;

        auto typeNode = root->first_node("Type");
        if (!typeNode) return nullptr;

        auto type = static_cast<MessageType>(std::stoi(typeNode->value()));
        auto message = createMessage(type);
        if (message && message->deserialize(xml)) {
            return message;
        }
    } catch (const std::exception& e) {
        // 解析失败
    }
    return nullptr;
}
} // namespace protocol