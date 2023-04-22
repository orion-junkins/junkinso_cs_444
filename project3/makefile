CC = gcc
CFLAGS = -Wall -Wextra
SRCS = pcseml.c eventbuf.c
TARGET = pcseml

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(TARGET)
