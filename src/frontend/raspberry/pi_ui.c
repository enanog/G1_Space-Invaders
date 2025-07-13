#include "pi_ui.h"
#include "disdrv.h"
#include <stdio.h>
#include "joydrv.h"
#include "entity.h"
#include "font.h"
#include <unistd.h>
#include "score.h"
#include "game.h"
#include <ctype.h>  // For toupper()
#include "playSound.h"

/* ==================== CONSTANTS AND DEFINITIONS ==================== */

#define BITCHECK(x,n) (((x)>>(n))&1)
#define ENEMIES_ROW 2
#define ENEMIES_COLUMN 3
#define DEBOUNCE_DELAY_MS 20
#define FRAME_DELAY_MS (1000/60)  // 60 FPS
#define SCROLL_DELAY_US 40000
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Menu state enumerations
enum {
    RESUME,
    START,
    SCOREBOARD,
    EXIT,
    MENU,
    CONTINUE
};

enum {
    PAUSED_FRONT = 8,
    GAME
};

// Menu strings
static const char *menu_strings[] = {"RSM", "STA", "TOP", "EXT", "MEN", "CON"};

// Invader sprite pattern
static const uint16_t invader_sprite[16] = {
    0x07E0, 0x1FF8, 0x3FFC, 0x6F6C, 0xFFFE, 0xFFFE, 0xFFFE, 0xF7DE,
    0xFFFE, 0x1C38, 0x3C3C, 0xE6DE, 0xCFF3, 0xC631, 0x3C3C, 0x180C
};

/* ==================== STATIC VARIABLES ==================== */

static char nameOfPlayer[4];  // Buffer for player name input

/* ==================== FORWARD DECLARATIONS ==================== */

// Input handling
static input_t joy_get_input(void);
static int debounce_joystick_switch(void);

// Menu and game state
static int menu_game(bool resumeLastGame);
static int game_paused(input_t *input);
static void handle_game_state(input_t *input, long long *lastTime, int *state, 
                            bool *running, int *level, int *lives);
static void handle_pause_state(input_t *input, long long *lastTime, int *state, 
                             bool *running, int *level, int *lives);
static void handle_game_over(void);

// Drawing functions
static void draw_entity(hitbox_t hitbox);
static void draw_player(void);
static void draw_enemies(void);
static void draw_barriers(void);
static void draw_bullets(void);
static void draw_player_bullet(void);
static void draw_enemies_bullets(void);
static void draw_mothership(void);
static void pi_ui_render(void);

// Display helpers
static void draw_char(char c, dcoord_t cords);
static void draw_string(const char *str, dcoord_t cords);
static void draw_number_centered(int number, const char *prefix);
static void draw_scrolling_text(const char *msg);

// UI screens
static void show_invader_sprite(void);
static void show_top_scores(void);
static void show_level_up(int level);
static void show_lives_remaining(int lives);

// Utility functions
static bool is_high_score(int score, score_t topScores[], int count);
void get_name(char *name_out);

/* ==================== PUBLIC FUNCTIONS ==================== */

/**
 * @brief Initialize the PI UI system
 */
void pi_ui_init(void) {
    disp_init();
    joy_init();
}

/**
 * @brief Main menu loop
 */
/**
 * @brief Main menu loop
 */
void pi_ui_menu(void) {
    int state = MENU;
    int selectedOption = START;  // Start with "STA" selected
    bool running = true;

    // Initial screen - show enemy sprite
    show_invader_sprite();
    disp_clear();  // Clear after showing sprite
    while (running) {
        joyinfo_t joyInfo = joy_read();

        switch (state) {
            case MENU:
            	playSound_playMusic(INTRO_MUSIC);
                // Handle joystick navigation
                if (joyInfo.y > 40) {  // Joystick down
                    selectedOption = (selectedOption == START) ? CONTINUE :
                    (selectedOption == CONTINUE) ? EXIT :
                    (selectedOption == EXIT) ? SCOREBOARD : START;
                    playSound_play(SOUND_MENU);
                    usleep(200000);  // Small delay to prevent rapid scrolling
                }
                else if (joyInfo.y < -40) {  // Joystick up
                    selectedOption = (selectedOption == START) ? SCOREBOARD :
                    (selectedOption == SCOREBOARD) ? EXIT :
                    (selectedOption == EXIT) ? CONTINUE : START;
                    playSound_play(SOUND_MENU);
                    usleep(200000);  // Small delay to prevent rapid scrolling
                }

                // Draw current selection
                disp_clear();
                draw_string(menu_strings[selectedOption], (dcoord_t){1, 6});
                disp_update();

                // Handle selection with switch button
                if (debounce_joystick_switch()) {
                    if (selectedOption == CONTINUE) {
                        // Check if saved game is over
                        input_t dummyInput = {0};
                        if (game_update(dummyInput) == RUNNING) {
                            // Game can be continued
                            state = menu_game(true);  // Resume saved game
                        } else {
                            // Game over - start new game
                            state = menu_game(false);
                        }
                    } else {
                        state = selectedOption;  // For other options
                    }
                    usleep(200000);  // Debounce delay
                }
                break;

            case START:
                state = menu_game(false);  // Always start new game
                break;

            case CONTINUE:
                // This should never be reached as CONTINUE is handled above
                state = menu_game(true);
                break;

            case SCOREBOARD:
                show_top_scores();
                state = MENU;  // Return to menu after showing scores
                break;

            case EXIT:
            	disp_clear();
            	disp_update();
                running = false;
                break;
        }
    }
}

