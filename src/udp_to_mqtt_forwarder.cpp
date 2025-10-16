#include "udp_to_mqtt_forwarder.h"
#include <iostream>
#include <chrono>

UdpToMqttForwarder::UdpToMqttForwarder(const std::string& mqtt_client_id,
                                       const std::string& mqtt_broker,
                                       int mqtt_port,
                                       const std::string& mqtt_topic,
                                       int mqtt_qos,
                                       const std::string& multicast_addr,
                                       int multicast_port)
    : mqtt_topic_(mqtt_topic),
      mqtt_qos_(mqtt_qos),
      running_(false),
      forwarded_count_(0),
      failed_count_(0) {
    
    // 创建MQTT客户端
    mqtt_client_ = std::make_unique<MqttClient>(mqtt_client_id, mqtt_broker, mqtt_port);
    
    // 创建UDP接收器
    udp_receiver_ = std::make_unique<UdpReceiver>(multicast_addr, multicast_port);
}

UdpToMqttForwarder::~UdpToMqttForwarder() {
    stop();
}

bool UdpToMqttForwarder::start() {
    if (running_) {
        std::cerr << "Forwarder is already running" << std::endl;
        return false;
    }

    std::cout << "Starting UDP to MQTT forwarder..." << std::endl;

    // 连接到MQTT broker
    std::cout << "Connecting to MQTT broker..." << std::endl;
    if (!mqtt_client_->connect()) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return false;
    }

    std::cout << "Connected to MQTT broker successfully" << std::endl;

    // 等待连接稳定
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 启动UDP接收器，设置回调函数
    std::cout << "Starting UDP receiver..." << std::endl;
    auto callback = [this](const std::string& message) {
        this->onUdpMessageReceived(message);
    };

    if (!udp_receiver_->start(callback)) {
        std::cerr << "Failed to start UDP receiver" << std::endl;
        mqtt_client_->disconnect();
        return false;
    }

    running_ = true;
    std::cout << "UDP to MQTT forwarder started successfully" << std::endl;
    std::cout << "Forwarding UDP messages to MQTT topic: " << mqtt_topic_ << std::endl;

    return true;
}

void UdpToMqttForwarder::stop() {
    if (!running_) {
        return;
    }

    std::cout << "Stopping UDP to MQTT forwarder..." << std::endl;

    // 停止UDP接收器
    udp_receiver_->stop();

    // 断开MQTT连接
    mqtt_client_->disconnect();

    running_ = false;

    std::cout << "UDP to MQTT forwarder stopped" << std::endl;
    std::cout << "Statistics: Forwarded: " << forwarded_count_ 
              << ", Failed: " << failed_count_ << std::endl;
}

bool UdpToMqttForwarder::isRunning() const {
    return running_;
}

uint64_t UdpToMqttForwarder::getForwardedMessageCount() const {
    return forwarded_count_;
}

uint64_t UdpToMqttForwarder::getFailedMessageCount() const {
    return failed_count_;
}

void UdpToMqttForwarder::resetStatistics() {
    forwarded_count_ = 0;
    failed_count_ = 0;
    std::cout << "Statistics reset" << std::endl;
}

void UdpToMqttForwarder::onUdpMessageReceived(const std::string& message) {
    if (!running_) {
        return;
    }

    std::cout << "\n[Forwarder] Received UDP message, forwarding to MQTT..." << std::endl;

    // 将消息发布到MQTT
    if (mqtt_client_->publish(mqtt_topic_, message, mqtt_qos_)) {
        forwarded_count_++;
        std::cout << "[Forwarder] Message forwarded successfully (Total: " 
                  << forwarded_count_ << ")" << std::endl;
    } else {
        failed_count_++;
        std::cerr << "[Forwarder] Failed to forward message (Failed: " 
                  << failed_count_ << ")" << std::endl;
    }
}
