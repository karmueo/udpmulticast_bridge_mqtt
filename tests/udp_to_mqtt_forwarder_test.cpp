#include "udp_to_mqtt_forwarder.h"
#include <arpa/inet.h>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <condition_variable>
#include <mosquitto.h>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <vector>

/**
 * UdpToMqttForwarder的单元测试
 * 使用Catch2测试框架
 */

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * 等待指定的毫秒数
 */
void waitMs(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

/**
 * 发送UDP组播消息，用于触发转发流程
 */
bool sendUdpMulticastMessage(const std::string &message,
                             const std::string &address, int port)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        return false;
    }

    int loop = 1;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    int ttl = 1;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(address.c_str());
    addr.sin_port = htons(port);

    ssize_t sent = sendto(sock, message.data(), message.size(), 0,
                          reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    close(sock);
    return sent == static_cast<ssize_t>(message.size());
}

// ============================================================================
// 测试用例
// ============================================================================

/**
 * 测试1: 构造函数正确初始化对象
 */
// TEST_CASE("UdpToMqttForwarderConstructorInitialization", "[constructor]")
// {
//     UdpToMqttForwarder forwarder("test_client", "localhost", 1883, "test/topic",
//                                  1, "224.0.0.1", 5555);

//     REQUIRE_FALSE(forwarder.isRunning());
//     REQUIRE(forwarder.getForwardedMessageCount() == 0);
//     REQUIRE(forwarder.getFailedMessageCount() == 0);
// }

// /**
//  * 测试2: 不同的构造参数
//  */
// TEST_CASE("UdpToMqttForwarderConstructorWithDifferentParameters",
//           "[constructor]")
// {
//     UdpToMqttForwarder forwarder1("client1", "localhost", 1883, "topic1", 0,
//                                   "224.0.0.1", 5555);
//     UdpToMqttForwarder forwarder2("client2", "broker.example.com", 8883,
//                                   "topic2", 2, "239.255.255.250", 8888);

//     REQUIRE_FALSE(forwarder1.isRunning());
//     REQUIRE_FALSE(forwarder2.isRunning());
// }

// /**
//  * 测试3: 启动和停止转发器（需要本地MQTT broker）
//  */
// // TEST_CASE("UdpToMqttForwarderStartAndStop", "[lifecycle]")
// // {
// //     UdpToMqttForwarder forwarder("test_forwarder", "localhost", 1883,
// //                                  "test/forward", 1, "224.0.0.1", 5556);

// //     // 尝试启动（假设本地有mosquitto运行）
// //     bool started = forwarder.start();
// //     if (started)
// //     {
// //         REQUIRE(forwarder.isRunning());

// //         waitMs(100);

// //         forwarder.stop();
// //         REQUIRE_FALSE(forwarder.isRunning());
// //     }
// //     else
// //     {
// //         // 如果启动失败，可能是因为没有MQTT broker
// //         REQUIRE_FALSE(forwarder.isRunning());
// //     }
// // }

// /**
//  * 测试4: 不能启动多次
//  */
// TEST_CASE("UdpToMqttForwarderCannotStartMultipleTimes", "[lifecycle]")
// {
//     UdpToMqttForwarder forwarder("test_forwarder", "localhost", 1883,
//                                  "test/forward", 1, "224.0.0.1", 5557);

//     bool first_start = forwarder.start();
//     if (first_start)
//     {
//         REQUIRE(forwarder.isRunning());

//         bool second_start = forwarder.start();
//         REQUIRE_FALSE(second_start);

//         forwarder.stop();
//     }
// }

// /**
//  * 测试5: 安全地停止未运行的转发器
//  */
// TEST_CASE("UdpToMqttForwarderStopWhenNotRunning", "[lifecycle]")
// {
//     UdpToMqttForwarder forwarder("test_forwarder", "localhost", 1883,
//                                  "test/forward", 1, "224.0.0.1", 5558);

//     // 不应该崩溃
//     forwarder.stop();
//     REQUIRE_FALSE(forwarder.isRunning());
// }

// /**
//  * 测试6: 统计计数器
//  */
// TEST_CASE("UdpToMqttForwarderStatistics", "[statistics]")
// {
//     UdpToMqttForwarder forwarder("test_forwarder", "localhost", 1883,
//                                  "test/forward", 1, "224.0.0.1", 5559);

//     REQUIRE(forwarder.getForwardedMessageCount() == 0);
//     REQUIRE(forwarder.getFailedMessageCount() == 0);

//     // 重置统计
//     forwarder.resetStatistics();
//     REQUIRE(forwarder.getForwardedMessageCount() == 0);
//     REQUIRE(forwarder.getFailedMessageCount() == 0);
// }

// /**
//  * 测试7: 析构函数清理
//  */
// TEST_CASE("UdpToMqttForwarderDestructorCleanup", "[destructor]")
// {
//     {
//         UdpToMqttForwarder forwarder("test_forwarder", "localhost", 1883,
//                                      "test/forward", 1, "224.0.0.1", 5560);
//         // 对象在作用域结束时销毁
//     }
//     REQUIRE(true); // 如果没有崩溃，测试通过
// }

