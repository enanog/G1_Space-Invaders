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
#include "enemyFont.h"
#include <math.h>

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_FONT *font = NULL;
ALLEGRO_BITMAP *background = NULL;

static char** readCreditsFile(int* line_count);
static gameState_t menuShow(ALLEGRO_DISPLAY *display);
static gameState_t mainMenu(void);
static gameState_t gameRender(gameState_t state, int enemyRow, int enemyCol);
static char keyboard_input(void);
static gameState_t pauseMenu(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *background);
static hitbox_t clipHitbox(hitbox_t hb, float margin_x, float margin_y, float inner_w, float inner_h);
static gameState_t showGameOver(void);
static bool isHighScore(int score, score_t topScores[], int count);
static gameState_t showScoreboard(void);
static gameState_t showCredits(void);

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

    font = al_load_ttf_font("assets/fonts/space-invaders-full-version.otf", 0.02f *al_get_display_height(display), 0);
    if (!font) {
        return false;
    }
    
    background = al_load_bitmap("assets/images/backgroundGame.png");
    if (!background) {
        fprintf(stderr, "Failed to load background image.\n");
        return false;
    }
	return true;
}

void gameLoop(void)
{
    gameState_t state = STATE_SPLASH;
    bool running = true;
    while (running) 
    {
        if(state != STATE_SPLASH)
        {
            
        }
        switch (state) 
        {
            case STATE_SPLASH: 
                playSound_playMusic(INTRO_MUSIC);
                state = mainMenu();
                break;

            case STATE_MENU:
                playSound_playMusic(INTRO_MUSIC);
                state = menuShow(display); // Devolvés un GameState
                break;

            case STATE_NEW_GAME:
                playSound_playMusic(GAME_MUSIC);
                printf("Starting new game...\n");
                state = gameRender(STATE_NEW_GAME, 3, 8);
                break;

            case STATE_RESUME_GAME:
                playSound_playMusic(GAME_MUSIC);
                printf("Resuming game...\n");
                state = gameRender(STATE_RESUME_GAME, -1, -1);
                break;

            case STATE_GAME_OVER:
                playSound_pauseMusic();
                playSound_play(SOUND_GAMEOVER);
                showGameOver();
                state = STATE_MENU;
                break;
            
            case STATE_SCOREBOARD:
                state = showScoreboard();
                break;

            case STATE_EXIT:
                running = false;
                break;

            case STATE_CREDITS:
                state = showCredits();
                break;  
            
            case STATE_PAUSE:
                playSound_pauseMusic();
                ALLEGRO_BITMAP *snapshot = al_create_bitmap(al_get_display_width(display), al_get_display_height(display));
                al_set_target_bitmap(snapshot);
                al_draw_bitmap(al_get_backbuffer(display), 0, 0, 0); // Captura del frame actual
                al_set_target_backbuffer(display);
                state = pauseMenu(display, snapshot);
                al_destroy_bitmap(snapshot);
                playSound_resumeMusic();
                break;

            default:
                running = false;
                break;
        }
    }

    al_destroy_font(font);
    playSound_stopMusic();
    playSound_shutdown();
}

static gameState_t mainMenu(void) 
{
    gameState_t state;
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));

    ALLEGRO_BITMAP *background = NULL;
    background = al_load_bitmap("assets/images/backgroundMenu.png");
    if (!background) {
        fprintf(stderr, "Failed to load background image.\n");
        return STATE_EXIT;
    }

    bool running = true;
    bool show_press_enter = false;
    double start_time = al_get_time();

    while (running) 
    {
        al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
                              0, 0, al_get_display_width(display), al_get_display_height(display), 0);

        // After 2 seconds, show "Press ENTER to start"
        if (al_get_time() - start_time > 2.0) 
        {
            show_press_enter = true;
        }
        if (show_press_enter) 
        {
            al_draw_text(font, al_map_rgb(255, 255, 255), al_get_display_width(display) * 0.45f, al_get_display_height(display) * 0.8, ALLEGRO_ALIGN_CENTER, "Press ENTER to start");
        }

        al_flip_display();

        ALLEGRO_EVENT event;
        if (al_wait_for_event_timed(queue, &event, 0.05)) 
        {
            if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) 
            {
                running = false;
                state = STATE_EXIT;
            }
            if (show_press_enter && event.keyboard.keycode == ALLEGRO_KEY_ENTER) 
            {
                running = false;
                state = STATE_MENU;
            }
        }
    }

    al_destroy_event_queue(queue);
    al_destroy_bitmap(background);
    return state;
}

