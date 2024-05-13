// reading csv file

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

int INFO_LEN  = 6;
const std::string DELIM = ",";

int JUNK_SIZE = 3;

using namespace std;

enum BillType
{
    ELECTRICITY,
    GAS,
    WATER,
    UNKNOWN
};

vector<int> parse_bills(string line)
{
    int pos = 0;
    vector<int> tokens;
    while (pos < line.size() && tokens.size() < INFO_LEN + 2)
    {
        pos = line.find(DELIM);
        int new_token = stoi(line.substr(0, pos));
        tokens.insert(tokens.end(), new_token);
        line.erase(0, pos + 1);
    }
    int new_token = stoi(line.substr(0, line.size()));
    tokens.insert(tokens.end(), new_token);

    //tokens.erase(tokens.begin(), tokens.begin() + 3);
    return tokens;
}

class Bill
{
    struct month
    {
        int month_num;
        vector<vector<int>> data;
    };

    enum BillType type;
    vector<month> months;

    int get_month_id(int num)
    {
        for (int i=0; i < months.size(); i++)
        {
            if (months[i].month_num == num)
                return i;
        }
        return -1;
    }

    public:
    void add_line(vector<int> line)
    {
        int month_id = get_month_id(line[1]);
        if (month_id<0)
        {
            months.push_back(month());
            month_id = months.size() - 1;
            months[month_id].month_num = line[1];
        }

        line.erase(line.begin(), line.begin() + JUNK_SIZE);
        months[month_id].data.push_back(line);
    }

    BillType get_type()
    {
        return type;
    }

    void set_type (BillType _type) 
    {
        type = _type;
    }

    int hour_usage(int hour)
    {
        int sum = 0;
        for(auto curr_month:months)
            for (auto line: curr_month.data)
                sum += line[hour];
        return sum;
    }

    vector<int> monthly_usage(int month)
    {
        vector<int> all_sums;
        for (int hour=0;hour<INFO_LEN;hour++)
        {
            int sum = 0;
            for (auto line: months[month].data)
                sum += line[hour];
            all_sums.push_back(sum);
        }
        return all_sums;
    }

    int get_max_hour()
    {
        int max_hour = -1, max_sum = 0;
        for (int i=0, usage;i<INFO_LEN;i++)
            if ((usage = hour_usage(i)) > max_sum)
            {
                max_sum = usage;
                max_hour = i;
            }

        return max_hour;
    }

    int get_months_size()
    {
        return months.size();
    }

};

string type_to_path(BillType type)
{
    switch(type)
    {
    case GAS:
        return "Gas.csv";
    case ELECTRICITY:
        return "Electricity.csv";
    case WATER:
        return "Water.csv";
    default:
        return NULL;
    }
}

Bill read_file(BillType type, string path)
{
    Bill bill;
    bill.set_type(type);
    ifstream bill_file;
    bill_file.open(path);

    string line;
    getline(bill_file, line); //first line is junk
    
    while(getline(bill_file, line))
        bill.add_line(parse_bills(line));
    // close(bill_file);

    return bill;
}

BillType str_to_bill(string str)
{
    if(!str.compare("Gas"))
        return GAS;
    if(!str.compare("Electricity"))
        return ELECTRICITY;
    if(!str.compare("Water"))
        return WATER;
    return UNKNOWN;
}

Bill get_bill(string path, string type_str="")
{
    BillType type = str_to_bill(type_str);
    return read_file(type, path);
}

Bill get_bill()
{
    JUNK_SIZE = 2;
    INFO_LEN = 3;
    Bill bill = get_bill("bills.csv");
    JUNK_SIZE = 3;
    INFO_LEN = 6;
    return bill;
}
