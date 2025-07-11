#ifndef FONT_H
#define FONT_H

#define NUM_LETRAS 26
#define FONT_ROWS 5
#define FONT_COLS 4

void mostrar_letra(int letra);
int get_letter_bitmap(char letter);

extern const int letras_5x4[NUM_LETRAS][FONT_ROWS][FONT_COLS];

#endif //FONT_H
