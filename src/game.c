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
const bool BARRIER_SHAPE[BARRIER_ROWS][BARRIER_COLUMNS] = {
    {0,1,1,1,0},
    {1,1,1,1,1},
    {1,0,0,0,1}
};
#else
const bool BARRIER_SHAPE[BARRIER_ROWS][BARRIER_COLUMNS] = {
    {1,1,1}
};
#endif

static game_t game;
static void enemiesBulletActive(void);
static void updateBarrier(void);
static bool collisionEnemyBullet(hitbox_t *hitbox);
static bool collisionEnemyHitbox(hitbox_t *hitbox);
static void mothershipGenerate(void);
static void mothershipUpdate(float dt);
static void saveGameState(void);
static void loadGameState(void);

void game_init(int enemiesRow, int enemiesColumn, bool resumeLastGame) 
{
    if(resumeLastGame)
    {
        loadGameState();
        long long currentTime = getTimeMillis();
        game.lastTimeUpdated = currentTime;
        game.lastTimeEnemyShoot = currentTime;
        return;
    }

	// Initialize the player's position at the bottom center of the screen
    hitbox_t playerInitialHitbox = {
        {0.5f - PLAYER_WIDTH/2.0f, PLAYER_BOTTOM_OFFSET},
        {0.5f + PLAYER_WIDTH/2.0f, PLAYER_BOTTOM_OFFSET + PLAYER_HEIGHT}
    };

    game.player.hitbox=playerInitialHitbox;

	game.player.lives = 3;
	game.player.bullet.speed = PLAYER_BULLET_SPEED;
	game.player.bullet.active = false;
    game.cantPlayerShots = 0;

    game.enemiesDirection = 1;
	game.enemiesSpeed = ENEMY_SPEED;
	game.enemiesRow = enemiesRow;
	game.enemiesColumn = enemiesColumn;

    game.enemyShotInterval = INITIAL_SHOOTING_INTERVAL;
    game.lastTimeEnemyShoot = getTimeMillis();
    srand(time(NULL));

    game.enemiesHands = false;
	game_create_enemy_map(enemiesRow,enemiesColumn);

	game_create_barriers();

	game.mothership.alive = false;
	game.score = 0;
	game.level = 0;
	game.state = RUNNING;

	game.lastTimeUpdated = getTimeMillis();
}

