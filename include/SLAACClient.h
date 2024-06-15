// 文件: SLAACClient.h
#ifndef SLAACCLIENT_H
#define SLAACCLIENT_H

#include "NetworkInterface.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>


class SLAACClient {
public:
    SLAACClient(NetworkInterface& netif)
        : net_interface(netif) {}

    void send_rs(); // 发送路由请求
    void handle_ra(const std::vector<uint8_t>& packet); // 处理路由通告

private:
    NetworkInterface& net_interface;

    std::vector<uint8_t> create_link_local_address();
    void configure_global_address(const std::vector<uint8_t>& prefix, uint8_t prefix_length);
    void send_icmpv6_packet(const std::vector<uint8_t>& packet, const std::string& dest_ip);
};

//发送IPv6的路由请求
void SLAACClient::send_rs() {
    std::vector<uint8_t> rs_packet(24, 0); // ICMPv6 Router Solicitation packet
    rs_packet[0] = 133; // ICMPv6 Type: Router Solicitation
    rs_packet[1] = 0;   // ICMPv6 Code
    rs_packet[2] = 0;   // Checksum (to be calculated by the kernel)
    rs_packet[3] = 0;

    std::cout << "Sending Router Solicitation" << std::endl;
    send_icmpv6_packet(rs_packet, "ff02::2");
}

//处理接收到的IPv6路由通告
void SLAACClient::handle_ra(const std::vector<uint8_t>& packet) {
    std::cout << "Received Router Advertisement" << std::endl;

    // 假设我们从路由通告中解析出了前缀信息
    std::vector<uint8_t> prefix = { 0x20, 0x01, 0x0d, 0xb8, 0xac, 0x10, 0xfe, 0x01 };
    uint8_t prefix_length = 64;
    configure_global_address(prefix, prefix_length);
}

//创建IPv6的链路本地地址
std::vector<uint8_t> SLAACClient::create_link_local_address() {
    std::vector<uint8_t> link_local_address(16, 0);
    link_local_address[0] = 0xFE;
    link_local_address[1] = 0x80;
    std::memcpy(link_local_address.data() + 8, net_interface.get_mac_address().data(), 6);
    link_local_address[8] ^= 0x02; // 修改第七位
    return link_local_address;
}

//配置IPv6的全局地址
void SLAACClient::configure_global_address(const std::vector<uint8_t>& prefix, uint8_t prefix_length) {
    std::vector<uint8_t> global_address(16, 0);
    std::memcpy(global_address.data(), prefix.data(), prefix_length / 8);
    std::memcpy(global_address.data() + prefix_length / 8, net_interface.get_mac_address().data(), 6);
    global_address[prefix_length / 8] ^= 0x02; // 修改第七位

    std::string global_address_str;
    char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, global_address.data(), str, INET6_ADDRSTRLEN);
    global_address_str = std::string(str);

    std::cout << "Configured global address: " << global_address_str << std::endl;
}

//发送IPv6的ICMPv6消息
void SLAACClient::send_icmpv6_packet(const std::vector<uint8_t>& packet, const std::string& dest_ip) {
    SOCKET sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    struct sockaddr_in6 dest_addr;
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, dest_ip.c_str(), &dest_addr.sin6_addr);

    if (sendto(sockfd, (const char*)packet.data(), packet.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        std::cerr << "Failed to send packet." << std::endl;
    }

    closesocket(sockfd);
}

#endif // SLAACCLIENT_H
