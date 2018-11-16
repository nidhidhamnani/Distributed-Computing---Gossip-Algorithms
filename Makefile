all: push pull

N=10

push:
	mpic++ src/push/*.cpp -lpthread -std=c++14 -o push.out -I ./include

pull:
	mpic++ src/pull/*.cpp -lpthread -std=c++14 -o pull.out -I ./include

run_push:
	mpirun -n $(N) push.out inp.txt

run_pull:
	mpirun -n $(N) pull.out inp.txt

gen_input:
	python scripts/gen_input.py $(N) > inp.txt