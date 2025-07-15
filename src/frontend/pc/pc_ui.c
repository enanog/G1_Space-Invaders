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
 * 	MAGLIOLA, Nicolas
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
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "game.h"
#include "config.h"
#include "playSound.h"
#include "entity.h"
#include "pc_ui.h"
#include "score.h"
#include "entitiesFont.h"

/* ======================== CONSTANTS ======================== */
//Defines
#define ENEMY_ROW 5
#define ENEMY_COL 6

/* ======================== STATIC VARIABLES ======================== */
// Global Allegro resources
static ALLEGRO_DISPLAY *display = NULL;
static ALLEGRO_FONT *font = NULL;
static ALLEGRO_BITMAP *background = NULL;

/* ======================== FORWARD DECLARATIONS ======================== */
static gameState_t menuShow(ALLEGRO_DISPLAY *display);
static gameState_t mainMenu(ALLEGRO_DISPLAY *display);
static gameState_t gameRender(gameState_t state, int enemyRow, int enemyCol);
static gameState_t pauseMenu(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *background);
static hitbox_t clipHitbox(hitbox_t hb, float margin_x, float margin_y, float inner_w, float inner_h);
static gameState_t showGameOver(ALLEGRO_DISPLAY *display);
static bool isHighScore(int score, score_t topScores[], int count);
static gameState_t showScoreboard(ALLEGRO_DISPLAY *display);
static gameState_t showCredits(ALLEGRO_DISPLAY *display);

/* ======================== HELPER FUNCTION DECLARATIONS ======================== */
static gameState_t event_handle(ALLEGRO_EVENT_QUEUE *queque, ALLEGRO_EVENT *event, bool *running, ALLEGRO_DISPLAY *display);
static void draw_centered_text(float x, float y, const char *text, ALLEGRO_COLOR color);
static void draw_menu_options(const char **options, int count, int selected, float start_y, float spacing);
static void setup_event_queue(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_DISPLAY *disp);
static void draw_scaled_background(ALLEGRO_BITMAP *bg);
static bool draw_game_entities(float margin_x, float margin_y, float inner_w, float inner_h, bool show_hitboxes);
static void draw_barriers(int barrier, float margin_x, float margin_y, float inner_w, float inner_h, bool show_hitboxes);
static char** readCreditsFile(int* line_count);


