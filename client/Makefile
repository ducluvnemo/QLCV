CC = gcc
CFLAGS = -Wall

# ===== CLI client =====
SRCS = client.c ui.c
OBJS = $(SRCS:.c=.o)

# ===== GTK client =====
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)

GTK_SRCS = gtk/main.c gtk/net.c
GTK_OBJS = $(GTK_SRCS:.c=.o)

all: client gtk_client

# ---- CLI client ----
client: $(OBJS)
	$(CC) $(CFLAGS) -o client $(OBJS)

# ---- GTK client ----
gtk_client: ui.o $(GTK_OBJS)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) \
		-o gtk_client ui.o $(GTK_OBJS) \
		$(GTK_LIBS) -pthread

# ---- Compile rules ----
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

gtk/%.o: gtk/%.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

clean:
	rm -f *.o gtk/*.o client gtk_client
