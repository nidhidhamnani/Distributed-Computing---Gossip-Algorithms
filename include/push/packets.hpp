#ifndef __PUSH_PACKETS_HPP__
#define __PUSH_PACKETS_HPP__

#include <string>
#include <vector>
#include "common/buffer_utils.hpp"

enum PACKET_TYPE : char {
    GOSSIP = 0, TERMINATE = 1
};

// structure of a gossip message.
struct gossip_data {
    int id;
    int data;

    gossip_data() {}
    gossip_data(int id, int data): id(id), data(data) {}

    const static int marshall_size = 2 * sizeof(int);

    // Encode the packet into storable bytes.
    int marshal(void *original) {
        void *ptr = original;
        ptr = write_int(ptr, id);
        ptr = write_int(ptr, data);
        return 2 * sizeof(int);
    }

    // Decode the packet from bytes.
    void unmarshal(void *original) {
        void *ptr = original;
        id = read_int(&ptr);
        data = read_int(&ptr);
    } 
};

// Structure of a packet.
struct packet {
    PACKET_TYPE type;
    int from; // the sender of the packet.

    // Used only for gossip.
    std::vector<gossip_data> gsp_data;

    packet() {}

    packet(PACKET_TYPE type, int from): type(type), from(from) {}

    std::string type_string() {
        switch(type) {
            case GOSSIP:
                return std::string("GOSSIP");
            case TERMINATE:
                return std::string("TERMINATE");
            default:
                return std::string("");
        }
    }

    // Encode the packet into storable bytes.
    int marshal(void *original) {
        void *ptr = original;
        ptr = write_char(ptr, type);
        ptr = write_int(ptr, from);
        int size = sizeof(char) + sizeof(int);

        if(type == GOSSIP) {
            ptr = write_int(ptr, gsp_data.size());
            size += sizeof(int);
            for(auto g: gsp_data) {
                size += g.marshal(ptr);
                increment(&ptr, gossip_data::marshall_size);
            }
        }

        return size;
    }

    // Decode the packet from bytes.
    void unmarshal(void *original) {
        void *ptr = original;
        type = PACKET_TYPE(read_char(&ptr));
        from = read_int(&ptr);

        if(type == GOSSIP) {
            int size = read_int(&ptr);
            gsp_data.resize(size);
            for(int i=0; i<size; i++) {
                gsp_data[i].unmarshal(ptr);
                increment(&ptr, gossip_data::marshall_size);
            }
        }
    }
};


#endif