CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

all: synCheck.exe unittest/test_syntax_checker.exe

synCheck.exe: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

unittest/test_syntax_checker.exe: unittest/test_syntax_checker.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test: unittest/test_syntax_checker.exe
	./unittest/test_syntax_checker.exe

clean:
	rm -f *.exe *.o test.c test_output.txt test_output.html
	rm -f unittest/*.exe unittest/*.o

.PHONY: all test clean 