void game_create_enemy_map(int enemiesRow, int enemiesColumn)
{
	float total_width  = enemiesColumn * ENEMY_WIDTH  + (enemiesColumn - 1) * ENEMY_H_SPACING;

	float start_x = (1.0f - total_width)  / 2.0f;
	float start_y = ENEMY_TOP_OFFSET;
	int row, col;

	for (row = 0; row < ENEMIES_ROW_MAX; row++) 
	{
		for (col = 0; col < ENEMIES_COLUMNS_MAX; col++) 
		{
            if(row >= enemiesRow || col >= enemiesColumn)
            {
                game.enemies[row][col].bullet.active = false;
                game.enemies[row][col].alive = false;
                continue;
            }
            
			float x = start_x + col * (ENEMY_WIDTH + ENEMY_H_SPACING);
			float y = start_y + row * (ENEMY_HEIGHT + ENEMY_V_SPACING);
			game.enemies[row][col].hitbox.start.x = x;
			game.enemies[row][col].hitbox.start.y = y;
			game.enemies[row][col].hitbox.end.x = x + ENEMY_WIDTH;
			game.enemies[row][col].hitbox.end.y = y + ENEMY_HEIGHT;
			game.enemies[row][col].alive = true;

			// Tipo de enemigo según la proporción de la fila
			float frow = (float)row / enemiesRow;
			if (frow < 0.2f)
			{
				game.enemies[row][col].type = ALIEN_TIER3;
			}
			else if (frow < 0.6f)
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


void game_create_barriers()
{
	float total_width = BARRIER_QUANTITY_MAX * BARRIER_WIDTH + (BARRIER_QUANTITY_MAX-1) * BARRIER_SPACING;
	float start_x = (1.0f - total_width) / 2;
	float base_y = 1.0f - BARRIER_BOTTOM_OFFSET - BARRIER_HEIGHT;

    int elem, row, col;
	for (elem = 0; elem < BARRIER_QUANTITY_MAX; elem++) 
    {
		float barrier_x = start_x + elem * (BARRIER_WIDTH + BARRIER_SPACING);

		for (row = 0; row < BARRIER_ROWS; row++) 
        {
			for (col = 0; col < BARRIER_COLUMNS; col++) 
            {
                if(!BARRIER_SHAPE[row][col])
                {
                    game.barriers[elem].mat[row][col].lives = 0;
                    continue;
                }

				game.barriers[elem].mat[row][col].hitbox.start.x = barrier_x + col * (BARRIER_WIDTH / BARRIER_COLUMNS);
				game.barriers[elem].mat[row][col].hitbox.start.y = base_y + row * (BARRIER_HEIGHT / BARRIER_ROWS);
				game.barriers[elem].mat[row][col].hitbox.end.x = barrier_x + (col+1) * (BARRIER_WIDTH / BARRIER_COLUMNS);
				game.barriers[elem].mat[row][col].hitbox.end.y = base_y + (row+1) * (BARRIER_HEIGHT / BARRIER_ROWS);
                #ifndef RASPBERRY
				game.barriers[elem].mat[row][col].lives = 1;
                #else
                game.barriers[elem].mat[row][col].lives = BARRIER_LIVES;
                #endif
			}
		}
	}
}

int game_update(input_t player)
{
    if(player.pause)
    {
        game.lastTimeUpdated = getTimeMillis();
        //playSound_stop(SOUND_UFO_LOW);
        saveGameState();
        if(player.exit)
        {
            game.state = QUIT;
            return QUIT;
        }
        return RUNNING;
    }

	long long dt = (getTimeMillis()-game.lastTimeUpdated);
	game.player.hitbox.start.x += player.direction * dt * PLAYER_SPEED;
	game.player.hitbox.end.x += player.direction * dt * PLAYER_SPEED;

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
	//printf("estoy en game_update");
	
	int row, col;
	int rightLimit = -1, leftLimit = game.enemiesColumn, bottomLimit = -1;

	bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];

	getEnemyBitMap(matEnemy);

	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			if(matEnemy[row][col])
			{
				if(col > rightLimit)
				{
					rightLimit = col;
				}
				if(leftLimit > col)
				{
					leftLimit = col;
				}
				if(row > bottomLimit)
				{
					bottomLimit = row;
				}
			}
		}
	}

	// Recorrio todo el for y no hay enemigos vivos
	if(rightLimit == -1 && \
	   leftLimit == game.enemiesColumn && \
	   bottomLimit == -1)
	{
        static long long lastTimeLevelUp;
        static bool firstTimeLevelUp = true;
        // Reset player position and hitbox
        hitbox_t playerInitialHitbox = {
            {0.5f - PLAYER_WIDTH/2.0f, PLAYER_BOTTOM_OFFSET},
            {0.5f + PLAYER_WIDTH/2.0f, PLAYER_BOTTOM_OFFSET + PLAYER_HEIGHT}
        };

        game.player.hitbox=playerInitialHitbox;
        game.mothership.alive = false;

        for(row = 0; row < game.enemiesRow; row++)
        {
            for(col = 0; col < game.enemiesColumn; col++)
            {
                game.enemies[row][col].bullet.active = false;
            }
        }

        if(firstTimeLevelUp)
        {
            lastTimeLevelUp = getTimeMillis();
            firstTimeLevelUp = false;
        }

        long long currentTime = getTimeMillis();
        if(currentTime - lastTimeLevelUp > 3000)
        {
            game_level_up();
            lastTimeLevelUp = currentTime;
            firstTimeLevelUp = true;
            game.lastTimeUpdated = currentTime;
            return RUNNING;
        }
        return RUNNING; // No enemies left, game continues
	}
	
	float rightmostEnemyNextX = game.enemies[0][rightLimit].hitbox.end.x + game.enemiesDirection * dt *  game.enemiesSpeed;
	float leftmostEnemyNextX = game.enemies[0][leftLimit].hitbox.start.x + game.enemiesDirection * dt *  game.enemiesSpeed;

	if(rightmostEnemyNextX > 1 || leftmostEnemyNextX < 0)
	{
		game.enemiesDirection = (rightmostEnemyNextX > 1)? -1: 1;
        game.enemiesSpeed += ENEMY_SPEED_INCREMENT;
        if (ENEMY_SPEED + game.level * ENEMY_SPEED_INCREMENT > ENEMY_MAX_SPEED)
        {
            game.enemiesSpeed = ENEMY_MAX_SPEED;
        }
        else
        {
            game.enemiesSpeed = ENEMY_SPEED + game.level * ENEMY_SPEED_INCREMENT;
        }
		for(row = 0; row < game.enemiesRow; row++)
		{
			for(col = 0; col < game.enemiesColumn; col++)
			{
				game.enemies[row][col].hitbox.start.y += ENEMY_DESCENT_STEP;
				game.enemies[row][col].hitbox.end.y += ENEMY_DESCENT_STEP;
			}
		}
	}

	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			game.enemies[row][col].hitbox.start.x += game.enemiesDirection * dt *  game.enemiesSpeed;
			game.enemies[row][col].hitbox.end.x += game.enemiesDirection * dt *  game.enemiesSpeed;
            
		}
	}
    static float enemiesMovement = 0;
    enemiesMovement += dt * game.enemiesSpeed;
    if(enemiesMovement > 0.1f)
    {
        playSound_play((game.enemiesHands) ? SOUND_FAST1 : SOUND_FAST2);
        printf("Enemies hands: %d\n", game.enemiesHands);
        enemiesMovement = 0;
        game.enemiesHands = !game.enemiesHands;
    }

    enemiesBulletActive();
	update_player_bullet(player, dt);
    update_enemy_bullet(dt);
    mothershipUpdate(dt);
    updateBarrier();

	game.lastTimeUpdated = getTimeMillis();
    game.state = game_over();
    if(game.state == GAME_OVER)
    {
        saveGameState();
    }
	return game.state;
}

