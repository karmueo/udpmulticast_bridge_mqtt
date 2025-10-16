#include "mqtt_client.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>

MqttClient::MqttClient(const std::string& client_id, const std::string& broker, int port)
    : broker_(broker), port_(port), connected_(false) {
    
    // 初始化mosquitto库
    mosquitto_lib_init();
    
    // 创建mosquitto客户端实例
    mosq_ = mosquitto_new(client_id.c_str(), true, this);
    
    if (!mosq_) {
        std::cerr << "Failed to create mosquitto client" << std::endl;
        return;
    }
    
    // 设置回调函数
    mosquitto_connect_callback_set(mosq_, on_connect_callback);
    mosquitto_publish_callback_set(mosq_, on_publish_callback);
    mosquitto_disconnect_callback_set(mosq_, on_disconnect_callback);
}

MqttClient::~MqttClient() {
    if (mosq_) {
        mosquitto_destroy(mosq_);
    }
    mosquitto_lib_cleanup();
}

bool MqttClient::connect() {
    // 启动网络循环（先启动线程，再异步连接）
    int rc = mosquitto_loop_start(mosq_);
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to start loop: " << mosquitto_strerror(rc) << std::endl;
        return false;
    }
    
    // 使用异步连接，不会阻塞
    rc = mosquitto_connect_async(mosq_, broker_.c_str(), port_, 60);
    
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect async: " << mosquitto_strerror(rc) << std::endl;
        mosquitto_loop_stop(mosq_, true);
        return false;
    }
    
    // 等待连接建立
    int retry = 0;
    while (!connected_ && retry < 5) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        retry++;
    }
    
    return connected_;
}

bool MqttClient::publish(const std::string& topic, const std::string& message, int qos) {
    if (!connected_) {
        std::cerr << "Not connected to broker" << std::endl;
        return false;
    }
    
    int mid;
    int rc = mosquitto_publish(mosq_, &mid, topic.c_str(), 
                               message.length(), message.c_str(), qos, false);
    
    if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish: " << mosquitto_strerror(rc) << std::endl;
        return false;
    }
    
    std::cout << "Message published successfully (mid: " << mid << ")" << std::endl;
    return true;
}

void MqttClient::disconnect() {
    if (mosq_) {
        mosquitto_loop_stop(mosq_, true);
        mosquitto_disconnect(mosq_);
    }
    connected_ = false;
}

void MqttClient::on_connect_callback(struct mosquitto* mosq, void* obj, int result) {
    MqttClient* client = static_cast<MqttClient*>(obj);
    
    if (result == 0) {
        std::cout << "Connected to broker successfully" << std::endl;
        client->connected_ = true;
    } else {
        std::cerr << "Connection failed with code: " << result << std::endl;
        client->connected_ = false;
    }
}

void MqttClient::on_publish_callback(struct mosquitto* mosq, void* obj, int mid) {
    std::cout << "Message with mid " << mid << " has been published" << std::endl;
}

void MqttClient::on_disconnect_callback(struct mosquitto* mosq, void* obj, int rc) {
    MqttClient* client = static_cast<MqttClient*>(obj);
    client->connected_ = false;
    
    if (rc == 0) {
        std::cout << "Disconnected successfully" << std::endl;
    } else {
        std::cerr << "Unexpected disconnect: " << mosquitto_strerror(rc) << std::endl;
    }
}
