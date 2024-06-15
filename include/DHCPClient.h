#ifndef DHCPCLIENT_H
#define DHCPCLIENT_H

#include <iostream>
#include "NetworkInterface.h"
#include <vector>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

//#include "Network.h"


class DHCPClient {
public:
    DHCPClient(NetworkInterface& netif);

    void send_dhcp_discover();
    void handle_dhcp_offer(const std::vector<uint8_t>& packet);
    void send_dhcp_request();
    void handle_dhcp_ack(const std::vector<uint8_t>& packet);

    struct DHCPMessage {
        uint8_t op;
        uint8_t htype;
        uint8_t hlen;
        uint8_t hops;
        uint32_t xid;
        uint16_t secs;
        uint16_t flags;
        uint32_t ciaddr;
        uint32_t yiaddr;
        uint32_t siaddr;
        uint32_t giaddr;
        uint8_t chaddr[16];
        uint8_t sname[64];
        uint8_t file[128];
        uint8_t options[312];

        DHCPMessage() {
            std::memset(this, 0, sizeof(DHCPMessage));
        }
    };

private:
    NetworkInterface& net_interface;
    uint32_t transaction_id;

    std::vector<uint8_t> serialize_dhcp_message(const DHCPMessage& message);
    DHCPMessage deserialize_dhcp_message(const std::vector<uint8_t>& data);

    void handle_dhcp_error(const std::string& error_message);
    void parse_dhcp_options(const DHCPMessage& message);
    void send_udp_packet(const std::vector<uint8_t>& packet, uint16_t src_port, uint16_t dest_port, const std::string& dest_ip);
};

DHCPClient::DHCPClient(NetworkInterface& netif)
    : net_interface(netif), transaction_id(0x12345678) {
}

void DHCPClient::send_dhcp_discover() {
    DHCPMessage discover;
    discover.op = 1; // Boot Request
    discover.htype = 1; // Ethernet
    discover.hlen = 6;
    discover.xid = htonl(transaction_id);
    std::memcpy(discover.chaddr, net_interface.get_mac_address().data(), 6);
    discover.options[0] = 53; // DHCP Message Type
    discover.options[1] = 1;
    discover.options[2] = 1; // DHCP Discover

    std::vector<uint8_t> packet = serialize_dhcp_message(discover);
    std::cout << "Sending DHCP Discover" << std::endl;
    send_udp_packet(packet, 68, 67, "255.255.255.255");
}

void DHCPClient::handle_dhcp_offer(const std::vector<uint8_t>& packet) {
    if (packet.size() < sizeof(DHCPMessage)) {
        handle_dhcp_error("Received DHCP offer packet is too small.");
        return;
    }
    DHCPMessage offer = deserialize_dhcp_message(packet);
    if (offer.xid == htonl(transaction_id)) {
        parse_dhcp_options(offer);
        IPAddress ip_addr(ntohl(offer.yiaddr)); // 使用uint32_t类型的构造函数
        net_interface.set_ip_address(ip_addr);
        std::cout << "Received DHCP Offer: " << ip_addr.to_string() << std::endl;
        send_dhcp_request();
    }
}

void DHCPClient::send_dhcp_request() {
    DHCPMessage request;
    request.op = 1; // Boot Request
    request.htype = 1; // Ethernet
    request.hlen = 6;
    request.xid = htonl(transaction_id);
    std::memcpy(request.chaddr, net_interface.get_mac_address().data(), 6);
    request.options[0] = 53; // DHCP Message Type
    request.options[1] = 1;
    request.options[2] = 3; // DHCP Request

    std::vector<uint8_t> packet = serialize_dhcp_message(request);
    std::cout << "Sending DHCP Request" << std::endl;
    send_udp_packet(packet, 68, 67, "255.255.255.255");
}

void DHCPClient::handle_dhcp_ack(const std::vector<uint8_t>& packet) {
    if (packet.size() < sizeof(DHCPMessage)) {
        handle_dhcp_error("Received DHCP ack packet is too small.");
        return;
    }
    DHCPMessage ack = deserialize_dhcp_message(packet);
    if (ack.xid == htonl(transaction_id)) {
        parse_dhcp_options(ack);
        IPAddress ip_addr(ntohl(ack.yiaddr)); // 使用uint32_t类型的构造函数
        net_interface.set_ip_address(ip_addr);
        std::cout << "Received DHCP Ack: " << ip_addr.to_string() << std::endl;
    }
}

std::vector<uint8_t> DHCPClient::serialize_dhcp_message(const DHCPMessage& message) {
    std::vector<uint8_t> buffer(sizeof(DHCPMessage));
    std::memcpy(buffer.data(), &message, sizeof(DHCPMessage));
    return buffer;
}

DHCPClient::DHCPMessage DHCPClient::deserialize_dhcp_message(const std::vector<uint8_t>& data) {
    DHCPMessage message;
    std::memcpy(&message, data.data(), sizeof(DHCPMessage));
    return message;
}

void DHCPClient::handle_dhcp_error(const std::string& error_message) {
    std::cerr << "DHCP Error: " << error_message << std::endl;
}

void DHCPClient::parse_dhcp_options(const DHCPMessage& message) {
    size_t offset = 0;
    while (offset < sizeof(message.options) && message.options[offset] != 0xFF) { // 0xFF 表示选项结束
        uint8_t option = message.options[offset++];
        uint8_t length = message.options[offset++];
        std::vector<uint8_t> data(message.options + offset, message.options + offset + length);
        offset += length;

        switch (option) {
        case 1: // 子网掩码
            if (length == 4) {
                uint32_t mask;
                std::memcpy(&mask, data.data(), 4);
                net_interface.set_subnet_mask(IPAddress(ntohl(mask)));
            }
            break;
        case 3: // 网关
            if (length == 4) {
                uint32_t gateway;
                std::memcpy(&gateway, data.data(), 4);
                net_interface.set_gateway(IPAddress(ntohl(gateway)));
            }
            break;
        case 6: // DNS 服务器
            for (size_t i = 0; i < length; i += 4) {
                uint32_t dns;
                std::memcpy(&dns, data.data() + i, 4);
                net_interface.add_dns_server(IPAddress(ntohl(dns)));
            }
            break;
        default:
            break;
        }
    }
}

void DHCPClient::send_udp_packet(const std::vector<uint8_t>& packet, uint16_t src_port, uint16_t dest_port, const std::string& dest_ip) {
#ifdef _WIN32
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        handle_dhcp_error("Failed to create socket.");
        return;
    }

    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) < 0) {
        handle_dhcp_error("Failed to set socket options.");
        closesocket(sockfd);
        return;
    }

    struct sockaddr_in dest_addr;
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    inet_pton(AF_INET, dest_ip.c_str(), &dest_addr.sin_addr);

    if (sendto(sockfd, (char*)packet.data(), packet.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        handle_dhcp_error("Failed to send packet.");
    }

    closesocket(sockfd);
#else
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        handle_dhcp_error("Failed to create socket.");
        return;
    }

    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        handle_dhcp_error("Failed to set socket options.");
        close(sockfd);
        return;
    }

    struct sockaddr_in dest_addr;
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    inet_pton(AF_INET, dest_ip.c_str(), &dest_addr.sin_addr);

    if (sendto(sockfd, packet.data(), packet.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        handle_dhcp_error("Failed to send packet.");
    }

    close(sockfd);
#endif
}

#endif // DHCPCLIENT_H
