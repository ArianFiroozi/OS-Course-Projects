#include "read_file.cpp"

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "named_msg.cpp"

using namespace std;

typedef vector<vector<int>> bill_vector;

const int BILL_COUNT = 3;

int rdfd=-1, wrfd=-1;

struct
{
    vector<vector<vector<int>>> bills;
    vector<int> max_hours;
} all_bills;

vector<int> get_month_bills(string line, int pos)
{
    vector<int> bills;
    while(pos!=string::npos)
    {
        int new_pos = line.find("#",pos);
        if (new_pos<0) break;
        bills.push_back(stoi(line.substr(pos, new_pos-pos)));
        pos = new_pos+1;
    }
    return bills;
}

void parse(string line)
{
    bill_vector bill;
    int i=0,j=0;
    int pos = 0;

    while(pos>=0)
    {
        int new_pos = line.find("%",pos);
        if (new_pos<0) break;
        string new_line = line.substr(pos, new_pos - pos);
        bill.push_back(get_month_bills(new_line, 0));
        pos = new_pos+1;
    }
    int max_hour = stoi(line.substr(line.find("|")+1));

    all_bills.bills.push_back(bill);
    all_bills.max_hours.push_back(max_hour);
}

int make_bill(const char address[1024])
{
    int pipefd[2];
    if (pipe(pipefd)<0)
    {
        cerr<<"pipe faild!\n";
        return -1;
    }

    pid_t fork_pid = fork();
    if (fork_pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], 1);
        execl("./bill.o","./build.o", address);
    }
    close(pipefd[1]);
    return pipefd[0];
}

void get_all_bills(char* path)
{
    vector<pid_t> child_pids;
    vector<const char*> files {"Gas.csv", "Electricity.csv", "Water.csv"};
    for (auto file_add: files)
    {
        char dest[1024] = {};
        strcat(dest, "./");
        strcat(dest, path);
        strcat(dest, "/");
        strcat(dest, file_add);
        child_pids.push_back(make_bill(dest));
    }
    
    for (auto pipefd: child_pids)
    {
        char msg[1024];
        read(pipefd, &msg, 1024);
        //cout<<"parent read:"<<msg<<endl;
        parse(msg);
        close(pipefd);
    }
}

void connect_to_acc(char argc[1024])
{
    int accfd = open("build_to_acc.fifo", O_WRONLY);
    char rdmsg[1024];

    char fifo_name[1024], wrfifo_name[1024];
    strcpy(fifo_name, argc);
    strcat(fifo_name, ".fifo");
    mkfifo(fifo_name, 0666);

    strcpy(wrfifo_name, argc);
    strcat(wrfifo_name, "acc.fifo");
    mkfifo(wrfifo_name, 0666);

    vector<string> msg_str;
    msg_str.push_back(fifo_name);
    StrMsg msg(FIFO_REQ_STR, msg_str);
    write(accfd, msg.msg_str.c_str(), 1024);

    rdfd = open(fifo_name, O_RDONLY);
    wrfd = open(wrfifo_name, O_WRONLY);

    close(accfd);
}

int get_response()
{
    char acc_res[1024] = {};
    read(rdfd, &acc_res, 1024);
    NumMsg accres(acc_res);
    return accres.get_nums()[0];
}

void send_response(vector<int> ans, string res_type)
{
    NumMsg src_res(res_type, ans);
    write(1, src_res.msg_str.c_str(), 1024);
}

void send_done()
{
    vector<int>a ={};
    NumMsg accres(DONE_STR, a);
    write(wrfd, accres.msg_str.c_str(), 1024);
}

void done_res()
{
    vector<int>a ={};
    send_response(a, DONE_STR);
}

vector<int> water_bill()
{
    vector<int> water_bill = {};
    for(auto month_bill: all_bills.bills[WATER_IDX])
    {
        month_bill.push_back(all_bills.max_hours[WATER_IDX]);
        NumMsg accmsg(WATER_BILL_STR, month_bill);
        write(wrfd, accmsg.msg_str.c_str(), 1024);
        water_bill.push_back(get_response());
    }
    return water_bill;
}