/* ==================== PRIVATE FUNCTIONS ==================== */

/**
 * @brief Main game loop
 * @param resumeLastGame Whether to resume previous game
 * @return Next state after game ends
 */
static int menu_game(bool resumeLastGame) {
    input_t input = {0};
    int state = GAME;
    bool running = true;
    
    // Initialize game state
    game_init(ENEMIES_ROW, ENEMIES_COLUMN, resumeLastGame);
    if (game_update(input)) {  // Check if game was already lost
        game_init(ENEMIES_ROW, ENEMIES_COLUMN, false);
    }
    
    long long lastTime = getTimeMillis();
    int level = getLevel();
    int lives = getPlayerLives();
    
	playSound_playMusic(GAME_MUSIC);

    while (running) {
        switch (state) {
            case GAME:
            	playSound_resumeMusic();
                handle_game_state(&input, &lastTime, &state, &running, &level, &lives);
                break;
                
            case PAUSED_FRONT:
            	playSound_pauseMusic();
                handle_pause_state(&input, &lastTime, &state, &running, &level, &lives);
                break;
        }
    }
    
    return state;
}

/**
 * @brief Handle game running state
 */
static void handle_game_state(input_t *input, long long *lastTime, int *state, 
                            bool *running, int *level, int *lives) {
    *input = joy_get_input();
    long long currentTime = getTimeMillis();
    
    if (currentTime - *lastTime > FRAME_DELAY_MS) {
        *lastTime = currentTime;
        
        disp_clear();
        int gameStatus = game_update(*input);
        
        if (gameStatus != RUNNING) {
            handle_game_over();
            *running = false;
            *state = SCOREBOARD;
            return;
        }
        
        pi_ui_render();
        disp_update();
    }
    
    // Check for pause or state change
    if (input->pause) {
        *state = PAUSED_FRONT;
    }
    else if (*level != getLevel() || *lives != getPlayerLives()) {
        *state = PAUSED_FRONT;
    }
}

/**
 * @brief Handle game over scenario
 */
static void handle_game_over(void) {
    draw_scrolling_text("GAME OVER");
    
    score_t topScores[3];
    int count = getTopScore(topScores, 3);
    int newScore = getScore();
    
    if (is_high_score(newScore, topScores, count)) {
        get_name(nameOfPlayer);
        topScoreUpdate(newScore, nameOfPlayer);
    }
}

/**
 * @brief Handle pause state
 */
static void handle_pause_state(input_t *input, long long *lastTime, int *state,
                               bool *running, int *level, int *lives) {
    if (*level != getLevel()) {
        *level = getLevel();
        show_level_up(*level);
        *state = GAME;
        *lastTime = getTimeMillis();
        input_t aux = {0, 0, 1, 0};
        game_update(aux);
    }
    else if (*lives != getPlayerLives()) {
        *lives = getPlayerLives();
        show_lives_remaining(*lives);
        *state = GAME;
        *lastTime = getTimeMillis();
        input_t aux = {0, 0, 1, 0};
        game_update(aux);
    }
    else {
        int newState = game_paused(input);
        if (newState == RESUME) {
            *state = GAME;
            *lastTime = getTimeMillis();
            input_t aux = {0, 0, 1, 0};
            game_update(aux);
        }
        else if (newState == MENU) {
            *running = false;  // Exit to main menu
            *state = MENU;
        }
        else if (newState == EXIT) {
            *running = false;  // Exit game completely
            *state = EXIT;
        }
    }
}

/**
 * @brief Pause menu handler
 * @param input Pointer to input structure
 * @return Selected menu option
 */
static int game_paused(input_t *input) {
    int state = RESUME;
    joyinfo_t joyInfo;
    bool selectionMade = false;

    while (!selectionMade) {
        // Handle joystick navigation
        joyInfo = joy_read();

        if (joyInfo.y > 40) {  // Joystick down
            state = (state == RESUME) ? MENU :
            (state == MENU) ? EXIT : RESUME;
            usleep(200000);  // Small delay to prevent rapid scrolling
        }
        else if (joyInfo.y < -40) {  // Joystick up
            state = (state == RESUME) ? EXIT :
            (state == EXIT) ? MENU : RESUME;
            usleep(200000);  // Small delay to prevent rapid scrolling
        }

        // Draw current selection
        disp_clear();
        draw_string(menu_strings[state], (dcoord_t){1, 6});
        disp_update();

        // Handle selection with switch button
        if (debounce_joystick_switch()) {
            switch (state) {
                case RESUME:
                    selectionMade = true;
                    break;

                case MENU:  // Return to main menu
                    return MENU;

                case EXIT:  // Exit game
                    return EXIT;
            }
            usleep(200000);  // Debounce delay
        }

        usleep(10000);  // Small delay to prevent CPU overuse
    }

    return RESUME;
}

/**
 * @brief Get joystick input with debouncing
 * @return Current input state
 */
static input_t joy_get_input(void) {
    input_t input = {0};
    joyinfo_t joyInfo = joy_read();
    
    input.direction = (joyInfo.x > 40) - (joyInfo.x < -40);
    input.shot = (joyInfo.y > 40 || joyInfo.y < -40);
    
    if (debounce_joystick_switch()) {
        input.pause = !input.pause;
    }
    
    return input;
}

/**
 * @brief Debounce joystick switch
 * @return 1 if switch pressed, 0 otherwise
 */
static int debounce_joystick_switch(void) {
    static int last_state = 1;
    static int debounced_state = 1;
    static long long last_debounce_time = 0;
    
    int current_state = joy_read().sw;
    long long now = getTimeMillis();
    
    if (current_state != last_state) {
        last_debounce_time = now;
    }
    
    if ((now - last_debounce_time) > DEBOUNCE_DELAY_MS) {
        if (debounced_state != current_state) {
            debounced_state = current_state;
            if (debounced_state == J_PRESS) {
                last_state = current_state;
                return 1;
            }
        }
    }
    
    last_state = current_state;
    return 0;
}

/* ==================== DRAWING FUNCTIONS ==================== */

/**
 * @brief Render all game entities
 */
static void pi_ui_render(void) {
    draw_player();
    draw_enemies();
    draw_barriers();
    draw_bullets();
    draw_mothership();
}

/**
 * @brief Draw player entity
 */
static void draw_player(void) {
    hitbox_t playerHitbox = getPlayerPosition();
    draw_entity(playerHitbox);
}

/**
 * @brief Draw all active enemies
 */
static void draw_enemies(void) {
    for (int row = 0; row < ENEMIES_ROW_MAX; row++) {
        for (int col = 0; col < ENEMIES_COLUMNS_MAX; col++) {
            if (getIsEnemyAlive(row, col)) {
                draw_entity(getEnemyPosition(row, col));
            }
        }
    }
}

/**
 * @brief Draw all active barriers
 */
static void draw_barriers(void) {
    for (int barrier = 0; barrier < BARRIER_QUANTITY_MAX; barrier++) {
        for (int row = 0; row < BARRIER_ROWS; row++) {
            for (int col = 0; col < BARRIER_COLUMNS; col++) {
                if (getBarrierIsAlive(barrier, row, col)) {
                    draw_entity(getBarrierPosition(barrier, row, col));
                }
            }
        }
    }
}

/**
 * @brief Draw all active bullets (player and enemies)
 */
static void draw_bullets(void) {
    draw_player_bullet();
    draw_enemies_bullets();
}

/**
 * @brief Draw player bullet if active
 */
static void draw_player_bullet(void) {
    bullet_t playerBullet = getPlayerBulletinfo();
    if (playerBullet.active) {
        draw_entity(playerBullet.hitbox);
    }
}

/**
 * @brief Draw all active enemy bullets
 */
