#include <string>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <vector>

#include "named_msg.cpp"

using namespace std;

const vector<string> actions = {BILL_STR, AVG_MONTH_USE_STR, MONTH_USE_STR, MOST_USE_STR, BILL_STR, DIFFERENECE_STR, ALL_STR};

void print_body(NumMsg gmsg, NumMsg emsg, NumMsg wmsg)
{
    for (int i=0;i<gmsg.get_nums().size();i++)
    {
        cerr<< i+1 <<"\t\t"<<gmsg.get_nums()[i]<<"\t\t"<<emsg.get_nums()[i]<<"\t\t"<<wmsg.get_nums()[i]<<endl;
    }
    cerr<<endl;
}

void bill()
{
    cerr<<"Bill report:"<<endl;
    cerr<<"MONTH\t\t"<<GAS_BILL_STR<<"\t\t"<<ELEC_BILL_STR<<"\t\t"<<WATER_BILL_STR<<endl;
}

void month_use()
{
    cerr<<"Month use report:"<<endl;
    cerr<<"MONTH\t\t"<<"GAS_USE"<<"\t\t"<<"ELEC_USE"<<"\t"<<"WATER_USE"<<endl;
}

void avg_use()
{
    cerr<<"Avg use report:"<<endl;
    cerr<<"MONTH\t"<<"GAS_AVG_USE"<<"\t"<<"ELEC_AVG_USE"<<"\t"<<"WATER_AVG_USE"<<endl;
}

void most_use()
{
    cerr<<"Most use report:"<<endl;
    cerr<<"MONTH\t"<<"GAS_MOST_USE"<<"\t"<<"MOST_USE"<<"\t"<<"WATER_MOST_USE"<<endl;
}

void discount()
{
    cerr<<"Differenece report:"<<endl;
    cerr<<"MONTH\t"<<"GAS_DIFFERNECE"<<"\t"<<"ELEC_DIFFERNECE"<<"\t"<<"WATER_DIFFERNECE"<<endl;
}

void switch_msg(NumMsg gmsg, NumMsg emsg, NumMsg wmsg)
{
    if(gmsg.type==MONTH_USE) month_use();
    if(gmsg.type==GAS_BILL) bill();
    if(gmsg.type==AVG_MONTH_USE) avg_use();
    if(gmsg.type==MOST_USE) most_use();
    if(gmsg.type==DIFFERENECE) discount();
    print_body(gmsg, emsg, wmsg);
}

void get_report(vector<int> build_fds, vector<string> dirs)
{
    for (int i = 0; i< build_fds.size(); i++)
    {
        int fd = build_fds[i];
        int is_done = 0;
        int rep_printed = 0;
        while(!is_done){
            char msg[1024]={};
            read(fd, &msg, 1024);
            if (!rep_printed) 
            {
                cout<<endl<<"******Report for: "<<dirs[i]<<"******"<<endl;
                rep_printed = 1;
            }

            NumMsg gmsg(msg);
            if (gmsg.type != DONE)
            {
                read(fd, &msg, 1024);
                NumMsg emsg(msg);
                read(fd, &msg, 1024);
                NumMsg wmsg(msg);
                switch_msg(gmsg, emsg, wmsg);
            }
            if (gmsg.type == DONE) is_done = 1;

        }
        cout<<"******Report for "<<dirs[i]<<" ended!***"<<endl<<endl;
    }
}

bool action_set(int ac, vector<int> prev_acc)
{
    for (auto prev: prev_acc)
        if (ac == prev) return true;

    return false;
}

vector<string> get_work(string dir)
{
    cout<<"please enter actions for: "<<dir<<" (enter -1 to end actions list)"<<endl;
    vector<int> prev_acs = {};
    vector<string> action_list = {};
    int action=0;
    do{
        cout<<">>";
        cin>>action;
        if (action == -1) break;
        if (action == actions.size()-1)
        {
            cout<<"all actions will be done. closing the action list."<<endl;
            vector<string> all_action;
            all_action.push_back(ALL_STR);
            return all_action;
        }
        else if(action > actions.size()-1)
        {
            cout<<"invalid action number. please try again!"<<endl;
        }
        else if (action_set(action, prev_acs))
        {
            cout<<"action already set. please try again!"<<endl;
        }
        else
        {
            prev_acs.push_back(action);
            if (action == 0)
            {
                action_list.push_back(GAS_BILL_STR);
                action_list.push_back(ELEC_BILL_STR);
                action_list.push_back(WATER_BILL_STR);
            }
            else 
                action_list.push_back(actions[action]);
        }
        
    }while(action != -1);
    return action_list;
}

vector<StrMsg> get_all_works(vector<string> dirs)
{
    vector<StrMsg> all_works;
    cout << "following directories found:"<<endl;
    for (int i=0;i<dirs.size();i++)
    {
        cout<<dirs[i]<<endl;
    }

    cout<<"actions available for each building is:"<<endl;
    for (int i=0; i<actions.size(); i++)
        cout<<i<<"\t"<<actions[i]<<endl;

    for (auto dir: dirs)
        all_works.push_back(StrMsg(DO_STR, get_work(dir)));

    cout<<endl<<"getting reports from buildings..."<<endl;

    return all_works;
}
