#include <stdio.h>
#include "../vc.h"

int main(void)
{
    IVC *image;

    image = vc_read_image("../../Images/Additional/pens.ppm");

    if (image == NULL)
    {
        printf("Error -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    vc_rgb_get_red_gray(image);
    vc_image_free(image);

    image = vc_read_image("../../Images/Additional/pens.ppm");

    if (image == NULL)
    {
        printf("Error -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    vc_rgb_get_green_gray(image);
    vc_image_free(image);

    image = vc_read_image("../../Images/Additional/pens.ppm");

    if (image == NULL)
    {
        printf("Error -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    vc_rgb_get_blue_gray(image);
    vc_image_free(image);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}