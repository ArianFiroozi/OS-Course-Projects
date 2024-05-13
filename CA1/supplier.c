#include "lib.h"
#include "file.h"
#include "msg_parse.h"

struct
{
    char name[1024];
    struct Tcp *tcp;
    struct Udp *udp;
    int req_time;
    int ans_port;

}supplier;

int accept_client(int sock_fd)
{
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);

    client_fd = accept(sock_fd, (struct sockaddr *)&client_address, (socklen_t *)&address_len);
    print("new request!\n");
    return client_fd;
}

void setup_tcp(struct Tcp *tcp, int port)
{
    tcp->port = port;
    tcp->sock_fd = setup_server(port);
}

void udp_command(char *cmd)
{
    cut_msg(cmd);
    enum MsgType type = parse(cmd);
    char response[1024]={0};

    switch(type)
    {
        case GET_NAME:
            if (!strlen(supplier.name)) break;

            format(NAME_STR, supplier.name, response);
            send_udp(supplier.udp, response);
            break;

        case ALL_NAME:
            format(GET_NAME_STR, "", response);
            send_udp(supplier.udp, response);
            break;

        case GET_INFO:
            if (!supplier.tcp->port) break;
            char res1[1024],res2[1024];
            sprintf(res1, "%s%c%d", supplier.name, PARSE_CHAR, supplier.tcp->port);
            format(SUPPLIER_STR, res1, res2);
            format(INFO_STR, res2, response);
            send_udp(supplier.udp, response);
            break;

        case ALL_INFO:
            format(GET_INFO_STR, "", response);
            send_udp(supplier.udp, response);
            break;

        default:
            return;

            
    }
}

void add_req(char *msg, int i)
{
    if (parse(msg)!=REQ) return;
    supplier.req_time = time(NULL);
    supplier.ans_port = i;
}

void answer_request()
{
    if (time(NULL)- supplier.req_time > MAX_SUPP_WAIT)
    {
        perror("no active requests!\n");
        return;
    }

    print("please type your answer (Yes/No):");
    char ans[1024];
    get_user_inp(ans);
    if (str_eq(ans, "Yes"))
    {
        send(supplier.ans_port, ACCEPT_STR, strlen(ACCEPT_STR), 0);
    }
    else
    {
        send(supplier.ans_port, DENY_STR, strlen(DENY_STR), 0);
    }
    supplier.req_time = 0;
}

void user_command(const char *cmd)
{
    enum UMsgType type = str_to_umsg(cmd);

    switch (type)
    {
        case ANS_REQ:
            answer_request();
            break;
        default:
            break;
    }
}

void run_supplier()
{
    char msg[1024];
    fd_set master_set, working_set;

    FD_ZERO(&master_set);
    FD_ZERO(&working_set);
    FD_SET(supplier.tcp->sock_fd, &master_set);
    FD_SET(supplier.udp->sock, &master_set);
    FD_SET(STDIN_FILENO, &master_set);

    int max_sd = (supplier.tcp->sock_fd > supplier.udp->sock) ? supplier.tcp->sock_fd : supplier.udp->sock;

    while (1)
    {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {

                if (i == supplier.udp->sock)
                {
                    recv(supplier.udp->sock, msg, 1024,0);
                    udp_command(msg);
                    memset(msg, 0, 1024);
                    continue;
                }
                else if (i == STDIN_FILENO)
                {
                    if (i!=STDIN_FILENO) continue;

                    read(i, msg, 1024);
                    cut_msg(msg); // because I can't use fflush
                    user_command(msg);
                    memset(msg, 0, 1024);
                    continue;
                }
                else if (i == supplier.tcp->sock_fd)
                {
                    if (time(NULL) - supplier.req_time < MAX_SUPP_WAIT) continue;

                    int new_socket = accept_client(supplier.tcp->sock_fd);
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
                        add_req(msg, i);
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

    supplier.tcp = &tcp;
    supplier.udp = &udp;
    supplier.udp->port = atoi(argv[argc-1]);
    supplier.tcp->port = 0;
    supplier.req_time = 0;
    
    setup_udp(supplier.udp);
    set_name(supplier.name, supplier.udp);
    
    char print_line[1024];
    sprintf(print_line,"Wellcome %s as supplier!\n", supplier.name);
    print(print_line);

    setup_tcp(supplier.tcp, get_tcp_port(supplier.udp));

    run_supplier();

    return 0;
}