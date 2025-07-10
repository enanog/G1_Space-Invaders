/* ---------------------------------------------------
 * main.c
 * ---------------------------------------------------
 * GRUPO 1:
 * 	CASTRO, Tomás
 *	FRIGERIO, Dylan
 * 	VALENZUELA, Agustín
 * 	YAGGI, Lucca
 *
 * Profesores:
 * 	MAGLIIOLA, Nicolas
 * 	JACOBY, Daniel
 * 	VACATELLO, Pablo
 *
 * fecha: 15/07/2025
 * ---------------------------------------------------*/

#include "pc_ui.h"
#include "playSound.h"
#include <stdio.h>

int main(void)
{
	if (!allegro_init()) {
		fprintf(stderr, "ERROR: Failed to initialize Allegro.\n");
		return 1;
	}

	if (!playSound_init()) 
	{
		return -1;
	}
	
	gameLoop();

	return 0;
}
