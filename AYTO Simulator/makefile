CC=g++
CFLAGS=-lpthread -std=c++11

all: mpayto

# multi-threaded worst-case evaluation solver
mpayto: mpayto.cpp
	$(CC) $(CFLAGS) -o $@ $^

# single-threaded worst-case evaluation solver
aytowc: aytowc.cpp
	$(CC) $(CFLAGS) -o $@ $^

# original ayto solver, buggy
ayto: ayto.cpp
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) a.out ayto aytowc *~ *#
