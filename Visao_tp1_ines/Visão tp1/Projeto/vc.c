//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2019/2020
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//          [  BRUNO LOPES - a16970@alunos.ipca.pt  ]
//          [  INES Alves  - a17016@alunos.ipca.pt  ]
//          [  RUI COSTA   - a17005@alunos.ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "vc.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#pragma region New_Free  // Estas Funções são da autoria do professor Duarte Duque (dduque@ipca.pt)

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
#pragma endregion

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#pragma region Leitura_Escrita  // Estas Funções são da autoria do professor Duarte Duque (dduque@ipca.pt)

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

#pragma endregion


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           FUNÇÕES: Processamento da Imagem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma region Filtro_Gaussiano  // Criação e aplicação do filtro gaussiano

// Cria a matriz para ser aplicada no filtro gaussiano
//  radius ->  raio do kernel a aplicar o filtro
//  sigma  ->  desvio padrao do filtro
//  return --> filtro a aplicar
FILTER* vc_filter_create_gauss(int radius, double sigma) {
	// Alocar memoria para o filtro
	FILTER *filter = (FILTER*) malloc(sizeof(FILTER));
	filter->radius = radius;

	// Tamanho do kernel
	int kernel = 2*radius+1;

	// Alocação de memoria para a matriz do filtro
	filter->matrix = (double**) malloc(kernel * sizeof(double*));
	for(int i = 0; i < kernel; i++)
		filter->matrix[i] = (double*) malloc(kernel * sizeof(double));

	// Calculo dos valores da matriz
	double sum = 0.0;
	for(int x = -radius; x <= radius; x++){
		for(int y = -radius; y <= radius; y++) {
			filter->matrix[x+radius][y+radius] = (1.0 / (2 * 3.14159265 * sigma * sigma)*exp(-(x*x+y*y)/(2 * sigma * sigma)));
			sum += filter->matrix[x+radius][y+radius];
		}
	}

	// Normalizar os elementos da matriz
	for(int x = 0; x < kernel; x++)
		for(int y = 0; y < kernel; y++)
			filter->matrix[x][y] /= sum;

	return filter;
}

// Aplica o filtro gaussiano a uma imagem rgb
//  src    ->  imagem rgb onde será aplicado o filtro
//  radius ->  raio do kernel a aplicar o filtro
//  sigma  ->  desvio padrao do filtro
//  return --> imagem apos aplicar o filtro gaussiano
IVC* vc_apply_filter(IVC *src, int radius, double sigma) {
	
	// Variaveis com parametros da imagem para auxiliar a utilização
	int byteperline = src->width*src->channels;
	int channels = src->channels;
	
	IVC *dst = vc_image_new(src->width, src->height, src->channels, src->levels);   // Imagem de destino
	FILTER* filter = vc_filter_create_gauss(radius, sigma);                         // Filtro a aplicar
	
	long int pos, pos2;                                                             // Variaveis para auxiliar na movimentação ao longo da imagem

	// Ciclos em x e y para movimentar pela imagem
	for(int y = 0; y < src->height; y++){
		for(int x = 0; x < src->width; x++){

			pos = y * byteperline + x * channels;  // Calculo da posição atual

			unsigned char res[3] = {0};            // Variavel para o calculo do resultado do filtro para cada uma das 3 componentes de um pixel
			double fil;                            // Valor de uma posição da matriz do filtro ( auxiliar na movimentação ao longo da matriz )

			// Ciclos para movimentar ao longo de toda a matriz do filtro
			for(int i = -filter->radius; i <= filter->radius; i++) {
				for(int j = -filter->radius; j <= filter->radius; j++) {

					if(y+i > src->height || x+j > src->width || y+i < 0 || x+j < 0) continue;  // Caso os limites da imagem serem ultrapassados estes valores não devem ser calculados

					fil = filter->matrix[i+filter->radius][j+filter->radius];                  // Valor da respetiva posição da matriz do filtro
					
					pos2 = (y+i) * byteperline + (x+j) * channels;                             // Calculo da posição atual na imagem consoante a posição da matriz

					res[0] += fil * src->data[pos2];                                           // Calculo da componente vermelha
					res[1] += fil * src->data[pos2+1];                                         // Calculo da componente verde
					res[2] += fil * src->data[pos2+2];                                         // Calculo da componente azul
				}
			}

			// Definição das componentes do pixel apos ser aplicado o calculo do filtro
			dst->data[pos] = res[0];
			dst->data[pos+1] = res[1];
			dst->data[pos+2] = res[2];
		}
	}

	free(filter);

	return dst;
}