static void draw_enemies_bullets(void) {
    bullet_t enemyBulletBitMap[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];
    getEnemiesBulletsInfo(enemyBulletBitMap);
    
    for (int row = 0; row < ENEMIES_ROW; row++) {
        for (int col = 0; col < ENEMIES_COLUMN; col++) {
            if (enemyBulletBitMap[row][col].active) {
                draw_entity(enemyBulletBitMap[row][col].hitbox);
            }
        }
    }
}

/**
 * @brief Draw mothership if active
 */
static void draw_mothership(void) {
    if (getIsMothershipAlive()) {
        draw_entity(getMothershipPosition());
    }
}

/**
 * @brief Generic entity drawing function
 * @param hitbox Entity's hitbox coordinates
 */
static void draw_entity(hitbox_t hitbox) {
    uint8_t start_x = (uint8_t)(hitbox.start.x * DISP_CANT_X_DOTS);
    uint8_t start_y = (uint8_t)(hitbox.start.y * DISP_CANT_Y_DOTS);
    uint8_t end_x = (uint8_t)(hitbox.end.x * DISP_CANT_X_DOTS);
    uint8_t end_y = (uint8_t)(hitbox.end.y * DISP_CANT_Y_DOTS);
    
    // Clamp to display bounds
    start_x = MIN(start_x, DISP_CANT_X_DOTS - 1);
    end_x = MIN(end_x, DISP_CANT_X_DOTS - 1);
    start_y = MIN(start_y, DISP_CANT_Y_DOTS - 1);
    end_y = MIN(end_y, DISP_CANT_Y_DOTS - 1);
    
    for (uint8_t x = start_x; x <= end_x; x++) {
        for (uint8_t y = start_y; y <= end_y; y++) {
            disp_write((dcoord_t){x, y}, D_ON);
        }
    }
}

/* ==================== DISPLAY HELPERS ==================== */

/**
 * @brief Draw a character at specified coordinates
 * @param c Character to draw
 * @param cords Drawing coordinates
 */
void draw_char(char c, dcoord_t cords) {
    int char_index = -1;

    if (c >= 'A' && c <= 'Z') {
        char_index = get_letter_bitmap(c);
    }
    else if (c >= '0' && c <= '9') {
        char_index = get_number_bitmap(c);
        // Use the numbers array instead of letters
        for (int row = 0; row < FONT_ROWS; row++) {
            for (int col = 0; col < FONT_COLS; col++) {
                dcoord_t pixel = {cords.x + col, cords.y + row};
                if (pixel.x < 0 || pixel.x > DISP_MAX_X) continue;
                if (numeros_5x4[char_index][row][col]) {
                    disp_write(pixel, D_ON);
                }
            }
        }
        return;
    }

    // Default to letters if not a number
    if (char_index >= 0) {
        for (int row = 0; row < FONT_ROWS; row++) {
            for (int col = 0; col < FONT_COLS; col++) {
                dcoord_t pixel = {cords.x + col, cords.y + row};
                if (pixel.x < 0 || pixel.x > DISP_MAX_X) continue;
                if (letras_5x4[char_index][row][col]) {
                    disp_write(pixel, D_ON);
                }
            }
        }
    }
}

/**
 * @brief Draw a string at specified coordinates
 * @param str String to draw
 * @param cords Drawing coordinates
 */
static void draw_string(const char *str, dcoord_t cords) {
    for (int i = 0; str[i] != '\0' && i < 3; i++) {
        draw_char(str[i], cords);
        cords.x += FONT_COLS + 1;
    }
}

/**
 * @brief Draw a number centered with prefix
 * @param number Number to display
 * @param prefix Prefix text (3 chars max)
 */
static void draw_number_centered(int number, const char *prefix) {
    if (number < 0) number = -number;
    
    // Convert number to string
    char buffer[12];
    int len = 0;
    do {
        buffer[len++] = '0' + (number % 10);
        number /= 10;
    } while (number > 0);
    
    // Reverse digits
    for (int i = 0; i < len / 2; i++) {
        char tmp = buffer[i];
        buffer[i] = buffer[len - 1 - i];
        buffer[len - 1 - i] = tmp;
    }
    
    // Calculate scrolling animation
    const uint8_t y_center = 7;
    const int total_width = len * FONT_COLS + (len - 1) * 1;
    const int start_x = DISP_MAX_X + 1;
    const int end_x = -total_width;
    
    for (int scroll_x = start_x; scroll_x >= end_x; scroll_x--) {
        disp_clear();
        draw_string(prefix, (dcoord_t){1, 0});
        
        int digit_x = scroll_x;
        for (int i = 0; i < len; i++) {
            draw_char(buffer[i], (dcoord_t){digit_x, y_center});
            digit_x += FONT_COLS + 1;
        }
        
        disp_update();
        if (scroll_x != end_x) usleep(80000);
    }
}