/* ======================== PUBLIC FUNCTIONS ======================== */
bool allegro_init(void) 
{
	const bool init_success = 
		al_init() && 
		al_init_font_addon() && 
		al_init_ttf_addon() &&
		al_init_primitives_addon() && 
		al_init_image_addon() &&
		al_install_keyboard() &&
		al_install_mouse(); 

	if (!init_success) return false;

	display = al_create_display(SCREEN_W, SCREEN_H);
	if (!display) return false;

	const int font_size = 0.02f * al_get_display_height(display);
	font = al_load_ttf_font("assets/fonts/space-invaders-full-version.otf", font_size, 0);
	if (!font) return false;
	
	background = al_load_bitmap("assets/images/backgroundGame.png");
	if (!background) {
		fprintf(stderr, "Failed to load background image\n");
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
		switch (state) 
		{
			case STATE_SPLASH: 
				playSound_playMusic(INTRO_MUSIC);
				state = mainMenu(display);
				break;

			case STATE_MENU:
				playSound_playMusic(INTRO_MUSIC);
				state = menuShow(display);
				break;

			case STATE_NEW_GAME:
				playSound_playMusic(GAME_MUSIC);
				state = gameRender(STATE_NEW_GAME, ENEMY_ROW, ENEMY_COL);
				break;

			case STATE_RESUME_GAME:
				playSound_playMusic(GAME_MUSIC);
				state = gameRender(STATE_RESUME_GAME, -1, -1);
				break;

			case STATE_GAME_OVER:
				playSound_pauseMusic();
				playSound_play(SOUND_GAMEOVER);
				state = showGameOver(display);
				break;
			
			case STATE_SCOREBOARD:
				state = showScoreboard(display);
				break;

			case STATE_EXIT:
				running = false;
				break;

			case STATE_CREDITS:
				state = showCredits(display);
				break;  
			
			case STATE_PAUSE:
				playSound_pauseMusic();
                 // Create snapshot of current game state for pause menu
				ALLEGRO_BITMAP *snapshot = al_create_bitmap(
					al_get_display_width(display), 
					al_get_display_height(display)
				);
				al_set_target_bitmap(snapshot);
				al_draw_bitmap(al_get_backbuffer(display), 0, 0, 0);
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

	// Cleanup
	playSound_stopMusic();
	playSound_shutdown();
}

void allegro_shutdown(void) 
{
	if (display) al_destroy_display(display);
	if (font) al_destroy_font(font);
	if (background) al_destroy_bitmap(background);
}

/* ======================== PRIVATE FUNCTIONS ======================== */

static gameState_t event_handle(ALLEGRO_EVENT_QUEUE *queque, ALLEGRO_EVENT *event, bool *running, ALLEGRO_DISPLAY *display)
{
	static bool fullscreen = false;
	switch (event->type)
	{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			*running = false;
			return STATE_EXIT;
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			switch (event->keyboard.keycode) 
			{
				case ALLEGRO_KEY_F11:
					fullscreen = !fullscreen;
					al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
					al_resize_display(display, SCREEN_W, SCREEN_H);
					const int font_size = 0.02f * al_get_display_height(display);
					al_destroy_font(font);
					font = al_load_ttf_font("assets/fonts/space-invaders-full-version.otf", font_size, 0);
					break;
				case ALLEGRO_KEY_F4:
					if(event->keyboard.modifiers & ALLEGRO_KEYMOD_ALT)
					{
						*running = false;
						return STATE_EXIT;
					}
					break;
			}
			break;
		default:
			break;
	}
	return 0;
}

/* ---------------------------------------------------
 * @brief Main menu game loop handling state transitions
 * ---------------------------------------------------*/
static gameState_t mainMenu(ALLEGRO_DISPLAY *display) 
{
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	ALLEGRO_BITMAP *bg = al_load_bitmap("assets/images/backgroundMenu.png");
	if (!bg) {
		al_destroy_event_queue(queue);
		return STATE_EXIT;
	}

	bool running = true;
	bool show_prompt = false;
	const double start_time = al_get_time();
	gameState_t next_state = STATE_EXIT;

	while (running) 
	{
		draw_scaled_background(bg);

		// Show "Press ENTER" after 2 seconds
		if (al_get_time() - start_time > 2.0) show_prompt = true;
		if (show_prompt) {
			draw_centered_text(0.45f, 0.8f, "Press ENTER to start", al_map_rgb(255, 255, 255));
		}

		al_flip_display();

		ALLEGRO_EVENT event;
		if (al_wait_for_event_timed(queue, &event, 0.05)) 
		{
			if(event_handle(queue, &event, &running, display) == STATE_EXIT)
				next_state = STATE_EXIT;
			else if (show_prompt && event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
				running = false;
				next_state = STATE_MENU;
			}
		}
	}

	al_destroy_event_queue(queue);
	al_destroy_bitmap(bg);
	return next_state;
}

/* ---------------------------------------------------
 * @brief Displays and handles the main menu interface
 * 
 * Renders the game's main menu with options, scoreboard, 
 * and processes user navigation input. Returns the next 
 * game state based on selection.
 * 
 * @param display Pointer to the Allegro display window
 * @return Next game state (gameState_t enum)
 * 
 * Menu Options:
 * - Start Game: Begins new game (STATE_NEW_GAME)
 * - Resume: Continues saved game (STATE_RESUME_GAME)
 * - ScoreBoard: Shows high scores (STATE_SCOREBOARD)
 * - Credits: Displays credits screen (STATE_CREDITS)
 * - Exit: Quits application (STATE_EXIT)
 * ---------------------------------------------------*/
static gameState_t menuShow(ALLEGRO_DISPLAY *display) 
{
	gameState_t next_state;
	const char *options[] = {"Start Game", "Resume", "ScoreBoard", "Credits", "Exit"};
	const int option_count = sizeof(options)/sizeof(options[0]);
	int selected = 0;

	score_t topScores[5];
	const int topCount = getTopScore(topScores, 5);

	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	bool choosing = true;

	while (choosing) 
	{
		float screen_w = al_get_display_width(display);
		float screen_h = al_get_display_height(display);

		float title_y = 0.07f;
		float menu_start_y = 0.55f;
		float option_spacing = 0.08f;

		float rect_width = 0.3f;
		float font_size = al_get_font_line_height(font);
		float rect_width_px = 0.3f * screen_w;
		float rect_height = font_size * 1.2f + 5 * (font_size * 1.1f) + font_size * 0.8f;
		float rect_x = (screen_w - rect_width_px) / 2.0f;
		float rect_y = 0.55f * screen_h - rect_height - 0.05f * screen_h;
		draw_scaled_background(background);

		// Draw title
		draw_title(screen_w / 2, title_y * screen_h, display);

		// Draw score box
		al_draw_filled_rectangle(rect_x, rect_y, rect_x + rect_width * screen_w, 
								rect_y + rect_height, al_map_rgb(100, 100, 100));
		al_draw_rectangle(rect_x, rect_y, rect_x + rect_width * screen_w, 
						  rect_y + rect_height, al_map_rgb(255, 255, 255), 2);

		// Draw scores title
		draw_centered_text((rect_x + rect_width_px / 2.0f) / screen_w, 
						   (rect_y + font_size * 0.2f) / screen_h,
						   "TOP SCORES", al_map_rgb(255, 255, 255));

		// Draw scores 
		for (int i = 0; i < 5 && i < topCount; ++i) 
		{
			char buffer[64];
			snprintf(buffer, sizeof(buffer), "%d. %-15s\t%d", i + 1, topScores[i].name, topScores[i].score);
			al_draw_text(font, al_map_rgb(255, 255, 255), rect_x + 0.01f * screen_w,
						 rect_y + font_size * 1.2f + i * (font_size * 1.1f), 0, buffer);
		}

		// Draw menu options
		draw_menu_options(options, option_count, selected, 
						 menu_start_y * screen_h, 
						 option_spacing * screen_h);

		al_flip_display();

		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);
		if(event_handle(queue, &event, &choosing, display) == STATE_EXIT)
				next_state = STATE_EXIT;

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
					choosing = false;
					break;
			}
		}
	}

	al_destroy_event_queue(queue);

	if(next_state == STATE_EXIT)
		return STATE_EXIT;
	// Map selection to game state
	switch (selected) 
	{
		case 0: next_state = STATE_NEW_GAME; break;
		case 1: next_state = STATE_RESUME_GAME; break;
		case 2: next_state = STATE_SCOREBOARD; break;
		case 3: next_state = STATE_CREDITS; break;
		case 4: next_state = STATE_EXIT; break;
		default: next_state = STATE_MENU; break;
	}
	return next_state;
}

/* ---------------------------------------------------
 * @brief Show splash screen and wait for user input
 * @return Next game state (STATE_MENU or STATE_EXIT)
 * ---------------------------------------------------*/
