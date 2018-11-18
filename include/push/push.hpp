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

    // To mark that no more gossip message will be added.
    // This is needed for termination.
    void IAmDone() {
         i_am_done.store(true);
    };

    // Wait till all the gossip stuff is over.
    void Wait();

    // Total messages sent.
    int TotalMessages() {
        return total_messages_sent.load();
    }

private:
    // The parsed input file.
    file_input file_inp;

    int PROCESS_ID, 
        K; // K neighbours to send gossip to.

    // Outgoing channels.
    std::vector<int> outgoing_neighbours;
    
    std::atomic_int total_messages_sent;

    // This lock is to be acquired to do any actions on message set
    // and while sending any message.
    std::mutex send_lock;

    // true if no more new messages from my side.
    std::atomic_bool i_am_done;
    // true if terminate message is sent.
    bool sent_terminate;

    void *send_buf, *recv_buf;
    std::atomic_int ended_count;

    // Set all the local data.
    void init();

    // All the data that I need to gossip,
    // my own and from other nodes.
    std::vector<gossip_data> all_gossips;

    // Thread which is gossiping.
    std::thread gossip_thread;  
    // Send(gossip) all the data from gossip_data to all neighbours.
    void gossip();
    // Add new message in the gossip set.
    void add_gossip(int id, int data);

    // Thread which is receiving and processing packets.
    std::thread recv_thread;
    // A single event in recv_thread.
    void recv_event();
    // Process the received packet.
    void process_message(int size);

    // returns true if we have received all gossips.
    bool received_all_gossip();

    // Sends terminate to all processess.
    void send_terminate_to_all();

    // returns true if all other processess have terminated.
    bool rest_all_terminated();

};

#endif