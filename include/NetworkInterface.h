#ifndef NETWORKINTERFACE_H
#define NETWORKINTERFACE_H

#include <string>
#include <vector>
#include "IPAddress.h"

class NetworkInterface {
public:
    NetworkInterface(const std::string& name)
        : interface_name(name), mtu(1500) {
        std::memset(mac_address, 0, sizeof(mac_address));
    }

    void set_ip_address(const IPAddress& addr) {
        ip_address = addr;
    }

    void set_subnet_mask(const IPAddress& mask) {
        subnet_mask = mask;
    }

    void set_gateway(const IPAddress& gw) {
        gateway = gw;
    }

    void add_dns_server(const IPAddress& dns) {
        dns_servers.push_back(dns);
    }

    void set_mac_address(const std::vector<uint8_t>& mac) {
        if (mac.size() == 6) {
            std::memcpy(mac_address, mac.data(), 6);
        }
    }

    std::string get_interface_name() const {
        return interface_name;
    }

    IPAddress get_ip_address() const {
        return ip_address;
    }

    IPAddress get_subnet_mask() const {
        return subnet_mask;
    }

    IPAddress get_gateway() const {
        return gateway;
    }

    std::vector<IPAddress> get_dns_servers() const {
        return dns_servers;
    }

    const std::vector<uint8_t> get_mac_address() const {
        return std::vector<uint8_t>(mac_address, mac_address + 6);
    }

    void set_mtu(int mtu_size) {
        mtu = mtu_size;
    }

    int get_mtu() const {
        return mtu;
    }

private:
    std::string interface_name;
    IPAddress ip_address;
    IPAddress subnet_mask;
    IPAddress gateway;
    std::vector<IPAddress> dns_servers;
    uint8_t mac_address[6];
    int mtu;
};

#endif // NETWORKINTERFACE_H
