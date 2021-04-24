#pragma warning (disable: 4996)


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


// Definições da obtenção do maximo e minimo entre dois valores
#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                  ESTRUTURAS DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Bin�rio/Cinzentos=1; RGB=3
	int levels;				// Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// Área
	int label;					// Etiqueta
} OVC;

typedef struct {
	int radius;       // Raio do filtro
	double **matrix;  // Matriz do filtro
} FILTER;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

// Funçoes para detetar a localização da matricula e das letras
OVC vc_plate_blob(IVC* src);
OVC* vc_letters_blobs(IVC* src, OVC plate, int* nletters);

// Função para desenhar a caixa delimitadora
int vc_draw_boundingbox(IVC *src, OVC blob, int r, int g, int b);