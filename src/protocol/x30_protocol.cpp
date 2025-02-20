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

// =============== 请求消息实现 ===============

// NavigationTaskRequest实现
std::string NavigationTaskRequest::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::NAVIGATION_TASK_REQ) << "</Type>\n";
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

bool NavigationTaskRequest::deserialize(const std::string& xml) {
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

// CancelTaskRequest实现
std::string CancelTaskRequest::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::CANCEL_TASK_REQ) << "</Type>\n";
    xml << "<Command>1</Command>\n";
    xml << "<Time>" << timestamp << "</Time>\n";
    xml << "<Items/>\n";
    xml << "</PatrolDevice>";
    return xml.str();
}

bool CancelTaskRequest::deserialize(const std::string& xml) {
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

// QueryStatusRequest实现
std::string QueryStatusRequest::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::QUERY_STATUS_REQ) << "</Type>\n";
    xml << "<Command>1</Command>\n";
    xml << "<Time>" << timestamp << "</Time>\n";
    xml << "<Items/>\n";
    xml << "</PatrolDevice>";
    return xml.str();
}

bool QueryStatusRequest::deserialize(const std::string& xml) {
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

// =============== 响应消息实现 ===============

// NavigationTaskResponse实现
std::string NavigationTaskResponse::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::NAVIGATION_TASK_RESP) << "</Type>\n";
    xml << "<Command>1</Command>\n";
    xml << "<Time>" << timestamp << "</Time>\n";
    xml << "<Items>\n";
    xml << "  <Value>" << value << "</Value>\n";
    xml << "  <ErrorCode>" << static_cast<int>(errorCode) << "</ErrorCode>\n";
    xml << "  <ErrorStatus>" << errorStatus << "</ErrorStatus>\n";
    xml << "</Items>\n";
    xml << "</PatrolDevice>";
    return xml.str();
}

bool NavigationTaskResponse::deserialize(const std::string& xml) {
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(const_cast<char*>(xml.c_str()));
        auto root = doc.first_node("PatrolDevice");
        if (!root) return false;

        auto timeNode = root->first_node("Time");
        if (timeNode) timestamp = timeNode->value();

        auto itemsNode = root->first_node("Items");
        if (itemsNode) {
            auto valueNode = itemsNode->first_node("Value");
            if (valueNode) value = std::stoi(valueNode->value());

            auto errorCodeNode = itemsNode->first_node("ErrorCode");
            if (errorCodeNode) errorCode = static_cast<ErrorCode>(std::stoi(errorCodeNode->value()));

            auto errorStatusNode = itemsNode->first_node("ErrorStatus");
            if (errorStatusNode) errorStatus = std::stoi(errorStatusNode->value());
        }

        return true;
    } catch (const rapidxml::parse_error& e) {
        return false;
    }
}

// CancelTaskResponse实现
std::string CancelTaskResponse::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::CANCEL_TASK_RESP) << "</Type>\n";
    xml << "<Command>1</Command>\n";
    xml << "<Time>" << timestamp << "</Time>\n";
    xml << "<Items>\n";
    xml << "  <ErrorCode>" << static_cast<int>(errorCode) << "</ErrorCode>\n";
    xml << "</Items>\n";
    xml << "</PatrolDevice>";
    return xml.str();
}

bool CancelTaskResponse::deserialize(const std::string& xml) {
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(const_cast<char*>(xml.c_str()));
        auto root = doc.first_node("PatrolDevice");
        if (!root) return false;

        auto timeNode = root->first_node("Time");
        if (timeNode) timestamp = timeNode->value();

        auto itemsNode = root->first_node("Items");
        if (itemsNode) {
            auto errorCodeNode = itemsNode->first_node("ErrorCode");
            if (errorCodeNode) errorCode = static_cast<ErrorCode>(std::stoi(errorCodeNode->value()));
        }

        return true;
    } catch (const rapidxml::parse_error& e) {
        return false;
    }
}

// QueryStatusResponse实现
std::string QueryStatusResponse::serializeToXml() const {
    std::stringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<PatrolDevice>\n";
    xml << "<Type>" << static_cast<int>(MessageType::QUERY_STATUS_RESP) << "</Type>\n";
    xml << "<Command>1</Command>\n";
    xml << "<Time>" << timestamp << "</Time>\n";
    xml << "<Items>\n";
    xml << "  <Value>" << value << "</Value>\n";
    xml << "  <Status>" << static_cast<int>(status) << "</Status>\n";
    xml << "  <ErrorCode>" << static_cast<int>(errorCode) << "</ErrorCode>\n";
    xml << "</Items>\n";
    xml << "</PatrolDevice>";
    return xml.str();
}

bool QueryStatusResponse::deserialize(const std::string& xml) {
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(const_cast<char*>(xml.c_str()));
        auto root = doc.first_node("PatrolDevice");
        if (!root) return false;

        auto timeNode = root->first_node("Time");
        if (timeNode) timestamp = timeNode->value();

        auto itemsNode = root->first_node("Items");
        if (itemsNode) {
            auto valueNode = itemsNode->first_node("Value");
            if (valueNode) value = std::stoi(valueNode->value());

            auto statusNode = itemsNode->first_node("Status");
            if (statusNode) status = static_cast<NavigationStatus>(std::stoi(statusNode->value()));

            auto errorCodeNode = itemsNode->first_node("ErrorCode");
            if (errorCodeNode) errorCode = static_cast<ErrorCode>(std::stoi(errorCodeNode->value()));
        }

        return true;
    } catch (const rapidxml::parse_error& e) {
        return false;
    }
}

// MessageFactory实现
std::unique_ptr<IMessage> MessageFactory::createMessage(MessageType type) {
    if (static_cast<int>(type) >= 2000) {
        return createResponseMessage(type);
    } else {
        return createRequestMessage(type);
    }
}

std::unique_ptr<IMessage> MessageFactory::createRequestMessage(MessageType type) {
    switch (type) {
        case MessageType::NAVIGATION_TASK_REQ:
            return std::make_unique<NavigationTaskRequest>();
        case MessageType::CANCEL_TASK_REQ:
            return std::make_unique<CancelTaskRequest>();
        case MessageType::QUERY_STATUS_REQ:
            return std::make_unique<QueryStatusRequest>();
        default:
            return nullptr;
    }
}

std::unique_ptr<IMessage> MessageFactory::createResponseMessage(MessageType type) {
    switch (type) {
        case MessageType::NAVIGATION_TASK_RESP:
            return std::make_unique<NavigationTaskResponse>();
        case MessageType::CANCEL_TASK_RESP:
            return std::make_unique<CancelTaskResponse>();
        case MessageType::QUERY_STATUS_RESP:
            return std::make_unique<QueryStatusResponse>();
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
        std::cerr << "Failed to parse message: " << e.what() << std::endl;
    }
    return nullptr;
}

} // namespace x30::protocol