static gameState_t menuShow(ALLEGRO_DISPLAY *display) 
{
    const char *options[] = {"Start Game", "Resume", "ScoreBoard", "Credits", "Exit"};
    int option_count = sizeof(options)/sizeof(options[0]);
    int selected = 0;

    score_t topScores[5];
    int topCount = getTopScore(topScores, 5); // Load top 5 scores

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool choosing = true;
    ALLEGRO_EVENT event;

    // Screen dimensions
    int screen_w = al_get_display_width(display)*0.92;
    int screen_h = al_get_display_height(display);

    float font_size = al_get_font_line_height(font);

    float title_y = screen_h * 0.07f;
    float option_spacing = screen_h * 0.08f;
    float menu_start_y = screen_h * 0.55f;

    // Puntajes rect: tamaño relativo y centrado arriba del menú
    float rect_width = screen_w * 0.3f;
    float rect_height = font_size * 1.2f + 5 * (font_size * 1.1f) + font_size * 0.8f;
    float rect_x = (screen_w - rect_width) / 2;
    float rect_y = menu_start_y - rect_height - screen_h * 0.05f;

    while (choosing) 
    {
        al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
                              0, 0, al_get_display_width(display), al_get_display_height(display), 0);
        // Draw title
        draw_title(screen_w / 2, title_y, display);

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
            snprintf(buffer, sizeof(buffer), "%d. %-15s\t%d", i + 1, topScores[i].name, topScores[i].score);

            al_draw_text(font, al_map_rgb(255, 255, 255),
                         rect_x + screen_w * 0.01f,
                         rect_y + font_size * 1.2f + i * (font_size * 1.1f),
                         0, buffer);
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
                    playSound_play(SOUND_MENU);
                    selected = (selected - 1 + option_count) % option_count;
                    break;
                case ALLEGRO_KEY_DOWN:
                    playSound_play(SOUND_MENU);
                    selected = (selected + 1) % option_count;
                    break;
                case ALLEGRO_KEY_ENTER:
                case ALLEGRO_KEY_SPACE:
                    printf("elegi");
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
	ALLEGRO_TIMER *timer = al_create_timer(1.0 / 1000.0);

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

    bool f3_pressed = false;
    bool b_pressed = false;

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
            state = STATE_EXIT;
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
			else if(event.keyboard.keycode == ALLEGRO_KEY_F4 && (event.keyboard.modifiers & ALLEGRO_KEYMOD_ALT)) 
			{ 
				running = false;
                allegro_shutdown();
                exit(0); // Exit the game when Alt+F4 is pressed
			}
            else if(event.keyboard.keycode == ALLEGRO_KEY_F3)
            {
                f3_pressed = !f3_pressed;
            }
            else if(event.keyboard.keycode == ALLEGRO_KEY_B)
            {
                b_pressed = !b_pressed;
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

        float margin_x = al_get_display_width(display) * 0.08f; // 8% horizontal
        float margin_y = al_get_display_height(display) * 0.10f; // 10% vertical
        float inner_w = al_get_display_width(display) - 3 * margin_x;
        float inner_h = al_get_display_height(display) - 2 * margin_y;

		if(redraw && al_is_event_queue_empty(queue))
		{
			al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
                                  0, 0, al_get_display_width(display), al_get_display_height(display), 0);

			int gameState = game_update(player);
            if(gameState == GAME_OVER && state == STATE_RESUME_GAME)
            {
                state = STATE_MENU;
                running = false;
                break;
            }
			if(gameState == GAME_OVER)
			{
				printf("GAME OVER\n");
				running = false;
                state = STATE_GAME_OVER;
                break;
			}

            draw_hearts(getPlayerLives(), display);
            
            draw_player_score(display, getScore());
            
            al_set_clipping_rectangle(margin_x, margin_y, inner_w, inner_h);
			
			for(row = 0; row < ENEMIES_ROW_MAX; row++)
			{
				for(col = 0; col < ENEMIES_COLUMNS_MAX; col++)
				{
					if(!getIsEnemyAlive(row,col)) 
                        continue;
					hitbox_t hitbox = clipHitbox(getEnemyPosition(row,col), margin_x, margin_y, inner_w, inner_h);
					ALLEGRO_COLOR color;
					switch (getEnemyTier(row))
					{
						case ALIEN_TIER1: color = al_map_rgb(255, 0, 0); break;
						case ALIEN_TIER2: color = al_map_rgb(0, 255, 0); break;
						case ALIEN_TIER3: color = al_map_rgb(0, 0, 255); break;
						default: break;
					}
                    draw_invaders(hitbox, row, display);
                    if(f3_pressed && b_pressed)
					    al_draw_rectangle(hitbox.start.x, hitbox.start.y, hitbox.end.x, hitbox.end.y, color, 2.0f);
				}
			}

			hitbox_t hitboxPlayer = clipHitbox(getPlayerPosition(), margin_x, margin_y, inner_w, inner_h);
            if(f3_pressed && b_pressed)
			    al_draw_rectangle(hitboxPlayer.start.x, hitboxPlayer.start.y, hitboxPlayer.end.x, hitboxPlayer.end.y, al_map_rgb(0,255,0), 2.0f);
            draw_player(hitboxPlayer, display);

			for (int barrier = 0; barrier < BARRIER_QUANTITY_MAX; barrier++) 
			{
				for (row = 0; row < BARRIER_ROWS; row++)
				{
					for (col = 0; col < BARRIER_COLUMNS; col++) 
					{
						if (!getBarrierIsAlive(barrier, row, col)) 
                        {
                            continue;
                        }
						hitbox_t hitboxBarrier = clipHitbox(getBarrierPosition(barrier, row, col), margin_x, margin_y, inner_w, inner_h);
                        //if(f3_pressed && b_pressed)
                        al_draw_rectangle(hitboxBarrier.start.x, hitboxBarrier.start.y, hitboxBarrier.end.x, hitboxBarrier.end.y, al_map_rgb(0,255,0), 2.0f);
					}
				}
			}

			bullet_t bullet = getPlayerBulletinfo();
			if (bullet.active) 
            {
                hitbox_t bulletHitbox = clipHitbox(bullet.hitbox, margin_x, margin_y, inner_w, inner_h);
                draw_bullet(bulletHitbox, display);
                if(f3_pressed && b_pressed)
				    al_draw_rectangle(bulletHitbox.start.x, bulletHitbox.start.y, bulletHitbox.end.x, bulletHitbox.end.y, al_map_rgb(255, 255, 0), 2.0f);
			}

			bullet_t enemyBulletBitMap[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];
			getEnemiesBulletsInfo(enemyBulletBitMap);
			for(row = 0; row < ENEMIES_ROW_MAX; row++)
			{
				for(col = 0; col < ENEMIES_COLUMNS_MAX; col++)
				{
					if(!enemyBulletBitMap[row][col].active) continue;
                    hitbox_t enemyBulletHitbox = clipHitbox(enemyBulletBitMap[row][col].hitbox, margin_x, margin_y, inner_w, inner_h);
                    draw_bullet(enemyBulletHitbox, display);
                    if(f3_pressed && b_pressed)
					    al_draw_rectangle(enemyBulletHitbox.start.x, enemyBulletHitbox.start.y, enemyBulletHitbox.end.x, enemyBulletHitbox.end.y, al_map_rgb(255, 255, 0), 2.0f);
				}
			}

            hitbox_t mothershipHitbox = clipHitbox(getMothershipPosition(), margin_x, margin_y, inner_w, inner_h);
            if(getIsMothershipAlive())
            {
                draw_mothership(mothershipHitbox, display);
                if(f3_pressed && b_pressed) 
                    al_draw_rectangle(mothershipHitbox.start.x, mothershipHitbox.start.y, mothershipHitbox.end.x, mothershipHitbox.end.y, al_map_rgb(255, 255, 0), 2.0f);
            }

            al_reset_clipping_rectangle();
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
    gameState_t state = STATE_RESUME_GAME;
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

    while (choosing) 
    {
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
            state = STATE_RESUME_GAME;
            break;
        case 1: 
            state = STATE_NEW_GAME;
            break;
        case 2: 
            state = STATE_MENU;
            break;
        case 3: 
            state = STATE_EXIT;
            break;
    }

    return state;
}

static gameState_t showScoreboard(void) 
{
    const int SCORES_PER_PAGE = 10;
    const int MAX_SCORES = 100;
    score_t scores[MAX_SCORES];
    int total_scores = getTopScore(scores, MAX_SCORES);
    int current_page = 0;
    int total_pages = (total_scores + SCORES_PER_PAGE - 1) / SCORES_PER_PAGE;

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));

    bool viewing = true;
    ALLEGRO_EVENT event;

    // Load arrow bitmap
    ALLEGRO_BITMAP *arrow = al_load_bitmap("assets/images/arrows.png");
    if (!arrow) {
        fprintf(stderr, "Failed to load arrow images\n");
        // Handle error - maybe use text arrows instead
    }

    // Arrow hitboxes
    hitbox_t left_arrow_hb = {
        .start = {.x = 0.05f, .y = 0.85f},
        .end = {.x = 0.10f, .y = 0.95f}
    };
    hitbox_t right_arrow_hb = {
        .start = {.x = 0.90f, .y = 0.85f},
        .end = {.x = 0.95f, .y = 0.95f}
    };

    while (viewing) 
    {
        al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
                              0, 0, al_get_display_width(display), al_get_display_height(display), 0);

        // Draw semi-transparent overlay
        al_draw_filled_rectangle(0, 0, al_get_display_width(display), al_get_display_height(display),
                                al_map_rgba(0, 0, 0, 200));

        // Draw scoreboard background
        float board_width = al_get_display_width(display) * 0.8f;
        float board_height = al_get_display_height(display) * 0.7f;
        float board_x = (al_get_display_width(display) - board_width) / 2;
        float board_y = (al_get_display_height(display) - board_height) / 2;

        al_draw_filled_rectangle(board_x, board_y, board_x + board_width, board_y + board_height,
                                al_map_rgb(50, 50, 80));
        al_draw_rectangle(board_x, board_y, board_x + board_width, board_y + board_height,
                          al_map_rgb(255, 255, 255), 3);

        // Draw title
        al_draw_text(font, al_map_rgb(255, 255, 0),
                     al_get_display_width(display) / 2,
                     board_y + 20,
                     ALLEGRO_ALIGN_CENTER, "TOP SCORES");

        // Draw page info
        char page_info[32];
        snprintf(page_info, sizeof(page_info), "Page %d/%d", current_page + 1, total_pages);
        al_draw_text(font, al_map_rgb(200, 200, 200),
                     al_get_display_width(display) / 2,
                     board_y + 60,
                     ALLEGRO_ALIGN_CENTER, page_info);

        // Draw scores for current page
        int start_idx = current_page * SCORES_PER_PAGE;
        int end_idx = start_idx + SCORES_PER_PAGE;
        if (end_idx > total_scores) end_idx = total_scores;

        for (int i = start_idx; i < end_idx; i++) {
            float y_pos = board_y + 100 + (i - start_idx) * 40;
            
            // Rank
            char rank[16];
            snprintf(rank, sizeof(rank), "%d.", i + 1);
            al_draw_text(font, al_map_rgb(255, 255, 255),
                         board_x + 20,
                         y_pos,
                         ALLEGRO_ALIGN_LEFT, rank);

            // Name
            al_draw_text(font, al_map_rgb(255, 255, 255),
                         board_x + 90,
                         y_pos,
                         ALLEGRO_ALIGN_LEFT, scores[i].name);

            // Score
            char score[16];
            snprintf(score, sizeof(score), "%d", scores[i].score);
            al_draw_text(font, al_map_rgb(255, 255, 255),
                         board_x + board_width - 50,
                         y_pos,
                         ALLEGRO_ALIGN_RIGHT, score);
        }

        // Tamaños comunes para las flechas
        float display_w = al_get_display_width(display);
        float display_h = al_get_display_height(display);

        // Draw left arrow (flipped horizontally)
        if (current_page > 0 && arrow) {
            float arrow_w = (left_arrow_hb.end.x - left_arrow_hb.start.x) * display_w;
            float arrow_h = (left_arrow_hb.end.y - left_arrow_hb.start.y) * display_h;
            float draw_x = left_arrow_hb.start.x * display_w;
            float draw_y = left_arrow_hb.start.y * display_h;

            al_draw_scaled_bitmap(
                arrow,
                0, 0,
                al_get_bitmap_width(arrow), al_get_bitmap_height(arrow),
                draw_x + arrow_w, draw_y,  // Position adjusted by arrow_w
                -arrow_w,  // Negative width flips horizontally
                arrow_h,
                0
            );
        }

        // Draw right arrow (normal)
        if (current_page < total_pages - 1 && arrow) {
            float arrow_w = (right_arrow_hb.end.x - right_arrow_hb.start.x) * display_w;
            float arrow_h = (right_arrow_hb.end.y - right_arrow_hb.start.y) * display_h;
            float draw_x = right_arrow_hb.start.x * display_w;
            float draw_y = right_arrow_hb.start.y * display_h;

            al_draw_scaled_bitmap(
                arrow,
                0, 0,
                al_get_bitmap_width(arrow), al_get_bitmap_height(arrow),
                draw_x, draw_y,
                arrow_w,
                arrow_h,
                0
            );
        }


        // Draw return instruction
        al_draw_text(font, al_map_rgb(200, 200, 200),
                     al_get_display_width(display) / 2,
                     al_get_display_height(display) * 0.95f,
                     ALLEGRO_ALIGN_CENTER, "Press ESC to return to menu");

        al_flip_display();

        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            viewing = false;
            allegro_shutdown();
            exit(0);
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_ESCAPE:
                    viewing = false;
                    break;
                case ALLEGRO_KEY_LEFT:
                    if (current_page > 0) {
                        current_page--;
                        playSound_play(SOUND_MENU);
                    }
                    break;
                case ALLEGRO_KEY_RIGHT:
                    if (current_page < total_pages - 1) {
                        current_page++;
                        playSound_play(SOUND_MENU);
                    }
                    break;
            }
        }
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            float mouse_x = event.mouse.x / (float)al_get_display_width(display);
            float mouse_y = event.mouse.y / (float)al_get_display_height(display);

            // Check if left arrow was clicked
            if (current_page > 0 && 
                mouse_x >= left_arrow_hb.start.x && mouse_x <= left_arrow_hb.end.x &&
                mouse_y >= left_arrow_hb.start.y && mouse_y <= left_arrow_hb.end.y) {
                current_page--;
                playSound_play(SOUND_MENU);
            }
            // Check if right arrow was clicked
            else if (current_page < total_pages - 1 &&
                     mouse_x >= right_arrow_hb.start.x && mouse_x <= right_arrow_hb.end.x &&
                     mouse_y >= right_arrow_hb.start.y && mouse_y <= right_arrow_hb.end.y) {
                current_page++;
                playSound_play(SOUND_MENU);
            }
        }
    }

    // Cleanup
    al_destroy_event_queue(queue);
    if (arrow) al_destroy_bitmap(arrow);

    return STATE_MENU;
}

