#include <termio.h>

#include "lib.h"
#include "file.h"
#include "msg_parse.h"


void udp_command(char *cmd);

enum Result
{
    ONGOING,
    TIME_OUT,
    ACCEPTED,
    DENIED
};

struct Request
{
    int food_idx;
    int time;
    int sock;
    char *name;
    enum Result res;
};

struct
{
    char name[1024];
    int is_open;

    struct Tcp *tcp;
    struct Udp *udp;

    struct Ingred * ingreds;
    struct Food * foods;
    int food_count, ingred_count;

    struct Request requests[1024];
    int req_count;

}server;

int accept_client(int sock_fd)
{
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);

    client_fd = accept(sock_fd, (struct sockaddr *)&client_address, (socklen_t *)&address_len);
    print("new order!\n");
    return client_fd;
}

void setup_tcp(struct Tcp *tcp, int port)
{
    tcp->port = port;
    tcp->sock_fd = setup_server(port);
}

void check_reqs()
{
    for (int i=0; i<server.req_count;i++)
        if (server.requests[i].res == ONGOING && (time(NULL) - server.requests[i].time) > REQUEST_TIMEOUT)
            server.requests[i].res = TIME_OUT;
}

void add_req(int food_idx, int port, char *name)
{
    server.requests[server.req_count].food_idx = food_idx;
    server.requests[server.req_count].time = time(NULL);
    server.requests[server.req_count].name = name;
    server.requests[server.req_count].sock = port;
    server.requests[server.req_count].res = ONGOING;
    server.req_count++;
}

void tcp_command(char* msg, int sock)
{
    enum MsgType type = parse(msg);
    char response[1024];
    if (type != FOOD_REQ) return;
    char name[1024], food_idx[1024];

    int i = 0;
    for (i=0;i<strlen(msg);i++)
    {
        if (msg[i] != '#') name[i]=msg[i];
        else
        {
            name[i] = '\0';
            break;
        }
    }
    int j=++i;
    for (; j<strlen(msg);j++)
    {
        food_idx[j-i] = msg[j];
    }
    food_idx[j-i] = '\0';

    add_req(atoi(food_idx), sock, name);
}

void close_rest()
{
    char response[1024];
    char msg[1024];
    if (!server.is_open)
    {
        print("already closed!\n");
        return;
    }
    server.is_open = 0;

    sprintf(msg, "%s restaurant closed!", server.name);
    format(MSG_STR, msg, response);
    send_udp(server.udp, response);
}

void open_rest()
{
    char response[1024];
    char msg[1024];
    if (server.is_open)
    {
        print("already open!\n");
        return;
    }
    server.is_open = 1;

    sprintf(msg, "%s restaurant opened!", server.name);
    format(MSG_STR, msg, response);
    send_udp(server.udp, response);
}

void show_ingreds()
{
    print("remaining ingredients:\n");

    int printed_count=0;
    for (int i=0;i<server.ingred_count;i++)
    {
        if (!server.ingreds[i].value) continue;

        char print_line[1024];
        sprintf(print_line, "%d. %s:\tremaining:%d\n", ++printed_count, 
                server.ingreds[i].name, server.ingreds[i].value);
        
        print(print_line);
    }
    print("----------------------------------\n");
}

void show_recipes()
{
    print("\nrecipes:\n");

    for (int i=0;i<server.food_count;i++)
    {
        char print_line[1024];
        sprintf(print_line, "%d. %s:\n", i+1, server.foods[i].name);
        print(print_line);
        for (int j=0; j<server.foods[i].ingred_count;j++)
        {
            sprintf(print_line, "\t%s:%d\n", server.foods[i].ingreds[j].name,
                     server.foods[i].ingreds[j].value);
            print(print_line);
        }
    }
    print("----------------------------------\n");
}

void show_suppliers()
{
    print("active suppliers:\n");

    char supps[100][2][1000];
    int count = get_info(supps, SUPPLIER, server.udp);

    for (int i=0;i<count;i++)
    {
        char print_line[1024];
        sprintf(print_line, "%s: %s\n", supps[i][0], supps[i][1]);
        print(print_line);
    }
    print("----------------------------------\n");
}

int get_supp_port(char supp_name[1024])
{
    print("port of supplier");
    char port_str[1024];
    get_user_inp(port_str);
    char supps[100][2][1000] = {0};

    int count = get_info(supps, SUPPLIER, server.udp);
    for (int i=0;i<count;i++)
        if (str_eq(supps[i][1], port_str))
        {
            int port = atoi(supps[i][1]);
            strcpy(supp_name, supps[i][0]);
            return port;
        }
    return -1 ;
}

