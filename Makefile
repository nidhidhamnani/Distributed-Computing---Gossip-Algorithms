all: push pull

N=5
P=10
PK=2
FILE=inp.txt

push:
	mpic++ src/push/*.cpp -lpthread -std=c++14 -o push.out -I ./include

pull:
	mpic++ src/pull/*.cpp -lpthread -std=c++14 -o pull.out -I ./include

run_push:
	mpirun -n $(N) push.out $(FILE) $(P) $(PK)

run_pull:
	mpirun -n $(N) pull.out $(FILE) $(P) $(PK)

gen_input:
	python scripts/gen_input.py $(N) > inp.txt