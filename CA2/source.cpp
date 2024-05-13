#include <string>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "interface.cpp"
using namespace std;

vector<string> get_buildings()
{
    vector<string> files;
    vector<string> dirs;
    string command = "ls ";
    
    FILE *pipe = popen(command.c_str(), "r");

    char str[1024], old_str[1024];
    do{
        files.push_back(str);
        strcpy(old_str, str);
        fscanf(pipe, "%s", str);
    } while(strcmp(str, old_str));

    for (auto file: files)
    {
        struct stat s;
        if(stat(file.c_str(),&s) == 0 )
        if( s.st_mode & S_IFDIR )
            dirs.push_back(file);
    }
    return dirs;
}

int make_building(const char address[1024], const char msg[1024])
{
    int pipefd[2];
    if (pipe(pipefd)<0)
    {
        cerr<<"pipe failed!\n";
        return -1;
    }

    pid_t fork_pid = fork();
    if (fork_pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], 1);
        execl("./building.o","./building.o", address, msg);
    }
    close(pipefd[1]);
    return pipefd[0];
}

vector<int> make_buildings(vector<string> adds, vector<StrMsg> msgs)
{
    vector<int> child_pids;
    int count=0;
    for (auto dir_add: adds)
        child_pids.push_back(make_building(dir_add.c_str(), msgs[count++].msg_str.c_str()));
    
    return child_pids;
}

void make_account(string add)
{
    mkfifo("build_to_acc.fifo", 0666);
    if (fork() == 0) // named pipe
        execl("./account.o","./account.o", add.c_str());
}

int main(int argc, char ** argv)
{
    vector<string> dirs = get_buildings();
    vector<StrMsg> msgs {get_all_works(dirs)};

    make_account("bills.csv");
    vector<int> build_fds = make_buildings(dirs, msgs);

    sleep(1);
    get_report(build_fds, dirs);

    int pid=0;
    while(pid!=-1)
        pid = wait(&pid);

    for (auto fd: build_fds)
        close(fd);

    cout<<"done"<<endl;
}