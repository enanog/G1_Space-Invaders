/* ---------------------------------------------------
 * font.h
 * ---------------------------------------------------
 * GRUPO 1:
 * 	CASTRO, Tomás
 *	FRIGERIO, Dylan
 * 	VALENZUELA, Agustín
 * 	YAGGI, Lucca
 *
 * Profesores:
 * 	MAGLIOLA, Nicolas
 * 	JACOBY, Daniel
 * 	VACATELLO, Pablo
 *
 * fecha: 15/07/2025
 * ---------------------------------------------------*/

#ifndef FONT_H
#define FONT_H

#define NUM_LETRAS 26
#define FONT_ROWS 5
#define FONT_COLS 4
#define NUM_NUMEROS 10

void mostrar_letra(int letra);
int get_letter_bitmap(char letter);
int get_number_bitmap(char digit);

extern const int letras_5x4[NUM_LETRAS][FONT_ROWS][FONT_COLS];
extern const int numeros_5x4[NUM_NUMEROS][FONT_ROWS][FONT_COLS];

#endif //FONT_H
