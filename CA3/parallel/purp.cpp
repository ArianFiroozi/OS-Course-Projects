#include <fstream>
#include <iostream>
#include <pthread.h>

#include "lib.hpp"

pthread_mutex_t purp_mutex;

const int THREAD_NUM_PURP = 4;

void *purple_filter_part(void *args)
{
    int extra = cols % 4;
    for (int i = (*(Args *)args).row_start; i < (*(Args *)args).row_end; i++)
    {
        (*(Args *)args).count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            int shift = -i * cols * 3 + j * 3;

            int blue = int_rgb(file_buff[buff_size - (*(Args *)args).count + shift + 0]);
            int green = int_rgb(file_buff[buff_size - (*(Args *)args).count + shift + 1]);
            int red = int_rgb(file_buff[buff_size - (*(Args *)args).count + shift + 2]);

            file_buff[buff_size - (*(Args *)args).count + shift + 0] = int_rgb(0.6 * red + 0.2 * green + 0.8 * blue);
            file_buff[buff_size - (*(Args *)args).count + shift + 1] = int_rgb(0.16 * red + 0.5 * green + 0.16 * blue);
            file_buff[buff_size - (*(Args *)args).count + shift + 2] = int_rgb(0.5 * red + 0.3 * green + 0.5 * blue);
        }
    }
    pthread_exit(NULL);
}

void purple_filter()
{
    pthread_t threads[THREAD_NUM_PURP];
    pthread_mutex_init(&purp_mutex, NULL);
    int count = 2;
    int extra = cols % 4;
    int thread_shift = rows / THREAD_NUM_PURP;

    struct Args args[THREAD_NUM_PURP];
    args[0].row_start = 0;
    args[0].row_end = thread_shift;
    args[0].count = count + extra;

    for (int i = 1; i < THREAD_NUM_PURP; i++)
    {
        count += extra;
        args[i].row_start = args[i - 1].row_end;
        args[i].row_end = args[i - 1].row_end + thread_shift;
        args[i].count = args[i - 1].count + extra * thread_shift;
    }
    args[THREAD_NUM_PURP-1].row_end = rows;

    for (int i = 0; i < THREAD_NUM_PURP; i++)
        pthread_create(&threads[i], NULL, purple_filter_part, (void *)(args + i));

    for (int i = 0; i < THREAD_NUM_PURP; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&purp_mutex);
}