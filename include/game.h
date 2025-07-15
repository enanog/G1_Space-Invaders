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

/* ======================== PUBLIC FUNCTION DECLARATIONS ======================== */

/* ---------------------------------------------------
 * @brief Initialize game state
 * @param enemiesRow Number of enemy rows
 * @param enemiesColumn Number of enemy columns
 * @param resumeLastGame Whether to load saved game
 * @return true if initialization succeeded, false otherwise
 * ---------------------------------------------------*/
bool game_init(int enemiesRow, int enemiesColumn, bool resumeLastGame);

/* ---------------------------------------------------
 * @brief Update game state based on input and time
 * @param player Player input state
 * @return Current game state
 * ---------------------------------------------------*/
int game_update(input_t player);

/* ======================== PUBLIC GETTER DECLARATIONS ======================== */

void getEnemiesBulletsInfo(bullet_t matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX]);
int getPlayerLives(void);
hitbox_t getMothershipPosition(void);
bool getIsMothershipAlive(void);
hitbox_t getPlayerPosition(void);
hitbox_t getEnemyPosition(int row, int column);
bool getIsEnemyAlive(int row, int column);
bool getBarrierIsAlive(int barrier, int row, int column);
hitbox_t getBarrierPosition(int barrier, int row, int column);
int getEnemyTier(int row);
long long getTimeMillis(void);
bullet_t getPlayerBulletinfo(void);
bool getEnemiesHands(void);
int getLevel(void);
int getScore(void);

#endif // GAME_H