static gameState_t gameRender(gameState_t state, int enemyRow, int enemyCol)
{
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	ALLEGRO_TIMER *timer = al_create_timer(1.0 / 1000);//1000 fps 
	setup_event_queue(queue, display);
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_register_event_source(queue, al_get_mouse_event_source());
	al_start_timer(timer);

	// Initialize game state
	if (state == STATE_NEW_GAME) 
	{
		game_init(enemyRow, enemyCol, false);
	} 
	else if (state == STATE_RESUME_GAME) 
	{
		if(!game_init(enemyRow, enemyCol, true))
		{
			return STATE_MENU;
		}
	}

	input_t player = {0};
	bool show_hitboxes = false;
	bool f3_pressed = false, b_pressed = false, flag_hitboxes = true;

	bool running = true;
	bool redraw = true;
	gameState_t next_state = state;
    bool firstUpdate = false;

	while (running)
	{   
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);

		if(event_handle(queue, &event, &running, display) == STATE_EXIT)
				next_state = STATE_EXIT;
		
		switch (event.type)
		{
			case ALLEGRO_EVENT_TIMER:
				redraw = true;
				break;

			case ALLEGRO_EVENT_KEY_DOWN:
				switch (event.keyboard.keycode) 
				{
					case ALLEGRO_KEY_A:
					case ALLEGRO_KEY_LEFT:  player.direction = -1; break;
					case ALLEGRO_KEY_D:
					case ALLEGRO_KEY_RIGHT: player.direction = 1; break;
					case ALLEGRO_KEY_SPACE: player.shot = true; break;
					case ALLEGRO_KEY_ESCAPE: 
						player.pause = !player.pause;
						if (player.pause) 
						{
							game_update(player);
							next_state = STATE_PAUSE;
							running = false;
						}
						break;
					case ALLEGRO_KEY_F3: f3_pressed = true; break;
					case ALLEGRO_KEY_B: b_pressed = true; break;
				}
				break;

			case ALLEGRO_EVENT_KEY_UP:
				switch (event.keyboard.keycode) 
				{
					case ALLEGRO_KEY_A:
					case ALLEGRO_KEY_D:
					case ALLEGRO_KEY_LEFT:
					case ALLEGRO_KEY_RIGHT: player.direction = 0; break;
					case ALLEGRO_KEY_SPACE: player.shot = false; break;
					case ALLEGRO_KEY_F3: 
						f3_pressed = false; 
						flag_hitboxes = true;
						break;
					case ALLEGRO_KEY_B: b_pressed = false; break;
				}
				break;
		}

		// Toggle hitboxes if F3+B pressed
		if (f3_pressed && b_pressed && flag_hitboxes) 
		{
			show_hitboxes = !show_hitboxes;
			flag_hitboxes = false;
		}

		if (redraw && al_is_event_queue_empty(queue))
		{
			redraw = false;
			draw_scaled_background(background);

			// Game update and state check
			int game_status = game_update(player);
			if (game_status == GAME_OVER) 
			{
				running = false;
                if(!firstUpdate)
                {
				    next_state = (state == STATE_RESUME_GAME) ? STATE_MENU : STATE_GAME_OVER;
                }
                else
                    next_state = STATE_GAME_OVER;
				break;
			}
            firstUpdate = true;
			// Draw UI elements
			draw_hearts(getPlayerLives(), display);
			draw_player_score(display, getScore());

			// Set up clipping area
			float margin_x = al_get_display_width(display) * 0.08f;
			float margin_y = al_get_display_height(display) * 0.10f;
			float inner_w = al_get_display_width(display) - 3 * margin_x;
			float inner_h = al_get_display_height(display) - 2 * margin_y;
			al_set_clipping_rectangle(margin_x, margin_y, inner_w, inner_h);

			// Draw game entities
			player.pause = draw_game_entities(margin_x, margin_y, inner_w, inner_h, show_hitboxes);

			al_reset_clipping_rectangle();

			al_flip_display();
		}
	}

	al_destroy_timer(timer);
	al_destroy_event_queue(queue);
	return next_state;
}

/* ---------------------------------------------------
 * @brief Draw all game entities with optional hitboxes
 * @param margin_x X margin for clipping
 * @param margin_y Y margin for clipping
 * @param inner_w Inner width for clipping
 * @param inner_h Inner height for clipping
 * @param show_hitboxes Whether to render hitboxes
 * ---------------------------------------------------*/
