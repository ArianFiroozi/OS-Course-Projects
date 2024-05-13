#include "lib.h"

void setup_udp(struct Udp *udp)
{
  int broadcast = 1, opt = 1;

  udp->sock = socket(AF_INET, SOCK_DGRAM, 0);
  setsockopt(udp->sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
  setsockopt(udp->sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

  udp->bc_address.sin_family = AF_INET; 
  udp->bc_address.sin_port = htons(udp->port); 
  udp->bc_address.sin_addr.s_addr = inet_addr("10.0.2.255");

  if (bind(udp->sock, (struct sockaddr *)&udp->bc_address, sizeof(udp->bc_address))<0)
  {
    perror("couldn't bind\n");
    exit(0);
  }

}

int send_udp(struct Udp *udp, char * buffer)
{
  return sendto(udp->sock, buffer, strlen(buffer), 0,(struct sockaddr *)&udp->bc_address, sizeof(udp->bc_address));
}

int setup_server(int port)
{
    struct sockaddr_in address;
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(sock_fd, (struct sockaddr *)&address, sizeof(address));

    listen(sock_fd, 4);

    return sock_fd;
}

int setup_client(int port)
{
    int sock_fd = connect_server(port);
    if (sock_fd < 0)
    {
        perror("can't connect the server\n");
    }

    return sock_fd;
}

int connect_server(int port) {
    int fd;
    struct sockaddr_in server_add;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_add.sin_family = AF_INET; 
    server_add.sin_port = htons(port); 
    server_add.sin_addr.s_addr = inet_addr(SYSTEM_UDP_PORT);

    if (connect(fd, (struct sockaddr *)&server_add, sizeof(server_add)) < 0) 
    {
        perror("Error in connecting to server\n");
        return -1;
    }

    return fd;
}

int send_tcp(struct Tcp *tcp, char * buffer)
{
    int i = send(tcp->sock_fd, buffer, strlen(buffer), 0);
    return i;
}

void set_name(char *new_name, struct Udp * udp)
{
    print("Enter your name:\n");
    char name[1024];

    get_user_inp(name);

    char all_names[100][100];
    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    FD_SET(udp->sock, &master_set);

    int max_sd = udp->sock;
    int all_recieved = 0;

    char udp_cmd[1024];
    format(ALL_NAME_STR, "", udp_cmd);

    char response[1024];
    format(GET_NAME_STR, "", response);
    send_udp(udp, response);

    int name_buff = 0;

    while (!all_recieved)
    {
        all_recieved = 1;

        working_set = master_set;
        struct timeval wait;
        wait.tv_sec = 0;
        wait.tv_usec = 1000;
        select(max_sd + 1, &working_set, NULL, NULL, &wait);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                if (i == udp->sock)
                {
                    char msg[100] = {0};

                    int bytes = recv(udp->sock, msg, 1024,0);
                    if (bytes<=0) break;
                    cut_msg(msg);

                    enum MsgType type = parse(msg);
                    all_recieved = 0;

                    if (type == NAME) strcpy(all_names[name_buff++], msg);
                    continue;
                }
            }
        }
    }

    while(is_name_used(name, all_names, name_buff))
    {
        print("Name already in use. Enter another name:\n");
        get_user_inp(name);
    }
    strcpy(new_name, name);
}

int get_info(char users[100][2][1000], enum UserType user_type, struct Udp * udp)
{
    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    FD_SET(udp->sock, &master_set);

    int max_sd = udp->sock;
    int all_recieved = 0;

    char response[1024];
    format(GET_INFO_STR, "", response);
    send_udp(udp, response);

    int user_buff = 0;

    while (!all_recieved)
    {
        all_recieved = 1;

        working_set = master_set;
        struct timeval wait;
        wait.tv_sec = 0;
        wait.tv_usec = 1000;
        select(max_sd + 1, &working_set, NULL, NULL, &wait);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                if (i == udp->sock)
                {
                    char new_user[1024] = {0};
                    int bytes = recv(udp->sock, new_user, 1024,0);
                    if (bytes<=0) break;
                    cut_msg(new_user);

                    enum MsgType type = parse(new_user);
                    all_recieved = 0;

                    if (type == INFO)
                    {
                        enum UserType new_type = parse_usertype(new_user);
                        if (new_type==user_type)
                        {
                          char name[1024];
                          for (int i=0; i<strlen(new_user); i++)
                          {
                            if (new_user[i] == PARSE_CHAR)
                            {
                              name[i] = '\0'; 
                              strsl(new_user, i+1);
                              break;
                            }
                            name[i] = new_user[i];
                          }

                          strcpy(users[user_buff][0], name); // name
                          strcpy(users[user_buff++][1], new_user); //port
                        }

                    }
                    continue;
                }
            }
        }
    }

    return user_buff;
}

int get_tcp_port(struct Udp* udp)
{
  char rests[100][2][1000], supps[100][2][1000], clients[100][2][1000];
  int r_buff, s_buff, c_buff;
  int max_port = udp->port;

  r_buff = get_info(rests, RESTAURANT, udp);
  s_buff = get_info(supps, SUPPLIER, udp);
  c_buff = get_info(clients, CLIENT, udp);

  for (int i=0;i<r_buff;i++) if (max_port < atoi(rests[i][1])) max_port=atoi(rests[i][1]);
  for (int i=0;i<s_buff;i++) if (max_port < atoi(supps[i][1])) max_port= atoi(supps[i][1]);
  for (int i=0;i<c_buff;i++) if (max_port < atoi(clients[i][1])) max_port= atoi(clients[i][1]);

  return max_port + 1;
}
