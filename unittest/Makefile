CC = gcc
CFLAGS = -Wall -g

all: test_syntax_checker.exe

test_syntax_checker.exe: test_syntax_checker.c
	$(CC) $(CFLAGS) -o $@ $^

test: test_syntax_checker.exe
	./test_syntax_checker.exe

clean:
	rm -f *.exe *.o output.txt test.c

.PHONY: all test clean