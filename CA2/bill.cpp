#include "read_file.cpp"

using namespace std;

int main(int argv, char *argc[])
{
    if (argv < 2)
    {
        cerr<<"path not provided for bill.cpp!\n";
        exit(0);
    }

    string path = argc[1];
    Bill bill = get_bill(path);
    vector<vector<int>> month_sums;
    for (int i=0;i<bill.get_months_size();i++)
    {
        for (auto hour:bill.monthly_usage(i))
            cout<<hour<<"#";
        cout<<"%";
    }
    
    int max_hour = bill.get_max_hour();
    cout<<"|"<<max_hour<<endl;

    return 0;
}