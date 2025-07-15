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
 * 	MAGLIOLA, Nicolas
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

/* ======================== CONSTANTS ======================== */
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

/* ======================== TYPE DEFINITIONS ======================== */
/* ---------------------------------------------------
 * @brief Main game state structure containing all entities and game data
 * ---------------------------------------------------*/
typedef struct
{
	player_t player;                                 ///< Player character data
	enemy_t enemies[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX]; ///< 2D array of enemies
	barrier_t barriers[BARRIER_QUANTITY_MAX];        ///< Array of defensive barriers
	mothership_t mothership;                         ///< Special mothership entity
	int score;                                       ///< Current player score
	int level;                                       ///< Current game level
	int state;                                       ///< Current game state
	int prevState;                                   ///< Previous game state
	int enemiesRow;                                  ///< Current number of enemy rows
	int enemiesColumn;                               ///< Current number of enemy columns
	float enemiesSpeed;                              ///< Current enemy movement speed
	int enemiesDirection;                            ///< Current enemy movement direction (1=right, -1=left)
	int barrirersQuantity;                           ///< Number of active barriers
	int barriersRow;                                 ///< Barrier row count (unused?)
	int barriersColumn;                              ///< Barrier column count (unused?)
	int enemyShotInterval;                           ///< Time between enemy shots
	int cantPlayerShots;                             ///< Player shot counter (for mothership trigger)
	bool enemiesHands;                               ///< Animation state for enemies
	long long lastTimeEnemyShoot;                    ///< Timestamp of last enemy shot
	long long lastTimeUpdated;                       ///< Timestamp of last game update
	long long lastTimeMothershipGenerated;           ///< Timestamp of last mothership spawn
} game_t;

/* ======================== STATIC VARIABLES ======================== */
static game_t game;  ///< Global game state instance

/* ======================== PRIVATE FUNCTION DECLARATIONS ======================== */
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
static bool loadGameState(void);
static int getEnemyBitMap(bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX]);

/* ======================== PUBLIC FUNCTION IMPLEMENTATIONS ======================== */

/* ---------------------------------------------------
 * @brief Initialize game state
 * @param enemiesRow Number of enemy rows
 * @param enemiesColumn Number of enemy columns
 * @param resumeLastGame Whether to load saved game
 * @return true if initialization succeeded, false otherwise
 * ---------------------------------------------------*/
bool game_init(int enemiesRow, int enemiesColumn, bool resumeLastGame)
{
	// Load saved game if requested
	if(resumeLastGame)
	{
		if(!loadGameState())
			return false;
			
		// Initialize timestamps
		long long currentTime = getTimeMillis();
		game.lastTimeUpdated = currentTime;
		game.lastTimeEnemyShoot = currentTime;
		game.lastTimeMothershipGenerated = currentTime;
		return true;
	}

	// Initialize game entities
	playerInit();
	enemiesInit(enemiesRow, enemiesColumn);
	game_create_barriers();
	
	// Seed random number generator
	srand(time(NULL));

	// Initialize game state
	game.score = 0;
	game.level = 0;
	game.state = RUNNING;

	// Initialize timestamp
	game.lastTimeUpdated = getTimeMillis();
	return true;
}

/* ---------------------------------------------------
 * @brief Update game state based on input and time
 * @param player Player input state
 * @return Current game state
 * ---------------------------------------------------*/
