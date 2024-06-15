#ifndef ROUTE_H
#define ROUTE_H

#include "IPAddress.h"

class Route {
public:
    Route(const IPAddress& dest, const IPAddress& mask, const IPAddress& gw)
        : destination(dest), netmask(mask), gateway(gw) {}

    IPAddress get_destination() const {
        return destination;
    }

    IPAddress get_netmask() const {
        return netmask;
    }

    IPAddress get_gateway() const {
        return gateway;
    }

private:
    IPAddress destination;
    IPAddress netmask;
    IPAddress gateway;
};

#endif // ROUTE_H
