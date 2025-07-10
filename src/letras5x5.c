#include <stdio.h>
#include "letras5x5.h"

void mostrar_letra(int letra) {
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            printf("%c", letras_5x5[letra][i][j] ? '#' : ' ');
        }
        printf("\n");
    }
}