int game_update(input_t player)
{
	// Handle pause input
	if(player.pause)
	{
		game.prevState = PAUSED;
		game.lastTimeUpdated = getTimeMillis();
		playSound_stop(SOUND_UFO_LOW);
		saveGameState();
		
		// Handle exit request
		if(player.exit)
		{
			game.state = QUIT;
			return QUIT;
		}
		return RUNNING;
	}

	game.prevState = game.state;

	// Level up state machine variables
	static int LevelUpState = 0;
	static long long lastTimeLevelUp = 0;
	static float enemiesMovement = 0;  // Accumulated enemy movement
	
	long long currentTime = getTimeMillis();
	long long dt = (currentTime - game.lastTimeUpdated);

	// Level up if no enemies remain
	if(!updateEnemiesPosition(dt))
	{
		switch (LevelUpState)
		{
		case 0:  // Start level up sequence
			lastTimeLevelUp = getTimeMillis();
			LevelUpState = 1;
			game.level++;
			playSound_setMusicVolume(0.2);
			playSound_play(SOUND_LEVELUP);
			break;

		case 1:  // Wait before resetting level
			currentTime = getTimeMillis();
			if(currentTime - lastTimeLevelUp > 1000)
			{
				playSound_setMusicVolume(1);
				game_level_up();
				lastTimeLevelUp = currentTime;
				LevelUpState = 0;
				game.lastTimeUpdated = currentTime;
			}
			break;
		}
		return RUNNING;
	}

	// Play enemy movement sounds at intervals
	enemiesMovement += dt * game.enemiesSpeed;
	if(enemiesMovement > 0.1f)
	{
		playSound_play((game.enemiesHands) ? SOUND_FAST1 : SOUND_FAST2);
		enemiesMovement = 0;
		game.enemiesHands = !game.enemiesHands;
	}

	// Update game entities
	updatePlayerPosition(player, dt);
	update_enemy_bullet(dt);
	update_player_bullet(player, dt);
	shootRandomEnemyBullet();
	mothershipUpdate(dt);
	updateBarriers();

	// Update timestamp and check game over
	game.lastTimeUpdated = getTimeMillis();
	game.state = game_over();
	
	// Save state on game over
	if(game.state == GAME_OVER)
	{
		saveGameState();
	}
	
	return game.state;
}

/* ======================== PUBLIC GETTER IMPLEMENTATIONS ======================== */

/* ---------------------------------------------------
 * @brief Get enemy bullet states
 * @param matEnemy Output matrix for enemy bullets
 * ---------------------------------------------------*/
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

/* ---------------------------------------------------
 * @brief Get current player lives
 * @return Number of remaining lives
 * ---------------------------------------------------*/
int getPlayerLives(void)
{
	return game.player.lives;
}

/* ---------------------------------------------------
 * @brief Get mothership position
 * @return Mothership hitbox
 * ---------------------------------------------------*/
hitbox_t getMothershipPosition(void)
{
	return game.mothership.hitbox;
}

/* ---------------------------------------------------
 * @brief Check if mothership is active
 * @return true if active, false otherwise
 * ---------------------------------------------------*/
bool getIsMothershipAlive(void)
{
	return game.mothership.alive;
}

/* ---------------------------------------------------
 * @brief Get player position
 * @return Player hitbox
 * ---------------------------------------------------*/
hitbox_t getPlayerPosition(void)
{
	return game.player.hitbox;
}

/* ---------------------------------------------------
 * @brief Get enemy position
 * @param row Enemy row index
 * @param column Enemy column index
 * @return Enemy hitbox
 * ---------------------------------------------------*/
hitbox_t getEnemyPosition(int row, int column)
{
	return game.enemies[row][column].hitbox;
}

/* ---------------------------------------------------
 * @brief Check if enemy is alive
 * @param row Enemy row index
 * @param column Enemy column index
 * @return true if alive, false otherwise
 * ---------------------------------------------------*/
bool getIsEnemyAlive(int row, int column)
{
	return game.enemies[row][column].alive;
}

/* ---------------------------------------------------
 * @brief Check if barrier block is alive
 * @param barrier Barrier index
 * @param row Block row index
 * @param column Block column index
 * @return true if alive, false otherwise
 * ---------------------------------------------------*/
bool getBarrierIsAlive(int barrier, int row, int column)
{
	return game.barriers[barrier].block[row][column].lives > 0;
}

/* ---------------------------------------------------
 * @brief Get barrier block position
 * @param barrier Barrier index
 * @param row Block row index
 * @param column Block column index
 * @return Block hitbox
 * ---------------------------------------------------*/
hitbox_t getBarrierPosition(int barrier, int row, int column)
{
	return game.barriers[barrier].block[row][column].hitbox;
}

/* ---------------------------------------------------
 * @brief Get enemy tier for a row
 * @param row Enemy row index
 * @return Enemy tier type
 * ---------------------------------------------------*/
int getEnemyTier(int row)
{
	return game.enemies[row][0].type;
}

/* ---------------------------------------------------
 * @brief Get current system time in milliseconds
 * @return Timestamp in milliseconds
 * ---------------------------------------------------*/
long long getTimeMillis(void)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;
}

