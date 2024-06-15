#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <vector>
#include "Route.h"

class RoutingTable {
public:
    void add_route(const Route& route) {
        routes.push_back(route);
    }

    Route find_route(const IPAddress& dest) const {
        for (const auto& route : routes) {
            if (match(dest, route.get_destination(), route.get_netmask())) {
                return route;
            }
        }
        throw std::runtime_error("No route found");
    }

private:
    std::vector<Route> routes;

    bool match(const IPAddress& addr, const IPAddress& dest, const IPAddress& mask) const {
        if (addr.get_type() != dest.get_type() || addr.get_type() != mask.get_type()) {
            return false;
        }

        const uint8_t* addr_bytes = addr.get_address();
        const uint8_t* dest_bytes = dest.get_address();
        const uint8_t* mask_bytes = mask.get_address();

        for (size_t i = 0; i < (addr.get_type() == IPAddress::IPv4 ? 4 : 16); ++i) {
            if ((addr_bytes[i] & mask_bytes[i]) != (dest_bytes[i] & mask_bytes[i])) {
                return false;
            }
        }

        return true;
    }
};

#endif // ROUTINGTABLE_H
