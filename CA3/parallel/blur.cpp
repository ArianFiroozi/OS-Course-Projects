#include <fstream>
#include <iostream>
#include <pthread.h>

#include "lib.hpp"

pthread_mutex_t blur_mutex;

const int THREAD_NUM_BLUR = 4;

void replace_main_buff()
{
    int extra = cols % 4;
    int count = 1;
    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
                file_buff[buff_size - count + k + format_idx(i, j)] = new_buff[k + format_idx(i, j) + buff_size];
        }
    }
}

void *blur_filter_part(void *args)
{
    int extra = cols % 4;
    for (int i = (*(Args *)args).row_start; i < (*(Args *)args).row_end; i++)
    {
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
                new_buff[format_idx(i, j) + k + buff_size] = conv(file_buff, filter_matrix, 
                                                                buff_size - (*(Args *)args).count + k + format_idx(i, j), extra);
        }
    }
    pthread_exit(NULL);
}

void blur_filter()
{
    pthread_t threads[THREAD_NUM_BLUR];
    pthread_mutex_init(&blur_mutex, NULL);
    int count = 1;
    int extra = cols % 4;
    int thread_shift = rows / THREAD_NUM_BLUR;

    struct Args args[THREAD_NUM_BLUR];
    args[0].row_start = 3;
    args[0].row_end = thread_shift;
    args[0].count = count + extra;

    for (int i = 1; i < THREAD_NUM_BLUR; i++)
    {
        count += extra;
        args[i].row_start = args[i - 1].row_end;
        args[i].row_end = args[i - 1].row_end + thread_shift;
        args[i].count = args[i - 1].count + extra * thread_shift;
    }
    args[THREAD_NUM_BLUR-1].row_end = rows-3;

    for (int i = 0; i < THREAD_NUM_BLUR; i++)
        pthread_create(&threads[i], NULL, blur_filter_part, (void *)(args + i));

    for (int i = 0; i < THREAD_NUM_BLUR; i++)
        pthread_join(threads[i], NULL);
    pthread_mutex_destroy(&blur_mutex);

    replace_main_buff();
}