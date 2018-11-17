#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <string>
#include <fstream>
#include <chrono>

struct file_input {
    int N, M;
    std::vector<std::vector<int>> graph;

    int num_gossip_msg;
};

long current_ts() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void parse_file(std::string filename, file_input *inp) {
    std::ifstream file;
    file.open(filename.c_str());

    int N, M;
    file >> N >> M;
    inp->N = N;
    inp->M = M;
    int size, val;
    inp->graph.resize(N);
    for(int i=0; i<N; i++) {
        file >> size;
        inp->graph[i].resize(size);
        for(int j=0; j<size; j++) {
            file >> val;
            inp->graph[i][j] = val;
        }
    }

    file.close();
}

#endif