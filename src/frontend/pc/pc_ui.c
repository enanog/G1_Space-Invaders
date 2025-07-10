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
#include "pc_ui.h"

static ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_FONT *font = NULL;

static void splash_show(ALLEGRO_DISPLAY* display);
static gameState_t menu_show(ALLEGRO_DISPLAY *display);

static char keyboard_input(void);

void gameLoop(void)
{
    gameState_t state = STATE_SPLASH;
    bool running = true;

    while (running) 
    {
        switch (state) 
        {
            case STATE_SPLASH:
                splash_show(display); // Espera tecla y pasa al siguiente estado
                state = STATE_GAME;
                break;

            case STATE_MENU:
                //state = menu_show(display); // Devolvés un GameState
                break;

            case STATE_GAME:
                map();  // Tu función map adaptada
                break;

            case STATE_EXIT:
                running = false;
                break;

            default:
                running = false;
                break;
        }
    }

    al_destroy_font(font);
    allegro_shutdown();
    playSound_shutdown();
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

    font = al_create_builtin_font();
    if (!font) {
        return false;
    }

	return true;
}

static void splash_show(ALLEGRO_DISPLAY* display)
{
    al_clear_to_color(al_map_rgb(0, 0, 0)); // Fondo negro

    // Título del juego
    al_draw_text(font, al_map_rgb(255, 255, 255),
                 al_get_display_width(display) / 2,
                 al_get_display_height(display) / 3,
                 ALLEGRO_ALIGN_CENTER, "SPACE INVADERS");

    // Instrucción para continuar
    al_draw_text(font, al_map_rgb(180, 180, 180),
                 al_get_display_width(display) / 2,
                 al_get_display_height(display) / 2,
                 ALLEGRO_ALIGN_CENTER, "Press any key to start");

    al_flip_display();

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());

    ALLEGRO_EVENT event;
    bool key_pressed = false;

    while (!key_pressed) {
        al_wait_for_event(queue, &event);
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            key_pressed = true;
        }
    }

    al_destroy_event_queue(queue);
}

