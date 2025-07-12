#include "pi_ui.h"
#include "disdrv.h"
#include <stdio.h>
#include "joydrv.h"
#include "entity.h"
#include "font.h"
#include <unistd.h>
#include "score.h"

static input_t joy_get_input(void);
static void draw_player(void);
static void draw_enemies(void);
static void draw_barriers(void);
static void draw_enemies_bullets(void);
static void draw_player_bullet(void);

static void draw_mothership(void);

static int menu_game(bool resumeLastGame);
static void pi_ui_render(void);

static void draw_hitbox_filled(hitbox_t hitbox);
static void disp_write_string(const char *str);
const char * menu_strings[]={"RSM","STA","TOP","EXT","MEN","CON"};
static void disp_write_char(const char c,dcoord_t cords);
static int game_paused (input_t * input);

static int debounce_joystick_switch(void) ;
static void show_top3(void);
static void disp_write_string_top(const char *str);
static void disp_write_digit(char digit,dcoord_t cords);
void disp_write_long_number_center(int number,char const *str);

static void show_16x16_enemy(void);
static const uint16_t invader_sprite[16] = {
    0x07E0,
    0x1FF8,
    0x3FFC,
    0x6F6C,
    0xFFFE,
    0xFFFE,
    0xFFFE,
    0xF7DE,
    0xFFFE,
    0x1C38,
    0x3C3C,
    0xE6DE,
    0xCFF3,
    0xC631,
    0x3C3C,
    0x180C
};

#define BITCHECK(x,n) (((x)>>(n))&1)


#define ENEMIES_ROW 2
#define ENEMIES_COLUMN 3

enum {
      RESUME,
      START,
      SCOREBOARD,
      EXIT,MENU,CONTINUE};
enum{
    PAUSED=8,GAME
};

void pi_ui_init(void) 
{
    disp_init();
    joy_init();
}
void pi_ui_menu(void)
{
    int state=MENU, nextState=START, running=true;
    joyinfo_t joyInfo;
    //show_16x16_enemy();
    
    while(running)
    {
        switch (state)
        {
        case MENU:
            joyInfo = joy_read();
            if (debounce_joystick_switch())
            {
                state=nextState;
                break;
            }
            if (joyInfo.y > 40)
            {
                if (nextState==START)
                    nextState=CONTINUE;
                else if ( nextState==CONTINUE)
                    nextState=EXIT;
                else if (nextState==EXIT)
                    nextState=SCOREBOARD;
                else if (nextState==SCOREBOARD)
                    nextState=START;
            }
            else if (joyInfo.y<-40)
            {
                if (nextState==START)
                    nextState=SCOREBOARD;
                else if ( nextState==SCOREBOARD)
                    nextState=EXIT;
                else if (nextState==EXIT)
                    nextState=CONTINUE;
                else if (nextState==CONTINUE)
                    nextState=START;
            }
            else
            {
                disp_clear();
                disp_write_string(menu_strings[nextState]);
                disp_update();
            }
            
            break;
        
        case START:
            state=menu_game(false);
            break;
        case CONTINUE:
            state=menu_game(true);
            break;
        case SCOREBOARD:
            printf("Showing top 3 scores...\n");
            show_top3();
            state=MENU;
            break;
        case EXIT:
            show_16x16_enemy();
            running=false;
            break;
        default:
            break;
        }
    }
}

