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
#include "score.h"

#define DONT_MATTER -1

static ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_FONT *font = NULL;

static gameState_t menuShow(ALLEGRO_DISPLAY *display);
static void mainMenu(void);
static gameState_t gameRender(gameState_t state, int enemyRow, int enemyCol);
static char keyboard_input(void);
static gameState_t pauseMenu(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *background);

void gameLoop(void)
{
    gameState_t state = STATE_SPLASH;
    bool running = true;

    while (running) 
    {
        switch (state) 
        {
            case STATE_SPLASH: 
                mainMenu();
                state = STATE_MENU;
                break;

            case STATE_MENU:
                state = menuShow(display); // Devolvés un GameState
                break;

            case STATE_NEW_GAME:
                printf("Starting new game...\n");
                state = gameRender(STATE_NEW_GAME, 3, 3);
                break;

            case STATE_RESUME_GAME:
                printf("Resuming game...\n");
                state = gameRender(STATE_RESUME_GAME, DONT_MATTER, DONT_MATTER);
                break;

            case STATE_SCOREBOARD:
                break;

            case STATE_EXIT:
                running = false;
                break;

            case STATE_CREDITS:
                break;  
            
            case STATE_PAUSE:
                ALLEGRO_BITMAP *snapshot = al_create_bitmap(al_get_display_width(display), al_get_display_height(display));
                al_set_target_bitmap(snapshot);
                al_draw_bitmap(al_get_backbuffer(display), 0, 0, 0); // Captura del frame actual
                al_set_target_backbuffer(display);
                printf("Game paused. Showing pause menu...\n");
                state = pauseMenu(display, snapshot);
                al_destroy_bitmap(snapshot);
                if (state == STATE_EXIT) 
                {
                    running = false;
                }
                break;

            case STATE_RESTART_GAME:
                printf("Restarting game...\n");
                state = gameRender(STATE_NEW_GAME, 3, 3);
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

static void mainMenu(void) 
{
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));

    bool running = true;
    bool show_press_enter = false;
    double start_time = al_get_time();

    while (running) 
    {
        //ALLEGRO_FONT *font1 = al_load_ttf_font("invaders.ttf", 32, 0);
        al_clear_to_color(al_map_rgb(0, 0, 0));

        // Draw "SPACE INVADERS" big and centered (simulate style)
        //al_draw_text(font1, al_map_rgb(255, 255, 0), SCREEN_W / 2, 120, ALLEGRO_ALIGN_CENTER, (char)17);

        int screen_w = al_get_display_width(display);
        int screen_h = al_get_display_height(display);

        al_draw_text(font, al_map_rgb(255, 255, 0), screen_w / 2, screen_h * 0.2, ALLEGRO_ALIGN_CENTER, "SPACE");
        al_draw_text(font, al_map_rgb(255, 255, 0), screen_w / 2, screen_h * 0.3, ALLEGRO_ALIGN_CENTER, "INVADERS");

        int rows = 5;
        int cols = 11;

        // Tamaño relativo de cada invasor (ancho y alto)
        float invader_width = screen_w * ENEMY_WIDTH;
        float invader_height = screen_h * ENEMY_HEIGHT;

        // Espaciado horizontal y vertical entre invasores
        float h_spacing = screen_w * ENEMY_H_SPACING;
        float v_spacing = screen_h * ENEMY_V_SPACING;

        // Coordenada de inicio (centrado horizontalmente)
        float total_width = cols * invader_width + (cols - 1) * h_spacing;
        float start_x = (screen_w - total_width) / 2;

        // Coordenada Y inicia
        float start_y = screen_h * 0.4f;

        // Colores para cada fila
        ALLEGRO_COLOR row_colors[] = 
        {
            al_map_rgb(255, 0, 255),   // pink
            al_map_rgb(255, 255, 0),   // yellow
            al_map_rgb(255, 0, 0),     // red
            al_map_rgb(0, 255, 255),   // blue
            al_map_rgb(0, 255, 255)    // blue
        };

        for (int row = 0; row < rows; row++) 
        {
            for (int col = 0; col < cols; col++) 
            {
                float x1 = start_x + col * (invader_width + h_spacing);
                float y1 = start_y + row * (invader_height + v_spacing);
                float x2 = x1 + invader_width;
                float y2 = y1 + invader_height;

                al_draw_filled_rectangle(x1, y1, x2, y2, row_colors[row]);
            }
        }

        // After 2 seconds, show "Press ENTER to start"
        if (al_get_time() - start_time > 2.0) 
        {
            show_press_enter = true;
        }
        if (show_press_enter) 
        {
            al_draw_text(font, al_map_rgb(255, 255, 255), screen_w / 2, screen_h * 0.8, ALLEGRO_ALIGN_CENTER, "Press ENTER to start");
        }

        al_flip_display();

        ALLEGRO_EVENT event;
        if (al_wait_for_event_timed(queue, &event, 0.05)) 
        {
            if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) 
            {
                running = false;
                allegro_shutdown();
                exit(0);
            }
            if (show_press_enter && event.keyboard.keycode == ALLEGRO_KEY_ENTER) 
            {
                running = false;
            }
        }
    }
    al_destroy_event_queue(queue);
}

