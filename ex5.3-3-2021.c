#include <stdio.h>
#include "vc.h"

int main(void)
{
    IVC *image;

    image = vc_read_image("../Images/Classic/boats.pgm");

    if (image == NULL)
    {
        printf("Error -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    vc_gray_negative(image);

    vc_image_free(image);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}