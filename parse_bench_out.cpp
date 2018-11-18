#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
using namespace std;

string push_file_name(int N, int P, int PK) {
    return  string("out_push.N_") +to_string(N)+ ".PK_" +to_string(PK)+ ".P_" +to_string(P);
}

string pull_file_name(int N, int P, int PK) {
    return  string("out_pull.N_") +to_string(N)+ ".PK_" +to_string(PK)+ ".P_" +to_string(P);
}

int main() {
    int Ns[] = {5, 6, 7, 8, 9, 10};
    int PKs[] = {20, 50};
    int Ps[] = {0, 10, 50};

    for(int i=0; i<2; i++) {
        int PK = PKs[i];
        for(int j=0; j<3; j++) {
            int P = Ps[j];
            printf("\n\nPK=%d, P=%d\n", PK, P);
            cout << "PUSH" << endl;
            for(int k=0; k<6; k++) {
                int N = Ns[k];
                ifstream file;
                string filename = string("bench_push/") + push_file_name(N, P, PK);
                file.open(filename.c_str());

                float sum = 0;
                int temp;
                for(int x=0; x<N; x++) {
                    file >> temp;
                    sum += temp;
                }
                float avg = sum / float(N);

                cout << avg << endl;
            }


            cout << endl << "PULL" << endl;
            for(int k=0; k<6; k++) {
                int N = Ns[k];
                ifstream file;
                string filename = string("bench_pull/") + pull_file_name(N, P, PK);
                file.open(filename.c_str());

                float sum = 0;
                int temp;
                for(int x=0; x<N; x++) {
                    file >> temp;
                    sum += temp;
                }
                float avg = sum / float(N);

                cout << avg << endl;
            }
        }
    }

}