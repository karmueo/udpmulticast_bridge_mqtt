#ifndef UDP_TO_MQTT_FORWARDER_H
#define UDP_TO_MQTT_FORWARDER_H

#include <string>
#include <memory>
#include <atomic>
#include "mqtt_client.h"
#include "udp_receiver.h"

/**
 * @class UdpToMqttForwarder
 * @brief 将UDP组播消息转发到MQTT的转发器类
 * 
 * 此类集成了UdpReceiver和MqttClient，可以接收UDP组播消息并将其发布到MQTT broker
 */
class UdpToMqttForwarder {
public:
    /**
     * @brief 构造函数
     * @param mqtt_client_id MQTT客户端ID
     * @param mqtt_broker MQTT broker地址
     * @param mqtt_port MQTT broker端口
     * @param mqtt_topic MQTT发布主题
     * @param mqtt_qos MQTT消息QoS（默认1）
     * @param multicast_addr UDP组播地址
     * @param multicast_port UDP接收端口
     */
    UdpToMqttForwarder(const std::string& mqtt_client_id,
                       const std::string& mqtt_broker,
                       int mqtt_port,
                       const std::string& mqtt_topic,
                       int mqtt_qos,
                       const std::string& multicast_addr,
                       int multicast_port);
    
    ~UdpToMqttForwarder();

    /**
     * @brief 启动转发器
     * @return true 启动成功，false 启动失败
     */
    bool start();

    /**
     * @brief 停止转发器
     */
    void stop();

    /**
     * @brief 检查转发器是否运行中
     * @return true 运行中，false 未运行
     */
    bool isRunning() const;

    /**
     * @brief 获取已转发的消息数
     * @return 消息计数
     */
    uint64_t getForwardedMessageCount() const;

    /**
     * @brief 获取失败的消息数
     * @return 失败计数
     */
    uint64_t getFailedMessageCount() const;

    /**
     * @brief 重置统计计数
     */
    void resetStatistics();

private:
    std::unique_ptr<MqttClient> mqtt_client_;
    std::unique_ptr<UdpReceiver> udp_receiver_;
    
    std::string mqtt_topic_;
    int mqtt_qos_;
    
    std::atomic<bool> running_;
    std::atomic<uint64_t> forwarded_count_;
    std::atomic<uint64_t> failed_count_;

    /**
     * @brief UDP接收回调函数
     * 当收到UDP消息时调用此函数
     */
    void onUdpMessageReceived(const std::string& message);
};

#endif // UDP_TO_MQTT_FORWARDER_H
