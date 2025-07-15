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
 * 	MAGLIOLA, Nicolas
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

/* ---------------------------------------------------
 * @brief Draw the main game title at the specified position
 *
 * Loads a large custom font and renders the title symbol centered
 * at the (x, y) position on the given display.
 *
 * @param x Horizontal position (centered)
 * @param y Vertical position (top baseline of text)
 * @param display Pointer to the Allegro display
 * ---------------------------------------------------*/
void draw_title(float x, float y, ALLEGRO_DISPLAY *display);

/* ---------------------------------------------------
 * @brief Render player's remaining lives as heart icons
 *
 * Draws up to 5 hearts in two rows at the bottom-right corner of the display.
 * Hearts representing lost lives are shown in a dimmed gray color.
 * Positions and spacing are calculated dynamically based on display size.
 *
 * @param lives Number of remaining lives (0 to 5)
 * @param display Pointer to the Allegro display for rendering
 * ---------------------------------------------------*/
void draw_hearts(int lives, ALLEGRO_DISPLAY *display);

/* ---------------------------------------------------
 * @brief Render the player's score on the screen
 *
 * This function displays the text "SCORE:" followed by the player's
 * current score, right-aligned near the top corner of the display.
 * The score is padded with zeros to ensure consistent width.
 *
 * @param display Pointer to the Allegro display
 * @param score Integer value representing the player's score
 * ---------------------------------------------------*/
void draw_player_score(ALLEGRO_DISPLAY *display, int score);

/* ---------------------------------------------------
 * @brief Draw barrier segments with visual effects
 * @param barrier Barrier index
 * @param margin_x X margin for clipping
 * @param margin_y Y margin for clipping
 * @param inner_w Inner width for clipping
 * @param inner_h Inner height for clipping
 * @param show_hitboxes Whether to render hitboxes
 * ---------------------------------------------------*/
void draw_barriers(int barrier, float margin_x, float margin_y, float inner_w, float inner_h, bool show_hitboxes);

/* ---------------------------------------------------
 * @brief Draw an invader enemy at specified position and row tier
 *
 * Selects the font size based on predefined enemy height and current display size,
 * then draws the enemy character according to its tier (1, 2, or 3).
 * The enemy's hand position (up or down) toggles dynamically via getEnemiesHands().
 *
 * @param enemy Hitbox representing the enemy's position and size
 * @param row The row index of the enemy to determine its tier
 * @param display Pointer to the Allegro display used for rendering
 * ---------------------------------------------------*/
void draw_invaders(hitbox_t enemy, int row, ALLEGRO_DISPLAY *display);

/* ---------------------------------------------------
 * @brief Render the player character at the specified position
 *
 * Loads the player font scaled by PLAYER_HEIGHT relative to display height,
 * then draws the player sprite at the given hitbox coordinates.
 *
 * @param player Hitbox struct indicating the player's position on screen
 * @param display Pointer to the Allegro display for rendering
 * ---------------------------------------------------*/
void draw_player(hitbox_t player, ALLEGRO_DISPLAY *display);

/* ---------------------------------------------------
 * @brief Render the mothership entity at the specified position
 *
 * Loads the mothership font scaled by MOTHERSHIP_HEIGHT relative to display height,
 * then draws the mothership sprite at the given hitbox coordinates.
 *
 * @param mothership Hitbox struct indicating the mothership's position on screen
 * @param display Pointer to the Allegro display for rendering
 * ---------------------------------------------------*/
void draw_mothership(hitbox_t mothership, ALLEGRO_DISPLAY *display);

/* ---------------------------------------------------
 * @brief Draw the bullet sprite at the specified position
 *
 * Loads the bullet font scaled by ENEMY_HEIGHT relative to display height,
 * then renders the bullet character at the bullet's hitbox coordinates.
 *
 * @param bullet Hitbox struct representing the bullet's position on screen
 * @param display Pointer to the Allegro display used for rendering
 * ---------------------------------------------------*/
void draw_bullet(hitbox_t bullet, ALLEGRO_DISPLAY *display);

/* ---------------------------------------------------
 * @brief Draw explosion effect at specified position with size depending on enemy type
 *
 * Loads an explosion font glyph scaled by enemy type size and renders it
 * in red at the explosion hitbox coordinates.
 *
 * @param explosion Hitbox defining the position to draw the explosion
 * @param display Pointer to the Allegro display for size calculations
 * @param enemy Boolean flag indicating if explosion is from enemy;
 *              if equal to MOTHERSHIP_EXPLOSION, uses mothership scaling
 * ---------------------------------------------------*/
void draw_explosion(hitbox_t explosion, ALLEGRO_DISPLAY *display, bool enemy);

/* ---------------------------------------------------
 * @brief Draw the player death animation using font glyphs
 *
 * Displays a red "dead player" symbol at the player's location.
 * If the time since death is within a certain range, an explosion
 * glyph is toggled to simulate blinking/flashing. After a fixed
 * duration, the animation stops drawing.
 *
 * @param player Hitbox representing the player's screen position
 * @param display Pointer to the active Allegro display
 * @param time Time in milliseconds since the player died
 * ---------------------------------------------------*/
void draw_player_died(hitbox_t player, ALLEGRO_DISPLAY *display, long long time);

/* ---------------------------------------------------
 * @brief Adjust hitbox coordinates based on display margins
 * @param hb Original hitbox
 * @param margin_x Horizontal margin
 * @param margin_y Vertical margin
 * @param inner_w Inner width
 * @param inner_h Inner height
 * @return Adjusted hitbox
 * ---------------------------------------------------*/
hitbox_t clipHitbox(hitbox_t hb, float margin_x, float margin_y, float inner_w, float inner_h);

#endif // ENEMYFONT_H_