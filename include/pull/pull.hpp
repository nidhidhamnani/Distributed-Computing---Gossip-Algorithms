#ifndef __PUSH_HPP__
#define __PUSH_HPP__

#include <atomic>
#include <utility>
#include <set>

#include "common/common.hpp"

// Global variables
int NODEID; // Process ID
int K; // Number of random neighbours
void *sendbuf, *recvbuf; // Message sending and receiving buffers
int MAX_SIZE = 65536; // Max size of sending and receiving buffers
std::set<pairs> my_msgs; // Set of all messages received
file_input fi; // Struct to store input file
int gossip_repeat_interval = 1000000; // Gossip interval
mutex buffer_lock; // Mutex to lock set
std::vector<bool> terminated_processes; // Vector to check the active/terminate status of other processes
atomic_int total_messages_sent, total_messages_received; // Count of sent and recieved messages

// Initialising random number generator
std::random_device rd;
std::mt19937 engine(rd()); // Mersenne twister MT19937
std::uniform_real_distribution<double> distribution(1, 100);
std::uniform_int_distribution<int> int_distribution;

#endif