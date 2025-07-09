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
    
    game.player.x = 0.5f;
    game.player.y = 0.9f;
    game.player.alive = true;
    game.player.lives = 3;

    game.enemiesDirection = 1;
    game.enemiesSpeed = ENEMY_SPEED;
    game.enemiesRow = enemiesRow;
    game.enemiesColumn = enemiesColumn;
    int row, column;
    for(row = 0; row < enemiesRow; row++)
    {
        for(column = 0; column < enemiesColumn; column++)
        {
            game.enemies[row][column].x =  0.8f * column / (enemiesColumn - 1) + 0.1f ;
            game.enemies[row][column].y =  0.5f * row / (enemiesRow-1) + 0.1f;
            game.enemies[row][column].alive = true;
            game.enemies[row][column].type = (row < enemiesRow/2)? ALIEN_TIER1: ((row < enemiesRow*4/5)? ALIEN_TIER2: ALIEN_TIER3); // Magic numbers
            game.enemies[row][column].bullet.active = false;
        }
    }

    game.barrirersQuantity = barrierQuantity;
    game.barriersRow = barrierRow;
    game.barriersColumn = barrierColumn;
    int cant;
    for(cant = 0; cant < barrierQuantity; cant++)
    {
        for(row = 0; row < barrierRow; row++)
        {
            for(column = 0; column < barrierColumn; column++)
            {
                game.barriers[cant].mat[row][column].alive = true;
                game.barriers[cant].mat[row][column].x = 1;
                game.barriers[cant].mat[row][column].y = 1;
            }
        }
    }

    game.mothership.alive = false;
    game.score = 0;
    game.level = 0;
    game.state = RUNNING;

    game.lastTimeUpdated = getTimeMillis();
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

    game.lastTimeUpdated = getTimeMillis();

    return game_over();
}

int game_over(void)
{
    int row, column;
    for(row = game.enemiesRow-1; row > 0; row--)
    {
        for(column = 0; column < game.enemiesColumn; column++)
        {
            if(game.enemies[row][column].alive == true)
            {
                if(game.enemies[row][column].y > game.player.y - ENEMY_DESCENT_STEP/2)
                {
                    return 1;
                }
                return 0;
            }
        }
    }
    return 0;
}

coord_t getPlayerPosition(void)
{
    coord_t position = {game.player.x, game.player.y};
    return position;
}

coord_t getEnemyPosition(int row, int column)
{
    coord_t position = {game.enemies[row][column].x , game.enemies[row][column].y};
    return position;
}

coord_t getBarrierPosition(int barrier, int row, int column)
{

}

static long long getTimeMillis(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;
}