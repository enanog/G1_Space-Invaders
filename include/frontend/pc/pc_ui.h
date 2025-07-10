/* ---------------------------------------------------
 * pc_ui.h
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

#ifndef PC_UI_H
#define PC_UI_H

#include <stdbool.h>

typedef enum {
    STATE_SPLASH,
    STATE_MENU,
    STATE_GAME,
    STATE_RESUME,
    STATE_SCOREBOARD,
    STATE_EXIT
} gameState_t;

void gameLoop(void);

void map(void);
bool allegro_init(void);
void allegro_shutdown(void);

#endif
