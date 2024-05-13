#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "named_msg.cpp"
#include "read_file.cpp"

using namespace std;

const int AVG_ELEC_USE = 1500;
vector<vector<int>> fds = {};

struct
{
    vector<vector<int>> bills;
} all_bills;

void mkpipe(StrMsg msg)
{
    vector<string> add = msg.get_strings();
    add[0]= add[0].substr(0, add[0].find("."));

    char wrfifo[1024];
    strcpy(wrfifo, add[0].c_str());
    strcat(wrfifo, ".fifo");
    int rfd = open(wrfifo, O_WRONLY);
    add[0].append("acc.fifo");
    int wfd = open(add[0].c_str(), O_RDONLY);
    vector<int> fd;
    fd.push_back(rfd);
    fd.push_back(wfd);
    fds.push_back(fd);

    char connect[1024];
    strcpy(connect, "connected >>\0");
    strcat(connect, add[0].c_str());
    write(rfd, connect , 1024);
}

int most_use(NumMsg msg)
{
    int max =0, max_id = 0;
    for (int i = 0; i < msg.get_nums().size(); i++)
    {
        if (msg.get_nums()[i] > max)
        {
            max = msg.get_nums()[i];
            max_id = i;
        }
    }
    return max_id;
}

int month_use(NumMsg msg)
{
    int sum = 0;
    for (int n:msg.get_nums()) sum += n;
    return sum;
}

int avg_use(NumMsg msg)
{
    return month_use(msg) / 30;
}   

int water_bill(NumMsg msg)
{
    int ans = 0;
    for(int i=0; i < msg.get_nums().size()-1; i++)
    {
        ans += msg.get_nums()[i] * all_bills.bills[WATER_IDX][i];
        if (i == msg.get_nums()[most_use(msg)])
            ans += (msg.get_nums()[i] * all_bills.bills[WATER_IDX][i])/4;
    }
    return ans;
}

int gas_bill(NumMsg msg)
{
    int ans = 0;
    for(int i=0; i < msg.get_nums().size()-1; i++)
        ans += msg.get_nums()[i] * all_bills.bills[GAS_IDX][i];
    return ans;
}

int elec_bill(NumMsg msg)
{
    int ans = 0;
    for(int i=0; i < msg.get_nums().size()-1; i++)
    {
        ans += msg.get_nums()[i] * all_bills.bills[ELEC_IDX][i];
        if (i == msg.get_nums()[most_use(msg)])
        {
            if (msg.get_nums()[i] < AVG_ELEC_USE)
                ans -= (msg.get_nums()[i] * all_bills.bills[ELEC_IDX][i])/4;
            else
                ans += (msg.get_nums()[i] * all_bills.bills[ELEC_IDX][i])/4;
        }
    }
    return ans;
}

int discount(NumMsg msg)
{
    return (msg.get_nums()[most_use(msg)] - avg_use(msg));
}

void send_msg(int res, int fd, string type)
{
    vector<int> res_num (1,res);
    NumMsg msg(type, res_num);
    write(fd, msg.msg_str.c_str(), 1024);
}

int msg_hndlr(char msg_txt[1024], int fd)
{
    NumMsg msg(msg_txt);

    switch (msg.type)
    {
    case WATER_BILL:
        send_msg(water_bill(msg), fd, WATER_BILL_STR);
        break;
    case GAS_BILL:
        send_msg(gas_bill(msg), fd, GAS_BILL_STR);
        break;
    case ELEC_BILL:
        send_msg(elec_bill(msg), fd, ELEC_BILL_STR);
        break;
    case MONTH_USE:
        send_msg(month_use(msg), fd, MONTH_USE_STR);
        break;
    case AVG_MONTH_USE:
        send_msg(avg_use(msg), fd, AVG_MONTH_USE_STR);
        break;
    case MOST_USE:
        send_msg(most_use(msg), fd, MOST_USE_STR);
        break;
    case DIFFERENECE:
        send_msg(discount(msg), fd, DIFFERENECE_STR);
        break;
    case DONE:
        return 1;
    default:
        break;
    }
    return 0;
}

bool msg_new(string msg, vector<string> prev)
{
    int matched = 0;
    for (auto m: prev)
        if (m==msg) matched++;
    return matched == 1;
}

void get_all_buildings()
{
    int fd = open("build_to_acc.fifo", O_RDONLY);
    sleep(1);
    char msg[1024];
    vector<string> prev;
    do{
        StrMsg strmsg(msg);
        if (strmsg.type == FIFO_REQ)   
            mkpipe(strmsg);
        read(fd, &msg, 1024);
        prev.push_back(msg);
    } while(msg_new(msg, prev));

    close(fd);
}

void get_info()
{
    for (auto fd: fds)
    {
        int is_done = 0;
        while(!is_done){
            char buff [1024] = {};
            read(fd[1], buff, 1024);
            //cerr<<"build to acc :"<<buff<<fd[0]<<endl;
            is_done = msg_hndlr(buff, fd[0]);
        }
    }
}

void get_bills()
{
    Bill bill = get_bill();
    for (int i=0;i<3;i++) all_bills.bills.push_back(vector<int>(0,0));
    for(int i=0; i<bill.get_months_size();i++)
    {
        all_bills.bills[WATER_IDX].push_back(bill.monthly_usage(i)[WATER_IDX]);
        all_bills.bills[ELEC_IDX].push_back(bill.monthly_usage(i)[ELEC_IDX]);
        all_bills.bills[GAS_IDX].push_back(bill.monthly_usage(i)[GAS_IDX]);
    }
}

int main()
{
    cerr<<"--- account started\n";

    get_all_buildings();
    get_bills();
    get_info();

    sleep(1);
    cerr<<"--- account done!\n";
}
