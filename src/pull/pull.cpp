#include <thread>
#include <string>
#include <unistd.h>
#include <random>
#include <set>
#include <vector>
#include <mutex>
#include <iostream> 
#include <utility>
#include <algorithm>
#include <atomic>

#include "pull/packets.hpp"
#include "pull/pull.hpp"
#include "common/network.hpp"
#include "common/common.hpp"
#include "common/log.hpp"

using namespace std;


double prob, percent_K;
bool drop_packet() {
    return (distribution(engine) < prob);
}

// Function to send terminate message after ending
void send_terminate() {
    packet p;
    p.type = TERMINATE;
    p.sender_id = NODEID;
    int size = p.encode_msg(sendbuf);

    for(int i=0; i<fi.N; i++) {
        if(i == NODEID) 
            continue;
        net::send_msg(i, sendbuf, size);
    }
    terminated_processes[NODEID] = true;
}

// Function to check if all the other processes are ended
bool all_processes_ended() {
    for(int i=0; i<fi.N; i++) {
        if(i == NODEID) continue;
        if(!terminated_processes[i]) {
            return false;
        }
    }
    return true;
}

void send_message(int receiver, PACKET_TYPE type) {

    packet p;
    p.type = type;
    p.sender_id = NODEID;

    if(type == GOSSIP || type == ENQUIRE) {
        p.msg = my_msgs;
    }
    int size = p.encode_msg(sendbuf);
    total_messages_sent++;
    net::send_msg(receiver, sendbuf, size);
}

void process_packet(packet p) {
    int before = my_msgs.size();
    set<pairs> :: iterator it;
    for (it = p.msg.begin(); it!=p.msg.end(); it++) {
        my_msgs.insert(*it);
    }
    int after = my_msgs.size();
    if(after > before) {
        LOG_INFO(current_ts(), {"pid", std::to_string(NODEID), "msg", "Added new gossip message", "from", std::to_string(p.sender_id)});
    }
}

void recv_messages() {

    packet p;
    int size = net::recv_msg(recvbuf);
    if(size <= 0) {
        LOG_WARNING(current_ts(), {"pid", std::to_string(NODEID), "msg", "Receive failed"});
        return;
    }
    p.decode_msg(recvbuf);

    auto ts = current_ts();
    if(drop_packet() && p.type != TERMINATE) {
        LOG_WARNING(current_ts(), {"pid", std::to_string(NODEID), "msg", "Packet dropped"});
        return;
    }

    LOG_INFO_MAG(current_ts(), {"pid", std::to_string(NODEID), "msg", "Received packet", "type", p.type_string(), "from", std::to_string(p.sender_id)});

    switch(p.type) {

        case ENQUIRE: {
            total_messages_received++;
            std::set<pairs> result;
            std::set_difference(my_msgs.begin(), my_msgs.end(), p.msg.begin(), p.msg.end(),
                std::inserter(result, result.end()));

            if (result.size()>0) {
                buffer_lock.lock();
                LOG_INFO_CYAN(current_ts(), {"pid", std::to_string(NODEID), "msg", "Sending gossip", "to", std::to_string(p.sender_id)});
                send_message(p.sender_id, GOSSIP);
                buffer_lock.unlock();
            }
            break;
        }
        case GOSSIP: {
            total_messages_received++;
            buffer_lock.lock();
            process_packet(p);
            buffer_lock.unlock();
            break;
        }
        case TERMINATE: {
            terminated_processes[p.sender_id]=true;
            break;
        }
    }

}

bool received_all_messages() {
    buffer_lock.lock();
    int size = my_msgs.size();
    buffer_lock.unlock();
    return size == (fi.N*fi.M);
}

void start_gossip(){

    LOG_INFO(current_ts(), {"pid", std::to_string(NODEID), "msg", "Starting gossip routine"});


    thread insert_thread([](){
        for(int i=0; i<fi.M; i++) {
            pairs pr;
            pr.first = (NODEID*fi.M)+i;
            pr.second = NODEID;
            buffer_lock.lock();
            my_msgs.insert(pr);
            buffer_lock.unlock();
            sleep(2);
        }
    });

    vector<int> my_neigh;
    int num_neigh = fi.graph[NODEID].size();
    for(int i=0;i<num_neigh;i++) {
        my_neigh.push_back(fi.graph[NODEID][i]);
    }

    while (!received_all_messages()) {
        set<int> random_K;
        while(random_K.size() < K) {
            int rand_num = int_distribution(engine);
            random_K.emplace(rand_num);
        }
        for(int rand_num: random_K) {
            buffer_lock.lock();
            send_message(my_neigh[rand_num], ENQUIRE);
            buffer_lock.unlock();
        }
        usleep(gossip_repeat_interval);
    }
    insert_thread.join();

    LOG_INFO_GREEN(current_ts(), {"pid", std::to_string(NODEID), "msg", "Received all gossip"});
    fflush(stdout);
    buffer_lock.lock();
    send_terminate();
    buffer_lock.unlock();
    LOG_INFO_WHITE(current_ts(), {"pid", std::to_string(NODEID), "msg", "Sent terminate to all"});
    LOG_INFO(current_ts(), {"pid", std::to_string(NODEID), "msg", "Ending gossip routine"});
}


void init() {
    int_distribution = std::uniform_int_distribution<int>(0, fi.graph[NODEID].size()-1);
    K = int(float(fi.N * percent_K) / 100.0);
    if(K==0) K = 2;
    total_messages_sent = 0;
    total_messages_received = 0;
    sendbuf = malloc(65536);
    recvbuf = malloc(65536);
    terminated_processes.resize(fi.N, false);
}


int main(int argc, char const *argv[]) {

 
    MPI_Init(NULL, NULL);
    int process_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);

    NODEID=process_id;
    auto filename = argv[1];
    prob = atoi(argv[2]);
    percent_K = atoi(argv[3]);

    LOG_INFO(current_ts(), {"pid", std::to_string(process_id), "msg", "Waiting to sync"});

    LOG_INFO(current_ts(), {"pid", std::to_string(process_id), "msg", "Starting process"});

    // Initialisation
    parse_file(filename, &fi);
    init();
    
    LOG_INFO(current_ts(), {"pid", std::to_string(NODEID), "msg", "Starting receive routine"});
    thread recv_thd([](){
        while(!all_processes_ended() || !received_all_messages()) {
            recv_messages();
        }
    });
    LOG_INFO(current_ts(), {"pid", std::to_string(NODEID), "msg", "Ending receive routine"});
    start_gossip();
    recv_thd.join();

    LOG_INFO_GREEN(current_ts(), {"pid", std::to_string(process_id), "msg","Success", "total_messages_sent", to_string(total_messages_sent.load()), 
            "total_messages_received", to_string(total_messages_received.load()) });

    fprintf(stderr, "%d\n", total_messages_sent.load());

    MPI_Finalize();

    return 0;

}
