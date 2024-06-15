#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include "Ethernet.h"
#include "TCP.h"
#include "UDP.h"
#include "IP.h"
#include "TCPConnection.h"
#include "NetworkInterface.h"
#include "RoutingTable.h"
#include "DHCPClient.h"
#include "SLAACClient.h"
#include "Network.h"    // 跨平台网络支持


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

    // 添加IPv6路由
    routing_table.add_route(Route(IPAddress("::/0"), IPAddress("::"), IPAddress("fe80::1")));
    routing_table.add_route(Route(IPAddress("2001:db8::"), IPAddress("ffff:ffff:ffff:ffff::"), IPAddress("::")));

    // 查找路由
    try {
        Route route = routing_table.find_route(IPAddress("8.8.8.8"));
        std::cout << "Route found: " << route.get_destination().to_string() << " via " << route.get_gateway().to_string() << std::endl;
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        Route route = routing_table.find_route(IPAddress("2001:4860:4860::8888"));
        std::cout << "Route found: " << route.get_destination().to_string() << " via " << route.get_gateway().to_string() << std::endl;
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }

    cleanup_network();  // 清理网络（Windows）

    return 0;
}