#ifndef MSG_PARSE_H
#define MSG_PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "consts.h"


void cut_msg(char* msg);
void print(char *print_str);
void get_user_inp(char *inp);
int str_eq(const char *s1, const char *s2);
void format(const char * type, char * cmd, char * formatted);
enum MsgType str_to_msg(char *str);
enum UMsgType str_to_umsg(char *str);
enum UserType str_to_usertype(char *str);
const char* umsg_to_str(enum UMsgType type);
const char* msg_to_str(enum MsgType msg);
const char* usertype_to_str(enum UserType type);
void strsl(char *str,int k);
enum MsgType parse(char * str);
enum UserType parse_usertype(char * str);
int is_name_used(char name[100], char all_names[100][100], int len);

#include "msg_parse.c"
#endif