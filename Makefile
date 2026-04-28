CC = gcc
CFLAGS = -Wall -Wextra

TARGET = mainPage

OBJS = mainPage.o udp_server.o udp_client.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

mainPage.o: mainPage.c
	$(CC) $(CFLAGS) -c mainPage.c

udp_server.o: udp_server.c
	$(CC) $(CFLAGS) -c udp_server.c

udp_client.o: udp_client.c
	$(CC) $(CFLAGS) -c udp_client.c

clean:
	rm -f *.o $(TARGET)