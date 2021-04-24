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
#include <stdlib.h>
#include <string.h>
#include "vc.h"

int main(int argc, char *argv[]){

    IVC* image;    // Imagem a trabalhar
    OVC plate;     // Blob da matricula
    OVC* letters;  // Blobs das letras da matricula
    int nletters;  // Numero de letras lidas

    // Verificação dos parametros de entrada
    if(argc < 2 ){
        printf("ERROR -> Unspecified file!\n");
        getchar();
        return 0;
    }
    
    // Inicialização da imagem de entrada
    image = vc_read_image(argv[1]);

    // Verificação de erro ao iniciar a imagem
    if(image == NULL){
        printf("ERROR -> VC_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }
    
    // Deteção do blob da matricula
    plate = vc_plate_blob(image);
    
    // Deteção dos blobs das letras da matricula
    letters = vc_letters_blobs(image, plate, &nletters);
    
    // Desenhar caixa delimitadora da matricula
    vc_draw_boundingbox(image, plate, 255, 0, 0);

    // Desenhar caixa delimitador das letras da matricula
    for (int i = 0; i < nletters ; i++){
        vc_draw_boundingbox(image, letters[i], 100, 100, 100);
    }

    // Escrita da imagem final
    vc_write_image("vc-0001.ppm", image);

    // Libertar memoria da imagem
    vc_image_free(image);

    return 0;
}