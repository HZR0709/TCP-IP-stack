//ARP协议（地址解析协议）
//ARP协议用于将IPv4地址映射到MAC地址。
#ifndef ARP_H
#define ARP_H

#include <cstdint>
#include <vector>
#include <cstring>
#include <iostream>

class ARP {
public:
    static std::vector<uint8_t> create_arp_request(uint32_t src_ip, uint8_t* src_mac, uint32_t dest_ip);
    static std::vector<uint8_t> create_arp_reply(uint32_t src_ip, uint8_t* src_mac, uint32_t dest_ip, uint8_t* dest_mac);
    static bool parse_arp_packet(const std::vector<uint8_t>& packet, uint32_t& src_ip, uint8_t* src_mac, uint32_t& dest_ip, uint8_t* dest_mac, bool& is_request);
};

std::vector<uint8_t> ARP::create_arp_request(uint32_t src_ip, uint8_t* src_mac, uint32_t dest_ip) {
    std::vector<uint8_t> packet(28, 0);

    packet[0] = 0x00; // Hardware type (Ethernet)
    packet[1] = 0x01;
    packet[2] = 0x08; // Protocol type (IPv4)
    packet[3] = 0x00;
    packet[4] = 0x06; // Hardware size
    packet[5] = 0x04; // Protocol size
    packet[6] = 0x00; // Opcode (request)
    packet[7] = 0x01;
    std::memcpy(&packet[8], src_mac, 6);
    std::memcpy(&packet[14], &src_ip, 4);
    std::memset(&packet[18], 0x00, 6);
    std::memcpy(&packet[24], &dest_ip, 4);

    return packet;
}

std::vector<uint8_t> ARP::create_arp_reply(uint32_t src_ip, uint8_t* src_mac, uint32_t dest_ip, uint8_t* dest_mac) {
    std::vector<uint8_t> packet(28, 0);

    packet[0] = 0x00; // Hardware type (Ethernet)
    packet[1] = 0x01;
    packet[2] = 0x08; // Protocol type (IPv4)
    packet[3] = 0x00;
    packet[4] = 0x06; // Hardware size
    packet[5] = 0x04; // Protocol size
    packet[6] = 0x00; // Opcode (reply)
    packet[7] = 0x02;
    std::memcpy(&packet[8], src_mac, 6);
    std::memcpy(&packet[14], &src_ip, 4);
    std::memcpy(&packet[18], dest_mac, 6);
    std::memcpy(&packet[24], &dest_ip, 4);

    return packet;
}

bool ARP::parse_arp_packet(const std::vector<uint8_t>& packet, uint32_t& src_ip, uint8_t* src_mac, uint32_t& dest_ip, uint8_t* dest_mac, bool& is_request) {
    if (packet.size() < 28) return false;

    uint16_t hardware_type = (packet[0] << 8) | packet[1];
    uint16_t protocol_type = (packet[2] << 8) | packet[3];
    uint8_t hardware_size = packet[4];
    uint8_t protocol_size = packet[5];
    uint16_t opcode = (packet[6] << 8) | packet[7];

    if (hardware_type != 1 || protocol_type != 0x0800 || hardware_size != 6 || protocol_size != 4) {
        return false;
    }

    std::memcpy(src_mac, &packet[8], 6);
    std::memcpy(&src_ip, &packet[14], 4);
    std::memcpy(dest_mac, &packet[18], 6);
    std::memcpy(&dest_ip, &packet[24], 4);

    is_request = (opcode == 1);
    return true;
}

#endif // ARP_H
