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
	static bool fullscreen = false;	 // Track fullscreen state
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
					// Toggle fullscreen mode
					fullscreen = !fullscreen;
					al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
					// Reset display size and reload font at proper scale
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

	// Load splash screen background
	ALLEGRO_BITMAP *bg = al_load_bitmap("assets/images/backgroundMenu.png");
	if (!bg) {
		al_destroy_event_queue(queue);
		return STATE_EXIT;
	}

	bool running = true;
	bool show_prompt = false;  // Controls "press enter" visibility
	const double start_time = al_get_time();  // For delayed prompt
	gameState_t next_state = STATE_EXIT;  // Default exit if error

	while (running) 
	{
		draw_scaled_background(bg); // Show prompt after 2 second delay

		// Show prompt after 2 second delay
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

	// Load top scores for display
	score_t topScores[5];
	const int topCount = getTopScore(topScores, 5);

	// Set up input handling
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	bool choosing = true;

	while (choosing) 
	{
		// Calculate dynamic layout based on current display size
		float screen_w = al_get_display_width(display);
		float screen_h = al_get_display_height(display);

		draw_scaled_background(background);

		// Vertical positioning parameters
		float title_y = 0.07f;
		float menu_start_y = 0.55f;
		float option_spacing = 0.08f;

		// Score box dimensions
		float rect_width = 0.3f;
		float font_size = al_get_font_line_height(font);
		float rect_width_px = 0.3f * screen_w;
		float rect_height = font_size * 1.2f + 5 * (font_size * 1.1f) + font_size * 0.8f;
		float rect_x = (screen_w - rect_width_px) / 2.0f;
		float rect_y = 0.55f * screen_h - rect_height - 0.05f * screen_h;

		// 1. Draw game title graphic
		draw_title(screen_w / 2, title_y * screen_h, display);

		// 2. Draw score display box
		al_draw_filled_rectangle(rect_x, rect_y, rect_x + rect_width * screen_w, 
								rect_y + rect_height, al_map_rgb(100, 100, 100));
		al_draw_rectangle(rect_x, rect_y, rect_x + rect_width * screen_w, 
						  rect_y + rect_height, al_map_rgb(255, 255, 255), 2);

		// 3. Draw scores title
		draw_centered_text((rect_x + rect_width_px / 2.0f) / screen_w, 
						   (rect_y + font_size * 0.2f) / screen_h,
						   "TOP SCORES", al_map_rgb(255, 255, 255));

		// 4. Render each high score entry 
		for (int i = 0; i < 5 && i < topCount; ++i) 
		{
			char buffer[64];
			// Format: "1. PlayerName    12345"
			snprintf(buffer, sizeof(buffer), "%d. %-15s\t%d", i + 1, topScores[i].name, topScores[i].score);
			al_draw_text(font, al_map_rgb(255, 255, 255), rect_x + 0.01f * screen_w,
						 rect_y + font_size * 1.2f + i * (font_size * 1.1f), 0, buffer);
		}

		// 5. Draw menu options with selection highlight
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
					// Navigation sound
					playSound_play(SOUND_MENU);
					selected = (selected - 1 + option_count) % option_count;
					break;
				case ALLEGRO_KEY_DOWN:
					// Navigation sound
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

	// Map selection to game state
	if(next_state == STATE_EXIT)
		return STATE_EXIT;

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
		// Try to load saved game, return to menu if failed
		if(!game_init(enemyRow, enemyCol, true))
		{
			return STATE_MENU;
		}
	}
	// Player input tracking
	input_t player = {0};
	// Debug mode flag
	bool show_hitboxes = false;
	bool f3_pressed = false, b_pressed = false, flag_hitboxes = true;

	bool running = true;
	// Controls frame updates
	bool redraw = true;
	gameState_t next_state = state;
	// Tracks if game has started
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
				redraw = true;	// Trigger frame update
				break;

			case ALLEGRO_EVENT_KEY_DOWN:
				// Player movement and actions
				switch (event.keyboard.keycode) 
				{
					case ALLEGRO_KEY_A:
					case ALLEGRO_KEY_LEFT:  player.direction = -1; break;	// Move left
					case ALLEGRO_KEY_D:
					case ALLEGRO_KEY_RIGHT: player.direction = 1; break;	// Move right
					case ALLEGRO_KEY_SPACE: player.shot = true; break;		// Fire weapon
					case ALLEGRO_KEY_ESCAPE: 
						player.pause = !player.pause;	// Toggle pause
						if (player.pause) 
						{
							game_update(player);		// Final update before pause
							next_state = STATE_PAUSE;
							running = false;
						}
						break;
					case ALLEGRO_KEY_F3: f3_pressed = true; break;	// Debug combo
					case ALLEGRO_KEY_B: b_pressed = true; break;	// Debug combo
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

		// Debug mode toggle (F3+B)
		if (f3_pressed && b_pressed && flag_hitboxes) 
		{
			show_hitboxes = !show_hitboxes;	// Toggle hitbox visibility
			flag_hitboxes = false;			// Toggle hitbox visibility
		}

		// Only redraw when necessary and no pending events
		if (redraw && al_is_event_queue_empty(queue))
		{
			redraw = false;
			draw_scaled_background(background);

			// Update game state and check for game over
			int game_status = game_update(player);
			if (game_status == GAME_OVER) 
			{
				running = false;
				if(!firstUpdate)
				{
					// Handle special case for failed resume
					next_state = (state == STATE_RESUME_GAME) ? STATE_MENU : STATE_GAME_OVER;
				}
				else
					next_state = STATE_GAME_OVER;
				break;
			}
			// Mark game as started
			firstUpdate = true;

			// Draw UI elements
			draw_hearts(getPlayerLives(), display);	// Life counter
			draw_player_score(display, getScore());	// Score display

			// Set up gameplay area clipping
			float margin_x = al_get_display_width(display) * 0.08f;
			float margin_y = al_get_display_height(display) * 0.10f;
			float inner_w = al_get_display_width(display) - 3 * margin_x;
			float inner_h = al_get_display_height(display) - 2 * margin_y;
			al_set_clipping_rectangle(margin_x, margin_y, inner_w, inner_h);

			// Draw all game objects
			player.pause = draw_game_entities(margin_x, margin_y, inner_w, inner_h, show_hitboxes);

			// Restore full-screen drawing
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

	hitbox_t player_hb = clipHitbox(getPlayerPosition(), margin_x, margin_y, inner_w, inner_h);
	
	// Check if the player has lost a life since the last frame
	if(playerLives > getPlayerLives())
	{
		playerDieTime = getTimeMillis();	// Register time of death
		draw_player_died(player_hb, display, currentTime - playerDieTime);
		gamePaused = true;
		flag = true;	// Mark animation should play
	}
	else if(currentTime - playerDieTime < 500 && flag)
	{
		// Continue drawing death animation if within 500ms window
		draw_player_died(player_hb, display, currentTime - playerDieTime);
		gamePaused = true;
	}
	else
	{
		// Otherwise, draw normal player sprite
		draw_player(player_hb, display);
		playerDieTime = currentTime;
		gamePaused = false;
		flag = false;
	}

	// Update persistent life tracker
	playerLives = getPlayerLives();
	
	// Optionally draw player's hitbox
	if (show_hitboxes) 
		al_draw_rectangle(player_hb.start.x, player_hb.start.y, player_hb.end.x, player_hb.end.y, 
						  al_map_rgb(0,255,0), 2.0f);

	// Static matrix storing enemy alive state from previous frame
	static bool enemyAlive[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];

	// Stores timestamp of last enemy explosion per grid cell
	static long long explosionEnemyTime[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX] = {0};

	// Loop over all enemies
	int row, col;
	for (row = 0; row < ENEMIES_ROW_MAX; row++) 
	{
		for (col = 0; col < ENEMIES_COLUMNS_MAX; col++) 
		{
			hitbox_t hitbox = clipHitbox(getEnemyPosition(row,col), margin_x, margin_y, inner_w, inner_h);
			if (!getIsEnemyAlive(row,col))
			{
				// If enemy was just destroyed
				if(enemyAlive[row][col])
				{
					draw_explosion(hitbox, display, 0);
					enemyAlive[row][col] = 0;
					explosionEnemyTime[row][col] = currentTime;
				}
				// Show lingering explosion for a short duration
				else if(currentTime - explosionEnemyTime[row][col] < 150 && explosionEnemyTime[row][col] != 0)
					draw_explosion(hitbox, display, 0);
				else
					explosionEnemyTime[row][col] = 0;

				continue;

			}
			// Assign enemy color by tier (used for hitbox rendering)
			ALLEGRO_COLOR color;
			switch (getEnemyTier(row)) 
			{
				case ALIEN_TIER1: color = al_map_rgb(255, 0, 0); break;
				case ALIEN_TIER2: color = al_map_rgb(0, 255, 0); break;
				case ALIEN_TIER3: color = al_map_rgb(0, 0, 255); break;
				default: break;
			}
			// Draw enemy sprite
			draw_invaders(hitbox, row, display);
			if (show_hitboxes) 
			{
				al_draw_rectangle(hitbox.start.x, hitbox.start.y, hitbox.end.x, hitbox.end.y, color, 2.0f);
			}
			enemyAlive[row][col] = 1;
		}
	}
	

	// Draw all barriers
	int barrier;
	for (barrier = 0; barrier < BARRIER_QUANTITY_MAX; barrier++)
		draw_barriers(barrier, margin_x, margin_y, inner_w, inner_h, show_hitboxes);

	// Get player bullet and draw if active
	bullet_t bullet = getPlayerBulletinfo();
	if (bullet.active) 
	{
		hitbox_t bullet_hb = clipHitbox(bullet.hitbox, margin_x, margin_y, inner_w, inner_h);
		draw_bullet(bullet_hb, display);
		if (show_hitboxes) 
			al_draw_rectangle(bullet_hb.start.x, bullet_hb.start.y, bullet_hb.end.x, bullet_hb.end.y, 
							 al_map_rgb(255, 255, 0), 2.0f);
	}

	// Get enemy bullets and render them
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

	// Static variables for handling mothership explosion animation
	static long long explosionMothershipTime;
	static bool mothershipAlive;
	
	// Get mothership hitbox
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
		currentTime = getTimeMillis();

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

		// Calculate menu box dimensions
		float option_spacing = screen_h * 0.06f;
		float rect_width = screen_w * 0.4f;
		float rect_height = option_count * option_spacing + 40;
		float rect_x = (screen_w - rect_width) / 2;
		float rect_y = (screen_h - rect_height) / 2;

		// 1. Draw frozen game state with dark overlay
		al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
							  0, 0, al_get_display_width(display), al_get_display_height(display), 0);
		al_draw_filled_rectangle(0, 0, screen_w, screen_h, al_map_rgba(0, 0, 0, 128));

		// 2. Draw menu container
		al_draw_filled_rectangle(rect_x, rect_y, rect_x + rect_width, rect_y + rect_height,
								al_map_rgb(50, 50, 50));
		al_draw_rectangle(rect_x, rect_y, rect_x + rect_width, rect_y + rect_height,
						 al_map_rgb(255, 255, 255), 2);

		// 3. Draw menu options with selection highlight
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
	
	al_destroy_event_queue(queue);

	// Map selection to game state
	if(next_state == STATE_EXIT)
		return STATE_EXIT;

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
	const int MAX_SCORES = 100;	// Maximum number of scores to store
	score_t scores[MAX_SCORES];	// Array to hold scores
	int total_scores = getTopScore(scores, MAX_SCORES);
	int current_page = 0;

	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	bool viewing = true;
	ALLEGRO_EVENT event;

	// Load arrow bitmap used for navigation
	ALLEGRO_BITMAP *arrow = al_load_bitmap("assets/images/arrows.png");
	if (!arrow)
		fprintf(stderr, "Failed to load arrow images\n");

	// Define clickable hitboxes for left and right arrows
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
		// Get screen dimensions
		float screen_w = al_get_display_width(display);
		float screen_h = al_get_display_height(display);

		// Define scoreboard box dimensions
		float board_width = 0.8f * screen_w;
		float board_height = 0.7f * screen_h;
		float board_x = (screen_w - board_width) / 2.0f;
		float board_y = (screen_h - board_height) / 2.0f;

		// Position offsets within scoreboard
		float title_offset_y = 0.05f * board_height;
		float page_info_offset_y = 0.1f * board_height;
		float list_start_y = 0.2f * board_height;

		// Height of each score entry
		float entry_height = 0.05f * screen_h;

		// Calculate how many scores fit per page
		int SCORES_PER_PAGE = (int)((board_height - list_start_y) / entry_height);
		if (SCORES_PER_PAGE < 1) SCORES_PER_PAGE = 1;

		// Calculate total number of pages
		int total_pages = (total_scores + SCORES_PER_PAGE - 1) / SCORES_PER_PAGE;


		al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
							  0, 0, al_get_display_width(display), al_get_display_height(display), 0);

		// Draw semi-transparent overlay to dim background
		al_draw_filled_rectangle(0, 0, al_get_display_width(display), al_get_display_height(display),
								al_map_rgba(0, 0, 0, 200));

		// Draw scoreboard container
		al_draw_filled_rectangle(board_x, board_y, board_x + board_width, board_y + board_height,
								al_map_rgb(50, 50, 80));
		al_draw_rectangle(board_x, board_y, board_x + board_width, board_y + board_height,
						  al_map_rgb(255, 255, 255), 3);

		// Draw title text
		al_draw_text(font, al_map_rgb(255, 255, 0),
					screen_w / 2,
					board_y + title_offset_y,
					ALLEGRO_ALIGN_CENTER, "TOP SCORES");

		// Draw current page indicator
		char page_info[32];
		snprintf(page_info, sizeof(page_info), "Page %d/%d", current_page + 1, total_pages);
		al_draw_text(font, al_map_rgb(200, 200, 200),
					screen_w / 2,
					board_y + page_info_offset_y,
					ALLEGRO_ALIGN_CENTER, page_info);

		// Calculate score indices for current page
		int start_idx = current_page * SCORES_PER_PAGE;
		int end_idx = (start_idx + SCORES_PER_PAGE < total_scores) ? start_idx + SCORES_PER_PAGE : total_scores;

		// Draw score entries on screen
		for (int i = start_idx; i < end_idx; i++) 
		{
			float y_pos = board_y + list_start_y + (i - start_idx) * entry_height;

			// Draw rank number
			char rank[16];
			snprintf(rank, sizeof(rank), "%-3d.", i + 1);
			al_draw_text(font, al_map_rgb(255, 255, 255),
						board_x + 0.05f * board_width,
						y_pos,
						ALLEGRO_ALIGN_LEFT, rank);

			// Draw player name
			al_draw_text(font, al_map_rgb(255, 255, 255),
						board_x + 0.18f * board_width,
						y_pos,
						ALLEGRO_ALIGN_LEFT, scores[i].name);

			// Draw player score
			char score_str[16];
			snprintf(score_str, sizeof(score_str), "%d", scores[i].score);
			al_draw_text(font, al_map_rgb(255, 255, 255),
						board_x + 0.95f * board_width,
						y_pos,
						ALLEGRO_ALIGN_RIGHT, score_str);
		}

		// Draw left navigation arrow if not on first page
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
				draw_x + arrow_w, draw_y, 
				-arrow_w,  // Flip horizontally	
				arrow_h,
				0
			);
		}

		// Draw right navigation arrow if more pages remain
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

		// Instruction to exit scoreboard
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

	// ---------- Count number of lines ----------
	int count = 0;
	char ch;
	while (!feof(file)) 
	{
		ch = fgetc(file);
		if (ch == '\n') count++;	// Increment count for each newline found
	}
	count++; // Account for the last line if it doesn't end with newline
	rewind(file); // Reset file pointer to the beginning for reading

	// ---------- Allocate memory for line pointers ----------
	char** lines = (char**)malloc(count * sizeof(char*));
	if (!lines) 
	{
		fclose(file);
		return NULL;
	}

	// ---------- Read lines one by one ----------
	char buffer[256];
	int i = 0;
	while (fgets(buffer, sizeof(buffer), file) && i < count) 
	{
		buffer[strcspn(buffer, "\n")] = 0; 	// Remove trailing newline character
		lines[i] = strdup(buffer);			// Duplicate the string into heap memory
		if (!lines[i]) {
			// Free already allocated strings on failure
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

	// -------- Load credits from file or use fallback --------
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

		// Count fallback lines
		credit_count = 0;
		while (fallback_credits[credit_count]) credit_count++;

		// Allocate space for fallback credit
		credits = (char**)malloc((credit_count+1) * sizeof(char*));

		// Copy fallback strings
		for (int i = 0; i < credit_count; i++)
			credits[i] = strdup(fallback_credits[i]);

		credits[credit_count] = NULL;
	}

	// -------- Load background image --------
	ALLEGRO_BITMAP *credits_bg = al_load_bitmap("assets/images/credits_bg.png");
	if (!credits_bg) 
		credits_bg = al_load_bitmap("assets/images/backgroundMenu.png");

	// -------- Initialize scrolling state and timer --------
	float scroll_pos = al_get_display_height(display);
	bool credits_done = false;
	ALLEGRO_TIMER *timer = al_create_timer(1.0 / 1000.0);
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_start_timer(timer);

	// -------- Main loop --------
	while (!credits_done)
	{
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);

		if(event_handle(queue, &event, &credits_done, display) == STATE_EXIT)
				next_state = STATE_EXIT;
		
		// Any non-ALT/F4/F11 key will also end credits early
		else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			if (event.keyboard.keycode != ALLEGRO_KEYMOD_ALT  && event.keyboard.keycode != ALLEGRO_KEY_F4 &&
				event.keyboard.keycode != ALLEGRO_KEY_F11 )
			{
				credits_done = true;
			}
		}
		
		// Timer event: update scroll position
		else if (event.type == ALLEGRO_EVENT_TIMER) 
		{
			// Scroll speed
			scroll_pos -= 0.7f;

			if (scroll_pos < -1300.0f)
			{
				credits_done = true;
				break;
			}
		}

		// -------- Draw background --------
		if (credits_bg) 
			al_draw_scaled_bitmap(credits_bg, 0, 0, al_get_bitmap_width(credits_bg),
								al_get_bitmap_height(credits_bg), 0, 0, 
								al_get_display_width(display), al_get_display_height(display), 0);
		else 
			al_clear_to_color(al_map_rgb(0, 0, 0));

		// Add semi-transparent black overlay for better contrast (like scoreboard)
		al_draw_filled_rectangle(0, 0, 
						 al_get_display_width(display), 
						 al_get_display_height(display), 
						 al_map_rgba(0, 0, 0, 200));

		// -------- Render each line of credits --------
		float y = scroll_pos;
		for (int i = 0; i < credit_count; i++) 
		{
			ALLEGRO_COLOR color = al_map_rgb(255, 255, 255);
			int font_size = al_get_font_line_height(font);
			float x = al_get_display_width(display) / 2;

			// Apply custom styles based on keywords
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

	// Allocate space for top scores
	score_t * topScores=(score_t *)calloc(100,sizeof(score_t));
	int count = getTopScore(topScores, 100);	// Load up to 100 scores
	
	// Check if current score qualifies for highscore
	bool highscore = isHighScore(getScore(), topScores, count);
	
	// Player name entry buffer
	char name[15] = "";
	int name_pos = 0;

	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	setup_event_queue(queue, display);

	bool done = false;
	ALLEGRO_EVENT event;

	while (!done) 
	{
		// Draw background scaled to screen size
		al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
								  0, 0, al_get_display_width(display), al_get_display_height(display), 0);


		// Draw "GAME OVER" title
		al_draw_text(font, al_map_rgb(255, 255, 255),
					 al_get_display_width(display)/2,
					 al_get_display_height(display) * 0.1f,
					 ALLEGRO_ALIGN_CENTER, "GAME OVER");

		// Show player's score
		char scoreText[64];
		snprintf(scoreText, sizeof(scoreText), "Your score: %-6d", getScore());
		al_draw_text(font, al_map_rgb(255, 255, 0),
					 al_get_display_width(display)/2,
					 al_get_display_height(display) * 0.2f,
					 ALLEGRO_ALIGN_CENTER, scoreText);

		// Draw Top Scores header
		al_draw_text(font, al_map_rgb(255, 255, 255),
					 al_get_display_width(display)/2,
					 al_get_display_height(display) * 0.3f,
					 ALLEGRO_ALIGN_CENTER, "Top Scores:");

		// Show top 10 scores
		for (int i = 0; i < 10; ++i) 
		{
			char buffer[64];
			snprintf(buffer, sizeof(buffer), "%-2d. %-15s - %-6d", i + 1, topScores[i].name, topScores[i].score);
			al_draw_text(font, al_map_rgb(255, 255, 255),
						 al_get_display_width(display)/2,
						 al_get_display_height(display) * (0.35f + i * 0.05f),
						 ALLEGRO_ALIGN_CENTER, buffer);
		}

		// Prompt user input if score is high enough
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

		// Handle character input for name entry
		if (event.type == ALLEGRO_EVENT_KEY_CHAR && name_pos < 14) 
		{
			if (event.keyboard.unichar > 32 && event.keyboard.unichar <= 126) 
			{
				name[name_pos++] = event.keyboard.unichar;
				name[name_pos] = '\0';
			} 
		}
		// Handle backspace for editing name
		if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && name_pos > 0)
			name[--name_pos] = '\0';

		// Confirm and save score
		if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_ENTER)
		{
			if (!highscore || (highscore && name_pos > 0)) 
			{
				if (highscore)
					topScoreUpdate(getScore(), name); // Add new high score
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