// 文件: Socket.h
#ifndef SOCKET_H
#define SOCKET_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

enum SocketType {
    SOCKET_TYPE_UDP,
    SOCKET_TYPE_TCP
};

class Socket {
public:
    Socket(SocketType type);
    ~Socket();

    bool bind(uint16_t port);
    bool sendto(const std::vector<uint8_t>& data, const std::string& dest_ip, uint16_t dest_port);
    bool recvfrom(std::vector<uint8_t>& data, std::string& src_ip, uint16_t& src_port);

private:
    SocketType type;
    uint16_t bound_port;
    std::vector<uint8_t> buffer;
};

Socket::Socket(SocketType type) : type(type), bound_port(0) {}

Socket::~Socket() {}

bool Socket::bind(uint16_t port) {
    bound_port = port;
    return true;
}

bool Socket::sendto(const std::vector<uint8_t>& data, const std::string& dest_ip, uint16_t dest_port) {
    // 模拟发送数据（这里只是输出到控制台）
    std::cout << "Sending data to " << dest_ip << ":" << dest_port << std::endl;
    buffer = data; // 将数据存入缓冲区，以供接收模拟使用
    return true;
}

bool Socket::recvfrom(std::vector<uint8_t>& data, std::string& src_ip, uint16_t& src_port) {
    // 模拟接收数据（从缓冲区读取数据）
    if (buffer.empty()) {
        return false;
    }

    data = buffer;
    src_ip = "127.0.0.1"; // 模拟源IP
    src_port = bound_port; // 模拟源端口
    buffer.clear();
    return true;
}

#endif // SOCKET_H
