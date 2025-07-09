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
#include <time.h>
#include "playSound.h"

static gameState_t game;
static long long getTimeMillis(void) ;

void game_init(int enemiesRow, int enemiesColumn, int barrierQuantity, int barrierRow, int barrierColumn) 
{
	
	// Initialize the player's position at the bottom center of the screen
	game.player.hitbox.start.x = 0.5f - PLAYER_WIDTH/2.0f;
	game.player.hitbox.start.y = 0.9f;
	game.player.hitbox.end.x = 0.5f + PLAYER_WIDTH/2;
	game.player.hitbox.end.y = 0.9f + PLAYER_HEIGHT;

	game.player.alive = true;
	game.player.lives = 3;
	game.player.bullet.speed = BULLET_SPEED;
	game.player.bullet.active = false;

	game_create_enemy_map(enemiesRow,enemiesColumn);

	game_create_barriers(barrierQuantity,barrierRow,barrierColumn);

	game.mothership.alive = false;
	game.score = 0;
	game.level = 0;
	game.state = RUNNING;

	game.lastTimeUpdated = getTimeMillis();
}
void game_create_enemy_map(int enemiesRow, int enemiesColumn)
{
	game.enemiesDirection = 1;
	game.enemiesSpeed = ENEMY_SPEED;
	game.enemiesRow = enemiesRow;
	game.enemiesColumn = enemiesColumn;

	float total_width  = enemiesColumn * ENEMY_WIDTH  + (enemiesColumn - 1) * ENEMY_H_SPACING;
	float total_height = enemiesRow    * ENEMY_HEIGHT + (enemiesRow    - 1) * ENEMY_V_SPACING;

	float start_x = (1.0f - total_width)  / 2.0f;
	float start_y = ENEMY_TOP_OFFSET;
	int row, col;

	for ( row = 0; row < enemiesRow; row++) 
	{
		for ( col = 0; col < enemiesColumn; col++) 
		{

			float x = start_x + col * (ENEMY_WIDTH + ENEMY_H_SPACING);
			float y = start_y + row * (ENEMY_HEIGHT + ENEMY_V_SPACING);
			printf("%f", ENEMY_V_SPACING);
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


void game_create_barriers(int barrierQuantity, int barrierRow, int barrierColumn)
{
	game.barrirersQuantity = barrierQuantity;
	game.barriersRow = barrierRow;
	game.barriersColumn = barrierColumn;

	float barrier_width  = barrierColumn * BARRIER_WIDTH_UNITS;
	float barrier_height = barrierRow * BARRIER_HEIGHT_UNITS;

	float total_width = barrierQuantity * barrier_width + (barrierQuantity-1) * BARRIER_SPACING;
	float start_x = (1.0f - total_width);
	float base_y = 1.0f - BARRIER_BOTTOM_OFFSET - barrier_height;

	for (int elem = 0; elem < barrierQuantity; elem++) {
		float barrier_x = start_x + elem * (barrier_width + BARRIER_SPACING);

		for (int row = 0; row < barrierRow; row++) {
			for (int col = 0; col < barrierColumn; col++) {
				game.barriers[elem].mat[row][col].hitbox.start.x = barrier_x + col * BARRIER_WIDTH_UNITS;
				game.barriers[elem].mat[row][col].hitbox.start.y = base_y + row * BARRIER_HEIGHT_UNITS;
				game.barriers[elem].mat[row][col].hitbox.end.x = barrier_x + col * BARRIER_WIDTH_UNITS + BARRIER_WIDTH_UNITS;
				game.barriers[elem].mat[row][col].hitbox.end.y = base_y + row * BARRIER_HEIGHT_UNITS + BARRIER_HEIGHT_UNITS;
				game.barriers[elem].mat[row][col].alive = true;
			}
		}
	}
}



int game_update(input_t player)
{
	long long dt = (getTimeMillis()-game.lastTimeUpdated);
	game.player.hitbox.start.x += player.direction * dt * PLAYER_SPEED;
	game.player.hitbox.end.x += player.direction * dt * PLAYER_SPEED;
	// printf("%f\n", (getTimeMillis()-game.lastTimeUpdated) * PLAYER_SPEED * player.direction);
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
	// printf("%d", player.direction);
	
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
		//game_level_up();
		return 0;
	}
	
	float rightmostEnemyNextX = game.enemies[0][rightLimit].hitbox.end.x + game.enemiesDirection * dt * ENEMY_SPEED;
	float leftmostEnemyNextX = game.enemies[0][leftLimit].hitbox.start.x + game.enemiesDirection * dt * ENEMY_SPEED;

	if(rightmostEnemyNextX > 1 || leftmostEnemyNextX < 0)
	{
		game.enemiesDirection = (rightmostEnemyNextX > 1)? -1: 1;
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
			game.enemies[row][col].hitbox.start.x += game.enemiesDirection * dt * ENEMY_SPEED;
			game.enemies[row][col].hitbox.end.x += game.enemiesDirection * dt * ENEMY_SPEED;
		}
	}
	update_player_bullet(player, dt);

	game.lastTimeUpdated = getTimeMillis();

	//return game_over();
	return 0;
}

void update_enemy_bullet(float dt)
{	
	int row, col;
	for(row = 0; row < game.enemiesColumn; row++)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			if(game.enemies[row][col].bullet.active == true)
			{
				
			}
		}
	}
}

