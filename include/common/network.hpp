#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__

#include <stdlib.h> 
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <memory.h>

#include "log.hpp"
#include "common.hpp"
#include "buffer_utils.hpp"

namespace net {

const char* LOCAL_IP = "0.0.0.0";

// creates server on given port and return the socket.
int create_udp_server(int port) {
    int sock;
    struct sockaddr_in server_addr;

    if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        LOG_FATAL(current_ts(), {"msg", "socket() failed"});
    }

    memset (&server_addr, 0, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    server_addr.sin_port = htons (port);

    if (bind (sock, (struct sockaddr *) &server_addr, sizeof (server_addr)) <0) {
        LOG_FATAL(current_ts(), {"msg", "bind() failed"});
    }

    return sock;
}

// returns a new socket for sending messages.
int send_socket() {
    int sock;
    if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        LOG_FATAL(current_ts(), {"msg", "socket() failed"});
    }
    return sock;
}

bool send_msg(int sock, void *data, int data_len, const char* server_ip, int server_port) {
    struct sockaddr_in server_addr;
    memset (&server_addr, 0, sizeof (server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr (server_ip);
    server_addr.sin_port = htons (server_port);

    if (sendto(sock, data, data_len, 0, (struct sockaddr *) &server_addr, sizeof(server_addr))!=data_len) {
        LOG_WARNING(current_ts(), {"msg", "Could not send packet entirely"});
        return false;
    }

    return true;
}

socklen_t client_addr_len = sizeof (struct sockaddr_in);

// receives a UDP packet and writes it to data_out.
// returns length of received packet.
int recv_msg(int sock, void *data_out) {
    struct sockaddr_in client_addr;
    return recvfrom (sock, data_out, sizeof (max_packet_size), 0,
                (struct sockaddr *) &client_addr, &client_addr_len);
}

}
#endif