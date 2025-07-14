/* ---------------------------------------------------
 * game.c
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

#include "game.h"
#include "entity.h"
#include "config.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "playSound.h"
#include <string.h>

#ifndef RASPBERRY
const bool BARRIER_SHAPE[BARRIER_ROWS][BARRIER_COLUMNS] =
{
	{0,1,1,1,0},
	{1,1,1,1,1},
	{1,0,0,0,1}
};
#else
const bool BARRIER_SHAPE[BARRIER_ROWS][BARRIER_COLUMNS] =
{
	{1,1,1}
};
#endif

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
	float enemiesSpeed;
	int enemiesDirection;
	int barrirersQuantity;
	int barriersRow;
	int barriersColumn;
	int enemyShotInterval;
	int cantPlayerShots;
	bool enemiesHands;
	long long lastTimeEnemyShoot;
	long long lastTimeUpdated;
	long long lastTimeMothershipGenerated;
} game_t;

static game_t game;

static void game_create_barriers();
static void game_create_enemy_map(int enemiesRow, int enemiesColumn);
static void update_enemy_bullet(float dt);
static void update_player_bullet(input_t input, float dt);
static void game_level_up(void);
static int game_over(void);

static void playerPositionInit(void);
static void playerInit(void);
static void enemiesInit(int enemiesRow, int enemiesColumn);

static void updatePlayerPosition(input_t player, long long dt);
static bool updateEnemiesPosition(long long dt);

static void shootRandomEnemyBullet(void);
static void shootEnemyBullet(int row, int col);

static void updateBarriers(void);

static bool collisionEnemyBullet(hitbox_t *hitbox);
static bool collisionEnemyHitbox(hitbox_t *hitbox);

static void mothershipGenerate(void);
static void mothershipUpdate(float dt);

static void saveGameState(void);
static void loadGameState(void);

static int getEnemyBitMap(bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX]);

static void loadGameState(void)
{
	FILE *file = fopen("data/savegame.dat", "rb");
	if(!file)
	{
		perror("Failed to open save file");
		return;
	}

	fread(&game, sizeof(game), 1, file);
	fclose(file);
}

static void saveGameState(void)
{
	FILE *file = fopen("data/savegame.dat", "wb");
	if(!file)
	{
		perror("Failed to open save file");
		return;
	}

	fwrite(&game, sizeof(game), 1, file);
	fclose(file);
}

static void playerPositionInit(void)
{
	// Initialize the player's position at the bottom center of the screen
	game.player.hitbox.start.x = 0.5f - PLAYER_WIDTH / 2.0f;
	game.player.hitbox.start.y = PLAYER_BOTTOM_OFFSET;
	game.player.hitbox.end.x = 0.5f + PLAYER_WIDTH / 2.0f;
	game.player.hitbox.end.y = PLAYER_BOTTOM_OFFSET + PLAYER_HEIGHT;
}

static void playerInit(void)
{
	playerPositionInit();
	game.player.lives = INITIAL_PLAYER_LIVES;
	game.player.bullet.active = false;
	game.player.bullet.speed = PLAYER_BULLET_SPEED;
}

static void enemiesInit(int enemiesRow, int enemiesColumn)
{
	game.cantPlayerShots = 0;
	game.mothership.alive = false;
	game.enemiesDirection = 1;
	game.enemiesSpeed = ENEMY_INITIAL_SPEED;
	game.enemiesRow = enemiesRow;
	game.enemiesColumn = enemiesColumn;
	game.enemiesHands = false;
	game.enemyShotInterval = INITIAL_SHOOTING_INTERVAL;
	game.lastTimeEnemyShoot = getTimeMillis();

	game_create_enemy_map(enemiesRow, enemiesColumn);
}

static void updatePlayerPosition(input_t player, long long dt)
{
	float dx = player.direction * dt * PLAYER_SPEED;
	game.player.hitbox.start.x += dx;
	game.player.hitbox.end.x += dx;

	if(game.player.hitbox.start.x < 0)
	{
		game.player.hitbox.start.x = 0;
		game.player.hitbox.end.x = PLAYER_WIDTH;
	}
	else if(game.player.hitbox.end.x > 1)
	{
		game.player.hitbox.start.x = 1.0f - PLAYER_WIDTH;
		game.player.hitbox.end.x = 1.0f;
	}
}

static bool updateEnemiesPosition(long long dt)
{
	float dx = game.enemiesDirection * game.enemiesSpeed * dt;
	int row, col;
	int rightLimit;
	int leftLimit;
	bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];

	if(!getEnemyBitMap(matEnemy))
	{
		return 0;
	}

	for(row = 0, rightLimit = -1, leftLimit = game.enemiesColumn; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			if(matEnemy[row][col])
			{
				if(col > rightLimit)
				{
					rightLimit = col;
				}
				if(col < leftLimit)
				{
					leftLimit = col;
				}
			}
		}
	}

	if((game.enemies[0][rightLimit].hitbox.end.x + dx) > 1.0f || (game.enemies[0][leftLimit].hitbox.start.x + dx) < 0.0f)
	{
		game.enemiesDirection *= -1;
		game.enemiesSpeed += ENEMY_SPEED_INCREMENT_PER_ROW;
		if(game.enemiesSpeed > ENEMY_MAX_SPEED)
		{
			game.enemiesSpeed = ENEMY_MAX_SPEED;
		}
		for(row = 0; row < game.enemiesRow; row++)
		{
			for(col = 0; col < game.enemiesColumn; col++)
			{
				game.enemies[row][col].hitbox.start.y += ENEMY_DESCENT_STEP;
				game.enemies[row][col].hitbox.end.y += ENEMY_DESCENT_STEP;
			}
		}
		dx = game.enemiesDirection * game.enemiesSpeed * dt;
	}

	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			game.enemies[row][col].hitbox.start.x += dx;
			game.enemies[row][col].hitbox.end.x += dx;
		}
	}

	return 1;
}

static bool collisionEnemyBullet(hitbox_t *hitbox)
{
	int row, col;
	for(row = game.enemiesRow - 1; row >= 0; row--)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			if(game.enemies[row][col].bullet.active && HITBOX_COLLISION(*hitbox, game.enemies[row][col].bullet.hitbox))
			{
				game.enemies[row][col].bullet.active = false;
				return 1;
			}
		}
	}
	return 0;
}

static bool collisionEnemyHitbox(hitbox_t *hitbox)
{
	int row, col;
	for(row = game.enemiesRow - 1; row >= 0; row--)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			if(game.enemies[row][col].alive && HITBOX_COLLISION(*hitbox, game.enemies[row][col].hitbox))
			{
				return 1;
			}
		}
	}
	return 0;
}

static void mothershipGenerate(void)
{
	if(game.mothership.alive)
	{
		game.cantPlayerShots = 0;
		return;
	}
	if(game.cantPlayerShots < MOTHERSHIP_TRIGGER_SHOTS)
		return;

	game.mothership.alive = true;
	game.mothership.speed = (rand() % 2) ? MOTHERSHIP_SPEED : -MOTHERSHIP_SPEED;
	if(game.mothership.speed < 0)
	{
		game.mothership.hitbox.start.x = 1.0f;
		game.mothership.hitbox.end.x = 1.0f + MOTHERSHIP_WIDTH;
	}
	else
	{
		game.mothership.hitbox.start.x = 0.0f - MOTHERSHIP_WIDTH;
		game.mothership.hitbox.end.x = 0.0f;
	}

	game.mothership.hitbox.start.y = MOTHERSHIP_TOP_OFFSET;
	game.mothership.hitbox.end.y = MOTHERSHIP_TOP_OFFSET + MOTHERSHIP_HEIGHT;

	game.cantPlayerShots = 0;
	game.lastTimeMothershipGenerated = getTimeMillis();
	playSound_play(SOUND_UFO_LOW);
}

static void mothershipUpdate(float dt)
{
	mothershipGenerate();

	if(!game.mothership.alive)
	{
		return;
	}

	long long currentTime = getTimeMillis();
	if(currentTime - game.lastTimeMothershipGenerated > 690)
	{
		playSound_play(SOUND_UFO_LOW);
		game.lastTimeMothershipGenerated = currentTime;
	}

	game.mothership.hitbox.start.x += game.mothership.speed * dt;
	game.mothership.hitbox.end.x += game.mothership.speed * dt;

	if((game.mothership.hitbox.end.x < 0 && game.mothership.speed < 0) || (game.mothership.hitbox.start.x > 1.0f && game.mothership.speed > 0))
	{
		game.mothership.alive = false;
		return;
	}

	if(HITBOX_COLLISION(game.player.bullet.hitbox, game.mothership.hitbox))
	{
		game.score += MOTHERSHIP_SCORE;
		playSound_play(SOUND_EXPLOSION);
		game.mothership.alive = false;
		game.player.bullet.active = false;
	}
}

static void updateBarriers(void)
{
	int elem, row, col;
	for(elem = 0; elem < BARRIER_QUANTITY_MAX; elem++)
	{
		for(row = 0; row < BARRIER_ROWS; row++)
		{
			for(col = 0; col < BARRIER_COLUMNS; col++)
			{
				if(!game.barriers[elem].block[row][col].lives)
				{
					continue;
				}

				if(collisionEnemyHitbox(&game.barriers[elem].block[row][col].hitbox))
				{
					game.barriers[elem].block[row][col].lives = 0;
					continue;
				}

				if(collisionEnemyBullet(&game.barriers[elem].block[row][col].hitbox))
				{
					game.barriers[elem].block[row][col].lives--;
					if(!game.barriers[elem].block[row][col].lives)
					{
						continue;
					}
				}

				if(HITBOX_COLLISION(game.player.bullet.hitbox, game.barriers[elem].block[row][col].hitbox))
				{
					game.barriers[elem].block[row][col].lives--;
					game.player.bullet.active = false;
				}
			}
		}
	}
}

static void shootRandomEnemyBullet(void)
{
	long long time = getTimeMillis();
	long long dt = time - game.lastTimeEnemyShoot;
	int columnHasEnemiesThatCanShoot[game.enemiesColumn];
	int thereIsAtLeastOneThatCanShoot;
	int col;
	int row;

	if(dt < game.enemyShotInterval)
		return;

	for(col = 0, thereIsAtLeastOneThatCanShoot = 0; col < game.enemiesColumn; col++)
	{
		columnHasEnemiesThatCanShoot[col] = 0;
		for(row = 0; row < game.enemiesRow; row++)
		{
			if(game.enemies[row][col].alive && !game.enemies[row][col].bullet.active)
			{
				columnHasEnemiesThatCanShoot[col] = 1;
				thereIsAtLeastOneThatCanShoot = 1;
				break;
			}
		}
	}

	if(!thereIsAtLeastOneThatCanShoot)
		return;

	do
	{
		col = rand() % game.enemiesColumn;
	} while(!columnHasEnemiesThatCanShoot[col]);

	for(row = game.enemiesRow - 1; row >= 0 && !game.enemies[row][col].alive; row--);

	shootEnemyBullet(row, col);
	game.lastTimeEnemyShoot = time;
}

static int getEnemyBitMap(bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX])
{
	int row, col;
	int ammountOfEnemiesAlive;

	for(row = 0, ammountOfEnemiesAlive = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			matEnemy[row][col] = game.enemies[row][col].alive;
			ammountOfEnemiesAlive += matEnemy[row][col];
		}
	}

	return ammountOfEnemiesAlive;
}

static void game_create_enemy_map(int enemiesRow, int enemiesColumn)
{
	float total_width = enemiesColumn * ENEMY_WIDTH + (enemiesColumn - 1) * ENEMY_H_SPACING;
	float start_x = (1.0f - total_width) / 2.0f;
	float frow;
	int row, col;

	for(row = 0; row < ENEMIES_ROW_MAX; row++)
	{
		for(col = 0; col < ENEMIES_COLUMNS_MAX; col++)
		{
			if(row >= enemiesRow || col >= enemiesColumn)
			{
				game.enemies[row][col].bullet.active = false;
				game.enemies[row][col].alive = false;
				continue;
			}

			game.enemies[row][col].hitbox.start.x = start_x + col * (ENEMY_WIDTH + ENEMY_H_SPACING);
			game.enemies[row][col].hitbox.start.y = ENEMY_TOP_OFFSET + row * (ENEMY_HEIGHT + ENEMY_V_SPACING);
			game.enemies[row][col].hitbox.end.x = game.enemies[row][col].hitbox.start.x + ENEMY_WIDTH;
			game.enemies[row][col].hitbox.end.y = game.enemies[row][col].hitbox.start.y + ENEMY_HEIGHT;
			game.enemies[row][col].alive = true;

			// Tipo de enemigo según la proporción de la fila
			frow = (float)row / enemiesRow;
			if(frow < 0.2f)
			{
				game.enemies[row][col].type = ALIEN_TIER3;
			}
			else if(frow < 0.6f)
			{
				game.enemies[row][col].type = ALIEN_TIER2;
			}
			else
			{
				game.enemies[row][col].type = ALIEN_TIER1;
			}

			game.enemies[row][col].bullet.active = false;
		}
	}
}

static void game_create_barriers()
{
	#define TOTAL_WIDTH (BARRIER_QUANTITY_MAX * BARRIER_WIDTH + (BARRIER_QUANTITY_MAX-1) * BARRIER_SPACING)
	#define START_X ((1.0f - TOTAL_WIDTH) / 2.0f)
	#define BASE_Y (1.0f - BARRIER_BOTTOM_OFFSET - BARRIER_HEIGHT)

	float barrier_x;
	int elem, row, col;

	for(elem = 0; elem < BARRIER_QUANTITY_MAX; elem++)
	{
		barrier_x = START_X + elem * (BARRIER_WIDTH + BARRIER_SPACING);
		for(row = 0; row < BARRIER_ROWS; row++) 
		{
			for(col = 0; col < BARRIER_COLUMNS; col++)
			{
				if(!BARRIER_SHAPE[row][col])
				{
					game.barriers[elem].block[row][col].lives = 0;
					continue;
				}
				game.barriers[elem].block[row][col].hitbox.start.x = barrier_x + col * BLOCK_WIDTH;
				game.barriers[elem].block[row][col].hitbox.start.y = BASE_Y + row * BLOCK_HEIGHT;
				game.barriers[elem].block[row][col].hitbox.end.x = barrier_x + (col+1) * BLOCK_WIDTH;
				game.barriers[elem].block[row][col].hitbox.end.y = BASE_Y + (row+1) * BLOCK_HEIGHT;
				#ifndef RASPBERRY
					game.barriers[elem].block[row][col].lives = 1;
				#else
					game.barriers[elem].block[row][col].lives = BARRIER_LIVES;
				#endif
			}
		}
	}
}

static void shootEnemyBullet(int row, int col)
{
	if(game.enemies[row][col].bullet.active)
		return;

	game.enemies[row][col].bullet.hitbox.start.x = game.enemies[row][col].hitbox.start.x + ENEMY_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
	game.enemies[row][col].bullet.hitbox.end.x = game.enemies[row][col].hitbox.start.x + ENEMY_WIDTH / 2.0f + BULLET_WIDTH / 2.0f;
	game.enemies[row][col].bullet.hitbox.start.y = game.enemies[row][col].hitbox.end.y;
	game.enemies[row][col].bullet.hitbox.end.y = game.enemies[row][col].hitbox.end.y + BULLET_HEIGHT;
	game.enemies[row][col].bullet.active = true;
}

static void update_enemy_bullet(float dt)
{
	int row, col;
	for(row = 0; row < game.enemiesColumn; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			if(!game.enemies[row][col].bullet.active)
			{
				continue;
			}

			game.enemies[row][col].bullet.hitbox.start.y += ENEMY_BULLET_SPEED * dt;
			game.enemies[row][col].bullet.hitbox.end.y += ENEMY_BULLET_SPEED * dt;

			if(game.enemies[row][col].bullet.hitbox.end.y > 1.0f) 
			{
				game.enemies[row][col].bullet.active = false;
				continue;
			}

			if(HITBOX_COLLISION(game.enemies[row][col].bullet.hitbox, game.player.hitbox))
			{
				game.player.lives--;
				game.enemies[row][col].bullet.active = false;
			}
		}
	}
}

static void update_player_bullet(input_t input, float dt)
{
	// Si se presionó disparo y no hay bala activa
	if(input.shot && !game.player.bullet.active)
	{
		game.player.bullet.active = true;
		game.player.bullet.hitbox.start.x = game.player.hitbox.start.x + PLAYER_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
		game.player.bullet.hitbox.start.y = game.player.hitbox.start.y - BULLET_HEIGHT;
		game.player.bullet.hitbox.end.x = game.player.hitbox.start.x + PLAYER_WIDTH / 2.0f + BULLET_WIDTH / 2.0f;
		game.player.bullet.hitbox.end.y = game.player.hitbox.start.y;
		playSound_play(SOUND_SHOOT);
		game.cantPlayerShots++;
	}

	if(!game.player.bullet.active)
	{
		return;
	}

	// Movimiento
	game.player.bullet.hitbox.start.y -= game.player.bullet.speed * dt;
	game.player.bullet.hitbox.end.y -= game.player.bullet.speed * dt;

	if(game.player.bullet.hitbox.end.y < 0.0f) 
	{
		game.player.bullet.active = false;
		return;
	}

	int row, col;
	// Colisión con aliens
	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			if(!game.enemies[row][col].alive)
			{
				continue;
			}

			if(HITBOX_COLLISION(game.enemies[row][col].hitbox, game.player.bullet.hitbox))
			{
				game.enemies[row][col].alive = false;
				game.player.bullet.active = false;
				playSound_play(SOUND_EXPLOSION);
				playSound_play(SOUND_INVADER_KILLED);

				game.enemyShotInterval -= ENEMY_SHOOTING_INTERVAL_DECREMENT;
				if(game.enemyShotInterval < MIN_ENEMY_SHOOTING_INTERVAL)
				{
					game.enemyShotInterval = MIN_ENEMY_SHOOTING_INTERVAL;
				}

				switch (game.enemies[row][col].type) 
				{
				case ALIEN_TIER1: 
					game.score += 10; 
					break;
				case ALIEN_TIER2:
					game.score += 20; 
					break;
				case ALIEN_TIER3: 
					game.score += 30;
					break;
				default:
					break;
				}
			}
		}
	}

	// Colisión con balas enemigas
	if(collisionEnemyBullet(&game.player.bullet.hitbox))
	{
		game.player.bullet.active = false;
	}
}

static void game_level_up()
{
	game.level++;
	game.player.lives += game.player.lives < MAX_PLAYER_LIVES;
	game.enemiesSpeed = ENEMY_INITIAL_SPEED + game.level * ENEMY_SPEED_INCREMENT_PER_LEVEL;
	game.enemiesSpeed = (game.enemiesSpeed > ENEMY_MAX_SPEED) ? ENEMY_MAX_SPEED : game.enemiesSpeed;

	game.enemiesDirection = 1;
	game.enemyShotInterval = INITIAL_SHOOTING_INTERVAL;

	game.cantPlayerShots = 0;
	game.mothership.alive = false;
	playerPositionInit();
	game_create_barriers();
	game_create_enemy_map(game.enemiesRow, game.enemiesColumn);
	mothershipGenerate();
}

static int game_over(void)
{
	int row, col, gameContinues;

	if(game.player.lives <= 0)
	{
		return 1;
	}

	for(row = game.enemiesRow - 1, gameContinues = 0; row >= 0 && !gameContinues; row--)
	{
		for(col = 0; col < game.enemiesColumn && !gameContinues; col++)
		{
			if(game.enemies[row][col].alive == true)
			{
				if(game.enemies[row][col].hitbox.end.y >= game.player.hitbox.start.y)
				{
					return 1;   // Game Over
				}
				gameContinues = 1;
			}
		}
	}

	return 0;
}

void game_init(int enemiesRow, int enemiesColumn, bool resumeLastGame)
{
	if(resumeLastGame)
	{
		loadGameState();
		long long currentTime = getTimeMillis();
		game.lastTimeUpdated = currentTime;
		game.lastTimeEnemyShoot = currentTime;
		game.lastTimeMothershipGenerated = currentTime;		// Chequear si es necesario.
		return;
	}

	playerInit();
	enemiesInit(enemiesRow, enemiesColumn);
	game_create_barriers();
	srand(time(NULL));

	game.score = 0;
	game.level = 0;
	game.state = RUNNING;

	game.lastTimeUpdated = getTimeMillis();
}

int game_update(input_t player)
{
	if(player.pause)
	{
		game.lastTimeUpdated = getTimeMillis();
		//playSound_stop(SOUND_UFO_LOW);
		printf("pausado");
		saveGameState();
		if(player.exit)
		{
			game.state = QUIT;
			return QUIT;
		}
		return RUNNING;
	}

	static float enemiesMovement = 0;
	long long dt = (getTimeMillis() - game.lastTimeUpdated);

	updatePlayerPosition(player, dt);

	if(!updateEnemiesPosition(dt)) // Si no quedan enemigos vivos -> level up
	{
		game_level_up();
		game.lastTimeUpdated = getTimeMillis();
		return RUNNING; // No enemies left, game continues
	}

	enemiesMovement += dt * game.enemiesSpeed;
	if(enemiesMovement > 0.1f)
	{
		playSound_play((game.enemiesHands) ? SOUND_FAST1 : SOUND_FAST2);
		//printf("Enemies hands: %d\n", game.enemiesHands);
		enemiesMovement = 0;
		game.enemiesHands = !game.enemiesHands;
	}

	update_enemy_bullet(dt);
	update_player_bullet(player, dt);
	shootRandomEnemyBullet();
	mothershipUpdate(dt);
	updateBarriers();

	game.lastTimeUpdated = getTimeMillis();
	game.state = game_over();
	if(game.state == GAME_OVER)
	{
		saveGameState();
	}
	return game.state;
}

void getEnemiesBulletsInfo(bullet_t matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX])
{
	int row, col;
	for(row = 0; row < ENEMIES_ROW_MAX; row++)
	{
		for(col = 0; col < ENEMIES_COLUMNS_MAX; col++)
		{
			matEnemy[row][col] = game.enemies[row][col].bullet;
		}
	}
}

int getPlayerLives(void)
{
	return game.player.lives;
}

hitbox_t getMothershipPosition(void)
{
	return game.mothership.hitbox;
}

bool getIsMothershipAlive(void)
{
	return game.mothership.alive;
}

/**
 * @brief Returns the position of the player.
 *
 * @return coord_t Struct with player's position
 */
