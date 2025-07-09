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

#define ENEMIES_ROW_MAX 5
#define ENEMIES_COLUMNS_MAX 11
#define BARRIER_QUANTITY_MAX 4
#define BARRIER_ROWS_MAX 3
#define BARRIER_COLUMNS_MAX 9
#define PLAYER_LIVES 3
#define SCREEN_W 1900
#define SCREEN_H 1000
#define PLAYER_SPEED 0.0002f
#define ENEMY_SPEED 0.00005f

#define ENEMY_WIDTH      (1.0f / 16.0f)
#define ENEMY_HEIGHT     (1.0f / 16.0f)
#define ENEMY_H_SPACING  (ENEMY_WIDTH / 2.0f)
#define ENEMY_V_SPACING  (ENEMY_HEIGHT / 2.0f)
#define ENEMY_TOP_OFFSET 0.05f


#define BARRIER_WIDTH_UNITS   (1.0f / 20.0f)
#define BARRIER_HEIGHT_UNITS  (1.0f / 30.0f)
#define BARRIER_SPACING       (BARRIER_WIDTH_UNITS * 3.0f)
#define BARRIER_BOTTOM_OFFSET 0.1f


#ifndef RASBERRY
#define OFFSET 50
#else
#define OFFSET 1
#endif

#define ENEMY_DESCENT_STEP (1.0/SCREEN_H)*OFFSET
#define BULLET_SPEED 0.0025f

#endif // CONFIG_H
