SRCS := $(wildcard *.c) $(wildcard *.s)
DEPS := $(SRCS) $(wildcard *.h)

CC   := gcc
CFLAGS := -g -Wall
OPTS := -static

TARGET := main

all: $(TARGET)

$(TARGET): $(DEPS)
	$(CC) $(CFLAGS) $(OPTS) $(SRCS) -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o *~

re: clean all