void update_player_bullet(input_t input, float dt)
{
	// Si se presionó disparo y no hay bala activa
	if (input.shot && !game.player.bullet.active) {
		game.player.bullet.active = true;
		game.player.bullet.hitbox.start.x = game.player.hitbox.start.x + PLAYER_WIDTH / 2.0f - BULLET_WIDHT / 2.0f;
		game.player.bullet.hitbox.start.y = game.player.hitbox.start.y - BULLET_HEIGHT;
		game.player.bullet.hitbox.end.x = game.player.hitbox.start.x + PLAYER_WIDTH / 2.0f + BULLET_WIDHT / 2.0f;
		game.player.bullet.hitbox.end.y = game.player.hitbox.start.y;
		playSound_play(SOUND_SHOOT);
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

				switch (game.enemies[row][col].type) 
				{
					case ALIEN_TIER1: 
						game.score += 10; 
						break;
					case ALIEN_TIER2:
						game.score += 20; 
						break;
					case ALIEN_TIER3: 
						game.score += 40;
						break;
					default:
						break;
				}
			}
		}
	}

	// // Colisión con balas enemigas
	// for (int row = 0; row < game.enemiesRow; row++) {
	// 	for (int column = 0; column < game.enemiesColumn; column++) {
	// 		bullet_t* b = &game.enemies[row][column].bullet;
	// 		if (!b->active) continue;

	// 		if (game.player.bullet.x + BULLET_WIDHT >= b->x &&
	// 			game.player.bullet.x <= b->x + BULLET_WIDHT &&
	// 			game.player.bullet.y + BULLET_HEIGHT >= b->y &&
	// 			game.player.bullet.y <= b->y + BULLET_HEIGHT) {

	// 			game.player.bullet.active = false;
	// 			b->active = false;
	// 			return;
	// 		}
	// 	}
	// }

	// // Colisión con barreras
	// for (int b = 0; b < game.barrirersQuantity; b++) {
	// 	for (int r = 0; r < game.barriersRow; r++) {
	// 		for (int c = 0; c < game.barriersColumn; c++) {
	// 			if (!game.barriers[b].mat[r][c].alive) continue;

	// 			float bx = game.barriers[b].mat[r][c].x;
	// 			float by = game.barriers[b].mat[r][c].y;

	// 			if (game.player.bullet.x + BULLET_WIDHT >= bx &&
	// 				game.player.bullet.x <= bx + BARRIER_WIDTH_UNITS &&
	// 				game.player.bullet.y + BULLET_HEIGHT >= by &&
	// 				game.player.bullet.y <= by + BARRIER_HEIGHT_UNITS) {

	// 				game.barriers[b].mat[r][c].alive = false;
	// 				game.player.bullet.active = false;
	// 				return;
	// 			}
	// 		}
	// 	}
	// }
}


int game_over(void)
{
	int row, col;
	for(row = game.enemiesRow-1; row >= 0; row--)
	{
		for(col = 0; col < game.enemiesColumn; col++)
		{
			if(game.enemies[row][col].alive == true)
			{
				if(game.enemies[row][col].hitbox.end.y <= game.player.hitbox.start.y)
				{
					return 1;
				}
				return 0;
			}
		}
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
	hitbox_t hitbox = game.enemies[row][column].hitbox;
	return hitbox;
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

// hitbox_t getBarrierPosition(int barrier, int row, int column)
// {
// 	hitbox_t position = {0.0f, 0.0f};
// 	 if (barrier < 0 || barrier >= BARRIER_QUANTITY_MAX ||
// 		row < 0 || row >= BARRIER_ROWS_MAX ||
// 		column < 0 || column >= BARRIER_COLUMNS_MAX)
// 		return position;

// 	if (!game.barriers[barrier].mat[row][column].alive)
// 		return position;
// 	position.x = game.barriers[barrier].mat[row][column].x;
// 	position.y = game.barriers[barrier].mat[row][column].y;
// 	return position;
// }

int getEnemyTier(int row)
{
	return game.enemies[row][0].type;
}

static long long getTimeMillis(void)
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