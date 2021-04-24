//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT…CNICO DO C¡VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM¡TICOS
//                    VIS√O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "vc.h"
#pragma warning(disable:4996)



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN«’ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memÛria para uma imagem
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


// Libertar memÛria de uma imagem
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
//    FUN«’ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
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
            
            // Aloca memÛria para imagem
            image = vc_image_new(width, height, channels, levels);
            if(image == NULL) return NULL;
            
            sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
            tmp = (unsigned char *) malloc(sizeofbinarydata);
            if(tmp == NULL) return 0;
            
#ifdef VC_DEBUG
            //printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
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
            
            // Aloca memÛria para imagem
            image = vc_image_new(width, height, channels, levels);
            if(image == NULL) return NULL;
            
#ifdef VC_DEBUG
            //printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
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


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    //gerar negativo da imagem Gray cinzento
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int vc_gray_negative(IVC *srcdst)
{
    unsigned char *data=(unsigned char *) srcdst->data;
    int width = srcdst->width;
    int height=srcdst->height;
    int bytesperline=srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int x,y;
    long int pos;
    
    //verificaçao de erros
    if ((srcdst->width<=0)||(srcdst->height<=0)||(srcdst->data==NULL))return 0;
    if(channels!=1)return 0;
    
    //inverte a imagem Gray
    for(y=0;y<height;y++)
    {
        for(x=0;x<width;x++)
        {
            pos= y * bytesperline + x * channels;
            data[pos]=255-data[pos];
        }
    }
    return 1;
    
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    // Negativo de Imagem RGB cores para negativo
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int vc_rgb_negative(IVC *srcdst)
{
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int x, y;
    long int pos;
    
    // Verificação de erros
    if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
    {
        return 0;
    }
    if (channels != 3)
    {
        return 0;
    }
    
    // Inverte a imagem Gray
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
    
    return 1;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    /converter de RGB para gray
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int vc_rgb_to_gray(IVC *src, IVC *dst)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    int bytesperline_src=src->width*src->channels;
    int channels_src=src->channels;
    unsigned char *datadst = (unsigned char *) dst->data;
    int bytesperline_dst=dst->width *dst->channels;
    int channels_dst = dst->channels;
    int width=src->width;
    int height=src->height;
    int x, y;
    long int pos_src,pos_dst;
    float rf, gf, bf;
    
    //verificacao de erros
    if((src->width<=0)||(src->height<=0)||(src->data==NULL))return 0;
    if ((src->width!=dst->width)||(src->height!=dst->height)) return 0;
    if ((src->channels!=3)||(dst->channels!=1)) return 0;
    
    for(y=0;y<height;y++)
    {
        for(x=0;x<width;x++)
        {
            pos_src=y* bytesperline_src + x*channels_src;
            pos_dst=y*bytesperline_dst + x*channels_dst;
            
            rf=(float) datasrc[pos_src];
            gf=(float)datasrc[pos_src+1];
            bf=(float)datasrc[pos_src+2];
            
            datadst[pos_dst]=(unsigned char) ((rf*0.299)+(gf*0.587)+(bf*0.114));
            
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

// Converter de RGB para HSV para Amarelo
int vc_rgb_to_hsv_to_yellow(IVC *srcdst)
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
        
        if ((hue >= 35.0f) && (hue <= 65.0f) && (saturation >= 95.0f) && (val >= 95.9f))
        {
            data[i] = (255);
            data[i + 1] = (255);
            data[i + 2] = (255);
        }
        else
        {
            data[i] = (0);
            data[i + 1] = (0);
            data[i + 2] = (0);
        }
    }
    return 1;
    
}

// Converter de RGB para HSV (trabalho) SABER A COR TAMBEM
int vc_rgb_to_negativo(IVC *srcdst, int *cor)
{
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int i, size;
    float r, g, b, hue, saturation, val, maxrgb, minrgb;
    int contavermelho=0, contaazul=0;
    
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
        //vermelho
        if (((hue <= 15.0f) || (hue >= 350.0f)) && (saturation >= 98.0f) && (val >= 98.0f))
        {
            data[i] = (255);
            data[i + 1] = (255);
            data[i + 2] = (255);
            contavermelho++;
           
        }
        //azul
        else if (((hue <= 230.0f) || (hue >= 220.0f)) && (saturation >= 75.0f) && (val >= 45.0f))
        {
            data[i] = (255);
            data[i + 1] = (255);
            data[i + 2] = (255);
            contaazul++;


        }
        else
        {
            data[i] = (0);
            data[i + 1] = (0);
            data[i + 2] = (0);
        }
        if(contaazul<contavermelho)
        {
            *cor=1; //vermelho valor 1
        }
        else if ( contavermelho<contaazul)
        {
            *cor=2; //azul valor 2
        }
            
    }
    return 1;
    
}

//Converter de Gray para RGB (Scale)
int vc_scale_gray_to_rgb(IVC *src, IVC *dst)
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
    
    //VerificaÁ„o de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
    if ((src->width != dst->width) || (src->height != dst->height))return 0;
    if ((src->channels != 1) || (dst->channels != 3)) return 0;
    
    for (y = 0; y<height; y++)
    {
        for (x = 0; x<width; x++)
        {
            pos_src = y*bytesperline_src + x*channels_src;
            pos_dst = y*bytesperline_dst + x*channels_dst;
            
            if (datasrc[pos_src]<64)
            {
                datadst[pos_dst] = 0;
                datadst[pos_dst + 1] = datasrc[pos_src] * 4;
                datadst[pos_dst + 2] = 255;
            }
            else if (datasrc[pos_src]<128)
            {
                datadst[pos_dst] = 0;
                datadst[pos_dst + 1] = 255;
                datadst[pos_dst + 2] = 255 - ((datasrc[pos_src] - 64) * 4); //??
            }
            else if ((datasrc[pos_src]<192))
            {
                datadst[pos_dst] = (datasrc[pos_src] - 128) * 4;
                datadst[pos_dst + 1] = 255;
                datadst[pos_dst + 2] = 0;
            }
            else
            {
                datadst[pos_dst] = 255;
                datadst[pos_dst + 1] = 255 - ((datasrc[pos_src] - 192) * 4); //???
                datadst[pos_dst + 2] = 0;
            }
        }
    }
    
    return 1;
}

