#include <termio.h>
#include "lib.h"
#include "file.h"
#include "msg_parse.h"

struct
{
    char name[1024];
    struct Tcp* tcp;
    struct Udp* udp;
    char msg[1024];
    int food_count;
    char food_names[1024][1024];
}client;

void setup_tcp(struct Tcp * tcp, int port)
{
    tcp->sock_fd = setup_client(port);
    tcp->port = port; 
}

void udp_command(char *cmd)
{
    cut_msg(cmd);
    enum MsgType type = parse(cmd);
    char response[1024]={0};

    switch(type)
    {
        case GET_NAME:
            if (!strlen(client.name)) break;

            format(NAME_STR, client.name, response);
            send_udp(client.udp, response);
            break;

        case ALL_NAME:
            format(GET_NAME_STR, "", response);
            send_udp(client.udp, response);
            break;

        case GET_INFO:
            if (!client.tcp->port) break;
            char res1[1024],res2[1024];
            sprintf(res1, "%s%c%d", client.name, PARSE_CHAR, client.tcp->port);
            format(CLIENT_STR, res1, res2);
            format(INFO_STR, res2, response);
            send_udp(client.udp, response);
            break;

        case ALL_INFO:
            format(GET_INFO_STR, "", response);
            send_udp(client.udp, response);
            break;

        default:
            return;

            
    }
}

int get_food_idx()
{
    print("name of food");
    char food_str[1024];
    get_user_inp(food_str);

    for (int i=0; i<client.food_count; i++)
        if (str_eq(client.food_names[i], food_str))
            return i;
    
    return -1;
}

int get_rest_port(char rest_name[1024])
{
    print("port of restaurant");
    char port_str[1024];
    get_user_inp(port_str);
    char rests[100][2][1000] = {0};

    int count = get_info(rests, RESTAURANT, client.udp);
    for (int i=0;i<count;i++)
        if (str_eq(rests[i][1], port_str))
        {
            int port = atoi(rests[i][1]);
            strcpy(rest_name, rests[i][0]);
            return port;
        }
    return -1 ;
}

int recv_rest_response(struct Tcp * tcp)
{
    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    FD_SET(tcp->sock_fd, &master_set);
    FD_SET(client.udp->sock, &master_set);

    int max_sd = (tcp->sock_fd > client.udp->sock) ? tcp->sock_fd : client.udp->sock;
    int begin_time = time(NULL);
    int user_buff = 0;

    while (time(NULL)-begin_time < REQUEST_TIMEOUT)
    {
        working_set = master_set;
        struct timeval wait;
        wait.tv_sec = REQUEST_TIMEOUT;
        wait.tv_usec = 0;
        select(max_sd + 1, &working_set, NULL, NULL, &wait);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                if (i == client.udp->sock)
                {
                    char msg[1024];
                    recv(client.udp->sock, msg, 1024,0);
                    udp_command(msg);
                    memset(msg, 0, 1024);
                    continue;
                }
                else if (i == tcp->sock_fd)
                {
                    char rest_response[1024] = {};
                    recv(i, rest_response, 1024, 0);
                    return str_eq(rest_response, ACCEPT_STR);
                }
            }
        }
    }
    return 0;
}

void order()
{
    int food_idx = get_food_idx();
    if (food_idx<0)
    {
        perror("invalid food name!\n");
        return;
    }

    char rest_name[1024]={0};
    int port = get_rest_port(rest_name);
    if (port<0)
    {
        perror("invalid port!\n");
        return;
    }

    //requesting
    struct Tcp restaurant;
    restaurant.sock_fd = setup_client(port);
    if(restaurant.sock_fd<0)
    {
        perror("restaurant refused to connect!\n");
        return;
    }

    char request[1024];
    char req[1024];
    sprintf(req, "%s%c%d", client.name, PARSE_CHAR, food_idx);
    format(FOOD_REQ_STR, req, request);

    send_tcp(&restaurant, request);

    //recieve
    struct termios original, locked;
    tcgetattr(STDIN_FILENO, &original);
    locked = original;
    locked.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &locked);

    print("waiting for restaurant...\n");
    
    if (recv_rest_response(&restaurant))
    {
        char print_line[1024];
        sprintf(print_line, "%s restaurant accepted request!\n", rest_name);
        print(print_line);
    }
    else
    {
        char print_line[1024];
        sprintf(print_line, "%s restaurant denied request!\n", rest_name);
        print(print_line);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

void show_restaurants()
{
    print("active restaurants:\n");

    char rests[100][2][1000];
    int count = get_info(rests, RESTAURANT, client.udp);

    for (int i=0;i<count;i++)
    {
        char print_line[1024];
        sprintf(print_line, "%s: %s\n", rests[i][0], rests[i][1]);
        print(print_line);
    }
    print("----------------------------------\n");
}

void show_foods()
{
    print("foods:\n");

    int printed_count=0;
    for (int i=0;i<client.food_count;i++)
    {
        char print_line[1024];
        sprintf(print_line, "%d. %s\n", ++printed_count,  client.food_names[i]);
        print(print_line);
    }
    print("----------------------------------\n");
}

void user_command(const char *cmd)
{
    enum UMsgType type = str_to_umsg(cmd);

    switch (type)
    {
        case SHOW_RESTAURANTS:
            show_restaurants();
            break;

        case SHOW_FOODS:
            show_foods();
            break;

        case ORDER:
            order();
            break;

        default:
            break;

    }
}

void run_client()
{
    char msg[1024];
    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    FD_SET(client.udp->sock, &master_set);
    FD_SET(STDIN_FILENO, &master_set);

    int max_sd = client.udp->sock;

    while (1)
    {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {

                if (i == client.udp->sock)
                {
                    recv(client.udp->sock, msg, 1024,0);
                    udp_command(msg);
                    memset(msg, 0, 1024);
                    continue;
                }
                else if (i == STDIN_FILENO)
                {
                    if (i != STDIN_FILENO) continue;

                    read(i, msg, 1024);
                    cut_msg(msg);
                    user_command(msg);
                    memset(msg, 0, 1024);
                    continue;
                } 
            }
        }
    }
}

int main(int argc, char const *argv[]) 
{
    if (argc < 2) exit(-1);

    struct Udp udp;
    struct Tcp tcp;

    client.udp = &udp;
    client.tcp = &tcp;

    client.udp->port = atoi(argv[argc-1]);
    
    setup_udp(client.udp);
    set_name(client.name, client.udp);
    
    char print_line[1024];
    sprintf(print_line,"Wellcome %s as client!\n", client.name);
    print(print_line);

    client.food_count = 0;

    struct Food foods[1024];
    client.food_count = read_file(foods);
    get_all_foods(foods, client.food_names, client.food_count);

    run_client();

    close(client.tcp->sock_fd);

    return 0;
}