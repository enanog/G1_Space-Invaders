/* ---------------------------------------------------
 * enemyFont.h
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

#ifndef ENEMYFONT_H_
#define ENEMYFONT_H_
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "game.h"
#include <stdbool.h>

void draw_title(float x, float y, ALLEGRO_DISPLAY *display);
void draw_hearts(int lives, ALLEGRO_DISPLAY *display);
void draw_player_score(ALLEGRO_DISPLAY *display, int score);
void draw_invaders(hitbox_t enemy, int row, ALLEGRO_DISPLAY *display);
void draw_player(hitbox_t player, ALLEGRO_DISPLAY *display);
void draw_mothership(hitbox_t mothership, ALLEGRO_DISPLAY *display);
void draw_player_died(hitbox_t player, ALLEGRO_DISPLAY *display, long long time);
void draw_bullet(hitbox_t bullet, ALLEGRO_DISPLAY *display);
void draw_explosion(hitbox_t explosion, ALLEGRO_DISPLAY *display, bool enemy);

#endif // ENEMYFONT_H_