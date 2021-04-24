//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "vc.h"
#define MAX3(a, b, c) (a > b ? (a > c ? a : c) : (b > c ? b : c))
#define MIN3(a, b, c) (a < b ? (a < c ? a : c) : (b < c ? b : c))

#pragma region Codigo Base
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *)malloc(sizeof(IVC));

	if (image == NULL)
		return NULL;
	if ((levels <= 0) || (levels > 255))
		return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)))
			;
		if (c != '#')
			break;
		do
			c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF)
			break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#')
			ungetc(c, file);
	}

	*t = 0;

	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0)
		{
			channels = 1;
			levels = 1;
		} // Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0)
			channels = 1; // Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0)
			channels = 3; // Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL)
				return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL)
		return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL)
				return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

#pragma endregion

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//          FUN��ES: FUNÇÕES DESENVOLVIDAS EM AULA
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Calcular negativo de imagens em grayscale
int vc_gray_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_gray_negative():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL)
	{
		printf("Error -> vc_gray_negative():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if (srcdst->channels != 1 || srcdst->levels != 255)
	{
		printf("Error -> vc_gray_negative():\n\tNot Gray image!\n");
		getchar();
		return 0;
	}

	//Inverte a imagem Gray
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
		}
	}

	/*      ou

	int i;

	for (i = 0; i < srcdst->bytesperline*srcdst->height; i += srcdst->channels)
	{
		srcdst->data[i] = 255 - srcdst->data[i];
	}
	*/

	return 1;
}

//Calcular negativo de imagens RGB
int vc_rgb_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_rgb_negative():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL)
	{
		printf("Error -> vc_rgb_negative():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if (srcdst->channels != 3)
	{
		printf("Error -> vc_gray_negative():\n\tNot RGB image!\n");
		getchar();
		return 0;
	}

	//Inverte a imagem RGB
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}

	/*   ou
	int i;

	for (i = 0; i < srcdst->bytesperline*srcdst->height; i += srcdst->channels)
	{
		srcdst->data[i] = 255 - srcdst->data[i];
		srcdst->data[i + 1] = 255 - srcdst->data[i + 1];
		srcdst->data[i + 2] = 255 - srcdst->data[i + 2];
	}
	*/

	//////////////////////////////////////////////////////////////////////////////

	/*   ou    (menos eficiente talvez)
	int i;

	for (i = 0; i < srcdst->bytesperline*srcdst->height; i++)
	{
		srcdst->data[i] = 255 - srcdst->data[i];
	}
	*/

	return 1;
}

//Calcular Grayscale pelo Red
int vc_rgb_get_red_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_rgb_get_red_gray():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL)
	{
		printf("Error -> vc_rgb_get_red_gray():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if (srcdst->channels != 3)
	{
		printf("Error -> vc_rgb_get_red_gray():\n\tNot RGB image!\n");
		getchar();
		return 0;
	}

	//Pega no valor vermelho e iguala todos os outros a esse valor
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos + 1] = data[pos]; //Green                        //255 160 90 -> 255 255 255
			data[pos + 2] = data[pos]; //Blue
		}
	}
	return 1;
}

//Calcular Grayscale pelo Green
int vc_rgb_get_green_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_rgb_get_green_gray():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL)
	{
		printf("Error -> vc_rgb_get_green_gray():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if (srcdst->channels != 3)
	{
		printf("Error -> vc_rgb_get_green_gray():\n\tNot RGB image!\n");
		getchar();
		return 0;
	}

	//Pega no valor verde e iguala todos os outros a esse valor
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = data[pos + 1];	   //Red
			data[pos + 2] = data[pos + 1]; //Blue
		}
	}
	return 1;
}

//Calcular Grayscale pelo Blue
int vc_rgb_get_blue_gray(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_rgb_get_blue_gray():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL)
	{
		printf("Error -> vc_rgb_get_blue_gray():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if (srcdst->channels != 3)
	{
		printf("Error -> vc_rgb_get_blue_gray():\n\tNot RGB image!\n");
		getchar();
		return 0;
	}

	//Pega no valor azul e iguala todos os outros a esse valor
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = data[pos + 2];	   //Red
			data[pos + 1] = data[pos + 2]; //Green
		}
	}
	return 1;
}

//Calcular RGB para Grey com intensidades de cor
int vc_rgb_to_gray(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float r, g, b;

	//verificação de erros

	if (src == NULL)
	{
		printf("Error -> vc_rbg_to_gray():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Error -> vc_rbg_to_gray():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height))
	{
		printf("Error -> vc_rbg_to_gray():\n\tError in dimensions!\n");
		getchar();
		return 0;
	}

	if ((src->channels != 3) || (dst->channels != 1))
	{
		printf("Error -> vc_rbg_to_gray():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			r = (float)datasrc[pos_src];
			g = (float)datasrc[pos_src + 1];
			b = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((r * 0.299) + (g * 0.587) + (b * 0.114));

			//versão gimp
			//datadst[pos_dst] = (unsigned char) ((rf+ gf + bf)/ 3.0);
		}
	}

	return 1;
}

//Converter imagem do espaço RGB para o espaço HSV
int vc_rgb_to_hsv(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	float r, g, b, hue, saturation, value;
	/*
	Hue - Tonalidade ou Matiz
	Saturation - Saturacao
	Value - Valor
	*/
	float rgb_max, rgb_min;
	int i, size;

	// Verificacao de erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_rgb_to_hsv():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
	{
		printf("Error -> vc_rgb_to_hsv():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((srcdst->channels != 3))
	{
		printf("Error -> vc_rgb_to_hsv():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		r = (float)data[i];
		g = (float)data[i + 1];
		b = (float)data[i + 2];

		// Calcula valores maximo e minimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b)); //ou ver define no topo da pagina MAX3
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max; // Valor maior de todos os componentes
		if (value == 0.0)
		{
			hue = 0.0;
			saturation = 0.0;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * (float)255.0;

			if (saturation == 0.0) //Saturacao e zero quando o valor e igual a zero, ou seja, preto;
								   //ou quando o maximo = minimo em que so acontece quando r, g e b sao iguais, ou seja, cinza
			{
				hue = 0.0; // Como na saturacao, podemos atribuir arbitrariamente a Matiz de cores em tons de cinza o número zero para evitar variáveis indefinidas.
			}
			else
			{
				// Hue toma valores entre [0,360]                 //Seguindo a definição de Hue do ppt
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min); //Se, (Max = R) e (G ≥ B), então Hue ← 60 * (G - B) / (Max - Min)
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min); //Se, (Max = R) e (B > G), então Hue ← 360 + 60 * (G - B) / (Max - Min)
				}
				else if (rgb_max == g)
				{
					hue = 120 + 60 * (b - r) / (rgb_max - rgb_min); //Se Max = G, então Hue ← 120 + 60 * (B - R) / (Max - Min)
				}
				else /* rgb_max == b*/
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min); //Se Max = B, então Hue ← 240 + 60 * (R - G) / (Max - Min)
				}
			}
		}

		// Atribui valores entre [0,255]
		data[i] = (unsigned char)(hue / 360.0 * 255.0);
		data[i + 1] = (unsigned char)(saturation);
		data[i + 2] = (unsigned char)(value);
	}

	return 1;
}

//Selecionar cor HSV e retornar imagem binária com essa cor
int vc_hsv_segmentation(IVC *srcdst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int i, size;
	int h, s, v;

	//Verificacao de erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_hsv_segmentation():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
	{
		printf("Error -> vc_hsv_segmentation():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((srcdst->channels != 3))
	{
		printf("Error -> vc_hsv_segmentation():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		h = ((float)data[i]) / 255.0f * 360.0f;
		s = ((float)data[i + 1]) / 255.0f * 100.0f;
		v = ((float)data[i + 2]) / 255.0f * 100.0f;

		if ((h > hmin) && (h <= hmax) &&
			(s > smin) && (s <= smax) &&
			(v > vmin) && (v >= vmin))
		{
			data[i] = 255;
			data[i + 1] = 255;
			data[i + 2] = 255;
		}
		else
		{
			data[i] = 0;
			data[i + 1] = 0;
			data[i + 2] = 0;
		}
	}

	return 1;
}

//Binarização, por thresholding manual, de uma imagem em tons de cinzento.
int vc_gray_to_binary(IVC *srcdst, int threshold)
{ /*Thershold tem como objetivo isolar regiões de pixéis de uma imagem
	onde possuem uma intensidade luminosa distinta (destaques da imagem)
	que podem ser definidos manualmente ou automático*/
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//Verificacao de erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_gray_to_binary():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
	{
		printf("Error -> vc_gray_to_binary():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((srcdst->channels != 1))
	{
		printf("Error -> vc_gray_to_binary():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			if (data[pos] > threshold)
			//Para todos os valores superiores ao threshold destacamos em branco
			{
				data[pos] = 255;
			}
			//Tudo o resto fica a preto
			else
			{
				data[pos] = 0;
			}
		}
	}
	return 1;
}

//Binarização, por thresholding através da média global, de uma imagem em tons de cinzento
int vc_gray_to_fglobal_mean(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	int threshold = 0;

	//Verificacao de erros
	if (srcdst == NULL)
	{
		printf("Error -> vc_gray_to_binary_global_mean():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
	{
		printf("Error -> vc_gray_to_binary_global_mean():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((srcdst->channels != 1))
	{
		printf("Error -> vc_gray_to_binary_global_mean():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			threshold += data[pos]; //A guardar todas as intensidades dos pixeis
		}
	}

	threshold = (threshold) / (x * y); //Media das intensidades de todos os pixeis da imagem

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			pos = y * bytesperline + x * channels; //Posicao dos pixeis

			if (data[pos] > threshold) //Cada posicao com intensidade acima da média fica a branco
			{
				data[pos] = 255;
			}
			//Abaixo da media fica a preto
			else
			{
				data[pos] = 0;
			}
		}
	}
	return 1;
}

//Binarização, por thresholding adaptativo midpoint, de uma imagem em tons de cinzento
int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	int max, min;
	long int pos, posk;
	unsigned char threshold;

	//Verificacao de erros
	if (src == NULL)
	{
		printf("Error -> vc_gray_to_bynary_midpoint():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Error -> vc_gray_to_bynary_midpoint():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
	{
		printf("Error -> vc_gray_to_bynary_midpoint():\n\tError in dimensions!\n");
		getchar();
		return 0;
	}

	if (channels != 1)
	{
		printf("Error -> vc_gray_to_bynary_midpoint():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++) //Dois ciclos para navegar em cada pixel da imagem
		{
			pos = y * bytesperline + x * channels; //Posição do pixel central

			max = 0;
			min = 255;

			// NxM Vizinhos
			for (ky = -offset; ky <= offset; ky++) //Dois ciclos para aceder à informação à volta do pixel central (do Kernel/Margem)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width)) //Confirmar se a informação está dentro das margens da imagem
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels; //Posição do kernel (conforme o numero de pixeis à volta do pixel principal salta linhas e colunas)

						if (datasrc[posk] > max)
							max = datasrc[posk]; //Cálculo do máximo
						if (datasrc[posk] < min)
							min = datasrc[posk]; //Cálculo do mínimo
					}
				}
			}

			threshold = (unsigned char)((float)(max + min) / (float)2); //Média dos valores

			if (datasrc[pos] > threshold)
				datadst[pos] = 255; //Aplicação do threshold
			else
				datadst[pos] = 0;
		}
	}

	return 1;
}

//Binarização, por thresholding adaptativo bernsen, de uma imagem em tons de cinzento
int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel, int contrast)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	int max, min;
	long int pos, posk;
	unsigned char threshold;

	//Verificacao de erros
	if (src == NULL)
	{
		printf("Error -> vc_gray_to_binary_bernsen():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Error -> vc_gray_to_binary_bernsen():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
	{
		printf("Error -> vc_gray_to_binary_bernsen():\n\tError in dimensions!\n");
		getchar();
		return 0;
	}

	if (channels != 1)
	{
		printf("Error -> vc_gray_to_binary_bernsen():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++) //Dois ciclos para navegar em cada pixel da imagem
		{
			pos = y * bytesperline + x * channels; //Posição do pixel central

			max = 0;
			min = 255;

			// NxM Vizinhos
			for (ky = -offset; ky <= offset; ky++) //Dois ciclos para aceder à informação à volta do pixel central (do Kernel/Margem)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width)) //Confirmar se a informação está dentro das margens da imagem
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels; //Posição do kernel (conforme o numero de pixeis à volta do pixel principal salta linhas e colunas)

						if (datasrc[posk] > max)
							max = datasrc[posk]; //Cálculo do máximo
						if (datasrc[posk] < min)
							min = datasrc[posk]; //Cálculo do mínimo
					}
				}
			}

			if ((max - min) < contrast)
				threshold = (unsigned char)((float)(src->levels) / (float)2); //s𝑒 𝑣𝑚𝑎𝑥 − 𝑣𝑚𝑖𝑛 < 𝐶𝑚𝑖𝑛 (contraste)
			else
				threshold = (unsigned char)((float)(max + min) / (float)2); //Caso contrário utilizaro threshold como midpoint

			if (datasrc[pos] > threshold)
				datadst[pos] = 255; //Aplicação do threshold
			else
				datadst[pos] = 0;
		}
	}

	return 1;
}

//Binarização, por thresholding adaptativo Niblack, de uma imagem em tons de cinzento
int vc_gray_to_binary_niblack(IVC *src, IVC *dst, int kernel, float kvalue)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	float mean, stdeviation, sum, total;
	long int pos, posk;
	unsigned char threshold;

	//Verificacao de erros

	if (src == NULL)
	{
		printf("Error -> vc_gray_to_binary_niblack():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Error -> vc_gray_to_binary_niblack():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
	{
		printf("Error -> vc_gray_to_binary_niblack():\n\tError in dimensions!\n");
		getchar();
		return 0;
	}

	if (channels != 1)
	{
		printf("Error -> vc_gray_to_binary_niblack():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++) //Dois ciclos para navegar em cada pixel da imagem
		{
			pos = y * bytesperline + x * channels; //Posição do pixel central
			total = 0;
			sum = 0;
			stdeviation = 0;
			mean = 0;

			// NxM Vizinhos
			for (ky = -offset; ky <= offset; ky++) //Dois ciclos para aceder à informação à volta do pixel central (do Kernel/Margem)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width)) //Confirmar se a informação está dentro das margens da imagem
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels; //Posição do kernel (conforme o numero de pixeis à volta do pixel principal salta linhas e colunas)
						sum += (float)datasrc[posk];
						total++;
					}
				}
			}

			mean = (sum / total); //Cálculo da média

			for (ky = -offset; ky <= offset; ky++) //Dois ciclos para aceder à informação à volta do pixel central (do Kernel/Margem)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width)) //Confirmar se a informação está dentro das margens da imagem
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels; //Posição do kernel (conforme o numero de pixeis à volta do pixel principal salta linhas e colunas)
						stdeviation += pow((float)(datasrc[posk] - mean), 2); //Somatório de todos os desvios padrão dentro do kernel
					}
				}
			}

			stdeviation = stdeviation / total; //Cálculo do desvio padrão (divisão pelo número de pixeis)
			stdeviation = sqrt(stdeviation);  //Cálculo do desvio padrão (raiz quadrada)

			threshold = (unsigned char)(mean + stdeviation * kvalue); //Cálculo do threshold

			if (datasrc[pos] > threshold)
				datadst[pos] = 255; //Aplicação do threshold
			else
				datadst[pos] = 0;
		}
	}

	return 1;
}

