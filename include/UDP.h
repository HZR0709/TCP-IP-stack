#ifndef UDP_H
#define UDP_H

#include <vector>
#include <cstdint>
#include <cstring>

class UDPSegment {
public:
    uint16_t src_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
    std::vector<uint8_t> payload;

    UDPSegment(uint16_t sp, uint16_t dp, const std::vector<uint8_t>& p)
        : src_port(htons(sp)),
        dest_port(htons(dp)),
        length(htons(8 + static_cast<uint16_t>(p.size()))),
        checksum(0),
        payload(p) {}

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer(8 + payload.size());
        memcpy(buffer.data(), &src_port, 2);
        memcpy(buffer.data() + 2, &dest_port, 2);
        memcpy(buffer.data() + 4, &length, 2);
        memcpy(buffer.data() + 6, &checksum, 2);
        memcpy(buffer.data() + 8, payload.data(), payload.size());
        return buffer;
    }

    static UDPSegment deserialize(const std::vector<uint8_t>& data) {
        uint16_t src_port = (data[0] << 8) | data[1];
        uint16_t dest_port = (data[2] << 8) | data[3];
        uint16_t length = (data[4] << 8) | data[5];
        uint16_t checksum = (data[6] << 8) | data[7];
        std::vector<uint8_t> payload(data.begin() + 8, data.end());

        return UDPSegment(ntohs(src_port), ntohs(dest_port), payload);
    }
};

#endif // UDP_H
