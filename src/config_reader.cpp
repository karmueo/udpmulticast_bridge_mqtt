#include "config_reader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

ConfigReader::ConfigReader(const std::string& config_file)
    : config_file_(config_file), port_(1883), qos_(1), multicast_addr_("224.0.0.1"), multicast_port_(5555), interface_("") {
}

bool ConfigReader::load() {
    std::ifstream file(config_file_);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << config_file_ << std::endl;
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to parse JSON config: " << ex.what() << std::endl;
        return false;
    }

    // MQTT section
    if (j.contains("mqtt")) {
        auto& m = j["mqtt"];
        if (m.contains("broker")) broker_ = m["broker"].get<std::string>();
        if (m.contains("port")) port_ = m["port"].get<int>();
        if (m.contains("topic")) topic_ = m["topic"].get<std::string>();
        if (m.contains("qos")) qos_ = m["qos"].get<int>();
        if (m.contains("client_id")) client_id_ = m["client_id"].get<std::string>();
    }



    // New UDP/multicast section (optional). Support both top-level "udp" and "multicast" sections
    if (j.contains("udp") && j["udp"].is_object()) {
        auto& u = j["udp"];
        if (u.contains("multicast_addr")) multicast_addr_ = u["multicast_addr"].get<std::string>();
        if (u.contains("multicast_port")) multicast_port_ = u["multicast_port"].get<int>();
        if (u.contains("interface")) interface_ = u["interface"].get<std::string>();
    }

    if (j.contains("multicast") && j["multicast"].is_object()) {
        auto& mm = j["multicast"];
        if (mm.contains("addr")) multicast_addr_ = mm["addr"].get<std::string>();
        if (mm.contains("port")) multicast_port_ = mm["port"].get<int>();
    }

    // validation
    if (broker_.empty() || topic_.empty()) {
        std::cerr << "Missing required mqtt configuration (broker/topic)" << std::endl;
        return false;
    }

    return true;
}

std::string ConfigReader::getBroker() const {
    return broker_;
}

int ConfigReader::getPort() const {
    return port_;
}

std::string ConfigReader::getTopic() const {
    return topic_;
}

int ConfigReader::getQos() const {
    return qos_;
}

std::string ConfigReader::getClientId() const {
    return client_id_;
}


std::string ConfigReader::getMulticastAddr() const {
    return multicast_addr_;
}

int ConfigReader::getMulticastPort() const {
    return multicast_port_;
}

std::string ConfigReader::getInterface() const {
    return interface_;
}
