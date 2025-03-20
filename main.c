#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sobel.h"
#include "rtclock.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Sobel kernels
int Kx[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}};

int Ky[3][3] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}};

unsigned char **input_image;
unsigned char **output_image;
int width, height;
unsigned char threshold;
int num_threads;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Error Wrong Num Of Inputs: %s <input-file> <num-threads (>= 1)> <threshold (0-255)>\n", argv[0]);
        return -1;
    }

    char *filename = argv[1];
    num_threads = atoi(argv[2]);
    threshold = (unsigned char)atoi(argv[3]);

    if (num_threads < 1)
    {
        fprintf(stderr, "Error: input threads must be >= 1\n");
        return -1;
    }

    int threshold = atoi(argv[3]);

    if (threshold < 0)
        threshold = 0;
    if (threshold > 255)
        threshold = 255;
       
    printf("Clamped threshold: %d\n", threshold);
    printf("Raw threshold input: %s\n", argv[3]);

    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 1);
    if (!data)
    {
        fprintf(stderr, "Error loading image %s\n", filename);
        return -1;
    }

    input_image = (unsigned char **)malloc(height * sizeof(unsigned char *));
    for (int i = 0; i < height; i++)
    {
        input_image[i] = &data[i * width];
    }

    output_image = (unsigned char **)malloc(height * sizeof(unsigned char *));
    for (int i = 0; i < height; i++)
    {
        output_image[i] = (unsigned char *)malloc(width * sizeof(unsigned char));
    }

    printf("Loaded %s. Height=%d, Width=%d\n", filename, height, width);

    double start = rtclock();

    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_args *args = (thread_args *)malloc(num_threads * sizeof(thread_args));

    int rows_per_thread = height / num_threads;
    int extra_rows = height % num_threads;
    int current_row = 0;

    for (int t = 0; t < num_threads; t++)
    {
        args[t].start_row = current_row;
        args[t].end_row = current_row + rows_per_thread - 1 + (t < extra_rows);
        current_row = args[t].end_row + 1;
        pthread_create(&threads[t], NULL, sobel_filter, &args[t]);
    }

    for (int t = 0; t < num_threads; t++)
    {
        pthread_join(threads[t], NULL);
    }

    double end = rtclock();
    printf("Time taken (thread count = %d): %.6f sec\n", num_threads, end - start);

    char *d = strrchr(filename, '.');
    char *out_filename;
    if (d)
    {
        size_t base_len = d - filename;
        out_filename = (char *)malloc(base_len + 11);
        strncpy(out_filename, filename, base_len);
        strcpy(out_filename + base_len, "-sobel.jpg");
    }
    else
    {
        out_filename = (char *)malloc(strlen(filename) + 11);
        sprintf(out_filename, "%s-sobel.jpg", filename);
    }

    unsigned char *output_data = (unsigned char *)malloc(width * height);
    for (int i = 0; i < height; i++)
    {
        memcpy(output_data + i * width, output_image[i], width);
    }

    if (!stbi_write_jpg(out_filename, width, height, 1, output_data, 80))
    {
        fprintf(stderr, "Error writing %s\n", out_filename);
    }
    else
    {
        printf("Saved as %s\n", out_filename);
    }

    free(output_data);
    free(out_filename);
    free(threads);
    free(args);

    for (int i = 0; i < height; i++)
    {
        free(output_image[i]);
    }
    free(output_image);
    free(input_image);
    stbi_image_free(data);

    return 0;
}
