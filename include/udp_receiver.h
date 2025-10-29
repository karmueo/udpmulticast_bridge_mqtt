#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include <string>
#include <thread>
#include <atomic>
#include <functional>

class UdpReceiver {
public:
    // 接收回调函数类型
    using ReceiveCallback = std::function<void(const std::string&)>;

    UdpReceiver(const std::string& multicast_addr, int port, const std::string& interface = "");
    ~UdpReceiver();

    // 启动接收线程
    bool start(ReceiveCallback callback = nullptr);

    // 停止接收
    void stop();

    // 检查是否正在运行
    bool isRunning() const;

private:
    std::string multicast_addr_;
    int port_;
    std::string interface_;
    int socket_fd_;
    std::atomic<bool> running_;
    std::thread receive_thread_;

    // 接收线程主函数
    void receiveLoop(ReceiveCallback callback);

    // 解析并打印JSON
    void parseAndPrintJson(const std::string& json_str);

    // 检查字符串是否是有效的JSON
    bool isValidJson(const std::string& str);

    // 美化打印JSON
    void prettyPrintJson(const std::string& json_str, int indent = 0);
};

#endif // UDP_RECEIVER_H
