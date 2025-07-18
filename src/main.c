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
 * 	MAGLIOLA, Nicolas
 * 	JACOBY, Daniel
 * 	VACATELLO, Pablo
 *
 * fecha: 15/07/2025
 * ---------------------------------------------------*/

#include "playSound.h"

#ifndef RASPBERRY
#include "pc_ui.h"
#else
#include "pi_ui.h"
#endif

#include <stdio.h>


int main(void)
{

	if (!playSound_init()) 
	{
		return -1;
	}

    #ifndef RASPBERRY
	if (!allegro_init()) {
 	fprintf(stderr, "ERROR: Failed to initialize Allegro.\n");
 	return 1;
    }

    gameLoop();
    allegro_shutdown();
    #else
   
	pi_ui_init();
	pi_ui_menu();
	#endif

	return 0;
}
