CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = server

SRC = server.c logger.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o server.log
