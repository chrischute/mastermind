CC=g++
CFLAGS=-lpthread -std=c++11

all: ayto

# polished multi-threaded version
ayto: ayto.cpp
	$(CC) $(CFLAGS) -o $@ $^

# multi-threaded worst-case evaluation solver
mpayto: mpayto.cpp
	$(CC) $(CFLAGS) -o $@ $^

# single-threaded worst-case evaluation solver
aytowc: aytowc.cpp
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) a.out ayto aytowc mpayto *~ *#