/* ---------------------------------------------------
 * @brief Get player bullet state
 * @return Player bullet data
 * ---------------------------------------------------*/
bullet_t getPlayerBulletinfo(void)
{
	return game.player.bullet;
}

/* ---------------------------------------------------
 * @brief Get enemy animation state
 * @return true if "hands" animation frame, false otherwise
 * ---------------------------------------------------*/
bool getEnemiesHands(void)
{
	return game.enemiesHands;
}

/* ---------------------------------------------------
 * @brief Get current game level
 * @return Current level number
 * ---------------------------------------------------*/
int getLevel(void)
{
	return game.level;
}

/* ---------------------------------------------------
 * @brief Get current player score
 * @return Current score
 * ---------------------------------------------------*/
int getScore(void)
{
	return game.score;
}

/* ======================== PRIVATE FUNCTION IMPLEMENTATIONS ======================== */

/* ---------------------------------------------------
 * @brief Load game state from save file
 * @return true if load succeeded, false otherwise
 * ---------------------------------------------------*/
static bool loadGameState(void)
{
	FILE *file = fopen("data/savegame.dat", "rb");
	if(!file)
	{
		perror("Failed to open save file");
		return false;
	}

	// Read entire game state structure from file
	fread(&game, sizeof(game), 1, file);
	fclose(file);
	return true;
}

/* ---------------------------------------------------
 * @brief Save current game state to file
 * ---------------------------------------------------*/
static void saveGameState(void)
{
	FILE *file = fopen("data/savegame.dat", "wb");
	if(!file)
	{
		perror("Failed to open save file");
		return;
	}

	// Write entire game state structure to file
	fwrite(&game, sizeof(game), 1, file);
	fclose(file);
}

/* ---------------------------------------------------
 * @brief Initialize player position at bottom center of screen
 * ---------------------------------------------------*/
static void playerPositionInit(void)
{
	// Initialize player position at bottom center
	game.player.hitbox.start.x = 0.5f - PLAYER_WIDTH / 2.0f;
	game.player.hitbox.start.y = PLAYER_BOTTOM_OFFSET;
	game.player.hitbox.end.x = 0.5f + PLAYER_WIDTH / 2.0f;
	game.player.hitbox.end.y = PLAYER_BOTTOM_OFFSET + PLAYER_HEIGHT;
}

/* ---------------------------------------------------
 * @brief Initialize player state with default values
 * ---------------------------------------------------*/
static void playerInit(void)
{
	playerPositionInit();
	game.player.lives = INITIAL_PLAYER_LIVES;
	game.player.bullet.active = false;
	game.player.bullet.speed = PLAYER_BULLET_SPEED;
	game.player.cooldown = 0;  // Reset shot cooldown
}

/* ---------------------------------------------------
 * @brief Initialize enemy grid with specified dimensions
 * @param enemiesRow Number of enemy rows
 * @param enemiesColumn Number of enemy columns
 * ---------------------------------------------------*/
static void enemiesInit(int enemiesRow, int enemiesColumn)
{
	// Reset game state counters
	game.cantPlayerShots = 0;
	game.mothership.alive = false;
	game.enemiesDirection = 1;  // Start moving right
	game.enemiesSpeed = ENEMY_INITIAL_SPEED;
	game.enemiesRow = enemiesRow;
	game.enemiesColumn = enemiesColumn;
	game.enemiesHands = false;  // Animation state
	game.enemyShotInterval = INITIAL_SHOOTING_INTERVAL;
	game.lastTimeEnemyShoot = getTimeMillis();  // Initialize shot timer

	// Create enemy formation
	game_create_enemy_map(enemiesRow, enemiesColumn);
}

/* ---------------------------------------------------
 * @brief Update player position based on input
 * @param player Input state structure
 * @param dt Time delta since last update (ms)
 * ---------------------------------------------------*/
