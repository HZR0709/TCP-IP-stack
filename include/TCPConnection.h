#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "TCP.h"
#include "IP.h"
#include "Ethernet.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <map>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

class TCPConnection {
public:
    enum State {
        CLOSED,
        LISTEN,
        SYN_SENT,
        SYN_RECEIVED,
        ESTABLISHED,
        FIN_WAIT_1,
        FIN_WAIT_2,
        CLOSE_WAIT,
        CLOSING,
        LAST_ACK,
        TIME_WAIT
    };

    TCPConnection(uint16_t sp, uint16_t dp, uint32_t ss_addr, uint32_t d_addr) {
        state = CLOSED;
        src_port = sp;
        dest_port = dp;
        seq_num = 0;
        ack_num = 0;
        src_ip = ss_addr;
        dest_ip = d_addr;
    }

    void send_syn() {
        if (state == CLOSED) {
            TCPSegment syn_segment(src_port, dest_port, seq_num, ack_num, {}, TCPSegment::SYN);
            std::vector<uint8_t> serialized_syn_segment = syn_segment.serialize();
            IPPacket ip_syn_packet(IPPROTO_TCP, src_ip, dest_ip, serialized_syn_segment);
            std::vector<uint8_t> serialized_ip_syn_packet = ip_syn_packet.serialize();
            EthernetFrame ethernet_syn_frame(dest_mac, src_mac, 0x0800, serialized_ip_syn_packet);
            std::vector<uint8_t> serialized_ethernet_syn_frame = ethernet_syn_frame.serialize();
            send_ethernet_frame(serialized_ethernet_syn_frame);
            log("Sending SYN");
            state = SYN_SENT;
        }
    }

    void receive_syn(const TCPSegment& segment) {
        if (state == LISTEN && (segment.flags & TCPSegment::SYN)) {
            seq_num = segment.ack_num;
            ack_num = segment.seq_num + 1;
            state = SYN_RECEIVED;
            log("Received SYN, transitioning to SYN_RECEIVED");
        }
    }

    void receive_syn_ack(const TCPSegment& segment) {
        if (state == SYN_SENT && (segment.flags & (TCPSegment::SYN | TCPSegment::ACK))) {
            ack_num = segment.seq_num + 1;
            TCPSegment ack_segment(src_port, dest_port, seq_num + 1, ack_num, {}, TCPSegment::ACK);
            std::vector<uint8_t> serialized_ack_segment = ack_segment.serialize();
            IPPacket ip_ack_packet(IPPROTO_TCP, src_ip, dest_ip, serialized_ack_segment);
            std::vector<uint8_t> serialized_ip_ack_packet = ip_ack_packet.serialize();
            EthernetFrame ethernet_ack_frame(dest_mac, src_mac, 0x0800, serialized_ip_ack_packet);
            std::vector<uint8_t> serialized_ethernet_ack_frame = ethernet_ack_frame.serialize();
            send_ethernet_frame(serialized_ethernet_ack_frame);
            log("Received SYN-ACK, sending ACK");
            state = ESTABLISHED;
        }
    }

    void send_ack() {
        if (state == SYN_RECEIVED) {
            TCPSegment ack_segment(src_port, dest_port, seq_num, ack_num, {}, TCPSegment::ACK);
            std::vector<uint8_t> serialized_ack_segment = ack_segment.serialize();
            IPPacket ip_ack_packet(IPPROTO_TCP, src_ip, dest_ip, serialized_ack_segment);
            std::vector<uint8_t> serialized_ip_ack_packet = ip_ack_packet.serialize();
            EthernetFrame ethernet_ack_frame(dest_mac, src_mac, 0x0800, serialized_ip_ack_packet);
            std::vector<uint8_t> serialized_ethernet_ack_frame = ethernet_ack_frame.serialize();
            send_ethernet_frame(serialized_ethernet_ack_frame);
            log("Sending ACK");
            state = ESTABLISHED;
        }
    }

    void send_fin() {
        if (state == ESTABLISHED) {
            TCPSegment fin_segment(src_port, dest_port, seq_num + 1, ack_num, {}, TCPSegment::FIN);
            std::vector<uint8_t> serialized_fin_segment = fin_segment.serialize();
            IPPacket ip_fin_packet(IPPROTO_TCP, src_ip, dest_ip, serialized_fin_segment);
            std::vector<uint8_t> serialized_ip_fin_packet = ip_fin_packet.serialize();
            EthernetFrame ethernet_fin_frame(dest_mac, src_mac, 0x0800, serialized_ip_fin_packet);
            std::vector<uint8_t> serialized_ethernet_fin_frame = ethernet_fin_frame.serialize();
            send_ethernet_frame(serialized_ethernet_fin_frame);
            log("Sending FIN");
            state = FIN_WAIT_1;
        }
    }

    void receive_ack_for_fin() {
        if (state == FIN_WAIT_1) {
            log("Received ACK for FIN");
            state = FIN_WAIT_2;
        }
    }