//Converter de Gray para Binario
int vc_gray_to_binary(IVC *srcdst, int threshold)
{
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int x, y;
    long int pos;
    
    // VerificaÁ„o de erros
    if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
    {
        return 0;
    }
    if (channels != 1)
    {
        return 0;
    }
    
    //SegmentaÁ„o por thresholding
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;
            if (data[pos] > threshold)
            {
                data[pos] = 255;
            }
            else
            {
                data[pos] = 0;
            }
        }
    }
    return 1;
}
//Converter de Gray para Binario
int vc_gray_to_binary2(IVC *src, IVC* dst, int threshold)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;
    
    int width = src->width;
    int height = src->height;
    int bytesperline = src->width * src->channels;
    int channels = src->channels;
    int x, y;
    long int pos;
    
    // VerificaÁ„o de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
    {
        return 0;
    }
    if (channels != 1)
    {
        return 0;
    }
    
    //SegmentaÁ„o por thresholding
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos = y * bytesperline + x * channels;
            if (datasrc[pos] > threshold)
            {
                datadst[pos] = 255;
            }
            else
            {
                datadst[pos] = 0;
            }
        }
    }
    return 1;
}

//Converter de Gray para Binario, atravÈs da mÈdia global
int vc_gray_to_binary_global_mean(IVC *srcdst)
{
    unsigned char *data = (unsigned char *)srcdst->data;
    int width = srcdst->width;
    int height = srcdst->height;
    int bytesperline = srcdst->width * srcdst->channels;
    int channels = srcdst->channels;
    int i, size;
    float temp;
    float media;
    
    // VerificaÁ„o de erros
    if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL))
    {
        return 0;
    }
    if (channels != 1)
    {
        return 0;
    }
    
    size = width * height * channels;
    
    for (i = 0; i < size; i = i + channels)
    {
        temp = data[i] + temp;
    }
    
    media = temp / size;
    
    if (data[i] > media)
    {
        data[i] = 255;
    }
    else
    {
        data[i] = 0;
    }
    
    return 1;
}

