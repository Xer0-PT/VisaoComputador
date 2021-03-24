#include <stdio.h>
#include "vc.h"

int main(void)
{
    IVC * image;
    int x,y,i;
    long int pos;

    image = vc_image_new(256, 256, 1, 255);

    if (image == NULL)
    {
        printf("ERROR -> vc_read_image(): \n\tFile not found!\n");
        getchar();
        return 0;
    }


    // Gradiente 1
    for(i = 0; i < image->width*image->height; i++)
    {
        image->data[i] = i;
    }
    vc_write_image("gradiente1.pgm", image);


    // Gradiente 2
    for(x=0; x<image->width; x++)
    {
        for(y=0; y<image->height; y++)
        {
            pos = y * image->bytesperline + x * image->channels;

            image->data[pos] = y;
        }
    }
    vc_write_image("gradiente2.pgm", image);


    // Gradiente 3
    for(x=0; x<image->width; x++)
    {
        for(y=0; y<image->height; y++)
        {
            pos = y * image->bytesperline + x * image->channels;

            image->data[pos] = x / 2 + y/2;
        }
    }
    vc_write_image("gradiente3.pgm", image);

    vc_image_free(image);

    printf("Press any key to exit...\n");
    getchar();
    
    
    return 0;    
}