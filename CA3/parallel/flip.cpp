#include <fstream>
#include <iostream>
#include <pthread.h>

#include "lib.hpp"

pthread_mutex_t flip_mutex;

const int THREAD_NUM_FLIP = 4;

void *mirr_filter_part(void* args)
{
    int extra = cols % 4;
    for (int i = (*(Args*)args).row_start, i_down = rows - 1 - (*(Args*)args).row_start; 
            i < (*(Args*)args).row_end; i++, i_down--)
    {
        (*(Args*)args).count += extra;
        char top_row[3], down_row[3];
        for (int j = cols - 1; j >= 0; j--)
        {
            int shift = -i * cols * 3 + j * 3;
            int down_shift = -i_down * cols * 3 + j * 3;

            top_row[0] = file_buff[buff_size - (*(Args*)args).count + shift + 0];
            top_row[1] = file_buff[buff_size - (*(Args*)args).count + shift + 1];
            top_row[2] = file_buff[buff_size - (*(Args*)args).count + shift + 2];

            file_buff[buff_size - (*(Args*)args).count + shift + 0] = file_buff[buff_size - (*(Args*)args).count + down_shift + 0];
            file_buff[buff_size - (*(Args*)args).count + shift + 1] = file_buff[buff_size - (*(Args*)args).count + down_shift + 1];
            file_buff[buff_size - (*(Args*)args).count + shift + 2] = file_buff[buff_size - (*(Args*)args).count + down_shift + 2];

            file_buff[buff_size - (*(Args*)args).count + down_shift + 0] = top_row[0];
            file_buff[buff_size - (*(Args*)args).count + down_shift + 1] = top_row[1];
            file_buff[buff_size - (*(Args*)args).count + down_shift + 2] = top_row[2];
            
        }
    }

    pthread_exit(NULL);
}

void flip_filter()
{
    pthread_t threads[THREAD_NUM_FLIP];
    pthread_mutex_init(&flip_mutex, NULL);
    int count = 1;
    int extra = cols % 4;
    int thread_shift = rows / 2 / THREAD_NUM_FLIP;

    struct Args args[THREAD_NUM_FLIP];
    args[0].row_start = 0;
    args[0].row_end = thread_shift;
    args[0].count = count + extra;

    for (int i = 1; i < THREAD_NUM_FLIP; i++)
    {
        args[i].row_start = args[i - 1].row_end;
        args[i].row_end = args[i-1].row_end + thread_shift;
        args[i].count = args[i - 1].count + extra * thread_shift;
    }
    args[THREAD_NUM_FLIP-1].row_end = rows/2;

    for (int i = 0; i < THREAD_NUM_FLIP; i++)
        pthread_create(&threads[i], NULL, mirr_filter_part, (void *)(args + i));

    for (int i = 0; i < THREAD_NUM_FLIP; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&flip_mutex);
}