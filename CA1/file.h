#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "consts.h"
#include "lib.h"

struct Ingred
{
    char name[100];
    int value;
};

struct Food
{
    char name[100];
    struct Ingred ingreds[20];
    int ingred_count;
};

void parse_name(char * str, char * parse);
struct Food * make_new_food();
struct Ingred * make_new_ingred();
void check_format(char * file, int *buff);
struct Ingred get_ingreds(const char* file, int * buff);
struct Food get_food(char* file, int * buff);
int read_file(struct Food * foods);
int get_all_ingreds(struct Food* foods, struct Ingred* ingreds, int food_count);
void get_all_foods(struct Food* foods, char food_names[1024][1024], int food_count);

#include "file.c"
#endif
