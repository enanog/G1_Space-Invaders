/* ---------------------------------------------------
 * font.h
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

#ifndef FONT_H
#define FONT_H
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "game.h"
#include <stdbool.h>

void initFonts(ALLEGRO_DISPLAY *display);
void draw_invaders(hitbox_t enemy, int row, ALLEGRO_DISPLAY *display);
void draw_player(hitbox_t player, ALLEGRO_DISPLAY *display);

#endif // FONT_H