//Converter de Gray para Binario, atravÈs do ponto mÈdio
int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y;
    int xx, yy;
    int xxyymax = (kernel - 1) / 2;
    int xxyymin = -xxyymax;
    int max, min;
    long int pos, posk;
    unsigned char threshold;
    
    // VerificaÁ„o de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if (channels != 1) return 0;
    
    for (y = 0; y<height; y++)
    {
        for (x = 0; x<width; x++)
        {
            pos = y * bytesperline + x * channels;
            
            max = datasrc[pos];
            min = datasrc[pos];
            
            // NxM Vizinhos
            for (yy = xxyymin; yy <= xxyymax; yy++)
            {
                for (xx = xxyymin; xx <= xxyymax; xx++)
                {
                    if ((y + yy >= 0) && (y + yy < height) && (x + xx >= 0) && (x + xx < width))
                    {
                        posk = (y + yy) * bytesperline + (x + xx) * channels;
                        
                        if (datasrc[posk] > max) max = datasrc[posk];
                        if (datasrc[posk] < min) min = datasrc[posk];
                    }
                }
            }
            
            threshold = (unsigned char)((float)(max + min) / (float)2);
            
            if (datasrc[pos] > threshold) datadst[pos] = 255;
            else datadst[pos] = 0;
        }
    }
    
    return 1;
}
//binario para dilatada
int vc_binary_dilate(IVC *src, IVC *dst, int kernel)
{
   
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline_src = src->width* src->channels;
    int channels_src = src->channels;
    
    unsigned char *datadst = (unsigned char *) dst->data;
    int width = src->width;
    int height = src->height;
    
    int x,y,xx,yy;
    long int pos,posk;
    
    if((src->width <=0) || (src->height <=0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height) ) return 0;
    if(( src->channels !=1 ) || ( dst->channels!=1 )) return 0;
    
    for(y=0;y<height;y++){
        for(x=0; x<width;x++){
            pos = y*bytesperline_src + x*channels_src;
            datadst[pos]=0;
            if ((((y - kernel/2)>=0) && ((x-kernel/2)>=0) && ((y+kernel/2)<height-1) && ((x+kernel/2)<width-1))) {
                for(yy=y-kernel /2; yy<=y+kernel/2;yy++){
                    for(xx=x-kernel/2;xx<=x+kernel/2;xx++){
                        posk= yy * bytesperline_src + xx * channels_src;
                        if(datasrc[posk]==255) datadst[pos] = 255;
                    }
                }
            }
        }
    }
    return 1;
}
//binario para erosao
int vc_binary_erode(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    
    unsigned char *datadst = (unsigned char *) dst->data;
    int width = src->width;
    int height = src->height;
    
    int x,y,xx,yy;
    long int pos,posk;
    
	   //verificaÁ„o de erros
    if((src->width <=0) || (src->height <=0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height) ) return 0;
    if(( src->channels !=1 ) || ( dst->channels!=1 )) return 0;
    
    for(y=0; y<height;y++)
    {
        for(x=0;x<width;x++)
        {
            
            pos= y * bytesperline_src + x * channels_src;
            datadst[pos] = 255;
            if(((y-kernel /2) >=0) && ((x-kernel/2)>=0) && ((y+kernel/2)<height-1) && ((x+kernel/2)<width-1))
            {
                for(yy=y-kernel /2; yy<=y+kernel/2;yy++) //se o kernel for 25/2=12.5 mas fica 12 e depois - a variavel anda 12 casas para tras e 12 casas para a frente
                {
                    for(xx=x-kernel/2;xx<=x+kernel/2;xx++)
                    {
                        posk= yy * bytesperline_src + xx * channels_src;
                        if(datasrc[posk]==0) datadst[pos] = 0;
                    }
                }
            }
        }
    }
    return 1;
}

