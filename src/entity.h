/* ---------------------------------------------------
 * entity.h
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
#ifndef ENTITY_H
#define ENTITY_H

#include "config.h"
#include <stdbool.h>

typedef enum 
{
    ALIEN_TIER1 = 10,
    ALIEN_TIER2 = 20,
    ALIEN_TIER3 = 30
} alien_t;

typedef struct
{
    float x;
    float y;
}coord_t;

typedef struct 
{
    float x, y;
    bool active;
    float speed;
} bullet_t;

typedef struct 
{
    float x, y;
    bool alive;
    alien_t type;
    bullet_t bullet;
} enemy_t;

typedef struct 
{
    float x, y;
    bool alive;
} barrierBlock_t;

typedef struct
{
    barrierBlock_t mat[BARRIER_ROWS][BARRIER_COLUMNS];
} barrier_t;

typedef struct 
{
    float x, y;
    bool alive;
    float speed;
} mothership_t;

typedef struct 
{
    float x, y;
    int lives;
    bool alive;
    bullet_t bullet;
} player_t;

#endif // ENTITY_H