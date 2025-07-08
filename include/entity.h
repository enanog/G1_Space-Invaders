/* ---------------------------------------------------
 * score.h
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

typedef enum {
    ALIEN_TIER1 = 10,
    ALIEN_TIER2 = 20,
    ALIEN_TIER3 = 30
} alien_t;

typedef struct {
    float x, y;
    bool active;
} bullet_t;

typedef struct {
    float x, y;
    bool alive;
    alien_t type;
} enemy_t;

typedef struct {
    float x, y;
    bool alive;
    bool active;
} barrierBlock_t;

typedef struct {
    float x, y;
    bool alive;
    int direction;
} mothership_t;

typedef struct {
    float x, y;
    int lives;
    bool alive;
} player_t;

#endif // ENTITY_H