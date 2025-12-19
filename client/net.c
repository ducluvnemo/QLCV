#include "net.h"
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>


static int write_all(int fd, const char *buf, int len) {
    int sent = 0;
    while (sent < len) {
        int n = (int)write(fd, buf + sent, len - sent);
        if (n <= 0) return -1;
        sent += n;
    }
    return 0;
}

int net_connect_localhost(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

/*
 Protocol assumption (y như client console thường dùng):
 - gửi 1 dòng request kết thúc '\n'
 - server trả về: "<code>|<payload...>\n" hoặc "<code>|<payload...>" nhiều dòng
 Hàm này:
 - trả về code (0 là OK)
 - payload copy vào resp (không gồm "code|")
*/
int net_request(int sockfd, const char *req, char *resp, int resp_size) {
    if (!req || !resp || resp_size <= 0) return 1;
    resp[0] = '\0';

    int req_len = (int)strlen(req);
    if (req_len <= 0) return 1;

    // đảm bảo request có \n
    if (req[req_len - 1] != '\n') {
        // gửi req + "\n"
        if (write_all(sockfd, req, req_len) != 0) return 1;
        if (write_all(sockfd, "\n", 1) != 0) return 1;
    } else {
        if (write_all(sockfd, req, req_len) != 0) return 1;
    }

    // đọc response
    // (đơn giản: đọc 1 lần đủ to; đồ án thường ok)
    int n = (int)read(sockfd, resp, resp_size - 1);
    if (n <= 0) {
        resp[0] = '\0';
        return 1;
    }
    resp[n] = '\0';

    // parse "code|payload"
    // code là số trước dấu |
    int code = 1;
    char *bar = strchr(resp, '|');
    if (!bar) {
        // không đúng format -> coi như lỗi, giữ nguyên resp
        return 1;
    }
    *bar = '\0';
    code = atoi(resp);

    // payload = phần sau |
    char *payload = bar + 1;

    // trim \r
    for (char *p = payload; *p; p++) {
        if (*p == '\r') *p = '\n';
    }

    // copy payload về đầu buffer
    memmove(resp, payload, strlen(payload) + 1);

    // trim 1 ký tự \n cuối (nếu có)
    int L = (int)strlen(resp);
    while (L > 0 && (resp[L-1] == '\n' || resp[L-1] == '\r')) {
        resp[L-1] = '\0';
        L--;
    }

    return code;
}
