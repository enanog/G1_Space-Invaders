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
    STATE_NEW_GAME,
    STATE_RESUME_GAME,
    STATE_SCOREBOARD,
    STATE_CREDITS,
    STATE_EXIT,
    STATE_PAUSE,
    STATE_GAME_OVER
} gameState_t;


/* ---------------------------------------------------
 * @brief Initialize Allegro subsystems and resources
 * @return true if initialization succeeded, false otherwise
 * ---------------------------------------------------*/
bool allegro_init(void);

/* ---------------------------------------------------
 * @brief Main game loop that manages state transitions
 * 
 * Handles the core game flow by processing state changes and
 * executing the appropriate functions for each game state.
 * Manages music/sound effects during state transitions.
 * 
 * State Transitions:
 * - STATE_SPLASH:    Shows splash screen -> transitions to menu
 * - STATE_MENU:      Displays main menu -> handles user selection
 * - STATE_NEW_GAME:  Starts new game with default parameters
 * - STATE_RESUME_GAME: Loads saved game state
 * - STATE_GAME_OVER: Handles game over sequence
 * - STATE_SCOREBOARD: Displays high scores
 * - STATE_EXIT:      Terminates game loop
 * - STATE_CREDITS:   Shows credits screen
 * - STATE_PAUSE:     Handles pause menu with screen snapshot
 * 
 * Cleanup:
 * - Releases font resources
 * - Stops any playing music
 * - Shuts down audio system
 * ---------------------------------------------------*/
void gameLoop(void);

/* ---------------------------------------------------
 * @brief Cleanup Allegro resources
 * ---------------------------------------------------*/
void allegro_shutdown(void);

#endif