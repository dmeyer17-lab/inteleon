#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include "sobel.h"

void *sobel_filter(void *arg)
{
    thread_args *args = (thread_args *)arg;

    for (int i = args->start_row; i <= args->end_row; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1)
            {
                output_image[i][j] = 0;
                continue;
            }

            int mx = 0, my = 0;

            // apply sobel kernels
            for (int si = -1; si <= 1; si++)
            {
                for (int sj = -1; sj <= 1; sj++)
                {
                    int li = i + si;
                    int lj = j + sj;
                    unsigned char pixel = input_image[li][lj];
                    mx += pixel * Kx[si + 1][sj + 1];
                    my += pixel * Ky[si + 1][sj + 1];
                }
            }

            int magnitude = (int)sqrt(mx * mx + my * my);

            // clamp if necessary
            magnitude = (magnitude < 0) ? 0 : magnitude;
            magnitude = (magnitude > 255) ? 255 : magnitude;

            // apply threshold
            output_image[i][j] = (magnitude >= threshold) ? 255 : 0;
        }
    }
    return NULL;
}