static int menu_game(bool resumeLastGame)
{
    input_t input={0,0,0,0};
    printf("Initializing game...\n");
    bool running = true;
    game_init(ENEMIES_ROW,ENEMIES_COLUMN, resumeLastGame);
    if (game_update(input))//si es una partida ya perdida
        game_init(ENEMIES_ROW,ENEMIES_COLUMN,false);
    long long lastTime = getTimeMillis();
    long long currentTime;
    int state=GAME;
    while (running)
    {
        switch(state)
        {   
            case GAME:
           /// printf("ENTRO?%d\n",input.pause);
                input=joy_get_input();
                //printf("ENTRO?%d\n",input.pause);
                currentTime= getTimeMillis();
                if (currentTime - lastTime > 1000/60) // 60 FPS
                {
                    //printf ("%lld\n",currentTime-lastTime);
                    lastTime = currentTime;
                    disp_clear();
                    int i;
                //printf(" ,%d,%d,%d\n", input.direction, input.shot, input.pause);
                    if((i=game_update(input))!=0)//si perdes
                    {
                        running = false;
                        state=MENU;
                        printf("Game over!\n");
                    }
                    pi_ui_render();
                    disp_update();
                }
                if (input.pause)
                {
                    game_update(input);
                    printf(" ,%d,%d,%d\n", input.direction, input.shot, input.pause);
                    state=PAUSED;
                   //  printf("Entre pausa\n");
                }
            break;
            case PAUSED:
            if ((state=game_paused(&input))==RESUME)
                {
                    //for (int i=0;i<100000000;i++);
                    state=GAME;
                    lastTime=getTimeMillis();
                    input_t aux={0,0,1,0};
                    game_update(aux);
                   // printf("Sali pausa\n");
            
                }
            else
            {
                running=false;
            }
            break;
        }
    }
    return state;
}


static int game_paused(input_t *input)
{
    int state=RESUME;
    joyinfo_t joyInfo;
    while (!debounce_joystick_switch())
    {
        joyInfo = joy_read();
        if (joyInfo.y > 40)
        {
            if (state==RESUME)
                state=MENU;
            else if (state==MENU)
                state=EXIT;
            else if (state==EXIT)
                state=RESUME;
        }
        else if (joyInfo.y<-40)
        {
            if (state==RESUME)
                state=EXIT;
            else if (state==EXIT)
                state=MENU;
            else if (state==MENU)
                state=RESUME;
        }
        else
        {
            disp_clear();
            disp_write_string(menu_strings[state]);
            disp_update();
        }
    }
    return state;
}

static void pi_ui_render(void)
{
    draw_player();
    draw_enemies();
    draw_barriers();
    draw_player_bullet();
    draw_enemies_bullets();
    draw_mothership();
}
static int debounce_joystick_switch(void) {
    static int last_state = 1; // Assume switch starts HIGH (not pressed)
    static int debounced_state = 1;
    static long long last_debounce_time = 0;
    const long long debounce_delay = 20; // ms

    int current_state = joy_read().sw;
    long long now = getTimeMillis();

    if (current_state != last_state) {
        last_debounce_time = now;
    }

    if ((now - last_debounce_time) > debounce_delay) {
        if (debounced_state != current_state) {
            debounced_state = current_state;
            if (debounced_state == J_PRESS) {
                last_state = current_state;
                return 1; // Falling edge detected
            }
        }
    }

    last_state = current_state;
    return 0;
}
static input_t joy_get_input(void)
{
    //printf("entre\n");
    input_t input = {0, 0, 0, 0};
    joyinfo_t joyInfo = joy_read();

    input.direction = (joyInfo.x>40) - (joyInfo.x<-40);

    input.shot = (joyInfo.y>40 || joyInfo.y<-40);
   // printf("x:%d,y:%d,shot:%d\n",joyInfo.x,joyInfo.y,input.shot);
    //printf("direction:%d\n",input.direction);
    if (debounce_joystick_switch())
    {
        input.pause=!input.pause;
        printf("debouce %d\n",input.pause);
    }
    return input;
    
}