void game_level_up()
{
    game.level++;
    if(game.player.lives < MAX_PLAYER_LIVES)
    {
        game.player.lives++;
    }
    if (ENEMY_SPEED + game.level * ENEMY_SPEED_INCREMENT > ENEMY_MAX_SPEED)
    {
        game.enemiesSpeed = ENEMY_MAX_SPEED;
    }
    else
    {
        game.enemiesSpeed = ENEMY_SPEED + game.level * ENEMY_SPEED_INCREMENT;
    }

    game.enemiesDirection = 1;
    game.enemyShotInterval  = INITIAL_SHOOTING_INTERVAL;
    
    game.cantPlayerShots = 0;
    game_create_enemy_map(game.enemiesRow, game.enemiesColumn);
    game_create_barriers();
    mothershipGenerate();
}

void getEnemiesBulletsInfo(bullet_t matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX])
{
    int row, col;
	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
            matEnemy[row][col] = game.enemies[row][col].bullet;
        }
    }
}

void enemyBulletShot(int row, int col)
{
    if(!game.enemies[row][col].bullet.active)
    {
        game.enemies[row][col].bullet.hitbox.start.x = game.enemies[row][col].hitbox.start.x + ENEMY_WIDTH / 2.0f - BULLET_WIDTH / 2.0f;
        game.enemies[row][col].bullet.hitbox.end.x = game.enemies[row][col].hitbox.start.x + ENEMY_WIDTH / 2.0f + BULLET_WIDTH / 2.0f;
        game.enemies[row][col].bullet.hitbox.start.y = game.enemies[row][col].hitbox.end.y;
        game.enemies[row][col].bullet.hitbox.end.y = game.enemies[row][col].hitbox.end.y + BULLET_HEIGHT;
        game.enemies[row][col].bullet.active = true;
    }
}

void update_enemy_bullet(float dt)
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

            if (game.enemies[row][col].bullet.hitbox.end.y > 1.0f) 
            {
                game.enemies[row][col].bullet.active = false;
                continue;
            }

            if (HITBOX_COLLISION(game.enemies[row][col].bullet.hitbox, game.player.hitbox))
            {
                game.player.lives--;
                game.enemies[row][col].bullet.active = false;
            }
		}
	}
}