static void updatePlayerPosition(input_t player, long long dt)
{
	// Calculate movement based on input direction
	float dx = player.direction * dt * PLAYER_SPEED;
	game.player.hitbox.start.x += dx;
	game.player.hitbox.end.x += dx;

	// Constrain player within screen bounds
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

/* ---------------------------------------------------
 * @brief Update enemy positions and handle screen edge collision
 * @param dt Time delta since last update (ms)
 * @return true if enemies remain, false if all destroyed
 * ---------------------------------------------------*/
static bool updateEnemiesPosition(long long dt)
{
	// Calculate movement delta
	float dx = game.enemiesDirection * game.enemiesSpeed * dt;
	int row, col;
	int rightLimit;
	int leftLimit;
	bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];	// Enemy alive state matrix

	// Get enemy state matrix and check if any remain
	if(!getEnemyBitMap(matEnemy))
	{
		return 0;	// No enemies left
	}

	// Find leftmost and rightmost living enemies
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

	// Handle screen edge collision
	if((game.enemies[0][rightLimit].hitbox.end.x + dx) > 1.0f || (game.enemies[0][leftLimit].hitbox.start.x + dx) < 0.0f)
	{
		// Reverse direction and increase speed
		game.enemiesDirection *= -1;
		game.enemiesSpeed += ENEMY_SPEED_INCREMENT_PER_ROW;

		// Cap maximum enemy speed
		if(game.enemiesSpeed > ENEMY_MAX_SPEED)
		{
			game.enemiesSpeed = ENEMY_MAX_SPEED;
		}

		// Move enemies down one step
		for(row = 0; row < game.enemiesRow; row++)
		{
			for(col = 0; col < game.enemiesColumn; col++)
			{
				game.enemies[row][col].hitbox.start.y += ENEMY_DESCENT_STEP;
				game.enemies[row][col].hitbox.end.y += ENEMY_DESCENT_STEP;
			}
		}

		// Recalculate movement for new direction
		dx = (dx > 0) ? (1.0f - game.enemies[0][rightLimit].hitbox.end.x) : (0.0f - game.enemies[0][leftLimit].hitbox.start.x);
	}

	// Apply movement to all enemies
	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			game.enemies[row][col].hitbox.start.x += dx;
			game.enemies[row][col].hitbox.end.x += dx;
		}
	}

	return 1;	// Enemies remain
}

/* ---------------------------------------------------
 * @brief Check for collision between a hitbox and enemy bullets
 * @param hitbox Pointer to hitbox to check
 * @return true if collision detected, false otherwise
 * ---------------------------------------------------*/
static bool collisionEnemyBullet(hitbox_t *hitbox)
{
	int row, col;
	// Iterate from bottom to top for more realistic collision
	for(row = game.enemiesRow - 1; row >= 0; row--)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			// Skip inactive bullets
			if(!game.enemies[row][col].bullet.active) continue;
			
			// Check collision with active bullet
			if(HITBOX_COLLISION(*hitbox, game.enemies[row][col].bullet.hitbox))
			{
				// Deactivate bullet on collision
				game.enemies[row][col].bullet.active = false;
				return true;
			}
		}
	}
	return false;  // No collision
}

/* ---------------------------------------------------
 * @brief Check for collision between a hitbox and any enemy
 * @param hitbox Pointer to hitbox to check
 * @return true if collision detected, false otherwise
 * ---------------------------------------------------*/
static bool collisionEnemyHitbox(hitbox_t *hitbox)
{
	int row, col;
	// Iterate from bottom to top for more realistic collision
	for(row = game.enemiesRow - 1; row >= 0; row--)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			// Skip dead enemies
			if(!game.enemies[row][col].alive) continue;
			
			// Check collision with living enemy
			if(HITBOX_COLLISION(*hitbox, game.enemies[row][col].hitbox))
			{
				return true;
			}
		}
	}
	return false;  // No collision
}

/* ---------------------------------------------------
 * @brief Generate mothership if conditions are met
 * ---------------------------------------------------*/
static void mothershipGenerate(void)
{
	// Skip if mothership already active
	if(game.mothership.alive) 
	{
		game.cantPlayerShots = 0;
		return;
	}
	
	// Require minimum player shots before spawning
	if(game.cantPlayerShots < MOTHERSHIP_TRIGGER_SHOTS)
		return;

	// Activate mothership
	game.mothership.alive = true;
	
	// Set random movement direction
	game.mothership.speed = (rand() % 2) ? MOTHERSHIP_SPEED : -MOTHERSHIP_SPEED;
	
	// Position based on direction
	if(game.mothership.speed < 0)  // Moving left
	{
		game.mothership.hitbox.start.x = 1.0f;
		game.mothership.hitbox.end.x = 1.0f + MOTHERSHIP_WIDTH;
	}
	else  // Moving right
	{
		game.mothership.hitbox.start.x = 0.0f - MOTHERSHIP_WIDTH;
		game.mothership.hitbox.end.x = 0.0f;
	}

	// Set vertical position
	game.mothership.hitbox.start.y = MOTHERSHIP_TOP_OFFSET;
	game.mothership.hitbox.end.y = MOTHERSHIP_TOP_OFFSET + MOTHERSHIP_HEIGHT;

	// Reset shot counter and timestamp
	game.cantPlayerShots = 0;
	game.lastTimeMothershipGenerated = getTimeMillis();
}

