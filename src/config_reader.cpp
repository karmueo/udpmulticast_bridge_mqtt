#include "config_reader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

ConfigReader::ConfigReader(const std::string& config_file)
    : config_file_(config_file), port_(1883), qos_(1) {
}

bool ConfigReader::load() {
    std::ifstream file(config_file_);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << config_file_ << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();
    file.close();

    // 简单的JSON解析（手动实现，避免依赖第三方库）
    broker_ = extractNestedValue(json_content, "mqtt", "broker");
    topic_ = extractNestedValue(json_content, "mqtt", "topic");
    client_id_ = extractNestedValue(json_content, "mqtt", "client_id");
    
    std::string port_str = extractNestedValue(json_content, "mqtt", "port");
    if (!port_str.empty()) {
        port_ = std::stoi(port_str);
    }
    
    std::string qos_str = extractNestedValue(json_content, "mqtt", "qos");
    if (!qos_str.empty()) {
        qos_ = std::stoi(qos_str);
    }

    // 提取整个message对象
    size_t msg_start = json_content.find("\"message\"");
    if (msg_start != std::string::npos) {
        size_t obj_start = json_content.find("{", msg_start);
        if (obj_start != std::string::npos) {
            int brace_count = 1;
            size_t obj_end = obj_start + 1;
            
            while (obj_end < json_content.length() && brace_count > 0) {
                if (json_content[obj_end] == '{') brace_count++;
                else if (json_content[obj_end] == '}') brace_count--;
                obj_end++;
            }
            
            message_ = json_content.substr(obj_start, obj_end - obj_start);
        }
    }

    if (broker_.empty() || topic_.empty() || message_.empty()) {
        std::cerr << "Missing required configuration" << std::endl;
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

std::string ConfigReader::getMessage() const {
    return message_;
}

std::string ConfigReader::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\"");
    if (first == std::string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\n\r\",");
    return str.substr(first, (last - first + 1));
}

std::string ConfigReader::extractJsonValue(const std::string& json, const std::string& key) {
    std::string search_key = "\"" + key + "\"";
    size_t key_pos = json.find(search_key);
    
    if (key_pos == std::string::npos) {
        return "";
    }
    
    size_t colon_pos = json.find(":", key_pos);
    if (colon_pos == std::string::npos) {
        return "";
    }
    
    size_t value_start = colon_pos + 1;
    size_t value_end = json.find_first_of(",}\n", value_start);
    
    if (value_end == std::string::npos) {
        value_end = json.length();
    }
    
    std::string value = json.substr(value_start, value_end - value_start);
    return trim(value);
}

std::string ConfigReader::extractNestedValue(const std::string& json, 
                                             const std::string& section, 
                                             const std::string& key) {
    std::string search_section = "\"" + section + "\"";
    size_t section_pos = json.find(search_section);
    
    if (section_pos == std::string::npos) {
        return "";
    }
    
    size_t section_start = json.find("{", section_pos);
    if (section_start == std::string::npos) {
        return "";
    }
    
    int brace_count = 1;
    size_t section_end = section_start + 1;
    
    while (section_end < json.length() && brace_count > 0) {
        if (json[section_end] == '{') brace_count++;
        else if (json[section_end] == '}') brace_count--;
        section_end++;
    }
    
    std::string section_content = json.substr(section_start, section_end - section_start);
    return extractJsonValue(section_content, key);
}
