#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>
#include <map>

class ConfigReader {
public:
    ConfigReader(const std::string& config_file);
    
    bool load();
    std::string getBroker() const;
    int getPort() const;
    std::string getTopic() const;
    int getQos() const;
    std::string getClientId() const;
    std::string getMessage() const;

private:
    std::string config_file_;
    std::string broker_;
    int port_;
    std::string topic_;
    int qos_;
    std::string client_id_;
    std::string message_;

    std::string trim(const std::string& str);
    std::string extractJsonValue(const std::string& json, const std::string& key);
    std::string extractNestedValue(const std::string& json, const std::string& section, const std::string& key);
};

#endif // CONFIG_READER_H
