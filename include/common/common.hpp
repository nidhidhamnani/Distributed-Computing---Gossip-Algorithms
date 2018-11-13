#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <string>
#include <fstream>
#include <chrono>
#include "DS.hpp"

long current_ts() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void parse_file(std::string filename, file_input *inp, int base_port) {
    std::ifstream file;
    file.open(filename.c_str());

    int N;
    file >> N;
    inp->N = N;
    inp->base_port = base_port;
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