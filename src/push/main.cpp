#include <stdio.h>
#include <iostream>
#include "push/push.hpp"
#include "common/DS.hpp"
#include "common/common.hpp"
#include "common/log.hpp"
using namespace std;

int main(int argc, char const *argv[]) {

    int base_port = atoi(argv[1]);

    file_input fi;
    parse_file("inp.txt", &fi, base_port);

    cout << fi.N << " " << fi.base_port << endl;

    for(int i=0; i<fi.N; i++) {
        for(auto v: fi.graph[i]) {
            cout << v << " ";
        }
        cout << endl;
    }

    auto ts = current_ts(); // get this when you want to record the time.

    LOG_INFO(ts, {"msg","Success"});

    return 0;
}
