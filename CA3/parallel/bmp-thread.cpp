#include <fstream>
#include <iostream>
#include <chrono>

#include "flip.cpp"
#include "blur.cpp"
#include "purp.cpp"

using namespace std;
using namespace chrono;

int main(int argc, char *argv[])
{
    auto main_start = high_resolution_clock::now();

    auto read_start = high_resolution_clock::now();
    if (!fill_alloc(file_buff, argv[1], rows, cols, buff_size)) {
        std::cout << "File read error" << std::endl;
        return 1;
    }

    auto mirr_start = high_resolution_clock::now();
    flip_filter();

    auto blur_start = high_resolution_clock::now();
    blur_filter();

    auto purp_start = high_resolution_clock::now();
    purple_filter();


    auto diag_start = high_resolution_clock::now();
    diagonal_filter(buff_size, rows, cols, file_buff);

    auto write_start = high_resolution_clock::now();
    write_bmp(file_buff, "output.bmp", buff_size);
    auto main_stop = high_resolution_clock::now();

    auto read = duration_cast<microseconds>(mirr_start - read_start);
    auto mirr = duration_cast<microseconds>(blur_start - mirr_start);
    auto blur = duration_cast<microseconds>(purp_start - blur_start);
    auto purp = duration_cast<microseconds>(diag_start - purp_start);
    auto diag = duration_cast<microseconds>(write_start - diag_start);
    auto all = duration_cast<microseconds>(main_stop - main_start);

    printf("Read: %0.3f\n", double(read.count())/1000);
    printf("Flip: %0.3f\n", double(mirr.count())/1000);
    printf("Blur: %0.3f\n", double(blur.count())/1000);
    printf("Purple: %0.3f\n", double(purp.count())/1000);
    printf("Lines: %0.3f\n", double(diag.count())/1000);
    printf("Execution: %0.3f\n", double(all.count())/1000);
    return 0;
}