vector<int> elec_bill()
{
    vector<int> elec_bill = {};
    for(auto month_bill: all_bills.bills[ELEC_IDX])
    {
        month_bill.push_back(all_bills.max_hours[ELEC_IDX]);
        NumMsg accmsg(ELEC_BILL_STR, month_bill);
        write(wrfd, accmsg.msg_str.c_str(), 1024);
        elec_bill.push_back(get_response());
    }
    return elec_bill;
}

vector<int> gas_bill()
{
    vector<int> gas_bill = {};
    for(auto month_bill: all_bills.bills[GAS_IDX])
    {
        month_bill.push_back(all_bills.max_hours[GAS_IDX]);
        NumMsg accmsg(GAS_BILL_STR, month_bill);
        write(wrfd, accmsg.msg_str.c_str(), 1024);
        gas_bill.push_back(get_response());
    }
    return gas_bill;
}

void month_use()
{
    for(auto bill:all_bills.bills)
    {
        vector<int> month_use;
        for(auto month_bill: bill)
        {
            NumMsg accmsg(MONTH_USE_STR, month_bill);
            write(wrfd, accmsg.msg_str.c_str(), 1024);
            month_use.push_back(get_response());
        }
        send_response(month_use, MONTH_USE_STR);
    }
}

void avg_use()
{
    for(auto bill:all_bills.bills)
    {
        vector<int> month_use;
        for(auto month_bill: bill)
        {
            NumMsg accmsg(AVG_MONTH_USE_STR, month_bill);
            write(wrfd, accmsg.msg_str.c_str(), 1024);
            month_use.push_back(get_response());
        }
        send_response(month_use, AVG_MONTH_USE_STR);
    }
}

void most_use()
{
    for(auto bill:all_bills.bills)
    {
        vector<int> month_use;
        for(auto month_bill: bill)
        {
            NumMsg accmsg(MOST_USE_STR, month_bill);
            write(wrfd, accmsg.msg_str.c_str(), 1024);
            month_use.push_back(get_response());
        }
        send_response(month_use, MOST_USE_STR);
    }
}

void discount()
{
    for(auto bill:all_bills.bills)
    {
        vector<int> discount;
        for(auto month_bill: bill)
        {
            NumMsg accmsg(DIFFERENECE_STR, month_bill);
            write(wrfd, accmsg.msg_str.c_str(), 1024);
            discount.push_back(get_response());
        }
        send_response(discount, DIFFERENECE_STR);
    }
}

void do_all()
{
    send_response(gas_bill(), GAS_BILL_STR);
    send_response(elec_bill(), ELEC_BILL_STR);
    send_response(water_bill(), WATER_BILL_STR);
    month_use();
    avg_use();
    most_use();
    discount();
}

void do_work(const char works[1024])
{
    StrMsg bill_type (works);
    for(auto work:bill_type.get_strings())
    {
        vector<int> a(0,0);
        NumMsg msg(work, a);
        switch (msg.type)
        {
        case WATER_BILL:
            send_response(water_bill(), WATER_BILL_STR);
            break;
        case GAS_BILL:
            send_response(gas_bill(), GAS_BILL_STR);
            break;
        case ELEC_BILL:
            send_response(elec_bill(), ELEC_BILL_STR);
            break;
        case MONTH_USE:
            month_use();
            break;
        case AVG_MONTH_USE:
            avg_use();
            break;
        case MOST_USE:
            most_use();
            break;
        case DIFFERENECE:
            discount();
            break;
        case ALL:
            do_all();
            break;
        default:
            break;
        }
    }
    send_done();
    done_res();
}

int main(int argv, char *argc[])
{
    if (argv < 3)
    {
        cerr<<"path not provided for building.cpp!\n";
        exit(1);
    }

    cerr << "--- building " << argc[1]<< " started!" <<endl;

    get_all_bills(argc[1]);
    cerr <<"--- building " << argc[1] << " got the bills"<<endl;

    connect_to_acc(argc[1]);
    char connect[1024] = {};
    read(rdfd, connect,1024);
    cerr <<"--- building " << argc[1] << " connected to account"<<endl;

    do_work(argc[2]);

    close(rdfd);
    close(wrfd);

    sleep(1);
    cerr<<"--- building "<<argc[1]<< " done!"<<endl;
    return 0;
}