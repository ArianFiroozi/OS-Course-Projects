#ifndef LIB_HPP
#define LIB_HPP

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

int buff_size;
int rows;
int cols;
char *file_buff;

int filter_matrix[] = {1,2,1,2,4,2,1,2,1};
char *new_buff;

struct Args
{
    int row_start;
    int row_end;
    int count;
};

int format_idx(int i, int j)
{
    return - i * cols * 3 + j * 3;
}

int int_rgb(int color)
{
    while (color < 0) color += 256;
    if (color > 255) color =255;
    return color;
}

char conv(char *fileReadBuffer, int matrix[9], int shift, int extra)
{
    int sum = 0;
    for (int i=0; i<9; i++) sum += matrix[i];

    int new_char=0;
    for (int i=-1;i<2;i++)
    {
        for (int j=-1;j<2;j++)
        {
            int matrix_index = (i+1)*3+j+1;
            new_char += matrix[matrix_index] * int_rgb(fileReadBuffer[format_idx(i,j) + shift - extra*i]);
        }
    }
    return int_rgb(new_char/sum);
}

void write_bmp(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize)
{
    std::ofstream write(nameOfFileToCreate);
    if (!write)
    {
        std::cout << "Failed to write " << nameOfFileToCreate << std::endl;
        return;
    }
    write.write(fileBuffer, bufferSize);
}

void diagonal_filter(int end, int rows, int cols, char *file_read_buff)
{
    int count = 2;
    int extra = cols % 4;
    int diagonal1[2] = {0, cols-1};
    int diagonal2[2] = {0, cols/2};

    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            if (diagonal1[0] == i && diagonal1[1] == j)
            {
                int shift = - i * cols * 3 + j * 3;
                file_read_buff[end - count + shift + 0] = 255;
                file_read_buff[end - count + shift + 1] = 255;
                file_read_buff[end - count + shift + 2] = 255;
                diagonal1[0]++;
                diagonal1[1]--;
                
            }

            if (diagonal2[0] == i && diagonal2[1] == j)
            {
                int shift = - i * cols * 3 + j * 3;
                file_read_buff[end - count + shift + 0] = 255;
                file_read_buff[end - count + shift + 1] = 255;
                file_read_buff[end - count + shift + 2] = 255;
                diagonal2[0]++;
                diagonal2[1]--;
                if (diagonal2[1] < 0) diagonal2[1]=cols-1;
            }
        }
    }
}

bool fill_alloc(char *&buffer, const char *file_name, int &rows, int &cols, int &buff_size)
{
    std::ifstream file(file_name);
    if (!file)
    {
        std::cout << "File" << file_name << " doesn't exist!" << std::endl;
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
    buff_size = file_header->bfSize;

    new_buff = new char [cols*rows*3];
    return true;
}

#endif