/* ---------------------------------------------------
 * @brief Update mothership position and handle collisions
 * @param dt Time delta since last update (ms)
 * ---------------------------------------------------*/
static void mothershipUpdate(float dt)
{
	long long currentTime = getTimeMillis();
	
	// Attempt to generate mothership
	mothershipGenerate();

	// Skip update if mothership inactive
	if(!game.mothership.alive)
	{
		return;
	}
	
	// Play UFO sound at regular intervals
	if(currentTime - game.lastTimeMothershipGenerated >= 164)
	{
		playSound_play(SOUND_UFO_HIGH);
		game.lastTimeMothershipGenerated = currentTime;
	}

	// Update mothership position
	game.mothership.hitbox.start.x += game.mothership.speed * dt;
	game.mothership.hitbox.end.x += game.mothership.speed * dt;

	// Deactivate if moved off-screen
	if((game.mothership.hitbox.end.x < 0 && game.mothership.speed < 0) || 
	   (game.mothership.hitbox.start.x > 1.0f && game.mothership.speed > 0))
	{
		game.mothership.alive = false;
		return;
	}

	// Check collision with player bullet
	if(HITBOX_COLLISION(game.player.bullet.hitbox, game.mothership.hitbox))
	{
		// Award points and play sound
		game.score += MOTHERSHIP_SCORE;
		playSound_play(SOUND_MOTHERSHIPDEATH);
		
		// Deactivate mothership and bullet
		game.mothership.alive = false;
		game.player.bullet.active = false;
	}
}

/* ---------------------------------------------------
 * @brief Update barrier states and handle collisions
 * ---------------------------------------------------*/
static void updateBarriers(void)
{
	int elem, row, col;
	// Process each barrier
	for(elem = 0; elem < BARRIER_QUANTITY_MAX; elem++)
	{
		// Process each block in barrier
		for(row = 0; row < BARRIER_ROWS; row++)
		{
			for(col = 0; col < BARRIER_COLUMNS; col++)
			{
				// Skip destroyed blocks
				if(!game.barriers[elem].block[row][col].lives)
				{
					continue;
				}

				// Handle collision with enemy
				if(collisionEnemyHitbox(&game.barriers[elem].block[row][col].hitbox))
				{
					game.barriers[elem].block[row][col].lives = 0;
					continue;
				}

				// Handle collision with enemy bullet
				if(collisionEnemyBullet(&game.barriers[elem].block[row][col].hitbox))
				{
					game.barriers[elem].block[row][col].lives--;
					if(!game.barriers[elem].block[row][col].lives)
					{
						continue;
					}
				}

				// Handle collision with player bullet
				if(HITBOX_COLLISION(game.player.bullet.hitbox, game.barriers[elem].block[row][col].hitbox))
				{
					game.barriers[elem].block[row][col].lives--;
					game.player.bullet.active = false;
				}
			}
		}
	}
}

/* ---------------------------------------------------
 * @brief Select random enemy to shoot based on conditions
 * ---------------------------------------------------*/
