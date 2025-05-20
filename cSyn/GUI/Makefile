CC = gcc
CFLAGS = -Wall -Wextra $(shell pkg-config --cflags gtk+-3.0 gtksourceview-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0 gtksourceview-3.0)

TARGET = synCheckGUI
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean 