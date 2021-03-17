#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "vc.h"

// Abrir imagem em RGB, converter para HSV e gravar em novo ficheiro
int main(void)
{
	IVC *image;

	image = vc_read_image("./Imagens_Segmentacao/coins.pgm");
	if (image == NULL)
	{
		printf("ERROR -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}
    
    vc_gray_to_binary(image, 128);

	vc_image_free(image);

	system("cmd /c start FilterGear threshold.pgm"); 
	system("FilterGear coins.pgm");

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}