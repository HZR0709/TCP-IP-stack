#ifndef NETWORK_H
#define NETWORK_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

inline void init_network() {
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
}

inline void cleanup_network() {
    WSACleanup();
}

#else
#include <arpa/inet.h>

inline void init_network() {
    // No initialization needed for Linux
}

inline void cleanup_network() {
    // No cleanup needed for Linux
}

#endif

#endif // NETWORK_H
