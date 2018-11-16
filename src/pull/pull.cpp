#include "pull/pull.hpp"
#include <thread>
#include <string>
#include <unistd.h>
#include <random>
#include "common/network.hpp"
#include "common/common.hpp"
#include "common/log.hpp"
#include "common/buffer_utils.hpp"
#include <set>
#include <vector>
#include <mutex>
#include <iostream> 
#include <utility>
#include <algorithm>
#include <atomic>
#define KMAG  "\x1B[35m"

using namespace std;

int NODEID, K;
void *sendbuf, *recvbuf;
int MAX_SIZE = 65536;
typedef pair<int, int> pairs;
std::set<pairs> my_msgs;
file_input fi;
int gossip_repeat_interval = 1000000;
mutex buffer_lock;
atomic_bool received_all_messages;
std::vector<bool> terminated_processes;


enum PACKET_TYPE : char {
    ENQUIRE = 0, 
    GOSSIP = 1,
    TERMINATE = 2
};


std::random_device rd;
std::uniform_real_distribution<double> distribution(1, 100);
std::mt19937 engine(rd()); // Mersenne twister MT19937

bool drop_packet() {
    return (distribution(engine) < 50);
}


// Message
struct packet {
    
    PACKET_TYPE type;
    int sender_id;
    set<pairs> msg;

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


// Function to send terminate message after ending
void send_terminate() {

    packet p;
    p.type = TERMINATE;
    p.sender_id = NODEID;
    int size = p.encode_msg(sendbuf);

    for(int i=0; i<fi.N; i++) {
        if(i == NODEID) 
            continue;
        MPI_Send(sendbuf, size, MPI_BYTE, i, 0, MPI_COMM_WORLD);
        
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
    // printf("Process %d sent %d to %d\n",NODEID, type, receiver);
    MPI_Send(sendbuf, size, MPI_BYTE, receiver, 0, MPI_COMM_WORLD);

}

void process_packet(packet p) {

    set<pairs> :: iterator it;
    for (it = p.msg.begin(); it!=p.msg.end(); it++) {
        my_msgs.insert(*it);
    }
    printf("%sProcess %d, set len = %d\n", KMAG, NODEID, my_msgs.size());
    if (my_msgs.size() == fi.N) {
        received_all_messages = true;
    }
}

void recv_messages() {

    // printf("Process %d started recv_messages\n",NODEID);
    packet p;

    MPI_Recv(recvbuf, MAX_SIZE, MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    p.decode_msg(recvbuf);
    // printf("Process %d received message %d from %d\n", NODEID, p.type, p.sender_id);

    auto ts = current_ts();
    if(drop_packet() && p.type != TERMINATE) {
        LOG_WARNING(current_ts(), {"pid", std::to_string(NODEID), "msg", "Packet dropped"});
        return;
    }

    switch(p.type) {

        case ENQUIRE: {
            // printf("Process %d received message %d from %d\n", NODEID, p.type, p.sender_id);
            std::set<pairs> result;
            std::set_difference(my_msgs.begin(), my_msgs.end(), p.msg.begin(), p.msg.end(),
                std::inserter(result, result.end()));

            if (result.size()>0) {
                buffer_lock.lock();
                send_message(p.sender_id, GOSSIP);
                buffer_lock.unlock();
            }
            break;
        }
        case GOSSIP: {
            buffer_lock.lock();
            process_packet(p);
            buffer_lock.unlock();
            break;
        }
        case TERMINATE: {
            auto timestamp = chrono::duration_cast<std::chrono::microseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            printf("p%d receive's terminate message from %d at %ld\n", NODEID, p.sender_id, timestamp); 
            fflush(stdout);
            terminated_processes[p.sender_id]=true;
            break;
        }
    }

}

void start_gossip(){

    LOG_INFO_WHITE(current_ts(), {"pid", std::to_string(NODEID), "msg", "starting gossip"});

    pairs pr;
    pr.first = NODEID;
    pr.second = NODEID;
    buffer_lock.lock();
    my_msgs.insert(pr);
    buffer_lock.unlock();

    vector<int> my_neigh;
    int num_neigh = fi.graph[NODEID].size();
    for(int i=0;i<num_neigh;i++) {
        my_neigh.push_back(fi.graph[NODEID][i]);
    }

    for(int i=0;i<num_neigh;i++) {
        printf("Process %d, my_neigh[%d] = %d\n", NODEID, i, my_neigh[i]);
    }

    while (!received_all_messages.load()) {
        for(int i=0;i<K;i++) {
            int rand_num = int(rand()%num_neigh);
            buffer_lock.lock();
            // usleep(gossip_repeat_interval);
            send_message(my_neigh[rand_num], ENQUIRE);
            buffer_lock.unlock();
        }
        usleep(gossip_repeat_interval);
    }
    auto timestamp = chrono::duration_cast<std::chrono::microseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();

    printf("Process %d SEND TERMINATED! at %ld\n", NODEID, timestamp);
    fflush(stdout);
    send_terminate();
}

int main(int argc, char const *argv[]) {

    srand(time(0)); 
    // Getting process id.
    MPI_Init(NULL, NULL);
    int process_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);

    NODEID=process_id;
    auto filename = argv[1];

    LOG_INFO(current_ts(), {"pid", std::to_string(process_id), "msg", "Waiting to sync"});

    sleep(2);
    LOG_INFO(current_ts(), {"pid", std::to_string(process_id), "msg", "Starting process"});

    // Initialisation
   
    parse_file(filename, &fi);
    K = 2;
    received_all_messages = false;
    sendbuf = malloc(65536);
    recvbuf = malloc(65536);
    terminated_processes.resize(fi.N, false);


    thread recv_thd([](){
        while(true) {
            recv_messages();
        }
    });

    // thread recv_thd([](){
    //     while(!all_processes_ended()) {
    //         recv_messages();
    //     }
    // });

    start_gossip();
    
    sleep(2);
    recv_thd.join();

    MPI_Finalize();

    return 0;

}
