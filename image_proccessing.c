#include <stdio.h>
#include <stdlib.h>

int readbmp_processig(const char *fname_s, const char *fname_t,int threshold_value,int mode)
{
    FILE          *fp_s = NULL;    // source file handler
    FILE          *fp_t = NULL;    // target file handler
    unsigned int  x,y;             // for loop counter
    unsigned int  width, height;   // image width, image height
    unsigned char *image_s = NULL; // source image array
    unsigned char *image_t = NULL; // target image array
    unsigned char R, G, B;         // color of R, G, B
    unsigned int y_avg;            // average of y axle
    unsigned int y_t;              // target of y axle
    int gray;
    unsigned long histogram[256],new_hist[256];
    int histoeq[256];
    float hist[256], sum;


    unsigned char header[54] =
    {
        0x42,        // identity : B
        0x4d,        // identity : M
        0, 0, 0, 0,  // file size
        0, 0,        // reserved1
        0, 0,        // reserved2
        54, 0, 0, 0, // RGB data offset
        40, 0, 0, 0, // struct BITMAPINFOHEADER size
        0, 0, 0, 0,  // bmp width
        0, 0, 0, 0,  // bmp height
        1, 0,        // planes
        24, 0,       // bit per pixel
        0, 0, 0, 0,  // compression
        0, 0, 0, 0,  // data size
        0, 0, 0, 0,  // h resolution
        0, 0, 0, 0,  // v resolution
        0, 0, 0, 0,  // used colors
        0, 0, 0, 0   // important colors
    };

    unsigned int file_size;           // file size
    unsigned int rgb_raw_data_offset; // RGB raw data offset

    fp_s = fopen(fname_s, "rb");
    if (fp_s == NULL)
    {
        printf("fopen fp_s error\n");
        return -1;
    }

    // move offset to 10 to find rgb raw data offset
    fseek(fp_s, 10, SEEK_SET);
    fread(&rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s);
    // move offset to 18    to get width & height;
    fseek(fp_s, 18, SEEK_SET);
    fread(&width,  sizeof(unsigned int), 1, fp_s);
    fread(&height, sizeof(unsigned int), 1, fp_s);
    // move offset to rgb_raw_data_offset to get RGB raw data
    fseek(fp_s, rgb_raw_data_offset, SEEK_SET);

    image_s = (unsigned char *)malloc((size_t)width * height * 3);
    if (image_s == NULL)
    {
        printf("malloc images_s error\n");
        return -1;
    }

    image_t = (unsigned char *)malloc((size_t)width * height * 3);
    if (image_t == NULL)
    {
        printf("malloc image_t error\n");
        return -1;
    }

    fread(image_s, sizeof(unsigned char), (size_t)(long)width * height * 3, fp_s);


    for(x = 0; x<256;x++)
    {
        histogram[x] = 0;
        hist[x] = 0.0;
        new_hist[x] = 0;
    }

    for(y = 0; y != height; ++y)
    {
        for(x = 0; x != width; ++x)
        {
            R = *(image_s + 3 * (width * y + x) + 2);
            G = *(image_s + 3 * (width * y + x) + 1);
            B = *(image_s + 3 * (width * y + x) + 0);

            gray=(int)(0.299*((int)R)+0.587*((int)G)+0.114*((int)B));

            histogram[gray] += 1;

            *(image_t + 3 * (width * y + x) + 2) = (unsigned char)gray;
            *(image_t + 3 * (width * y + x) + 1) = (unsigned char)gray;
            *(image_t + 3 * (width * y + x) + 0) = (unsigned char)gray;
        }
    }
    if(mode ==1)
    {
        for(x = 0;x<256; x++)
        {
            printf("%d = %d\n",x,histogram[x]);
        }
    }
    if(mode == 2)
    {
        for(x = 0;x<256;x++)
        {
            hist[x] += histogram[x];
        }
        for(x = 0;x<256;x++)
        {
            sum = 0.0;
            for(y = 0;y<=x;y++)
            {
                sum += hist[y];
            }
            if(sum == 0.0)
            {
                histoeq[x] = 0;
            }
            else
            {
                histoeq[x] = (int)(sum*255/(width * height) + 0.5);
            }
        }
        for(x = 0;x<256;x++)
        {
            int pos;
            pos = histoeq[x];
            new_hist[pos] += histogram[x];
        }
        for(x = 0;x<256; x++)
        {
            printf("%d = %d\n",x,new_hist[x]);
        }
    }
    // write to new bmp
    fp_t = fopen(fname_t, "wb");
    if (fp_t == NULL)
    {
        printf("fopen fname_t error\n");
        return -1;
    }

    // file size
    file_size = width * height * 3 + rgb_raw_data_offset;
    header[2] = (unsigned char)(file_size & 0x000000ff);
    header[3] = (file_size >> 8)  & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    // width
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8)  & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    // height
    header[22] = height &0x000000ff;
    header[23] = (height >> 8)  & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    // write header
    fwrite(header, sizeof(unsigned char), rgb_raw_data_offset, fp_t);
    // write image
    fwrite(image_t, sizeof(unsigned char), (size_t)(long)width * height * 3, fp_t);

    fclose(fp_s);
    fclose(fp_t);

    return 0;
}

int main()
{
    char ofilename[20];
    char nfilename[20];
    int threshold_value;
    int mode;

    printf("please input the original image filename:");
    scanf("%s",&ofilename);
    printf("\nplease input the new image filename:");
    scanf("%s",&nfilename);
    printf("\nplease input mode(1.print histogram 2.print histogram equalization): ");
    scanf("%d",&mode);

    readbmp_processig(ofilename, nfilename,threshold_value,mode);

    printf("\nProcessing finished~~~");
}