//funcao para abrir a img e fazer erosao e dilatacao
int vc_binary_open(IVC *src, IVC *dst, int sizeerode, int sizedilate)
{
    
    int ret=1;
    
    IVC *temp=vc_image_new(src->width, src->height, src->channels, src->levels);
    
    ret &= vc_binary_erode(src, temp, sizeerode);
    ret &= vc_binary_dilate(temp, dst, sizedilate);
    
    vc_image_free(temp);
    return ret;
}

//funcao para abrir a img e fazer erosao e dilatacao
int vc_binary_close(IVC *src, IVC *dst,int sizedilate, int sizeerode)
{
    int ret=1;
    IVC *temp=vc_image_new(src->width, src->height, src->channels, src->levels);
    
    ret &= vc_binary_dilate(src,temp, sizedilate);
    ret &= vc_binary_erode(temp,dst,sizeerode);
    
    vc_image_free(temp);
    return ret;
}
//gray para dilatada
int vc_gray_dilate(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline_src = src->width* src->channels;
    int channels_src = src->channels;
    
    unsigned char *datadst = (unsigned char *) dst->data;
    int width = src->width;
    int height = src->height;
    
    int x,y,xx,yy;
    int maximo;
    long int pos,posk;
    
    if((src->width <=0) || (src->height <=0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height) ) return 0;
    if(( src->channels !=1 ) || ( dst->channels!=1 )) return 0;
    
    for(y=0;y<height;y++)
    {
        for(x=0; x<width;x++)
        {
            pos = y*bytesperline_src + x*channels_src;
            datadst[pos]=0;
            maximo=0;
            if ((((y - kernel/2)>=0) && ((x-kernel/2)>=0) && ((y+kernel/2)<height-1) && ((x+kernel/2)<width-1)))
            {
                for(yy=y-kernel /2; yy<=y+kernel/2;yy++)
                {
                    for(xx=x-kernel/2;xx<=x+kernel/2;xx++)
                    {
                        posk= yy * bytesperline_src + xx * channels_src;
                        
                        if(maximo<datasrc[posk])
                        {
                            maximo=datasrc[posk];
                        }
                    }
                }
                datadst[pos]=maximo;
            }
        }
    }
    return 1;
}
//gray para erode
int vc_gray_erode(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline_src = src->width* src->channels;
    int channels_src = src->channels;
    
    unsigned char *datadst = (unsigned char *) dst->data;
    int width = src->width;
    int height = src->height;
    
    int x,y,xx,yy;
    int minimo, contaKernel;
    long int pos,posk;
    
    if((src->width <=0) || (src->height <=0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height) ) return 0;
    if(( src->channels !=1 ) || ( dst->channels!=1 )) return 0;
    
    for(y=0;y<height;y++)
    {
        for(x=0; x<width;x++)
        {
            pos = y*bytesperline_src + x*channels_src;
            datadst[pos]=0;
            minimo=255;
            contaKernel=kernel/2;
            if ((((y - contaKernel)>=0) && ((x-contaKernel)>=0) && ((y+contaKernel)<height-1) && ((x+contaKernel)<width-1)))
            {
                for(yy=y-contaKernel; yy<=y+contaKernel;yy++)
                {
                    for(xx=x-contaKernel;xx<=x+contaKernel;xx++)
                    {
                        posk= yy * bytesperline_src + xx * channels_src;
                        
                        if(minimo>datasrc[posk])
                        {
                            minimo=datasrc[posk];
                        }
                    }
                }
                datadst[pos]=minimo;
            }
        }
    }
    return 1;
}
//funcao para abrir a img gray e fazer erosao e dilatacao
int vc_gray_open(IVC *src, IVC *dst, int sizeerode, int sizedilate)
{
    
    int ret=1;
    
    IVC *tmp= vc_image_new(src->width, src->height, src->channels, src->levels);
    
    ret &= vc_gray_erode(src, tmp, sizeerode);
    ret &= vc_gray_dilate(tmp, dst, sizedilate);
    
    vc_image_free(tmp);
    return ret;
    
}
//funcao para abrir a img gray e fazer erosao e dilatacao
int vc_gray_close(IVC *src, IVC *dst,int sizedilate, int sizeerode)
{
    int ret=1;
    IVC *temp=vc_image_new(src->width, src->height, src->channels, src->levels);
    
    ret &= vc_gray_dilate(src,temp, sizedilate);
    ret &= vc_gray_erode(temp,dst,sizeerode);
    
    vc_image_free(temp);
    return ret;
}


