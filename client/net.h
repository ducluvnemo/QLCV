#ifndef NET_H
#define NET_H

int net_connect_localhost(int port);
int net_request(int sockfd, const char *req, char *resp, int resp_size);

#endif
