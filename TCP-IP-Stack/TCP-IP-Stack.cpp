#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include "Ethernet.h"
#include "TCP.h"
#include "UDP.h"
#include "ARP.h"
#include "ND.h"
#include "IP.h"
#include "ICMP.h"
#include "TCPConnection.h"
#include "NetworkInterface.h"
#include "RoutingTable.h"
#include "DHCPClient.h"
#include "SLAACClient.h"

#include "Network.h"    // 跨平台网络支持
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

std::vector<uint8_t> create_dhcp_offer_packet(uint32_t yiaddr, uint32_t xid) {
    DHCPClient::DHCPMessage offer;
    offer.op = 2; // Boot Reply
    offer.htype = 1; // Ethernet
    offer.hlen = 6;
    offer.xid = htonl(xid);
    offer.yiaddr = htonl(yiaddr);
    offer.options[0] = 53; // DHCP Message Type
    offer.options[1] = 1;
    offer.options[2] = 2; // DHCP Offer

    std::vector<uint8_t> packet(sizeof(DHCPClient::DHCPMessage));
    std::memcpy(packet.data(), &offer, sizeof(DHCPClient::DHCPMessage));
    return packet;
}

std::vector<uint8_t> create_dhcp_ack_packet(uint32_t yiaddr, uint32_t xid) {
    DHCPClient::DHCPMessage ack;
    ack.op = 2; // Boot Reply
    ack.htype = 1; // Ethernet
    ack.hlen = 6;
    ack.xid = htonl(xid);
    ack.yiaddr = htonl(yiaddr);
    ack.options[0] = 53; // DHCP Message Type
    ack.options[1] = 1;
    ack.options[2] = 5; // DHCP Ack

    std::vector<uint8_t> packet(sizeof(DHCPClient::DHCPMessage));
    std::memcpy(packet.data(), &ack, sizeof(DHCPClient::DHCPMessage));
    return packet;
}

std::vector<uint8_t> create_ra_packet() {
    std::vector<uint8_t> ra_packet(16, 0); // 简单的ICMPv6 Router Advertisement packet
    ra_packet[0] = 134; // ICMPv6 Type: Router Advertisement
    ra_packet[1] = 0;   // ICMPv6 Code
    ra_packet[2] = 0;   // Checksum (to be calculated by the kernel)
    ra_packet[3] = 0;

    // 假设我们在RA包中添加了前缀信息
    ra_packet.insert(ra_packet.end(), { 0x20, 0x01, 0x0d, 0xb8, 0xac, 0x10, 0xfe, 0x01 });

    return ra_packet;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }
#endif

    init_network();  // 初始化网络（Windows）

    // 配置网络接口
    NetworkInterface net_if("eth0");
    net_if.set_mac_address({ 0x00, 0x0c, 0x29, 0x36, 0xbc, 0x17 });

    // 创建DHCP客户端并发送DHCP Discover
    DHCPClient dhcp_client(net_if);
    dhcp_client.send_dhcp_discover();

    // 模拟接收一个DHCP Offer包
    uint32_t offer_ip = inet_addr("192.168.0.100");
    std::vector<uint8_t> dhcp_offer = create_dhcp_offer_packet(offer_ip, 0x12345678);
    dhcp_client.handle_dhcp_offer(dhcp_offer);

    // 模拟接收一个DHCP Ack包
    uint32_t ack_ip = inet_addr("192.168.0.100");
    std::vector<uint8_t> dhcp_ack = create_dhcp_ack_packet(ack_ip, 0x12345678);
    dhcp_client.handle_dhcp_ack(dhcp_ack);

    // 打印网络接口配置信息
    std::cout << "Interface: " << net_if.get_interface_name() << std::endl;
    std::cout << "IP Address: " << net_if.get_ip_address().to_string() << std::endl;

    // 配置路由表
    RoutingTable routing_table;
    routing_table.add_route(Route(IPAddress("0.0.0.0"), IPAddress("0.0.0.0"), IPAddress("192.168.0.1")));
    routing_table.add_route(Route(IPAddress("192.168.0.0"), IPAddress("255.255.255.0"), IPAddress("0.0.0.0")));

    // 创建SLAAC客户端并发送Router Solicitation
    SLAACClient slaac_client(net_if);
    slaac_client.send_rs();

    // 模拟接收一个Router Advertisement包
    std::vector<uint8_t> ra_packet = create_ra_packet();
    slaac_client.handle_ra(ra_packet);

    // 创建TCP连接并进行三次握手
    TCPConnection tcp_conn(12345, 80, inet_addr("192.168.0.101"), inet_addr("192.168.0.1"));
    tcp_conn.send_syn();
    // 模拟接收SYN-ACK
    TCPSegment syn_ack_segment(80, 12345, 0, 1, {}, TCPSegment::SYN | TCPSegment::ACK);
    tcp_conn.receive_syn_ack(syn_ack_segment);
    // 模拟接收ACK
    tcp_conn.receive_ack();
    // 模拟发送FIN
    tcp_conn.send_fin();
    // 模拟接收ACK for FIN
    tcp_conn.receive_ack_for_fin();
    // 模拟接收FIN
    tcp_conn.receive_fin();
    // 模拟接收ACK
    tcp_conn.receive_ack();

    // 测试ARP协议
    std::vector<uint8_t> arp_request = ARP::create_arp_request(inet_addr("192.168.0.100"), (uint8_t *)net_if.get_mac_address().data(), inet_addr("192.168.0.1"));
    uint32_t src_ip, dest_ip;
    uint8_t src_mac[6], dest_mac[6];
    bool is_request;
    if (ARP::parse_arp_packet(arp_request, src_ip, src_mac, dest_ip, dest_mac, is_request)) {
        std::cout << "ARP Request parsed successfully" << std::endl;
    }

    // 测试ICMP Echo Request
    std::vector<uint8_t> icmp_request = ICMP::create_echo_request(1, 1);
    uint16_t id, seq;
    std::cout << "ICMP Request packet: ";
    for (const auto& byte : icmp_request) {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;

    // 解析 Echo Request
    if (ICMP::parse_echo_request(icmp_request, id, seq)) {
        std::cout << "ICMP Echo Request parsed successfully" << std::endl;
        std::cout << "ID: " << id << ", Sequence: " << seq << std::endl;
    }
    else {
        std::cout << "Failed to parse ICMP Echo Request" << std::endl;
    }

    // 创建并测试ICMP Echo Reply
    std::vector<uint8_t> icmp_reply = ICMP::create_echo_reply(1, 1);
    std::cout << "ICMP Reply packet: ";
    for (const auto& byte : icmp_reply) {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;

    // 解析 Echo Reply
    if (ICMP::parse_echo_reply(icmp_reply, id, seq)) {
        std::cout << "ICMP Echo Reply parsed successfully" << std::endl;
        std::cout << "ID: " << id << ", Sequence: " << seq << std::endl;
    }
    else {
        std::cout << "Failed to parse ICMP Echo Reply" << std::endl;
    }

    cleanup_network();  // 清理网络（Windows）

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}