# Distributed-Computing Gossip-Algorithms

## How to use

```
# Compile
$ make -j2

# Generate input example.
#   N = number of processess.
# This generates input for 10 processess.
$ make gen_input N=10

# Run push based algo.
#   N = number of processess.
#   PK = (PK% of N) is the number of neighbours to send gossip to.
#   P = (P/100) is the probability of packet loss. 
# NOTE: Input for given N should be generated before.
$ make run_push N=10 PK=20 P=10
$ make run_pull N=10 PK=40 P=20
```

## Benchmark

Inputs are present for benchmarks.

```
# Run this for benchmark.
$ make -j2
$ chmod +x *.sh
$ ./benchmark.sh
```

Output will be in `bench_out` directory.