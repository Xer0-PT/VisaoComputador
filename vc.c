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
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
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
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
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

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
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
			if((countbits > 8) || (x == width - 1))
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

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
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
			if((countbits > 8) || (x == width - 1))
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
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
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
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
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
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
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
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
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
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
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
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
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

/*********************************
*  Funções implementadas em aula *
*********************************/

int vc_gray_negative(IVC *srcdst)
{
	if(srcdst == NULL) return 0;

	for (int i = 0; i < srcdst->bytesperline*srcdst->width; i++)
	{
		srcdst->data[i] = 255 - srcdst->data[i];
	}
	
	vc_write_image("ex5.pbm", srcdst);

	return 1;
}

int vc_rgb_negative(IVC *srcdst)
{
	if(srcdst == NULL) return 0;

	for (int i = 0; i < srcdst->bytesperline*srcdst->width; i += srcdst->channels)
	{
		srcdst->data[i] = 255 - srcdst->data[i];
		srcdst->data[i + 1] = 255 - srcdst->data[i + 1];
		srcdst->data[i + 2] = 255 - srcdst->data[i + 2];
	}

	vc_write_image("ex5.1.pbm", srcdst);

	return 1;
}

int vc_rgb_get_red_gray(IVC *srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int widht = srcdst->width;
	int heigh = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y, red;
	long int pos;

	if(srcdst == NULL) return 0;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) {

		return 0;
	}

	if (channels != 3) {

		return 0;
	}

	for (y = 0; y < heigh; y++) {
		for (x = 0; x < widht; x++) {

			pos = y * bytesperline + x * channels;

			red = data[pos];
			data[pos + 1] = red;
			data[pos + 2] = red;
		}
	}

	vc_write_image("red_gray.ppm", srcdst);

	return 1;
}

int vc_rgb_get_green_gray(IVC *srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int widht = srcdst->width;
	int heigh = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y, green;
	long int pos;

	if(srcdst == NULL) return 0;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) {

		return 0;
	}

	if (channels != 3) {

		return 0;
	}

	for (y = 0; y < heigh; y++) {
		for (x = 0; x < widht; x++) {

			pos = y * bytesperline + x * channels;

			green = data[pos + 1];
			data[pos] = green;
			data[pos + 2] = green;
		}
	}

	vc_write_image("green_gray.ppm", srcdst);

	return 1;
}

int vc_rgb_get_blue_gray(IVC *srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int widht = srcdst->width;
	int heigh = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y, blue;
	long int pos;

	if(srcdst == NULL) return 0;

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) {

		return 0;
	}

	if (channels != 3) {

		return 0;
	}

	for (y = 0; y < heigh; y++) {
		for (x = 0; x < widht; x++) {

			pos = y * bytesperline + x * channels;

			blue = data[pos + 2];
			data[pos] = blue;
			data[pos + 1] = blue;
		}
	}

	vc_write_image("blue_gray.ppm", srcdst);

	return 1;
}

// Converte uma imagem RGB numa imagem em tons de cinza
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
	float rf, gf, bf;

	// Verifica‹o de Erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;
	// if (channels != 3) return 0;

	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}
	return 1;
}

// Converter de RGB para HSV
int vc_rgb_to_hsv(IVC *srcdst)
{
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int i, size;
    float r, g, b, hue, saturation, val, maxrgb, minrgb;
    
    //VerificaÁ„o de erros
    if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
    if (channels != 3) return 0;
    
    size = width * height * channels;
    
    for (i = 0; i < size; i = i + channels)
    {
        r = (float)data[i];
        g = (float)data[i + 1];
        b = (float)data[i + 2];
        
        
        maxrgb = (r > g ? (r > b ? r : b) : (g > b ? g : b)); // calcular maximo no rgb
        minrgb = (r < g ? (r < b ? r : b) : (g < b ? g : b)); // calcular minimo no rgb
        val = maxrgb;
        
        if (val == 0.0f)
        {
            saturation = 0.0f;
            hue = 0.0f;
        }
        else
        {
            // Saturation toma valores entre [0, 255]
            saturation = ((maxrgb - minrgb) / maxrgb) * 255.0f;
            if (saturation == 0.0f)
            {
                hue = 0.0f;
                val = 0.0f;
            }
            else if (maxrgb == r)
            {
                if (g >= b)
                {
                    hue = 60.0f * (g - b) / (maxrgb - minrgb);
                }
                else
                {
                    hue = 360.0f + 60.0f * (g - b) / (maxrgb - minrgb);
                }
            }
            else if (maxrgb == g) {
                hue = 120.0f + 60.0f * (b - r) / (maxrgb - minrgb);
            }
            else if (maxrgb == b) {
                hue = 240.0f + 60.0f * (r - g) / (maxrgb - minrgb);
            }
        }
        data[i] = (unsigned char)(hue / 360.0f * 255.0f);
        data[i + 1] = (unsigned char)(saturation);
        data[i + 2] = (unsigned char)(val);
    }
    return 1;
}