#include "net.h"
#include "../common.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static pthread_mutex_t g_net_lock = PTHREAD_MUTEX_INITIALIZER;

// Server format: "<code>|<payload>\n"
static int parse_server_line(const char *line, char *out_payload, size_t out_sz) {
    if (!line || !*line) {
        if (out_payload && out_sz) out_payload[0] = '\0';
        return 1;
    }

    int code = line[0] - '0';
    const char *p = strchr(line, '|');
    if (!p) {
        if (out_payload && out_sz) {
            strncpy(out_payload, line, out_sz - 1);
            out_payload[out_sz - 1] = '\0';
        }
        return code;
    }

    p++; // after '|'
    if (out_payload && out_sz) {
        strncpy(out_payload, p, out_sz - 1);
        out_payload[out_sz - 1] = '\0';
        // trim trailing newlines
        size_t len = strlen(out_payload);
        while (len > 0 && (out_payload[len-1] == '\n' || out_payload[len-1] == '\r')) {
            out_payload[len-1] = '\0';
            len--;
        }
    }
    return code;
}

int net_request(int sockfd, const char *line, char *out_payload, size_t out_sz) {
    if (!line) return 1;

    pthread_mutex_lock(&g_net_lock);

    // send
    if (send(sockfd, line, strlen(line), 0) <= 0) {
        pthread_mutex_unlock(&g_net_lock);
        return 1;
    }

    // recv (single response)
    char buf[BUF_SIZE];
    int n = recv(sockfd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        pthread_mutex_unlock(&g_net_lock);
        return 1;
    }
    buf[n] = '\0';

    int code = parse_server_line(buf, out_payload, out_sz);

    pthread_mutex_unlock(&g_net_lock);
    return code;
}
