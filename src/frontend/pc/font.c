/* ---------------------------------------------------
 * font.c
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

#include "font.h"
#include <stdio.h>

ALLEGRO_FONT *font1 = NULL;
ALLEGRO_FONT *font2 = NULL;
ALLEGRO_FONT *font3 = NULL;

const char TIER_1_HANDS_UP[] = "\x41"; // 36 decimal
const char TIER_1_HANDS_DOWN[] = "\x42"; // 37
const char TIER_2_HANDS_DOWN[] = "\x43"; // 29
const char TIER_2_HANDS_UP[] = "\x44";   // 30
const char TIER_3_HANDS_DOWN[] = "\x45"; // 40
const char TIER_3_HANDS_UP[] = "\x46";   // 41
const char PLAYER[] = "\x2D";   // 41


void initFonts(ALLEGRO_DISPLAY *display)
{
    font1 = al_load_ttf_font("assets/fonts/invaders.ttf", ENEMY_WIDTH*al_get_display_width(display), 0);
    if (!font1) {
        fprintf(stderr, "Failed to load font1\n");
    }

    font2 = al_load_ttf_font("assets/fonts/invaders.ttf", PLAYER_WIDTH*al_get_display_width(display), 0);
    if (!font2) {
        fprintf(stderr, "Failed to load font2\n");
    }

    font3 = al_load_ttf_font("assets/fonts/pixel-invaders.ttf", ENEMY_WIDTH*al_get_display_width(display), 0);
    if (!font3) {
        fprintf(stderr, "Failed to load font3\n");
    }
}

void draw_invaders(hitbox_t enemy, int row, ALLEGRO_DISPLAY *display)
{
    font1 = al_load_ttf_font("assets/fonts/invaders.ttf", ENEMY_WIDTH*al_get_display_width(display), 0);
    if (!font1) {
        fprintf(stderr, "Failed to load font1\n");
    }
    switch (getEnemyTier(row))
    {
        case ALIEN_TIER1:
            al_draw_text(font1, al_map_rgb(255, 255, 255), enemy.start.x * al_get_display_width(display), enemy.start.y * al_get_display_height(display), 0, (getEnemiesHands())? TIER_1_HANDS_UP : TIER_1_HANDS_DOWN);
            break;
        case ALIEN_TIER2:
            al_draw_text(font1, al_map_rgb(255, 255, 255), enemy.start.x * al_get_display_width(display), enemy.start.y * al_get_display_height(display), 0, (getEnemiesHands())? TIER_2_HANDS_UP : TIER_2_HANDS_DOWN);
            break;
        case ALIEN_TIER3:
            al_draw_text(font1, al_map_rgb(255, 255, 255), enemy.start.x * al_get_display_width(display), enemy.start.y * al_get_display_height(display), 0, (getEnemiesHands())? TIER_3_HANDS_UP : TIER_3_HANDS_DOWN);
            break;
    }
}

void draw_player(hitbox_t player, ALLEGRO_DISPLAY *display)
{
    al_draw_text(font2, al_map_rgb(255, 255, 255), player.start.x * al_get_display_width(display), player.start.y * al_get_display_height(display), 0, PLAYER);
}
