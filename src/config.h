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
#define OFFSET 			50
#define SCREEN_W 		1920
#define SCREEN_H        1080
#define BARRIER_ROWS	3
#define BARRIER_COLUMNS 5
#define PLAYER_LIVES    3
#else
#define OFFSET 1
#define SCREEN_W 16
#define SCREEN_H 16
#define BARRIER_ROWS            1
#define BARRIER_COLUMNS         2
#define PLAYER_LIVES            3
#endif

#define ENEMIES_ROW_MAX         10
#define ENEMIES_COLUMNS_MAX     20

#define PLAYER_WIDTH	0.05f
#define PLAYER_HEIGHT	0.02f
#define PLAYER_SPEED	0.0002f
#define PLAYER_BOTTOM_OFFSET 0.95f

#define ENEMY_WIDTH         (0.6f / ENEMIES_COLUMNS_MAX)
#define ENEMY_HEIGHT        (0.3f / ENEMIES_ROW_MAX)
#define ENEMY_H_SPACING     (ENEMY_WIDTH *  2.0f)
#define ENEMY_V_SPACING     (ENEMY_HEIGHT * 2.0f)
#define ENEMY_TOP_OFFSET    0.2f
#define ENEMY_SPEED         0.00005f
#define ENEMY_DESCENT_STEP  (1.0/SCREEN_H)*OFFSET
#define INITIAL_SHOOTING_INTERVAL 2000

#define BARRIER_QUANTITY_MAX    4
#define BARRIER_WIDTH           (0.3f / BARRIER_QUANTITY_MAX)
#define BARRIER_HEIGHT          (0.3f / BARRIER_QUANTITY_MAX)
#define BARRIER_SPACING         (BARRIER_WIDTH * 2.0f)
#define BARRIER_BOTTOM_OFFSET   0.15f

#define ENEMY_BULLET_SPEED 	0.0005f
#define PLAYER_BULLET_SPEED	0.001f
#define BULLET_WIDHT 	0.01f
#define BULLET_HEIGHT	0.01f

#endif // CONFIG_H