static char** readCreditsFile(int* line_count) {
    FILE* file = fopen("data/credits.txt", "r");
    if (!file) {
        fprintf(stderr, "Failed to load credits file\n");
        return NULL;
    }

    // First count the lines
    int count = 0;
    char ch;
    while (!feof(file)) {
        ch = fgetc(file);
        if (ch == '\n') count++;
    }
    count++; // For last line without newline
    rewind(file);

    // Allocate memory
    char** lines = (char**)malloc(count * sizeof(char*));
    if (!lines) {
        fclose(file);
        return NULL;
    }

    // Read each line
    char buffer[256];
    int i = 0;
    while (fgets(buffer, sizeof(buffer), file) && i < count) {
        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;
        
        lines[i] = strdup(buffer);
        if (!lines[i]) {
            // Cleanup if allocation fails
            for (int j = 0; j < i; j++) free(lines[j]);
            free(lines);
            fclose(file);
            return NULL;
        }
        i++;
    }
    *line_count = i;
    fclose(file);
    return lines;
}
static gameState_t showCredits(void)
{
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));

    // Load credits from file
    int credit_count = 0;
    char** credits = readCreditsFile(&credit_count);
    if (!credits) {
        // Fallback to hardcoded credits if file fails
        const char* fallback_credits[] = {
            "CREDITS FILE MISSING!",
            " ",
            "This is awkward...",
            "We seem to have misplaced our credits file",
            " ",
            "Please imagine something awesome here",
            " ",
            "PRESS ANY KEY TO RETURN",
            NULL
        };

        // Convert to same format as file reading
        credit_count = 0;
        while (fallback_credits[credit_count]) credit_count++;

        credits = (char**)malloc((credit_count+1) * sizeof(char*));
        for (int i = 0; i < credit_count; i++) {
            credits[i] = strdup(fallback_credits[i]);
        }
        credits[credit_count] = NULL;
    }

    // Load background
    ALLEGRO_BITMAP *credits_bg = al_load_bitmap("assets/images/credits_bg.png");
    if (!credits_bg) {
        credits_bg = al_load_bitmap("assets/images/backgroundMenu.png");
    }

    float scroll_pos = al_get_display_height(display);
    bool credits_done = false;
    ALLEGRO_EVENT event;
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

    bool shown_segfault_joke = false;

    while (!credits_done)
    {
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            credits_done = true;
            allegro_shutdown();
            exit(0);
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN ||
            event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            credits_done = true;
            }
            else if (event.type == ALLEGRO_EVENT_TIMER) {
                scroll_pos -= 0.7f;

                // Check for segfault joke position
                if (!shown_segfault_joke && scroll_pos < -1500.0f) {
                    for (int i = 0; i < credit_count; i++) {
                        if (strstr(credits[i], "*segmentation fault*")) {
                            playSound_play(SOUND_EXPLOSION);
                            shown_segfault_joke = true;
                            break;
                        }
                    }
                }

                if (scroll_pos < -2500.0f) {
                    credits_done = true;
                }
            }

            // Drawing
            if (credits_bg) {
                al_draw_scaled_bitmap(credits_bg, 0, 0,
                                      al_get_bitmap_width(credits_bg),
                                      al_get_bitmap_height(credits_bg),
                                      0, 0, al_get_display_width(display),
                                      al_get_display_height(display), 0);
            } else {
                al_clear_to_color(al_map_rgb(0, 0, 0));
            }

            // Draw credits
            float y = scroll_pos;
            for (int i = 0; i < credit_count; i++) {
                ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
                int font_size = al_get_font_line_height(font);
                float x = al_get_display_width(display) / 2;

                // Apply special formatting
                if (strstr(credits[i], "SPACE INVADERS")) {
                    color = al_map_rgb(0, 255, 255);
                    font_size *= 1.5;
                }
                else if (strstr(credits[i], "THE MOST AWESOME") ||
                    strstr(credits[i], "FEARLESS LEADERS") ||
                    strstr(credits[i], "SPECIAL THANKS") ||
                    strstr(credits[i], "FUN FACT") ||
                    strstr(credits[i], "DISCLAIMER") ||
                    strstr(credits[i], "MEMORY LEAK") ||
                    strstr(credits[i], "SEGMENTATION")) {
                    color = al_map_rgb(255, 255, 0);
                }
                else if (strstr(credits[i], "PRESS ANY KEY") ||
                    strstr(credits[i], "segmentation fault")) {
                    color = al_map_rgb(255, 0, 0);
                }
                al_draw_text(font, color, x, y, ALLEGRO_ALIGN_CENTER, credits[i]);
                y += font_size * 1.2;
            }
            al_flip_display();
    }

    // Cleanup
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    if (credits_bg) al_destroy_bitmap(credits_bg);

    // Free credits lines
    for (int i = 0; i < credit_count; i++) {
        free(credits[i]);
    }
    free(credits);

    return STATE_MENU;
}

