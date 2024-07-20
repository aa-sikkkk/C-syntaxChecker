CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iheaders
SOURCES = main.c helpers/style_check.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = code_analysis_tool

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
