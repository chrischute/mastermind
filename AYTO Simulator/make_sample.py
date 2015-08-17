from itertools import permutations
import sys
from random import randint

def main():
    if (len(sys.argv) != 3):
        print 'usage: python make_sample.py n nsamples'
        return

    n = int(sys.argv[1])
    nsamples = int(sys.argv[2])
    poss = list(permutations(range(n)))
    for i in range(nsamples):
        for digit in poss[randint(0, len(poss) - 1)]:
            sys.stdout.write(str(digit))
        sys.stdout.write('\n')


if __name__ == '__main__':
    main()