void allegro_shutdown(void) 
{
	if (display)
	{
		al_destroy_display(display);
	}
}

static bool isHighScore(int score, score_t topScores[], int count) 
{
    if (count < 10) 
        return true;
    return score > topScores[count - 1].score;
}

static gameState_t showGameOver(void)
{
    score_t topScores[100];
    int count = getTopScore(topScores, 100); // cargar top 10
    
    bool highscore = isHighScore(getScore(), topScores, count);
    
    char name[15] = "";
    int name_pos = 0;

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());

    bool done = false;
    ALLEGRO_EVENT ev;

    while (!done) {
        al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
                                  0, 0, al_get_display_width(display), al_get_display_height(display), 0);


        // Título
        al_draw_text(font, al_map_rgb(255, 255, 255),
                     al_get_display_width(display)/2,
                     al_get_display_height(display) * 0.1f,
                     ALLEGRO_ALIGN_CENTER, "GAME OVER");

        // Puntaje
        char scoreText[64];
        snprintf(scoreText, sizeof(scoreText), "Your score: %d", getScore());
        al_draw_text(font, al_map_rgb(255, 255, 0),
                     al_get_display_width(display)/2,
                     al_get_display_height(display) * 0.2f,
                     ALLEGRO_ALIGN_CENTER, scoreText);

        // TOP 10
        al_draw_text(font, al_map_rgb(255, 255, 255),
                     al_get_display_width(display)/2,
                     al_get_display_height(display) * 0.3f,
                     ALLEGRO_ALIGN_CENTER, "Top Scores:");

        for (int i = 0; i < 10; ++i) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%d. %-15s - %d", i + 1, topScores[i].name, topScores[i].score);
            al_draw_text(font, al_map_rgb(255, 255, 255),
                         al_get_display_width(display)/2,
                         al_get_display_height(display) * (0.35f + i * 0.05f),
                         ALLEGRO_ALIGN_CENTER, buffer);
        }

        // Si entra en top
        if (highscore) {
            al_draw_text(font, al_map_rgb(0, 255, 0),
                         al_get_display_width(display)/2,
                         al_get_display_height(display) * 0.85f,
                         ALLEGRO_ALIGN_CENTER, "New High Score! Enter your name:");

            al_draw_text(font, al_map_rgb(255, 255, 255),
                         al_get_display_width(display)/2,
                         al_get_display_height(display) * 0.9f,
                         ALLEGRO_ALIGN_CENTER, name);
        } else {
            al_draw_text(font, al_map_rgb(255, 0, 0),
                         al_get_display_width(display)/2,
                         al_get_display_height(display) * 0.85f,
                         ALLEGRO_ALIGN_CENTER, "Press ENTER to return to menu");
        }

        al_flip_display();

        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_KEY_CHAR && name_pos < 14) {
            if (ev.keyboard.unichar >= 32 && ev.keyboard.unichar <= 126) {
                name[name_pos++] = ev.keyboard.unichar;
                name[name_pos] = '\0';
            } 
        }
        if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && name_pos > 0) {
            name[--name_pos] = '\0';
        }

        if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
            if (!highscore || (highscore && name_pos > 0)) {
                if (highscore)
                    topScoreUpdate(getScore(), name); // Agregás el nuevo score

                done = true;
            }
        }
    }

    al_destroy_event_queue(queue);
    return STATE_MENU;
}

static hitbox_t clipHitbox(hitbox_t hb, float margin_x, float margin_y, float inner_w, float inner_h)
{
    hitbox_t result;

    result.start.x = margin_x + hb.start.x * inner_w;
    result.start.y = margin_y + hb.start.y * inner_h;

    result.end.x   = margin_x + hb.end.x * inner_w;
    result.end.y   = margin_y + hb.end.y * inner_h;

    return result;
} 
