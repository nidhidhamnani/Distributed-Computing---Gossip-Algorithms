#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__

#include <stdlib.h> 
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <memory.h>
#include <mpi.h>

#include "log.hpp"
#include "common.hpp"

namespace net {

const int max_packet_size = 65536;

MPI_Request request; 

// Sends a message asynchronously.
bool send_msg(int to, void *data, int data_len) {
    MPI_Isend(data, data_len, MPI_BYTE, to, 0, MPI_COMM_WORLD, &request);
    return true;
}

// receives a packet and writes it to data_out.
// returns length of received packet.
int recv_msg(void *data_out) {
    MPI_Status status;
    if(MPI_Recv(data_out, max_packet_size, MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status) != 0) {
        return 0;
    }
    int size;
    MPI_Get_count(&status, MPI_BYTE, &size);
    return size;
}

}
#endif