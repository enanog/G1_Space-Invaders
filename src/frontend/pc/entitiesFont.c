/* ---------------------------------------------------
 * entitiesFont.c
 * ---------------------------------------------------
 * GRUPO 1:
 * 	CASTRO, Tomás
 *	FRIGERIO, Dylan
 * 	VALENZUELA, Agustín
 * 	YAGGI, Lucca
 *
 * Profesores:
 * 	MAGLIOLA, Nicolas
 * 	JACOBY, Daniel
 * 	VACATELLO, Pablo
 *
 * fecha: 15/07/2025
 * ---------------------------------------------------*/

#include "entitiesFont.h"
#include <stdio.h>

#define MOTHERSHIP_EXPLOSION    1 

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
const char EXPLOSION[]          = "\x5A";
const char TITLE[]              = "\x2E";
const char HEART[]              = "\x7C";

void draw_hearts(int lives, ALLEGRO_DISPLAY *display)
{
    ALLEGRO_FONT *font_heart = al_load_ttf_font("assets/fonts/invaders.ttf", 0.05f*al_get_display_height(display), 0);
    if (!font_heart) 
    {
        fprintf(stderr, "Failed to load font_heart\n");
    }
    int screen_w = al_get_display_width(display);
    int screen_h = al_get_display_height(display);

    const float heart_spacing_x = screen_w * 0.04f;
    const float heart_spacing_y = screen_h * 0.05f;

    const float margin_x = screen_w * 0.03f;
    const float margin_y = screen_h * 0.1f;

    // Centro horizontal del grupo de corazones
    float center_x = screen_w - margin_x - heart_spacing_x;  // punto medio de la fila superior
    float base_y = screen_h - margin_y - heart_spacing_y * 2;  // fila superior

    ALLEGRO_COLOR color = al_map_rgb(255, 0, 0);

    for (int i = 0; i < 5; ++i)
    {
        float x, y;

        if(i >= lives)
        {
            color = al_map_rgb(143, 143, 143);
        }

        if (i < 3)
        {
            // Fila superior: centrados respecto al centro_x
            x = center_x + (i - 1) * heart_spacing_x; // -1, 0, 1 → centrado
            y = base_y;
        }
        else
        {
            // Fila inferior: entre los corazones de arriba
            x = center_x + (i - 3.5f) * heart_spacing_x; // da -0.5 y 0.5 → justo entre los de arriba
            y = base_y + heart_spacing_y;
        }

        al_draw_text(font_heart, color, x, y, ALLEGRO_ALIGN_CENTER, HEART);
    }
    al_destroy_font(font_heart);
}

void draw_player_score(ALLEGRO_DISPLAY *display, int score)
{
    ALLEGRO_FONT *font = al_load_ttf_font("assets/fonts/space-invaders-full-version.otf", 0.035f *al_get_display_height(display), 0);
    if (!font) 
    {
        fprintf(stderr, "Failed to load font_title\n");
    }
    int screen_w = al_get_display_width(display);
    int screen_h = al_get_display_height(display);

    const float heart_spacing_x = screen_w * 0.04f;
    const float margin_x = screen_w * 0.03f;
    const float margin_y = screen_h * 0.1f;

    // Centro horizontal de los corazones
    float center_x = screen_w - margin_x - heart_spacing_x;

    // Línea base para el texto superior
    float y_score_label = margin_y;

    // Altura de la fuente
    float font_height = al_get_font_line_height(font);

    // Línea base para el número debajo
    float y_score_value = y_score_label + font_height * 1.5f;

    // Dibujar "SCORE:"
    al_draw_text(font, al_map_rgb(255, 255, 255), center_x, y_score_label, ALLEGRO_ALIGN_CENTER, "SCORE:");

    // Dibujar el valor numérico
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%06d", score); // 5 dígitos con ceros a la izquierda
    al_draw_text(font, al_map_rgb(255, 255, 255), center_x, y_score_value, ALLEGRO_ALIGN_CENTER, buffer);
    al_destroy_font(font);
}

