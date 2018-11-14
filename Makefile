all: push pull

N=4

push:
	mpic++ src/push/*.cpp -lpthread -std=c++14 -o push.out -I ./include

pull:
	mpic++ src/pull/*.cpp -lpthread -std=c++14 -o pull.out -I ./include

run_push:
	mpirun -n $(N) push.out inp.txt

run_pull:
	mpirun -n $(N) pull.out inp.txt