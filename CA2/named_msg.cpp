#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <string>

using namespace std;

const int GAS_IDX = 0;
const int ELEC_IDX = 1;
const int WATER_IDX = 2;


const string MSG_DELIM = "#";
const string MSG_BGN_END = "%";

const string AVG_MONTH_USE_STR = "AVG_MONTH_USE";
const string MONTH_USE_STR = "MONTH_USE";
const string MOST_USE_STR = "MOST_USE";
const string BILL_STR = "BILL";
const string DIFFERENECE_STR = "DIFFERENECE";
const string FIFO_REQ_STR = "FIFO_REQ";
const string DONE_STR = "DONE";
const string WATER_BILL_STR = "WATER_BILL";
const string ELEC_BILL_STR = "ELEC_BILL";
const string GAS_BILL_STR = "GAS_BILL";
const string DO_STR = "DO";
const string ALL_STR = "ALL";

enum MsgType
{
    AVG_MONTH_USE,
    MONTH_USE,
    MOST_USE,
    BILL,
    DIFFERENECE,
    UNKNOWN_MSG,
    FIFO_REQ,
    ALL,
    DONE,
    WATER_BILL,
    GAS_BILL,
    ELEC_BILL,
    DO
};

class Msg
{
    protected:
    string msg_type_str(MsgType _type)
    {
        switch (_type)
        {
        case AVG_MONTH_USE: 
            return AVG_MONTH_USE_STR;
        case MONTH_USE: 
            return MONTH_USE_STR;
        case MOST_USE: 
            return MOST_USE_STR;
        case BILL: 
            return BILL_STR;
        case DIFFERENECE: 
            return DIFFERENECE_STR;
        case FIFO_REQ:
            return FIFO_REQ_STR;
        case DONE:
            return DONE_STR;
        case WATER_BILL:
            return WATER_BILL_STR;
        case GAS_BILL:
            return GAS_BILL_STR;
        case ELEC_BILL:
            return ELEC_BILL_STR;
        case DO:
            return DO_STR;
        case ALL:
            return ALL_STR;
        default:
            return "";
        }
    }

    MsgType str_to_type(string str)
    {
        if (str == AVG_MONTH_USE_STR) return AVG_MONTH_USE;
        if (str == MONTH_USE_STR) return MONTH_USE;
        if (str == MOST_USE_STR) return MOST_USE;
        if (str == BILL_STR) return BILL;
        if (str == DIFFERENECE_STR) return DIFFERENECE;
        if (str == FIFO_REQ_STR) return FIFO_REQ;
        if (str == DONE_STR) return DONE;
        if (str == GAS_BILL_STR) return GAS_BILL;
        if (str == WATER_BILL_STR) return WATER_BILL;
        if (str == ELEC_BILL_STR) return ELEC_BILL;
        if (str == ALL_STR) return ALL;
        if (str == DO_STR) return DO;
        return UNKNOWN_MSG;
    }

    public:
    MsgType type;
    string msg_str;
};

class NumMsg: public Msg
{
    string to_str(vector<int> nums)
    {
        string msg = this->msg_type_str(type);
        msg.append(MSG_BGN_END);
        for (auto num: nums)
        {
            msg.append(to_string(num));
            msg.append(MSG_DELIM);
        }
        msg.append(MSG_BGN_END);
        return msg;
    }

    vector<int> parse(string msg)
    {
        int pos = 0;
        vector<int> tokens;
        int msg_end = msg.find(MSG_BGN_END);

        while (pos < msg.size() && tokens.size() < msg_end)
        {
            pos = msg.find(MSG_DELIM);
            int new_token = stoi(msg.substr(0, pos));
            tokens.insert(tokens.end(), new_token);
            msg.erase(0, pos + 1);
        }
        return tokens;
    }

    string msg_nums_str()
    {
        string msg_str_nums = msg_str;

        string _type = msg_str_nums.substr(0, msg_str_nums.find(MSG_BGN_END));
        //type = str_to_type(_type);
        msg_str_nums.erase(0, msg_str_nums.find(MSG_BGN_END)+1);
        msg_str_nums.pop_back();
        return msg_str_nums;
    }

    public:
    NumMsg(string _type, vector<int> nums)
    {
        type = str_to_type(_type);
        msg_str = to_str(nums);
    }

    NumMsg(string _msg)
    {
        string _type = _msg.substr(0, _msg.find(MSG_BGN_END));
        type = str_to_type(_type);

        msg_str = _msg;
    }

    vector<int> get_nums()
    {
        string nums_str = msg_nums_str();
        vector<int> nums;

        int pos = 0, npos;
        while((npos=nums_str.find(MSG_DELIM, pos)) >= 0)
        {
            int num = stoi(nums_str.substr(pos, npos - pos));
            pos = npos+1;
            nums.push_back(num);
        }
        return nums;
    }
};

class StrMsg: public Msg
{
    string to_str(vector<string> strings)
    {
        string msg = msg_type_str(type);
        msg.append(MSG_BGN_END);
        for (auto string: strings)
        {
            msg.append(string);
            msg.append(MSG_DELIM);
        }
        msg.append(MSG_BGN_END);
        return msg;
    }

    vector<string> parse(string msg)
    {
        int pos = 0;
        vector<string> tokens;
        int msg_end = msg.find(MSG_BGN_END);

        while (pos < msg.size() && tokens.size() < msg_end)
        {
            pos = msg.find(MSG_DELIM);
            string new_token = msg.substr(0, pos);
            tokens.insert(tokens.end(), new_token);
            msg.erase(0, pos + 1);
        }
        return tokens;
    }

    string msg_strings()
    {
        string msg_str_nums = msg_str;

        string _type = msg_str_nums.substr(0, msg_str_nums.find(MSG_BGN_END));
        //type = str_to_type(_type);
        msg_str_nums.erase(0, msg_str_nums.find(MSG_BGN_END)+1);
        msg_str_nums.pop_back();
        return msg_str_nums;
    }

    public:
    StrMsg(string _type, vector<string> tokens)
    {
        type = str_to_type(_type);
        msg_str = to_str(tokens);
    }

    StrMsg(string _msg)
    {
        string _type = _msg.substr(0, _msg.find(MSG_BGN_END));
        type = str_to_type(_type);

        msg_str = _msg;
    }

    vector<string> get_strings()
    {
        string strings_str = msg_strings();
        vector<string> strings;

        int pos = 0, npos;
        while((npos=strings_str.find(MSG_DELIM, pos)) >= 0)
        {
            string str = strings_str.substr(pos, npos-pos);
            pos = npos+1;
            strings.push_back(str);
        }
        return strings;
    }
};
