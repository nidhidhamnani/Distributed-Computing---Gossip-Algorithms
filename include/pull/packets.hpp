#ifndef __PULL_PACKETS_HPP__
#define __PULL_PACKETS_HPP__

#include <string>
#include <utility>
#include <set>
#include "common/buffer_utils.hpp"

using namespace std; 

typedef pair<int, int> pairs;

enum PACKET_TYPE : char {
    ENQUIRE = 0, 
    GOSSIP = 1,
    TERMINATE = 2
};

// Message
struct packet {
    
    PACKET_TYPE type;
    int sender_id;
    set<pairs> msg;

    std::string type_string() {
        switch(type) {
            case GOSSIP:
                return std::string("GOSSIP");
            case ENQUIRE:
                return std::string("ENQUIRE");
            case TERMINATE:
                return std::string("TERMINATE");
            default:
                return std::string("");
        }
    }

    // Function to encode message to be sent
    int encode_msg(void *original) {
        void *ptr = original;
        ptr = write_char(ptr, type);
        ptr = write_int(ptr, sender_id);

        int total_size = sizeof(char) + sizeof(int);

        if (type == GOSSIP || type == ENQUIRE) {
            int set_size = msg.size();
            ptr = write_int(ptr, set_size);
            set<pairs> :: iterator it;
            pairs pr;
            for (it = msg.begin(); it!=msg.end(); it++) {
                pr = *it;
                ptr = write_int(ptr, pr.first);
                ptr = write_int(ptr, pr.second);
            }   
            total_size += (1 + 2 * set_size) * sizeof(int);
        }
        return total_size;
    }

    // Function to decode received message
    void decode_msg(void *original) {
        void *ptr = original;
        type = PACKET_TYPE(read_char(&ptr));
        sender_id = read_int(&ptr);
        if (type == GOSSIP || type == ENQUIRE) {
            int set_size = read_int(&ptr);
            for(int i=0;i<set_size;i++) {
                pairs pr;
                pr.first = read_int(&ptr);
                pr.second = read_int(&ptr);
                msg.insert(pr);
            }
        }
    }
};

#endif