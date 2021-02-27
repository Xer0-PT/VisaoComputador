#include <stdio.h>
#include "vc.h"

int main(void)
{
    IVC *image;
    int i, x, y;
    int width = 256;
    int height = 256;
    int color = 0;
    long int pos;

    image = vc_image_new(width, height, 1, 255);

    if (image == NULL)
    {
        printf("ERROR -> vc_image_new():\n\tOut of memory!\n");
        getchar();
        return 0;
    }

    // 1ª Imagem

/* 
    for (i = 0; i < width*height; i++)
    {
        image->data[i] = color;
        color++;
    }
    vc_write_image("ex4.1.pbm", image);
 */

    // 2ª Imagem
/* 
    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            pos = y * image->bytesperline + x * image->channels;
            image->data[pos] = color;
            color++;
        }
    }
    vc_write_image("ex4.2.pbm", image);
 */

    // 3ª Imagem
    // André Ferreira

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            pos = y * image->bytesperline + x * image->channels;
            image->data[pos] = color/2;
            color++;
        }
        color = x+1; //re-increments level
    }    
    vc_write_image("ex4.3.pbm", image);



    vc_image_free(image);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
    
}