static void shootRandomEnemyBullet(void)
{
	long long time = getTimeMillis();
	long long dt = time - game.lastTimeEnemyShoot;
	int columnHasEnemiesThatCanShoot[game.enemiesColumn];
	int thereIsAtLeastOneThatCanShoot;
	int col;
	int row;

	// Check if enough time has passed for next shot
	if(dt < game.enemyShotInterval)
		return;

	// Identify columns with shootable enemies
	for(col = 0, thereIsAtLeastOneThatCanShoot = 0; col < game.enemiesColumn; col++)
	{
		columnHasEnemiesThatCanShoot[col] = 0;
		for(row = 0; row < game.enemiesRow; row++)
		{
			// Enemy must be alive and not have active bullet
			if(game.enemies[row][col].alive && !game.enemies[row][col].bullet.active)
			{
				columnHasEnemiesThatCanShoot[col] = 1;
				thereIsAtLeastOneThatCanShoot = 1;
				break;
			}
		}
	}

	// Abort if no shootable enemies found
	if(!thereIsAtLeastOneThatCanShoot)
		return;

	// Select random column with shootable enemies
	do
	{
		col = rand() % game.enemiesColumn;
	} while(!columnHasEnemiesThatCanShoot[col]);

	// Find bottom-most enemy in column
	for(row = game.enemiesRow - 1; row >= 0 && !game.enemies[row][col].alive; row--);

	// Shoot from selected enemy
	shootEnemyBullet(row, col);
	game.lastTimeEnemyShoot = time;
}

/* ---------------------------------------------------
 * @brief Generate enemy state matrix and count living enemies
 * @param matEnemy Output matrix for enemy alive states
 * @return Number of living enemies
 * ---------------------------------------------------*/
static int getEnemyBitMap(bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX])
{
	int row, col;
	int ammountOfEnemiesAlive = 0;

	// Populate matrix and count living enemies
	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			matEnemy[row][col] = game.enemies[row][col].alive;
			ammountOfEnemiesAlive += matEnemy[row][col];
		}
	}

	return ammountOfEnemiesAlive;
}

/* ---------------------------------------------------
 * @brief Create enemy formation with specified dimensions
 * @param enemiesRow Number of enemy rows
 * @param enemiesColumn Number of enemy columns
 * ---------------------------------------------------*/
static void game_create_enemy_map(int enemiesRow, int enemiesColumn)
{
	// Calculate formation dimensions
	float total_width = enemiesColumn * ENEMY_WIDTH + (enemiesColumn - 1) * ENEMY_H_SPACING;
	float start_x = (1.0f - total_width) / 2.0f;  // Center horizontally
	float frow;
	int row, col;

	// Initialize enemy grid
	for(row = 0; row < ENEMIES_ROW_MAX; row++)
	{
		for(col = 0; col < ENEMIES_COLUMNS_MAX; col++)
		{
			// Deactivate enemies outside current formation
			if(row >= enemiesRow || col >= enemiesColumn)
			{
				game.enemies[row][col].bullet.active = false;
				game.enemies[row][col].alive = false;
				continue;
			}

			// Calculate enemy position
			game.enemies[row][col].hitbox.start.x = start_x + col * (ENEMY_WIDTH + ENEMY_H_SPACING);
			game.enemies[row][col].hitbox.start.y = ENEMY_TOP_OFFSET + row * (ENEMY_HEIGHT + ENEMY_V_SPACING);
			game.enemies[row][col].hitbox.end.x = game.enemies[row][col].hitbox.start.x + ENEMY_WIDTH;
			game.enemies[row][col].hitbox.end.y = game.enemies[row][col].hitbox.start.y + ENEMY_HEIGHT;
			game.enemies[row][col].alive = true;

			// Determine enemy tier based on row position
			frow = (float)row / enemiesRow;
			if(frow < 0.2f)
			{
				game.enemies[row][col].type = ALIEN_TIER3;  // Top tier
			}
			else if(frow < 0.6f)
			{
				game.enemies[row][col].type = ALIEN_TIER2;  // Mid tier
			}
			else
			{
				game.enemies[row][col].type = ALIEN_TIER1;  // Bottom tier
			}

			// Initialize bullet state
			game.enemies[row][col].bullet.active = false;
		}
	}
}

/* ---------------------------------------------------
 * @brief Create barrier formations at bottom of screen
 * ---------------------------------------------------*/
