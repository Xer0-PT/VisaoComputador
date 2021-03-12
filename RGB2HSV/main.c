#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "vc.h"

// Abrir imagem em RGB, converter para HSV e gravar em novo ficheiro
int main(void)
{
	IVC *image;

	image = vc_read_image("HSVTestImage01.ppm");
	if (image == NULL)
	{
		printf("ERROR -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	vc_rgb_to_hsv(image);
	//vc_hsv_segmentation(image, 30, 70, 50, 100, 60, 100); // Amarelo

	vc_write_image("vc-hsv2rgb.ppm", image);

	vc_image_free(image);

	system("cmd /c start FilterGear HSVTestImage01.ppm");
	system("FilterGear vc-hsv2rgb.ppm");

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}