static bool draw_game_entities(float margin_x, float margin_y, float inner_w, float inner_h, bool show_hitboxes)
{
	static int playerLives = 3;
	static long long playerDieTime;
	static bool flag = false;
	bool gamePaused = false;

	long long currentTime = getTimeMillis();
	// Draw player
	hitbox_t player_hb = clipHitbox(getPlayerPosition(), margin_x, margin_y, inner_w, inner_h);
	
	if(playerLives > getPlayerLives())
	{
		playerDieTime = getTimeMillis();
		draw_player_died(player_hb, display, currentTime - playerDieTime);
		gamePaused = true;
		flag = true;
	}
	else if(currentTime - playerDieTime < 500 && flag)
	{
		draw_player_died(player_hb, display, currentTime - playerDieTime);
		gamePaused = true;
	}
	else
	{
		draw_player(player_hb, display);
		playerDieTime = currentTime;
		gamePaused = false;
		flag = false;
	}

	playerLives = getPlayerLives();
	
	if (show_hitboxes) 
		al_draw_rectangle(player_hb.start.x, player_hb.start.y, player_hb.end.x, player_hb.end.y, 
						  al_map_rgb(0,255,0), 2.0f);

	// Draw enemies
	static bool enemyAlive[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];

	// Aca guardo una matriz de diferenciales de tiempo
	static long long explosionEnemyTime[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX] = {0};

	int row, col;
	for (row = 0; row < ENEMIES_ROW_MAX; row++) 
	{
		for (col = 0; col < ENEMIES_COLUMNS_MAX; col++) 
		{
			hitbox_t hitbox = clipHitbox(getEnemyPosition(row,col), margin_x, margin_y, inner_w, inner_h);
			if (!getIsEnemyAlive(row,col))
			{
				long long currentTime = getTimeMillis();

				if(enemyAlive[row][col])
				{
					draw_explosion(hitbox, display, 0);
					enemyAlive[row][col] = 0;
					explosionEnemyTime[row][col] = currentTime;
				}
				else if(currentTime - explosionEnemyTime[row][col] < 150 && explosionEnemyTime[row][col] != 0)
					draw_explosion(hitbox, display, 0);
				else
					explosionEnemyTime[row][col] = 0;

				continue;

			}
			ALLEGRO_COLOR color;
			switch (getEnemyTier(row)) 
			{
				case ALIEN_TIER1: color = al_map_rgb(255, 0, 0); break;
				case ALIEN_TIER2: color = al_map_rgb(0, 255, 0); break;
				case ALIEN_TIER3: color = al_map_rgb(0, 0, 255); break;
				default: break;
			}
			draw_invaders(hitbox, row, display);
			if (show_hitboxes) 
			{
				al_draw_rectangle(hitbox.start.x, hitbox.start.y, hitbox.end.x, hitbox.end.y, color, 2.0f);
			}
			enemyAlive[row][col] = 1;
		}
	}
	

	// Draw barriers
	int barrier;
	for (barrier = 0; barrier < BARRIER_QUANTITY_MAX; barrier++)
		draw_barriers(barrier, margin_x, margin_y, inner_w, inner_h, show_hitboxes);

	// Draw player bullet
	bullet_t bullet = getPlayerBulletinfo();
	if (bullet.active) 
	{
		hitbox_t bullet_hb = clipHitbox(bullet.hitbox, margin_x, margin_y, inner_w, inner_h);
		draw_bullet(bullet_hb, display);
		if (show_hitboxes) 
			al_draw_rectangle(bullet_hb.start.x, bullet_hb.start.y, bullet_hb.end.x, bullet_hb.end.y, 
							 al_map_rgb(255, 255, 0), 2.0f);
	}

	// Draw enemy bullets
	bullet_t enemy_bullets[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];
	getEnemiesBulletsInfo(enemy_bullets);
	for (row = 0; row < ENEMIES_ROW_MAX; row++) 
	{
		for (col = 0; col < ENEMIES_COLUMNS_MAX; col++) 
		{
			if (!enemy_bullets[row][col].active) continue;

			hitbox_t bullet_hb = clipHitbox(enemy_bullets[row][col].hitbox, margin_x, margin_y, inner_w, inner_h);
			draw_bullet(bullet_hb, display);
			if (show_hitboxes)
				al_draw_rectangle(bullet_hb.start.x, bullet_hb.start.y, bullet_hb.end.x, bullet_hb.end.y, 
								 al_map_rgb(255, 255, 0), 2.0f);
		}
	}

	static long long explosionMothershipTime;
	static bool mothershipAlive;
	// Draw mothership
	hitbox_t mothership_hb = clipHitbox(getMothershipPosition(), margin_x, margin_y, inner_w, inner_h);
	if (getIsMothershipAlive()) 
	{
		draw_mothership(mothership_hb, display);
		if (show_hitboxes) 
			al_draw_rectangle(mothership_hb.start.x, mothership_hb.start.y, 
							 mothership_hb.end.x, mothership_hb.end.y, 
							 al_map_rgb(255, 255, 0), 2.0f);
		mothershipAlive = true;
	}
	else
	{
		long long currentTime = getTimeMillis();

			if(mothershipAlive)
			{
				draw_explosion(mothership_hb, display, 1);
				mothershipAlive = false;
				explosionMothershipTime = currentTime;
			}
			else if(currentTime - explosionMothershipTime < 150 && explosionMothershipTime != 0)
				draw_explosion(mothership_hb, display, 1);
			else
				explosionMothershipTime = 0;
	}
	
	return gamePaused;
}

/* ---------------------------------------------------
 * @brief Draw barrier segments with visual effects
 * @param barrier Barrier index
 * @param margin_x X margin for clipping
 * @param margin_y Y margin for clipping
 * @param inner_w Inner width for clipping
 * @param inner_h Inner height for clipping
 * @param show_hitboxes Whether to render hitboxes
 * ---------------------------------------------------*/
