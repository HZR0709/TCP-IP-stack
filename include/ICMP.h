//ICMP–≠“È
#ifndef ICMP_H
#define ICMP_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

class ICMP {
public:
    static std::vector<uint8_t> create_echo_request(uint16_t id, uint16_t seq);
    static std::vector<uint8_t> create_echo_reply(uint16_t id, uint16_t seq);
    static bool parse_echo_reply(const std::vector<uint8_t>& packet, uint16_t& id, uint16_t& seq);
    static bool parse_echo_request(const std::vector<uint8_t>& packet, uint16_t& id, uint16_t& seq);
};

std::vector<uint8_t> ICMP::create_echo_request(uint16_t id, uint16_t seq) {
    std::vector<uint8_t> packet(8, 0);

    packet[0] = 0x08; // Type (Echo request)
    packet[1] = 0x00; // Code
    packet[2] = 0x00; // Checksum (to be calculated)
    packet[3] = 0x00;
    packet[4] = id >> 8;
    packet[5] = id & 0xFF;
    packet[6] = seq >> 8;
    packet[7] = seq & 0xFF;

    // Calculate checksum
    uint32_t sum = 0;
    for (size_t i = 0; i < packet.size(); i += 2) {
        sum += (packet[i] << 8) | packet[i + 1];
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    uint16_t checksum = ~sum;
    packet[2] = checksum >> 8;
    packet[3] = checksum & 0xFF;

    return packet;
}

std::vector<uint8_t> ICMP::create_echo_reply(uint16_t id, uint16_t seq) {
    std::vector<uint8_t> packet(8, 0);

    packet[0] = 0x00; // Type (Echo reply)
    packet[1] = 0x00; // Code
    packet[2] = 0x00; // Checksum (to be calculated)
    packet[3] = 0x00;
    packet[4] = id >> 8;
    packet[5] = id & 0xFF;
    packet[6] = seq >> 8;
    packet[7] = seq & 0xFF;

    // Calculate checksum
    uint32_t sum = 0;
    for (size_t i = 0; i < packet.size(); i += 2) {
        sum += (packet[i] << 8) | packet[i + 1];
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    uint16_t checksum = ~sum;
    packet[2] = checksum >> 8;
    packet[3] = checksum & 0xFF;

    return packet;
}

bool ICMP::parse_echo_reply(const std::vector<uint8_t>& packet, uint16_t& id, uint16_t& seq) {
    if (packet.size() < 8) {
        std::cerr << "Packet too small to be ICMP Echo Reply." << std::endl;
        return false;
    }

    uint8_t type = packet[0];
    uint8_t code = packet[1];

    if (type != 0x00 || code != 0x00) {
        std::cerr << "Packet is not an ICMP Echo Reply." << std::endl;
        std::cerr << "Type: " << static_cast<int>(type) << ", Code: " << static_cast<int>(code) << std::endl;
        return false;
    }

    id = (packet[4] << 8) | packet[5];
    seq = (packet[6] << 8) | packet[7];

    return true;
}

bool ICMP::parse_echo_request(const std::vector<uint8_t>& packet, uint16_t& id, uint16_t& seq) {
    if (packet.size() < 8) {
        std::cerr << "Packet too small to be ICMP Echo Request." << std::endl;
        return false;
    }

    uint8_t type = packet[0];
    uint8_t code = packet[1];

    if (type != 0x08 || code != 0x00) {
        std::cerr << "Packet is not an ICMP Echo Request." << std::endl;
        std::cerr << "Type: " << static_cast<int>(type) << ", Code: " << static_cast<int>(code) << std::endl;
        return false;
    }

    id = (packet[4] << 8) | packet[5];
    seq = (packet[6] << 8) | packet[7];

    return true;
}

#endif // ICMP_H
