#!/bin/bash

BENCHDIR=bench_out

rm -rf $BENCHDIR
mkdir $BENCHDIR

for NPROC in {5..10}
do
    for PERCENTK in 20 50
    do
        for PROB in 0 10 50
        do
            echo "make run_pull FILE=inputs/inp$NPROC.txt N=$NPROC P=$PROB PK=$PERCENTK > tempout 2> $BENCHDIR/out_pull.N_$NPROC.PK_$PERCENTK.P_$PROB"
            make run_pull FILE=inputs/inp$NPROC.txt N=$NPROC P=$PROB PK=$PERCENTK > tempout 2> $BENCHDIR/out_pull.N_$NPROC.PK_$PERCENTK.P_$PROB
        done
    done
done

for NPROC in {5..10}
do
    for PERCENTK in 20 50
    do
        for PROB in 0 10 50
        do
            echo "make run_push FILE=inputs/inp$NPROC.txt N=$NPROC P=$PROB PK=$PERCENTK > tempout 2> $BENCHDIR/out_push.N_$NPROC.PK_$PERCENTK.P_$PROB"
            make run_push FILE=inputs/inp$NPROC.txt N=$NPROC P=$PROB PK=$PERCENTK > tempout 2> $BENCHDIR/out_push.N_$NPROC.PK_$PERCENTK.P_$PROB
        done
    done
done