#pragma endregion

#pragma region RGB2G_G2B_GES     // Converção de imagem

// Converter uma imagem rgb numa imagem grayscale
//  src -> imagem rgb
//  dst -> imagem em tons de cinzento
int vc_rgb_to_gray(IVC *src, IVC *dst){
	unsigned char *datasrc = (unsigned char*) src->data;
	int bytesperline_src = src->bytesperline;
	int channels_src = src->channels;
	
	unsigned char *datadst = (unsigned char*) dst->data;
	int bytesperline_dst = dst->bytesperline;
	int channels_dst = dst->channels;
	
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y<height; y++){
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			
			rf = (float) datasrc[pos_src];
			gf = (float) datasrc[pos_src + 1];
			bf = (float) datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char) ((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
		
	}

	return 1;
}

// Converter uma imagem em tons de cinzento numa imagem binaria
//  src       -> imagem em tons de cinzento
//  dst       -> imagem binaria
//  threshold -> threshold a utilizar na conversao
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold) {

	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline = src->width*src->channels;
	int channels = src->channels;
	
	unsigned char *datadst = (unsigned char*)dst->data;
	
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y*bytesperline + x*channels;

			if (datasrc[pos] > threshold) {
				datadst[pos] = 0;
			}
			else
			{
				datadst[pos] = 255;
			}

		}
	}

	return 1;
}

// Deteção de contornos com algoritmo de Sobel
//  src -> imagem em tons de cinzento
//  dst -> imagem dos contornos
//  th  -> intensidade da deteção (0.00 -> 1.00)
int vc_gray_edge_sobel(IVC *src, IVC *dst, float th) {

	unsigned char *data = (unsigned char *)src->data;
	int byteperline = src->width*src->channels;
	int channels = src->channels;

	int width = src->width;
	int height = src->height;
	int x, y, i;
	long int pos, posA, posB, posC, posD, posE, posF, posG, posH;
	double mx, my;
	int hist[255] = {}, histmax, histth;

	if ((width <= 0) || (height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	for (y = 1; y < height-1; y++)
	{
		for (x = 1; x < width-1; x++)
		{
			// É possovel simplificar os calculos por linha visto que são pixeis seguidos e a imagem so tem um canal de cor
			posA = (y - 1)* byteperline + (x - 1) * channels;
			posB = posA + 1;
			posC = posA + 2;
			
			pos = y * byteperline + x * channels;
			posD = pos - 1;
			posE = pos + 1;
			
			posF = (y + 1)* byteperline + (x - 1)* channels;
			posG = posF + 1;
			posH = posF + 2;

			mx = ((-1 * data[posA]) + (1 * data[posC]) + (-2 * data[posD]) + (2 * data[posE]) + (-1 * data[posF]) + (1 * data[posH])) / 3;
			my = ((-1 * data[posA]) + (1 * data[posF]) + (-2 * data[posB]) + (2 * data[posG]) + (-1 * data[posC]) + (1 * data[posH])) / 3;

			dst->data[pos] = sqrt((mx * mx) + (my * my));
		}
	}

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			hist[dst->data[y * byteperline + x * channels]]++;
		}
	}

	histmax = 0;

	for(i = 0; i < 256; i++){
		histmax += hist[i];
		if ( histmax >= (((float)(height*width)) * th)) break;
	}
	histth = i;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * byteperline + x * channels;

			if (dst->data[pos] >= histth) dst->data[pos] = 255;
			else dst->data[pos] = 0;
		}
	}
	return 1;
}