static void draw_barriers(int barrier, float margin_x, float margin_y, float inner_w, float inner_h, bool show_hitboxes)
{
	// First pass: Draw all barrier segments
	for (int row = 0; row < BARRIER_ROWS; row++) 
	{
		for (int col = 0; col < BARRIER_COLUMNS; col++) 
		{
			if (!getBarrierIsAlive(barrier, row, col)) continue;
			
			hitbox_t hb = clipHitbox(getBarrierPosition(barrier, row, col), margin_x, margin_y, inner_w, inner_h);
			al_draw_filled_rectangle(hb.start.x, hb.start.y, hb.end.x, hb.end.y, al_map_rgba(120, 0, 180, 80));
		}
	}

	// Second pass: Draw energy grid
	for (int row = 0; row < BARRIER_ROWS; row++) 
	{
		for (int col = 0; col < BARRIER_COLUMNS; col++) 
		{
			if (!getBarrierIsAlive(barrier, row, col)) continue;
			
			hitbox_t hb = clipHitbox(getBarrierPosition(barrier, row, col), margin_x, margin_y, inner_w, inner_h);
			float center_x = (hb.start.x + hb.end.x) / 2;
			float center_y = (hb.start.y + hb.end.y) / 2;
			float time = al_get_time();
			float pulse = 0.5f + 0.5f * sin(time * 3.0f);
			
			// Horizontal energy lines
			for (int i = 0; i < 3; i++) 
			{
				float y = hb.start.y + (i+1) * (hb.end.y - hb.start.y)/4;
				al_draw_line(hb.start.x, y, hb.end.x, y, al_map_rgba_f(0.3f, 0.8f, 1.0f, 0.3f + pulse*0.3f), 1.5f);
			}
			
			// Connect to neighboring barriers
			if (col < BARRIER_COLUMNS-1 && getBarrierIsAlive(barrier, row, col+1)) 
			{
				hitbox_t right_hb = clipHitbox(getBarrierPosition(barrier, row, col+1), margin_x, margin_y, inner_w, inner_h);
				al_draw_line(hb.end.x, center_y, right_hb.start.x, center_y, al_map_rgba(100, 200, 255, 100), 2.0f);
			}
			
			if (row < BARRIER_ROWS-1 && getBarrierIsAlive(barrier, row+1, col)) 
			{
				hitbox_t bottom_hb = clipHitbox(getBarrierPosition(barrier, row+1, col), margin_x, margin_y, inner_w, inner_h);
				al_draw_line(center_x, hb.end.y, center_x, bottom_hb.start.y, al_map_rgba(100, 200, 255, 100), 2.0f);
			}
		}
	}

	// Third pass: Add pulsating core
	for (int row = 0; row < BARRIER_ROWS; row++) 
	{
		for (int col = 0; col < BARRIER_COLUMNS; col++) 
		{
			if (!getBarrierIsAlive(barrier, row, col)) continue;
			
			hitbox_t hb = clipHitbox(getBarrierPosition(barrier, row, col), margin_x, margin_y, inner_w, inner_h);
			float time = al_get_time();
			float pulse_size = 0.8f + 0.2f * sin(time * 4.0f);
			float size = (hb.end.x - hb.start.x) * 0.3f * pulse_size;
			
			if (show_hitboxes) 
				al_draw_rectangle(hb.start.x, hb.start.y, hb.end.x, hb.end.y, al_map_rgb(0,255,0), 2.0f);
			
			al_draw_filled_circle((hb.start.x + hb.end.x)/2, (hb.start.y + hb.end.y)/2, size,
								al_map_rgba_f(0.4f, 0.9f, 1.0f, 0.4f));
		}
	}
}

/* ---------------------------------------------------
 * @brief Display pause menu and handle user selection
 * @param display Allegro display
 * @param background Snapshot of game state
 * @return Selected game state
 * ---------------------------------------------------*/
