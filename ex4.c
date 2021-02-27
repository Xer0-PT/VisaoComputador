#include <stdio.h>
#include "vc.h"

int main(void)
{
    IVC *image;
    int i;
    int size = 256;
    int color = 0;

    image = vc_image_new(size, size, 1, 1);

    if (image == NULL)
    {
        printf("ERROR -> vc_image_new():\n\tOut of memory!\n");
        getchar();
        return 0;
    }

    for ( i = 0; i < size*size; i++)
    {
        image->data[i] = color;
        color++;
    }
    
    vc_write_image("ex4.pbm", image);

    vc_image_free(image);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
    
}