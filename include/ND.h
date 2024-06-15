//ND协议（邻居发现协议）
//ND协议用于将IPv6地址映射到MAC地址。
#ifndef ND_H
#define ND_H

#include <cstdint>
#include <vector>
#include <cstring>
#include <iostream>

class ND {
public:
    static std::vector<uint8_t> create_ns(uint32_t src_ip, uint8_t* src_mac, uint32_t dest_ip);
    static std::vector<uint8_t> create_na(uint32_t src_ip, uint8_t* src_mac, uint32_t dest_ip, uint8_t* dest_mac);
    static bool parse_na_packet(const std::vector<uint8_t>& packet, uint32_t& src_ip, uint8_t* src_mac, uint32_t& dest_ip, uint8_t* dest_mac, bool& is_solicited);
};

std::vector<uint8_t> ND::create_ns(uint32_t src_ip, uint8_t* src_mac, uint32_t dest_ip) {
    std::vector<uint8_t> packet(24, 0);

    packet[0] = 0x87; // ICMPv6 Type (Neighbor Solicitation)
    packet[1] = 0x00; // ICMPv6 Code
    packet[2] = 0x00; // Checksum (to be calculated)
    packet[3] = 0x00;
    std::memcpy(&packet[8], &dest_ip, 4); // Target Address

    return packet;
}

std::vector<uint8_t> ND::create_na(uint32_t src_ip, uint8_t* src_mac, uint32_t dest_ip, uint8_t* dest_mac) {
    std::vector<uint8_t> packet(24, 0);

    packet[0] = 0x88; // ICMPv6 Type (Neighbor Advertisement)
    packet[1] = 0x00; // ICMPv6 Code
    packet[2] = 0x00; // Checksum (to be calculated)
    packet[3] = 0x00;
    std::memcpy(&packet[8], &src_ip, 4); // Target Address

    return packet;
}

bool ND::parse_na_packet(const std::vector<uint8_t>& packet, uint32_t& src_ip, uint8_t* src_mac, uint32_t& dest_ip, uint8_t* dest_mac, bool& is_solicited) {
    if (packet.size() < 24) return false;

    uint8_t type = packet[0];
    uint8_t code = packet[1];

    if (type != 0x88 || code != 0x00) {
        return false;
    }

    std::memcpy(&src_ip, &packet[8], 4);
    std::memcpy(dest_mac, &packet[18], 6);

    is_solicited = (packet[4] & 0x40);
    return true;
}

#endif // ND_H
