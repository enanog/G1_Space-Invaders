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

static gameState_t game;
static long long getTimeMillis(void) ;

void game_init(int enemiesRow, int enemiesColumn, int barrierQuantity, int barrierRow, int barrierColumn) 
{
    
    // Initialize the player's position at the bottom center of the screen
    game.player.x = 0.5f;
    game.player.y = 0.9f;
    game.player.alive = true;
    game.player.lives = 3;

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

    for ( row= 0; row < enemiesRow; row++) {
        for ( col = 0; col < enemiesColumn; col++) {

            float x = start_x + col * (ENEMY_WIDTH + ENEMY_H_SPACING);
            float y = start_y + row * (ENEMY_HEIGHT + ENEMY_V_SPACING);

            game.enemies[row][col].x = x;
            game.enemies[row][col].y = y;
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
                game.barriers[elem].mat[row][col].x = barrier_x + col * BARRIER_WIDTH_UNITS;
                game.barriers[elem].mat[row][col].y = base_y + row * BARRIER_HEIGHT_UNITS;
                game.barriers[elem].mat[row][col].alive = true;
            }
        }
    }
}



int game_update(input_t player)
{
    long long dt = (getTimeMillis()-game.lastTimeUpdated);
    game.player.x += player.direction * dt * PLAYER_SPEED;
    //printf("%f\n", (getTimeMillis()-game.lastTimeUpdated) * PLAYER_SPEED * player.direction);
    game.player.x = (game.player.x < 0)? 0: ((game.player.x > 1)? 1: game.player.x);

    int row, column;
    float rightmostEnemyNextX = game.enemies[0][game.enemiesColumn-1].x + game.enemiesDirection * dt * ENEMY_SPEED;
    float leftmostEnemyNextX = game.enemies[0][0].x + game.enemiesDirection * dt * ENEMY_SPEED;

    if(rightmostEnemyNextX > 1 || leftmostEnemyNextX < 0)
    {
        game.enemiesDirection = (rightmostEnemyNextX > 1)? -1: 1;
        for(row = 0; row < game.enemiesRow; row++)
        {
            for(column = 0; column < game.enemiesColumn; column++)
            {
                game.enemies[row][column].y += ENEMY_DESCENT_STEP;
            }
        }
    }

    for(row = 0; row < game.enemiesRow; row++)
    {
        for(column = 0; column < game.enemiesColumn; column++)
        {
            game.enemies[row][column].x += game.enemiesDirection * dt * ENEMY_SPEED;
        }
    }
    update_player_bullet(player, dt);

    game.lastTimeUpdated = getTimeMillis();

    return game_over();
}

static bool playerShotLock = false;

void update_player_bullet(input_t input, float dt)
{
    // Si se presionó disparo y no hay bala activa
    if (input.shot && !playerShotLock && !game.player.bullet.active) {
        game.player.bullet.active = true;
        game.player.bullet.x = game.player.x + PLAYER_WIDTH / 2.0f - BULLET_WIDHT / 2.0f;
        game.player.bullet.y = game.player.y;
        playerShotLock = true;
    }
    else if (!input.shot)
    {
        playerShotLock = false;
    }
     if (!game.player.bullet.active)
        return;

    // Movimiento
    game.player.bullet.y -= BULLET_SPEED * dt;

    if (game.player.bullet.y < 0.0f) {
        game.player.bullet.active = false;
        return;
    }

    // Colisión con aliens
    for (int row = 0; row < game.enemiesRow; row++) {
        for (int column = 0; column < game.enemiesColumn; column++) {
            if (!game.enemies[row][column].alive) continue;

            float ex = game.enemies[row][column].x;
            float ey = game.enemies[row][column].y;

            if (game.player.bullet.x + BULLET_WIDHT  >= ex &&
                game.player.bullet.x <= ex + ENEMY_WIDTH &&
                game.player.bullet.y + BULLET_HEIGHT >= ey &&
                game.player.bullet.y <= ey + ENEMY_HEIGHT) {

                game.enemies[row][column].alive = false;
                game.player.bullet.active = false;

                switch (game.enemies[row][column].type) {
                    case ALIEN_TIER1: game.score += 10; break;
                    case ALIEN_TIER2: game.score += 20; break;
                    case ALIEN_TIER3: game.score += 40; break;
                }
                return;
            }
        }
    }

    // Colisión con balas enemigas
    for (int row = 0; row < game.enemiesRow; row++) {
        for (int column = 0; column < game.enemiesColumn; column++) {
            bullet_t* b = &game.enemies[row][column].bullet;
            if (!b->active) continue;

            if (game.player.bullet.x + BULLET_WIDHT >= b->x &&
                game.player.bullet.x <= b->x + BULLET_WIDHT &&
                game.player.bullet.y + BULLET_HEIGHT >= b->y &&
                game.player.bullet.y <= b->y + BULLET_HEIGHT) {

                game.player.bullet.active = false;
                b->active = false;
                return;
            }
        }
    }

    // Colisión con barreras
    for (int b = 0; b < game.barrirersQuantity; b++) {
        for (int r = 0; r < game.barriersRow; r++) {
            for (int c = 0; c < game.barriersColumn; c++) {
                if (!game.barriers[b].mat[r][c].alive) continue;

                float bx = game.barriers[b].mat[r][c].x;
                float by = game.barriers[b].mat[r][c].y;

                if (game.player.bullet.x + BULLET_WIDHT >= bx &&
                    game.player.bullet.x <= bx + BARRIER_WIDTH_UNITS &&
                    game.player.bullet.y + BULLET_HEIGHT >= by &&
                    game.player.bullet.y <= by + BARRIER_HEIGHT_UNITS) {

                    game.barriers[b].mat[r][c].alive = false;
                    game.player.bullet.active = false;
                    return;
                }
            }
        }
    }
}


int game_over(void)
{
    int row, column;
    for(row = game.enemiesRow-1; row >= 0; row--)
    {
        for(column = 0; column < game.enemiesColumn; column++)
        {
            if(game.enemies[row][column].alive == true)
            {
                if(game.enemies[row][column].y + ENEMY_H_SPACING > game.player.y)
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
coord_t getPlayerPosition(void)
{
    coord_t position = {game.player.x, game.player.y};
    return position;
}

/**
 * @brief Returns the position of a specific barrier cell.
 *
 * @param barrier Index of the barrier
 * @param row Row within the barrier
 * @param column Column within the barrier
 * @return coord_t Struct with the barrier cell's position
 */
coord_t getEnemyPosition(int row, int column)
{
    coord_t position;
    position.x = game.enemies[row][column].x;
    position.y = game.enemies[row][column].y;
    return position;
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
coord_t getProjectilePosition(void)
{
    coord_t position = {game.player.bullet.x, game.player.bullet.y};
    return position;
}

coord_t getBarrierPosition(int barrier, int row, int column)
{
    coord_t position = {0.0f, 0.0f};
     if (barrier < 0 || barrier >= BARRIER_QUANTITY_MAX ||
        row < 0 || row >= BARRIER_ROWS_MAX ||
        column < 0 || column >= BARRIER_COLUMNS_MAX)
        return position;

    if (!game.barriers[barrier].mat[row][column].alive)
        return position;
    position.x = game.barriers[barrier].mat[row][column].x;
    position.y = game.barriers[barrier].mat[row][column].y;
    return position;
}

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