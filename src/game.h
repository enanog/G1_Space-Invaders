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

typedef struct 
{
    player_t player;
    enemy_t enemies[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];
    barrier_t barriers[BARRIER_QUANTITY_MAX];
    mothership_t mothership;
    int score;
    int level;
    int lives;
    int state;
    int enemiesRow;
    int enemiesColumn;
    int enemiesSpeed;
    int enemiesDirection;
    int barrirersQuantity;
    int barriersRow;
    int barriersColumn;
    long long lastTimeUpdated;
} gameState_t;

typedef struct 
{
    int direction;
    bool shot;
} input_t;

/**
 * @brief Initializes the game state including enemies, player, barriers, and projectile.
 *
 * @param enemiesRow Number of enemy rows
 * @param enemiesColumn Number of enemy columns
 * @param barrierQuantity Number of barriers
 * @param barrierRows Number of rows per barrier
 * @param barrierColumns Number of columns per barrier
 */
void game_init(int enemiesRow, int enemiesColumn, int barrierQuantity, int barrierRow, int barrierColumn);
int game_update(input_t player);
int game_over(void);
void game_resume(void);
void game_pause(void);
void game_reset(void);

int getScore(void);
int getLevel(void);
coord_t getPlayerPosition(void);
coord_t getEnemyPosition(int row, int column);
coord_t getBarrierPosition(int barrier, int row, int column);

#endif // GAME_H
