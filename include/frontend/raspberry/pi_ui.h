/* ---------------------------------------------------
 * pi_ui.h
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

#ifndef PI_UI_H
#define PI_UI_H

#include "config.h"
#include "game.h"
#include <stdbool.h>

/**
 * @brief Initialize the PI UI system
 */
void pi_ui_init(void);

/**
 * @brief Main menu loop
 */
void pi_ui_menu(void);
#endif // PI_UI_H