#include "msg_parse.h"


void cut_msg(char* msg)
{
    for (int i=0;i<strlen(msg) && msg[i] != '\0';i++)
        if (msg[i] == '\n')
        {
            msg[i] = '\0';
            break;
        }
}

void print(char *print_str)
{
    int size = 0;
    while (print_str[size++] != '\0');
    write(1, print_str, size);
}

void get_user_inp(char *inp)
{
    print(">>");
    memset(inp, 0, 1024);
    int n = read(1, inp, 1024);
    inp[n-1] = '\0';
}

int str_eq(const char *s1, const char *s2)
{
  if (strlen(s1) != strlen(s2)) return 0;

  for (int i=0; i < strlen(s1);i++)
  {
    if (s1[i] != s2[i]) return 0;
  }

  return 1;
}

void format(const char * type, char * cmd, char * formatted)
{
  int itter;
  formatted[0] = PARSE_CHAR;

  for (itter=1;itter<=strlen(type);itter++) 
      formatted[itter] = type[itter-1];
  
  formatted[itter++] = PARSE_CHAR;
  for (int i=0; i < strlen(cmd) && itter < 1024 - 1; itter++,i++)
      formatted[itter] = cmd[i];

  formatted[itter] = '\0';
}

enum MsgType str_to_msg(char *str)
{
  if (str_eq(str, MSG_STR)) return MSG;
  else if (str_eq(str, NAME_STR)) return NAME;
  else if (str_eq(str, GET_NAME_STR)) return GET_NAME;
  else if (str_eq(str, ALL_NAME_STR)) return ALL_NAME;
  else if (str_eq(str, INFO_STR)) return INFO;
  else if (str_eq(str, GET_INFO_STR)) return GET_INFO;
  else if (str_eq(str, ALL_INFO_STR)) return ALL_INFO;
  else if (str_eq(str, REQ_INGREDS_STR)) return REQ;
  if (str_eq(str, FOOD_REQ_STR)) return FOOD_REQ;
  else return FAULT;
}

enum UMsgType str_to_umsg(char *str)
{
  if (str_eq(str, OPEN_STR)) return OPEN;
  if (str_eq(str, CLOSE_STR)) return CLOSE;
  if (str_eq(str, SHOW_INGRED_STR)) return SHOW_INGRED;
  if (str_eq(str, SHOW_RECIPES_STR)) return SHOW_RECIPES;
  if (str_eq(str, SHOW_SUPPS_STR)) return SHOW_SUPPS;
  if (str_eq(str, REQ_INGREDS_STR)) return REQ_INGREDS;
  if (str_eq(str, ANS_REQ_STR)) return ANS_REQ;
  if (str_eq(str, SHOW_FOODS_STR)) return SHOW_FOODS;
  if (str_eq(str, SHOW_RESTAURANTS_STR)) return SHOW_RESTAURANTS;
  if (str_eq(str, ORDER_STR)) return ORDER;
  if (str_eq(str, SHOW_REQS_STR)) return SHOW_REQS;
  if (str_eq(str, SHOW_HISTORY_STR)) return SHOW_HISTORY;
  else return UFAULT;
}

enum UserType str_to_usertype(char *str)
{
  if (str_eq(str, SUPPLIER_STR)) return SUPPLIER;
  if (str_eq(str, RESTAURANT_STR)) return RESTAURANT;
  if (str_eq(str, CLIENT_STR)) return CLIENT;
  else return ALL;
}

const char* umsg_to_str(enum UMsgType type)
{
  switch (type){
  case OPEN: return OPEN_STR;
  case CLOSE: return CLOSE_STR;
  case SHOW_INGRED: return SHOW_INGRED_STR;
  case SHOW_RECIPES: return SHOW_RECIPES_STR;
  case SHOW_SUPPS: return SHOW_SUPPS_STR;
  case REQ_INGREDS: return REQ_INGREDS_STR;
  case ANS_REQ: return ANS_REQ_STR;
  case SHOW_FOODS: return SHOW_FOODS_STR;
  case ORDER: return ORDER_STR;
  case SHOW_RESTAURANTS: return SHOW_RESTAURANTS_STR;
  case SHOW_REQS: return SHOW_REQS_STR;
  case SHOW_HISTORY: return SHOW_HISTORY_STR;
  default: return NULL;
  }
}

const char* msg_to_str(enum MsgType msg)
{
  switch(msg)
  {
    case NAME:
      return NAME_STR;
    case GET_NAME:
      return GET_NAME_STR;
    case ALL_NAME:
      return ALL_NAME_STR;
    case INFO:
      return INFO_STR;
    case GET_INFO:
      return GET_INFO_STR;
    case ALL_INFO:
      return ALL_INFO_STR;
    case FOOD_REQ:
      return FOOD_REQ_STR;
    default:
      return NULL;
  }
}

const char* usertype_to_str(enum UserType type)
{
  switch(type)
  {
    case SUPPLIER:
      return SUPPLIER_STR;
    case RESTAURANT:
      return RESTAURANT_STR;
    case CLIENT:
      return CLIENT_STR;
    default:
      return NULL;
  }
}

void strsl(char *str,int k)
{
  for (int i=k; i<strlen(str); i++)
  {
    str[i-k] = str[i];
  }
  str[strlen(str) - k] = '\0';
}

enum MsgType parse(char * str)
{
  if (strlen(str) <= 2 || str[0] != PARSE_CHAR) return FAULT;

  char command[1024];
  for (int i=1; i<strlen(str); i++)
  {
    if (str[i] == '\0') return FAULT;

    if (str[i] == PARSE_CHAR)
    {
      command[i-1] = '\0'; 
      strsl(str, i+1);
      break;
    }
    command[i-1] = str[i];
  }

  return str_to_msg(command);
}

enum UserType parse_usertype(char * str)
{
  if (strlen(str) <= 2 || str[0] != PARSE_CHAR) return ALL;

  char command[1024];
  for (int i=1; i<strlen(str); i++)
  {
    if (str[i] == '\0') return ALL;

    if (str[i] == PARSE_CHAR)
    {
      command[i-1] = '\0'; 
      strsl(str, i+1);
      break;
    }
    command[i-1] = str[i];
  }

  return str_to_usertype(command);
}

int is_name_used(char name[100], char all_names[100][100], int len)
{
    for (int i=0;i<len;i++)
        if (str_eq(name, all_names[i])) return 1;
    return 0;
}