static void draw_player(void)
{
    hitbox_t playerHitbox = getPlayerPosition();
    // Coordenada inicial del jugador
    uint8_t base_x = (uint8_t)(playerHitbox.start.x * DISP_CANT_X_DOTS);
    uint8_t base_y = (uint8_t)(playerHitbox.start.y * DISP_CANT_Y_DOTS);
    //printf("x:%f,y:%f\n",playerHitbox.start.x,playerHitbox.start.y);
    int x, y;
    for (x=0;x<PLAYER_WIDTH * DISP_CANT_X_DOTS; x++)
    {
        for (y=0;y<PLAYER_HEIGHT * DISP_CANT_Y_DOTS; y++)
        {
           // printf("basex:%d,basey:%d\n",base_x+x,base_y+y);
            if((y==0&&x==0)||(y==0 && x==(PLAYER_WIDTH*DISP_CANT_X_DOTS)-1))
                continue;
            dcoord_t cord={base_x+x,base_y+y};
            disp_write(cord,D_ON);  
        }
    }
}
static void draw_enemies(void)
{
    hitbox_t enemyHitbox;
    int row, col;
    for (row = 0; row < ENEMIES_ROW_MAX; row++)
    {
        for (col = 0; col < ENEMIES_COLUMNS_MAX; col++)
        {
           
            if (getIsEnemyAlive(row,col))
            {
                enemyHitbox = getEnemyPosition(row, col);
                draw_hitbox_filled(enemyHitbox);
            }
        }
    }
}
static void draw_barriers(void)
{
    hitbox_t barrierHitbox;
    int barrier, row, col;
    for (barrier = 0; barrier < BARRIER_QUANTITY_MAX; barrier++)
    {
        for (row = 0; row < BARRIER_ROWS; row++)
        {
            for (col = 0; col < BARRIER_COLUMNS; col++)
            {
                if (!getBarrierIsAlive(barrier, row, col))
                {
                    continue;
                }
                barrierHitbox = getBarrierPosition(barrier, row, col);
               draw_hitbox_filled(barrierHitbox);
            }
        }
    }
}
static void draw_hitbox_filled(hitbox_t hitbox)
{
    uint8_t start_x = (uint8_t)(hitbox.start.x * DISP_CANT_X_DOTS);
    uint8_t start_y = (uint8_t)(hitbox.start.y * DISP_CANT_Y_DOTS);
    uint8_t end_x = (uint8_t)(hitbox.end.x * DISP_CANT_X_DOTS);
    uint8_t end_y = (uint8_t)(hitbox.end.y * DISP_CANT_Y_DOTS);

    // Clamp to display bounds
    if (start_x >= DISP_CANT_X_DOTS) start_x = DISP_CANT_X_DOTS - 1;
    if (end_x >= DISP_CANT_X_DOTS) end_x = DISP_CANT_X_DOTS - 1;
    if (start_y >= DISP_CANT_Y_DOTS) start_y = DISP_CANT_Y_DOTS - 1;
    if (end_y >= DISP_CANT_Y_DOTS) end_y = DISP_CANT_Y_DOTS - 1;

    for (uint8_t x = start_x; x <= end_x; x++) {
        for (uint8_t y = start_y; y <= end_y; y++) {
            dcoord_t coord = {x, y};
            disp_write(coord, D_ON);
        }
    }
}

static void draw_enemies_bullets(void)
{
    bullet_t enemyBulletBitMap[ENEMIES_ROW_MAX][ENEMIES_COLUMNS_MAX];
    getEnemiesBulletsInfo(enemyBulletBitMap);
    int row, col;
    for (row = 0; row < ENEMIES_ROW; row++)
    {
        for (col = 0; col < ENEMIES_COLUMN; col++)
        {
            if (!enemyBulletBitMap[row][col].active)
            {
                continue;
            }
    
            hitbox_t bulletHitbox = enemyBulletBitMap[row][col].hitbox;
            draw_hitbox_filled(bulletHitbox);
        }
    }
}
static void draw_player_bullet(void)
{
    bullet_t playerBullet = getPlayerBulletinfo();
    if (playerBullet.active)
     {
        hitbox_t bulletHitbox = playerBullet.hitbox;
        draw_hitbox_filled(bulletHitbox);
    }
}
static void draw_mothership(void)
{
    if (getIsMothershipAlive())
    {
        hitbox_t mothershipHitbox = getMothershipPosition();
     
            draw_hitbox_filled(mothershipHitbox);
    }
}
static void disp_write_char(const char c,dcoord_t cords)
{
    int row, col;
    dcoord_t pixel;
    for (row=0;row<FONT_ROWS;row++)
    {
        for(col=0;col<FONT_COLS;col++)
        {
            pixel.x=cords.x+col;
            pixel.y=cords.y+row;
            if(letras_5x4[c-'A'][row][col])
                 disp_write(pixel, D_ON);
        }
    }
}

