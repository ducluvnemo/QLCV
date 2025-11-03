// khoi tao & ket noi socket
// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "logger.h"

#define PORT 5555
#define MAX_CLIENTS  FD_SETSIZE
#define BUF_SIZE 1024

int main() {
    int listen_fd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char buf[BUF_SIZE];

    fd_set master_set, read_fds;
    int fd_max;

    // 1. init logger
    init_logger("server.log");
    write_log("[SERVER] starting...");

    // 2. tao socket
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // tranh loi "Address already in use"
    int yes = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // 3. bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    // 4. listen
    if (listen(listen_fd, 10) == -1) {
        perror("listen");
        exit(1);
    }

    // 5. khoi tao FD
    FD_ZERO(&master_set);
    FD_ZERO(&read_fds);
    FD_SET(listen_fd, &master_set);
    fd_max = listen_fd;

    write_log("[SERVER] listening on port %d", PORT);
    printf("Server listening on port %d...\n", PORT);

    // 6. vong lap chinh
    for (;;) {
        read_fds = master_set;

        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        // duyet cac fd
        for (int i = 0; i <= fd_max; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listen_fd) {
                    // ket noi moi
                    addr_len = sizeof(client_addr);
                    new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);
                    if (new_fd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(new_fd, &master_set);
                        if (new_fd > fd_max) fd_max = new_fd;

                        write_log("[SERVER] new connection from %s on socket %d",
                                  inet_ntoa(client_addr.sin_addr), new_fd);
                        printf("New connection from %s on socket %d\n",
                               inet_ntoa(client_addr.sin_addr), new_fd);
                    }
                } else {
                    // du lieu tu client
                    int nbytes = recv(i, buf, sizeof(buf) - 1, 0);
                    if (nbytes <= 0) {
                        // client close
                        if (nbytes == 0) {
                            write_log("[SERVER] socket %d disconnected", i);
                            printf("Socket %d disconnected\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &master_set);
                    } else {
                        // nbytes > 0
                        buf[nbytes] = '\0';
                        write_log("[RECV][fd=%d] %s", i, buf);
                        printf("[fd=%d] %s\n", i, buf);

                        // phan hoi lai client
                        char reply[BUF_SIZE];
                        // dung %.*s de khong bi tran
                        snprintf(reply, sizeof(reply),
                                 "OK: received \"%.*s\"\n", nbytes, buf);

                        send(i, reply, strlen(reply), 0);
                        write_log("[SEND][fd=%d] %s", i, reply);
                    }
                }
            }
        }
    }

    close_logger();
    close(listen_fd);
    return 0;
}
