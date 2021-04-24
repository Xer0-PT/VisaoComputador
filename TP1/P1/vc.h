//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT…CNICO DO C¡VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM¡TICOS
//                    VIS√O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG
#define MAX(a,b) (a>b ? a : b)
#define M_PI 3.14159265358979323846;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
    unsigned char *data;
    int width, height;
    int channels;			// Bin·rio/Cinzentos=1; RGB=3
    int levels;				// Bin·rio=1; Cinzentos [1,255]; RGB [1,255]
    int bytesperline;		// width * channels
} IVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT”TIPOS DE FUN«’ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN«’ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN«’ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);
int vc_gray_negative(IVC *srcdst);
int vc_rgb_negative(IVC *srcdst);
int vc_rgb_to_gray(IVC *src, IVC *dst);


// Converter de RGB para HSV
int vc_rgb_to_hsv(IVC *srcdst);

// Converter de RGB para HSV para Amarelo
int vc_rgb_to_hsv_to_yellow(IVC *srcdst);

// Converter de RGB para HSV para Vermelho
int vc_rgb_to_hsv_to_red(IVC *srcdst);

//Converter de Gray para RGB
int vc_scale_gray_to_rgb(IVC *src, IVC *dst);

//Converter de Gray para Binario
int vc_gray_to_binary(IVC *srcdst, int threshold);
int vc_gray_to_binary2(IVC *src, IVC* dst, int threshold);


//Converter de Gray para Binario, atravÈs da mÈdia global
int vc_gray_to_binary_global_mean(IVC *srcdst);

//Converter de Gray para Binario, atravÈs do ponto mÈdio
int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel);

int vc_binary_dilate(IVC *src, IVC *dst, int kernel);
int vc_binary_erode(IVC *src, IVC *dst, int kernel);
int vc_binary_close(IVC *src, IVC *dst,int sizedilate, int sizeerode);
int vc_binary_open(IVC *src, IVC *dst, int sizeerode, int sizedilate);


int vc_gray_dilate(IVC *src, IVC *dst, int kernel);
int vc_gray_erode(IVC *src, IVC *dst, int kernel);
int vc_gray_open(IVC *src, IVC *dst, int sizeerode, int sizedilate);
int vc_gray_close(IVC *src, IVC *dst, int sizeerode, int sizedilate);






//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
    int area;					// ¡rea
    int xc, yc;					// Centro-de-massa
    int perimeter;				// PerÌmetro
    int label;					// Etiqueta
} OVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT”TIPOS DE FUN«’ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);

//trabalho
int vc_rgb_to_negativo(IVC *srcdst, int *cor);
int vc_binary_negativo(IVC *src, IVC *dst);
int vc_binary_corta(IVC *src, IVC *dst,OVC *blobs, int nblobs);
int vc_caixa(IVC *src, IVC *dst,OVC *blobs);
