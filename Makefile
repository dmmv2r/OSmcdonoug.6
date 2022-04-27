C = gcc

CFLAGS = -Wall -g

TARGETS = oss user_proc

all: $(TARGETS)

oss: oss.o
	$(C) $(CFLAGS) -o $@ oss.o

user_proc: user_proc.o
	$(C) $(CFLAGS) -o $@ user_proc.o

clean:
	rm -rf *.o $(TARGETS)