//LABELING BLOBS

// Etiquetagem de blobs
// src		: Imagem bin·ria de entrada
// dst		: Imagem grayscale (ir· conter as etiquetas)
// nlabels	: EndereÁo de memÛria de uma vari·vel, onde ser· armazenado o n˙mero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. … necess·rio libertar posteriormente esta memÛria.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y, a, b;
    long int i, size;
    long int posX, posA, posB, posC, posD;
    int labeltable[256] = { 0 };
    int labelarea[256] = { 0 };
    int label = 1; // Etiqueta inicial.
    int num, tmplabel;
    OVC *blobs; // Apontador para array de blobs (objectos) que ser· retornado desta funÁ„o.
    
				// VerificaÁ„o de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
    if (channels != 1) return NULL;
    
    // Copia dados da imagem bin·ria para imagem grayscale
    memcpy(datadst, datasrc, bytesperline * height);
    
    // Todos os pixÈis de plano de fundo devem obrigatÛriamente ter valor 0
    // Todos os pixÈis de primeiro plano devem obrigatÛriamente ter valor 255
    // Ser„o atribuÌdas etiquetas no intervalo [1,254]
    // Este algoritmo est· assim limitado a 255 labels
    for (i = 0, size = bytesperline * height; i<size; i++)
    {
        if (datadst[i] != 0) datadst[i] = 255;
    }
    
    // Limpa os rebordos da imagem bin·ria
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
                    
                    // Se A est· marcado
                    if (datadst[posA] != 0) num = labeltable[datadst[posA]];
                    // Se B est· marcado, e È menor que a etiqueta "num"
                    if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
                    // Se C est· marcado, e È menor que a etiqueta "num"
                    if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
                    // Se D est· marcado, e È menor que a etiqueta "num"
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
    
    // Contagem do n˙mero de blobs
    // Passo 1: Eliminar, da tabela, etiquetas repetidas
    for (a = 1; a<label - 1; a++)
    {
        for (b = a + 1; b<label; b++)
        {
            if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
        }
    }
    // Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n„o hajam valores vazios (zero) entre etiquetas
    *nlabels = 0;
    for (a = 1; a<label; a++)
    {
        if (labeltable[a] != 0)
        {
            labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
            (*nlabels)++; // Conta etiquetas
        }
    }
    
    // Se n„o h· blobs
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
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
    unsigned char *data = (unsigned char *)src->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y, i;
    long int pos;
    int xmin, ymin, xmax, ymax;
    long int sumx, sumy;
    
    // VerificaÁ„o de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if (channels != 1) return 0;
    
    // Conta ·rea de cada blob
    for (i = 0; i<nblobs; i++)
    {
        xmin = width - 1;
        ymin = height - 1;
        xmax = 0;
        ymax = 0;
        
        sumx = 0;
        sumy = 0;
        
        blobs[i].area = 0;
        
        for (y = 1; y<height - 1; y++)
        {
            for (x = 1; x<width - 1; x++)
            {
                pos = y * bytesperline + x * channels;
                
                if (data[pos] == blobs[i].label)
                {
                    // ¡rea
                    blobs[i].area++;
                    
                    // Centro de Gravidade
                    sumx += x;
                    sumy += y;
                    
                    // Bounding Box
                    if (xmin > x) xmin = x;
                    if (ymin > y) ymin = y;
                    if (xmax < x) xmax = x;
                    if (ymax < y) ymax = y;
                    
                    // PerÌmetro
                    // Se pelo menos um dos quatro vizinhos n„o pertence ao mesmo label, ent„o È um pixel de contorno
                    if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
                    {
                        blobs[i].perimeter++;
                    }
                }
            }
        }
        
        // Bounding Box
        blobs[i].x = xmin;
        blobs[i].y = ymin;
        blobs[i].width = (xmax - xmin) + 1;
        blobs[i].height = (ymax - ymin) + 1;
        
        // Centro de Gravidade
        //blobs[i].xc = (xmax - xmin) / 2;
        //blobs[i].yc = (ymax - ymin) / 2;
        blobs[i].xc = sumx / MAX(blobs[i].area, 1);
        blobs[i].yc = sumy / MAX(blobs[i].area, 1);
    }
    
    return 1;
}

