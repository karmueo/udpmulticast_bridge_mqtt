#include <iostream>
#include <thread>
#include <chrono>
#include "config_reader.h"
#include "udp_to_mqtt_forwarder.h"
#include <csignal>
#include <atomic>

int main(int argc, char* argv[]) {
    // 默认配置文件
    std::string config_file = "config.json";
    if (argc > 1) {
        config_file = argv[1];
    }

    std::cout << "Loading configuration from: " << config_file << std::endl;
    ConfigReader config(config_file);
    if (!config.load()) {
        std::cerr << "Failed to load configuration" << std::endl;
        return 1;
    }

    // 从配置中读取MQTT和UDP组播相关字段
    std::string broker = config.getBroker();
    int port = config.getPort();
    std::string topic = config.getTopic();
    int qos = config.getQos();
    std::string client_id = config.getClientId();

    // UDP multicast settings from config
    std::string multicast_addr = config.getMulticastAddr();
    int multicast_port = config.getMulticastPort();
    std::string interface = config.getInterface();

    std::cout << "MQTT broker: " << broker << ":" << port << std::endl;
    std::cout << "MQTT topic: " << topic << " qos=" << qos << std::endl;
    std::cout << "UDP multicast: " << multicast_addr << ":" << multicast_port << std::endl;
    if (!interface.empty()) {
        std::cout << "Network interface: " << interface << std::endl;
    }

    // 创建并启动转发器
    UdpToMqttForwarder forwarder(client_id, broker, port, topic, qos, multicast_addr, multicast_port, interface);
    if (!forwarder.start()) {
        std::cerr << "Failed to start UDP->MQTT forwarder" << std::endl;
        return 1;
    }

    // 运行直到用户中断（SIGINT/SIGTERM）
    static std::atomic<bool> keepRunning{true};

    auto signalHandler = [](int signum) {
        (void)signum;
        keepRunning.store(false);
    };

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "Forwarder running. Press Ctrl+C to stop..." << std::endl;
    // 主线程等待，直到接收到终止信号或内部转发器停止
    while (keepRunning.load() && forwarder.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    forwarder.stop();
    std::cout << "Exiting" << std::endl;
    return 0;
}
