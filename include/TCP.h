#ifndef TCP_H
#define TCP_H

#include <vector>
#include <cstdint>
#include <cstring>

class TCPSegment {
public:
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t data_offset_res_flags;
    uint8_t flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_pointer;
    std::vector<uint8_t> payload;

    enum Flag {
        FIN = 0x01,
        SYN = 0x02,
        RST = 0x04,
        PSH = 0x08,
        ACK = 0x10,
        URG = 0x20,
        ECE = 0x40,
        CWR = 0x80
    };

    TCPSegment(uint16_t sp, uint16_t dp, uint32_t seq, uint32_t ack, const std::vector<uint8_t>& p, uint8_t f = 0)
        : src_port(htons(sp)),
        dest_port(htons(dp)),
        seq_num(htonl(seq)),
        ack_num(htonl(ack)),
        data_offset_res_flags((5 << 4)), // Header length
        flags(f),
        window_size(htons(8192)),
        checksum(0),
        urgent_pointer(0),
        payload(p) {
        checksum = calculate_checksum();
    }

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buffer(20 + payload.size());
        memcpy(buffer.data(), &src_port, 2);
        memcpy(buffer.data() + 2, &dest_port, 2);
        memcpy(buffer.data() + 4, &seq_num, 4);
        memcpy(buffer.data() + 8, &ack_num, 4);
        buffer[12] = data_offset_res_flags;
        buffer[13] = flags;
        memcpy(buffer.data() + 14, &window_size, 2);
        memcpy(buffer.data() + 16, &checksum, 2);
        memcpy(buffer.data() + 18, &urgent_pointer, 2);
        memcpy(buffer.data() + 20, payload.data(), payload.size());
        return buffer;
    }

    static TCPSegment deserialize(const std::vector<uint8_t>& data) {
        uint16_t src_port = (data[0] << 8) | data[1];
        uint16_t dest_port = (data[2] << 8) | data[3];
        uint32_t seq_num = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
        uint32_t ack_num = (data[8] << 24) | (data[9] << 16) | (data[10] << 8) | data[11];
        uint8_t data_offset_res_flags = data[12];
        uint8_t flags = data[13];
        uint16_t window_size = (data[14] << 8) | data[15];
        uint16_t checksum = (data[16] << 8) | data[17];
        uint16_t urgent_pointer = (data[18] << 8) | data[19];
        std::vector<uint8_t> payload(data.begin() + 20, data.end());

        return TCPSegment(ntohs(src_port), ntohs(dest_port), ntohl(seq_num), ntohl(ack_num), payload, flags);
    }

private:
    uint16_t calculate_checksum() const {
        // 简单校验和计算，仅供示例
        uint32_t sum = 0;
        const uint16_t* header = reinterpret_cast<const uint16_t*>(this);
        for (int i = 0; i < 10; ++i) {
            sum += ntohs(header[i]);
        }
        for (size_t i = 0; i < payload.size(); i += 2) {
            uint16_t word = payload[i] << 8;
            if (i + 1 < payload.size()) {
                word |= payload[i + 1];
            }
            sum += ntohs(word);
        }
        sum = (sum & 0xFFFF) + (sum >> 16);
        sum += (sum >> 16);
        return htons(~sum);
    }
};

#endif // TCP_H
