/* ---------------------------------------------------
 * pc_ui.c
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

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "game.h"
#include "config.h"
#include "playSound.h"
#include <stdio.h>
#include <stdbool.h>
#include "entity.h"

ALLEGRO_DISPLAY *display = NULL;

void map(void);
bool allegro_init(void);
void allegro_shutdown(void);

int main(void)
{
    if (!allegro_init()) {
        fprintf(stderr, "ERROR: Failed to initialize Allegro.\n");
        return 1;
    }

	if (!playSound_init()) 
	{
		return -1;
	}

    // playSound_play(SOUND_SHOOT);
    // al_rest(0.4);
    // playSound_play(SOUND_INVADER_KILLED);
    // al_rest(0.5);
    // playSound_play(SOUND_UFO_HIGH);
    // al_rest(1.0);

    // playSound_shutdown();
    
	map();
    return 0;
}

bool allegro_init(void) 
{
    if (!al_init() || !al_init_font_addon() || !al_init_ttf_addon() ||
        !al_init_primitives_addon() || !al_init_image_addon())
    {
        return false;
    }
    if (!al_install_keyboard())
    {
        return false;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display)
    {
        return false;
    }

    return true;
}


void map(void)
{
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	ALLEGRO_TIMER *timer = al_create_timer(1.0 / 144.0);

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_register_event_source(queue, al_get_display_event_source(display));

	al_start_timer(timer);

    bool redraw = true;
    bool running = true;

	al_clear_to_color(al_map_rgb(0, 0, 0));
    game_init(5, 11, BARRIER_QUANTITY_MAX, BARRIER_ROWS_MAX, BARRIER_COLUMNS_MAX);
	input_t player = {0, 0};
    while(running)
    {
		
		ALLEGRO_EVENT event;
    	al_wait_for_event(queue, &event);

		if(event.type == ALLEGRO_EVENT_TIMER)
		{
			redraw = true;
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) 
        {
            running = false;
			break;
        }
		if(event.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			if(event.keyboard.keycode == ALLEGRO_KEY_LEFT)
			{
				printf("Me movi\n");
				player.direction = -1;
			}
			else if(event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
			{
				player.direction = 1;
			}
			else if(event.keyboard.keycode == ALLEGRO_KEY_SPACE)
			{
				player.shot = 1;
			}
		}
		else if(event.type == ALLEGRO_EVENT_KEY_UP)
		{
			if(event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
			{
				player.direction = 0;
			}
			else if(event.keyboard.keycode == ALLEGRO_KEY_SPACE)
			{
				player.shot = 0;
			}
		}


		if(redraw && al_is_event_queue_empty(queue))
		{
			al_clear_to_color(al_map_rgb(0, 0, 0)); 

			int state = game_update(player);
			if(state)
			{
				printf("GAME OVER");
			}
			int row, column;
			for(row = 0; row < ENEMIES_ROW_MAX; row++)
			{
				for(column = 0; column < ENEMIES_COLUMNS_MAX; column++)
				{
					if(!getIsEnemyAlive(row,column))
						continue;
					//printf("Enemy[%d][%d]: x:%f y:%f\n", row, column, position.x, position.y);
					hitbox_t hitbox = getEnemyPosition(row,column);
					ALLEGRO_COLOR color1, color2;
					switch (getEnemyTier(row))
					{
					case ALIEN_TIER1:
						color1 = al_map_rgb(255, 0, 0);
                        color2 = al_map_rgb(255, 255, 0);
						break;

					case ALIEN_TIER2:
						color1 = al_map_rgb(0, 255, 0);
                        color2 = al_map_rgb(0, 255, 255);
						break;

					case ALIEN_TIER3:
						color1 = al_map_rgb(0, 0, 255);
                        color1 = al_map_rgb(255, 0, 255);
						break;

					default:
						break;
					}
                    al_draw_filled_circle(hitbox.start.x * SCREEN_W, hitbox.start.y  * SCREEN_H, 10, color1);
                    al_draw_filled_circle(hitbox.end.x * SCREEN_W, hitbox.end.y  * SCREEN_H, 5, color2);
					al_draw_rectangle(hitbox.start.x * SCREEN_W, hitbox.start.y * SCREEN_H, hitbox.end.x * SCREEN_W, hitbox.end.y * SCREEN_H, color1, 2.0f);
				}
			}
			hitbox_t hitbox = getPlayerPosition();
			//printf("Player: x:%f y:%f\n", position.x, position.y);
            al_draw_filled_circle(hitbox.start.x * SCREEN_W, hitbox.start.y * SCREEN_H, 5, al_map_rgb(0,255,0));
			al_draw_rectangle(hitbox.start.x * SCREEN_W, hitbox.start.y * SCREEN_H, hitbox.end.x * SCREEN_W, hitbox.end.y * SCREEN_H, al_map_rgb(0,255,0), 2.0f);
			
			// int barrier;

			// for (barrier = 0; barrier < BARRIER_QUANTITY_MAX; barrier++) 
			// {
			// 	for (row = 0; row < BARRIER_ROWS_MAX; row++)
			// 	 {
			// 		for (column = 0; column < BARRIER_COLUMNS_MAX; column++) 
			// 		{
			// 			coord_t position = getBarrierPosition(barrier, row, column);

			// 			// Si la posición es (0, 0), asumimos que no existe o está fuera de rango
			// 			if (position.x == 0.0f && position.y == 0.0f)
			// 				continue;
						
			// 			al_draw_filled_circle(position.x * SCREEN_W, position.y * SCREEN_H, 5, al_map_rgb(0,255,255));
			// 		}
			// 	}
			// }
			bullet_t bullet = getPlayerBulletinfo();
			if (bullet.active) {
				al_draw_rectangle(
					bullet.hitbox.start.x * SCREEN_W,
					bullet.hitbox.start.y * SCREEN_H,
					bullet.hitbox.end.x * SCREEN_W,
					bullet.hitbox.end.y * SCREEN_H,
					al_map_rgb(255, 255, 0),
                    2.0f
				);
			}
			
			al_flip_display();
			redraw = false;
		}
    }

	al_destroy_timer(timer);
    al_destroy_event_queue(queue);
	allegro_shutdown();
}

void allegro_shutdown(void) 
{
    if (display)
	{
		al_destroy_display(display);
	}
}