// /**
//  * 测试8: 多次启动和停止循环
//  */
// // TEST_CASE("UdpToMqttForwarderMultipleStartStopCycles", "[lifecycle]")
// // {
// //     for (int i = 0; i < 3; ++i)
// //     {
// //         UdpToMqttForwarder forwarder("test_forwarder", "localhost", 1883,
// //                                      "test/forward", 1, "224.0.0.1", 5561 + i);

// //         bool started = forwarder.start();
// //         if (started)
// //         {
// //             REQUIRE(forwarder.isRunning());
// //             waitMs(50);
// //             forwarder.stop();
// //             REQUIRE_FALSE(forwarder.isRunning());
// //         }
// //         waitMs(50);
// //     }
// // }


// /**
//  * 测试10: 不同的QoS级别
//  */
// TEST_CASE("UdpToMqttForwarderDifferentQoS", "[qos]")
// {
//     for (int qos = 0; qos <= 2; ++qos)
//     {
//         UdpToMqttForwarder forwarder("test_client", "localhost", 1883,
//                                      "test/topic", qos, "224.0.0.1",
//                                      5570 + qos);
//         REQUIRE_FALSE(forwarder.isRunning());
//     }
// }

// /**
//  * 测试11: 不同的端口和地址
//  */
// TEST_CASE("UdpToMqttForwarderDifferentPortsAndAddresses", "[parameters]")
// {
//     std::vector<std::tuple<std::string, int>> configs = {
//         {"224.0.0.1", 6000}, {"239.255.255.250", 8888}, {"224.0.0.255", 1234}};

//     for (const auto &[addr, port] : configs)
//     {
//         UdpToMqttForwarder forwarder("test_client", "localhost", 1883,
//                                      "test/topic", 1, addr, port);
//         REQUIRE_FALSE(forwarder.isRunning());
//     }
// }

/**
 * 测试12: 将UDP组播JSON报文转发至本地mosquitto并验证内容
 */
TEST_CASE("UdpToMqttForwarderForwardsJsonToMosquitto", "[integration][json]")
{
    const std::string multicastAddress = "224.0.0.1";
    const int         multicastPort = 5650;
    const std::string topic = "test/forward/json";
    const std::string jsonPayload =
        R"({
                "command": "start-recording",
                "start": "2020-09-25T9:00:00.000+08:00",
                "end": "2020-09-25T10:00:00.000+08:00",
                "sensor": {
                    "id": "0"
                }
            })";

    UdpToMqttForwarder forwarder("forwarder_json_test_client", "localhost",
                                 1883, topic, 1, multicastAddress,
                                 multicastPort);

    bool started = forwarder.start();
    if (!started)
    {
        WARN("Forwarder failed to start. Ensure local mosquitto broker is "
             "running.");
        return;
    }

    struct SubscriberData
    {
        std::mutex              mutex;
        std::condition_variable cv;
        bool                    received = false;
        std::string             payload;
    } subscriberData;

    mosquitto *subscriber =
        mosquitto_new("forwarder_json_test_subscriber", true, &subscriberData);
    REQUIRE(subscriber != nullptr);

    mosquitto_message_callback_set(
        subscriber,
        [](mosquitto *, void *userdata, const mosquitto_message *message)
        {
            auto *data = static_cast<SubscriberData *>(userdata);
            {
                std::lock_guard<std::mutex> lock(data->mutex);
                data->payload.assign(
                    static_cast<const char *>(message->payload),
                    static_cast<size_t>(message->payloadlen));
                data->received = true;
            }
            data->cv.notify_one();
        });

    int rc = mosquitto_connect(subscriber, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        WARN("Subscriber failed to connect: " << mosquitto_strerror(rc));
        mosquitto_destroy(subscriber);
        forwarder.stop();
        return;
    }

    rc = mosquitto_loop_start(subscriber);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        WARN("Unable to start mosquitto network loop: "
             << mosquitto_strerror(rc));
        mosquitto_disconnect(subscriber);
        mosquitto_destroy(subscriber);
        forwarder.stop();
        return;
    }

    rc = mosquitto_subscribe(subscriber, nullptr, topic.c_str(), 1);
    if (rc != MOSQ_ERR_SUCCESS)
    {
        WARN("Subscribe failed: " << mosquitto_strerror(rc));
        mosquitto_loop_stop(subscriber, true);
        mosquitto_disconnect(subscriber);
        mosquitto_destroy(subscriber);
        forwarder.stop();
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    bool sent =
        sendUdpMulticastMessage(jsonPayload, multicastAddress, multicastPort);
    REQUIRE(sent);

    bool messageReceived = false;
    {
        std::unique_lock<std::mutex> lock(subscriberData.mutex);
        messageReceived = subscriberData.cv.wait_for(
            lock, std::chrono::seconds(5),
            [&subscriberData] { return subscriberData.received; });
    }

    mosquitto_loop_stop(subscriber, true);
    mosquitto_disconnect(subscriber);
    mosquitto_destroy(subscriber);

    forwarder.stop();

    REQUIRE(messageReceived);
    CHECK(subscriberData.payload == jsonPayload);
    CHECK(forwarder.getForwardedMessageCount() >= 1);
    CHECK(forwarder.getFailedMessageCount() == 0);
}

// ============================================================================
// 主程序由Catch2提供
// ============================================================================