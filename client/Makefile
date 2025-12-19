CC = gcc
CFLAGS = -Wall
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)

all: client gtk_client

client: client.o ui.o net.o
	$(CC) $(CFLAGS) -o client client.o ui.o net.o

gtk_client: gtk/main.o net.o
	$(CC) $(CFLAGS) -o gtk_client gtk/main.o net.o $(GTK_LIBS)

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

ui.o: ui.c
	$(CC) $(CFLAGS) -c ui.c

net.o: net.c
	$(CC) $(CFLAGS) -c net.c

gtk/main.o: gtk/main.c
	$(CC) $(CFLAGS) -I. $(GTK_CFLAGS) -c gtk/main.c -o gtk/main.o

clean:
	rm -f *.o gtk/*.o client gtk_client
