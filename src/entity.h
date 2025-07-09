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
	coord_t start;  //  Upper-left corner of the hitbox
	coord_t end;    //  Lower-right corner of the hitbox
} hitbox_t;

typedef struct 
{
	hitbox_t hitbox;
	bool active;
	float speed;
} bullet_t;

typedef struct 
{
	hitbox_t hitbox;
	bool alive;
	alien_t type;
	bullet_t bullet;
} enemy_t;

typedef struct 
{
	hitbox_t hitbox;
	bool alive;
} barrierBlock_t;

typedef struct
{
	barrierBlock_t mat[BARRIER_ROWS_MAX][BARRIER_COLUMNS_MAX];
} barrier_t;

typedef struct 
{
	hitbox_t hitbox;
	bool alive;
	float speed;
} mothership_t;

typedef struct 
{
	hitbox_t hitbox;
	int lives;
	bool alive;
	bullet_t bullet;
} player_t;

#define HITBOX_COLLISION(hb1, hb2) (!((hb1).start.x > (hb2).end.x || (hb1).end.x < (hb2).start.x || (hb1).end.y < (hb2).start.y || (hb1).start.y > (hb2).end.y))

void hitboxMove(hitbox_t *hitbox, hitbox_t movement);

#endif // ENTITY_H