static gameState_t pauseMenu(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *background)
{
	gameState_t next_state;
	const char *options[] = {"Resume", "Restart", "Quit to Menu", "Exit Game"};
	const int option_count = 4;
	int selected = 0;

	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	bool choosing = true;

	while (choosing) 
	{
		int screen_w = al_get_display_width(display);
		int screen_h = al_get_display_height(display);
		float option_spacing = screen_h * 0.06f;
		float rect_width = screen_w * 0.4f;
		float rect_height = option_count * option_spacing + 40;
		float rect_x = (screen_w - rect_width) / 2;
		float rect_y = (screen_h - rect_height) / 2;
		// Draw frozen game state with overlay
		al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
							  0, 0, al_get_display_width(display), al_get_display_height(display), 0);
		al_draw_filled_rectangle(0, 0, screen_w, screen_h, al_map_rgba(0, 0, 0, 128));

		// Draw menu box
		al_draw_filled_rectangle(rect_x, rect_y, rect_x + rect_width, rect_y + rect_height,
								al_map_rgb(50, 50, 50));
		al_draw_rectangle(rect_x, rect_y, rect_x + rect_width, rect_y + rect_height,
						 al_map_rgb(255, 255, 255), 2);

		// Draw menu options
		draw_menu_options(options, option_count, selected, rect_y + 20, option_spacing);

		al_flip_display();

		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);
		if(event_handle(queue, &event, &choosing, display) == STATE_EXIT)
				next_state = STATE_EXIT;
		
		if (event.type == ALLEGRO_EVENT_KEY_DOWN) 
		{
			switch (event.keyboard.keycode) 
			{
				case ALLEGRO_KEY_UP:
					selected = (selected - 1 + option_count) % option_count;
					playSound_play(SOUND_MENU);
					break;
				case ALLEGRO_KEY_DOWN:
					selected = (selected + 1) % option_count;
					playSound_play(SOUND_MENU);
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

	if(next_state == STATE_EXIT)
		return STATE_EXIT;
	
	al_destroy_event_queue(queue);

	// Map selection to game state
	switch (selected) 
	{
		case 0: next_state = STATE_RESUME_GAME; break;
		case 1: next_state = STATE_NEW_GAME; break;
		case 2: next_state = STATE_MENU; break;
		case 3: next_state = STATE_EXIT; break;
		default: next_state = STATE_RESUME_GAME; break;
	}

	return next_state;
}

/* ---------------------------------------------------
 * @brief Display scoreboard with pagination
 * @return Next game state (always returns to menu)
 * ---------------------------------------------------*/
static gameState_t showScoreboard(ALLEGRO_DISPLAY *display) 
{
	gameState_t next_state = STATE_MENU;
	const int MAX_SCORES = 100;
	score_t scores[MAX_SCORES];
	int total_scores = getTopScore(scores, MAX_SCORES);
	int current_page = 0;

	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	bool viewing = true;
	ALLEGRO_EVENT event;

	// Load arrow bitmap
	ALLEGRO_BITMAP *arrow = al_load_bitmap("assets/images/arrows.png");
	if (!arrow)
		fprintf(stderr, "Failed to load arrow images\n");
		// Handle error - maybe use text arrows instead

	// Arrow hitboxes
	hitbox_t left_arrow_hb = 
	{
		.start = {.x = 0.05f, .y = 0.85f},
		.end = {.x = 0.10f, .y = 0.95f}
	};
	hitbox_t right_arrow_hb = 
	{
		.start = {.x = 0.90f, .y = 0.85f},
		.end = {.x = 0.95f, .y = 0.95f}
	};

	while (viewing) 
	{
		// Pantalla
		float screen_w = al_get_display_width(display);
		float screen_h = al_get_display_height(display);

		// Dimensiones del tablero
		float board_width = 0.8f * screen_w;
		float board_height = 0.7f * screen_h;
		float board_x = (screen_w - board_width) / 2.0f;
		float board_y = (screen_h - board_height) / 2.0f;

		// Espaciado vertical proporcional
		float title_offset_y = 0.05f * board_height;
		float page_info_offset_y = 0.1f * board_height;
		float list_start_y = 0.2f * board_height;

		// Altura por línea (puntaje)
		float entry_height = 0.05f * screen_h;

		// Cálculo de cuántos puntajes entran
		int SCORES_PER_PAGE = (int)((board_height - list_start_y) / entry_height);
		if (SCORES_PER_PAGE < 1) SCORES_PER_PAGE = 1;
		int total_pages = (total_scores + SCORES_PER_PAGE - 1) / SCORES_PER_PAGE;
		al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
							  0, 0, al_get_display_width(display), al_get_display_height(display), 0);

		// Draw semi-transparent overlay
		al_draw_filled_rectangle(0, 0, al_get_display_width(display), al_get_display_height(display),
								al_map_rgba(0, 0, 0, 200));

		// Draw scoreboard background
		al_draw_filled_rectangle(board_x, board_y, board_x + board_width, board_y + board_height,
								al_map_rgb(50, 50, 80));
		al_draw_rectangle(board_x, board_y, board_x + board_width, board_y + board_height,
						  al_map_rgb(255, 255, 255), 3);
		// Título centrado
		al_draw_text(font, al_map_rgb(255, 255, 0),
					screen_w / 2,
					board_y + title_offset_y,
					ALLEGRO_ALIGN_CENTER, "TOP SCORES");

		// Página
		char page_info[32];
		snprintf(page_info, sizeof(page_info), "Page %d/%d", current_page + 1, total_pages);
		al_draw_text(font, al_map_rgb(200, 200, 200),
					screen_w / 2,
					board_y + page_info_offset_y,
					ALLEGRO_ALIGN_CENTER, page_info);

		// Mostrar entradas
		int start_idx = current_page * SCORES_PER_PAGE;
		int end_idx = (start_idx + SCORES_PER_PAGE < total_scores) ? start_idx + SCORES_PER_PAGE : total_scores;

		for (int i = start_idx; i < end_idx; i++) 
		{
			float y_pos = board_y + list_start_y + (i - start_idx) * entry_height;

			// Número
			char rank[16];
			snprintf(rank, sizeof(rank), "%d.", i + 1);
			al_draw_text(font, al_map_rgb(255, 255, 255),
						board_x + 0.05f * board_width,
						y_pos,
						ALLEGRO_ALIGN_LEFT, rank);

			// Nombre
			al_draw_text(font, al_map_rgb(255, 255, 255),
						board_x + 0.18f * board_width,
						y_pos,
						ALLEGRO_ALIGN_LEFT, scores[i].name);

			// Puntaje
			char score_str[16];
			snprintf(score_str, sizeof(score_str), "%d", scores[i].score);
			al_draw_text(font, al_map_rgb(255, 255, 255),
						board_x + 0.95f * board_width,
						y_pos,
						ALLEGRO_ALIGN_RIGHT, score_str);
		}

		// Tamaños comunes para las flechas

		// Draw left arrow (flipped horizontally)
		if (current_page > 0 && arrow) 
		{
			float arrow_w = (left_arrow_hb.end.x - left_arrow_hb.start.x) * screen_w;
			float arrow_h = (left_arrow_hb.end.y - left_arrow_hb.start.y) * screen_h;
			float draw_x = left_arrow_hb.start.x * screen_w;
			float draw_y = left_arrow_hb.start.y * screen_h;

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
		if (current_page < total_pages - 1 && arrow) 
		{
			float arrow_w = (right_arrow_hb.end.x - right_arrow_hb.start.x) * screen_w;
			float arrow_h = (right_arrow_hb.end.y - right_arrow_hb.start.y) * screen_h;
			float draw_x = right_arrow_hb.start.x * screen_w;
			float draw_y = right_arrow_hb.start.y * screen_h;

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

		if(event_handle(queue, &event, &viewing, display) == STATE_EXIT)
				next_state = STATE_EXIT;

		else if (event.type == ALLEGRO_EVENT_KEY_DOWN) 
		{
			switch (event.keyboard.keycode) 
			{
				case ALLEGRO_KEY_ESCAPE:
					viewing = false;
					break;
				case ALLEGRO_KEY_LEFT:
					if (current_page > 0) 
					{
						current_page--;
						playSound_play(SOUND_MENU);
					}
					break;
				case ALLEGRO_KEY_RIGHT:
					if (current_page < total_pages - 1) 
					{
						current_page++;
						playSound_play(SOUND_MENU);
					}
					break;
			}
		}
	}

	// Cleanup
	al_destroy_event_queue(queue);
	if (arrow) al_destroy_bitmap(arrow);

	return next_state;
}

/* ---------------------------------------------------
 * @brief Read credits from file into array
 * @param line_count Pointer to store number of lines read
 * @return Array of strings containing credits
 * ---------------------------------------------------*/
static char** readCreditsFile(int* line_count) 
{
	FILE* file = fopen("data/credits.txt", "r");
	if (!file) 
	{
		fprintf(stderr, "Failed to load credits file\n");
		return NULL;
	}

	// Count lines
	int count = 0;
	char ch;
	while (!feof(file)) 
	{
		ch = fgetc(file);
		if (ch == '\n') count++;
	}
	count++; // For last line without newline
	rewind(file);

	// Allocate and read lines
	char** lines = (char**)malloc(count * sizeof(char*));
	if (!lines) 
	{
		fclose(file);
		return NULL;
	}

	char buffer[256];
	int i = 0;
	while (fgets(buffer, sizeof(buffer), file) && i < count) 
	{
		buffer[strcspn(buffer, "\n")] = 0; // Remove newline
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

/* ---------------------------------------------------
 * @brief Display scrolling credits screen
 * @return Next game state (always returns to menu)
 * ---------------------------------------------------*/
static gameState_t showCredits(ALLEGRO_DISPLAY *display)
{
	gameState_t next_state = STATE_MENU;
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	// Load credits content
	int credit_count = 0;
	char** credits = readCreditsFile(&credit_count);
	
	// Fallback if file loading fails
	if (!credits) 
	{
		const char* fallback_credits[] = {
			"CREDITS FILE MISSING!", " ", "This is awkward...",
			"We seem to have misplaced our credits file", " ",
			"Please imagine something awesome here", " ",
			"PRESS ANY KEY TO RETURN", NULL
		};

		credit_count = 0;
		while (fallback_credits[credit_count]) credit_count++;

		credits = (char**)malloc((credit_count+1) * sizeof(char*));

		for (int i = 0; i < credit_count; i++)
			credits[i] = strdup(fallback_credits[i]);

		credits[credit_count] = NULL;
	}

	// Load background
	ALLEGRO_BITMAP *credits_bg = al_load_bitmap("assets/images/credits_bg.png");
	if (!credits_bg) 
		credits_bg = al_load_bitmap("assets/images/backgroundMenu.png");

	// Setup scrolling animation
	float scroll_pos = al_get_display_height(display);
	bool credits_done = false;
	bool shown_segfault_joke = false;
	ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_start_timer(timer);

	while (!credits_done)
	{
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);

		if(event_handle(queue, &event, &credits_done, display) == STATE_EXIT)
				next_state = STATE_EXIT;
		
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			if (event.keyboard.keycode != ALLEGRO_KEYMOD_ALT  && event.keyboard.keycode != ALLEGRO_KEY_F4 &&
				event.keyboard.keycode != ALLEGRO_KEY_F11 )
			{
				credits_done = true;
			}
		}
			
		else if (event.type == ALLEGRO_EVENT_TIMER) 
		{
			scroll_pos -= 0.7f;

			// Check for segfault joke position
			if (!shown_segfault_joke && scroll_pos < -1500.0f) 
			{
				for (int i = 0; i < credit_count; i++) 
				{
					if (strstr(credits[i], "*segmentation fault*")) 
					{
						playSound_play(SOUND_EXPLOSION);
						shown_segfault_joke = true;
						break;
					}
				}
			}

			if (scroll_pos < -2500.0f) 
				credits_done = true;
		}

		// Draw credits
		if (credits_bg) 
			al_draw_scaled_bitmap(credits_bg, 0, 0, al_get_bitmap_width(credits_bg),
								al_get_bitmap_height(credits_bg), 0, 0, 
								al_get_display_width(display), al_get_display_height(display), 0);
		else 
			al_clear_to_color(al_map_rgb(0, 0, 0));

		// Draw each credit line with appropriate formatting
		float y = scroll_pos;
		for (int i = 0; i < credit_count; i++) 
		{
			ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
			int font_size = al_get_font_line_height(font);
			float x = al_get_display_width(display) / 2;

			// Apply special formatting based on content
			if (strstr(credits[i], "SPACE INVADERS")) 
			{
				color = al_map_rgb(0, 255, 255);
				font_size *= 1.5;
			}
			else if (strstr(credits[i], "THE MOST AWESOME") ||
					strstr(credits[i], "FEARLESS LEADERS") ||
					strstr(credits[i], "SPECIAL THANKS") ||
					strstr(credits[i], "FUN FACT") ||
					strstr(credits[i], "DISCLAIMER") ||
					strstr(credits[i], "MEMORY LEAK") ||
					strstr(credits[i], "SEGMENTATION")) 
				color = al_map_rgb(255, 255, 0);

			else if (strstr(credits[i], "PRESS ANY KEY") ||
					 strstr(credits[i], "segmentation fault"))
				color = al_map_rgb(255, 0, 0);

			al_draw_text(font, color, x, y, ALLEGRO_ALIGN_CENTER, credits[i]);
			y += font_size * 1.2;
		}

		al_flip_display();
	}

	// Cleanup
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);
	if (credits_bg) al_destroy_bitmap(credits_bg);

	// Free credits memory
	for (int i = 0; i < credit_count; i++)
		free(credits[i]);

	free(credits);

	return next_state;
}

/* ---------------------------------------------------
 * @brief Display game over screen and handle high scores
 * @return Next game state (always returns to menu)
 * ---------------------------------------------------*/
static gameState_t showGameOver(ALLEGRO_DISPLAY *display)
{
	gameState_t next_state = STATE_MENU;
	score_t * topScores=(score_t *)calloc(100,sizeof(score_t));
	int count = getTopScore(topScores, 100); // cargar top 10
	
	bool highscore = isHighScore(getScore(), topScores, count);
	
	char name[15] = "";
	int name_pos = 0;

	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	bool done = false;
	ALLEGRO_EVENT event;

	while (!done) 
	{
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

		for (int i = 0; i < 10; ++i) 
		{
			char buffer[64];
			snprintf(buffer, sizeof(buffer), "%d. %-15s - %d", i + 1, topScores[i].name, topScores[i].score);
			al_draw_text(font, al_map_rgb(255, 255, 255),
						 al_get_display_width(display)/2,
						 al_get_display_height(display) * (0.35f + i * 0.05f),
						 ALLEGRO_ALIGN_CENTER, buffer);
		}

		// Si entra en top
		if (highscore) 
		{
			al_draw_text(font, al_map_rgb(0, 255, 0),
						 al_get_display_width(display)/2,
						 al_get_display_height(display) * 0.85f,
						 ALLEGRO_ALIGN_CENTER, "New High Score! Enter your name:");

			al_draw_text(font, al_map_rgb(255, 255, 255),
						 al_get_display_width(display)/2,
						 al_get_display_height(display) * 0.9f,
						 ALLEGRO_ALIGN_CENTER, name);
		} 
		else
		{
			al_draw_text(font, al_map_rgb(255, 0, 0),
						 al_get_display_width(display)/2,
						 al_get_display_height(display) * 0.85f,
						 ALLEGRO_ALIGN_CENTER, "Press ENTER to return to menu");
		}

		al_flip_display();

		al_wait_for_event(queue, &event);
		
		if(event_handle(queue, &event, &done, display) == STATE_EXIT)
				next_state = STATE_EXIT;

		if (event.type == ALLEGRO_EVENT_KEY_CHAR && name_pos < 14) 
		{
			if (event.keyboard.unichar > 32 && event.keyboard.unichar <= 126) 
			{
				name[name_pos++] = event.keyboard.unichar;
				name[name_pos] = '\0';
			} 
		}
		if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && name_pos > 0)
			name[--name_pos] = '\0';

		if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_ENTER)
		{
			if (!highscore || (highscore && name_pos > 0)) 
			{
				if (highscore)
					topScoreUpdate(getScore(), name); // Agregás el nuevo score

				done = true;
			}
		}
	}
    free(topScores);
	al_destroy_event_queue(queue);
	return next_state;
}