void update_player_bullet(input_t input, float dt)
{
	// Si se presionó disparo y no hay bala activa
	if (input.shot && !game.player.bullet.active) {
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
	

	if (game.player.bullet.hitbox.end.y < 0.0f) 
	{
		game.player.bullet.active = false;
		return;
	}

	int row, col;
	// Colisión con aliens
	for (row = 0; row < game.enemiesRow; row++) 
	{
		for (col = 0; col < game.enemiesColumn; col++) 
		{
			if (!game.enemies[row][col].alive) 
			{
				continue;
			}

			if (HITBOX_COLLISION(game.enemies[row][col].hitbox, game.player.bullet.hitbox))
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

hitbox_t getMothershipPosition(void)
{
    return game.mothership.hitbox;
}

bool getIsMothershipAlive(void)
{
    return game.mothership.alive;
}

static void saveGameState(void)
{
    FILE *file = fopen("data/savegame.dat", "wb");
    if (!file) {
        perror("Failed to open save file");
        return;
    }

    fwrite(&game, sizeof(game), 1, file);
    fclose(file);
}

static void loadGameState(void)
{
    FILE *file = fopen("data/savegame.dat", "rb");
    if (!file) {
        perror("Failed to open save file");
        return;
    }

    fread(&game, sizeof(game), 1, file);
    fclose(file);
}

static void mothershipGenerate(void)
{
    if(game.cantPlayerShots >= MOTHERSHIP_TRIGGER_SHOTS && !game.mothership.alive)
    {
        game.mothership.alive = true;
        game.mothership.speed = (rand() % 2) ? MOTHERSHIP_SPEED : -MOTHERSHIP_SPEED;
        if (game.mothership.speed < 0)
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
       // playSound_restart(SOUND_UFO_LOW);
    }
    else if(game.mothership.alive)
    {
        game.cantPlayerShots = 0;
    }
}

static void mothershipUpdate(float dt)
{
    mothershipGenerate();

    if(!game.mothership.alive)
    {
        return;
    }

    long long currentTime = getTimeMillis();
    if(currentTime - game.lastTimeMothershipGenerated > 860)
    {
        //playSound_restart(SOUND_UFO_LOW);
        game.lastTimeMothershipGenerated = currentTime;
    }
    
    game.mothership.hitbox.start.x += game.mothership.speed * dt;
    game.mothership.hitbox.end.x += game.mothership.speed * dt;

    if((game.mothership.hitbox.end.x < 0 && game.mothership.speed < 0) ||
       (game.mothership.hitbox.start.x > 1.0f && game.mothership.speed > 0))
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

static void updateBarrier(void)
{
    int elem, row, col;
	for (elem = 0; elem < BARRIER_QUANTITY_MAX; elem++) 
    {
		for (row = 0; row < BARRIER_ROWS; row++) 
        {
			for (col = 0; col < BARRIER_COLUMNS; col++) 
            {
				if (!game.barriers[elem].mat[row][col].lives)
                {
                    continue;
                }

				if (HITBOX_COLLISION(game.player.bullet.hitbox, game.barriers[elem].mat[row][col].hitbox))
                {
                    game.barriers[elem].mat[row][col].lives--;
					game.player.bullet.active = false;
					return;
                } 

                else if(collisionEnemyBullet(&game.barriers[elem].mat[row][col].hitbox))
                {
                    game.barriers[elem].mat[row][col].lives--;
                    return;
                }

                else if(collisionEnemyHitbox(&game.barriers[elem].mat[row][col].hitbox))
                {
                    game.barriers[elem].mat[row][col].lives = 0;
                    return;
                }
			}
		}
	}
}

static bool collisionEnemyBullet(hitbox_t *hitbox)
{
    int row, col;
    for (row = 0; row < game.enemiesRow; row++) 
    {
		for (col = 0; col < game.enemiesColumn; col++) 
        {
			if (HITBOX_COLLISION(*hitbox, game.enemies[row][col].bullet.hitbox) && game.enemies[row][col].bullet.active) 
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
    for (row = 0; row < game.enemiesRow; row++) 
    {
		for (col = 0; col < game.enemiesColumn; col++) 
        {
			if (HITBOX_COLLISION(*hitbox, game.enemies[row][col].hitbox) && game.enemies[row][col].alive) 
            {
				return 1;
			}
		}
	}
    return 0;
}

int game_over(void)
{
	int row, col, flag;
	for(row = game.enemiesRow-1, flag = 0; row >= 0 && !flag; row--)
	{
		for(col = 0; col < game.enemiesColumn && !flag; col++)
		{
			if(game.enemies[row][col].alive == true)
			{
				if(game.enemies[row][col].hitbox.end.y >= game.player.hitbox.start.y)
				{
					return 1;   // Game Over
				}
                flag = 1;
			}
		}
	}

    if(game.player.lives == 0)
    {
        return 1;
    }

	return 0;
}

/**
 * @brief Returns the position of the player.
 *
 * @return coord_t Struct with player's position
 */
hitbox_t getPlayerPosition(void)
{
	hitbox_t hitbox = game.player.hitbox;
	return hitbox;
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
    return game.barriers[barrier].mat[row][column].lives > 0;
}

hitbox_t getBarrierPosition(int barrier, int row, int column)
{
	return game.barriers[barrier].mat[row][column].hitbox;
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

void getEnemyBitMap(bool matEnemy[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX])
{
	int row, col;

	for(row = 0; row < game.enemiesRow; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			matEnemy[row][col] = game.enemies[row][col].alive;
		}
	}
}

bool getEnemiesHands(void)
{
    return game.enemiesHands;
}

static void enemiesBulletActive(void)
{
    long long time = getTimeMillis();
    long long dt = time - game.lastTimeEnemyShoot;
    if(dt > game.enemyShotInterval)
    {
        int col = rand() % game.enemiesColumn;
        int row;
        for(row = game.enemiesRow-1; row >= 0; row--)
        {
            if(game.enemies[row][col].alive)
            {
                if(game.enemies[row][col].bullet.active)
                {
                    break;
                }
                enemyBulletShot(row, col);
                game.lastTimeEnemyShoot = time;
                return;
            }
        }
    }
}