int ingred_idx_by_name(char *name)
{
    for (int i=0; i<server.ingred_count; i++)
        if (str_eq(server.ingreds[i].name, name))
            return i;
    
    return -1;
}

int get_ingred_idx()
{
    print("name of ingredient");
    char ingred_str[1024];
    get_user_inp(ingred_str);

    return ingred_idx_by_name(ingred_str);
}

int recv_supp_response(struct Tcp * tcp)
{
    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    FD_SET(tcp->sock_fd, &master_set);
    FD_SET(server.udp->sock, &master_set);

    int max_sd = (tcp->sock_fd > server.udp->sock) ? tcp->sock_fd : server.udp->sock;
    int begin_time = time(NULL);
    int user_buff = 0;

    while (time(NULL)-begin_time < MAX_SUPP_WAIT)
    {
        working_set = master_set;
        struct timeval wait;
        wait.tv_sec = MAX_SUPP_WAIT;
        wait.tv_usec = 0;
        select(max_sd + 1, &working_set, NULL, NULL, &wait);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                if (i == server.udp->sock)
                {
                    char msg[1024];
                    recv(server.udp->sock, msg, 1024,0);
                    udp_command(msg);
                    memset(msg, 0, 1024);
                    continue;
                }
                else if (i == tcp->sock_fd)
                {
                    char supp_response[1024] = {};
                    recv(i, supp_response, 1024, 0);
                    return str_eq(supp_response, ACCEPT_STR);
                }
            }
        }
    }
    return 0;
}

