#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "ui.h"
#include "net.h"   // <<< dùng network layer chung

int main() {
    printf("Connecting to server...\n");

    // dùng net.c để connect
    int sockfd = net_connect_localhost(SERVER_PORT);
    if (sockfd < 0) {
        perror("Cannot connect to server");
        return 1;
    }

    printf("Connected to server!\n");

    // Gọi menu chính (menu sẽ dùng net_request với sockfd này)
    main_menu(sockfd);

    close(sockfd);
    return 0;
}