    void receive_fin() {
        if (state == FIN_WAIT_2) {
            TCPSegment ack_segment(src_port, dest_port, seq_num + 1, ack_num, {}, TCPSegment::ACK);
            std::vector<uint8_t> serialized_ack_segment = ack_segment.serialize();
            IPPacket ip_ack_packet(IPPROTO_TCP, src_ip, dest_ip, serialized_ack_segment);
            std::vector<uint8_t> serialized_ip_ack_packet = ip_ack_packet.serialize();
            EthernetFrame ethernet_ack_frame(dest_mac, src_mac, 0x0800, serialized_ip_ack_packet);
            std::vector<uint8_t> serialized_ethernet_ack_frame = ethernet_ack_frame.serialize();
            send_ethernet_frame(serialized_ethernet_ack_frame);
            log("Received FIN, sending ACK");
            state = TIME_WAIT;
        }
        else if (state == ESTABLISHED) {
            log("Received FIN in ESTABLISHED state, transitioning to CLOSE_WAIT");
            state = CLOSE_WAIT;
        }
        else if (state == CLOSE_WAIT) {
            TCPSegment ack_segment(src_port, dest_port, seq_num + 1, ack_num, {}, TCPSegment::ACK);
            std::vector<uint8_t> serialized_ack_segment = ack_segment.serialize();
            IPPacket ip_ack_packet(IPPROTO_TCP, src_ip, dest_ip, serialized_ack_segment);
            std::vector<uint8_t> serialized_ip_ack_packet = ip_ack_packet.serialize();
            EthernetFrame ethernet_ack_frame(dest_mac, src_mac, 0x0800, serialized_ip_ack_packet);
            std::vector<uint8_t> serialized_ethernet_ack_frame = ethernet_ack_frame.serialize();
            send_ethernet_frame(serialized_ethernet_ack_frame);
            log("Received FIN in CLOSE_WAIT, sending ACK and transitioning to LAST_ACK");
            state = LAST_ACK;
        }
    }

    void receive_ack() {
        if (state == LAST_ACK) {
            log("Received ACK in LAST_ACK, transitioning to CLOSED");
            state = CLOSED;
        }
    }

    void handle_timeout() {
        auto now = std::chrono::steady_clock::now();
        if (now - last_sent_time > timeout_duration) {
            log("Timeout occurred, retransmitting last segment");
            retransmit_last_segment();
            last_sent_time = now;
        }
    }

    void retransmit_last_segment() {
        if (!sent_segments.empty()) {
            auto last_segment = sent_segments.rbegin();
            send_ethernet_frame(last_segment->second);
            log("Retransmitting segment with seq_num: " + std::to_string(last_segment->first));
        }
    }

    void log(const std::string& message) const {
        std::cout << "[" << state_to_string() << "] " << message << std::endl;
    }

    State get_state() const {
        return state;
    }

    std::string state_to_string() const {
        switch (state) {
        case CLOSED: return "CLOSED";
        case LISTEN: return "LISTEN";
        case SYN_SENT: return "SYN_SENT";
        case SYN_RECEIVED: return "SYN_RECEIVED";
        case ESTABLISHED: return "ESTABLISHED";
        case FIN_WAIT_1: return "FIN_WAIT_1";
        case FIN_WAIT_2: return "FIN_WAIT_2";
        case CLOSE_WAIT: return "CLOSE_WAIT";
        case CLOSING: return "CLOSING";
        case LAST_ACK: return "LAST_ACK";
        case TIME_WAIT: return "TIME_WAIT";
        default: return "UNKNOWN";
        }
    }

private:
    State state;
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint32_t src_ip;
    uint32_t dest_ip;
    uint8_t dest_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    uint8_t src_mac[6] = { 0x00, 0x0c, 0x29, 0x36, 0xbc, 0x17 };
    std::map<uint32_t, std::vector<uint8_t>> sent_segments;
    std::chrono::steady_clock::time_point last_sent_time;
    const std::chrono::seconds timeout_duration = std::chrono::seconds(3);

    void send_ethernet_frame(const std::vector<uint8_t>& frame) {
#ifdef _WIN32
        SOCKET sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        if (sockfd == INVALID_SOCKET) {
            log("Failed to create socket.");
            return;
        }

        sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = dest_ip;
        dest_addr.sin_port = htons(dest_port);

        if (sendto(sockfd, reinterpret_cast<const char*>(frame.data()), frame.size(), 0, (sockaddr*)&dest_addr, sizeof(dest_addr)) == SOCKET_ERROR) {
            log("Failed to send frame.");
        }

        closesocket(sockfd);
#else
        int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sockfd < 0) {
            log("Failed to create socket.");
            return;
        }

        struct sockaddr_ll dest_addr = { 0 };
        dest_addr.sll_ifindex = if_nametoindex("eth0");
        dest_addr.sll_protocol = htons(ETH_P_IP);
        dest_addr.sll_halen = ETH_ALEN;
        memcpy(dest_addr.sll_addr, dest_mac, 6);

        if (sendto(sockfd, frame.data(), frame.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
            log("Failed to send frame.");
        }

        close(sockfd);
#endif
    }
};

#endif // TCPCONNECTION_H
