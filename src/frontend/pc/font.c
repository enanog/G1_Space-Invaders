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

ALLEGRO_FONT *font_enemy = NULL;
ALLEGRO_FONT *font_player = NULL;
ALLEGRO_FONT *font_mothership = NULL;

const char TIER_1_HANDS_UP[]    = "\x46";
const char TIER_1_HANDS_DOWN[]  = "\x47"; 
const char TIER_2_HANDS_DOWN[]  = "\x42"; 
const char TIER_2_HANDS_UP[]    = "\x43";   
const char TIER_3_HANDS_DOWN[]  = "\x44"; 
const char TIER_3_HANDS_UP[]    = "\x45";   
const char PLAYER[]             = "\x57";
const char PLAYER_DIED[]        = "\x58";   
const char MOTHERSHIP[]         = "\x56";
const char BULLET[]             = "\x59";
const char ESPLOSION[]          = "\x5A";


void initFonts(ALLEGRO_DISPLAY *display)
{
    font_enemy = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", ENEMY_HEIGHT*al_get_display_height(display), 0);
    if (!font_enemy) {
        fprintf(stderr, "Failed to load font_enemy\n");
    }
    font_player = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", PLAYER_HEIGHT*al_get_display_height(display), 0);
    if (!font_player) {
        fprintf(stderr, "Failed to load font_player\n");
    }
    font_mothership = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", MOTHERSHIP_HEIGHT*al_get_display_height(display), 0);
    if (!font_mothership) 
    {
        fprintf(stderr, "Failed to load font_mothership\n");
    }
}

void draw_invaders(hitbox_t enemy, int row, ALLEGRO_DISPLAY *display)
{
    
    switch (getEnemyTier(row))
    {
        case ALIEN_TIER1:
            al_draw_text(font_enemy, al_map_rgb(255, 255, 255), enemy.start.x, enemy.start.y, 0, (getEnemiesHands())? TIER_1_HANDS_UP : TIER_1_HANDS_DOWN);
            break;
        case ALIEN_TIER2:
            al_draw_text(font_enemy, al_map_rgb(255, 255, 255), enemy.start.x, enemy.start.y, 0, (getEnemiesHands())? TIER_2_HANDS_UP : TIER_2_HANDS_DOWN);
            break;
        case ALIEN_TIER3:
            al_draw_text(font_enemy, al_map_rgb(255, 255, 255), enemy.start.x, enemy.start.y, 0, (getEnemiesHands())? TIER_3_HANDS_UP : TIER_3_HANDS_DOWN);
            break;
    }
}

void draw_player(hitbox_t player, ALLEGRO_DISPLAY *display)
{
    al_draw_text(font_player, al_map_rgb(255, 255, 255), player.start.x, player.start.y, 0, PLAYER);
}

void draw_mothership(hitbox_t mothership, ALLEGRO_DISPLAY *display)
{
    al_draw_text(font_mothership, al_map_rgb(255, 255, 255), mothership.start.x, mothership.start.y, 0, MOTHERSHIP);
}

void draw_bullet(hitbox_t bullet, ALLEGRO_DISPLAY *display)
{
    al_draw_text(font_enemy, al_map_rgb(255, 255, 255), bullet.start.x, bullet.start.y, 0, BULLET);
}

void draw_explosion(hitbox_t explosion, ALLEGRO_DISPLAY *display)
{
    al_draw_text(font_enemy, al_map_rgb(255, 0, 0), explosion.start.x, explosion.start.y, 0, ESPLOSION);
}

void draw_player_died(hitbox_t player, ALLEGRO_DISPLAY *display)
{
    al_draw_text(font_player, al_map_rgb(255, 0, 0), player.start.x, player.start.y, 0, PLAYER_DIED);
}