/* ---------------------------------------------------
 * @brief Check if score qualifies as high score
 * @param score Score to check
 * @param topScores Array of top scores
 * @param count Number of scores in array
 * @return true if score is high enough, false otherwise
 * ---------------------------------------------------*/
static bool isHighScore(int score, score_t topScores[], int count) 
{
	return (count < 10) || (score > topScores[count - 1].score);
}

/* ---------------------------------------------------
 * @brief Adjust hitbox coordinates based on display margins
 * @param hb Original hitbox
 * @param margin_x Horizontal margin
 * @param margin_y Vertical margin
 * @param inner_w Inner width
 * @param inner_h Inner height
 * @return Adjusted hitbox
 * ---------------------------------------------------*/
static hitbox_t clipHitbox(hitbox_t hb, float margin_x, float margin_y, float inner_w, float inner_h)
{
	return (hitbox_t)
	{
		.start = {
			.x = margin_x + hb.start.x * inner_w,
			.y = margin_y + hb.start.y * inner_h
		},
		.end = {
			.x = margin_x + hb.end.x * inner_w,
			.y = margin_y + hb.end.y * inner_h
		}
	};
}

/* ==================== HELPER FUNCTIONS ==================== */

/* ---------------------------------------------------
 * @brief Draw text centered at relative coordinates
 * @param x_rel Relative x position (0.0-1.0)
 * @param y_rel Relative y position (0.0-1.0)
 * @param text Text to draw
 * @param color Text color
 * ---------------------------------------------------*/
