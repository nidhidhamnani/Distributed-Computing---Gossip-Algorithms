#ifndef __PUSH_HPP__
#define __PUSH_HPP__

#include <vector>
#include <atomic>
#include <mutex>
#include <thread>

#include "common/common.hpp"
#include "packets.hpp"

// Classes and other things related to push based gossip

class GossipPush {

public:

    GossipPush(int my_id, file_input fi):
        PROCESS_ID(my_id), file_inp(fi)
    {
        init();
    }

    // Start the server.
    // Initiate all sending and receiving process.
    void Start();

    // Add another data element to be gossipped.
    void Gossip(int data);

    void IAmDone() {
         i_am_done.store(true);
    };

    // Wait till all the gossip stuff is over.
    void Wait();

    int TotalMessages() {
        return total_messages_sent.load();
    }

private:
    file_input file_inp;

    int PROCESS_ID, K;
    std::vector<int> outgoing_neighbours;
    std::atomic_bool end;
    std::atomic_int total_messages_sent;

    std::mutex send_lock;

    // true if no more new messages from my side.
    std::atomic_bool i_am_done;
    bool sent_terminate;

    void *send_buf, *recv_buf;

    // Set all the local data.
    void init();

    // All the data that I need to gossip,
    // my own and from other nodes.
    std::vector<gossip_data> all_gossips;

    std::thread gossip_thread;
    // Send(gossip) all the data from gossip_data to all neighbours.
    void gossip();
    void add_gossip(int id, int data);

    std::thread recv_thread;
    void recv_event();
    void process_message(int size);

    std::vector<bool> gossips_received;
    bool received_all_gossip();

    std::vector<bool> ended;
    void send_terminate_to_all();
    bool rest_all_terminated();

};

#endif