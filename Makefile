CC = gcc
CFLAGS = -Wall -O2
TARGET = collatz

all: $(TARGET)

$(TARGET): collatz.c
	$(CC) $(CFLAGS) -o $(TARGET) collatz.c

clean:
	rm -f $(TARGET)

.PHONY: all clean
