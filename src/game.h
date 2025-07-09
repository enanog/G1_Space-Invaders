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
    int state;
    int enemiesRow;
    int enemiesColumn;
    int enemiesSpeed;
    int enemiesDirection;
    int barrirersQuantity;
    int barriersRow;
    int barriersColumn;
    int enemyShotInterval;
    long long lastTimeEnemyShoot;
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
void game_init(int enemiesRow, int enemiesColumn);
void game_create_enemy_map(int enemiesRow, int enemiesColumn);
void game_create_barriers();
void update_player_bullet(input_t input, float dt);
void update_enemy_bullet(float dt);

int game_update(input_t player);
int game_over(void);
void game_resume(void);
void game_pause(void);
void game_reset(void);
//void game_level_up(void);


int getScore(void);
int getLevel(void);
int getEnemyTier(int row);

hitbox_t getPlayerPosition(void);
hitbox_t getEnemyPosition(int row, int column);

hitbox_t getBarrierPosition(int barrier, int row, int column);
bool getBarrierIsAlive(int barrier, int row, int column);

bullet_t getPlayerBulletinfo(void);

bool getIsEnemyAlive(int row, int column);
void getEnemyBitMap(bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX]);
void getEnemiesBulletsInfo(bullet_t matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX]);

#endif // GAME_H