#pragma endregion

#pragma region Operadores_Morfologicos  // Operam a forma dos objetos da imagem

// Aplica uma dilatação aos elementos de uma imagem binaria
//  # Utilizamos um kernel quadrado devido a sua eficiencia em questoes de processamento e a aplicação dada a esta
//  $ função neste caso em especifico
//  src -> imagem binaria original
//  dst -> imagem binaria dilatada
//  kernel -> tamanho do kernel a utilizar ( numero impar )
int vc_binary_dilate(IVC *src, IVC *dst, int kernel) {

	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline = src->width*src->channels;
	int channels = src->channels;
	
	unsigned char *datadst = (unsigned char*)dst->data;
	
	int width = src->width;
	int height = src->height;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	int verifica;
	kernel *= 0.5;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_dst = y*bytesperline + x*channels;

			verifica = 0;

			for (y2 = y-kernel; y2 <= y+kernel; y2++)
			{
				for (x2 = x-kernel; x2 <= x+kernel; x2++)
				{
					if (y2 < 0 || y2 > height || x2 < 0 || x2 > width) continue;  // Não exceder os limites da imagem

					pos_src = y2*bytesperline + x2*channels;
					if (datasrc[pos_src] == 255) { verifica = 1; break;} // Se algum dos pixeis do kernel for verificado ja não e necessario percorrer o resto do kernel
				}
				if (verifica) break;                                     // Parar o ciclo quando algum pixel do kernel for verificado
			}

			if (verifica == 1) { datadst[pos_dst] = 255; }
			else { datadst[pos_dst] = 0; }
		}
	}

	return 1;
}

// Aplica uma erosão aos elementos de uma imagem binaria
//  # Utilizamos um kernel quadrado devido a sua eficiencia em questoes de processamento e a aplicação dada a esta 
//  # função neste caso em especifico
//  src -> imagem binaria original
//  dst -> imagem binaria erodida
//  kernel -> tamanho do kernel a utilizar ( numero impar )
int vc_binary_erode(IVC *src, IVC *dst, int kernel) {
	
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline = src->width*src->channels;
	int channels = src->channels;
	
	unsigned char *datadst = (unsigned char*)dst->data;
	
	int width = src->width;
	int height = src->height;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	int verifica;
	kernel *= 0.5;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_dst = y*bytesperline + x*channels;

			verifica = 0;

			for (y2 = y - kernel; y2 <= y + kernel; y2++)
			{
				for (x2 = x - kernel; x2 <= x + kernel; x2++)
				{
					if (y2 < 0 || y2 > height || x2 < 0 || x2 > width) continue;  // Não exceder os limites da imagem

					pos_src = y2*bytesperline + x2*channels;
					if (datasrc[pos_src] == 0) { verifica = 1; break;}   // Se algum dos pixeis do kernel for verificado ja não e necessario percorrer o resto do kernel
				}
				if (verifica) break;                                     // Parar o ciclo quando algum pixel do kernel for verificado
			}

			if (verifica == 1) { datadst[pos_dst] = 0; }
			else { datadst[pos_dst] = 255; }

		}
	}

	return 1;
}

// Aplica uma operação de abertura nos elementos da imagem
//  src     -> imagem original
//  dst     -> imagem final
//  kerode  -> kernel a aplicar na erosão
//  kdilate -> kernel a aplicar na dilatação
int vc_binary_open(IVC *src, IVC *dst, int kerode, int kdilate) {

	int verifica=1;

	IVC *dstTemp = vc_image_new(src->width, src->height, src->channels, src->levels);

	verifica &= vc_binary_erode(src, dstTemp, kerode);
	verifica &= vc_binary_dilate(dstTemp, dst, kdilate);

	vc_image_free(dstTemp);

	return verifica;
}