OVC* vc_binary_blob_labelling1(IVC *src, IVC *dst, int *nlabels)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y, a, b;
    long int i, size;
    long int posX, posA, posB, posC, posD;
    int labeltable[256] = { 0 };
    int labelarea[256] = { 0 };
    int label = 1; // Etiqueta inicial.
    int num, tmplabel;
    OVC *blobs; // Apontador para array de blobs (objectos) que ser· retornado desta funÁ„o.
    
				// VerificaÁ„o de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
    if (channels != 1) return NULL;
    
    // Copia dados da imagem bin·ria para imagem grayscale
    memcpy(datadst, datasrc, bytesperline * height);
    
    // Todos os pixÈis de plano de fundo devem obrigatÛriamente ter valor 0
    // Todos os pixÈis de primeiro plano devem obrigatÛriamente ter valor 255
    // Ser„o atribuÌdas etiquetas no intervalo [1,254]
    // Este algoritmo est· assim limitado a 255 labels
    for (i = 0, size = bytesperline * height; i<size; i++)
    {
        if (datadst[i] != 0) datadst[i] = 255;
    }
    
    // Limpa os rebordos da imagem bin·ria
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
                    
                    // Se A est· marcado
                    if (datadst[posA] != 0) num = labeltable[datadst[posA]];
                    // Se B est· marcado, e È menor que a etiqueta "num"
                    if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
                    // Se C est· marcado, e È menor que a etiqueta "num"
                    if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
                    // Se D est· marcado, e È menor que a etiqueta "num"
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
    
    // Contagem do n˙mero de blobs
    // Passo 1: Eliminar, da tabela, etiquetas repetidas
    for (a = 1; a<label - 1; a++)
    {
        for (b = a + 1; b<label; b++)
        {
            if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
        }
    }
    // Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n„o hajam valores vazios (zero) entre etiquetas
    *nlabels = 0;
    for (a = 1; a<label; a++)
    {
        if (labeltable[a] != 0)
        {
            labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
            (*nlabels)++; // Conta etiquetas
        }
    }
    
    // Se n„o h· blobs
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

