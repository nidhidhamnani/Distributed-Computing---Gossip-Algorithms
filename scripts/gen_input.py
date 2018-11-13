import random
import sys

n = int(sys.argv[1])

num_neighbours = n/2

print n

a = range(n)

random.shuffle(a)

for i in range(n):
    print num_neighbours,
    flag = False
    for j in a[:num_neighbours]:
        if j == i:
            flag = True
        else:
            print j,
    if flag:
        print a[num_neighbours],
    random.shuffle(a)
    print ""