// Aplica uma operação de fecho nos elementos da imagem
//  src     -> imagem original
//  dst     -> imagem final
//  kdilate -> kernel a aplicar na dilatação
//  kerode  -> kernel a aplicar na erosão
int vc_binary_close(IVC *src, IVC *dst, int kdilate, int kerode) {

	int verifica = 1;
	
	IVC *dstTemp = vc_image_new(src->width, src->height, src->channels, src->levels);

	verifica &= vc_binary_dilate(src, dstTemp, kdilate);
	verifica &= vc_binary_erode(dstTemp, dst, kerode);

	vc_image_free(dstTemp);

	return verifica;
}

#pragma endregion

#pragma region Etiquetagem  // Etiquetagem dos objetos de uma imagem

// Etiquetar os elementos da imagem binaria
//  src      -> imagem a analizar
//  dst      -> imagem etiquetada
//  &nlabels -> numero de labels encontrados na imagem
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels){
	
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline = src->bytesperline;
	int channels = src->channels;

	int width = src->width;
	int height = src->height;
	
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;

	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1;                          // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs;                             // Apontador para array de blobs (objectos) que sera retornado desta funcao.

	// Verificacao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binaria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixeis de plano de fundo devem obrigatoriamente ter valor 0
	// Todos os pixeis de primeiro plano devem obrigatoriamente ter valor 255
	// Serao atribuidas etiquetas no intervalo [1,254]
	// Este algoritmo esta assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binaria
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

													// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A esta marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B esta marcado, e e menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C esta marcado, e e menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D esta marcado, e e menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do numero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se nao ha blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

// Preenche as restantes informação acreca de um cionjunto de blobs
//  # Alguns dos dados dos blobs que eram obtidos nesta função foram removidos
//  # deixando assim apenas o calculo da area e da caixa delimitadora ( apenas estes são necessários para o efeito )
//  src    -> imagem etiquetada
//  blobs  -> blobs cujo os dados serão preenchidos
//  nblobs -> numero de blobs a analizar
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs){
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;

	// Verificacao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta area de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// area
					blobs[i].area++;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;
	}

	return 1;
}

#pragma endregion

#pragma region Analize  // Cruzamento da quantidade de pixeis em x e y

// Cria uma imagem apartir do cruzamento da quantidade de pixeis brancos em x e y acima do valor medio
//  src    -> imagem a analizar
//  x      -> valor de x onde começar a analizar
//  y      -> valor de y onde começar a analizar
//  width  -> largura a analizar
//  height -> altura a analizar
//  return --> retorna uma imagem resultante do cruzamento da quantidade de pixeis brancos
IVC* vc_point_amount_image (IVC *src, int x, int y, int width, int height){

    unsigned char *data = (unsigned char *)src->data;
	int byteperline = src->width*src->channels;
	int channels = src->channels;
    
	int i, j;
    long int pos;

    int pixelsx[src->width];   // numero de pixeis por coluna
    int pixelsy[src->height];  // numero de pixeis por linha
    int mx, my;                // medias de x e y
    int tot=0;                 // total de pixeis brancos

    for (i = 0; i < src->width; i++) pixelsx[i] = 0;  // Preenchimento de todas as colunas com valor 0

    for (j = 0; j < src->height; j++) pixelsy[j] = 0; // Preenchimento de todas as linhas com valor 0

	// Percorrer toda a area pretendida e realizar as contagens
    for (i = x; i < width; i++){
        for (j = y; j < height; j++)
        {
            pos = j * byteperline + i * channels;
			
			if(data[pos] == 255) {  // Caso seja encontrado um pixel branco este é contado
				tot++;
				pixelsx[i]++;
				pixelsy[j]++;
			}
        }
    }

	// Calculo dos valores medios em x e y
    mx=tot/width;
    my=tot/height;

	// Criação da imagem de destino
    IVC* dst = vc_image_new(src->width, src->height, 1, 255);

	// Percorrer toda a imagem de destino e preencher os devidos valores
    for (i = 0; i < src->width; i++){
        for (j = 0; j < src->height; j++)
        {
            pos = j * byteperline + i * channels;

			if (pixelsx[i] > mx*0.7 && pixelsy[j] > my ) dst->data[pos] = 255;  // Caso a componente x e y estejam acima dos valores desejados o pixel e preenchido a branco
			else dst->data[pos] = 0;                                            // caso contrario este e colocado a preto
        }
    }

    return dst;
} 