static void game_create_barriers()
{
	// Calculate barrier positioning
	#define TOTAL_WIDTH (BARRIER_QUANTITY_MAX * BARRIER_WIDTH + (BARRIER_QUANTITY_MAX-1) * BARRIER_SPACING)
	#define START_X ((1.0f - TOTAL_WIDTH) / 2.0f)
	#define BASE_Y (1.0f - BARRIER_BOTTOM_OFFSET - BARRIER_HEIGHT)

	float barrier_x;
	int elem, row, col;

	// Create each barrier
	for(elem = 0; elem < BARRIER_QUANTITY_MAX; elem++)
	{
		barrier_x = START_X + elem * (BARRIER_WIDTH + BARRIER_SPACING);
		
		// Create blocks according to barrier shape
		for(row = 0; row < BARRIER_ROWS; row++) 
		{
			for(col = 0; col < BARRIER_COLUMNS; col++)
			{
				// Skip positions not in barrier shape
				if(!BARRIER_SHAPE[row][col])
				{
					game.barriers[elem].block[row][col].lives = 0;
					continue;
				}
				
				// Set block position and size
				game.barriers[elem].block[row][col].hitbox.start.x = barrier_x + col * BLOCK_WIDTH;
				game.barriers[elem].block[row][col].hitbox.start.y = BASE_Y + row * BLOCK_HEIGHT;
				game.barriers[elem].block[row][col].hitbox.end.x = barrier_x + (col+1) * BLOCK_WIDTH;
				game.barriers[elem].block[row][col].hitbox.end.y = BASE_Y + (row+1) * BLOCK_HEIGHT;
				
				// Set block durability
				#ifndef RASPBERRY
					game.barriers[elem].block[row][col].lives = 1;  // PC version
				#else
					game.barriers[elem].block[row][col].lives = BARRIER_LIVES;  // Raspberry version
				#endif
			}
		}
	}
}

/* ---------------------------------------------------
 * @brief Activate bullet from specified enemy
 * @param row Row index of shooting enemy
 * @param col Column index of shooting enemy
 * ---------------------------------------------------*/
static void shootEnemyBullet(int row, int col)
{
	// Skip if enemy already has active bullet
	if(game.enemies[row][col].bullet.active)
		return;

	// Position bullet below enemy
	game.enemies[row][col].bullet.hitbox.start.x = game.enemies[row][col].hitbox.start.x + ENEMY_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
	game.enemies[row][col].bullet.hitbox.end.x = game.enemies[row][col].hitbox.start.x + ENEMY_WIDTH / 2.0f + BULLET_WIDTH / 2.0f;
	game.enemies[row][col].bullet.hitbox.start.y = game.enemies[row][col].hitbox.end.y;
	game.enemies[row][col].bullet.hitbox.end.y = game.enemies[row][col].hitbox.end.y + BULLET_HEIGHT;
	
	// Activate bullet
	game.enemies[row][col].bullet.active = true;
}

/* ---------------------------------------------------
 * @brief Update enemy bullet positions and collisions
 * @param dt Time delta since last update (ms)
 * ---------------------------------------------------*/
static void update_enemy_bullet(float dt)
{
	int row, col;
	// Process all enemy bullets
	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			// Skip inactive bullets
			if(!game.enemies[row][col].bullet.active)
			{
				continue;
			}

			// Move bullet downward
			game.enemies[row][col].bullet.hitbox.start.y += ENEMY_BULLET_SPEED * dt;
			game.enemies[row][col].bullet.hitbox.end.y += ENEMY_BULLET_SPEED * dt;

			// Deactivate bullet if off-screen
			if(game.enemies[row][col].bullet.hitbox.end.y > 1.0f) 
			{
				game.enemies[row][col].bullet.active = false;
				continue;
			}

			// Check collision with player
			if(HITBOX_COLLISION(game.enemies[row][col].bullet.hitbox, game.player.hitbox))
			{
				playSound_play(SOUND_DEATH);
				game.player.lives--;  // Deduct life
				game.enemies[row][col].bullet.active = false;  // Deactivate bullet
			}
		}
	}
}

/* ---------------------------------------------------
 * @brief Update player bullet state and handle collisions
 * @param input Player input state
 * @param dt Time delta since last update (ms)
 * ---------------------------------------------------*/