/**
 * @brief Draw scrolling text message
 * @param msg Message to display
 */
static void draw_scrolling_text(const char *msg) {
    int len = 0;
    while (msg[len] != '\0') len++;
    
    int total_width = len * (FONT_COLS + 1) - 1;
    int final_scroll = total_width + DISP_CANT_X_DOTS;
    
    for (int scroll = 0; scroll <= final_scroll; scroll++) {
        disp_clear();
        int x = (DISP_MAX_X - FONT_COLS) - scroll;
        dcoord_t coord = {x, 6};
        
        for (int i = 0; i < len; i++) {
            if (msg[i] != ' ') {
                draw_char(msg[i], coord);
            }
            coord.x += FONT_COLS + 1;
        }
        
        disp_update();
        if (scroll != final_scroll) usleep(SCROLL_DELAY_US);
    }
}

/* ==================== UI SCREENS ==================== */

/**
 * @brief Show invader sprite on screen
 */
static void show_invader_sprite(void) {
    disp_clear();
    
    for (uint8_t y = 0; y < 16; y++) {
        for (uint8_t x = 0; x < 16; x++) {
            if (invader_sprite[y] & (1 << (15 - x))) {
                disp_write((dcoord_t){x, y}, D_ON);
            }
        }
    }
    
    disp_update();
    
    // Wait for button press but with timeout to prevent hanging
    long long startTime = getTimeMillis();
    while (!debounce_joystick_switch() && 
          (getTimeMillis() - startTime < 3000)) {
        usleep(10000);
    }
}

/**
 * @brief Display top 3 scores
 */
static void show_top_scores(void) {
    score_t topScores[3];
    int topCount = getTopScore(topScores, 3);
    
    for (int i = 0; i < topCount; i++) {
        char name[4];
        for (int j = 0; j < 3 && topScores[i].name[j] != '\0'; j++) {
            name[j] = toupper(topScores[i].name[j]);
        }
        name[3] = '\0';
        
        draw_number_centered(topScores[i].score, name);
        disp_clear();
        disp_update();
    }
}

/**
 * @brief Show level up notification
 * @param level New level number
 */
static void show_level_up(int level) {
    draw_number_centered(level, "LVL");
}

/**
 * @brief Show remaining lives
 * @param lives Number of lives remaining
 */
static void show_lives_remaining(int lives) {
    draw_number_centered(lives, "LIV");
}

/**
 * @brief Get player name input
 * @param name_out Buffer to store name (must be at least 4 chars)
 */
void get_name(char *name_out) {
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int letter_idx = 0;
    int pos = 0;
    
    draw_scrolling_text("YOU ARE IN THE TOP THREE PUT YOUR THREE LETTER NICKNAME");
    
    while (pos < 3) {
        disp_clear();
        
        // Show already selected letters
        dcoord_t cords = {1, 6};
        for (int i = 0; i < pos; i++) {
            draw_char(name_out[i], cords);
            cords.x += FONT_COLS + 1;
        }
        
        // Show current letter selection
        draw_char(alphabet[letter_idx], cords);
        disp_update();
        
        // Handle input
        joyinfo_t joyInfo;
        long long last_move = getTimeMillis();
        bool moved = false;
        
        while (!moved) {
            joyInfo = joy_read();
            long long now = getTimeMillis();
            
            if (joyInfo.y > 40 && (now - last_move > 80)) {
                letter_idx = (letter_idx + 1) % 26;
                last_move = now;
                moved = true;
            }
            else if (joyInfo.y < -40 && (now - last_move > 80)) {
                letter_idx = (letter_idx - 1 + 26) % 26;
                last_move = now;
                moved = true;
            }
            else if (debounce_joystick_switch()) {
                name_out[pos++] = alphabet[letter_idx];
                letter_idx = 0;
                moved = true;
            }
            
            usleep(10000);
        }
    }
    
    name_out[3] = '\0';
}

/**
 * @brief Check if score qualifies as high score
 * @param score Score to check
 * @param topScores Array of top scores
 * @param count Number of scores in array
 * @return true if score is high score, false otherwise
 */
static bool is_high_score(int score, score_t topScores[], int count) {
    return (count < 3) || (score > topScores[count - 1].score);
}