hitbox_t getPlayerPosition(void)
{
	return game.player.hitbox;
}

/**
 * @brief Returns the position of a specific barrier cell.
 *
 * @param barrier Index of the barrier
 * @param row Row within the barrier
 * @param column Column within the barrier
 * @return coord_t Struct with the barrier cell's position
 */
hitbox_t getEnemyPosition(int row, int column)
{
	return game.enemies[row][column].hitbox;
}

bool getIsEnemyAlive(int row, int column)
{
	return game.enemies[row][column].alive;
}

/**
 * @brief Returns the position of the projectile.
 *
 * @return coord_t Struct with projectile's position
 */
// hitbox_t getProjectilePosition(void)
// {
// 	hitbox_t hitbox = game.|;
// 	return hitbox;
// }

bool getBarrierIsAlive(int barrier, int row, int column)
{
	return game.barriers[barrier].block[row][column].lives > 0;
}

hitbox_t getBarrierPosition(int barrier, int row, int column)
{
	return game.barriers[barrier].block[row][column].hitbox;
}

int getEnemyTier(int row)
{
	return game.enemies[row][0].type;
}

long long getTimeMillis(void)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;
}

bullet_t getPlayerBulletinfo(void)
{
	return game.player.bullet;
}

bool getEnemiesHands(void)
{
	return game.enemiesHands;
}

int getLevel(void)
{
	return game.level;
}

int getScore(void)
{
	return game.score;
}