#ifndef NET_H
#define NET_H

#include <stddef.h>

// Thread-safe request/response over a single TCP socket.
// Returns 0 on success (server code==0), non-zero otherwise.
int net_request(int sockfd, const char *line, char *out_payload, size_t out_sz);

#endif
