#include <fstream>
#include <iostream>
#include <chrono>

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

int rows;
int cols;

bool fill_alloc(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
    std::ifstream file(fileName);
    if (!file)
    {
        std::cout << "File" << fileName << " doesn't exist!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return true;
}

void mirr_filter(int end, int rows, int cols, char *fileReadBuffer)
{
    int count = 1;
    int extra = cols % 4;
    for (int i = 0, i_down = rows - 1; i < rows / 2; i++, i_down--)
    {
        count += extra;

        char top_row[3], down_row[3];
        for (int j = cols - 1; j >= 0; j--)
        {
            int shift = -i * cols * 3 + j * 3;
            int down_shift = -i_down * cols * 3 + j * 3;
            top_row[0] = fileReadBuffer[end - count + shift + 0];
            top_row[1] = fileReadBuffer[end - count + shift + 1];
            top_row[2] = fileReadBuffer[end - count + shift + 2];

            fileReadBuffer[end - count + shift + 0] = fileReadBuffer[end - count + down_shift + 0];
            fileReadBuffer[end - count + shift + 1] = fileReadBuffer[end - count + down_shift + 1];
            fileReadBuffer[end - count + shift + 2] = fileReadBuffer[end - count + down_shift + 2];

            fileReadBuffer[end - count + down_shift + 0] = top_row[0];
            fileReadBuffer[end - count + down_shift + 1] = top_row[1];
            fileReadBuffer[end - count + down_shift + 2] = top_row[2];
        }
    }
}

int int_rgb(int color)
{
    while (color < 0)
        color += 256;
    if (color > 255)
        color = 255;
    return color;
}

void purple_filter(int end, int rows, int cols, char *fileReadBuffer)
{
    int count = 2;
    int extra = cols % 4;
    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            int shift = -i * cols * 3 + j * 3;

            int blue = int_rgb(fileReadBuffer[end - count + shift + 0]);
            int green = int_rgb(fileReadBuffer[end - count + shift + 1]);
            int red = int_rgb(fileReadBuffer[end - count + shift + 2]);

            fileReadBuffer[end - count + shift + 0] = int_rgb(0.6 * red + 0.2 * green + 0.8 * blue);
            fileReadBuffer[end - count + shift + 1] = int_rgb(0.16 * red + 0.5 * green + 0.16 * blue);
            fileReadBuffer[end - count + shift + 2] = int_rgb(0.5 * red + 0.3 * green + 0.5 * blue);
        }
    }
}

int format_idx(int i, int j)
{
    return -i * cols * 3 + j * 3;
}

char conv(char *fileReadBuffer, int matrix[9], int shift, int extra)
{
    int sum = 0;
    for (int i = 0; i < 9; i++)
        sum += matrix[i];

    int new_char = 0;
    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            int matrix_index = (i + 1) * 3 + j + 1;
            new_char += matrix[matrix_index] * int_rgb(fileReadBuffer[format_idx(i, j) + shift - extra * i]);
        }
    }
    return int_rgb(new_char / sum);
}

void kernel_filter(int end, int rows, int cols, char *fileReadBuffer, int matrix[9])
{
    int count = 1;
    int extra = cols % 4;
    char new_buff[rows * cols * 3];

    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
            for (int k = 0; k < 3; k++)
                new_buff[format_idx(i, j) + k] = conv(fileReadBuffer, matrix, end - count + k + format_idx(i, j), extra);
    }
    extra = cols % 4;
    count = 1;
    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
                fileReadBuffer[end - count + k + format_idx(i, j)] = new_buff[k + format_idx(i, j)];
        }
    }
}

void diagonal_filter(int end, int rows, int cols, char *fileReadBuffer)
{
    int count = 2;
    int extra = cols % 4;
    int diagonal1[2] = {0, cols - 1};
    int diagonal2[2] = {0, cols / 2};

    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            if (diagonal1[0] == i && diagonal1[1] == j)
            {
                int shift = -i * cols * 3 + j * 3;
                fileReadBuffer[end - count + shift + 0] = 255;
                fileReadBuffer[end - count + shift + 1] = 255;
                fileReadBuffer[end - count + shift + 2] = 255;
                diagonal1[0]++;
                diagonal1[1]--;
            }

            if (diagonal2[0] == i && diagonal2[1] == j)
            {
                int shift = -i * cols * 3 + j * 3;
                fileReadBuffer[end - count + shift + 0] = 255;
                fileReadBuffer[end - count + shift + 1] = 255;
                fileReadBuffer[end - count + shift + 2] = 255;
                diagonal2[0]++;
                diagonal2[1]--;
                if (diagonal2[1] < 0)
                    diagonal2[1] = cols - 1;
            }
        }
    }
}

void write_bmp(char *file_buff, const char *output_name, int bufferSize)
{
    std::ofstream write(output_name);
    if (!write)
    {
        std::cout << "Failed to write " << output_name << std::endl;
        return;
    }
    write.write(file_buff, bufferSize);
}

using namespace std;
using namespace chrono;

int main(int argc, char *argv[])
{
    auto main_start = high_resolution_clock::now();
    char *file_buff;
    int bufferSize;

    auto read_start = high_resolution_clock::now();
    if (!fill_alloc(file_buff, argv[1], rows, cols, bufferSize))
    {
        std::cout << "File read error" << std::endl;
        return 1;
    }

    auto mirr_start = high_resolution_clock::now();
    mirr_filter(bufferSize, rows, cols, file_buff);

    auto blur_start = high_resolution_clock::now();
    int filter_matrix[] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
    kernel_filter(bufferSize, rows, cols, file_buff, filter_matrix);

    auto purp_start = high_resolution_clock::now();
    purple_filter(bufferSize, rows, cols, file_buff);

    auto diag_start = high_resolution_clock::now();
    diagonal_filter(bufferSize, rows, cols, file_buff);

    auto write_start = high_resolution_clock::now();
    write_bmp(file_buff, "output.bmp", bufferSize);

    auto main_stop = high_resolution_clock::now();

    auto read = duration_cast<microseconds>(mirr_start - read_start);
    auto mirr = duration_cast<microseconds>(blur_start - mirr_start);
    auto blur = duration_cast<microseconds>(purp_start - blur_start);
    auto purp = duration_cast<microseconds>(diag_start - purp_start);
    auto diag = duration_cast<microseconds>(write_start - diag_start);
    auto all = duration_cast<microseconds>(main_stop - main_start);

    printf("Read: %0.3f\n", double(read.count()) / 1000);
    printf("Flip: %0.3f\n", double(mirr.count()) / 1000);
    printf("Blur: %0.3f\n", double(blur.count()) / 1000);
    printf("Purple: %0.3f\n", double(purp.count()) / 1000);
    printf("Lines: %0.3f\n", double(diag.count()) / 1000);
    printf("Execution: %0.3f\n", double(all.count()) / 1000);
    return 0;
}