#ifndef ETHERNET_H
#define ETHERNET_H

#include <vector>
#include <cstdint>
#include <cstring>

class EthernetFrame {
public:
    uint8_t dest[6];
    uint8_t src[6];
    uint16_t type;
    std::vector<uint8_t> payload;

    EthernetFrame(const uint8_t* d, const uint8_t* s, uint16_t t, const std::vector<uint8_t>& p) {
        memcpy(dest, d, 6);
        memcpy(src, s, 6);
        type = t;
        payload = p;
    }

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer(14 + payload.size());
        memcpy(buffer.data(), dest, 6);
        memcpy(buffer.data() + 6, src, 6);
        buffer[12] = (type >> 8) & 0xFF;
        buffer[13] = type & 0xFF;
        memcpy(buffer.data() + 14, payload.data(), payload.size());
        return buffer;
    }

    static EthernetFrame deserialize(const std::vector<uint8_t>& data) {
        uint8_t dest[6];
        uint8_t src[6];
        uint16_t type;
        memcpy(dest, data.data(), 6);
        memcpy(src, data.data() + 6, 6);
        type = (data[12] << 8) | data[13];
        std::vector<uint8_t> payload(data.begin() + 14, data.end());
        return EthernetFrame(dest, src, type, payload);
    }
};

#endif // ETHERNET_H
