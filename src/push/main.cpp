#include <stdio.h>
#include <iostream>
#include "push/push.hpp"
#include "common/DS.hpp"
#include "common/parse_input.hpp"
using namespace std;

int main(int argc, char const *argv[]) {
    printf("I am push\n");

    file_input fi;
    parse_file("inp.txt", &fi, 8000);

    cout << fi.N << " " << fi.base_port << endl;

    for(int i=0; i<fi.N; i++) {
        for(auto v: fi.graph[i]) {
            cout << v << " ";
        }
        cout << endl;
    }    


    return 0;
}
