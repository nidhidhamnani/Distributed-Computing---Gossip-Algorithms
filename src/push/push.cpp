#include <thread>
#include <string>
#include <set>
#include <unistd.h>
#include <random>

#include "push/push.hpp"
#include "push/packets.hpp"
#include "common/network.hpp"
#include "common/common.hpp"
#include "common/log.hpp"

std::random_device rd;
std::uniform_real_distribution<double> distribution(1, 100);
std::mt19937 engine(rd()); // Mersenne twister MT19937
std::uniform_int_distribution<int> int_distribution;

double prob, percent_K;
bool drop_packet() {
    return (distribution(engine) < prob);
}

// microseconds
const int gossip_repeat_interval = 1000000;

// Public functions

void GossipPush::Start() {
    gossip_thread = std::thread([this](){
        LOG_INFO(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Starting gossip routine"});
        while(!rest_all_terminated() || !i_am_done.load()) {
            usleep(gossip_repeat_interval); // make this random
            if(!rest_all_terminated() || !i_am_done.load())
                gossip();
        }
        LOG_INFO(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Ending gossip routine"});
    });
    
    recv_thread = std::thread([this](){
        LOG_INFO(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Starting receive routine"});
        while(!rest_all_terminated() || !received_all_gossip()) {
            recv_event();
        }
        LOG_INFO(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Ending receive routine"});
    });
}

void GossipPush::Gossip(int id) {
    add_gossip(id, PROCESS_ID);
}

void GossipPush::Wait() {
    gossip_thread.join();
    recv_thread.join();
}

// Private functions

void GossipPush::init() {
    ended_count = 0;
    i_am_done.store(false);
    end.store(false);
    sent_terminate = false;
    K = int(float(file_inp.N * percent_K) / 100.0);
    if(K==0) K = 2;
    total_messages_sent.store(0);
    outgoing_neighbours = file_inp.graph[PROCESS_ID];
    send_buf = malloc(net::max_packet_size);
    recv_buf = malloc(net::max_packet_size);
    int_distribution = std::uniform_int_distribution<int>(0, outgoing_neighbours.size()-1);
}

void GossipPush::gossip() {
    send_lock.lock();

    packet pkt(GOSSIP, PROCESS_ID);
    std::set<int> random_K;
    while(random_K.size() < K) {
        int rand_num = int_distribution(engine);
        random_K.emplace(rand_num);
    }
    pkt.gsp_data = all_gossips;
    int size = pkt.marshal(send_buf);
    for(auto neigh: random_K) {
        LOG_INFO_CYAN(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Sending gossip", "to", std::to_string(neigh)});
        total_messages_sent++;
        net::send_msg(outgoing_neighbours[neigh], send_buf, size);
    }

    send_lock.unlock();
}

void GossipPush::add_gossip(int id, int data) {
    send_lock.lock();
    for(auto g: all_gossips) {
        if(g.id == id) {
            send_lock.unlock();
            return;        
        }
    }
    LOG_INFO(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Adding gossip message", "id", std::to_string(id), "data", std::to_string(data)});
    all_gossips.emplace_back(gossip_data(id, data));
    send_lock.unlock();
}

void GossipPush::recv_event() {
    int size = net::recv_msg(recv_buf);
    if(size <= 0) {
        LOG_WARNING(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Receive failed"});
        return;
    }
    process_message(size);
}

void GossipPush::process_message(int size) {
    void *data = recv_buf;

    packet p;
    p.unmarshal(data);
    auto ts = current_ts();
    if(drop_packet() && p.type != TERMINATE) {
        LOG_WARNING(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Packet dropped"});
        return;
    }
    LOG_INFO_MAG(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Received packet", "type", p.type_string(), "from", std::to_string(p.from)});
    switch(p.type) {
        case GOSSIP: {
            for(auto g: p.gsp_data) {
                add_gossip(g.id, g.data);
            }
            if(received_all_gossip() && i_am_done.load() && !sent_terminate) {
                sent_terminate = true;
                LOG_INFO_GREEN(ts, {"pid", std::to_string(PROCESS_ID), "msg", "Received all gossip"});
                send_terminate_to_all();
            }
            break;
        }
        case TERMINATE: {
            ended_count++;
            break;
        }
    }
}

bool GossipPush::received_all_gossip() {
    send_lock.lock();
    int size = all_gossips.size();
    send_lock.unlock();
    return size == (file_inp.N*file_inp.M);
}

bool GossipPush::rest_all_terminated() {
    return ended_count.load() == (file_inp.N);
}

void GossipPush::send_terminate_to_all() {
    packet p(TERMINATE, PROCESS_ID);
    send_lock.lock();
    int size = p.marshal(send_buf);
    ended_count++;
    for(int i=0; i<file_inp.N; i++) {
        if(i == PROCESS_ID) continue;
        net::send_msg(i, send_buf, size);
    }
    send_lock.unlock();
    LOG_INFO_WHITE(current_ts(), {"pid", std::to_string(PROCESS_ID), "msg", "Sent terminate to all"});
}

int main(int argc, char const *argv[]) {

    // Getting process id.
    MPI_Init(NULL, NULL);
    int process_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);

    auto filename = argv[1];
    prob = atoi(argv[2]);
    percent_K = atoi(argv[3]);

    LOG_INFO(current_ts(), {"pid", std::to_string(process_id), "msg", "Waiting to sync"});

    sleep(2);
    LOG_INFO_GREEN(current_ts(), {"pid", std::to_string(process_id), "msg", "Starting process"});

    file_input fi;
    parse_file(filename, &fi);

    GossipPush gp(process_id, fi);

    gp.Start();

    usleep(2000000);

    for(int i=0; i<fi.M; i++) {
        gp.Gossip((process_id*fi.M)+i);
        sleep(2);
    }

    gp.IAmDone();

    gp.Wait();

    LOG_INFO_GREEN(current_ts(), {"pid", std::to_string(process_id), "msg","Success", "total_messages_sent", std::to_string(gp.TotalMessages())});

    fprintf(stderr, "%d\n", gp.TotalMessages());

    MPI_Finalize();
    return 0;
}