#pragma endregion

#pragma region Deteção  // Deteção da matricula e seus caracteres e desenho da caixa delimitadora

// Realiza a deteção do local onde se encontra a matricula
//  src    -> imagem original
//  return --> retorna um blob com os dados de onde se encontra a matricula e sua area
OVC vc_plate_blob(IVC* src){

	// Variaveis necessarias na deteção dos blobs
	int nlabels;
	OVC blob;
	OVC* blobs;
    
	// Imagens auxiliares
    IVC* src_2 = vc_image_new(src->width, src->height, 1, 255);
	IVC* temp  = vc_image_new(src->width, src->height, 1, 255);

    src = vc_apply_filter(src, 6, 3); // Aplicar um filtro gaussiano para evitar a deteção de pequenos detalhes desnecessarios na posterior deteção de contornos

    vc_rgb_to_gray(src, src_2);       // Converter a imagem rgb para uma imagem em tons de cinzento


	// # 1a deteçao ( determinar uma posição aproximada da matricula )

	vc_gray_edge_sobel(src_2, temp, 0.96);                                     // Realizar a deteção de contornos
    IVC* temp_2 = vc_point_amount_image(temp, 0, 0, src->width, src->height);  // Gerar a imagem do cruzamento de pixeis brancos
    vc_binary_close(temp_2, temp, 5, 5);                                       // Realizamos uma operação de close para que seja possivel a obtenção de areas mais consideraveis a partir da imagem do cruzamento de pixeis
    blobs = vc_binary_blob_labelling(temp, temp_2, &nlabels);                  // Deteção e etiquetagem dos elementos presentes na imagem do cruzamento de pixeis melhorada 
    vc_binary_blob_info(temp_2, blobs, nlabels);                               // Preenchimeto dos parametros das etiquetas

	// Deteção do blob que contem a posição aproximada da matricula apartir da sua proporção e area
    blob.area = 0; 
    for (int i = 0; i<nlabels;i++){
        if (((float)blobs[i].width/blobs[i].height) <= 5.50 && ((float)blobs[i].width/blobs[i].height) >= 3.40){
            if(blob.area < blobs[i].area) blob = blobs[i];
        }
    }


	// # 2a deteçao ( melhoramento da deteção da posiçao da matricula )

    vc_gray_edge_sobel(src_2, temp, 0.925);                    // Realizar a deteção de contornos com maior tolerancia que anteriormente
    temp_2 = vc_point_amount_image(temp, blob.x-(blob.height/4), blob.y-(blob.height/4), blob.x+blob.width+(blob.height/4), blob.y+blob.height+(blob.height/4)); // Gerar a imagem do cruzamento de pixeis brancos apenas para uma area ligeiramente maior do que a ja encontrada na deteção anterior
    vc_binary_close(temp_2, temp, 3, 5);                       // Realizamos uma operação de close para que seja possivel a obtenção de areas mais consideraveis a partir da imagem do cruzamento de pixeis
    blobs = vc_binary_blob_labelling(temp, temp_2, &nlabels);  // Deteção e etiquetagem dos elementos presentes na imagem do cruzamento de pixeis melhorada 
    vc_binary_blob_info(temp_2, blobs, nlabels);               // Preenchimeto dos parametros das etiquetas

	// Deteção do blob que contem a posição da matricula agora apenas pela area, visto que agora apenas poderão existir pequenos artefactos alem da matricula
	blob.area = 0; 
    for (int i = 0; i<nlabels; i++){
        if(blob.area < blobs[i].area) blob = blobs[i];
    }

	free(blobs);
	vc_image_free(src_2);
	vc_image_free(temp);
	vc_image_free(temp_2);

	return blob;
}

