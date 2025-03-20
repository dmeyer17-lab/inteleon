#define BLACK 0
#define WHITE 255

#include <pthread.h>

// These globals are declared here, but defined externally in another .c file
extern unsigned char **input_image;
extern unsigned char **output_image;
extern unsigned char threshold;
extern int width, height;
extern int num_threads;
extern int Kx[3][3];
extern int Ky[3][3];

typedef struct {
    int start_row;
    int end_row;
} thread_args;

void *sobel_filter(void *arg);
