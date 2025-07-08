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

static gameState_t game;

void game_init(int enemiesRow, int enemiesColumn, int barrierQuantity, int barrierRow, int barrierColumn) 
{
    game.player.x = 0.5f;
    game.player.y = 0.9f;
    game.player.alive = true;
    game.player.lives = 3;

    int row, column;
    for(row = 0; row < enemiesRow; row++)
    {
        for(column = 0; column < enemiesColumn; column++)
        {
            game.enemies[row][column].x =  0.8f * column / (enemiesColumn - 1) + 0.1f ;
            game.enemies[row][column].y =  0.5f * row / (enemiesRow-1) + 0.05f;
            game.enemies[row][column].alive = true;
            game.enemies[row][column].type = (row < enemiesRow/2)? ALIEN_TIER1: ((row < enemiesRow*4/5)? ALIEN_TIER2: ALIEN_TIER3); // Magic numbers
            game.enemies[row][column].bullet.active = false;
        }
    }

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