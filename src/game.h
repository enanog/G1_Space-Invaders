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

enum{PAUSED, RUNNING, QUIT};

typedef struct {
    player_t player;
    enemy_t enemies[ENEMIES_ROW][ENEMIES_COLUMNS];
    barrier_t barriers[BARRIER_QUANTITY];
    mothership_t mothership;
    int score;
    int level;
    int lives;
    int state;
} gameState_t;

void game_init(int enemiesRow, int enemiesColumn, int barrierQuantity, int barrierRow, int barrierColumn);
void game_update(void);
void game_over(void);
void game_resume(void);
void game_pause(void);
void game_reset(void);

int getScore(void);
int getLevel(void);
coord_t getPlayerPosition(void);
coord_t getEnemyPosition(int row, int column);
coord_t getBarrierPosition(int barrier, int row, int column);

#endif // GAME_H
