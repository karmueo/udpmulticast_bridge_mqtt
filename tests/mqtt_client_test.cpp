#include "mqtt_client.h"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

/**
 * MqttClient的单元测试
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

// ============================================================================
// 测试用例
// ============================================================================

/**
 * 测试1: 构造函数正确初始化对象
 */
TEST_CASE("MqttClientConstructorInitialization", "[constructor]")
{
    MqttClient client("test_client", "localhost", 1883);
    // 构造函数没有公开状态检查，但可以测试它不崩溃
    REQUIRE(true); // 如果构造函数成功，测试通过
}

/**
 * 测试3: 连接到不存在的broker（应该失败）
 */
TEST_CASE("MqttClientConnectToInvalidBroker", "[connect]")
{
    MqttClient client("test_client", "invalid.broker.address", 1883);

    bool connected = client.connect();
    REQUIRE_FALSE(connected);
}

/**
 * 测试4: 连接到本地broker（假设有mosquitto运行）
 */
TEST_CASE("MqttClientConnectToLocalhost", "[connect]")
{
    MqttClient client("test_client", "localhost", 1883);

    // 假设本地有mosquitto broker运行
    bool connected = client.connect();
    REQUIRE(connected);

    // 清理
    client.disconnect();
}

/**
 * 测试5: 发布消息到未连接的客户端（应该失败）
 */
TEST_CASE("MqttClientPublishWithoutConnection", "[publish]")
{
    MqttClient client("test_client", "localhost", 1883);

    // 不调用connect，直接publish
    bool published = client.publish("test/topic", "test message", 0);
    REQUIRE_FALSE(published);
}

/**
 * 测试16: 发布消息到已连接的客户端（应该成功）
 */
TEST_CASE("MqttClientPublishWithConnection", "[publish]")
{
    MqttClient client("test_client_publish", "localhost", 1883);

    bool connected = client.connect();
    REQUIRE(connected);

    waitMs(100); // 等待连接稳定

    bool published = client.publish("test/topic", "test message", 0);
    REQUIRE(published);

    client.disconnect();
}

/**
 * 测试17: 发布消息不同QoS到已连接的客户端
 */
TEST_CASE("MqttClientPublishDifferentQoSConnected", "[publish]")
{
    MqttClient client("test_client_qos", "localhost", 1883);

    bool connected = client.connect();
    REQUIRE(connected);

    waitMs(100); // 等待连接稳定

    // 测试QoS 0, 1, 2
    for (int qos = 0; qos <= 2; ++qos)
    {
        bool published = client.publish("test/topic", "test message qos " + std::to_string(qos), qos);
        REQUIRE(published);
        waitMs(50); // 短暂等待
    }

    client.disconnect();
}

/**
 * 测试6: 发布消息到无效broker（应该失败）
 */
TEST_CASE("MqttClientPublishToInvalidBroker", "[publish]")
{
    MqttClient client("test_client", "invalid.broker.address", 1883);

    // 尝试连接（会失败）
    client.connect();

    // 发布应该失败
    bool published = client.publish("test/topic", "test message", 0);
    REQUIRE_FALSE(published);
}

/**
 * 测试7: 断开未连接的客户端（应该安全）
 */
TEST_CASE("MqttClientDisconnectWithoutConnection", "[disconnect]")
{
    MqttClient client("test_client", "localhost", 1883);

    // 不调用connect，直接disconnect
    client.disconnect();
    REQUIRE(true); // 不应该崩溃
}

/**
 * 测试8: 多次断开连接（应该安全）
 */
TEST_CASE("MqttClientMultipleDisconnects", "[disconnect]")
{
    MqttClient client("test_client", "localhost", 1883);

    client.disconnect();
    client.disconnect();
    client.disconnect();
    REQUIRE(true); // 不应该崩溃
}

/**
 * 测试9: 析构函数清理（测试不崩溃）
 */
TEST_CASE("MqttClientDestructorCleanup", "[destructor]")
{
    {
        MqttClient client("test_client", "localhost", 1883);
        // 客户端在作用域结束时销毁
    }
    REQUIRE(true); // 如果没有崩溃，测试通过
}

/**
 * 测试10: 发布不同QoS级别
 */
TEST_CASE("MqttClientPublishDifferentQoS", "[publish]")
{
    MqttClient client("test_client", "localhost", 1883);

    // 测试QoS 0, 1, 2
    for (int qos = 0; qos <= 2; ++qos)
    {
        bool published = client.publish("test/topic", "test message", qos);
        REQUIRE_FALSE(published); // 应该失败，因为未连接
    }
}

/**
 * 测试11: 发布空消息
 */
TEST_CASE("MqttClientPublishEmptyMessage", "[publish]")
{
    MqttClient client("test_client", "localhost", 1883);

    bool published = client.publish("test/topic", "", 0);
    REQUIRE_FALSE(published);
}

/**
 * 测试12: 发布到空主题
 */
TEST_CASE("MqttClientPublishEmptyTopic", "[publish]")
{
    MqttClient client("test_client", "localhost", 1883);

    bool published = client.publish("", "test message", 0);
    REQUIRE_FALSE(published);
}

/**
 * 测试13: 连接超时测试
 * 注意: 该测试可能需要几秒钟来等待连接超时。
 * 由于mosquitto库的特性，连接到不可达地址可能导致阻塞。
 */
TEST_CASE("MqttClientConnectionTimeout", "[connect]")
{
    MqttClient client("test_client", "10.255.255.1", 1883); // 不可达的私有地址

    auto start_time = std::chrono::high_resolution_clock::now();
    bool connected = client.connect();
    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    REQUIRE_FALSE(connected);
    // 连接应该在合理时间内失败（例如少于5秒）
    REQUIRE(duration.count() < 5000);
}

/**
 * 测试14: 客户端ID为空字符串
 */
TEST_CASE("MqttClientEmptyClientId", "[constructor]")
{
    MqttClient client("", "localhost", 1883);
    REQUIRE(true); // 构造函数应该处理空ID
}

/**
 * 测试15: 端口号边界测试
 */
TEST_CASE("MqttClientPortBoundaries", "[constructor]")
{
    // 测试有效端口
    MqttClient client1("client", "localhost", 1);
    MqttClient client2("client", "localhost", 65535);
    REQUIRE(true);
}

// ============================================================================
// 主程序由Catch2提供
// ============================================================================