static gameState_t menuShow(ALLEGRO_DISPLAY *display) 
{
    const char *options[] = {"Start Game", "Resume", "ScoreBoard", "Credits", "Exit"};
    int option_count = 5;
    int selected = 0;

    score_t topScores[5];
    int topCount = getTopScore(topScores); // Load top 5 scores

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool choosing = true;
    ALLEGRO_EVENT event;

    // Screen dimensions
    int screen_w = al_get_display_width(display);
    int screen_h = al_get_display_height(display);

    // Font size dependent metrics
    float font_size = al_get_font_line_height(font);

    // Positions and sizes scaled to screen
    float title_y = screen_h * 0.1f;
    float menu_start_y = screen_h * 0.4f;
    float option_spacing = screen_h * 0.05f;

    float rect_width = screen_w * 0.25f;
    float rect_height = screen_h * 0.2f;
    float rect_x = (screen_w - rect_width) / 2;
    float rect_y = (title_y + menu_start_y) / 2 - rect_height / 2;

    while (choosing) 
    {
        al_clear_to_color(al_map_rgb(0, 0, 0)); // Black background

        // Draw title
        al_draw_text(font, al_map_rgb(255, 0, 0),
                     screen_w / 2,
                     title_y,
                     ALLEGRO_ALIGN_CENTER, "SPACE INVADERS");

        // Draw score box background
        al_draw_filled_rectangle(rect_x, rect_y, rect_x + rect_width, rect_y + rect_height,
                                 al_map_rgb(100, 100, 100));

        // Draw box border
        al_draw_rectangle(rect_x, rect_y, rect_x + rect_width, rect_y + rect_height,
                          al_map_rgb(255, 255, 255), 2);

        // Draw "TOP SCORES" title inside the box
        al_draw_text(font, al_map_rgb(255, 255, 255),
                     rect_x + rect_width / 2,
                     rect_y + font_size * 0.2f,
                     ALLEGRO_ALIGN_CENTER, "TOP SCORES");

        // Draw top 5 scores
        for (int i = 0; i < 5 && i < topCount; ++i) 
        {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%d. %s - %d", i + 1, topScores[i].name, topScores[i].score);

            al_draw_text(font, al_map_rgb(255, 255, 255),
                         rect_x + screen_w * 0.01f,
                         rect_y + font_size * 1.2f + i * (font_size * 1.1f),
                         ALLEGRO_ALIGN_LEFT, buffer);
        }

        // Draw menu options
        for (int i = 0; i < option_count; ++i) 
        {
            ALLEGRO_COLOR color = (i == selected) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
            al_draw_text(font, color,
                         screen_w / 2,
                         menu_start_y + i * option_spacing,
                         ALLEGRO_ALIGN_CENTER, options[i]);
        }

        al_flip_display();
        al_wait_for_event(queue, &event);

        // Keyboard input
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) 
        {
            switch (event.keyboard.keycode)
             {
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

    al_destroy_event_queue(queue);

    // Return next state
    switch (selected) 
    {
        case 0: 
            return STATE_NEW_GAME;
        case 1: 
            return STATE_RESUME_GAME;
        case 2: 
            return STATE_SCOREBOARD;
        case 3: 
            return STATE_CREDITS;
        case 4: 
            return STATE_EXIT;
    }

    return STATE_MENU;
}


static gameState_t gameRender(gameState_t state, int enemyRow, int enemyCol)
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

    if(state == STATE_NEW_GAME)
    {
        game_init(enemyRow, enemyCol, false);
    }
    else if(state == STATE_RESUME_GAME)
    {
        game_init(enemyRow, enemyCol, true);
    }

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
                state = STATE_PAUSE;
                running = false; // Exit the game loop when paused
			}
            else if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE && player.pause)
            {
                player.pause = false;
            }
            else if(event.keyboard.keycode == ALLEGRO_KEY_R && player.pause)
            {
                player.exit = true;
                game_update(player);
                state = STATE_EXIT;
                running = false; // Exit the game loop when paused
            }
			else if(event.keyboard.keycode == ALLEGRO_KEY_F4 && (event.keyboard.modifiers & ALLEGRO_KEYMOD_ALT)) 
			{ 
				running = false;
                allegro_shutdown();
                exit(0); // Exit the game when Alt+F4 is pressed
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

			int gameState = game_update(player);
			if(gameState == GAME_OVER)
			{
				printf("GAME OVER\n");
				running = false;
                state = STATE_MENU;
                break;
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
	return state;
}

static gameState_t pauseMenu(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *background)
{
    const char *options[] = {"Resume", "Restart", "Quit to Menu", "Exit Game"};
    int option_count = 4;
    int selected = 0;

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool choosing = true;
    ALLEGRO_EVENT event;

    int screen_w = al_get_display_width(display);
    int screen_h = al_get_display_height(display);
    float option_spacing = screen_h * 0.06f;
    float rect_width = screen_w * 0.4f;
    float rect_height = option_count * option_spacing + 40;
    float rect_x = (screen_w - rect_width) / 2;
    float rect_y = (screen_h - rect_height) / 2;

    while (choosing) {
        // Dibujar la imagen congelada del juego
        al_draw_bitmap(background, 0, 0, 0);

        // Dibujar fondo negro semi-transparente
        al_draw_filled_rectangle(0, 0, screen_w, screen_h,
            al_map_rgba(0, 0, 0, 128)); // Transparencia al 50%

        // Dibujar rectángulo de opciones
        al_draw_filled_rectangle(rect_x, rect_y, rect_x + rect_width, rect_y + rect_height,
                                 al_map_rgb(50, 50, 50));
        al_draw_rectangle(rect_x, rect_y, rect_x + rect_width, rect_y + rect_height,
                          al_map_rgb(255, 255, 255), 2);

        // Dibujar cada opción
        for (int i = 0; i < option_count; ++i) 
        {
            ALLEGRO_COLOR color = (i == selected) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
            al_draw_text(font, color,
                         rect_x + rect_width / 2,
                         rect_y + 20 + i * option_spacing,
                         ALLEGRO_ALIGN_CENTER, options[i]);
        }

        al_flip_display();
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_KEY_DOWN) 
        {
            switch (event.keyboard.keycode) 
            {
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
                case ALLEGRO_KEY_ESCAPE:
                    selected = 0; // Resume by default
                    choosing = false;
                    break;
            }
        }
    }

    al_destroy_event_queue(queue);

    // Devolver estado según opción
    switch (selected) 
    {
        case 0: 
            return STATE_RESUME_GAME;
        case 1: 
            return STATE_RESTART_GAME;
        case 2: 
            return STATE_MENU;
        case 3: 
            return STATE_EXIT;
    }

    return STATE_RESUME_GAME;
}



void allegro_shutdown(void) 
{
	if (display)
	{
		al_destroy_display(display);
	}
}

