/* ---------------------------------------------------
 * game.h
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

#ifndef GAME_H
#define GAME_H
#include <stdbool.h>
#include "entity.h"
#include "config.h"

enum
{
	RUNNING,
	GAME_OVER,
	QUIT,
	PAUSED
};

typedef struct
{
	int direction;
	bool shot;
	bool pause;
	bool exit;
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
bool game_init(int enemiesRow, int enemiesColumn, bool resumeLastGame);
long long getTimeMillis(void);

int game_update(input_t player);
void game_resume(void);
void game_pause(void);
void game_reset(void);

int getScore(void);
int getLevel(void);
int getEnemyTier(int row);

bool getEnemiesHands(void);
int getPlayerLives(void);

hitbox_t getPlayerPosition(void);
hitbox_t getEnemyPosition(int row, int column);

hitbox_t getBarrierPosition(int barrier, int row, int column);
bool getBarrierIsAlive(int barrier, int row, int column);

bullet_t getPlayerBulletinfo(void);

bool getIsEnemyAlive(int row, int column);
void getEnemiesBulletsInfo(bullet_t matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX]);


hitbox_t getMothershipPosition(void);
bool getIsMothershipAlive(void);

#endif // GAME_H