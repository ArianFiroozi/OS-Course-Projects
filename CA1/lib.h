#ifndef LIB_H
#define LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#include "consts.h"
#include "msg_parse.h"

struct Udp
{
  int sock;
  int port;
  struct sockaddr_in bc_address;
};

struct Tcp
{
    int port;
    int sock_fd;
};

void setup_udp(struct Udp *udp);
int send_udp(struct Udp *udp, char * buffer);
int setup_server(int port);
int setup_client(int port);
int connect_server(int port);
int send_tcp(struct Tcp *tcp, char * buffer);
void set_name(char *new_name, struct Udp * udp);
int get_info(char users[100][2][1000], enum UserType user_type, struct Udp * udp);
int get_tcp_port(struct Udp* udp);

#include "lib.c"
#endif