gameState_t menu_show(ALLEGRO_DISPLAY *display) {
    const char *options[] = {"Start Game", "Resume", "ScoreBoard", "Exit"};
    int option_count = 4;
    int selected = 0;

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool choosing = true;
    ALLEGRO_EVENT event;

    while (choosing) {
        al_clear_to_color(al_map_rgb(0, 0, 0));

        for (int i = 0; i < option_count; ++i) {
            ALLEGRO_COLOR color = (i == selected) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
            al_draw_text(font, color,
                         al_get_display_width(display) / 2,
                         al_get_display_height(display) / 3 + i * 30,
                         ALLEGRO_ALIGN_CENTER, options[i]);
        }

        al_flip_display();
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_UP:
                    selected = (selected - 1 + option_count) % option_count;
                    break;
                case ALLEGRO_KEY_DOWN:
                    selected = (selected + 1) % option_count;
                    break;
                case ALLEGRO_KEY_ENTER:
                case ALLEGRO_KEY_SPACE:
                    choosing = false;
                    break;
            }
        }
    }

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
	game_init(3, 3, 0);
	input_t player = {0, 0};
	int row, col;
	bool fullscreen = false;
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
				player.direction = -1;
			}
			else if(event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
			{
				player.direction = 1;
			}
			else if(event.keyboard.keycode == ALLEGRO_KEY_SPACE)
			{
				player.shot = true;
			}
            else if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE && !player.pause)
			{
				player.pause = true;
                game_update(player);
			}
            else if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE && player.pause)
            {
                player.pause = false;
            }
            else if(event.keyboard.keycode == ALLEGRO_KEY_R && player.pause)
            {
                player.exit = true;
                game_update(player);
                running = false; // Exit the game loop when paused
            }
			else if(event.keyboard.keycode == ALLEGRO_KEY_F4 && (event.keyboard.modifiers & ALLEGRO_KEYMOD_ALT)) 
			{ 
				running = false;
			}
			else if(event.keyboard.keycode == ALLEGRO_KEY_F11)
			{
				fullscreen = !fullscreen;
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


		if(fullscreen)
		{
			al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, true);
			al_resize_display(display, SCREEN_W, SCREEN_H);
		}
		else
		{
			al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, false);
			al_resize_display(display, SCREEN_W, SCREEN_H);
		}

		if(redraw && al_is_event_queue_empty(queue))
		{
			al_clear_to_color(al_map_rgb(0, 0, 0)); 

			int state = game_update(player);
			if(state)
			{
				printf("GAME OVER\n");
				running = 0;
			}
	
			for(row = 0; row < ENEMIES_ROW_MAX; row++)
			{
				for(col = 0; col < ENEMIES_COLUMNS_MAX; col++)
				{
					if(!getIsEnemyAlive(row,col))
						continue;
					hitbox_t hitbox = getEnemyPosition(row,col);
					ALLEGRO_COLOR color;
					switch (getEnemyTier(row))
					{
					case ALIEN_TIER1:
						color = al_map_rgb(255, 0, 0);
						break;

					case ALIEN_TIER2:
						color = al_map_rgb(0, 255, 0);
						break;

					case ALIEN_TIER3:
						color = al_map_rgb(0, 0, 255);
						break;

					default:
						break;
					}
					al_draw_rectangle(hitbox.start.x * SCREEN_W, hitbox.start.y * SCREEN_H, hitbox.end.x * SCREEN_W, hitbox.end.y * SCREEN_H, color, 2.0f);
				}
			}
			hitbox_t hitbox = getPlayerPosition();
			//printf("Player: x:%f y:%f\n", position.x, position.y);
			al_draw_filled_circle(hitbox.start.x * SCREEN_W,
								  hitbox.start.y * SCREEN_H,
								  5, al_map_rgb(0,255,0));
			al_draw_rectangle(hitbox.start.x * SCREEN_W,
							  hitbox.start.y * SCREEN_H,
							  hitbox.end.x * SCREEN_W,
							  hitbox.end.y * SCREEN_H,
							  al_map_rgb(0,255,0), 2.0f);
			
			int barrier;

			for (barrier = 0; barrier < BARRIER_QUANTITY_MAX; barrier++) 
			{
				for (row = 0; row < BARRIER_ROWS; row++)
				 {
					for (col = 0; col < BARRIER_COLUMNS; col++) 
					{
						bool barrierAlive = getBarrierIsAlive(barrier, row, col);
						if (!barrierAlive)
						{
							continue;
						}
						hitbox_t hitbox = getBarrierPosition(barrier, row, col);
						al_draw_rectangle(hitbox.start.x * SCREEN_W,
                                        hitbox.start.y * SCREEN_H,
                                        hitbox.end.x * SCREEN_W,
                                        hitbox.end.y * SCREEN_H,
                                        al_map_rgb(0,255,0), 2.0f);
					}
				}
			}

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
			
			bullet_t enemyBulletBitMap[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];
			getEnemiesBulletsInfo(enemyBulletBitMap);
			for(row = 0; row < ENEMIES_ROW_MAX; row++)
			{
				for(col = 0; col < ENEMIES_COLUMNS_MAX; col++)
				{
					if(!enemyBulletBitMap[row][col].active)
					{
						continue;
					}

					al_draw_rectangle(
					enemyBulletBitMap[row][col].hitbox.start.x * SCREEN_W,
					enemyBulletBitMap[row][col].hitbox.start.y * SCREEN_H,
					enemyBulletBitMap[row][col].hitbox.end.x * SCREEN_W,
					enemyBulletBitMap[row][col].hitbox.end.y * SCREEN_H,
					al_map_rgb(255, 255, 255), 2.0f);
				}
			}
            hitbox_t mothershipHitbox = getMothershipPosition();
            if(getIsMothershipAlive())
            {
                al_draw_rectangle(
                    mothershipHitbox.start.x * SCREEN_W,
                    mothershipHitbox.start.y * SCREEN_H,
                    mothershipHitbox.end.x * SCREEN_W,
                    mothershipHitbox.end.y * SCREEN_H,
                    al_map_rgb(255, 0, 255), 2.0f);
            }

			al_flip_display();
			redraw = false;
		}
	}

	al_destroy_timer(timer);
	al_destroy_event_queue(queue);
	allegro_shutdown();
	playSound_shutdown();
}

void allegro_shutdown(void) 
{
	if (display)
	{
		al_destroy_display(display);
	}
}

