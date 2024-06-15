#ifndef IP_H
#define IP_H

#include <vector>
#include <cstdint>
#include <cstring>

class IPPacket {
public:
    uint8_t version_ihl;
    uint8_t dscp_ecn;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t src;
    uint32_t dest;
    std::vector<uint8_t> payload;

    IPPacket(uint8_t proto, uint32_t s, uint32_t d, const std::vector<uint8_t>& p)
        : version_ihl((4 << 4) | 5), // IPv4 and header length
        dscp_ecn(0),
        total_length(htons(20 + static_cast<uint16_t>(p.size()))),
        identification(0),
        flags_fragment_offset(htons(0x4000)), // Don't fragment
        ttl(64),
        protocol(proto),
        header_checksum(0),
        src(htonl(s)),
        dest(htonl(d)),
        payload(p) {
        header_checksum = calculate_checksum();
    }

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer(20 + payload.size());
        buffer[0] = version_ihl;
        buffer[1] = dscp_ecn;
        memcpy(buffer.data() + 2, &total_length, 2);
        memcpy(buffer.data() + 4, &identification, 2);
        memcpy(buffer.data() + 6, &flags_fragment_offset, 2);
        buffer[8] = ttl;
        buffer[9] = protocol;
        memcpy(buffer.data() + 10, &header_checksum, 2);
        memcpy(buffer.data() + 12, &src, 4);
        memcpy(buffer.data() + 16, &dest, 4);
        memcpy(buffer.data() + 20, payload.data(), payload.size());
        return buffer;
    }

    static IPPacket deserialize(const std::vector<uint8_t>& data) {
        uint8_t version_ihl = data[0];
        uint8_t dscp_ecn = data[1];
        uint16_t total_length = (data[2] << 8) | data[3];
        uint16_t identification = (data[4] << 8) | data[5];
        uint16_t flags_fragment_offset = (data[6] << 8) | data[7];
        uint8_t ttl = data[8];
        uint8_t protocol = data[9];
        uint16_t header_checksum = (data[10] << 8) | data[11];
        uint32_t src = (data[12] << 24) | (data[13] << 16) | (data[14] << 8) | data[15];
        uint32_t dest = (data[16] << 24) | (data[17] << 16) | (data[18] << 8) | data[19];
        std::vector<uint8_t> payload(data.begin() + 20, data.end());

        return IPPacket(protocol, ntohl(src), ntohl(dest), payload);
    }

private:
    uint16_t calculate_checksum() const {
        uint32_t sum = 0;
        const uint16_t* header = reinterpret_cast<const uint16_t*>(this);
        for (int i = 0; i < 10; ++i) {
            sum += ntohs(header[i]);
        }
        sum = (sum & 0xFFFF) + (sum >> 16);
        sum += (sum >> 16);
        return htons(~sum);
    }
};

#endif // IP_H
