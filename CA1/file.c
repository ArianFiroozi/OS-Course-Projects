#include "file.h"

void parse_name(char * str, char * parse)
{
    for (int i=1; i<strlen(str);i++)
    {
        if (str[i]=='"') return;

        parse[i-1] = str[i];
        parse[i] = '\0';
    }
}

struct Food * make_new_food()
{
    struct Food * new_food = (struct Food*) malloc(sizeof(struct Food*));
    new_food->ingred_count = 0;
    return new_food;
}

struct Ingred * make_new_ingred()
{
    struct Ingred * new_ingred = (struct Ingred*) malloc(sizeof(struct Ingred*));
    return new_ingred;
}

void check_format(char * file, int *buff)
{
    char header[10];
    int new_buff = 0;
    sscanf(file + *buff, "%s%n", header, &new_buff);
    *buff += new_buff;

    if (!str_eq(header, "{") )
    {
        perror("wrong file format\n");
        exit(-1);
    }
}

struct Ingred get_ingreds(const char* file, int * buff)
{
    struct Ingred new_ingred ;

    char name[100], foo[100], num[100];

    int new_buff = 0;
    sscanf(file + *buff, "%s%s%s%n", name, foo, num, &new_buff);
    *buff += new_buff;

    parse_name(name, new_ingred.name);
    new_ingred.value = atoi(num);
    return new_ingred;
}

struct Food get_food(char* file, int * buff)
{
    struct Food food;

    char read_str[1000];

    int new_buff = 0;
    sscanf(file + *buff, "%s%n", read_str, &new_buff);
    *buff += new_buff;

    parse_name(read_str, food.name);
    food.ingred_count = 0;
    return food;
}

int read_file(struct Food * foods)
{
    int food_count = 0;

    int fd = open("recipes.json", O_RDONLY);
    if (fd < 0)
    {
        perror("file not found!\n");
        exit(-1);
    }
    
    char file[2000]={0};
    int buff = 0;
    read(fd, file, 2000);
    
    
    
    char header[10];
    sscanf(file, "%s%n", header, &buff);
    if (!str_eq(header, "{") )
    {
        perror("wrong file format\n");
        exit(-1);
    }

    while(buff)
    {
        foods[food_count] = get_food(file, &buff);

        // ingreds
        check_format(file, &buff);

        char footer[10];
        sscanf(file + buff, "%s", footer);
        while(!str_eq(footer, "},") && !str_eq(footer, "}"))
        {
            foods[food_count].ingreds[foods[food_count].ingred_count] = get_ingreds(file, &buff);
            foods[food_count].ingred_count++;
            sscanf(file + buff, "%s", footer);
        }
        food_count++;

        if (str_eq(footer, "}")) 
        {
            buff = 0;
            continue;
        }

        int new_buff;
        sscanf(file + buff, "%s%n", footer, &new_buff);
        buff += new_buff;
    }

    close(fd);
    return food_count;
}

int get_all_ingreds(struct Food* foods, struct Ingred* ingreds, int food_count)
{
    int ingred_count = 0;
    for (int i=0;i<food_count;i++)
        for (int j=0;j<foods[i].ingred_count;j++)
        {
            int added_flg = 0;
            for (int k=0; k<ingred_count;k++)
                if (str_eq(foods[i].ingreds[j].name, ingreds[k].name)) added_flg = 1;
            
            if (!added_flg)
            {
                ingreds[ingred_count++] = foods[i].ingreds[j];
                ingreds[ingred_count - 1].value = 0;
            }
        }

    return ingred_count;
}

void get_all_foods(struct Food* foods, char food_names[1024][1024], int food_count)
{
    for (int i=0;i<food_count;i++)
        strcpy(food_names[i], foods[i].name);
}