static void draw_centered_text(float x_rel, float y_rel, const char *text, ALLEGRO_COLOR color)
{
	const float x = x_rel * al_get_display_width(display);
	const float y = y_rel * al_get_display_height(display);
	al_draw_text(font, color, x, y, ALLEGRO_ALIGN_CENTER, text);
}

/* ---------------------------------------------------
 * @brief Draw menu options with selection highlighting
 * @param options Array of option strings
 * @param count Number of options
 * @param selected Currently selected option index
 * @param start_y Starting y position
 * @param spacing Vertical spacing between options
 * ---------------------------------------------------*/
static void draw_menu_options(const char **options, int count, int selected, float start_y, float spacing)
{
	for (int i = 0; i < count; ++i) 
	{
		ALLEGRO_COLOR color = (i == selected) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
		al_draw_text(font, color, al_get_display_width(display) / 2,
					start_y + i * spacing, ALLEGRO_ALIGN_CENTER, options[i]);
	}
}

/* ---------------------------------------------------
 * @brief Set up common event sources for a queue
 * @param queue Event queue to configure
 * @param disp Display to register
 * ---------------------------------------------------*/
static void setup_event_queue(ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_DISPLAY *disp)
{
	al_register_event_source(queue, al_get_keyboard_event_source());
	if (disp) al_register_event_source(queue, al_get_display_event_source(disp));
}

/* ---------------------------------------------------
 * @brief Draw background scaled to fill display
 * @param bg Background bitmap
 * ---------------------------------------------------*/
static void draw_scaled_background(ALLEGRO_BITMAP *bg)
{
	al_draw_scaled_bitmap(bg, 0, 0, al_get_bitmap_width(bg), al_get_bitmap_height(bg),
						 0, 0, al_get_display_width(display), al_get_display_height(display), 0);
}