void draw_title(float x, float y, ALLEGRO_DISPLAY *display)
{
    ALLEGRO_FONT *font_title = al_load_ttf_font("assets/fonts/invaders.ttf", 0.2*al_get_display_height(display), 0);
    if (!font_title) 
    {
        fprintf(stderr, "Failed to load font_title\n");
    }
    al_draw_text(font_title, al_map_rgb(255, 255, 255),x, y, ALLEGRO_ALIGN_CENTER, TITLE);
    al_destroy_font(font_title);
}

void draw_invaders(hitbox_t enemy, int row, ALLEGRO_DISPLAY *display)
{
    ALLEGRO_FONT *font_enemy = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", ENEMY_HEIGHT*al_get_display_height(display), 0);
    if (!font_enemy) {
        fprintf(stderr, "Failed to load font_enemy\n");
    }
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
    al_destroy_font(font_enemy);
}

void draw_player(hitbox_t player, ALLEGRO_DISPLAY *display)
{
    ALLEGRO_FONT *font_player = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", PLAYER_HEIGHT*al_get_display_height(display), 0);
    if (!font_player) {
        fprintf(stderr, "Failed to load font_player\n");
    }
    al_draw_text(font_player, al_map_rgb(255, 255, 255), player.start.x, player.start.y, 0, PLAYER);
    al_destroy_font(font_player);
}

void draw_mothership(hitbox_t mothership, ALLEGRO_DISPLAY *display)
{
    ALLEGRO_FONT *font_mothership = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", MOTHERSHIP_HEIGHT*al_get_display_height(display), 0);
    if (!font_mothership) 
    {
        fprintf(stderr, "Failed to load font_mothership\n");
    }
    al_draw_text(font_mothership, al_map_rgb(255, 255, 255), mothership.start.x, mothership.start.y, 0, MOTHERSHIP);
    al_destroy_font(font_mothership);
}

void draw_bullet(hitbox_t bullet, ALLEGRO_DISPLAY *display)
{
    ALLEGRO_FONT *font_bullet = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", ENEMY_HEIGHT*al_get_display_height(display), 0);
    if (!font_bullet) {
        fprintf(stderr, "Failed to load font_bullet\n");
    }
    al_draw_text(font_bullet, al_map_rgb(255, 255, 255), bullet.start.x, bullet.start.y, 0, BULLET);
    al_destroy_font(font_bullet);
}

void draw_explosion(hitbox_t explosion, ALLEGRO_DISPLAY *display, bool enemy)
{
    ALLEGRO_FONT *font_explosion = NULL;
    if(enemy != MOTHERSHIP_EXPLOSION)
        font_explosion = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", 1.2f*ENEMY_HEIGHT*al_get_display_height(display), 0);
    else
        font_explosion = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", 1.2f*MOTHERSHIP_HEIGHT*al_get_display_height(display), 0);
    if (!font_explosion) {
        fprintf(stderr, "Failed to load font_enemy\n");
    }  
    al_draw_text(font_explosion, al_map_rgb(255, 0, 0), explosion.start.x, explosion.start.y, 0, EXPLOSION);
    al_destroy_font(font_explosion);
}

void draw_player_died(hitbox_t player, ALLEGRO_DISPLAY *display, long long time)
{
    ALLEGRO_FONT *font_player = al_load_ttf_font("assets/fonts/Invaders-From-Space.ttf", PLAYER_HEIGHT*al_get_display_height(display), 0);
    if (!font_player) {
        fprintf(stderr, "Failed to load font_player\n");
    }
    if(time > 350)
    {
        al_destroy_font(font_player);
        return;
    }

    al_draw_text(font_player, al_map_rgb(255, 0, 0), player.start.x, player.start.y, 0, PLAYER_DIED);
    if(time % 100 > 50)
    {
        al_draw_text(font_player, al_map_rgb(255, 0, 0), player.start.x, player.start.y, 0, EXPLOSION);
    }
    
    al_destroy_font(font_player);
}