static void disp_write_string(const char *str)
{
    int i;
    dcoord_t cords={1,6};
    for (i=0;str[i]!='\0' && i<3;i++)
    {
        disp_write_char(str[i],cords);
        cords.x+=FONT_COLS+1;
    }
}
static void disp_write_string_top(const char *str)
{
    int i;
    dcoord_t cords={1,0};
    for (i=0;str[i]!='\0' && i<3;i++)
    {
        disp_write_char(str[i],cords);
        cords.x+=FONT_COLS+1;
    }
}


static void show_16x16_enemy(void)
{
     disp_clear();
     uint8_t y, x;
    for (y=0; y < 16; y++) 
    {
        for (x = 0; x < 16; x++) 
        {
            if (invader_sprite[y] & (1 << (15 - x))) 
            {
                dcoord_t coord = {x, y};
                disp_write(coord, D_ON);
            }
        }
    }
    disp_update();
    usleep(3000000);
    disp_clear();
    disp_update();
}

static void show_top3(void)
{
    score_t topScores[3];
    int topCount = getTopScore(topScores, 3);
    printf("%d aa\n", topCount);
    char aux[4];
    int i,j;
    disp_clear();
    disp_update();;
    for(i=0;i<topCount;i++)
    {
        for (j=0;topScores[i].name[j]!='\0' && j<3;j++)
        {
            aux[j]=topScores[i].name[j];
            if (aux[j]>'a' && aux[j]<'z')
                aux[j]-=32; // Convert to upperca
        }
        aux[j]='\0';
        
        disp_write_long_number_center(topScores[i].score, aux);
        disp_clear();
        disp_update();
    }

}

static void disp_write_digit(char digit, dcoord_t coord)
{
    if (digit < '0' || digit > '9') return;

    int row,col;
    for (row = 0; row < FONT_ROWS; row++) 
    {
        for (col = 0; col < FONT_COLS; col++) 
        {
            if (!numeros_5x4[digit - '0'][row][col]) continue;

            int px = coord.x + col;
            int py = coord.y + row;
            if (px >= 0 && px <= 15 && py >= 0 && py <= 15) 
            {
                disp_write((dcoord_t){px, py}, D_ON);
            }
        }
    }
}
void disp_write_long_number_center(int number, const char *str)
{
    if (number < 0) number = -number;

    char buffer[12];
    int len = 0;

    // Convert number to string in reverse
    do {
        buffer[len++] = '0' + (number % 10);
        number /= 10;
    } while (number > 0);

    // Reverse digits to print left to right
    for (int i = 0; i < len / 2; i++) {
        char tmp = buffer[i];
        buffer[i] = buffer[len - 1 - i];
        buffer[len - 1 - i] = tmp;
    }

    const uint8_t y_center = 7;
    const uint8_t FONT_WIDTH = FONT_COLS;

    int total_width = len * FONT_WIDTH + (len - 1);
    int start_x = DISP_MAX_X - FONT_WIDTH;  // comienza al borde visible derecho
    int final_scroll = total_width + DISP_CANT_X_DOTS;  // cuanto tengo que moverme para sacarlo del todo
    int scroll, i;
    for ( scroll= 0; scroll <= final_scroll; scroll++) 
    {
        disp_clear();
        disp_write_string_top(str);

        int x = start_x - scroll;
        for (i = 0; i < len; i++)
        {
            if (x >= DISP_MIN - FONT_WIDTH && x <= DISP_MAX_X) 
            {
                dcoord_t coord = { .x = x, .y = y_center };
                disp_write_digit(buffer[i], coord);
            }
            x += FONT_WIDTH;
        }

        disp_update();
        if(scroll!= final_scroll)
            usleep(200000);  // 200ms entre frames
    }
}

