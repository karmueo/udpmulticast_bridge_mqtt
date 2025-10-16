#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>

class ConfigReader {
public:
    ConfigReader(const std::string& config_file);
    
    bool load();
    std::string getBroker() const;
    int getPort() const;
    std::string getTopic() const;
    int getQos() const;
    std::string getClientId() const;
    std::string getMulticastAddr() const;
    int getMulticastPort() const;

private:
    std::string config_file_;
    std::string broker_;
    int port_;
    std::string topic_;
    int qos_;
    std::string client_id_;

    // UDP multicast settings
    std::string multicast_addr_;
    int multicast_port_;
};

#endif // CONFIG_READER_H
