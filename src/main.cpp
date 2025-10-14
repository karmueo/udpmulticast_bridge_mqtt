#include <iostream>
#include <thread>
#include <chrono>
#include "mqtt_client.h"
#include "config_reader.h"

int main(int argc, char* argv[]) {
    // 默认配置文件路径
    std::string config_file = "config.json";
    
    // 如果提供了命令行参数，使用指定的配置文件
    if (argc > 1) {
        config_file = argv[1];
    }
    
    std::cout << "Loading configuration from: " << config_file << std::endl;
    
    // 读取配置
    ConfigReader config(config_file);
    if (!config.load()) {
        std::cerr << "Failed to load configuration" << std::endl;
        return 1;
    }
    
    std::cout << "Configuration loaded successfully" << std::endl;
    std::cout << "Broker: " << config.getBroker() << ":" << config.getPort() << std::endl;
    std::cout << "Topic: " << config.getTopic() << std::endl;
    std::cout << "QoS: " << config.getQos() << std::endl;
    std::cout << "Message: " << config.getMessage() << std::endl;
    
    // 创建MQTT客户端
    MqttClient client(config.getClientId(), config.getBroker(), config.getPort());
    
    // 连接到broker
    std::cout << "\nConnecting to MQTT broker..." << std::endl;
    if (!client.connect()) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return 1;
    }
    
    // 等待连接稳定
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 发布消息
    std::cout << "\nPublishing message..." << std::endl;
    if (!client.publish(config.getTopic(), config.getMessage(), config.getQos())) {
        std::cerr << "Failed to publish message" << std::endl;
        client.disconnect();
        return 1;
    }
    
    // 等待消息发送完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 断开连接
    std::cout << "\nDisconnecting..." << std::endl;
    client.disconnect();
    
    std::cout << "Done!" << std::endl;
    return 0;
}