void req_ingreds()
{
    char supp_name[1024]={0};
    int port = get_supp_port(supp_name);
    if (port<0)
    {
        perror("invalid port!\n");
        return;
    }

    int ingred_idx = get_ingred_idx();
    if (ingred_idx<0)
    {
        perror("invalid ingredient name!\n");
        return;
    }

    char num_str[100];
    int num;
    print("number of ingredient");
    get_user_inp(num_str);
    num = atoi(num_str);
    if(num<=0)
    {
        perror("invalid number!\n");
        return;
    }

    //requesting
    struct Tcp supplier;
    supplier.sock_fd = setup_client(port);
    if(supplier.sock_fd<0)
    {
        perror("supplier refused to connect!\n");
        return;
    }

    char request[1024];
    format(REQ_INGREDS_STR, "", request);
    send_tcp(&supplier, request);

    //recieve
    struct termios original, locked;
    tcgetattr(STDIN_FILENO, &original);
    locked = original;
    locked.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &locked);

    print("waiting for supplier...\n");
    
    if (recv_supp_response(&supplier))
    {
        char print_line[1024];
        sprintf(print_line, "%s supplier accepted request!\n", supp_name);
        print(print_line);
        server.ingreds[ingred_idx].value += num;
    }
    else
    {
        char print_line[1024];
        sprintf(print_line, "%s supplier denied request!\n", supp_name);
        print(print_line);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

void udp_command(char *cmd)
{
    cut_msg(cmd);
    enum MsgType type = parse(cmd);
    char response[1024]={0};

    switch(type)
    {
        case GET_NAME:
            if (!strlen(server.name)) break;

            format(NAME_STR, server.name, response);
            send_udp(server.udp, response);
            break;

        case ALL_NAME:
            format(GET_NAME_STR, "", response);
            send_udp(server.udp, response);
            break;

        case GET_INFO:
            if (!server.tcp->port) break;
            char res1[1024],res2[1024];
            sprintf(res1, "%s%c%d", server.name, PARSE_CHAR, server.tcp->port);
            format(RESTAURANT_STR, res1, res2);
            format(INFO_STR, res2, response);
            send_udp(server.udp, response);
            break;

        case ALL_INFO:
            format(GET_INFO_STR, "", response);
            send_udp(server.udp, response);
            break;

        default:
            return;

            
    }
}

char *res_to_str(enum Result res)
{
    switch(res)
    {
        case ACCEPTED: return ACCEPT_STR;
        case DENIED: return DENY_STR;
        case ONGOING: return ONGOING_STR;
        case TIME_OUT: return TIME_OUT_STR;
    }
}

void show_history()
{
    check_reqs();
    print("history:\n");
    for (int i = 0;i<server.req_count;i++)
    {
        char print_line[1024];
        sprintf(print_line, "%s %s: %s\n", server.requests[i].name, 
                server.foods[server.requests[i].food_idx].name, res_to_str(server.requests[i].res));
        print(print_line);
    }

    print("----------------------------------\n");
}

void show_reqs()
{
    check_reqs();
    print("requests:\n");
    for (int i = 0;i<server.req_count;i++)
    {
        if (server.requests[i].res != ONGOING) continue;

        char print_line[1024];
        sprintf(print_line, "%s %d %s\n", server.requests[i].name, server.requests[i].sock,
                server.foods[server.requests[i].food_idx].name);
        print(print_line);
    }

    print("----------------------------------\n");
}

int is_food_available(struct Food *food)
{
    for(int i=0;i<food->ingred_count;i++)
    {
        if (food->ingreds[i].value > server.ingreds[ingred_idx_by_name(food->ingreds[i].name)].value)
            return 0;
    }
    return 1;
}

void decrease_ingreds(struct Food *food)
{
    for(int i=0;i<food->ingred_count;i++)
    {
        server.ingreds[ingred_idx_by_name(food->ingreds[i].name)].value -=food->ingreds[i].value;
    }
}

int req_idx(int sock)
{
    for (int i=0;i<server.req_count;i++)
        if (server.requests[i].sock == sock) 
            return i;

    return -1;
}

void ans_req()
{
    print("enter port of request");
    char port_str[1024];
    get_user_inp(port_str);
    int idx = req_idx(atoi(port_str));

    check_reqs();
    if (idx < 0 || server.requests[idx].res != ONGOING)
    {
        print("request not found or timed out!\n");
        return;
    }

    if (!is_food_available(&server.foods[server.requests->food_idx]))
    {
        print("not enough ingredients!\n");
        return;
    }

    print("please type your answer (Yes/No):");
    char ans[1024];
    get_user_inp(ans);
    if (str_eq(ans, "Yes"))
    {
        send(atoi(port_str), ACCEPT_STR, strlen(ACCEPT_STR), 0);
        server.requests[idx].res = ACCEPTED;
        decrease_ingreds(&server.foods[server.requests->food_idx]);
    }
    else
    {
        send(atoi(port_str), DENY_STR, strlen(DENY_STR), 0);
        server.requests[idx].res = DENIED;
    }
}

void user_command(const char *cmd)
{
    enum UMsgType type = str_to_umsg(cmd);

    switch (type)
    {
        case OPEN:
            open_rest();
            break;

        case CLOSE:
            close_rest();
            break;

        case SHOW_INGRED:
            show_ingreds();
            break;

        case SHOW_RECIPES:
            show_recipes();
            break;

        case SHOW_SUPPS:
            show_suppliers();
            break;

        case REQ_INGREDS:
            req_ingreds();
            break;

        case SHOW_HISTORY:
            show_history();
            break;

        case SHOW_REQS:
            show_reqs();
            break;

        case ANS_REQ:
            ans_req();
            break;

        default:
            break;

    }
}

void run_resturant()
{
    char msg[1024];
    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    FD_SET(server.tcp->sock_fd, &master_set);
    FD_SET(server.udp->sock, &master_set);
    FD_SET(STDIN_FILENO, &master_set);

    int max_sd = (server.tcp->sock_fd > server.udp->sock) ? server.tcp->sock_fd : server.udp->sock;

    while (1)
    {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {

                if (i == server.udp->sock)
                {
                    recv(server.udp->sock, msg, 1024,0);
                    udp_command(msg);
                    memset(msg, 0, 1024);
                    continue;
                }
                else if (i == STDIN_FILENO || !server.is_open)
                {
                    if (i!=STDIN_FILENO) continue;

                    read(i, msg, 1024);
                    cut_msg(msg); // because I can't use fflush
                    user_command(msg);
                    memset(msg, 0, 1024);
                    continue;
                }
                else if (i == server.tcp->sock_fd)
                { 
                    int new_socket = accept_client(server.tcp->sock_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                }    
                else
                {
                    memset(msg, 0, 1024);
                    int bytes_received;
                    bytes_received = recv(i, msg, 1024, 0);

                    if (bytes_received == 0)
                    {
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
                    else
                    {
                        tcp_command(msg, i);
                    }
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
    server.foods = (struct Food *) malloc(100 * sizeof(struct Food));
    server.ingreds = (struct Ingred *) malloc(200 * sizeof(struct Ingred));

    server.tcp = &tcp;
    server.udp = &udp;
    server.is_open = 1;
    strcpy(server.name, "");
    server.tcp->port = 0;
    server.udp->port =  atoi(argv[argc-1]);
    server.req_count = 0;

    setup_udp(server.udp);
    set_name(server.name, server.udp);

    char print_line[1024];
    sprintf(print_line,"Wellcome %s as restaurant!\n", server.name);
    print(print_line);
    
    setup_tcp(server.tcp, get_tcp_port(server.udp));

    server.food_count = read_file(server.foods);
    server.ingred_count = get_all_ingreds(server.foods, server.ingreds, server.food_count);

    run_resturant();

    return 0;
}