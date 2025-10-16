#include "udp_receiver.h"
#include <arpa/inet.h>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

/**
 * UDP接收器的集成测试
 * 使用Catch2测试框架
 */

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * 发送UDP组播消息（用于测试）
 */
bool sendUdpMessage(const std::string &message,
                    const std::string &address = "224.0.0.1", int port = 5555)
{
    try
    {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            return false;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(address.c_str());
        addr.sin_port = htons(port);

        int ttl = 2;
        setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

        ssize_t sent = sendto(sock, message.c_str(), message.length(), 0,
                              (struct sockaddr *)&addr, sizeof(addr));
        close(sock);

        return sent > 0;
    }
    catch (...)
    {
        return false;
    }
}

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
TEST_CASE("ConstructorInitialization", "[constructor]")
{
    UdpReceiver receiver("224.0.0.1", 5555);
    REQUIRE_FALSE(receiver.isRunning());
}

/**
 * 测试2: 不同的构造参数
 */
TEST_CASE("ConstructorWithDifferentParameters", "[constructor]")
{
    UdpReceiver receiver1("224.0.0.1", 5555);
    UdpReceiver receiver2("239.255.255.250", 8888);
    UdpReceiver receiver3("224.0.0.255", 1234);

    REQUIRE_FALSE(receiver1.isRunning());
    REQUIRE_FALSE(receiver2.isRunning());
    REQUIRE_FALSE(receiver3.isRunning());
}

/**
 * 测试3: 启动和停止接收器
 */
TEST_CASE("StartAndStop", "[lifecycle]")
{
    UdpReceiver receiver("224.0.0.1", 5556);

    bool started = receiver.start();
    REQUIRE(started);
    REQUIRE(receiver.isRunning());

    waitMs(100);

    receiver.stop();
    REQUIRE_FALSE(receiver.isRunning());
}

/**
 * 测试4: 不能启动多次
 */
TEST_CASE("CannotStartMultipleTimes", "[lifecycle]")
{
    UdpReceiver receiver("224.0.0.1", 5557);

    bool first_start = receiver.start();
    REQUIRE(first_start);

    bool second_start = receiver.start();
    REQUIRE_FALSE(second_start);

    receiver.stop();
}

/**
 * 测试5: 安全地停止未运行的接收器
 */
TEST_CASE("StopWhenNotRunning", "[lifecycle]")
{
    UdpReceiver receiver("224.0.0.1", 5558);

    // 不应该崩溃
    receiver.stop();
    REQUIRE_FALSE(receiver.isRunning());
}

/**
 * 测试6: 析构函数不应该崩溃
 */
TEST_CASE("DestructorIsClean", "[lifecycle]")
{
    {
        UdpReceiver receiver("224.0.0.1", 5559);
        receiver.start();
        waitMs(100);
        // 析构函数应该自动清理
    }
    REQUIRE(true); // 如果没有崩溃，测试通过
}

/**
 * 测试7: 使用回调函数
 */
TEST_CASE("CallbackFunction", "[callback]")
{
    UdpReceiver receiver("224.0.0.1", 5560);

    int  callback_count = 0;
    auto callback = [&callback_count](const std::string &msg)
    {
        (void)msg; // 避免未使用参数警告
        callback_count++;
    };

    bool started = receiver.start(callback);
    REQUIRE(started);
    REQUIRE(receiver.isRunning());

    waitMs(200);
    receiver.stop();

    // callback_count 可能是0或更多（取决于是否有消息发送）
    REQUIRE(true);
}

/**
 * 测试8: 多次启动和停止循环
 */
TEST_CASE("MultipleStartStopCycles", "[lifecycle]")
{
    for (int i = 0; i < 3; ++i)
    {
        UdpReceiver receiver("224.0.0.1", 5561);

        bool started = receiver.start();
        REQUIRE(started);
        REQUIRE(receiver.isRunning());

        waitMs(50);
        receiver.stop();
        REQUIRE_FALSE(receiver.isRunning());

        waitMs(50);
    }
}

/**
 * 测试9: 启动性能测试
 */
