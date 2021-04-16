#include <stdio.h>
#include "vc.h"

int main(void)
{
    IVC * rgb;
    IVC * gray;
    int i;
    unsigned long int color;

    rgb = vc_read_image("../Images/Classic/airplane.ppm");
    gray = vc_read_image("../Images/OldClassic/cameraman.pgm");

    vc_rgb_negative(rgb);
    vc_gray_negative(gray);

    vc_write_image("negative_rgb.ppm", rgb);
    vc_write_image("negative_gray.pgm", gray);

    vc_image_free(rgb);
    vc_image_free(gray);

    printf("Press any key to exit...\n");
    getchar();
    
    
    return 0;    
}
