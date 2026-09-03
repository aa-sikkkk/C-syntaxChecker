CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

all: synCheck.exe

synCheck.exe: main.c lexer.c analysis.c report.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: synCheck.exe
	../unittest/test_syntax_checker.exe

clean:
	rm -f *.exe *.o

.PHONY: all test clean
