#include "udp_receiver.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

UdpReceiver::UdpReceiver(const std::string& multicast_addr, int port)
    : multicast_addr_(multicast_addr), port_(port), socket_fd_(-1), running_(false) {
}

UdpReceiver::~UdpReceiver() {
    stop();
}

bool UdpReceiver::start(ReceiveCallback callback) {
    if (running_) {
        std::cerr << "UDP receiver is already running" << std::endl;
        return false;
    }

    // 创建UDP套接字
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ < 0) {
        std::cerr << "Failed to create UDP socket" << std::endl;
        return false;
    }

    // 设置套接字为可重用
    int reuse = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    // 绑定到指定端口
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    if (bind(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind UDP socket to port " << port_ << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    // 加入组播组
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_addr_.c_str());
    if (mreq.imr_multiaddr.s_addr == INADDR_NONE) {
        std::cerr << "Invalid multicast address: " << multicast_addr_ << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        std::cerr << "Failed to join multicast group" << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    running_ = true;
    receive_thread_ = std::thread(&UdpReceiver::receiveLoop, this, callback);
    
    std::cout << "UDP receiver started on " << multicast_addr_ << ":" << port_ << std::endl;
    return true;
}

void UdpReceiver::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }

    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }

    std::cout << "UDP receiver stopped" << std::endl;
}

bool UdpReceiver::isRunning() const {
    return running_;
}

void UdpReceiver::receiveLoop(ReceiveCallback callback) {
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in src_addr;
    socklen_t src_addr_len = sizeof(src_addr);

    std::cout << "Listening for UDP multicast messages..." << std::endl;

    while (running_) {
        // 设置接收超时，避免阻塞
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recvfrom(socket_fd_, buffer, BUFFER_SIZE - 1, 0,
                                      (struct sockaddr*)&src_addr, &src_addr_len);

        if (bytes_received < 0) {
            // 超时或错误，继续循环
            continue;
        }

        if (bytes_received > 0) {
            std::string message(buffer, bytes_received);
            
            std::cout << "\n=== Received UDP Message ===" << std::endl;
            std::cout << "From: " << inet_ntoa(src_addr.sin_addr) << ":" 
                      << ntohs(src_addr.sin_port) << std::endl;
            std::cout << "Size: " << bytes_received << " bytes" << std::endl;
            std::cout << "Raw data: " << message << std::endl;

            // 尝试解析JSON
            if (isValidJson(message)) {
                std::cout << "\nParsed JSON:" << std::endl;
                prettyPrintJson(message);
            }

            // 如果提供了回调函数，调用它
            if (callback) {
                callback(message);
            }

            std::cout << "============================\n" << std::endl;
        }
    }
}

bool UdpReceiver::isValidJson(const std::string& str) {
    if (str.empty()) return false;
    
    // 移除前后空格
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return false;
    
    size_t last = str.find_last_not_of(" \t\n\r");
    
    // 检查是否以{或[开头，以}或]结尾
    char first_char = str[first];
    char last_char = str[last];
    
    return (first_char == '{' && last_char == '}') || 
           (first_char == '[' && last_char == ']');
}

void UdpReceiver::prettyPrintJson(const std::string& json_str, int indent) {
    bool in_string = false;
    bool escape = false;
    int current_indent = indent;

    for (size_t i = 0; i < json_str.length(); ++i) {
        char c = json_str[i];

        // 处理转义字符和字符串
        if (c == '"' && !escape) {
            in_string = !in_string;
        }
        escape = (c == '\\' && !escape && in_string);

        if (in_string) {
            std::cout << c;
            continue;
        }

        switch (c) {
            case '{':
            case '[':
                std::cout << c << '\n';
                current_indent += 2;
                for (int j = 0; j < current_indent; ++j) std::cout << ' ';
                break;
            case '}':
            case ']':
                std::cout << '\n';
                current_indent -= 2;
                for (int j = 0; j < current_indent; ++j) std::cout << ' ';
                std::cout << c;
                break;
            case ',':
                std::cout << c << '\n';
                for (int j = 0; j < current_indent; ++j) std::cout << ' ';
                break;
            case ':':
                std::cout << c << ' ';
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                // 跳过空白字符
                break;
            default:
                std::cout << c;
        }
    }
    std::cout << std::endl;
}

void UdpReceiver::parseAndPrintJson(const std::string& json_str) {
    if (isValidJson(json_str)) {
        std::cout << "Valid JSON detected:" << std::endl;
        prettyPrintJson(json_str);
    } else {
        std::cout << "Not a valid JSON format" << std::endl;
    }
}
