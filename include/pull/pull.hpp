#ifndef __PUSH_HPP__
#define __PUSH_HPP__

#include <atomic>
#include <utility>
#include <set>

#include "common/common.hpp"

int NODEID, K;
void *sendbuf, *recvbuf;
std::set<pairs> my_msgs;
file_input fi;
int gossip_repeat_interval = 1000000;
mutex buffer_lock;
std::vector<bool> terminated_processes;
atomic_int total_messages_sent, total_messages_received;

std::random_device rd;
std::mt19937 engine(rd()); // Mersenne twister MT19937

std::uniform_real_distribution<double> distribution(1, 100);
std::uniform_int_distribution<int> int_distribution;

#endif