//Operadores Morfológicos (Binários): Dilatação
int vc_binary_dilate(IVC *src, IVC *dst, int kernel)
{
	/* A dilatação consiste em adicionar pixéis aos limites de uma região segmentada, 
	aumentando assim a sua área e preenchendo algumas zonas no seu interior	*/

	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	float mean, stdeviation, sum, total;
	long int pos, posk;
	unsigned char threshold;
	int max, min;
	int aux = 0;

	
	//Verificacao de erros
	if (src == NULL)
	{
		printf("Error -> vc_gray_to_binary_bernsen():\n\tImage is empy!\n");
		getchar();
		return 0;
	}

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
	{
		printf("Error -> vc_gray_to_binary_bernsen():\n\tImage Dimensions or data are missing!\n");
		getchar();
		return 0;
	}

	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels))
	{
		printf("Error -> vc_gray_to_binary_bernsen():\n\tError in dimensions!\n");
		getchar();
		return 0;
	}

	if (channels != 1)
	{
		printf("Error -> vc_gray_to_binary_bernsen():\n\tImages with incorrect format!\n");
		getchar();
		return 0;
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++) //Dois ciclos para navegar em cada pixel da imagem
		{
			pos = y * bytesperline + x * channels; //Posição do pixel central

			max = datasrc[pos];
			min = datasrc[pos];
			aux = 0;

			// NxM Vizinhos
			for (ky = -offset; ky <= offset; ky++) //Dois ciclos para aceder à informação à volta do pixel central (do Kernel/Margem)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < height) && (x + kx >= 0) && (x + kx < width)) //Confirmar se a informação está dentro das margens da imagem
					{
						posk = (y + ky) * bytesperline + (x + kx) * channels; //Posição do kernel (conforme o numero de pixeis à volta do pixel principal salta linhas e colunas)

						if (datasrc[posk] == 255) aux = 255; //Se no kernel houver um pixel segmentado, o limite fica a branco também
					}
				}
			}
			datadst[pos] = aux;
		}
	}

	return 1;
}