//FUNCAO PARA INVERTER UMA IMAGEM BINARIA
int vc_binary_negativo(IVC *src, IVC *dst)
{
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    
    unsigned char *datadst = (unsigned char *) dst->data;
    int width = src->width;
    int height = src->height;
    
    int x,y,xx,yy;
    long int pos,posk;
    
	   //verificaÁ„o de erros
    if((src->width <=0) || (src->height <=0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height) ) return 0;
    if(( src->channels !=1 ) || ( dst->channels!=1 )) return 0;
    
    for(y=0; y<height;y++)
    {
        for(x=0;x<width;x++)
        {
            
            pos= y * bytesperline_src + x * channels_src;
            datadst[pos] = 255;
            if(datasrc[pos]==0) datadst[pos] = 255;
            if(datasrc[pos]==255) datadst[pos] = 0;
        }
    }
    return 1;
}

//FUNCAO PARA CORTAR UMA IMAGEM PELO LABEL
int vc_binary_corta(IVC *src, IVC *dst,OVC *blobs, int nblobs)
{
    
 
    unsigned char *datasrc = (unsigned char *) src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    
    unsigned char *datadst = (unsigned char *) dst->data;
    int width = src->width;
    int height = src->height;
    int x,y,xx,yy;
    long int pos,posk;
    int cont;
    int temparea;
    int posarea=0;
    temparea = blobs[0].area;
	   //verificaÁ„o de erros
    if((src->width <=0) || (src->height <=0) || (src->data == NULL)) return 0;
    if((src->width != dst->width) || (src->height != dst->height) ) return 0;
    if(( src->channels !=1 ) || ( dst->channels!=1 )) return 0;
   
    for( cont=0;cont<nblobs;cont++)
    {
        
        //saber a label com maior area
        if (temparea <= blobs[cont].area)
        {
            
            temparea=blobs[cont].area;
            posarea=cont;
            
        }
        
    }

    for(y=0; y<height;y++)
    {
        for(x=0;x<width;x++)
        {
            if (blobs[posarea].y < y && (blobs[posarea].y + blobs[posarea].height)>y && blobs[posarea].x < x && (blobs[posarea].x + blobs[posarea].width)>x )
            {
                pos= y * bytesperline_src + x * channels_src;
                datadst[pos]=datasrc[pos];
                
            }
            else
            {
                
                pos= y * bytesperline_src + x * channels_src;
                datadst[pos] = 0;
            }
            
            
        }
    }
    return 1;
}

//FAZER CAIXA A VOLTA DO SINAL
int vc_caixa(IVC *src, IVC *dst, OVC *blobs)
{
    unsigned char *data_src = (unsigned char *)src->data;
    int bytesperline_src = src->width * src->channels;
    int channels_src = src->channels;
    
    unsigned char *data_dst = (unsigned char *)dst->data;
    int bytesperline_dst = src->width * dst->channels;
    int channels_dst = dst->channels;
    
    int width = src->width;
    int height = src->height;
    int x, y;
    long int pos_src;
    int xmin, xmax, ymin, ymax;
    xmin= blobs[0].x;
    ymin= blobs[0].y;
    xmax=blobs[0].width + blobs[0].x;
    ymax=blobs[0].height+ blobs[0].y;
    
    // verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if (channels_src != 1 || channels_dst != 1) return 0;
    
    for (x = 0; x < width; x++)
       {
        for (y = 0; y < height; y++)
        {
            pos_src = y*bytesperline_dst + x*channels_dst;
            
            data_dst[pos_src]   = data_src[pos_src];
            
            // linha direita
            if (y >= ymin && y <= ymax && (x == xmax || x == xmax + 1))
            {
                data_dst[pos_src] = 255;
            }
            
            // linha esquerda
            if (y >= ymin && y <= ymax && (x == xmin || x == xmin + 1))
            {
                data_dst[pos_src] = 255;
            }

            // linha superior
            if (x >= xmin && x <= xmax && (y == ymin || y == ymin + 1))
            {
                data_dst[pos_src] = 255;
         
            }
            
            // linha inferior
            if (x >= xmin && x <= xmax && (y == ymax || y == ymax + 1))
            {
                data_dst[pos_src] = 255;
                
            }
            
        }
    }
    
    return 1;
}