static void update_player_bullet(input_t input, float dt)
{
	long long currentTime = getTimeMillis();
	int bulletHit = 0;
	int row, col;

	// Fire new bullet if conditions met
	if(input.shot && !game.player.bullet.active && currentTime - game.player.cooldown > PLAYER_BULLET_COOLDOWN)
	{
		// Activate bullet and position above player
		game.player.bullet.active = true;
		game.player.bullet.hitbox.start.x = game.player.hitbox.start.x + PLAYER_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
		game.player.bullet.hitbox.start.y = game.player.hitbox.start.y - BULLET_HEIGHT;
		game.player.bullet.hitbox.end.x = game.player.hitbox.start.x + PLAYER_WIDTH / 2.0f + BULLET_WIDTH / 2.0f;
		game.player.bullet.hitbox.end.y = game.player.hitbox.start.y;

		// Play sound and update counters
		playSound_play(SOUND_SHOOT);
		game.cantPlayerShots++;
		game.player.cooldown = currentTime;
	}

	// Skip if no active bullet
	if(!game.player.bullet.active)
	{
		return;
	}

	// Move bullet upward
	game.player.bullet.hitbox.start.y -= game.player.bullet.speed * dt;
	game.player.bullet.hitbox.end.y -= game.player.bullet.speed * dt;

	// Deactivate bullet if off-screen
	if(game.player.bullet.hitbox.end.y < 0.0f) 
	{
		game.player.bullet.active = false;
		return;
	}

	// Check collision with enemies
	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			// Skip dead enemies
			if(!game.enemies[row][col].alive)
			{
				continue;
			}

			// Handle enemy hit
			if(HITBOX_COLLISION(game.enemies[row][col].hitbox, game.player.bullet.hitbox))
			{
				game.enemies[row][col].alive = false;	// Kill enemy
				//game.player.bullet.active = false;	// Deactivate bullet
				bulletHit = 1;	// The bullect can collide with multiple enemies at once

				// Play explosion sounds
				playSound_play(SOUND_EXPLOSION);
				playSound_play(SOUND_INVADER_KILLED);

				// Increase game difficulty
				game.enemyShotInterval -= ENEMY_SHOOTING_INTERVAL_DECREMENT;
				game.enemiesSpeed += ENEMY_SPEED_INCREMENT_PER_ENEMY_KILLED;

				// Cap minimum shot interval
				if(game.enemyShotInterval < MIN_ENEMY_SHOOTING_INTERVAL)
				{
					game.enemyShotInterval = MIN_ENEMY_SHOOTING_INTERVAL;
				}

				// Award points based on enemy tier
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

	game.player.bullet.active &= !bulletHit;

	// Check collision with enemy bullets
	if(game.player.bullet.active && collisionEnemyBullet(&game.player.bullet.hitbox))
	{
		game.player.bullet.active = false;
	}
}

/* ---------------------------------------------------
 * @brief Advance to next level and reset game state
 * ---------------------------------------------------*/
static void game_level_up()
{
	// Increase player lives if below max
	game.player.lives += (game.player.lives < MAX_PLAYER_LIVES);
	
	// Increase enemy speed for new level
	game.enemiesSpeed = ENEMY_INITIAL_SPEED + game.level * ENEMY_SPEED_INCREMENT_PER_LEVEL;
	
	// Cap maximum enemy speed
	game.enemiesSpeed = (game.enemiesSpeed > ENEMY_MAX_SPEED) ? ENEMY_MAX_SPEED : game.enemiesSpeed;

	// Reset enemy movement direction
	game.enemiesDirection = 1;
	
	// Reset shot interval
	game.enemyShotInterval = INITIAL_SHOOTING_INTERVAL;

	// Reset counters
	game.cantPlayerShots = 0;
	game.mothership.alive = false;
	
	// Reset game entities
	playerPositionInit();
	game_create_barriers();
	game_create_enemy_map(game.enemiesRow, game.enemiesColumn);
	
	// Attempt to spawn mothership
	mothershipGenerate();
}

/* ---------------------------------------------------
 * @brief Check game over conditions
 * @return 1 if game over, 0 otherwise
 * ---------------------------------------------------*/
static int game_over(void)
{
	int row, col, gameContinues;

	// Check player lives
	if(game.player.lives <= 0)
	{
		return 1;  // Game over
	}

	// Check if enemies reached player
	for(row = game.enemiesRow - 1, gameContinues = 0; row >= 0 && !gameContinues; row--)
	{
		for(col = 0; col < game.enemiesColumn && !gameContinues; col++)
		{
			if(game.enemies[row][col].alive == true)
			{
				// Game over if enemy reaches player height
				if(game.enemies[row][col].hitbox.end.y >= game.player.hitbox.start.y)
				{
					return 1;
				}
				gameContinues = 1;  // Enemies still remain
			}
		}
	}

	return 0;  // Game continues
}