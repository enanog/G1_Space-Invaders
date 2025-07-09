/* ---------------------------------------------------
 * config.h
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

#ifndef CONFIG_H
#define CONFIG_H

#ifndef RASBERRY
#define OFFSET 50
#define SCREEN_W 1900
#define SCREEN_H 1000
#else
#define OFFSET 1
#define SCREEN_W 16
#define SCREEN_H 16
#endif

#define ENEMIES_ROW_MAX 10
#define ENEMIES_COLUMNS_MAX 20
#define BARRIER_QUANTITY_MAX 4
#define BARRIER_ROWS_MAX 3
#define BARRIER_COLUMNS_MAX 9
#define PLAYER_LIVES 3

#define PLAYER_WIDTH	0.05f
#define PLAYER_HEIGHT	0.01f
#define PLAYER_SPEED	0.0002f


#define ENEMY_WIDTH         (0.5f / ENEMIES_COLUMNS_MAX)
#define ENEMY_HEIGHT        (0.2f / ENEMIES_ROW_MAX)
#define ENEMY_H_SPACING     (ENEMY_WIDTH *  2.0f)
#define ENEMY_V_SPACING     (ENEMY_HEIGHT * 2.0f)
#define ENEMY_TOP_OFFSET    0.05f
#define ENEMY_SPEED         0.00005f
#define ENEMY_DESCENT_STEP  (1.0/SCREEN_H)*OFFSET


#define BARRIER_WIDTH_UNITS   (1.0f / 20.0f)
#define BARRIER_HEIGHT_UNITS  (1.0f / 30.0f)
#define BARRIER_SPACING       (BARRIER_WIDTH_UNITS * 3.0f)
#define BARRIER_BOTTOM_OFFSET 0.1f


#define BULLET_SPEED 	0.0005f
#define BULLET_WIDHT 	0.01f
#define BULLET_HEIGHT	0.01f

#endif // CONFIG_H
