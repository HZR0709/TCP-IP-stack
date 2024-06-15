#ifndef IPADDRESS_H
#define IPADDRESS_H

#include <string>
#include <vector>
#include <cstring>
#include "Socket.h"
//#include "Network.h"

class IPAddress {
public:
    enum Type {
        IPv4,
        IPv6
    };

    IPAddress() : type(IPv4) {
        std::memset(address, 0, sizeof(address));
    }

    IPAddress(const std::string& addr) {
        if (addr.find(':') != std::string::npos) {
            type = IPv6;
            inet_pton(AF_INET6, addr.c_str(), address);
        }
        else {
            type = IPv4;
            inet_pton(AF_INET, addr.c_str(), address);
        }
    }

    IPAddress(uint32_t addr) : type(IPv4) {
        std::memset(address, 0, sizeof(address));
        std::memcpy(address, &addr, 4);
    }

    std::string to_string() const {
        char str[INET6_ADDRSTRLEN];
        if (type == IPv4) {
            inet_ntop(AF_INET, address, str, INET_ADDRSTRLEN);
        }
        else {
            inet_ntop(AF_INET6, address, str, INET6_ADDRSTRLEN);
        }
        return std::string(str);
    }

    Type get_type() const {
        return type;
    }

    const uint8_t* get_address() const {
        return address;
    }

private:
    Type type;
    uint8_t address[16]; // 128-bit for IPv6, only first 32-bit used for IPv4
};

#endif // IPADDRESS_H
