#ifndef CONSTS_H
#define CONSTS_H

enum MsgType
{
  MSG,
  NAME,
  GET_NAME,
  ALL_NAME,
  INFO,
  GET_INFO,
  ALL_INFO,
  REQ,
  FOOD_REQ,
  IGNORE,
  FAULT
};

enum UMsgType
{
  OPEN,
  CLOSE,
  SHOW_INGRED,
  SHOW_RECIPES,
  SHOW_SUPPS,
  REQ_INGREDS,
  ANS_REQ,
  SHOW_RESTAURANTS,
  SHOW_FOODS,
  ORDER,
  SHOW_HISTORY,
  SHOW_REQS,
  UFAULT
};

enum UserType
{
  SUPPLIER,
  RESTAURANT,
  CLIENT,
  ALL
};

const char PARSE_CHAR = '#';
const char* SYSTEM_UDP_PORT = "127.0.0.1";
const int MAX_SUPP_WAIT = 90;
const int REQUEST_TIMEOUT = 120;

//user modes
const char* SUPPLIER_STR = "supplier";
const char* RESTAURANT_STR = "restaurant";
const char* CLIENT_STR = "client";
const char* TIME_OUT_STR = "time out";
const char* ONGOING_STR = "ongoing";

//messages
const char* MSG_STR = "msg";
const char* GET_NAME_STR = "get_name";
const char* NAME_STR = "name";
const char* INFO_STR = "info";
const char* ALL_NAME_STR = "all_name";
const char* ALL_INFO_STR = "all_info";
const char* GET_INFO_STR = "get_info";
const char* FOOD_REQ_STR = "food_req";
const char* ACCEPT_STR = "accept";
const char* DENY_STR = "deny";

// user messages
const char* OPEN_STR = "open";
const char* CLOSE_STR = "break";
const char* SHOW_INGRED_STR = "show ingredients";
const char* SHOW_RECIPES_STR = "show recipes";
const char* SHOW_SUPPS_STR = "show suppliers";
const char* REQ_INGREDS_STR = "request ingredient";
const char* ANS_REQ_STR = "answer request";
const char* SHOW_RESTAURANTS_STR = "show restaurants";
const char* SHOW_FOODS_STR = "show menu";
const char* ORDER_STR = "order food";
const char* ANSWER_REQ_STR = "answer request";
const char* SHOW_REQS_STR = "show request list";
const char* SHOW_HISTORY_STR = "show sales history";

#endif