// Realiza a deteção dos carateres da matricula
//  src       -> imagem original
//  plate     -> blob da matricula
//  *nletters -> numero de carateres encontrados
//  return    --> retorna os blobs que contem os carateres da matricula
OVC* vc_letters_blobs(IVC* src, OVC plate, int* nletters){

	// variaveis necessarias para deteção dos blobs
    int nlabels;
    OVC* blobs;
    OVC* letters = (OVC*) calloc(6, sizeof(OVC));
    
	// Imagens auxiliares
	IVC* temp = vc_image_new(src->width, src->height, 1, 255);
    IVC* temp_2 = vc_image_new(src->width, src->height, 1, 255);

	vc_rgb_to_gray(src, temp);  // Converter a imagem rgb para uma imagem em tons de cinzento

    vc_gray_to_binary(temp, temp_2, 80);                       // Converter a imagem em tons de cionzento numa imagem binaria
    vc_binary_open(temp_2, temp, 3, 3);                        // Realizar uma operação de abertura para salientar os carateres da matricula
    temp_2 = vc_point_amount_image(temp, plate.x+(plate.height/10), plate.y+(plate.height/10), plate.x+plate.width-(plate.height/10), plate.y+plate.height-(plate.height/10));  // Gerar a imagem do cruzamento de pixeis brancos apenas para uma area ligeiramente menor do que a da matricula para que sejam detetados os carateres
	blobs = vc_binary_blob_labelling(temp_2, temp, &nlabels);  // Deteção e etiquetagem dos elementos presentes na imagem do cruzamento de pixeis
    vc_binary_blob_info(temp, blobs, nlabels);				   // Preenchimeto dos parametros das etiquetas

    OVC temp_blob; // blob temporario necessario para a organização dos blobs

	// Organizar os blobs pela sua area
    for (int i = 0; i < (nlabels - 1); ++i)
    {
        for (int j = i + 1; j < nlabels; ++j)
        {
            if (blobs[i].area < blobs[j].area)
            {
                temp_blob = blobs[j];
                blobs[j] = blobs[i];
                blobs[i] = temp_blob;
            }
        }
    }

	// Verificação do numero de blobs encontrados
	if ( nlabels < 6 ) *nletters = nlabels;
	else *nletters = 6;

	// Os maiores blobs encontrados deverão ser os carateres da matricula
    for (int i = 0; i < *nletters ; i++){
        letters[i] = blobs[i];
    }

	free(blobs);
	vc_image_free(temp);
	vc_image_free(temp_2);

	return letters;
}

// Desenha a caixa delimitadora no blob fornecido
//  src     -> imagem
//  blob    -> blob que contem a area a delimitar
//  r, g, b -> componentes RGB a atribuir a linha delimitadora
int vc_draw_boundingbox(IVC *src, OVC blob, int r, int g, int b){

	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int pos;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;

	// Desenhar linhas horizontais
	for (int y = blob.y; y < blob.height+blob.y; y++){
		pos = y * bytesperline + blob.x * channels;
		src->data[pos] = r;
		src->data[pos+1] = g;
		src->data[pos+2] = b;

		pos += blob.width * channels;
		src->data[pos] = r;
		src->data[pos+1] = g;
		src->data[pos+2] = b;
	}

	// Desenhar linhas verticais
	for (int x = blob.x; x < blob.width+blob.x; x++){
		pos = blob.y * bytesperline + x * channels;
		src->data[pos] = r;
		src->data[pos+1] = g;
		src->data[pos+2] = b;
		
		pos += blob.height * bytesperline;
		src->data[pos] = r;
		src->data[pos+1] = g;
		src->data[pos+2] = b;
	}

	return 1;
}

#pragma endregion