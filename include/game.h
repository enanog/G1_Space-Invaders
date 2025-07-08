/* ---------------------------------------------------
 * score.h
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

#ifndef GAME_H
#define GAME_H
#include <stdbool.h>
#include "entity.h"
#include "config.h"

typedef struct {
    player_t player;
    enemy_t enemies[ENEMIES_ROW][ENEMIES_COLUMNS];
    barrierBlock_t (barriers[BARRIER_QUANTITY])[BARRIER_ROWS][BARRIER_COLUMNS];
    mothership_t mothership;
    int score;
    int level;
    int lives;
    int game_state;
    float enemy_dx;
} gameState_t;

void game_init(gameState_t *game);
void game_update(gameState_t *game);
void game_over(gameState_t *game);



#endif // GAME_H