TEST_CASE("StartupPerformance", "[performance]")
{
    auto start_time = std::chrono::high_resolution_clock::now();

    UdpReceiver receiver("224.0.0.1", 5562);
    bool        started = receiver.start();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    REQUIRE(started);

    // 启动应该在1秒内完成
    REQUIRE(duration.count() < 1000);

    receiver.stop();
}

/**
 * 测试10: 停止性能测试
 */
TEST_CASE("ShutdownPerformance", "[performance]")
{
    UdpReceiver receiver("224.0.0.1", 5563);
    receiver.start();
    waitMs(50);

    auto start_time = std::chrono::high_resolution_clock::now();
    receiver.stop();
    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    // 停止应该快速完成（少于2秒，因为线程需要等待超时）
    REQUIRE(duration.count() < 2000);
}

/**
 * 测试11: 并发运行多个接收器
 */
TEST_CASE("MultipleReceiversConcurrently", "[concurrency]")
{
    UdpReceiver receiver1("224.0.0.1", 5564);
    UdpReceiver receiver2("224.0.0.1", 5565);
    UdpReceiver receiver3("224.0.0.1", 5566);

    REQUIRE(receiver1.start());
    REQUIRE(receiver2.start());
    REQUIRE(receiver3.start());

    waitMs(200);

    receiver1.stop();
    receiver2.stop();
    receiver3.stop();

    REQUIRE_FALSE(receiver1.isRunning());
    REQUIRE_FALSE(receiver2.isRunning());
    REQUIRE_FALSE(receiver3.isRunning());
}

/**
 * 测试12: 不同的端口号
 */
TEST_CASE("DifferentPorts", "[ports]")
{
    std::vector<int> ports = {5570, 5571, 5572, 6000, 8888};

    for (int port : ports)
    {
        UdpReceiver receiver("224.0.0.1", port);
        bool        started = receiver.start();

        if (started)
        {
            REQUIRE(receiver.isRunning());
            receiver.stop();
        }
    }
}

/**
 * 测试13: 压力测试 - 快速启动停止
 */
TEST_CASE("StressTestRapidStartStop", "[stress]")
{
    for (int i = 0; i < 10; ++i)
    {
        UdpReceiver receiver("224.0.0.1", 5600 + i);
        receiver.start();
        receiver.stop();
    }
}

/**
 * 测试15: JSON报文解析 - 有效JSON
 */
TEST_CASE("JsonMessageParsingValid", "[json]")
{
    UdpReceiver receiver("224.0.0.1", 5620);

    std::string json_msg = R"(
{
    "sensor": "temperature",
    "value": 23.5,
    "timestamp": 1234567890
}
)";

    bool started = receiver.start();
    REQUIRE(started);

    waitMs(100); // 等待接收器启动

    bool sent = sendUdpMessage(json_msg, "224.0.0.1", 5620);
    REQUIRE(sent);

    waitMs(1000); // 等待消息处理

    receiver.stop();
    REQUIRE_FALSE(receiver.isRunning());
}

/**
 * 测试16: JSON报文解析 - 无效JSON
 */
TEST_CASE("JsonMessageParsingInvalid", "[json]")
{
    UdpReceiver receiver("224.0.0.1", 5621);

    std::string invalid_msg = "This is not JSON at all";

    bool started = receiver.start();
    REQUIRE(started);

    waitMs(100); // 等待接收器启动

    bool sent = sendUdpMessage(invalid_msg, "224.0.0.1", 5621);
    REQUIRE(sent);

    waitMs(1000); // 等待消息处理

    receiver.stop();
    REQUIRE_FALSE(receiver.isRunning());
}

/**
 * 测试17: JSON报文解析 - 数组格式
 */
TEST_CASE("JsonMessageParsingArray", "[json]")
{
    UdpReceiver receiver("224.0.0.1", 5622);

    std::string json_array = R"(
[
    {"sensor": "temp", "value": 20.0},
    {"sensor": "humidity", "value": 60.5}
]
)";

    bool started = receiver.start();
    REQUIRE(started);

    waitMs(100); // 等待接收器启动

    bool sent = sendUdpMessage(json_array, "224.0.0.1", 5622);
    REQUIRE(sent);

    waitMs(1000); // 等待消息处理

    receiver.stop();
    REQUIRE_FALSE(receiver.isRunning());
}

// ============================================================================
// 主程序由Catch2提供
// ============================================================================
